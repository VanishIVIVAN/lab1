#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace lab1 {

/**
 * @brief Static polymorphism: compile-time selected text representation of a value.
 *
 * The primary template expects a `toString()` member (own classes);
 * specializations cover arithmetic types, std::string and std::vector.
 */
template <typename T, typename Enable = void>
struct TextTraits {
  static std::string toText(const T& value) { return value.toString(); }
};

template <typename T>
struct TextTraits<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
  static std::string toText(const T& value) {
    std::ostringstream out;
    out << value;
    return out.str();
  }
};

template <>
struct TextTraits<std::string, void> {
  static std::string toText(const std::string& value) { return '"' + value + '"'; }
};

template <typename T>
struct TextTraits<std::vector<T>, void> {
  static std::string toText(const std::vector<T>& values) {
    std::string result = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
      if (i > 0) result += ", ";
      result += TextTraits<T>::toText(values[i]);
    }
    return result + "]";
  }
};

/// Joins text representations of values with a separator.
template <typename T>
std::string joinValues(const std::vector<T>& values, const std::string& separator) {
  std::string result;
  for (std::size_t i = 0; i < values.size(); ++i) {
    if (i > 0) result += separator;
    result += TextTraits<T>::toText(values[i]);
  }
  return result;
}

}  // namespace lab1
