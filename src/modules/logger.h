// Copyright (C) 2023  Sylar & PanGu contributors.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <string>

#include "pangu/pg_base.h"
#include "spdlog/spdlog.h"

namespace pangu {
namespace modules {
namespace logger {

// Use the async logger or not.
#define ASYNC_LOGGER 0

// This class is used to initialize the logger.
class pangu_logger {
 public:
  // Set the path of log file.
  static void set_path(const std::string &path);

  // Set the max size of log file.
  static void set_max_size(int size);

  // Set the log level.
  static void set_level(pangu::log_level level);

  // Get current log level.
  static pangu::log_level get_level() { return instance_.level_; }

  // Log the message.
  template <typename... Args>
  static void log(pangu::log_level level, Args &&...args) {
    instance_.logger_->log(
        spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION},
        static_cast<spdlog::level::level_enum>(level),
        std::forward<Args>(args)...);
  }

 private:
  pangu_logger();
  ~pangu_logger();

  // Reset the logger.
  void reset();

  // Flush and shutdown.
  void clear();

 private:
  static pangu_logger instance_;  // static instance

  std::shared_ptr<spdlog::logger> logger_;           // logger
  std::string path_;                                 // default path is empty
  pangu::log_level level_ = pangu::log_level::info;  // default level info
  int size_ = 1024 * 1024 * 2;                       // 2MB
};

#define LOG_OUTPUT(LEVEL, ...) \
  pangu::modules::logger::pangu_logger::log(LEVEL, __VA_ARGS__)

#define LOG_TRACE(...) LOG_OUTPUT(pangu::log_level::trace, __VA_ARGS__)
#define LOG_DEBUG(...) LOG_OUTPUT(pangu::log_level::debug, __VA_ARGS__)
#define LOG_INFO(...) LOG_OUTPUT(pangu::log_level::info, __VA_ARGS__)
#define LOG_WARN(...) LOG_OUTPUT(pangu::log_level::warn, __VA_ARGS__)
#define LOG_ERROR(...) LOG_OUTPUT(pangu::log_level::error, __VA_ARGS__)
#define LOG_FATAL(...) LOG_OUTPUT(pangu::log_level::critical, __VA_ARGS__)

}  // namespace logger
}  // namespace modules
}  // namespace pangu