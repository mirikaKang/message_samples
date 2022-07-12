#!/bin/bash
rm -rf bin
rm -rf build

mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="../../vcpkg/scripts/buildsystems/vcpkg.cmake"
make -B
export LC_ALL=C
unset LANGUAGE

./messaging_system/unittest/unittest

cd ..
mkdir bin
cp ./build/container_sample/container_sample ./bin/container_sample
cp ./build/echo_client/echo_client ./bin/echo_client
cp ./build/echo_server/echo_server ./bin/echo_server
cp ./build/logging_sample/logging_sample ./bin/logging_sample
cp ./build/threads_sample/threads_sample ./bin/threads_sample

rm -rf build