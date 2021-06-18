#!/bin/bash

echo "this only be verify on debain 8/ubuntu 20.04 LTS version"

echo $1

pushd $1
echo "enter workshop dir: `pwd`"

# clone the source
git clone https://github.com/grpc/grpc.git --depth=10 --recursive

pushd grpc
echo "enter source dir: `pwd`"

git submodule update --init --recursive

popd
echo "back to: `pwd`"

# build grpc from source
mkdir build_grpc
pushd build_grpc
echo "enter grpc build dir: `pwd`"
#install requirements
sudo apt-get install build-essential autoconf libtool pkg-config zlib1g-dev
echo "verify with cmake version 3.20.x, old version may have installation bug"
echo "more detail see:https://github.com/grpc/grpc/issues/13841"
cmake -DgRPC_INSTALL=ON \
  -DgRPC_BUILD_TESTS=OFF \
  -DgRPC_ZLIB_PROVIDER=package \
  -DCMAKE_BUILD_TYPE=Release ../grpc/

sudo make install -j8
popd
echo "back to: `pwd`"

# build abseil-cpp bz bad install
mkdir build_abseil
pushd build_abseil
echo "enter abseil build dir: `pwd`"
cmake -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE ../grpc/third_party/abseil-cpp
sudo make -j4 install
popd
echo "back to: `pwd`"

