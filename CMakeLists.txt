cmake_minimum_required(VERSION 3.16)

project(Device_driver C)

set(CMAKE_C_STANDARD 11)


add_subdirectory(userLib)
add_subdirectory(driver)


add_custom_target(kernel_module ALL
        COMMAND make
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/driver
        COMMENT "Building Linux kernel module via Kbuild"
)

# Clean kernel build when CMake cleans
add_custom_target(kernel_clean
        COMMAND make clean
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/driver
)