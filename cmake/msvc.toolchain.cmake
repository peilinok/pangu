# *****************************************************************************
#
#                           INFORMATION / HELP
#
###############################################################################
#                                  OPTIONS                                    #
###############################################################################
#
# PLATFORM: (default "x86")
#    x86 = Build for x86 platform.
#
# ARCHS: (x86 x86_64) If specified, will override the default architectures for the given PLATFORM
#    x86 = x86
#    x86_64 = x86_64 
#
# NOTE: When manually specifying ARCHS, put a semi-colon between the entries. E.g., -DARCHS="x86;x86_64"
#
###############################################################################
#                                END OPTIONS                                  #
###############################################################################
#
# This toolchain defines the following properties (available via get_property()) for use externally:
#
# PLATFORM: The currently targeted platform.
#
# This toolchain defines the following macros for use externally:
#
# set_msvc_property (TARGET MSVC_PROPERTY MSVC_VALUE MSVC_VARIANT)
#   A convenience macro for setting msvc specific properties on targets.
#   Available variants are: All, Release, RelWithDebInfo, Debug, MinSizeRel
#   example: set_msvc_property (mylib IPHONEOS_DEPLOYMENT_TARGET "3.1" "All").
#

# This toolchain file requires CMake 3.8.0 or later.
cmake_minimum_required(VERSION 3.8.0)

# CMake invokes the toolchain file twice during the first build, but only once during subsequent rebuilds.
if(DEFINED ENV{_MSVC_TOOLCHAIN_HAS_RUN})
  return()
endif()
set(ENV{_MSVC_TOOLCHAIN_HAS_RUN} true)

if (MSVC)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd /Zi /arch:SSE2")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT /Zi /arch:SSE2")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MT /Zi /arch:SSE2")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /Zi /arch:SSE2")
endif()

#
# Some helper-macros below to simplify and beautify the CMakeFile
#

# This little macro lets you set any MSVC specific property.
macro(set_msvc_property TARGET MSVC_PROPERTY MSVC_VALUE MSVC_RELVERSION)
  set(MSVC_RELVERSION_I "${MSVC_RELVERSION}")
  if(MSVC_RELVERSION_I STREQUAL "All")
    set_property(TARGET ${TARGET} PROPERTY MSVC_ATTRIBUTE_${MSVC_PROPERTY} "${MSVC_VALUE}")
  else()
    set_property(TARGET ${TARGET} PROPERTY MSVC_ATTRIBUTE_${MSVC_PROPERTY}[variant=${MSVC_RELVERSION_I}] "${MSVC_VALUE}")
  endif()
endmacro(set_msvc_property)