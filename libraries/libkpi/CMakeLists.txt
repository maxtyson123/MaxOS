# NOTE: Cant use MAKE_LIBRARY() as dependency of libc

# Set compiler flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -Wall -Wextra -mno-red-zone -mno-80387 -mno-mmx -fno-use-cxa-atexit")

# Disable noisy warnings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pointer-arith -Wno-address-of-packed-member -Wno-trigraphs -Wno-unused-variable -Wno-unused-function")

# Collect sources
FILE(GLOB_RECURSE KPI_SRCS src/*.cpp src/*.s)

# Build as a dynamic library
ADD_LIBRARY(libkpi SHARED ${KPI_SRCS})
TARGET_INCLUDE_DIRECTORIES(libkpi PUBLIC include)
INSTALL(TARGETS libkpi LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)

# Build as a static library
ADD_LIBRARY(libkpi_static STATIC ${KPI_SRCS})
TARGET_INCLUDE_DIRECTORIES(libkpi_static PUBLIC include)
INSTALL(TARGETS libkpi_static DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)