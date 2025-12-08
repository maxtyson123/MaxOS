#TODO: Make func to do this shit

# Set compiler flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -Wall -Wextra -mno-red-zone -mno-80387 -mno-mmx -fno-use-cxa-atexit")

# Disable noisy warnings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pointer-arith -Wno-address-of-packed-member -Wno-trigraphs -Wno-unused-variable -Wno-unused-function")

# Collect sources
FILE(GLOB_RECURSE SYSTEM_SRCS src/*.cpp src/*.s)

# Build as a dynamic library
ADD_LIBRARY(libdriver SHARED ${SYSTEM_SRCS})
TARGET_INCLUDE_DIRECTORIES(libdriver PUBLIC include)
TARGET_LINK_LIBRARIES(libdriver libkpi)
INSTALL(TARGETS libdriver LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)

# Build as a static library
ADD_LIBRARY(libdriver_static STATIC ${SYSTEM_SRCS})
TARGET_INCLUDE_DIRECTORIES(libdriver_static PUBLIC include)
TARGET_LINK_LIBRARIES(libdriver_static libkpi_static)
INSTALL(TARGETS libdriver_static DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)