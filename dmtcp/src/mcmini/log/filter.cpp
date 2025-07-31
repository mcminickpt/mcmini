#include "mcmini/log/filter.hpp"

#include "mcmini/log/severity_level_parser.hpp"
#include "mcmini/misc/ini-parser.hpp"

using namespace logging;

bool whitelist_filter::apply(const std::string &subsystem,
                             severity_level log_record_severity) const {
  if (registered_subsystems.count(subsystem) != 0) {
    return log_record_severity <= registered_subsystems.at(subsystem);
  }
  severity_level most_permissive = severity_level::nothing;
  for (const auto &pair : registered_regexes) {
    if (std::regex_match(subsystem, pair.first)) {
      most_permissive = std::max(most_permissive, pair.second);
    }
  }
  return log_record_severity <= most_permissive;
}

bool blacklist_filter::apply(const std::string &subsystem,
                             severity_level log_record_severity) const {
  if (registered_subsystems.count(subsystem) != 0) {
    return log_record_severity > registered_subsystems.at(subsystem);
  }
  severity_level most_restrictive = severity_level::nothing;
  for (const auto &pair : registered_regexes) {
    if (std::regex_match(subsystem, pair.first)) {
      most_restrictive = std::max(most_restrictive, pair.second);
    }
  }
  return log_record_severity > most_restrictive;
}

composite_filter::composite_filter(const std::string &ini_file_path) {
  // IniParser ini_contents(ini_file_path);

  // for (const auto &section : ini_contents) {
  //   std::string lowerSection = section.first;
  //   std::transform(lowerSection.begin(), lowerSection.end(),
  //                  lowerSection.begin(), ::tolower);

  //   if (lowerSection == "blacklist") {
  //     for (const auto &item : section.second)
  //       mapping[item.first] = parse_severity(item.second);
  //   }
  //   else if (lowerSection == "whitelist") {

  //   }
  // }
  // return mapping;
}
