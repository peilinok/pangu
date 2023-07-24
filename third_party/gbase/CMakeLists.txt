project(gbase)

# Require c++ 17 or newer.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR
        "gbase requires CMAKE_CXX_STANDARD >= 17 (got: ${CMAKE_CXX_STANDARD})")
endif()

# Set gbase root directory and source directory.
set(GBASE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(GBASE_SRC_DIR "${GBASE_ROOT_DIR}/base")

# BUILD FALGS 'BUILD_RUST_JSON_READER' and 'BUILD_RUST_BASE_CONVERSIONS'
option(BUILD_RUST_JSON_READER "Build the Rust JSON reader" OFF)

if(BUILD_RUST_JSON_READER)
    add_definitions(-DBUILD_RUST_JSON_READER)
endif()

option(BUILD_RUST_BASE_CONVERSIONS "Build the Rust base conversions" OFF)

if(BUILD_RUST_BASE_CONVERSIONS)
    add_definitions(-BUILD_RUST_BASE_CONVERSIONS)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.rust_buildflags.h.in
    ${GBASE_SRC_DIR}/rust_buildflags.h
    @ONLY
)

# BUILD FALGS 'CRONET_BUILD'
option(CRONET_BUILD "Build the Cronet implementation" OFF)

if(CRONET_BUILD)
    add_definitions(-DCRONET_BUILD)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.message_loop.ios_cronet_buildflags.h.in
    ${GBASE_SRC_DIR}/message_loop/ios_cronet_buildflags.h
    @ONLY
)

# BUILD FALGS 'ENABLE_MESSAGE_PUMP_EPOLL'
option(ENABLE_MESSAGE_PUMP_EPOLL "Enable the epoll message pump" OFF)

if(ENABLE_MESSAGE_PUMP_EPOLL)
    add_definitions(-DENABLE_MESSAGE_PUMP_EPOLL)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.message_loop.message_pump_buildflags.h.in
    ${GBASE_SRC_DIR}/message_loop/message_pump_buildflags.h
    @ONLY
)

# BUILD FALGS 'CFI_CAST_CHECK' 'CFI_DIAG CFI_ICALL_CHECK' 'CFI_ENFORCEMENT_TRAP' 'CFI_ENFORCEMENT_DIAGNOSTIC'
option(CFI_CAST_CHECK "Enable the cfi cast check" OFF)

if(CFI_CAST_CHECK)
    add_definitions(-DCFI_CAST_CHECK)
endif()

option(CFI_DIAG "Enable the cfi diag" OFF)

if(CFI_DIAG)
    add_definitions(-DCFI_DIAG)
endif()

option(CFI_ICALL_CHECK "Enable the cfi icall check" OFF)

if(CFI_ICALL_CHECK)
    add_definitions(-DCFI_ICALL_CHECK)
endif()

option(CFI_ENFORCEMENT_TRAP "Enable the cfi enforcement trap" OFF)

if(CFI_ENFORCEMENT_TRAP)
    add_definitions(-DCFI_ENFORCEMENT_TRAP)
endif()

option(CFI_ENFORCEMENT_DIAGNOSTIC "Enable the cfi enforcement diagnostic" OFF)

if(CFI_ENFORCEMENT_DIAGNOSTIC)
    add_definitions(-DCFI_ENFORCEMENT_DIAGNOSTIC)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.cfi_buildflags.h.in
    ${GBASE_SRC_DIR}/cfi_buildflags.h
    @ONLY
)

# BUILD FALGS 'DCHECK_IS_CONFIGURABLE' 'ENABLE_PROFILING' 'CAN_UNWIND_WITH_FRAME_POINTERS' 'UNSAFE_DEVELOPER_BUILD' 'CAN_UNWIND_WITH_CFI_TABLE'
option(DCHECK_IS_CONFIGURABLE "Enable the dcheck is configurable" OFF)

if(DCHECK_IS_CONFIGURABLE)
    add_definitions(-DDCHECK_IS_CONFIGURABLE)
endif()

option(ENABLE_PROFILING "Enable the profiling" OFF)

if(ENABLE_PROFILING)
    add_definitions(-DENABLE_PROFILING)
endif()

option(CAN_UNWIND_WITH_FRAME_POINTERS "Enable the can unwind with frame pointers" OFF)

if(CAN_UNWIND_WITH_FRAME_POINTERS)
    add_definitions(-DCAN_UNWIND_WITH_FRAME_POINTERS)
endif()

option(UNSAFE_DEVELOPER_BUILD "Enable the unsafe developer build" OFF)

