#TODO: Make func to do this shit

# Set compiler flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -Wall -Wextra -mno-red-zone -mno-80387 -mno-mmx -fno-use-cxa-atexit")

# Disable noisy warnings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pointer-arith -Wno-address-of-packed-member -Wno-trigraphs -Wno-unused-variable -Wno-unused-function")

# Collect sources
FILE(GLOB_RECURSE SYSTEM_SRCS src/*.cpp src/*.s)

# Build as a dynamic library
ADD_LIBRARY(driver SHARED ${SYSTEM_SRCS})
TARGET_INCLUDE_DIRECTORIES(driver PUBLIC include)
TARGET_LINK_LIBRARIES(driver kpi)
INSTALL(TARGETS driver LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)

# Build as a static library
ADD_LIBRARY(driver_static STATIC ${SYSTEM_SRCS})
TARGET_INCLUDE_DIRECTORIES(driver_static PUBLIC include)
TARGET_LINK_LIBRARIES(driver_static kpi)
INSTALL(TARGETS driver_static DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)