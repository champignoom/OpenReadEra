LOCAL_PATH := $(call my-dir)

##################################################
###                openjpeg-simd-arm7-neon     ###
##################################################
 
include $(CLEAR_VARS)

LOCAL_MODULE := openjpeg-simd-arm7-neon
 
LOCAL_MODULE_TAGS := release

LOCAL_CFLAGS    := $(APP_CFLAGS)   -march=armv7-a -mfpu=neon
LOCAL_CPPFLAGS  := $(APP_CPPFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../openjpeg/include
 
LOCAL_SRC_FILES := \
	src/neon/dwt.c \
	src/neon/mct.c \
	src/neon/tcd_int.c \
	src/neon/tcd_real.c

include $(BUILD_STATIC_LIBRARY)
 
##################################################
###                openjpeg-simd-arm8-neon     ###
##################################################
 
include $(CLEAR_VARS)

LOCAL_MODULE := openjpeg-simd-arm8-neon
 
LOCAL_MODULE_TAGS := release

LOCAL_CFLAGS    := $(APP_CFLAGS)
LOCAL_CPPFLAGS  := $(APP_CPPFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../openjpeg/include
 
LOCAL_SRC_FILES := \
	src/neon/dwt.c \
	src/neon/mct.c \
	src/neon/tcd_int.c \
	src/neon/tcd_real.c

include $(BUILD_STATIC_LIBRARY)
 
##################################################
###                openjpeg-simd-sse           ###
##################################################
 
include $(CLEAR_VARS)

LOCAL_MODULE := openjpeg-simd-sse
 
LOCAL_MODULE_TAGS := release

LOCAL_CFLAGS    := $(APP_CFLAGS)   -D__SSE__
LOCAL_CPPFLAGS  := $(APP_CPPFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../openjpeg/include
 
LOCAL_SRC_FILES := \
	src/sse/dwt.c \
	src/sse/mct.c \
	src/sse/tcd.c


include $(BUILD_STATIC_LIBRARY)
 
##################################################
###                simd                        ###
##################################################

include $(CLEAR_VARS)

LOCAL_MODULE := openjpeg-simd
 
LOCAL_MODULE_TAGS := release

LOCAL_CFLAGS    := $(APP_CFLAGS)
LOCAL_CPPFLAGS  := $(APP_CPPFLAGS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../openjpeg/include \
 	$(LOCAL_PATH)/../../../orebridge/include
 
ifeq ($(TARGET_ARCH_ABI),armeabi)
	LOCAL_SRC_FILES := src/opj_simd.c src/opj_simd_none.c
endif # TARGET_ARCH_ABI == armeabi

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	LOCAL_CFLAGS    += -march=armv7-a -mfpu=neon
	LOCAL_SRC_FILES := src/opj_simd.c src/opj_simd_neon.c
	LOCAL_STATIC_LIBRARIES := openjpeg-simd-arm7-neon
endif # TARGET_ARCH_ABI == armeabi-v7a

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a-hard)
	LOCAL_CFLAGS    += -march=armv7-a -mfpu=neon
	LOCAL_SRC_FILES := src/opj_simd.c src/opj_simd_neon.c
	LOCAL_STATIC_LIBRARIES := openjpeg-simd-arm7-neon
endif # TARGET_ARCH_ABI == armeabi-v7a

ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
	LOCAL_SRC_FILES := src/opj_simd.c src/opj_simd_neon.c
	LOCAL_STATIC_LIBRARIES := openjpeg-simd-arm8-neon
endif # TARGET_ARCH_ABI == arm64-v8a

ifeq ($(TARGET_ARCH_ABI),x86)
	LOCAL_SRC_FILES := src/opj_simd.c src/opj_simd_sse.c
	LOCAL_STATIC_LIBRARIES := openjpeg-simd-sse
endif # TARGET_ARCH_ABI == x86

ifeq ($(TARGET_ARCH_ABI),x86_64)
	LOCAL_SRC_FILES := src/opj_simd.c src/opj_simd_sse.c
	LOCAL_STATIC_LIBRARIES := openjpeg-simd-sse
endif # TARGET_ARCH_ABI == x86_64

ifeq ($(TARGET_ARCH_ABI),mips)
	LOCAL_SRC_FILES := src/opj_simd.c src/opj_simd_none.c
endif # TARGET_ARCH_ABI == mips

include $(BUILD_STATIC_LIBRARY)


