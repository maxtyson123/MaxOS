# Set the flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -mno-red-zone -mno-80387 -mno-mmx -mcmodel=kernel -fno-use-cxa-atexit -m64 -mno-sse -mno-sse2 -mno-sse3 -mno-sse4 -mno-avx -mno-avx2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-pointer-arith -Wno-address-of-packed-member -Wno-trigraphs -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-parameter") # Warnings
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fstack-protector-all -g") # Debug -fsanitize=kernel-address
ENDIF()

# Enable assembly
SET(CMAKE_ASM_FLAGS "-f elf64")
SET(CMAKE_ASM_COMPILE_OBJECT "nasm <FLAGS> -o <OBJECT> <SOURCE>")
FILE(GLOB_RECURSE ASM_SRCS src/*.s)
SET_SOURCE_FILES_PROPERTIES(${ASM_SRCS} PROPERTIES LANGUAGE ASM)

# Find all the cpp and s files in the src directory (recursive)
FILE(GLOB_RECURSE KERNEL_SRCS src/*.cpp src/*.s)

# Remove the tests if not in Debug mode
IF(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    LIST(FILTER KERNEL_SRCS EXCLUDE REGEX "src/tests/.*")
ENDIF()

# Update the version before building
SET(VERSION_HEADER       "${CMAKE_SOURCE_DIR}/kernel/include/common/version.h")
SET(VERSION_HEADER_TMP   "${CMAKE_SOURCE_DIR}/kernel/include/common/version.h.tmp")
SET(POST_PROCESS_SCRIPT  "${CMAKE_SOURCE_DIR}/toolchain/pre_process/run.sh")
ADD_CUSTOM_COMMAND(
        OUTPUT      ${VERSION_HEADER}
        COMMAND     ${POST_PROCESS_SCRIPT}
        COMMAND     ${CMAKE_COMMAND} -E copy_if_different "${VERSION_HEADER_TMP}" "${VERSION_HEADER}"
        COMMAND     ${CMAKE_COMMAND} -E remove "${VERSION_HEADER_TMP}"
        DEPENDS     ${KERNEL_SRCS}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT     "Regenerating version.h because kernel sources changed"
)
ADD_CUSTOM_TARGET(VersionScript  DEPENDS "${VERSION_HEADER}")
LIST(APPEND KERNEL_SRCS ${VERSION_HEADER})

# Kernel Build function
FUNCTION(BUILD_KERNEL target_name)

    # Create the kernel
    ADD_EXECUTABLE(${target_name} ${KERNEL_SRCS})
    ADD_DEPENDENCIES(${target_name} VersionScript)
    TARGET_COMPILE_DEFINITIONS(${target_name} PUBLIC MAXOS_KERNEL)

    # Linker
    SET(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/src/linker.ld)
    SET_TARGET_PROPERTIES(${target_name} PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

    # Set the include directories
    TARGET_LINK_LIBRARIES(${target_name} PRIVATE gcc libkpi_static libcommon_kernel)
    target_link_options(${target_name} PRIVATE -T ${LINKER_SCRIPT} -nostdlib -n)
    TARGET_INCLUDE_DIRECTORIES(${target_name} PRIVATE include)
endfunction()

# Must have a build to extract the symbols from
BUILD_KERNEL(MaxOSk64_nosym)

# Generate symbols
SET(SYMBOLS_HEADER "${CMAKE_SOURCE_DIR}/kernel/include/common/symbols.h")
SET(SYMBOLS_HEADER_TMP "${CMAKE_SOURCE_DIR}/kernel/include/common/symbols.h.tmp")
SET(SYMBOLS_HEADER_DEP "${CMAKE_BINARY_DIR}/generated/symbols.dep")
SET(GEN_SYM_SCRIPT "${CMAKE_SOURCE_DIR}/toolchain/pre_process/symbols.sh")
SET(OBJCOPY "${CMAKE_SOURCE_DIR}/toolchain/cross_compiler/cross/bin/x86_64-elf-objcopy")
SET(SYM_ELF "${CMAKE_BINARY_DIR}/MaxOS.sym")
ADD_CUSTOM_COMMAND(
        OUTPUT      ${SYMBOLS_HEADER_DEP}
        COMMAND     ${OBJCOPY} --only-keep-debug $<TARGET_FILE:MaxOSk64_nosym> ${SYM_ELF}
        COMMAND     ${GEN_SYM_SCRIPT} ${SYM_ELF} ${SYMBOLS_HEADER_TMP}
        COMMAND     ${CMAKE_COMMAND} -E copy_if_different "${SYMBOLS_HEADER_TMP}" "${SYMBOLS_HEADER}"
        COMMAND     ${CMAKE_COMMAND} -E remove "${SYMBOLS_HEADER_TMP}"
        DEPENDS     MaxOSk64_nosym ${GEN_SYM_SCRIPT}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT     "Generating symbols.h from ${SYM_ELF}"
)
ADD_CUSTOM_TARGET(GenerateSymbols DEPENDS ${SYMBOLS_HEADER_DEP})
ADD_DEPENDENCIES(GenerateSymbols MaxOSk64_nosym)

# Build final kernel
BUILD_KERNEL(MaxOSk64)
ADD_DEPENDENCIES(MaxOSk64 GenerateSymbols)

# Install kernel
INSTALL(TARGETS MaxOSk64 DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/boot)