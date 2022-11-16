LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := orebridge

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include

LOCAL_SRC_FILES := \
	StBridge.cpp \
	StProtocol.cpp \
	StQueue.cpp \
	StRequestQueue.cpp \
	StResponseQueue.cpp \
	StStringNaturalCompare.cpp \
	StSearchUtils.cpp \
	StSocket.cpp \
	openreadera.cpp \
	debug_intentional_crash.cpp

include $(BUILD_STATIC_LIBRARY)
