# Set the flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -Wall -Wextra  -mno-red-zone -mno-80387 -mno-mmx -mcmodel=kernel -fno-use-cxa-atexit")

# Disable certain warnings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pointer-arith -Wno-address-of-packed-member -Wno-trigraphs -Wno-unused-variable -Wno-unused-function")

# Enable assembly
SET(CMAKE_ASM_FLAGS "-f elf64")
SET(CMAKE_ASM_COMPILE_OBJECT "nasm <FLAGS> -o <OBJECT> <SOURCE>")
FILE(GLOB_RECURSE ASM_SRCS src/*.s)
SET_SOURCE_FILES_PROPERTIES(${ASM_SRCS} PROPERTIES LANGUAGE ASM)

# Find all the cpp and s files in the src directory (recursive)
FILE(GLOB_RECURSE KERNEL_SRCS src/*.cpp src/*.s)

# Update the version before building
SET(VERSION_HEADER       "${CMAKE_SOURCE_DIR}/kernel/include/common/version.h")
SET(VERSION_HEADER_TMP   "${CMAKE_SOURCE_DIR}/kernel/include/common/version.h.tmp")
SET(POST_PROCESS_SCRIPT  "${CMAKE_SOURCE_DIR}/toolchain/pre_process/run.sh")
ADD_CUSTOM_COMMAND(
        OUTPUT      "${VERSION_HEADER}"
        COMMAND     "${POST_PROCESS_SCRIPT}"
        COMMAND     ${CMAKE_COMMAND} -E copy_if_different "${VERSION_HEADER_TMP}" "${VERSION_HEADER}"
        COMMAND     ${CMAKE_COMMAND} -E remove "${VERSION_HEADER_TMP}"
        DEPENDS     ${KERNEL_SRCS}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT     "Regenerating version.h because kernel sources changed"
)
ADD_CUSTOM_TARGET(VersionScript  DEPENDS "${VERSION_HEADER}")

# Create the kernel
ADD_EXECUTABLE(MaxOSk64 ${KERNEL_SRCS} ${VERSION_HEADER})
ADD_DEPENDENCIES(MaxOSk64 VersionScript)
TARGET_COMPILE_DEFINITIONS(MaxOSk64 PUBLIC MAXOS_KERNEL)

# Linker
SET(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/src/linker.ld)
SET_TARGET_PROPERTIES(MaxOSk64 PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
TARGET_LINK_LIBRARIES(MaxOSk64 PRIVATE gcc syscore_static)
TARGET_LINK_OPTIONS(MaxOSk64 PRIVATE -T ${LINKER_SCRIPT} -nostdlib -n)

# Set the include directories
TARGET_INCLUDE_DIRECTORIES(MaxOSk64 PRIVATE include)

# Install kernel
INSTALL(TARGETS MaxOSk64 DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/boot)