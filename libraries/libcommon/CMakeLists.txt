# Build as a user library
MAKE_LIB_NOSTD(common src/)
TARGET_INCLUDE_DIRECTORIES(common PRIVATE ${CMAKE_SOURCE_DIR}/libraries/libkpi/include)

# Build as a kernel library
MAKE_LIB_NOSTD(common_kernel src/)
TARGET_INCLUDE_DIRECTORIES(common_kernel PRIVATE ${CMAKE_SOURCE_DIR}/kernel/include)
TARGET_COMPILE_DEFINITIONS(common_kernel PUBLIC MAXOS_KERNEL)
