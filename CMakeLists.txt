# ##############################################################################
# CMake Section
# ##############################################################################

# Set minimum verison of cmake to 3.10.
cmake_minimum_required(VERSION 3.10)

# Recognize ``TEST`` as a operator for the ``if()`` command.
if(POLICY CMP0064)
    cmake_policy(SET CMP0064 NEW)
endif(POLICY CMP0064)

# ``MACOSX_RPATH`` is enabled by default.
if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif(POLICY CMP0042)

# Set default build to release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

message(STATUS "[PG]: Build type: " ${CMAKE_BUILD_TYPE})

# Include toolchain folder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Enable folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Enable generate lib file for shared library.
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# ##############################################################################
# Options Section
# ##############################################################################

# build shared option
option(PANGU_BUILD_SHARED "Build shared library" ON)

# testing options
option(PANGU_BUILD_TESTS "Build tests" ON)

# sanitizer options
option(PANGU_SANITIZE_ADDRESS "Enable address sanitizer in tests" OFF)

# debugger options
option(PANGU_DEBUGGER "Enable debugger in tests" OFF)

# ##############################################################################
# Project Section
# ##############################################################################

# Include cmake/utils.cmake
include(utils)

# Extract version of pangu from src/common/version.h file as ``PANGU_VERSION``
pangu_extract_version()

# Declare project pangu
project(pangu VERSION ${PANGU_VERSION} LANGUAGES CXX)
message(STATUS "[PG]: Build version: ${PANGU_VERSION}")

set(CMAKE_CXX_STANDARD 17)

# Set output directory
set(PANGU_OUTPUT_DIR "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
message(STATUS "[PG]: Set PANGU_OUTPUT_DIR: ${PANGU_OUTPUT_DIR}")

set(PANGU_OUTPUT_NAME "pangu")
message(STATUS "[PG]: Set PANGU_OUTPUT_NAME: ${PANGU_OUTPUT_NAME}")

# Determine whether current system or target system is supported or not.
if(NOT MACOS AND NOT IOS AND NOT ANDROID AND NOT WIN32)
    message(FATAL_ERROR "[PG]: Current target system ${CMAKE_SYSTEM_NAME} is not supported for now")
endif()

if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows" AND NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    message(FATAL_ERROR "[PG]: Current host system ${CMAKE_HOST_SYSTEM_NAME} is not supported to build for now")
endif()

# Check compiler
# MSVC is the only compiler supported on Windows for now and should use after project declaration
if(WIN32 AND NOT MSVC)
    message(FATAL_ERROR "[PG]: Only MSVC is supported on Windows")
elseif(APPLE)
elseif(ANDROID)
endif()

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG")

    if(MSVC)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd /Zi /arch:SSE2")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT /Zi /arch:SSE2")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MT /Zi /arch:SSE2")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /Zi /arch:SSE2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wo4018 /wo4244 /wo4251 /wo4275 /wo4996 /we4033 /we4700")
    endif()
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ggdb -Wall -Werror=uninitialized -Werror=return-type")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -Wall -Werror=uninitialized -Werror=return-type")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

if(MACOS)
    set(CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "10.10")
    set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
    set(BUILD_UNITY_BUNDLE TRUE)
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fPIC")
endif()

# Include toolchain from cmake folder
if(WIN32)
    include(msvc.toolchain)
elseif(APPLE)
    if(MAC)
        include(darwin.toolchain)
    elseif(IOS)
        include(ios.toolchain)
    endif()
elseif(ANDROID)
    # include(android.toolchain)
endif()

# ##############################################################################
# Thridparty Section
# ##############################################################################

# gbase
add_subdirectory(third_party/gbase)
set_property(TARGET gbase PROPERTY FOLDER "third_party")
set(PANGU_THIRD_PARTY_GBASE_INCLUDE_DIR "${CMAKE_HOME_DIRECTORY}/third_party/gbase")

# spdlog
add_subdirectory(third_party/spdlog)
set_property(TARGET spdlog PROPERTY FOLDER "third_party")
set(PANGU_THIRD_PARTY_SPDLOG_INCLUDE_DIR "${CMAKE_HOME_DIRECTORY}/third_party/spdlog/include")

# nlohmann json
set(PANGU_THIRD_PARTY_NLOHMANN_INCLUDE_DIR "${CMAKE_HOME_DIRECTORY}/third_party/nlohmann/include")

# asio
set(PANGU_THIRD_PARTY_ASIO_INCLUDE_DIR "${CMAKE_HOME_DIRECTORY}/third_party/asio/asio/include")