if(UNSAFE_DEVELOPER_BUILD)
    add_definitions(-DUNSAFE_DEVELOPER_BUILD)
endif()

option(CAN_UNWIND_WITH_CFI_TABLE "Enable the can unwind with cfi table" OFF)

if(CAN_UNWIND_WITH_CFI_TABLE)
    add_definitions(-DCAN_UNWIND_WITH_CFI_TABLE)
endif()

option(EXCLUDE_UNWIND_TABLES "Enable the exclude unwind tables" OFF)

if(EXCLUDE_UNWIND_TABLES)
    add_definitions(-DEXCLUDE_UNWIND_TABLES)
endif()

option(ENABLE_GDBINIT_WARNING "Enable the gdbinit warning" OFF)

if(ENABLE_GDBINIT_WARNING)
    add_definitions(-DENABLE_GDBINIT_WARNING)
endif()

option(ENABLE_LLDBINIT_WARNING "Enable the lldbinit warning" OFF)

if(ENABLE_LLDBINIT_WARNING)
    add_definitions(-DENABLE_LLDBINIT_WARNING)
endif()

option(EXPENSIVE_DCHECKS_ARE_ON "Enable the expensive dchecks are on" OFF)

if(EXPENSIVE_DCHECKS_ARE_ON)
    add_definitions(-DEXPENSIVE_DCHECKS_ARE_ON)
endif()

option(ENABLE_STACK_TRACE_LINE_NUMBERS "Enable the stack trace line numbers" OFF)

if(ENABLE_STACK_TRACE_LINE_NUMBERS)
    add_definitions(-DENABLE_STACK_TRACE_LINE_NUMBERS)
endif()

option(ENABLE_COMMANDLINE_SEQUENCE_CHECKS "Enable the commandline sequence checks" OFF)

if(ENABLE_COMMANDLINE_SEQUENCE_CHECKS)
    add_definitions(-DENABLE_COMMANDLINE_SEQUENCE_CHECKS)
endif()

option(ENABLE_ALLOCATION_STACK_TRACE_RECORDER "Enable the allocation stack trace recorder" OFF)

if(ENABLE_ALLOCATION_STACK_TRACE_RECORDER)
    add_definitions(-DENABLE_ALLOCATION_STACK_TRACE_RECORDER)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.debug.debugging_buildflags.h.in
    ${GBASE_SRC_DIR}/debug/debugging_buildflags.h
    @ONLY
)

# BUILD FALGS 'ENABLE_BANNED_BASE_FEATURE_PREFIX'
option(ENABLE_BANNED_BASE_FEATURE_PREFIX "Enable the banned base feature prefix" OFF)

if(ENABLE_BANNED_BASE_FEATURE_PREFIX)
    add_definitions(-DENABLE_BANNED_BASE_FEATURE_PREFIX)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.feature_list_buildflags.h.in
    ${GBASE_SRC_DIR}/feature_list_buildflags.h
    @ONLY
)

# BUILD FALGS 'ENABLE_LOG_ERROR_NOT_REACHED' 'USE_RUNTIME_VLOG'
option(ENABLE_LOG_ERROR_NOT_REACHED "Enable the log error not reached" OFF)

if(ENABLE_LOG_ERROR_NOT_REACHED)
    add_definitions(-DENABLE_LOG_ERROR_NOT_REACHED)
endif()

option(USE_RUNTIME_VLOG "Enable the use runtime vlog" OFF)

if(USE_RUNTIME_VLOG)
    add_definitions(-DUSE_RUNTIME_VLOG)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.logging_buildflags.h.in
    ${GBASE_SRC_DIR}/logging_buildflags.h
    @ONLY
)

# BUILD FALGS 'DEVTOOLS_INSTRUMENTATION_DUMPING' 'ORDERFILE_INSTRUMENTATION'
option(DEVTOOLS_INSTRUMENTATION_DUMPING "Enable the devtools instrumentation dumping" OFF)

if(DEVTOOLS_INSTRUMENTATION_DUMPING)
    add_definitions(-DDEVTOOLS_INSTRUMENTATION_DUMPING)
endif()

option(ORDERFILE_INSTRUMENTATION "Enable the orderfile instrumentation" OFF)

if(ORDERFILE_INSTRUMENTATION)
    add_definitions(-DORDERFILE_INSTRUMENTATION)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.android.orderfile.orderfile_buildflags.h.in
    ${GBASE_SRC_DIR}/android/orderfile/orderfile_buildflags.h
    @ONLY
)

