# NOTE: Cant use MAKE_LIBRARY() as dependency of libc

# Set compiler flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -Wall -Wextra -mcmodel=kernel -mno-red-zone -mno-80387 -mno-mmx -fno-use-cxa-atexit")

# Disable noisy warnings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pointer-arith -Wno-address-of-packed-member -Wno-trigraphs -Wno-unused-variable -Wno-unused-function")

# Collect sources
FILE(GLOB_RECURSE COMMON_SRCS src/*.cpp src/*.s)

# Build as a user library
ADD_LIBRARY(libcommon STATIC ${COMMON_SRCS})
TARGET_INCLUDE_DIRECTORIES(libcommon PUBLIC include)
TARGET_INCLUDE_DIRECTORIES(libcommon PRIVATE ${CMAKE_SOURCE_DIR}/libraries/libkpi/include)
INSTALL(TARGETS libcommon DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)

# Build as a kernel library
ADD_LIBRARY(libcommon_kernel STATIC ${COMMON_SRCS})
TARGET_INCLUDE_DIRECTORIES(libcommon_kernel PUBLIC include)
TARGET_INCLUDE_DIRECTORIES(libcommon_kernel PRIVATE ${CMAKE_SOURCE_DIR}/kernel/include)
TARGET_COMPILE_DEFINITIONS(libcommon_kernel PUBLIC MAXOS_KERNEL)
INSTALL(TARGETS libcommon_kernel DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)