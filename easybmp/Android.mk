LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := easybmp

LOCAL_CPPFLAGS := $(APP_CPPFLAGS)
LOCAL_CFLAGS := $(APP_CFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/

LOCAL_SRC_FILES := \
	EasyBMP.cpp

include $(BUILD_STATIC_LIBRARY)