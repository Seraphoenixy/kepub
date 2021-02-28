#!/bin/bash

set -e

source $(dirname "$0")/install-system.sh

if [ ! -d "dependencies" ]; then
    echo "The dependencies directory does not exist"
    exit 1
fi

cd dependencies

cd fmt-*
sudo cmake --build build --config Release --target install
cd ..
echo "Install fmt completed"

cd icu/source
sudo make install
cd ..
echo "Install icu completed"

cd cd boost_*
sudo ./b2 --toolset=gcc-10 --with-program_options install
cd ..
echo "Install Boost completed"

cd ..

sudo ldconfig

echo "Install completed"
