# Ignore cross compiler
LIST(FILTER SOURCES EXCLUDE REGEX ".*/toolchain/cross_compiler.*")

# Setup CMAKE
CMAKE_MINIMUM_REQUIRED(VERSION 3.5.0)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_SKIP_RPATH TRUE)
SET(CMAKE_CXX_STANDARD 20)
PROJECT(MaxOS C CXX ASM)

# Setup LibC
SET(LIBC_TARGET "example")

# Set up libraries
INCLUDE_DIRECTORIES(libraries/)
LINK_DIRECTORIES(${TOOLCHAIN_ROOT_DIR}/lib/gcc/${TOOLCHAIN_PLATFORM}/${GCC_VER}/)
LINK_DIRECTORIES(${TOOLCHAIN_ROOT_DIR}/${TOOLCHAIN_PLATFORM}/lib/)

# DEBUG / PROD
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
ENDIF()

# Generate RPC stubs when first setup
SET(RPC_SCRIPT "${CMAKE_SOURCE_DIR}/toolchain/pre_process/rpc.py")
EXECUTE_PROCESS(COMMAND ${RPC_SCRIPT} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# For manual RPC gen when changing IDL files
ADD_CUSTOM_TARGET(rpc
    COMMENT "Generating RPC stubs"
    COMMAND ${CMAKE_COMMAND} -E env "SOURCE_DIR=${CMAKE_SOURCE_DIR}" python3 ${RPC_SCRIPT}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    USES_TERMINAL
)

# Function to add all subdirectories with CMakeLists.txt in a given base directory
FUNCTION(ADD_SUB_DIRS BASE_DIR)
    IF(NOT BASE_DIR)
        SET(BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    ENDIF()

    FILE(GLOB CHILD_DIRS RELATIVE ${BASE_DIR} ${BASE_DIR}/*)

    FOREACH (dir ${CHILD_DIRS})
        IF(IS_DIRECTORY ${BASE_DIR}/${dir})
            IF(EXISTS ${BASE_DIR}/${dir}/CMakeLists.txt)
                add_subdirectory(${BASE_DIR}/${dir} ${dir})
            ENDIF()
        ENDIF()
    ENDFOREACH()
ENDFUNCTION()

# Look to build in these directories
ADD_SUBDIRECTORY(kernel)
ADD_SUBDIRECTORY(libraries)
ADD_SUBDIRECTORY(programs)

ADD_CUSTOM_TARGET(image
        # Everything needs to be installed (compiled) before we can create the disk image
        # DEPENDS install (TODO Rewrite so auto installs)

        # Create the disk image
        COMMENT "Creating disk image"
        COMMAND ${CMAKE_COMMAND} -E env "SOURCE_DIR=${CMAKE_SOURCE_DIR}" ${CMAKE_SOURCE_DIR}/toolchain/create_disk_img.sh
        BYPRODUCTS ${CMAKE_SOURCE_DIR}/MaxOS.img
        BYPRODUCTS ${CMAKE_SOURCE_DIR}/filesystem/boot/MaxOSk64
        BYPRODUCTS ${CMAKE_SOURCE_DIR}/filesystem/boot/test.elf


        # Copy the files to the disk image
        COMMENT "Copying files to disk image"
        COMMAND ${CMAKE_COMMAND} -E env "SOURCE_DIR=${CMAKE_SOURCE_DIR}" ${CMAKE_SOURCE_DIR}/toolchain/copy_filesystem.sh

        # Ensure it runs in the terminal
        USES_TERMINAL
)

ADD_CUSTOM_TARGET(run
        # Requires a bootable image to be created
        DEPENDS image

        # Run qemu
        COMMENT "Running qemu"
        COMMAND ${CMAKE_COMMAND} -E env "SOURCE_DIR=${CMAKE_SOURCE_DIR}" ${CMAKE_SOURCE_DIR}/toolchain/run_qemu.sh

        # Ensure it runs in the terminal
        USES_TERMINAL
)

# Run qemu with debug if the debug flag is set
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_CUSTOM_TARGET(gdb

        # Requires a bootable image to be created
        DEPENDS image

        # Run qemu
        COMMENT "Running qemu with gdb"
        COMMAND ${CMAKE_COMMAND} -E env "SOURCE_DIR=${CMAKE_SOURCE_DIR}" ${CMAKE_SOURCE_DIR}/toolchain/run_qemu.sh --debug

        # Ensure it runs in the terminal
        USES_TERMINAL
    )
ENDIF()