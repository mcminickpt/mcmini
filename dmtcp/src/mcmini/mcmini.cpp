#include "mcmini/Thread_queue.h"
#include "mcmini/common/shm_config.h"
#include "mcmini/coordinator/coordinator.hpp"
#include "mcmini/coordinator/restore-objects.hpp"
#include "mcmini/model/config.hpp"
#include "mcmini/model/objects/condition_variables.hpp"
#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/objects/semaphore.hpp"
#include "mcmini/model/objects/thread.hpp"
#include "mcmini/model_checking/algorithm.hpp"
#include "mcmini/model_checking/algorithms/classic_dpor.hpp"
#include "mcmini/real_world/fifo.hpp"
#include "mcmini/real_world/process/dmtcp_process_source.hpp"
#include "mcmini/real_world/process/multithreaded_fork_process_source.hpp"
#include "mcmini/real_world/process/resources.hpp"
#include "mcmini/signal.hpp"

#define _XOPEN_SOURCE_EXTENDED 1

#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

using namespace extensions;
using namespace model;
using namespace model_checking;
using namespace objects;
using namespace real_world;

visible_object_state* translate_recorded_object_to_model(
    const ::visible_object& recorded_object,
    const std::unordered_map<
        void*, std::vector<std::pair<runner_id_t, condition_variable_status>>>
        cv_waiting_threads) {
  // TODO: A function table would be slightly better, but this works perfectly
  // fine too.
  switch (recorded_object.type) {
    case MUTEX: {
      auto mutex_state =
          static_cast<objects::mutex::state>(recorded_object.mut_state);
      pthread_mutex_t* mutex_location =
          (pthread_mutex_t*)recorded_object.location;
      return new objects::mutex(mutex_state, mutex_location);
    }
    case CONDITION_VARIABLE: {
      // Create the condition variable model object with full state information
      auto cv_state = static_cast<objects::condition_variable::state>(
          recorded_object.cond_state.status);

      runner_id_t interacting_thread =
          recorded_object.cond_state.interacting_thread;
      pthread_mutex_t* associated_mutex =
          recorded_object.cond_state.associated_mutex;
      int count = recorded_object.cond_state.count;
      // get waiting threads from the map
      auto it = cv_waiting_threads.find(recorded_object.location);
      std::vector<std::pair<runner_id_t, condition_variable_status>>
          waiters_with_state =
              (it != cv_waiting_threads.end())
                  ? it->second
                  : std::vector<
                        std::pair<runner_id_t, condition_variable_status>>();
      return new objects::condition_variable(cv_state, interacting_thread,
                                             associated_mutex, count,
                                             waiters_with_state);
    }
    case SEMAPHORE: {
      return new objects::semaphore(static_cast<objects::semaphore::state>(
                                        recorded_object.sem_state.status),
                                    recorded_object.sem_state.count);
    }
    // Other objects here
    // case ...  { }
    // ...
    default: {
      std::cerr << "The new object type" << recorded_object.type
                << "hasn't been implemented yet\n";
      std::abort();
    }
  }
}

runner_state* translate_recorded_runner_to_model(
    const ::visible_object& recorded_object) {
  switch (recorded_object.type) {
    case THREAD: {
      return new objects::thread(static_cast<objects::thread::state>(
          recorded_object.thrd_state.status));
    }
    default: {
      std::abort();
    }
  }
}

void finished_trace_classic_dpor(const coordinator& c) {
  static uint32_t trace_id = 1;

  std::stringstream ss;
  const auto& program_model = c.get_current_program_model();
  ss << "TRACE " << trace_id << "\n";
  for (const auto& t : program_model.get_trace()) {
    ss << "thread " << t->get_executor() << ": " << t->to_string() << "\n";
  }
  ss << "\nNEXT THREAD OPERATIONS\n";
  for (const auto& tpair : program_model.get_pending_transitions()) {
    ss << "thread " << tpair.first << ": " << tpair.second->to_string() << "\n";
  }
  std::cout << ss.str();
  std::cout.flush();
  trace_id++;
}

void found_undefined_behavior(const coordinator& c,
                              const undefined_behavior_exception& ub) {
  std::cerr << "UNDEFINED BEHAVIOR:\n" << ub.what() << std::endl;
  finished_trace_classic_dpor(c);
}

