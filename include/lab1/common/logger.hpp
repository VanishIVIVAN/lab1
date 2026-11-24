#pragma once

#include <iostream>
#include <string>

namespace lab1 {

/// Severity levels of the simple logger.
enum class LogLevel { Debug = 0, Info, Warning, Error, Off };

/**
 * @brief Minimal process-wide logger (disabled by default, enabled with --verbose).
 */
class Logger {
 public:
  static Logger& instance() {
    static Logger logger;
    return logger;
  }

  void setLevel(LogLevel level) { level_ = level; }
  void setSink(std::ostream* sink) { sink_ = sink; }

  /// Writes the message if its level is not below the configured one.
  void log(LogLevel level, const std::string& message) const {
    if (level < level_ || sink_ == nullptr) return;
    (*sink_) << '[' << levelName(level) << "] " << message << '\n';
  }

  void debug(const std::string& message) const { log(LogLevel::Debug, message); }
  void warning(const std::string& message) const { log(LogLevel::Warning, message); }

 private:
  Logger() = default;

  static const char* levelName(LogLevel level) {
    switch (level) {
      case LogLevel::Debug: return "debug";
      case LogLevel::Info: return "info";
      case LogLevel::Warning: return "warn";
      case LogLevel::Error: return "error";
      default: return "off";
    }
  }

  LogLevel level_ = LogLevel::Off;
  std::ostream* sink_ = &std::clog;
};

}  // namespace lab1
