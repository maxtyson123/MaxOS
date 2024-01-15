# Set the standard and flags
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS " -ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -Wall -Wextra -Wno-address-of-packed-member -Wno-trigraphs -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel -g")

# Enable assembly
SET(CMAKE_ASM_FLAGS "-f elf64")
SET(CMAKE_ASM_COMPILE_OBJECT "nasm <FLAGS> -o <OBJECT> <SOURCE>")
FILE(GLOB_RECURSE ASM_SRCS src/*.s)
SET_SOURCE_FILES_PROPERTIES(${ASM_SRCS} PROPERTIES LANGUAGE ASM)

# Find all the cpp and s files in the src directory (recursive)
FILE(GLOB_RECURSE KERNEL_SRCS src/*.cpp src/*.s)

# Create the kernel
ADD_EXECUTABLE(MaxOSk64 ${KERNEL_SRCS})
TARGET_COMPILE_DEFINITIONS(MaxOSk64 PUBLIC MAXOS_KERNEL)

# Update the version before building
ADD_CUSTOM_COMMAND(
    COMMENT "Versioning kernel"
    TARGET MaxOSk64
    PRE_BUILD
    COMMAND ${CMAKE_SOURCE_DIR}/toolchain/version.sh
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/kernel/include/common/version.h.tmp ${CMAKE_SOURCE_DIR}/kernel/include/common/version.h
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_SOURCE_DIR}/kernel/include/common/version.h.tmp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Linker
SET(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/src/linker.ld)
SET_TARGET_PROPERTIES(MaxOSk64 PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
TARGET_LINK_LIBRARIES(MaxOSk64 gcc)
TARGET_LINK_OPTIONS(MaxOSk64 PRIVATE -T ${LINKER_SCRIPT} -nostdlib -n)

# Set the include directories
TARGET_INCLUDE_DIRECTORIES(MaxOSk64 PRIVATE include)

# Install kernel
INSTALL(TARGETS MaxOSk64 DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/boot)
