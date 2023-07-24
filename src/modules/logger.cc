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

#include "logger.h"

#include "base/files/file_path.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "pangu/pg_platform.h"

#if BUILDFLAG(IS_WIN)
#include <Windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif  // BUILDFLAG(IS_WIN)

#if ASYNC_LOGGER
#include "spdlog/async.h"
#endif
#if BUILDFLAG(IS_ANDROID)
#include "spdlog/sinks/android_sink.h"
#endif  // BUILDFLAG(IS_ANDROID)
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"

namespace pangu {
namespace modules {
namespace logger {

// Declare the default logger.
pangu_logger pangu_logger::instance_;

pangu_logger::pangu_logger() {
#if ASYNC_LOGGER
  spdlog::init_thread_pool(8192, 1);
#endif
  reset();
}

pangu_logger::~pangu_logger() { clear(); }

void pangu_logger::set_path(const std::string &path) {
  instance_.path_ = path;
  instance_.reset();
}

void pangu_logger::set_max_size(int size) {
  instance_.size_ = size;
  instance_.reset();
}

void pangu_logger::set_level(pangu::log_level level) {
  instance_.level_ = level;
  spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
}

void pangu_logger::reset() {
  clear();

  // define logger name.
  static const std::string logger_name("pangu");

  std::vector<spdlog::sink_ptr> sinks;
  try {
#if BUILDFLAG(IS_ANDROID)
    sinks.push_back(std::make_shared<spdlog::sinks::android_sink_mt>());
#else
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
#endif  // BUILDFLAG(IS_ANDROID)

    std::string exception;

    const size_t max_files = 5;
#if BUILDFLAG(IS_WIN)
    base::FilePath file_path(base::UTF8ToWide(path_));
#else
    base::FilePath file_path(path_);
#endif

    const auto file_name = FILE_PATH_LITERAL("pangu.log");
    if (file_path.empty()) {
      // default set current log file path to kCurrentDirectory + file_name;
      file_path = base::FilePath(
          base::FilePath::StringType(base::FilePath::kCurrentDirectory) +
          base::FilePath::kSeparators + file_name);
    } else if (file_path.EndsWithSeparator()) {
      file_path = file_path.Append(file_name);
    }

    path_ =
#if BUILDFLAG(IS_WIN)
        base::WideToUTF8(file_path.value());
#else
        file_path.value();
#endif
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        path_, size_, max_files));

#if ASYNC_LOGGER
    logger_ = std::make_shared<spdlog::async_logger>(
        logger_name, sinks.begin(), sinks.end(), spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    spdlog::flush_every(std::chrono::seconds(1));

#else
    logger_ = std::make_shared<spdlog::logger>(logger_name, sinks.begin(),
                                               sinks.end());
#endif

    logger_->flush_on(spdlog::level::info);
    logger_->set_pattern("[%x %X.%e][%t][%^%l][%s:%#%$] %v");
    logger_->set_level(static_cast<spdlog::level::level_enum>(level_));

    spdlog::register_logger(logger_);

  } catch (std::exception &e) {
    SPDLOG_CRITICAL("reset logger with path {} exception {}", path_.c_str(),
                    e.what());
  }
}

void pangu_logger::clear() {
  if (logger_) {
    logger_->flush();
    spdlog::shutdown();
  }
}

}  // namespace logger
}  // namespace modules
}  // namespace pangu