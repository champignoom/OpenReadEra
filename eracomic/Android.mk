LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := eracomic

## WARNING! 'minizip' module SHOULD go last in static library list, not to interfere with modules which include libzip dynamically!!!11

LOCAL_STATIC_LIBRARIES := orebridge orecrop jpeg-turbo orelibpng easybmp webp dmc_unrar minizip
LOCAL_LDLIBS := -llog

LOCAL_CPPFLAGS := $(APP_CPPFLAGS)
LOCAL_CFLAGS := $(APP_CFLAGS)

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS += -DARCH_ARM -DARCH_THUMB -DARCH_ARM_CAN_LOAD_UNALIGNED
endif # TARGET_ARCH == armeabi

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include

LOCAL_SRC_FILES := \
	EraComicMain.cpp \
	EraComicBridge.cpp \
    EraComicManager.cpp \
    EraComicRender.cpp \
    EraComicRarUtils.cpp \
	EraCbrManager.cpp \
	EraCbzManager.cpp

include $(BUILD_EXECUTABLE)
