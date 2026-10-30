#!/bin/sh

# CREDIT/SOURCE: https://github.com/alperakcan/fuse-ext2
# Todo clean build to test

export PATH=/opt/gnu/bin:$PATH
export PKG_CONFIG_PATH=/opt/gnu/lib/pkgconfig:/opt/homebrew/lib/pkgconfig:/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH


SCRIPTDIR=$(dirname "$BASH_SOURCE")
source $SCRIPTDIR/../MaxOS.sh

mkdir fuse-ext2.build
cp  Working_with_latest_macfuse.patch fuse-ext2.build/Working_with_latest_macfuse.patch
cd fuse-ext2.build



if [ ! -d fuse-ext2 ]; then
    git clone https://github.com/alperakcan/fuse-ext2.git
    cp  Working_with_latest_macfuse.patch fuse-ext2/Working_with_latest_macfuse.patch
    cd fuse-ext2
    git apply Working_with_latest_macfuse.patch || fail "Coulnt apply fix"
    cd ../
fi

# m4
if [ ! -f m4-1.4.17.tar.gz ]; then
    curl -O -L http://ftp.gnu.org/gnu/m4/m4-1.4.17.tar.gz
    tar -zxvf m4-1.4.17.tar.gz
fi
if [ ! -f /opt/gnu/bin/m4 ]; then
    cd m4-1.4.17
    ./configure --prefix=/opt/gnu
    make -j 8 || fail "Failed to build m4"
    sudo make install -j 8 || fail "Failed to install m4"
    cd ../
fi

# autoconf
if [ ! -f autoconf-2.69.tar.gz ]; then
    curl -O -L http://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz
    tar -zxvf autoconf-2.69.tar.gz
fi
if [ ! -f /opt/gnu/bin/autoconf ]; then
    cd autoconf-2.69
    ./configure --prefix=/opt/gnu
    make || fail "Failed to build autoconf"
    cd ../
fi

# automake
if [ ! -f automake-1.15.tar.gz ]; then
    curl -O -L http://ftp.gnu.org/gnu/automake/automake-1.15.tar.gz
    tar -zxvf automake-1.15.tar.gz
fi
if [ ! -f /opt/gnu/bin/automake ]; then
    cd automake-1.15
    ./configure --prefix=/opt/gnu

    make -j 8 || fail "Failed to build automake"
    sudo make install -j 8 || fail "Failed to install automake"
    cd ../
fi

# libtool
if [ ! -f libtool-2.4.6.tar.gz ]; then
    curl -O -L http://ftpmirror.gnu.org/libtool/libtool-2.4.6.tar.gz
    tar -zxvf libtool-2.4.6.tar.gz
fi
if [ ! -f /opt/gnu/bin/libtool ]; then
    cd libtool-2.4.6
    ./configure --prefix=/opt/gnu
    make -j 8 || fail "Failed to make libtoo"
    sudo make install -j 8 || fail "Failed to make libtool"
    cd ../
fi

# e2fsprogs
if [ ! -f e2fsprogs-1.43.4.tar.gz ]; then
    curl -O -L https://www.kernel.org/pub/linux/kernel/people/tytso/e2fsprogs/v1.43.4/e2fsprogs-1.43.4.tar.gz
    tar -zxvf e2fsprogs-1.43.4.tar.gz
fi
if [ ! -f /opt/gnu/lib/libext2fs.a ]; then
    cd e2fsprogs-1.43.4
    ./configure --prefix=/opt/gnu --disable-nls
    make -j 8 || fail "Failed to make e2fsprogs"
    sudo make install -j 8 || fail "Failed to isntall e2fsprogs"
    sudo make install-libs -j 8 || fail "Failed to instal e2fsprobs libs"
    sudo cp /opt/gnu/lib/pkgconfig/* /usr/local/lib/pkgconfig
    cd ../
fi

# fuse-ext2
mkdir -p /tmp/wrapper-bin

# Seems to want to use MFMount which doesnt exist on the latest macos so remove that 
cat << 'EOF' > /tmp/wrapper-bin/gcc
#!/bin/bash
args=()
while [[ $# -gt 0 ]]; do
    if [[ "$1" == "-framework" && "$2" == "MFMount" ]]; then
        shift 2
    elif [[ "$1" == "-arch" && "$2" == "x86_64" ]]; then
        shift 2
    elif [[ "$1" == "x86_64" && "${args[-1]}" == "-arch" ]]; then
        unset 'args[-1]'
        shift
    else
        args+=("$1")
        shift
    fi
done
exec /usr/bin/gcc "-arch" "arm64" "-isysroot" "$(xcrun --show-sdk-path)" "${args[@]}"
EOF

# Ignores the INSTALL_PREFPANE flag for some reason 
cat << 'EOF' > /tmp/wrapper-bin/xcodebuild
#!/bin/bash
exit 0
EOF

chmod +x /tmp/wrapper-bin/gcc
chmod +x /tmp/wrapper-bin/xcodebuild
cp /tmp/wrapper-bin/gcc /tmp/wrapper-bin/clang
cp /tmp/wrapper-bin/gcc /tmp/wrapper-bin/cc

export PATH=/tmp/wrapper-bin:/opt/gnu/bin:$PATH
export PKG_CONFIG_PATH=/opt/gnu/lib/pkgconfig:/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH

cd fuse-ext2

./autogen.sh
CFLAGS="-arch arm64 -idirafter /opt/gnu/include -idirafter /usr/local/include/macfuse -idirafter /opt/homebrew/include" \
LDFLAGS="-arch arm64 -L/opt/gnu/lib -L/usr/local/lib -L/opt/homebrew/lib" \
./configure --prefix=/usr/local --host=aarch64-apple-darwin --disable-prefpane || fail "Failed to configure fuse-ext2"

make -j 8 || fail "Failed to build fuse-ext2 "
sudo make install INSTALL_PREFPANE=no -j 8  || fail "Failed to isntall fuse-ext2"
