LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := jbig2dec
LOCAL_CFLAGS    := $(APP_CFLAGS) -DHAVE_CONFIG_H
LOCAL_CPPFLAGS  := $(APP_CPPFLAGS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := \
	src/jbig2.c \
	src/jbig2_arith.c \
	src/jbig2_arith_iaid.c \
	src/jbig2_arith_int.c \
	src/jbig2_generic.c \
	src/jbig2_halftone.c \
	src/jbig2_huffman.c \
	src/jbig2_image.c \
	src/jbig2_image_pbm.c \
	src/jbig2_metadata.c \
	src/jbig2_mmr.c \
	src/jbig2_page.c \
	src/jbig2_refinement.c \
	src/jbig2_segment.c \
	src/jbig2_symbol_dict.c \
	src/jbig2_text.c \
	src/jbig2dec.c \
	src/sha1.c \
	src/memento.c

include $(BUILD_STATIC_LIBRARY)