void found_deadlock(const coordinator& c) {
  std::cerr << "DEADLOCK" << std::endl;
  std::stringstream ss;
  const auto& program_model = c.get_current_program_model();
  for (const auto& t : program_model.get_trace()) {
    ss << "thread " << t->get_executor() << ": " << t->to_string() << "\n";
  }
  ss << "\nNEXT THREAD OPERATIONS\n";
  for (const auto& tpair : program_model.get_pending_transitions()) {
    ss << "thread " << tpair.first << ": " << tpair.second->to_string() << "\n";
  }
  std::cout << ss.str();
  std::cout.flush();
}

void do_model_checking(const config& config) {
  algorithm::callbacks c;
  target target_program(config.target_executable,
                        config.target_executable_args);
  coordinator coordinator(program::starting_from_main(),
                          transition_registry::default_registry(),
                          make_unique<fork_process_source>(target_program));
  std::cerr << "\n\n**************** INTIAL STATE *********************\n\n";
  coordinator.get_current_program_model().dump_state(std::cerr);
  std::cerr << "\n\n**************** INTIAL STATE *********************\n\n";
  std::cerr.flush();

  model_checking::classic_dpor classic_dpor_checker;
  c.trace_completed = &finished_trace_classic_dpor;
  c.deadlock = &found_deadlock;
  c.undefined_behavior = &found_undefined_behavior;
  classic_dpor_checker.verify_using(coordinator, c);
  std::cout << "Model checking completed!" << std::endl;
}

void do_model_checking_from_dmtcp_ckpt_file(const config& config) {
  volatile mcmini_shm_file* rw_region =
      xpc_resources::get_instance().get_rw_region()->as<mcmini_shm_file>();

  std::unique_ptr<process_source> dmtcp_template_handle;

  if (config.use_multithreaded_fork) {
    dmtcp_template_handle =
        extensions::make_unique<multithreaded_fork_process_source>(
            config.checkpoint_file);
  } else {
    dmtcp_template_handle =
        extensions::make_unique<dmtcp_process_source>(config.checkpoint_file);
  }

  // Make sure that `dmtcp_restart` has executed and that the template
  // process is ready for execution; otherwise, the state restoration will not
  // work as expected.
  algorithm::callbacks c;
  transition_registry tr = transition_registry::default_registry();
  coordinator coordinator(program(), tr, std::move(dmtcp_template_handle));
  {
    model_to_system_map recorder(coordinator);

    fifo fifo("/tmp/mcmini-fifo");
    ::visible_object current_obj;
    std::vector<::visible_object> recorded_threads;
    std::unordered_map<
        void*, std::vector<std::pair<runner_id_t, condition_variable_status>>>
        cv_waiting_threads;
    while (fifo.read(&current_obj) && current_obj.type != UNKNOWN) {
      if (current_obj.type == THREAD) {
        recorded_threads.emplace_back(std::move(current_obj));
      } else if (current_obj.type == CV_WAITERS_QUEUE) {
        // Capture both thread ID and state
        cv_waiting_threads[current_obj.waiting_queue_state.cv_location]
            .push_back(
                std::make_pair(current_obj.waiting_queue_state.waiting_id,
                               current_obj.waiting_queue_state.cv_state));
      } else {
        recorder.observe_object(current_obj.location,
                                translate_recorded_object_to_model(
                                    current_obj, cv_waiting_threads));
      }
    }

    std::sort(recorded_threads.begin(), recorded_threads.end(),
              [](const ::visible_object& lhs, const ::visible_object& rhs) {
                return lhs.thrd_state.id < rhs.thrd_state.id;
              });

    for (const ::visible_object& recorded_thread : recorded_threads) {
      recorder.observe_runner(
          (void*)recorded_thread.thrd_state.pthread_desc,
          translate_recorded_runner_to_model(recorded_thread));
    }

    for (const ::visible_object& recorded_thread : recorded_threads) {
      // Translates from what each user space thread recorded as its next
      // transition. This happens _after_ DMTCP has restarted the checkpoint
      // image but _before_ the template thread told the McMini process (i.e.
      // this one) about the recorded objects. In other words, each user space
      // thread has "recorded" (in the sense of "marked" and not in the sense
      // of "during the RECORD phase of `libmcmini.so`") the next transition
      // it would have run had McMini not just now intervened.
      runner_id_t recorded_id = recorded_thread.thrd_state.id;
      volatile runner_mailbox* mb = &rw_region->mailboxes[recorded_id];
      transition_registry::transition_discovery_callback callback =
          tr.get_callback_for(mb->type);
      if (!callback) {
        throw std::runtime_error("Expected a callback for " +
                                 std::to_string(mb->type));
      }
      const size_t num_objects_before =
          coordinator.get_current_program_model().get_state_sequence().count();
      recorder.observe_runner_transition(callback(recorded_id, *mb, recorder));
      const size_t num_objects_after =
          coordinator.get_current_program_model().get_state_sequence().count();

      // Ensures that no objects were added during `callback`.
      //
      // Callbacks insert objects into the model when they notice
      // that the model is missing an association for a particular
      // real world address. Since we already translated all recorded
      // objects from `libmcmini.so`, if an object is ever added
      // when determining the pending operation of `recorded_thread`,
      // this means that the object was not added to the model and
      // hence was not sent to the McMini proces by `libmcmini.so`.
      //
      // In short, this ensures that recording worked as expected.
      assert(num_objects_before == num_objects_after);
    }
  }

  std::cerr << "\n\n**************** INTIAL STATE *********************\n\n";
  coordinator.get_current_program_model().dump_state(std::cerr);
  std::cerr << "\n\n**************** INTIAL STATE *********************\n\n";
  std::cerr.flush();

  model_checking::classic_dpor classic_dpor_checker;
  c.trace_completed = &finished_trace_classic_dpor;
  c.undefined_behavior = &found_undefined_behavior;
  c.deadlock = &found_deadlock;
  classic_dpor_checker.verify_using(coordinator, c);
  std::cerr << "Deep debugging completed!" << std::endl;
}

