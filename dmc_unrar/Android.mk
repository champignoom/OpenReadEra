LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := dmc_unrar

LOCAL_CPPFLAGS := $(APP_CPPFLAGS)
LOCAL_CFLAGS := $(APP_CFLAGS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/

LOCAL_SRC_FILES += \
    dmc_unrar.c
	
include $(BUILD_STATIC_LIBRARY)