LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := eramobi
LOCAL_STATIC_LIBRARIES := orebridge
LOCAL_LDLIBS += -llog

LOCAL_CPPFLAGS := $(APP_CPPFLAGS)
LOCAL_CFLAGS := $(APP_CFLAGS)
LOCAL_CFLAGS +=  -DUSE_MINIZ -DUSE_XMLWRITER

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../orebridge/include \
    $(LOCAL_PATH)/src \
    $(LOCAL_PATH)/tools

LOCAL_SRC_FILES := \
	EraMobiMain.cpp \
	EraMobiBridge.cpp \
	EraMobiMeta.cpp \
	EraMobiConvert.cpp \
	EraMobiUtils.cpp

LOCAL_SRC_FILES += \
	src/buffer.c \
    src/compression.c \
    src/debug.c \
    src/encryption.c \
    src/index.c \
    src/memory.c \
    src/meta.c \
    src/opf.c \
    src/parse_rawml.c \
    src/read.c \
    src/sha1.c \
    src/structure.c \
    src/util.c \
    src/write.c \
    src/xmlwriter.c \
    src/miniz.c \
    tools/common.c

include $(BUILD_EXECUTABLE)