# BUILD FALGS 'ENABLE_MUTEX_PRIORITY_INHERITANCE'
option(ENABLE_MUTEX_PRIORITY_INHERITANCE "Enable the mutex priority inheritance" OFF)

if(ENABLE_MUTEX_PRIORITY_INHERITANCE)
    add_definitions(-DENABLE_MUTEX_PRIORITY_INHERITANCE)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.synchronization.synchronization_buildflags.h.in
    ${GBASE_SRC_DIR}/synchronization/synchronization_buildflags.h
    @ONLY
)

# BUILD FALGS 'USE_LLD' 'SUPPORTS_CODE_ORDERING'
option(USE_LLD "Enable the use lld" OFF)

if(USE_LLD)
    add_definitions(-DUSE_LLD)
endif()

option(SUPPORTS_CODE_ORDERING "Enable the supports code ordering" OFF)

if(SUPPORTS_CODE_ORDERING)
    add_definitions(-DSUPPORTS_CODE_ORDERING)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.android.library_loader.anchor_functions_buildflags.h.in
    ${GBASE_SRC_DIR}/android/library_loader.anchor_functions_buildflags.h
    @ONLY
)

# BUILD FALGS 'CLANG_PROFILING' 'CLANG_PROFILING_INSIDE_SANDBOX' 'USE_CLANG_COVERAGE'
option(CLANG_PROFILING "Enable the use lld" OFF)

if(CLANG_PROFILING)
    add_definitions(-DCLANG_PROFILING)
endif()

option(CLANG_PROFILING_INSIDE_SANDBOX "Enable the supports code ordering" OFF)

if(CLANG_PROFILING_INSIDE_SANDBOX)
    add_definitions(-DCLANG_PROFILING_INSIDE_SANDBOX)
endif()

option(USE_CLANG_COVERAGE "Enable the use clang coverage" OFF)

if(USE_CLANG_COVERAGE)
    add_definitions(-DUSE_CLANG_COVERAGE)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.clang_profiling_buildflags.h.in
    ${GBASE_SRC_DIR}/clang_profiling_buildflags.h
    @ONLY
)

# BUILD FALGS 'IS_HWASAN' 'USING_SANITIZER'
option(IS_HWASAN "Enable the is hwasan" OFF)

if(IS_HWASAN)
    add_definitions(-DIS_HWASAN)
endif()

option(USING_SANITIZER "Enable the using sanitizer" OFF)

if(USING_SANITIZER)
    add_definitions(-DUSING_SANITIZER)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.sanitizer_buildflags.h.in
    ${GBASE_SRC_DIR}/sanitizer_buildflags.h
    @ONLY
)

# BUILD FALGS 'ENABLE_BASE_TRACING' 'USE_PERFETTO_CLIENT_LIBRARY' 'OPTIONAL_TRACE_EVENTS_ENABLED'
option(ENABLE_BASE_TRACING "Enable the enable base tracing" OFF)

if(ENABLE_BASE_TRACING)
    add_definitions(-DENABLE_BASE_TRACING)
endif()

option(USE_PERFETTO_CLIENT_LIBRARY "Enable the use perfetto client library" OFF)

if(USE_PERFETTO_CLIENT_LIBRARY)
    add_definitions(-DUSE_PERFETTO_CLIENT_LIBRARY)
endif()

option(OPTIONAL_TRACE_EVENTS_ENABLED "Enable the optional trace events enabled" OFF)

if(OPTIONAL_TRACE_EVENTS_ENABLED)
    add_definitions(-DOPTIONAL_TRACE_EVENTS_ENABLED)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.tracing_buildflags.h.in
    ${GBASE_SRC_DIR}/tracing_buildflags.h
    @ONLY
)

# BUILD FALGS 'ENABLE_ARM_CFI_TABLE' 'IOS_STACK_PROFILER_ENABLED'
option(ENABLE_ARM_CFI_TABLE "Enable the enable arm cfi table" OFF)

if(ENABLE_ARM_CFI_TABLE)
    add_definitions(-DENABLE_ARM_CFI_TABLE)
endif()

option(IOS_STACK_PROFILER_ENABLED "Enable the ios stack profiler enabled" OFF)

if(IOS_STACK_PROFILER_ENABLED)
    add_definitions(-DIOS_STACK_PROFILER_ENABLED)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.profiler.profiler_buildflags.h.in
    ${GBASE_SRC_DIR}/profiler/profiler_buildflags.h
    @ONLY
)

# BUILD FALGS 'HAS_BATTERY_LEVEL_PROVIDER_IMPL'
option(HAS_BATTERY_LEVEL_PROVIDER_IMPL "Enable the enable arm cfi table" OFF)

