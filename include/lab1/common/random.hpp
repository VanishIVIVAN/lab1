#pragma once

#include <cctype>
#include <cstddef>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

namespace lab1 {

/// Seedable source of random numbers shared by all generators.
class RandomSource {
 public:
  explicit RandomSource(unsigned seed = std::random_device{}()) : engine_(seed) {}

  /// Uniform integer in [low, high].
  int intInRange(int low, int high) {
    std::uniform_int_distribution<int> distribution(low, high);
    return distribution(engine_);
  }

  /// Uniform index in [0, size). `size` must be positive.
  std::size_t index(std::size_t size) {
    return static_cast<std::size_t>(intInRange(0, static_cast<int>(size) - 1));
  }

  /// Random lowercase word of length in [minLength, maxLength].
  std::string word(int minLength, int maxLength) {
    std::string result;
    const int length = intInRange(minLength, maxLength);
    for (int i = 0; i < length; ++i) result += static_cast<char>('a' + intInRange(0, 25));
    return result;
  }

  /// Random word starting with an uppercase letter.
  std::string capitalizedWord(int minLength, int maxLength) {
    std::string result = word(minLength, maxLength);
    result[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[0])));
    return result;
  }

 private:
  std::mt19937 engine_;
};

/**
 * @brief Static polymorphism: compile-time selected random value generator.
 *
 * Own classes add explicit specializations (see ElementHandle).
 */
template <typename T, typename Enable = void>
struct RandomValue;

template <typename T>
struct RandomValue<T, std::enable_if_t<std::is_integral_v<T>>> {
  static T generate(RandomSource& rng) { return static_cast<T>(rng.intInRange(-99, 99)); }
};

template <typename T>
struct RandomValue<T, std::enable_if_t<std::is_floating_point_v<T>>> {
  static T generate(RandomSource& rng) {
    return static_cast<T>(rng.intInRange(-9999, 9999) / 100.0);
  }
};

template <>
struct RandomValue<std::string, void> {
  static std::string generate(RandomSource& rng) { return rng.word(3, 7); }
};

template <typename T>
struct RandomValue<std::vector<T>, void> {
  static std::vector<T> generate(RandomSource& rng) {
    std::vector<T> values;
    const int count = rng.intInRange(1, 3);
    for (int i = 0; i < count; ++i) values.push_back(RandomValue<T>::generate(rng));
    return values;
  }
};

}  // namespace lab1
