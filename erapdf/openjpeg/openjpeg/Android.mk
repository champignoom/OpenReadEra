LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := openjpeg
LOCAL_STATIC_LIBRARIES := openjpeg-simd orebridge

LOCAL_CFLAGS    := $(APP_CFLAGS)
LOCAL_CPPFLAGS  := $(APP_CPPFLAGS)
#LOCAL_CFLAGS += -DOPJ_PROFILE

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../openjpeg-simd/include \
	$(LOCAL_PATH)/../../../orebridge/include

LOCAL_SRC_FILES := \
	src/bio.c \
	src/cio.c \
	src/dwt.c \
	src/event.c \
	src/function_list.c \
	src/image.c \
	src/invert.c \
	src/j2k.c \
	src/jp2.c \
	src/mct.c \
	src/mqc.c \
	src/openjpeg.c \
	src/opj_clock.c \
	src/pi.c \
	src/t1.c \
	src/t2.c \
	src/tcd.c \
	src/tgt.c \
    src/cidx_manager.c \
    src/opj_malloc.c \
    src/phix_manager.c \
    src/ppix_manager.c \
    src/sparse_array.c \
    src/test_sparse_array.c \
    src/thread.c \
    src/tpix_manager.c \
    src/thix_manager.c

include $(BUILD_STATIC_LIBRARY)
