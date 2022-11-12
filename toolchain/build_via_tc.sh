#Setup Env variables
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

#Test
$HOME/opt/cross/bin/$TARGET-gcc --version

#Run the makefile
cd ../
make maxOS.iso GCC_EXEC=$HOME/opt/cross/bin/$TARGET-gcc
