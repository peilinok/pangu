# Get pangu version from include/version.h and put it in PANGU_VERSION
function(pangu_extract_version)
    file(READ "${CMAKE_HOME_DIRECTORY}/src/common/version.h" file_contents)

    # major
    string(REGEX MATCH "PANGU_VER_MAJOR ([0-9]+)" _ "${file_contents}")

    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract major version number from include/version.h")
    endif()

    set(ver_major ${CMAKE_MATCH_1})

    # minor
    string(REGEX MATCH "PANGU_VER_MINOR ([0-9]+)" _ "${file_contents}")

    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract minor version number from include/version.h")
    endif()

    set(ver_minor ${CMAKE_MATCH_1})

    # patch
    string(REGEX MATCH "PANGU_VER_PATCH ([0-9]+)" _ "${file_contents}")

    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract patch version number from include/version.h")
    endif()

    set(ver_patch ${CMAKE_MATCH_1})

    # build
    string(REGEX MATCH "PANGU_VER_BUILD ([0-9]+)" _ "${file_contents}")

    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract build version number from include/version.h")
    endif()

    set(ver_build ${CMAKE_MATCH_1})

    set(PANGU_VERSION_MAJOR ${ver_major} PARENT_SCOPE)
    set(PANGU_VERSION_MINOR ${ver_minor} PARENT_SCOPE)
    set(PANGU_VERSION_PATCH ${ver_patch} PARENT_SCOPE)
    set(PANGU_VERSION_BUILD ${ver_build} PARENT_SCOPE)
    set(PANGU_VERSION "${ver_major}.${ver_minor}.${ver_patch}.${ver_build}" PARENT_SCOPE)
endfunction()

# Enable address sanitizer (gcc/clang only)
function(pangu_enable_sanitizer target_name)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "Sanitizer supported only for gcc/clang")
    endif()

    message(STATUS "Address sanitizer enabled")
    target_compile_options(${target_name} PRIVATE -fsanitize=address,undefined)
    target_compile_options(${target_name} PRIVATE -fno-sanitize=signed-integer-overflow)
    target_compile_options(${target_name} PRIVATE -fno-sanitize-recover=all)
    target_compile_options(${target_name} PRIVATE -fno-omit-frame-pointer)
    target_link_libraries(${target_name} PRIVATE -fsanitize=address,undefined -fuse-ld=gold)
endfunction()