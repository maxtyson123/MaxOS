# Set the minimum version of CMake that can be used
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)

# Set the project name and the languages
project(MaxOS C CXX ASM)

# Logs the compiler commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dont enable runtime type information
set(CMAKE_SKIP_RPATH TRUE)

# Set the standard to C++17
set(CMAKE_CXX_STANDARD 17)

# Set where the libraries are located
INCLUDE_DIRECTORIES(libraries/)

# Tell the linker to look in the toolchain for libraries
LINK_DIRECTORIES(${TOOLCHAIN_ROOT_DIR}/lib/gcc/${TOOLCHAIN_PLATFORM}/${GCC_VER}/)
LINK_DIRECTORIES(${TOOLCHAIN_ROOT_DIR}/${TOOLCHAIN_PLATFORM}/lib/)

# Look to build in these directories
ADD_SUBDIRECTORY(kernel)
#ADD_SUBDIRECTORY(libraries)
#ADD_SUBDIRECTORY(programs)
#ADD_SUBDIRECTORY(ports)

# TODO: Build disk image
# TODO: Run qemu