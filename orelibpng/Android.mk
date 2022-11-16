LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := orelibpng

LOCAL_CPPFLAGS := $(APP_CPPFLAGS)
LOCAL_CFLAGS := $(APP_CFLAGS)
# -DPNG_ARM_NEON_OPT=0: compiler defines __ARM_NEON__ on ARM builds and when
# __ARM_NEON__ is defined, orelibpng by default sets PNG_ARM_NEON_OPT=2,
# which makes build fail, since we not building ARM NEON part of the code.
# So we explicitly disabling ARM NEON optimizations with -DPNG_ARM_NEON_OPT=0.
LOCAL_CFLAGS += -DHAVE_CONFIG_H -DPNG_ARM_NEON_OPT=0

LOCAL_SRC_FILES += \
    pngerror.c  \
    pngget.c  \
    pngpread.c \
    pngrio.c \
    pngrutil.c \
    pngvcrd.c \
    png.c \
    pngwrite.c \
    pngwutil.c \
    pnggccrd.c \
    pngmem.c \
    pngread.c \
    pngrtran.c \
    pngset.c \
    pngtrans.c \
    pngwio.c \
    pngwtran.c

include $(BUILD_STATIC_LIBRARY)