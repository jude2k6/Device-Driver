cmake_minimum_required(VERSION 3.15)
project(my_driver C)

# 1. Detect running kernel version and build path
execute_process(
        COMMAND uname -r
        OUTPUT_VARIABLE KERNEL_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(KERNEL_DIR /lib/modules/${KERNEL_VERSION}/build)

# 2. Kernel headers interface library for IntelliSense
add_library(kernel_headers INTERFACE)
target_include_directories(kernel_headers INTERFACE
        ${KERNEL_DIR}/include
        ${KERNEL_DIR}/include/generated
        ${KERNEL_DIR}/arch/x86/include
        ${KERNEL_DIR}/arch/x86/include/generated
)
# Optional: Add kernel macros for parsing
target_compile_definitions(kernel_headers INTERFACE
        __KERNEL__
        KBUILD_MODNAME=\"my_driver\"
)

# 3. Your driver sources
set(SRC
        mouse_char.c
        mouse.h
        ioctl_defs.h
        mouse_usb.c
        mouse_driver.c
)

# 4. Dummy target for CLion IntelliSense
#    This target is never built; it's just for code completion
add_executable(dummy ${SRC})
target_link_libraries(dummy PRIVATE kernel_headers)
set_target_properties(dummy PROPERTIES EXCLUDE_FROM_ALL TRUE)