#!/bin/sh
set -e

URL=$1
PLUGIN=$2

if [ -z "$URL" ]; then
  echo "invalid URL"
  exit 1
fi

if [ -z "$PLUGIN" ]; then
  echo "invalid PLUGIN"
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

NCPU=`sysctl -n hw.ncpu`
TARGETS="Luau.Ast Luau.Compiler Luau.VM Luau.Config Luau.Analysis Luau.EqSat"
PLATFORMS="mac ios android64 android32"
ANDROID_NDK_PATH="$HOME/Library/Android/sdk/ndk/25.1.8937393"

echo downloading from $URL
curl -LO $URL

echo extracting $TARBALL to $TARDIR
tar zxf $TARBALL
cd $TARDIR

for PLATFORM in $PLATFORMS; do
  mkdir build_$PLATFORM
done

cd build_mac
CMAKE_OSX_ARCHITECTURES="arm64;x86_64" /Applications/CMake.app/Contents/bin/cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 ..
for TARGET in $TARGETS; do
  CMAKE_OSX_ARCHITECTURES="arm64;x86_64" /Applications/CMake.app/Contents/bin/cmake --build . --target $TARGET --config Release -j$NCPU
done

cd ../build_ios
/Applications/CMake.app/Contents/bin/cmake -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 ..
for TARGET in $TARGETS; do
  /Applications/CMake.app/Contents/bin/cmake --build . --target $TARGET --config Release -j$NCPU
done

cd ../build_android64
/Applications/CMake.app/Contents/bin/cmake -DCMAKE_SYSTEM_NAME=Android -DCMAKE_ANDROID_NDK="${ANDROID_NDK_PATH}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_ANDROID_ARCH_ABI="arm64-v8a" -DCMAKE_ANDROID_API=21 -DCMAKE_ANDROID_STL_TYPE="c++_static" ..
for TARGET in $TARGETS; do
  /Applications/CMake.app/Contents/bin/cmake --build . --target $TARGET --config Release -j$NCPU
done

cd ../build_android32
/Applications/CMake.app/Contents/bin/cmake -DCMAKE_SYSTEM_NAME=Android -DCMAKE_ANDROID_NDK="${ANDROID_NDK_PATH}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_ANDROID_ARCH_ABI="armeabi-v7a" -DCMAKE_ANDROID_API=21 -DCMAKE_ANDROID_STL_TYPE="c++_static" ..
for TARGET in $TARGETS; do
  /Applications/CMake.app/Contents/bin/cmake --build . --target $TARGET --config Release -j$NCPU
done

cd ..

for PLATFORM in $PLATFORMS; do
  for TARGET in $TARGETS; do
    cp "build_$PLATFORM/lib$TARGET.a" "$PLUGIN/Source/ThirdParty/lib/lib${TARGET}_${PLATFORM}.a"
  done
done

cp -R */include/ "$PLUGIN/Source/ThirdParty/luau"

echo done.
