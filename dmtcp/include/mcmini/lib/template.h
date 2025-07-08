#pragma once

void mc_exit_main_thread_in_child(void);
void mc_prepare_new_child_process(pid_t ppid_before_fork, pid_t model_checker_pid);
void mc_template_process_loop_forever(pid_t (*)(void));
