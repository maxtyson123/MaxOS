# NOTE: Cant use MAKE_LIBRARY() as dependency of libc

# Set compiler flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -mcmodel=kernel -ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -Wall -Wextra -mno-red-zone -mno-80387 -mno-mmx -fno-use-cxa-atexit")

# Disable noisy warnings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pointer-arith -Wno-address-of-packed-member -Wno-trigraphs -Wno-unused-variable -Wno-unused-function")

# Collect sources
FILE(GLOB_RECURSE KPI_SRCS src/*.cpp src/*.s)

# Build as a static library
ADD_LIBRARY(kpi STATIC ${KPI_SRCS})
TARGET_INCLUDE_DIRECTORIES(kpi PUBLIC include)
TARGET_LINK_LIBRARIES(kpi PUBLIC common)
INSTALL(TARGETS kpi DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)