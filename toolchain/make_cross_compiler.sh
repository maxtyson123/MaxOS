#!/bin/bash
source ./MaxOS.sh

# Install Dependencies if not told otherwise
if [ "$1" != "--no-deps" ]; then
    msg "Installing extra dependencies"
    sudo apt update
    sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo libisl-dev cmake nasm
fi

# Make A Directory For The Cross Compiler
mkdir ./cross_compiler
cd ./cross_compiler

# Prefix should be a bin directory in the current directory
export PREFIX="$PWD/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

# Store versions in a variable
BINUTILS_VERSION=2.39
GCC_VERSION=12.2.0

# Print what we are doing
msg "Installing binutils-$BINUTILS_VERSION and gcc-$GCC_VERSION for $TARGET to $PREFIX"

# == Build Binutils ==

# Download Binutils if not already downloaded
if [ ! -f binutils-$BINUTILS_VERSION.tar.gz ]; then
    msg "Downloading binutils-$BINUTILS_VERSION"
    wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz || fail "Couldn't download binutils-$BINUTILS_VERSION"
    tar xf binutils-$BINUTILS_VERSION.tar.gz
fi


# Configure binutils
msg "Configuring binutils-$BINUTILS_VERSION"
mkdir build-binutils
cd build-binutils
../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror || fail "Configuring binutils failed"

# Build binutils
msg "Building binutils-$BINUTILS_VERSION"
make            || fail "Building binutils failed"
make install    || fail "Installing binutils to $PREFIX failed"
cd ../

# == Build GCC ==

# Download GCC if not already downloaded
if [ ! -f gcc-$GCC_VERSION.tar.gz ]; then
    msg "Downloading gcc-$GCC_VERSION"
    wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz || fail "Couldn't download gcc-$GCC_VERSION"
    tar xf gcc-$GCC_VERSION.tar.gz
fi

# Configure GCC
msg "Configuring gcc-$GCC_VERSION"
mkdir build-gcc
cd build-gcc
../gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers  || fail "Configuring gcc failed"

# Build GCC
msg "Building gcc-$GCC_VERSION"
make all-gcc                  || fail "Building gcc failed"
make all-target-libgcc        || fail "Building libgcc failed"
make install-gcc              || fail "Installing gcc failed"
make install-target-libgcc    || fail "Installing libgcc failed"

#  Leave the build directory
cd ../../

# Make a  build directory for cmake
mkdir ../cmake-build

# Setup the first version of the kernel
./version.sh --force