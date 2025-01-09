LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := liblua53
LOCAL_SRC_FILES := lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c lutf8lib.c llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lctype.c lzio.c lauxlib.c lbaselib.c ldblib.c liolib.c lmathlib.c loslib.c lstrlib.c ltablib.c lbitlib.c lcorolib.c loadlib.c linit.c

# Auxiliary lua user defined file
# LOCAL_SRC_FIILES += luauser.c
# LOCAL_CFLAGS := -DLUA_DL_DLOPEN -DLUA_USER_H='"luauser.h"'

LOCAL_CFLAGS := -DLUA_USE_POSIX -DLUA_DL_DLOPEN -DLUA_COMPAT_5_2 -DLUA_USER_C89 -std=gnu99 -Wall -Wextra
# LOCAL_LDLIBS += -L$(SYSROT)/usr/lib -llog -ldl
LOCAL_CFLAGS += -pie -fPIE
# LOCAL_LDFLAGS += -pie -fPIE

include $(BUILD_STATIC_LIBRARY)
