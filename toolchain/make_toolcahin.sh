#Install Dependency's
sudo apt-get install bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo build-essential

mkdir src
cd src

#Download the source
wget https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.xz
tar -xvf gcc-12.2.0.tar.xz

wget https://ftp.gnu.org/gnu/binutils/binutils-2.39.tar.xz
tar -xvf binutils-2.39.tar.xz

#Setup Env variables
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"


#Build binutils
mkdir build-binutils
cd build-binutils
../binutils-2.39/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
cd ../


# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is not in the PATH

# Build GCC
mkdir build-gcc
cd build-gcc
../gcc-12.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc