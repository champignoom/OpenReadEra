LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := minizip

LOCAL_CPPFLAGS := $(APP_CPPFLAGS)
LOCAL_CFLAGS := $(APP_CFLAGS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/ \
    $(LOCAL_PATH)/zlib/ \

LOCAL_SRC_FILES += \
    ioapi.c \
    ioapi_buf.c \
    ioapi_mem.c \
    miniunz.c \
    minizip.c \
    unzip.c \
    zip.c

LOCAL_SRC_FILES += \
    aes/aescrypt.c \
    aes/aeskey.c \
    aes/aestab.c \
    aes/entropy.c \
    aes/fileenc.c \
    aes/hmac.c \
    aes/prng.c \
    aes/pwd2key.c \
    aes/sha1.c


LOCAL_SRC_FILES += \
    zlib/adler32.c \
    zlib/compress.c \
    zlib/crc32.c \
    zlib/deflate.c \
    zlib/gzclose.c \
    zlib/gzlib.c \
    zlib/gzread.c \
    zlib/gzwrite.c \
    zlib/infback.c \
    zlib/inffast.c \
    zlib/inflate.c \
    zlib/inftrees.c \
    zlib/trees.c \
    zlib/uncompr.c \
    zlib/zutil.c
	
include $(BUILD_STATIC_LIBRARY)