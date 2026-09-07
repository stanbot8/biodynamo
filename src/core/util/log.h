// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_UTIL_LOG_H_
#define CORE_UTIL_LOG_H_

#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>

#include "core/util/string.h"

namespace bdm {

class Log {
 public:
  enum class Level { kDebug, kInfo, kWarning, kError };

  static void SetLevel(Level level) { level_.store(level); }
  static Level GetLevel() { return level_.load(); }

  template <typename... Args>
  static void Debug(const std::string& location, const Args&... parts) {
    Write(Level::kDebug, "Debug", location, Concat(parts...));
  }

  template <typename... Args>
  static void Info(const std::string& location, const Args&... parts) {
    Write(Level::kInfo, "Info", location, Concat(parts...));
  }

  template <typename... Args>
  static void Warning(const std::string& location, const Args&... parts) {
    Write(Level::kWarning, "Warning", location, Concat(parts...));
  }

  template <typename... Args>
  static void Error(const std::string& location, const Args&... parts) {
    Write(Level::kError, "Error", location, Concat(parts...));
  }

  template <typename... Args>
  static void Break(const std::string& location, const Args&... parts) {
    Write(Level::kError, "Break", location, Concat(parts...));
  }

  template <typename... Args>
  static void SysError(const std::string& location, const Args&... parts) {
    Write(Level::kError, "SysError", location,
          Concat(parts..., ": ", std::strerror(errno)));
  }

  template <typename... Args>
  [[noreturn]] static void Fatal(const std::string& location,
                                 const Args&... parts) {
    std::string message = Concat(parts...);
    std::fprintf(stderr, "Error in <%s>: %s\n", location.c_str(),
                 message.c_str());
    std::exit(1);
  }

  template <typename... Args>
  static void Condition(const std::function<bool()>& lambda,
                        const Args&... parts) {
    if (lambda()) {
      std::string message = Concat(parts...);
      std::fprintf(stdout, "%s\n", message.c_str());
    }
  }

 private:
  static void Write(Level level, const char* name, const std::string& location,
                    const std::string& message) {
    if (level >= level_.load()) {
      std::fprintf(stderr, "%s in <%s>: %s\n", name, location.c_str(),
                   message.c_str());
    }
  }

  inline static std::atomic<Level> level_ = Level::kWarning;
};

}  // namespace bdm

#endif  // CORE_UTIL_LOG_H_