if(HAS_BATTERY_LEVEL_PROVIDER_IMPL)
    add_definitions(-DHAS_BATTERY_LEVEL_PROVIDER_IMPL)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.power_monitor.power_monitor_buildflags.h.in
    ${GBASE_SRC_DIR}/power_monitor/power_monitor_buildflags.h
    @ONLY
)

# BUILD FALGS 'USE_ALLOCATOR_SHIM' 'USE_PARTITION_ALLOC_AS_GWP_ASAN_STORE'
option(USE_ALLOCATOR_SHIM "Enable the use allocator shim" OFF)

if(USE_ALLOCATOR_SHIM)
    add_definitions(-DUSE_ALLOCATOR_SHIM)
endif()

option(USE_PARTITION_ALLOC_AS_GWP_ASAN_STORE "Enable the use partition alloc as gwp asan store" OFF)

if(USE_PARTITION_ALLOC_AS_GWP_ASAN_STORE)
    add_definitions(-DUSE_PARTITION_ALLOC_AS_GWP_ASAN_STORE)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.allocator.buildflags.h.in
    ${GBASE_SRC_DIR}/allocator/buildflags.h
    @ONLY
)

# BUILD FALGS 'PA_IS_CHROMEOS_ASH'
option(PA_IS_CHROMEOS_ASH "Enable the pa is cast android" OFF)

if(PA_IS_CHROMEOS_ASH)
    add_definitions(-DPA_IS_CHROMEOS_ASH)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.allocator.partition_allocator.chromeos_buildflags.h.in
    ${GBASE_SRC_DIR}/allocator/partition_allocator/chromeos_buildflags.h
    @ONLY
)

# BUILD FALGS 'PA_IS_CAST_ANDROID' 'PA_IS_CASTOS'
option(PA_IS_CAST_ANDROID "Enable the pa is cast android" OFF)

if(PA_IS_CAST_ANDROID)
    add_definitions(-DPA_IS_CAST_ANDROID)
endif()

option(PA_IS_CASTOS "Enable the pa is castos" OFF)

if(PA_IS_CASTOS)
    add_definitions(-DPA_IS_CASTOSE)
endif()

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.allocator.partition_allocator.chromecast_buildflags.h.in
    ${GBASE_SRC_DIR}/allocator/partition_allocator/chromecast_buildflags.h
    @ONLY
)

# Disable partition allocator and remove partition allocator sources.
option(ENABLE_PARTITION_ALLOCATOR "Disable the partition allocator" OFF)

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.allocator.partition_allocator.partition_alloc_buildflags.h.in
    ${GBASE_SRC_DIR}/allocator/partition_allocator/partition_alloc_buildflags.h
    @ONLY
)

configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.allocator.partition_allocator.partition_alloc_base.debug.debugging_buildflags.h.in
    ${GBASE_SRC_DIR}/allocator/partition_allocator/partition_alloc_base/debug/debugging_buildflags.h
    @ONLY
)

# Add base files
file(GLOB GBASE_SRC_FILES
    "${GBASE_ROOT_DIR}/base/allocator/partition_allocator/partition_alloc_base/compiler_specific.h"
    "${GBASE_ROOT_DIR}/base/allocator/partition_allocator/pointers/raw_ptr_exclusion.h"
    "${GBASE_ROOT_DIR}/base/allocator/partition_allocator/partition_alloc_buildflags.h"
    "${GBASE_ROOT_DIR}/base/containers/*.h"
    "${GBASE_ROOT_DIR}/base/files/*.h"
    "${GBASE_ROOT_DIR}/base/files/*.h"
    "${GBASE_ROOT_DIR}/base/files/*.cc"
    "${GBASE_ROOT_DIR}/base/functional/*.h"
    "${GBASE_ROOT_DIR}/base/functional/*.cc"
    "${GBASE_ROOT_DIR}/base/memory/*.h"
    "${GBASE_ROOT_DIR}/base/memory/*.cc"
    "${GBASE_ROOT_DIR}/base/numerics/*.h"
    "${GBASE_ROOT_DIR}/base/numerics/*.cc"
    "${GBASE_ROOT_DIR}/base/ranges/*.h"
    "${GBASE_ROOT_DIR}/base/ranges/*.cc"
    "${GBASE_ROOT_DIR}/base/strings/*.h"
    "${GBASE_ROOT_DIR}/base/strings/*.cc"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/bignum-dtoa.cc"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/bignum-dtoa.h"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/bignum.cc"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/bignum.h"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/cached-powers.cc"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/cached-powers.h"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/diy-fp.h"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/double-conversion.h"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/double-to-string.cc"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/double-to-string.h"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/fast-dtoa.cc"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/fast-dtoa.h"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/fixed-dtoa.cc"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/fixed-dtoa.h"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/ieee.h"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/string-to-double.cc"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/string-to-double.h"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/strtod.cc"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/strtod.h"
    "${GBASE_ROOT_DIR}/base/third_party/double_conversion/double-conversion/utils.h"
    "${GBASE_ROOT_DIR}/base/third_party/icu/icu_utf.h"
    "${GBASE_ROOT_DIR}/base/third_party/modp_b64/modp_b64_data.h"
    "${GBASE_ROOT_DIR}/base/third_party/modp_b64/modp_b64.cc"
    "${GBASE_ROOT_DIR}/base/third_party/modp_b64/modp_b64.h"
    "${GBASE_ROOT_DIR}/base/types/*.h"
    "${GBASE_ROOT_DIR}/base/types/*.cc"
    "${GBASE_ROOT_DIR}/base/*.h"
    "${GBASE_ROOT_DIR}/base/*.cc"
)

