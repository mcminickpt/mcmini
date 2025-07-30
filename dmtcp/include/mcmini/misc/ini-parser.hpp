#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace mcmini {

using SectionMap =
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>>;

class IniParser : public SectionMap {
 public:
  IniParser()
      : fieldSep_('='), escapeChar_('\\'), commentPrefixes_({"#", ";"}) {}
  IniParser(const std::string &filename) : IniParser() {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Cannot open file: " + filename);
    safe_decode(in);
  }
  IniParser(std::istream &is) : IniParser() { decode(is); }

 private:
  char fieldSep_;
  char escapeChar_;
  std::vector<std::string> commentPrefixes_;

 private:
  inline void trim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    [](int ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         [](int ch) { return !std::isspace(ch); })
                .base(),
            s.end());
  }

  inline void removeComments(std::string &line) {
    for (const auto &prefix : commentPrefixes_) {
      size_t pos = 0;
      while (true) {
        pos = line.find(prefix, pos);
        if (pos == std::string::npos) break;
        if (pos == 0 || line[pos - 1] != escapeChar_) {
          line.erase(pos);
          break;
        } else {
          line.erase(pos - 1, 1);
          pos += prefix.size();
        }
      }
    }
  }

  void safe_decode(std::istream &is) {
    try {
      decode(is);
    } catch (const std::runtime_error &e) {
      this->clear();
      throw e;
    }
  }

  void decode(std::istream &is) {
    this->clear();
    SectionMap keyLineTracker;
    std::unordered_map<std::string, std::unordered_map<std::string, int>>
        lineTracker;
    std::string line, currentSection;
    std::string lastNonRelativeSection;
    int lineNo = 0;
    while (std::getline(is, line)) {
      ++lineNo;
      removeComments(line);
      trim(line);
      if (line.empty()) continue;
      if (line.front() == '[') {
        size_t endPos = line.find(']');
        if (endPos == std::string::npos)
          throw std::runtime_error("Line " + std::to_string(lineNo) +
                                   ": section not closed");
        std::string secName = line.substr(1, endPos - 1);
        trim(secName);
        if (secName.empty())
          throw std::runtime_error("Line " + std::to_string(lineNo) +
                                   ": empty section name");
        if (secName.front() == '.') {
          if (!lastNonRelativeSection.empty())
            secName = lastNonRelativeSection + secName;
          else
            secName = secName.substr(1);
        } else {
          lastNonRelativeSection = secName;
        }
        currentSection = secName;
        if (this->find(currentSection) == this->end()) {
          (*this)[currentSection] =
              std::unordered_map<std::string, std::string>();
          lineTracker[currentSection] = std::unordered_map<std::string, int>();
        }
        continue;
      }
      size_t sepPos = line.find(fieldSep_);
      if (sepPos == std::string::npos)
        throw std::runtime_error("Line " + std::to_string(lineNo) +
                                 ": no separator '" +
                                 std::string(1, fieldSep_) + "' found");
      std::string key = line.substr(0, sepPos);
      std::string value = line.substr(sepPos + 1);
      trim(key);
      trim(value);
      if (key.empty())
        throw std::runtime_error("Line " + std::to_string(lineNo) +
                                 ": empty key");
      if (currentSection.empty()) currentSection = "global";
      if (this->find(currentSection) == this->end()) {
        (*this)[currentSection] =
            std::unordered_map<std::string, std::string>();
        lineTracker[currentSection] = std::unordered_map<std::string, int>();
      }
      if ((*this)[currentSection].count(key)) {
        int firstLine = lineTracker[currentSection][key];
        std::stringstream ss;
        ss << "The key `" << key << "` in section `" << currentSection
           << "` has already been assigned at line " << firstLine << " to `"
           << (*this)[currentSection][key] << "`. Overriding to `" << value
           << "` at line " << lineNo << "...\n";
        throw std::runtime_error(ss.str());
      } else {
        lineTracker[currentSection][key] = lineNo;
      }
      (*this)[currentSection][key] = value;
    }
  }
};

}  // namespace mcmini
