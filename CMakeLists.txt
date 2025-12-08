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

# Function to make a library easier
FUNCTION(MAKE_LIBRARY LIBNAME)

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

ENDFUNCTION()

FUNCTION(MAKE_LIBRARY_NOSTD LIBNAME LIBRARY_SRCS)

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -Wall -Wextra -mno-red-zone -mno-80387 -mno-mmx -fno-use-cxa-atexit")

    # Disable noisy warnings
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-pointer-arith -Wno-address-of-packed-member -Wno-trigraphs -Wno-unused-variable -Wno-unused-function")

    # Find all .cpp and .s files recursively in the given source directory
    FILE(GLOB_RECURSE PROJ_SRCS ${LIBRARY_SRCS}/*.cpp ${LIBRARY_SRCS}/*.s)

    # Build as a static library
    ADD_LIBRARY(${LIBNAME}_static STATIC ${PROJ_SRCS})
    TARGET_INCLUDE_DIRECTORIES(${LIBNAME}_static PUBLIC ${CMAKE_SOURCE_DIR}/${LIBRARY_SRCS}/include)
    INSTALL(TARGETS ${LIBNAME}_static DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/os/lib)
ENDFUNCTION()

# Function to make a program easier
FUNCTION(MAKE_PROG_NOSTD TARGET_NAME SRC_DIR)

    # Set compiler flags freestanding no stdlib
    SET(CMAKE_CXX_FLAGS "-ffreestanding -fno-exceptions -fno-rtti -fpermissive -nostdlib -Wall -Wextra -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mcmodel=kernel -g -fno-use-cxa-atexit" CACHE STRING "" FORCE)

    # Find all .cpp and .s files recursively in the given source directory
    FILE(GLOB_RECURSE PROJ_SRCS ${SRC_DIR}/*.cpp ${SRC_DIR}/*.s)

    # Create the executable
    ADD_EXECUTABLE(${TARGET_NAME}.elf ${PROJ_SRCS})

    # Set the linker script path
    SET(LINKER_SCRIPT ${SRC_DIR}/linker.ld)

    # Ensure the target rebuilds if the linker script changes
    SET_TARGET_PROPERTIES(${TARGET_NAME}.elf PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

    # Link libraries and pass linker options
    TARGET_LINK_LIBRARIES(${TARGET_NAME}.elf gcc libkpi_static)
    TARGET_COMPILE_OPTIONS(${TARGET_NAME}.elf PRIVATE -T ${LINKER_SCRIPT} -nostdlib -n)
    TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME}.elf PRIVATE include)

    # Install the executable to the boot filesystem
    INSTALL(TARGETS ${TARGET_NAME}.elf DESTINATION ${CMAKE_SOURCE_DIR}/filesystem/boot)
ENDFUNCTION()

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