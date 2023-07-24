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

#include <stdint.h>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#ifdef __cplusplus
#define EXTERN_C extern "C"
#define EXTERN_C_ENTER extern "C" {
#define EXTERN_C_LEAVE }
#else
#define EXTERN_C
#define EXTERN_C_ENTER
#define EXTERN_C_LEAVE
#endif

#ifdef __GNUC__
#define PANGU_GCC_VERSION_AT_LEAST(x, y) \
  (__GNUC__ > (x) || __GNUC__ == (x) && __GNUC_MINOR__ >= (y))
#else
#define PANGU_GCC_VERSION_AT_LEAST(x, y) 0
#endif

#if defined(_WIN32)
#define PANGU_CALL __cdecl
#if defined(PANGU_EXPORT)
#define PANGU_API EXTERN_C __declspec(dllexport)
#define PANGU_CPP_API __declspec(dllexport)
#else
#define PANGU_API EXTERN_C __declspec(dllimport)
#define PANGU_CPP_API __declspec(dllimport)
#endif
#elif defined(__APPLE__)
#if PANGU_GCC_VERSION_AT_LEAST(3, 3)
#define PANGU_API __attribute__((visibility("default"))) EXTERN_C
#define PANGU_CPP_API __attribute__((visibility("default")))
#else
#define PANGU_API EXTERN_C
#define PANGU_CPP_API
#endif
#define PANGU_CALL
#elif defined(__ANDROID__) || defined(__linux__)
#if PANGU_GCC_VERSION_AT_LEAST(3, 3)
#define PANGU_API EXTERN_C __attribute__((visibility("default")))
#define PANGU_CPP_API __attribute__((visibility("default")))
#else
#define PANGU_API EXTERN_C
#define PANGU_CPP_API
#endif
#define PANGU_CALL
#else
#define PANGU_API EXTERN_C
#define PANGU_CPP_API
#define PANGU_CALL
#endif

#if PANGU_GCC_VERSION_AT_LEAST(3, 1)
#define PANGU_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define PANGU_DEPRECATED
#else
#define PANGU_DEPRECATED
#endif

#if defined(__GUNC__)
#define COMPILER_IS_GCC
#if defined(__MINGW32__) || defined(__MINGW64__)
#define COMPILER_IS_MINGW
#endif
#if defined(__MSYS__)
#define COMPILER_ON_MSYS
#endif
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
#define COMPILER_ON_CYGWIN
#endif
#if defined(__clang__)
#define COMPILER_IS_CLANG
#endif
#elif defined(_MSC_VER)
#define COMPILER_IS_MSVC
#else
#define COMPILER_IS_UNKNOWN
#endif