# libyuv
add_subdirectory(third_party/libyuv)
set_property(TARGET yuv PROPERTY FOLDER "third_party/libyuv")
set_property(TARGET yuv_shared PROPERTY FOLDER "third_party/libyuv")
set_property(TARGET yuvconvert PROPERTY FOLDER "third_party/libyuv")
set_property(TARGET yuvconstants PROPERTY FOLDER "third_party/libyuv")

# ##############################################################################
# Library Section
# ##############################################################################

# Add srcs
add_subdirectory(src)

# Notice that source_group() must call in the same CMakeLists.txt with target_sources()
source_group(TREE ${CMAKE_HOME_DIRECTORY}/src FILES ${PANGU_SRCS})

if(PANGU_BUILD_SHARED OR BUILD_SHARED_LIBS)
    message(STATUS "[PG]: Build shared library")

    set(PANGU_BUILD_SHARED ON)
    add_definitions(-DPANGU_BUILD_SHARED)
    add_definitions(-DPANGU_SHARED)

    # we need a extra version file to declare version info for windows.
    if(WIN32)
        add_definitions(-DPANGU_EXPORT -D_UNICODE -DUNICODE)
        configure_file(${CMAKE_HOME_DIRECTORY}/cmake/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc @ONLY)
        list(APPEND PANGU_SRCS ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
        source_group("" FILES ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
    endif()

    if(NOT APPLE)
        file(MAKE_DIRECTORY "${PANGU_OUTPUT_DIR}/include/pangu")
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${PANGU_PUBLIC_HEADERS} "${PANGU_OUTPUT_DIR}/include/pangu"
            WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
        )
    endif()

    add_library(pangu SHARED ${PANGU_SRCS})
else()
    message(STATUS "[PG]: Build static library")
    add_library(pangu STATIC ${PANGU_SRCS})
endif()

# Declare pangu as alias of library pangu::pangu
add_library(pangu::pangu ALIAS pangu)

# Generate export header
# include(GenerateExportHeader)
# generate_export_header(pangu EXPORT_FILE_NAME "${CMAKE_HOME_DIRECTORY}/include/export.h")

# Link gbase
target_link_libraries(pangu PUBLIC gbase)
add_dependencies(pangu gbase)

# Link spdlog
target_link_libraries(pangu PUBLIC spdlog)
add_dependencies(pangu spdlog)

# Link yuv
target_link_libraries(pangu PUBLIC yuv)
add_dependencies(pangu yuv)

# Add include directories
set(PANGU_INCLUDE_DIR
    ${CMAKE_HOME_DIRECTORY}/src
    ${CMAKE_HOME_DIRECTORY}/src/include
    ${PANGU_THIRD_PARTY_GBASE_INCLUDE_DIR}
    ${PANGU_THIRD_PARTY_ASIO_INCLUDE_DIR}
    ${PANGU_THIRD_PARTY_NLOHMANN_INCLUDE_DIR}
    ${PANGU_THIRD_PARTY_SPDLOG_INCLUDE_DIR}
)
target_include_directories(pangu PUBLIC "$<BUILD_INTERFACE:${PANGU_INCLUDE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

set_target_properties(pangu PROPERTIES
    VERSION "${PANGU_VERSION}"
    SOVERSION "${PANGU_VERSION}"
    PUBLIC_HEADER "${PANGU_PUBLIC_HEADERS}"
    ARCHIVE_OUTPUT_DIRECTORY "${PANGU_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${PANGU_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PANGU_OUTPUT_DIR}"
    PDB_OUTPUT_DIRECTORY "${PANGU_OUTPUT_DIR}"
    OUTPUT_NAME "${PANGU_OUTPUT_NAME}"
)

# ##############################################################################
# UnitTest Section
# ##############################################################################
if(PANGU_BUILD_TESTS)
    message(STATUS "[PG]: Build tests")
    enable_testing()

    add_compile_definitions(PANGU_BUILD_TESTS=1)
    add_subdirectory(third_party/googletest googletest)
    set_property(TARGET gtest PROPERTY FOLDER "third_party/googletest")
    set_property(TARGET gtest_main PROPERTY FOLDER "third_party/googletest")
    set_property(TARGET gmock PROPERTY FOLDER "third_party/googletest")
    set_property(TARGET gmock_main PROPERTY FOLDER "third_party/googletest")

    add_subdirectory(tests)
endif()

# ##############################################################################
# Extra modules Section
# ##############################################################################

# Debugger
if(PANGU_DEBUGGER)
    add_subdirectory(debugger)
endif()