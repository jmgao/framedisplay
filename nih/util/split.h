#pragma once

#include <string.h>

#include <string>
#include <vector>
#include <gsl.h>

inline std::vector<gsl::cstring_span> Split(gsl::cstring_span str, const char* delimiters) {
  std::vector<gsl::cstring_span> result;
  str = str.subspan(0, strnlen(str.data(), str.size()));
  auto it = str.begin();

  while (it != str.end()) {
    char buf[2] = { *it, '\0'};
    if (strstr(delimiters, buf)) {
      gsl::cstring_span before(str.begin(), it);
      gsl::cstring_span after(it + 1, str.end());
      result.push_back(before);
      str = after;
    }
    ++it;
  }
  if (!str.empty()) {
    result.push_back(str);
  }
  return result;
}

template<typename T>
std::string Join(T strings, const char* delimiter) {
  if (strings.empty()) {
    return std::string();
  }

  std::string result;
  for (const auto& string : strings) {
    if (!string.empty()) {
      result.append(&string[0], string.length());
    }
    result.append(delimiter);
  }
  result.resize(result.size() - strlen(delimiter));
  return result;
}
