# NOTE: Cant use MAKE_LIBRARY() as dependency of libc

# Set compiler flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -Wall -Wextra -mno-red-zone -mno-80387 -mno-mmx -fno-use-cxa-atexit")

# Disable noisy warnings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pointer-arith -Wno-address-of-packed-member -Wno-trigraphs -Wno-unused-variable -Wno-unused-function")

# Collect sources
FILE(GLOB_RECURSE IPC_SRCS src/*.cpp src/*.s)

# Build as a dynamic library
ADD_LIBRARY(ipc SHARED ${IPC_SRCS})
TARGET_INCLUDE_DIRECTORIES(ipc PUBLIC include)
TARGET_LINK_LIBRARIES(ipc PUBLIC system_static) # Has to be free standing
INSTALL(TARGETS ipc LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)

# Build as a static library
ADD_LIBRARY(ipc_static STATIC ${IPC_SRCS})
TARGET_INCLUDE_DIRECTORIES(ipc_static PUBLIC include)
TARGET_LINK_LIBRARIES(ipc_static PUBLIC system_static)
INSTALL(TARGETS ipc_static DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)