void do_recording(const config& config) {
  char dir[PATH_MAX];
  // FIXME:  This depends on mcmini starting in root dir of git repo.
  std::string libmcini_dir = getcwd(dir, sizeof(dir)) ? dir : "PATH_TOO_LONG";
  std::string libmcmini_path = libmcini_dir + "/libmcmini.so";
  std::vector<std::string> dmtcp_launch_args;
  dmtcp_launch_args.push_back("--disable-alloc-plugin");
  dmtcp_launch_args.push_back("-i");
  dmtcp_launch_args.push_back(std::to_string(config.checkpoint_period.count()));
  dmtcp_launch_args.push_back("--with-plugin");
  dmtcp_launch_args.push_back(libmcmini_path);
  dmtcp_launch_args.push_back("--modify-env");
  dmtcp_launch_args.push_back(config.target_executable);
  for (const std::string& target_arg : config.target_executable_args)
    dmtcp_launch_args.push_back(target_arg);
  real_world::target target_program("dmtcp_launch", dmtcp_launch_args);

  std::cout << "Recording: " << target_program << std::endl;
  target_program.execvp();
}

std::string find_first_ckpt_file_in_cwd() {
  try {
    // Open the current directory
    DIR* dir = opendir(".");
    if (dir == nullptr) {
      perror("opendir");
      return "";
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
      // Check if the entry is a regular file and has the .foo extension
      if (entry->d_type == DT_REG) {  // DT_REG indicates a regular file
        std::string filename(entry->d_name);
        std::cout << filename << std::endl;
        if (filename.size() >= 6 &&
            filename.substr(filename.size() - 6) == ".dmtcp") {
          std::cout << "Found file: " << filename << std::endl;
          closedir(dir);
          return filename;
        }
      }
    }
    std::cout
        << "No file with the `.dmtcp` extension found in the current directory."
        << std::endl;
    closedir(dir);
    return "";
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return "";
  }
}

