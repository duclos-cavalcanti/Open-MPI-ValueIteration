add_custom_target("run"
    COMMENT "Running ${PROJECT_NAME}"
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/build
    COMMAND ./${PROJECT_NAME} debug
)
