#pragma once

#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

#include "mcmini/log/log_control.hpp"
#include "mcmini/log/severity_level.hpp"

#define log_severity(logger, severity) \
  logger.make_stream(__FILE__, __LINE__) << severity
#define log_very_verbose(logger) \
  log_severity(logger, mcmini::log::severity_level::very_verbose)
#define log_verbose(logger) \
  log_severity(logger, mcmini::log::severity_level::verbose)
#define log_debug(logger) \
  log_severity(logger, mcmini::log::severity_level::debug)
#define log_info(logger) log_severity(logger, mcmini::log::severity_level::info)
#define log_unexpected(logger) \
  log_severity(logger, mcmini::log::severity_level::unexpected)
#define log_error(logger) \
  log_severity(logger, mcmini::log::severity_level::error)
#define log_critical(logger) \
  log_severity(logger, mcmini::log::severity_level::critical)
#define log_abort(logger) \
  log_severity(logger, mcmini::log::severity_level::abort)

namespace mcmini {
namespace log {
class logger {
 public:
  logger() = default;
  logger(const std::string &subsystem) : subsystem(subsystem) {}

 public:
  template <typename T>
  void set_instance(T *instance) {
    std::stringstream strm;
    strm << "0x" << std::hex << reinterpret_cast<uint64_t>(instance);
    this->instance = strm.str();
  }

 public:
  struct stream {
   public:
    ~stream() { flush(); }
    template <typename T>
    stream &operator<<(const T &value) {
      ostream << value;
      return *this;
    }

    stream &operator<<(severity_level severity) {
      if (severity != current_severity) {
        flush();
        this->current_severity = severity;
      }
      // Ignore otherwise
      return *this;
    }

   private:
    stream &operator=(stream &&) = default;
    stream(stream &&) = default;
    explicit stream(logger *log, const char *file = __FILE__,
                    int line = __LINE__)
        : log(log), file(file), line(line) {}
    void flush() {
      if (ostream.str() != "") {
        this->log->log_raw(ostream.str(), current_severity, file, line);
        this->ostream = std::stringstream();
      };
    }

   private:
    logger *log;
    const char *file;
    int line;

    severity_level current_severity = severity_level::info;
    std::stringstream ostream;

   private:
    friend class logger;
  };

 public:
  template <typename T>
  stream operator<<(const T &item) {
    return make_stream(__FILE__, __LINE__);
  }

  stream make_stream(const char *file, int line) {
    return mcmini::log::logger::stream(this, file, line);
  }

 public:
  inline void log_raw(const std::string &message, severity_level severity,
                      const char *file = __FILE__, int line = __LINE__) {
    log_control::instance().log_raw(message, subsystem, severity, file, line);
  }

 private:
  std::string instance;
  std::string subsystem;

 private:
  friend struct stream;
};
}  // namespace log
}  // namespace mcmini
