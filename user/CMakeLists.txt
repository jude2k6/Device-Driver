add_executable(test_app main.c device_api.c)
target_include_directories(test_app PRIVATE ${PROJECT_SOURCE_DIR}/include)
