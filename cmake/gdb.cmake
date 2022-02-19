add_custom_target("debug"
    COMMENT "Debugging with GDB"
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/build
    COMMAND gdb -tui --args ${PROJECT_NAME} debug
)
