#pragma once

#include <optional>
#include <regex>
#include <unordered_map>

#include "mcmini/log/severity_level.hpp"

namespace mcmini {
namespace log {
class filter {
 public:
  filter() = default;
  filter(std::unordered_map<std::string, severity_level> mapping)
      : registered_subsystems(std::move(mapping)) {}

 public:
  virtual bool apply(const std::string &subsystem, severity_level) const = 0;

 protected:
  std::unordered_map<std::string, severity_level> registered_subsystems;
  std::vector<std::pair<std::regex, severity_level>> registered_regexes;
};

class whitelist_filter : public filter {
 public:
  whitelist_filter() = default;
  whitelist_filter(whitelist_filter &&) = default;
  whitelist_filter(const whitelist_filter &) = default;
  whitelist_filter &operator=(whitelist_filter &&) = default;
  whitelist_filter &operator=(const whitelist_filter &) = default;
  whitelist_filter(std::unordered_map<std::string, severity_level> mapping)
      : filter(std::move(mapping)) {}
  bool apply(const std::string &, severity_level) const override;
};

class blacklist_filter : public filter {
 public:
  blacklist_filter() = default;
  blacklist_filter(blacklist_filter &&) = default;
  blacklist_filter(const blacklist_filter &) = default;
  blacklist_filter &operator=(blacklist_filter &&) = default;
  blacklist_filter &operator=(const blacklist_filter &) = default;
  blacklist_filter(std::unordered_map<std::string, severity_level> mapping)
      : filter(std::move(mapping)) {}
  bool apply(const std::string &, severity_level) const override;
};

class composite_filter : public filter {
 public:
  composite_filter(const std::string &ini_file);
  composite_filter(const std::vector<whitelist_filter> &whitelists,
                   const std::vector<blacklist_filter> &blacklists)
      : whitelists(whitelists), blacklists(blacklists) {}
  bool apply(const std::string &subsystem, severity_level sl) const override {
    for (const whitelist_filter &wl : whitelists)
      if (!wl.apply(subsystem, sl)) return false;
    for (const blacklist_filter &bl : blacklists)
      if (!bl.apply(subsystem, sl)) return false;
    return true;
  }

 private:
  std::vector<whitelist_filter> whitelists;
  std::vector<blacklist_filter> blacklists;
};

class severity_filter : public filter {
 public:
  severity_filter(severity_level sev) : minimum_level(sev) {}
  bool apply(const std::string &, severity_level severity) const override {
    return severity >= minimum_level;
  }

 private:
  severity_level minimum_level;
};

class permissive_filter : public filter {
 public:
  bool apply(const std::string &, severity_level) const override {
    return true;
  }
};

}  // namespace log
}  // namespace mcmini
