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
#

# This toolchain file requires CMake 3.8.0 or later.
cmake_minimum_required(VERSION 3.8.0)

# CMake invokes the toolchain file twice during the first build, but only once during subsequent rebuilds.
if(DEFINED ENV{_DARWIN_TOOLCHAIN_HAS_RUN})
  return()
endif()
set(ENV{_DARWIN_TOOLCHAIN_HAS_RUN} true)