# Set the standard and flags
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "-ffreestanding -fno-exceptions -fno-rtti -nostdlib -Wall -Wextra")

# Enable assembly
SET(CMAKE_ASM_FLAGS "")
SET(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>")
FILE(GLOB_RECURSE ASM_SRCS src/*.s)
SET_SOURCE_FILES_PROPERTIES(${ASM_SRCS} PROPERTIES LANGUAGE ASM)

# Find all the cpp and s files in the src directory (recursive)
FILE(GLOB_RECURSE KERNEL_SRCS src/*.cpp src/*.s)

# Create the kernel
ADD_EXECUTABLE(kernel ${KERNEL_SRCS})
TARGET_COMPILE_DEFINITIONS(kernel PUBLIC MAXOS_KERNEL)

# Linker
SET(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/src/linker.ld)
SET_TARGET_PROPERTIES(kernel PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
TARGET_LINK_LIBRARIES(kernel gcc)
TARGET_LINK_OPTIONS(kernel PRIVATE -T ${LINKER_SCRIPT} -nostdlib)

# Set the include directories
TARGET_INCLUDE_DIRECTORIES(kernel PRIVATE include)

# Install kernel
INSTALL(TARGETS kernel DESTINATION ${CMAKE_SOURCE_DIR})