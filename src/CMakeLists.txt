# ---------------------------------------------------------------------------------------
# Declare the library source files
# ---------------------------------------------------------------------------------------

set(PANGU_SRC_DIR "${CMAKE_HOME_DIRECTORY}/src")

# build
file(GLOB PANGU_BUILD_FILES "${PANGU_SRC_DIR}/build/*.h")

# common
file(GLOB PANGU_COMMON_FILES "${PANGU_SRC_DIR}/common/*.h")

# engine
file(GLOB PANGU_ENGINE_FILES
    "${PANGU_SRC_DIR}/engine/*.h"
    "${PANGU_SRC_DIR}/engine/*.cc"
)

# public
file(GLOB PANGU_PUBLIC_HEADERS "${PANGU_SRC_DIR}/include/pangu/*.h")
set(PANGU_PUBLIC_HEADERS ${PANGU_PUBLIC_HEADERS} PARENT_SCOPE)

# modules
file(GLOB PANGU_MODULE_FILES
    "${PANGU_SRC_DIR}/modules/*.h"
    "${PANGU_SRC_DIR}/modules/*.cc"
)

# pangu
set(PANGU_SRCS
    ${PANGU_BUILD_FILES}
    ${PANGU_COMMON_FILES}
    ${PANGU_ENGINE_FILES}
    ${PANGU_PUBLIC_HEADERS}
    ${PANGU_MODULE_FILES}
    PARENT_SCOPE
)
