project(debugger)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src PANGU_DEBUGGER_SRC)
file(GLOB PANGU_DEBUGGER_HEADERS "${CMAKE_CURRENT_LIST_DIR}/src/*.h")

add_executable(debugger ${PANGU_DEBUGGER_SRC} ${PANGU_DEBUGGER_HEADERS})

target_link_libraries(debugger PRIVATE pangu)
add_dependencies(debugger pangu)

set_target_properties(debugger PROPERTIES
    VERSION "${PANGU_VERSION}"
    SOVERSION "${PANGU_VERSION}"
    ARCHIVE_OUTPUT_DIRECTORY "${PANGU_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${PANGU_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PANGU_OUTPUT_DIR}"
    PDB_OUTPUT_DIRECTORY "${PANGU_OUTPUT_DIR}"
)