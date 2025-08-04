#!/bin/sh
set -e

URL=$1
PLUGIN=$2
UE_PATH=$3

if [ -z "$URL" ]; then
  echo "invalid URL"
  exit 1
fi

if [ -z "$PLUGIN" ]; then
  echo "invalid PLUGIN"
  exit 1
fi

if [ -z "$UE_PATH" ]; then
  echo "invalid UE_PATH"
  exit 1
fi

if [ ! -d "$PLUGIN" ]; then
  echo "PLUGIN directory does not exist"
  exit 1
fi

TARBALL=`basename $URL`
TARDIR=luau-`basename $TARBALL .tar.gz`

if [ -d "$TARDIR" ]; then
  echo "TARBALL directory already exists"
  exit 1
fi

NCPU=`nproc`
TARGETS="Luau.Ast Luau.Compiler Luau.VM Luau.Config Luau.Analysis Luau.EqSat"
PLATFORMS="linux_x64 linux_arm64"
TOOLCHAIN_PATH="$HOME/v23_clang-18.1.0-rockylinux8"

echo downloading from $URL
curl -LO $URL

echo extracting $TARBALL to $TARDIR
tar zxf $TARBALL
cd $TARDIR

for PLATFORM in $PLATFORMS; do
  mkdir build_$PLATFORM
done

cd build_linux_x64
cmake .. -DCMAKE_C_COMPILER=$TOOLCHAIN_PATH/x86_64-unknown-linux-gnu/bin/clang -DCMAKE_CXX_COMPILER=$TOOLCHAIN_PATH/x86_64-unknown-linux-gnu/bin/clang++ -DCMAKE_CXX_FLAGS="-stdlib=libc++ -L$UE_PATH/Engine/Source/ThirdParty/Unix/LibCxx/lib/Unix/x86_64-unknown-linux-gnu/ -I$UE_PATH/Engine/Source/ThirdParty/Unix/LibCxx/include/c++/v1/" -DCMAKE_SYSROOT="$TOOLCHAIN_PATH/x86_64-unknown-linux-gnu" -DCMAKE_BUILD_TYPE=Release
for TARGET in $TARGETS; do
  cmake --build . --target $TARGET --config Release -j$NCPU
done

cd ..

cd build_linux_arm64
cmake .. -DCMAKE_C_COMPILER=$TOOLCHAIN_PATH/aarch64-unknown-linux-gnueabi/bin/clang -DCMAKE_CXX_COMPILER=$TOOLCHAIN_PATH/aarch64-unknown-linux-gnueabi/bin/clang++ -DCMAKE_C_FLAGS="--target=aarch64-unknown-linux-gnueabi" -DCMAKE_CXX_FLAGS="--target=aarch64-unknown-linux-gnueabi -stdlib=libc++ -L$UE_PATH/Engine/Source/ThirdParty/Unix/LibCxx/lib/Unix/aarch64-unknown-linux-gnueabi/ -I$UE_PATH/Engine/Source/ThirdParty/Unix/LibCxx/include/c++/v1/" -DCMAKE_SYSROOT="$TOOLCHAIN_PATH/aarch64-unknown-linux-gnueabi" -DCMAKE_BUILD_TYPE=Release
for TARGET in $TARGETS; do
  cmake --build . --target $TARGET --config Release -j$NCPU
done

cd ..

for PLATFORM in $PLATFORMS; do
  for TARGET in $TARGETS; do
    cp "build_$PLATFORM/lib$TARGET.a" "$PLUGIN/Source/ThirdParty/lib/lib${TARGET}_${PLATFORM}.a"
  done
done

echo done.
