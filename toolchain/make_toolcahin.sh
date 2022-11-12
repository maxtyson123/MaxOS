#Install Dependency's
sudo apt-get install bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo build-essential

mkdir src
cd src

#Download the source
wget http://ftp.gnu.org/gnu/gcc/gcc-4.9.2/gcc-4.9.2.tar.bz2
tar -xvf gcc-4.9.2.tar.bz2

wget http://ftp.gnu.org/gnu/binutils/binutils-2.24.tar.bz2
tar -xvf binutils-2.24.tar.bz2

#Setup Env variables
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"


#Build binutils
mkdir build-binutils
cd build-binutils
../binutils-2.24/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

cd ../


# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is not in the PATH

# Build GCC

make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc