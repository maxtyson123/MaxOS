# Set the minimum version of CMake that can be used
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)

# Set the project name and the languages
PROJECT(MaxOS C CXX ASM)

# Logs the compiler commands
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# don't enable runtime type information
SET(CMAKE_SKIP_RPATH TRUE)

# Set the standard to C++20
SET(CMAKE_CXX_STANDARD 20)

# Set where the libraries are located
INCLUDE_DIRECTORIES(libraries/)

# Tell the linker to look in the toolchain for libraries
LINK_DIRECTORIES(${TOOLCHAIN_ROOT_DIR}/lib/gcc/${TOOLCHAIN_PLATFORM}/${GCC_VER}/)
LINK_DIRECTORIES(${TOOLCHAIN_ROOT_DIR}/${TOOLCHAIN_PLATFORM}/lib/)

# DEBUG / PROD
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
ENDIF()

# Set flags based on build type
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-DTARGET_DEBUG)
ENDIF()

# Function to make a library easier
function(MAKE_LIBRARY LIBNAME)

    # TODO: ADD_DEPENDENCIES(${LIBNAME} libc) (& for static)

    # Install dynamic library
    ADD_LIBRARY(${LIBNAME} SHARED ${LIBRARY_SRCS})
    TARGET_INCLUDE_DIRECTORIES(${LIBNAME} PUBLIC include)
    INSTALL(TARGETS ${LIBNAME} LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)

    # Install static library
    ADD_LIBRARY(${LIBNAME}_static STATIC ${LIBRARY_SRCS})
    TARGET_INCLUDE_DIRECTORIES(${LIBNAME}_static PUBLIC include)
    INSTALL(TARGETS ${LIBNAME}_static DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)

    # TODO: Install headers onto fs

endfunction()

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