# Set the  flags
SET(CMAKE_CXX_FLAGS " -ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -Wall -Wextra  -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel -g -fno-use-cxa-atexit")

# Find all the cpp and s files in the src directory (recursive)
FILE(GLOB_RECURSE PROJ_SRCS src/*.cpp src/*.s)

# Create the executable
ADD_EXECUTABLE(test1.elf ${PROJ_SRCS})

# Set the linker info
SET(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/programs/Example/src/linker.ld)
SET_TARGET_PROPERTIES(test1.elf PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
TARGET_LINK_LIBRARIES(test1.elf gcc libkpi_static)
TARGET_LINK_OPTIONS(test1.elf PRIVATE -T ${LINKER_SCRIPT} -nostdlib -n)

# Install the executable
INSTALL(TARGETS test1.elf DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/boot)