if(NOT POSIX)
    list(REMOVE_ITEM GBASE_SRC_FILES "${GBASE_ROOT_DIR}/base/strings/string_util_posix.h")
    list(REMOVE_ITEM GBASE_SRC_FILES "${GBASE_ROOT_DIR}/base/strings/sys_string_conversions_posix.cc")
endif()

if(NOT MAC)
    list(REMOVE_ITEM GBASE_SRC_FILES "${GBASE_ROOT_DIR}/base/strings/sys_string_conversions_mac.mm")
endif()

if(NOT WIN32)
    list(REMOVE_ITEM GBASE_SRC_FILES "${GBASE_ROOT_DIR}/base/strings/string_util_win.h")
    list(REMOVE_ITEM GBASE_SRC_FILES "${GBASE_ROOT_DIR}/base/strings/string_util_win.cc")
    list(REMOVE_ITEM GBASE_SRC_FILES "${GBASE_ROOT_DIR}/base/strings/sys_string_conversions_win.cc")
endif()

# Generate the build date header.
configure_file(
    ${GBASE_ROOT_DIR}/cmake/base.generated_build_date.h.in
    ${GBASE_SRC_DIR}/generated_build_date.h
    @ONLY
)

# Build UNICODE for windows or set SYSTEM_NATIVE_UTF8 for other platforms.
if(WIN32)
    add_definitions(-DUNICODE)
    add_definitions(-D_UNICODE)
else()
    add_definitions(-DSYSTEM_NATIVE_UTF8)
endif()

# Set the source group for IDEs.
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${GBASE_SRC_FILES})

# Create the library.
add_library(gbase STATIC ${GBASE_SRC_FILES})

# Set the include directories.
target_include_directories(gbase PRIVATE ${GBASE_ROOT_DIR})

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS

        # Linking with shcore.lib causes the shcore api forwarder dll to load.
        "/DELAYLOAD:api-ms-win-shcore-scaling-l1-1-1.dll"

        # Linking with OneCore.lib causes the next three dlls to load.
        "/DELAYLOAD:api-ms-win-core-realtime-l1-1-1.dll"
        "/DELAYLOAD:api-ms-win-power-base-l1-1-0.dll"
        "/DELAYLOAD:api-ms-win-power-setting-l1-1-0.dll"
        "/DELAYLOAD:cfgmgr32.dll"
        "/DELAYLOAD:powrprof.dll"
        "/DELAYLOAD:setupapi.dll"
        "/DELAYLOAD:shcore.dll"
    )
    target_link_libraries(gbase PRIVATE
        "cfgmgr32.lib"
        "ntdll.lib"
        "onecore.lib"
        "powrprof.lib"
        "propsys.lib"
        "setupapi.lib"
        "shcore.lib"
        "userenv.lib"
        "wbemuuid.lib"
        "winmm.lib"
    )
endif()

# Add abseil-cpp.
target_include_directories(gbase PRIVATE ${GBASE_ROOT_DIR}/third_party/abseil-cpp)

add_subdirectory(${GBASE_ROOT_DIR}/third_party/abseil-cpp)
target_link_libraries(gbase PRIVATE
    absl::algorithm
    absl::base
    absl::debugging
    absl::flat_hash_map
    absl::flags
    absl::memory
    absl::meta
    absl::numeric
    absl::random_random
    absl::strings
    absl::synchronization
    absl::time
    absl::utility
)