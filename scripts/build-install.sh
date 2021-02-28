#!/bin/bash

set -e

source $(dirname "$0")/install-system.sh

if [ ! -d "dependencies" ]; then
    echo "mkdir dependencies"
    mkdir dependencies
fi

cd dependencies

# fmt
if [ ! -f "fmt-7.1.3.zip" ]; then
    wget -q https://github.com/fmtlib/fmt/archive/7.1.3.zip -O fmt-7.1.3.zip
fi
unzip -q fmt-*.zip
rm fmt-*.zip
cd fmt-*
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DFMT_DOC=OFF -DFMT_TEST=OFF -DBUILD_SHARED_LIBS=TRUE
cmake --build build --config Release -j$(nproc)
sudo cmake --build build --config Release --target install
cd ..
echo "Build and install fmt completed"

# icu
if [ ! -f "icu4c-68_2-src.tgz" ]; then
    wget -q https://github.com/unicode-org/icu/releases/download/release-68-2/icu4c-68_2-src.tgz \
        -O icu4c-68_2-src.tgz
fi
tar -xf icu4c-*-src.tgz
rm icu4c-*-src.tgz
cd icu/source
./configure --enable-tests=no --enable-samples=no
make -j$(nproc)
sudo make install
cd ../..
echo "Build and install icu completed"

# boost
if [ ! -f "boost_1_75_0.tar.gz" ]; then
    wget -q https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz \
        -O boost_1_75_0.tar.gz
fi
tar -xf boost_*.tar.gz
rm boost_*.tar.gz
cd boost_*
./bootstrap.sh
./b2 --toolset=gcc-10 --with-program_options
sudo ./b2 --toolset=gcc-10 --with-program_options install
cd ..
echo "Build and install boost completed"

cd ..

sudo ldconfig

echo "Build and install completed"
