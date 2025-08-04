#pragma once

#include <cstdarg>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "mcmini/constants.hpp"
#include "mcmini/log/filter.hpp"
#include "mcmini/log/severity_level.hpp"
#include "mcmini/misc/rwlock.hpp"

namespace logging {
class log_control {
 public:
  using subsystem_list = std::unordered_map<std::string, severity_level>;

  /// @brief Permit all logs from all subsystems
  ///
  /// This is the most permissive log. All log messages will succeed at
  /// the _global_ level. Sink-specific filtering may still apply if the
  /// sinks have been configured that way.
  void allow_everything();

  /// @brief Permit logs only from those subsystems whose severity is at
  /// least as important as that in the mapping
  ///
  /// Whitelist logging permits the logs from only those subsystems on
  /// the list to successfully be processed by the sink frontends.
  /// Filtering is interpreted as follows:
  ///
  /// Given a log from subsystem "A" with severity "B", if the whitelist
  /// does not contain subsystem "A", the log is dropped; otherwise, if
  /// the severity level "B" is less important than the severity
  /// permitted for the subsytem, the log is again dropped. If both
  /// checks pass, the log is forwarded to the sinks registered with the
  /// controller.
  void whitelist(const subsystem_list &mapping) {
    whitelist(whitelist_filter(std::move(mapping)));
  }

  /// @brief Permit logs only from those subsystems which pass the given filter.
  ///
  /// Whitelist logging permits the logs from only those subsystems on
  /// the list to successfully be processed by the sink frontends.
  /// Filtering is interpreted as follows:
  ///
  /// Given a log from subsystem "A" with severity "B", if the whitelist
  /// does not contain subsystem "A", the log is dropped; otherwise, if
  /// the severity level "B" is less important than the severity
  /// permitted for the subsytem, the log is tested against any regex patterns
  /// matched with the filter. The most permissive severity level is chosen and
  /// compared with the severity of the log record. If thses checks pass, the
  /// log is forwarded to the sinks registered with the controller.
  void whitelist(whitelist_filter wl);

  /// @brief Permit all logs except for those from subsystems with a
  /// severity less important than that permitted by the subsystem.
  ///
  /// Blacklist logging prevents logs from those subsystems on the list.
  /// Any log that is not on the blacklist will be successfully
  /// processed by the sink frontends. Filtering is interpreted as
  /// follows:
  ///
  /// Given a log from subsystem "A" with severity "B", if the blacklist
  /// does not contain subsystem "A", the log is processed; otherwise,
  /// if the severity level "B" is at least as important as the severity
  /// permitted for the subsytem, the log is again processed. If both
  /// checks fail, the log is dropped.
  void blacklist(const subsystem_list &mapping) {
    blacklist(blacklist_filter(std::move(mapping)));
  }

  /// @brief Permit all logs except for those from subsystems with a
  /// severity less important than that permitted by the subsystem.
  ///
  /// Blacklist logging prevents logs from those subsystems on the list.
  /// Any log that is not on the blacklist will be successfully
  /// processed by the sink frontends. Filtering is interpreted as
  /// follows:
  ///
  /// Given a log from subsystem "A" with severity "B", if the blacklist
  /// does not contain subsystem "A", the log is processed; otherwise,
  /// if the severity level "B" is at least as important as the severity
  /// permitted for the subsytem, the log is again processed. If both
  /// checks fail, the log is dropped.
  void blacklist(blacklist_filter bl);

  /// @brief Permit logs from all subsystems whose severity level is at
  /// least as great as the level provided.
  ///
  /// This is a convience method that whitelists all subsystems whose
  /// logging severity level is at least as high as the provided one.
  /// This is more efficient than simply listing all subsytems in a
  /// whitelist as only the severity level need be checked.
  void allow_everything_over(severity_level);

 public:
  void set_filter(filter *filt);

 public:
  inline static log_control &instance() {
    static log_control lc;
    return lc;
  }
  void log_raw(const std::string &instance, const std::string &subsystem,
               const std::string &message, const severity_level severity,
               const char *file = __FILE__, int line = __LINE__);

 private:
  RWLock filter_lock;
  std::unique_ptr<filter> active_filter;
};
}  // namespace logging
