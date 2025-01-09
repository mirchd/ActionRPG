
# Setup paths to stuff we need
XCODE_PATH=`xcode-select -print-path`
export PATH="${XCODE_PATH}/Toolchains/XcodeDefault.xctoolchain/usr/bin/:$PATH"

SDK_VERSION=""

IPHONEOS_TOP="${XCODE_PATH}/Platforms/iPhoneOS.platform/Developer"
IPHONEOS_SDK=iPhoneOS${SDK_VERSION}.sdk

IPHONESIMULATOR_TOP="${XCODE_PATH}/Platofrms/iPhoneSimulator.platform/Developer"
IPHONESIMULATOR_SDK=iPhoneSimulator${SDK_VERSION}.sdk

export IPHONEOS_DEPLOYMENT_TARGET=6.0

DEBUG=false

#
# lua armv7
#

echo =============
echo lua armv7
echo =============

export ARCH=armv7
export SDK="${IPHONEOS_TOP}/SDKs/${IPHONEOS_SDK}"

export HOST="$ARCH-apple-darwin"
export CC="clang -arch $ARCH -mios-version-min=7.0 -isysroot $SDK"
export CFLAGS="-pipe -gdwarf-2 -O2 -Wall -Werror -Wextra -DLUA_USE_POSIX -DLUA_USEDLOPEN -DLUA_COMPAT_5_2 -std=gnu99"
export LDFLAGS="-arch $ARCH -L$CUR_PATH/libs/$ARCH/"

$CC $CFLAGS -c *.c
rm lua.o luac.o
ar -rv liblua53-armv7.a *.o && rm *.o

#
# lua arm64
#

echo =============
echo lua arm64
echo =============

export ARCH=arm64
export SDK="${IPHONEOS_TOP}/SDKs/${IPHONEOS_SDK}"

export HOST="arm-apple-darwin"
export CC="clang -arch $ARCH -mios-version-min=7.0 -isysroot $SDK"
export CFLAGS="-pipe -gdwarf-2 -O2 -Wall -Werror -Wextra -DLUA_USE_POSIX -DLUA_USEDLOPEN -DLUA_COMPAT_5_2 -std=gnu99"
export LDFLAGS="-arch $ARCH"

$CC $CFLAGS -c *.c
rm lua.o luac.o
ar -rv liblua53-arm64.a *.o && rm *.o

#
# lua i386
#

echo =============
echo lua i386
echo =============

export ARCH=i386
export SDK="${IHONESIMULATOR_TOP}/SDKs/${IPHONESIMULATOR_SDK}"

export HOST="$ARCH-apple-darwin"
export CC="clang -arch $ARCH -isysroot $SDK"
export CFLAGS="-pipe -gdwarf-2 -O2 -Wall -Werror -Wextra -DLUA_USE_POSIX -DLUA_USEDLOPEN -DLUA_COMPAT_5_2 -std=gnu99"
export LDFLAGS="-arch $ARCH -L$CUR_PATH/libs/$ARCH/"

$CC $CFLAGS -c *.c
rm lua.o luac.o
ar -rv liblua53-i386.a *.o && rm *.o

echo =============
echo lua universal library
echo =============

#
# make universal library for both the simulator and the device
#

lipo -create ./liblua53-armv7.a ./liblua53-arm64.a ./liblua53-i386.a -output liblua53_ios.a

[[ $DEBUG = false]] && strip -S liblua.a
