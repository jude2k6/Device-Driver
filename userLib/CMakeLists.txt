add_executable(test_app
        set.c
)

target_include_directories(test_app PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)