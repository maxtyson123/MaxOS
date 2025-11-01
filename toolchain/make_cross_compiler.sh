#!/bin/bash
source ./MaxOS.sh

# Install Dependencies if not told otherwise
if [ "$1" != "--no-deps" ]; then
    msg "Installing extra dependencies"

    # If we are on MacOS, install dependencies using brew
    if [ "$IS_MACOS" -eq 1 ]; then
        msg "Installing dependencies using brew"
        brew install  coreutils\
                      bison \
                      gmp \
                      libmpc \
                      mpfr \
                      texinfo \
                      isl \
                      cmake \
                      nasm \
                      telnet \
                      || fail "Couldn't install dependencies"

        # Scripting tools that linux has by default
        brew install wget gcc@13 gnu-sed
    else
        msg "Installing dependencies using apt"
        sudo apt update
        sudo apt install -y build-essential \
                            bison \
                            libgmp3-dev \
                            libmpc-dev \
                            libmpfr-dev \
                            texinfo \
                            libisl-dev \
                            cmake \
                            nasm \
                            telnet \
                            rsync \
            || fail "Couldn't install dependencies"
    fi


fi

# Make A Directory For The Cross Compiler
mkdir ./cross_compiler
cd ./cross_compiler

# Prefix should be a bin directory in the current directory
export PREFIX="$PWD/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"
export SYSROOT="$PREFIX/$TARGET/sysroot"

# Don't include debug info in toolchain, optimize TODO: Mac M chip doesn't support -mtune=native causing clang to fail
#export CFLAGS="-g0 -O2 "
#export CXXFLAGS="-g0 -O2 "

# Mac Os likes to use its own gcc which fails to build the cross compiler
if [ "$IS_MACOS" -eq 1 ]; then
      export CPATH=/opt/homebrew/include:/Library/Developer/CommandLineTools/SDKs/MacOSX15.2.sdk/usr/include/
      export LIBRARY_PATH=/opt/homebrew/lib:/Library/Developer/CommandLineTools/SDKs/MacOSX15.2.sdk/usr/include/
      export CC=gcc-13
      export CXX=g++-13
      export CPP=cpp-13
      export LD=gcc-13
fi

# Configure the build
FTP_MIRROR="https://ftp.wayne.edu" # "https://ftp.gnu.org" <-- official but sometimes slow
BINUTILS_VERSION=2.41
GCC_VERSION=13.2.0
NUM_JOBS=$(nproc)

# Create the sysroot directory
mkdir -p "$SYSROOT"
msg "Installing binutils-$BINUTILS_VERSION and gcc-$GCC_VERSION for $TARGET to $PREFIX"

# == Build Binutils ==

# Download Binutils if not already downloaded
if [ ! -f binutils-$BINUTILS_VERSION.tar.gz ]; then
    msg "Downloading binutils-$BINUTILS_VERSION"
    wget $FTP_MIRROR/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz || fail "Couldn't download binutils-$BINUTILS_VERSION"
    tar xf binutils-$BINUTILS_VERSION.tar.gz
fi


# Configure binutils
msg "Configuring binutils-$BINUTILS_VERSION"
mkdir build-binutils
cd build-binutils
../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror || fail "Configuring binutils failed"

# Build binutils
msg "Building binutils-$BINUTILS_VERSION"
make -j "$NUM_JOBS"       || fail "Building binutils failed"
make install              || fail "Installing binutils to $PREFIX failed"
cd ../

# == Build GCC ==

# Download GCC if not already downloaded
if [ ! -f gcc-$GCC_VERSION.tar.gz ]; then
    msg "Downloading gcc-$GCC_VERSION"
    wget $FTP_MIRROR/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz || fail "Couldn't download gcc-$GCC_VERSION"
    tar xf gcc-$GCC_VERSION.tar.gz
fi

# Configure GCC
msg "Configuring gcc-$GCC_VERSION"
mkdir build-gcc
cd build-gcc
extra_config_args=()
if [ "$IS_MACOS" -eq 1 ]; then
  for library in gmp mpfr mpc; do
	  [ "$library" = "mpc" ] && brew_formula="libmpc" || brew_formula="$library"
	  extra_config_args+=("--with-$library=$(brew --prefix --installed "$brew_formula")")
	done
fi

../gcc-$GCC_VERSION/configure --prefix="$PREFIX" --target="$TARGET" --disable-nls --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx "${extra_config_args[@]}" || fail "Configuring gcc failed"

# Build GCC
msg "Building gcc-$GCC_VERSION"
make all-gcc -j "$NUM_JOBS"                 || fail "Building gcc failed"
make all-target-libgcc -j "$NUM_JOBS"       || fail "Building libgcc failed"
make all-target-libstdc++-v3 -j "$NUM_JOBS" || fail "Building libstdc++ failed"
make install-gcc                            || fail "Installing gcc failed"
make install-target-libgcc                  || fail "Installing libgcc failed"
make install-target-libstdc++-v3            || fail "Installing libstdc++ failed"
cd ../


# Build grub manually on Mac OS only
if [ "$IS_MACOS" -eq 1 ]; then

    brew install i686-elf-grub


#    msg "Installing grub dependencies"
#    brew install automake autoconf gettext pkg-config objconv
#
#    # Setup where the cross compiler is
#    export PATH="/opt/homebrew/opt/flex/bin:$SCRIPTDIR/../cross/bin:$PATH"
#    export CC=$TARGET-gcc
#    export CPP=$TARGET-cpp
#    export LD=$TARGET-ld
#    export AR=$TARGET-ar
#    export AS=$TARGET-as
#    export RANLIB=$TARGET-ranlib
#
#
#    msg "Downloading grub"
#    git clone git://git.savannah.gnu.org/grub.git
#    cd grub
#    git checkout 77063f4cb672f423272db7e21ca448cf3de98dcf || fail "Couldn't checkout grub revision"
#    ./autogen.sh || fail "Couldn't run autogen.sh"
#    cd ../
#
#    msg "Configuring grub"
#    mkdir build-grub
#    cd build-grub
#    ../grub/configure --prefix="$PREFIX" --target="$TARGET" --disable-werror || fail "Configuring grub failed"
#
#    # Build grub
#    msg "Building grub"
#    make -j "$NUM_JOBS"       || fail "Building grub failed"
#    make install              || fail "Installing grub to $PREFIX failed"
fi

#  Leave the build directory
cd ../

ls

# Setup the first version of the kernel
cd pre_process
./version.sh --force
./symbols.sh pre ../../kernel/include/common/symbols.h
cd ../