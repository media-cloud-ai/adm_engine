
#include "utils.hpp"

#include <regex>

namespace admengine {

const std::map<std::string, std::string> specCharMap = {
  { " ", "_" },
  { "é", "e" },
  { "è", "e" },
  { "ê", "e" },
  { "ë", "e" },
  { "à", "a" },
  { "â", "a" },
  { "ä", "a" },
  { "ù", "u" },
  { "û", "u" },
  { "ü", "u" }
  // TODO: complete this list...
};

const std::regex specCharRegex("[^a-zA-Z0-9_-]");

std::string replaceSpecialCharacters(std::string& text) {
  // map some special characters
  std::map<std::string, std::string>::const_iterator it;
  for (it = specCharMap.begin(); it != specCharMap.end(); it++) {
    text = std::regex_replace(text, std::regex(it->first), it->second);
  }

  // remove remaining special characters
  text = std::regex_replace(text, specCharRegex, "_");
  return text;
}

}
