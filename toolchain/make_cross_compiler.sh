# Install Dependencies
sudo apt install -y build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo libisl-dev

# Make A Directory For The Cross Compiler
mkdir -p ./cross_compiler
cd ./cross_compiler

# Export the paths
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# Store versions in a variable
BINUTILS_VERSION=2.39
GCC_VERSION=12.2.0

# == Build Binutils ==

# Download Binutils if not already downloaded
if [ ! -f binutils-$BINUTILS_VERSION.tar.gz ]; then
    wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz
    tar xf binutils-$BINUTILS_VERSION.tar.gz
fi

# Configure binutils
mkdir build-binutils
cd build-binutils
../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror

# Build binutils
make
make install
cd ../

# == Build GCC ==
which -- $TARGET-as || echo $TARGET-as is not in the PATH

# Download GCC if not already downloaded
if [ ! -f gcc-$GCC_VERSION.tar.gz ]; then
    wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz
    tar xf gcc-$GCC_VERSION.tar.gz
fi

# Configure GCC
mkdir build-gcc
cd build-gcc
../gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers

# Build GCC
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

# Test
$HOME/opt/cross/bin/$TARGET-gcc --version