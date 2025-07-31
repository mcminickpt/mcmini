#include "mcmini/log/logger.hpp"

#include <vector>

#include "mcmini/log/log_control.hpp"
#include "mcmini/log/severity_level.hpp"

using namespace logging;

static const char *log_level_strs[] = {
    "NOTHING",    "VERY_VERBOSE", "VERBOSE",  "DEBUG", "INFO",
    "UNEXPECTED", "ERROR",        "CRITICAL", "ABORT", "EVERYTHING"};

void log_control::allow_everything() {
  this->set_filter(new permissive_filter());
}

void log_control::allow_everything_over(severity_level level) {
  this->set_filter(new severity_filter(level));
}

void log_control::blacklist(blacklist_filter bl) {
  this->set_filter(new blacklist_filter(std::move(bl)));
}

void log_control::set_filter(filter *filt) {
  RWLock::WriteGuard guard(this->filter_lock);
  this->active_filter.reset(filt);
}

void log_control::log_raw(const std::string &instance,
                          const std::string &subsystem,
                          const std::string &message,
                          const severity_level severity, const char *file,
                          int line) {
  if (active_filter) {
    RWLock::ReadGuard guard(this->filter_lock);
    if (!active_filter->apply(subsystem, severity)) {
      return;
    }
  }

  tm tm;
  time_t t = std::time(nullptr);
  localtime_r(&t, &tm);
  char buf[100];
  buf[std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm)] = '\0';
  std::string instance_str = instance != "" ? (" (" + instance + ")") : "";

  std::stringstream ss;
  ss << "[" << constants::getpid() << "] " << subsystem << instance_str << " "
     << buf << " " << std::left << std::setw(5)
     << log_level_strs[static_cast<uint32_t>(severity)] << " " << file << ":"
     << line << ": ";
  const std::string prefix = ss.str();

  std::vector<std::string> lines;
  std::istringstream ss_msg(message);
  for (std::string line; std::getline(ss_msg, line);) {
    lines.push_back(line);
  }

  if (lines.size() > 1) {
    uint32_t lineno = 0;
    for (const auto &line : lines)
      std::clog << prefix << "(lineno: " << lineno++ << ") " << line << "\n";
  } else if (!lines.empty()) {
    std::clog << prefix << lines[0] << "\n";
  }
}