int main_cpp(int argc, const char** argv) {
  model::config mcmini_config;

  const char** cur_arg = &argv[1];
  if (argc == 1) {
    cur_arg[0] = "--help";
    cur_arg[1] = NULL;
  }

  // TODO: Use argp.h instead (more options, better descriptions, etc)
  while (cur_arg[0] != NULL && cur_arg[0][0] == '-') {
    if (strcmp(cur_arg[0], "--max-depth-per-thread") == 0 ||
        strcmp(cur_arg[0], "-m") == 0) {
      mcmini_config.max_thread_execution_depth =
          strtoul(cur_arg[1], nullptr, 10);

      char* endptr;
      if (strtol(cur_arg[1], &endptr, 10) == 0 || endptr[0] != '\0') {
        fprintf(stderr, "%s: illegal value\n", "--max-depth-per-thread");
        exit(1);
      }
      cur_arg += 2;
    } else if (strcmp(cur_arg[0], "--interval") == 0 ||
               strcmp(cur_arg[0], "-i") == 0) {
      mcmini_config.record_target_executable_only = true;
      mcmini_config.checkpoint_period =
          std::chrono::seconds(strtoul(cur_arg[1], nullptr, 10));
      cur_arg += 2;
    } else if (strcmp(cur_arg[0], "--from-checkpoint") == 0 ||
               strcmp(cur_arg[0], "-ckpt") == 0) {
      mcmini_config.checkpoint_file = cur_arg[1];
      cur_arg += 2;
    } else if (strcmp(cur_arg[0], "--from-first-checkpoint") == 0) {
      mcmini_config.checkpoint_file = find_first_ckpt_file_in_cwd();
      cur_arg += 1;
    } else if (strcmp(cur_arg[0], "--multithreaded-fork") == 0 ||
               strcmp(cur_arg[0], "-mtf") == 0) {
      mcmini_config.use_multithreaded_fork = true;
      cur_arg++;
    } else if (cur_arg[0][1] == 'm' && isdigit(cur_arg[0][2])) {
      mcmini_config.max_thread_execution_depth =
          strtoul(cur_arg[1], nullptr, 10);
      cur_arg++;
    } else if (strcmp(cur_arg[0], "--first-deadlock") == 0 ||
               strcmp(cur_arg[0], "--first") == 0 ||
               strcmp(cur_arg[0], "-f") == 0) {
      mcmini_config.stop_at_first_deadlock = true;
      cur_arg++;
    } else if (strcmp(cur_arg[0], "--print-at-traceId") == 0 ||
               strcmp(cur_arg[0], "-p") == 0) {
      mcmini_config.target_trace_id = strtoul(cur_arg[1], nullptr, 10);
      char* endptr;
      if (strtol(cur_arg[1], &endptr, 10) == 0 || endptr[0] != '\0') {
        fprintf(stderr, "%s: illegal value\n", "--print-at-traceId");
        exit(1);
      }
      cur_arg += 2;
    } else if (cur_arg[0][1] == 'p' && isdigit(cur_arg[0][2])) {
      mcmini_config.target_trace_id = strtoul(cur_arg[2], nullptr, 10);
      cur_arg++;
    } else if (strcmp(cur_arg[0], "--help") == 0 ||
               strcmp(cur_arg[0], "-h") == 0) {
      fprintf(
          stderr,
          "Usage: mcmini (experimental)\n"
          "              [--record|-r <seconds>] \n"
          "              [--from-checkpoint <ckpt>] [--multithreaded-fork] \n"
          "              [--max-depth-per-thread|-m <num>]\n"
          "              [--first-deadlock|--first|-f]\n"
          "              [--help|-h]\n"
          "              target_executable\n");
      exit(1);
    } else {
      printf("mcmini: unrecognized option: %s\n", cur_arg[0]);
      exit(1);
    }
  }

  if (mcmini_config.checkpoint_file == "") {
    struct stat stat_buf;
    if (cur_arg[0] == NULL || stat(cur_arg[0], &stat_buf) == -1) {
      fprintf(stderr, "*** Missing target_executable or no such file.\n\n");
      exit(1);
    }

    assert(cur_arg[0][strlen(cur_arg[0])] == '\0');
    char idx = strlen(cur_arg[0]) - strlen("mcmini") - 1 >= 0
                   ? strlen(cur_arg[0]) - strlen("mcmini") - 1
                   : strlen(cur_arg[0]);
    // idx points to 'X' when cur_arg[0] == "...Xmcmini"
    if (strcmp(cur_arg[0], "mcmini") == 0 ||
        strcmp(cur_arg[0] + idx, "/mcmini") == 0) {
      fprintf(stderr,
              "\n*** McMini being called on 'mcmini'.  This doesn't work.\n");
      exit(1);
    }
    mcmini_config.target_executable = std::string(cur_arg[0]);
    // Collect all remaining arguments as target executable arguments
    cur_arg++;
    while (cur_arg[0] != NULL) {
      mcmini_config.target_executable_args.push_back(std::string(cur_arg[0]));
      cur_arg++;
    }
  }

  install_process_wide_signal_handlers();
  if (mcmini_config.record_target_executable_only) {
    do_recording(mcmini_config);
  } else if (mcmini_config.checkpoint_file != "") {
    do_model_checking_from_dmtcp_ckpt_file(mcmini_config);
  } else {
    do_model_checking(mcmini_config);
  }

  return EXIT_SUCCESS;
}

int main(int argc, const char** argv) {
  return main_cpp(argc, argv);
  // try {
  //   return main_cpp(argc, argv);
  // } catch (const std::exception& e) {
  //   std::cerr << "ERROR: " << e.what() << std::endl;
  //   return EXIT_FAILURE;
  // } catch (...) {
  //   std::cerr << "ERROR: Unknown error occurred" << std::endl;
  //   return EXIT_FAILURE;
  // }
}
