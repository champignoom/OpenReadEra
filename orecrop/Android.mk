LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := orecrop

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include

LOCAL_SRC_FILES := \
	smartcrop.cpp

include $(BUILD_STATIC_LIBRARY)
