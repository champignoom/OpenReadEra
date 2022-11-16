LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := jpeg-turbo

ifneq ($(filter $(TARGET_ARCH_ABI), armeabi-v7a arm64-v8a x86),)
LOCAL_ARM_NEON := true

LOCAL_CFLAGS += -D__ARM_HAVE_NEON
endif

LOCAL_CFLAGS += -DANDROID -DANDROID_TILE_BASED_DECODE -DANDROID_RGB -DENABLE_ANDROID_NULL_CONVERT

LOCAL_ASMFLAGS += -DELF

ifeq ($(TARGET_ARCH_ABI),x86_64)
LOCAL_SRC_FILES += \
	$(LOCAL_PATH)/simd/x86_64/jsimd.c \
	$(LOCAL_PATH)/simd/x86_64/jfdctflt-sse.asm \
	$(LOCAL_PATH)/simd/x86_64/jfdctfst-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jfdctint-avx2.asm \
	$(LOCAL_PATH)/simd/x86_64/jfdctint-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jidctflt-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jidctfst-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jidctint-avx2.asm \
	$(LOCAL_PATH)/simd/x86_64/jidctint-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jidctred-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jccolor-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jccolor-avx2.asm \
	$(LOCAL_PATH)/simd/x86_64/jcgray-avx2.asm \
	$(LOCAL_PATH)/simd/x86_64/jcgray-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jcsample-avx2.asm \
	$(LOCAL_PATH)/simd/x86_64/jcsample-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jdcolor-avx2.asm \
	$(LOCAL_PATH)/simd/x86_64/jdcolor-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jdmerge-avx2.asm \
	$(LOCAL_PATH)/simd/x86_64/jdmerge-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jdsample-avx2.asm \
	$(LOCAL_PATH)/simd/x86_64/jdsample-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jquantf-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jquanti-avx2.asm \
	$(LOCAL_PATH)/simd/x86_64/jquanti-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jsimdcpu.asm \
	$(LOCAL_PATH)/simd/x86_64/jchuff-sse2.asm \
	$(LOCAL_PATH)/simd/x86_64/jcphuff-sse2.asm \


LOCAL_CFLAGS += \
	-DSIZEOF_SIZE_T=8 \

LOCAL_ASMFLAGS += -D__x86_64__

else ifeq ($(TARGET_ARCH_ABI),x86)
LOCAL_SRC_FILES += \
	$(LOCAL_PATH)/simd/i386/jsimd.c \
	$(LOCAL_PATH)/simd/i386/jsimdcpu.asm \
	$(LOCAL_PATH)/simd/i386/jccolor-avx2.asm \
	$(LOCAL_PATH)/simd/i386/jccolor-mmx.asm \
	$(LOCAL_PATH)/simd/i386/jccolor-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jcgray-avx2.asm \
	$(LOCAL_PATH)/simd/i386/jcgray-mmx.asm \
	$(LOCAL_PATH)/simd/i386/jcgray-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jchuff-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jcphuff-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jcsample-avx2.asm \
	$(LOCAL_PATH)/simd/i386/jcsample-mmx.asm \
	$(LOCAL_PATH)/simd/i386/jcsample-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jdcolor-avx2.asm \
	$(LOCAL_PATH)/simd/i386/jdcolor-mmx.asm \
	$(LOCAL_PATH)/simd/i386/jdcolor-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jdmerge-avx2.asm \
	$(LOCAL_PATH)/simd/i386/jdmerge-mmx.asm \
	$(LOCAL_PATH)/simd/i386/jdmerge-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jdsample-avx2.asm \
	$(LOCAL_PATH)/simd/i386/jdsample-mmx.asm \
	$(LOCAL_PATH)/simd/i386/jdsample-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jfdctflt-3dn.asm \
	$(LOCAL_PATH)/simd/i386/jfdctflt-sse.asm \
	$(LOCAL_PATH)/simd/i386/jfdctfst-mmx.asm \
	$(LOCAL_PATH)/simd/i386/jfdctfst-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jfdctint-avx2.asm \
	$(LOCAL_PATH)/simd/i386/jfdctint-mmx.asm \
	$(LOCAL_PATH)/simd/i386/jfdctint-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jidctflt-3dn.asm \
	$(LOCAL_PATH)/simd/i386/jidctflt-sse.asm \
	$(LOCAL_PATH)/simd/i386/jidctflt-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jidctfst-mmx.asm \
	$(LOCAL_PATH)/simd/i386/jidctfst-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jidctint-avx2.asm \
	$(LOCAL_PATH)/simd/i386/jidctint-mmx.asm \
	$(LOCAL_PATH)/simd/i386/jidctint-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jidctred-mmx.asm \
	$(LOCAL_PATH)/simd/i386/jidctred-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jquant-3dn.asm \
	$(LOCAL_PATH)/simd/i386/jquant-mmx.asm \
	$(LOCAL_PATH)/simd/i386/jquant-sse.asm \
	$(LOCAL_PATH)/simd/i386/jquantf-sse2.asm \
	$(LOCAL_PATH)/simd/i386/jquanti-avx2.asm \
	$(LOCAL_PATH)/simd/i386/jquanti-sse2.asm \

LOCAL_CFLAGS += \
	-DSIZEOF_SIZE_T=4 \

LOCAL_ASMFLAGS += -DPIC

else ifneq ($(filter $(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard),)
LOCAL_SRC_FILES += \
	$(LOCAL_PATH)/simd/arm/aarch32/jchuff-neon.c \
	$(LOCAL_PATH)/simd/arm/aarch32/jsimd.c \
	$(LOCAL_PATH)/simd/arm/aarch32/jsimd_neon.S \
	$(LOCAL_PATH)/simd/arm/jccolor-neon.c \
    $(LOCAL_PATH)/simd/arm/jcgray-neon.c \
    $(LOCAL_PATH)/simd/arm/jcphuff-neon.c \
    $(LOCAL_PATH)/simd/arm/jcsample-neon.c \
    $(LOCAL_PATH)/simd/arm/jdcolor-neon.c \
    $(LOCAL_PATH)/simd/arm/jdmerge-neon.c \
    $(LOCAL_PATH)/simd/arm/jdsample-neon.c \
    $(LOCAL_PATH)/simd/arm/jfdctfst-neon.c \
    $(LOCAL_PATH)/simd/arm/jfdctint-neon.c \
    $(LOCAL_PATH)/simd/arm/jidctfst-neon.c \
    $(LOCAL_PATH)/simd/arm/jidctint-neon.c \
    $(LOCAL_PATH)/simd/arm/jidctred-neon.c \
    $(LOCAL_PATH)/simd/arm/jquanti-neon.c \

LOCAL_CFLAGS += \
	-DSIZEOF_SIZE_T=4 \

else ifeq ($(TARGET_ARCH_ABI),armeabi)
LOCAL_CFLAGS += \
	-DSIZEOF_SIZE_T=4 \

else ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
LOCAL_SRC_FILES += \
	$(LOCAL_PATH)/simd/arm/aarch64/jchuff-neon.c \
	$(LOCAL_PATH)/simd/arm/aarch64/jsimd.c \
	$(LOCAL_PATH)/simd/arm/aarch64/jsimd_neon.S \
	$(LOCAL_PATH)/simd/arm/jccolor-neon.c \
    $(LOCAL_PATH)/simd/arm/jcgray-neon.c \
    $(LOCAL_PATH)/simd/arm/jcphuff-neon.c \
    $(LOCAL_PATH)/simd/arm/jcsample-neon.c \
    $(LOCAL_PATH)/simd/arm/jdcolor-neon.c \
    $(LOCAL_PATH)/simd/arm/jdmerge-neon.c \
    $(LOCAL_PATH)/simd/arm/jdsample-neon.c \
    $(LOCAL_PATH)/simd/arm/jfdctfst-neon.c \
    $(LOCAL_PATH)/simd/arm/jfdctint-neon.c \
    $(LOCAL_PATH)/simd/arm/jidctfst-neon.c \
    $(LOCAL_PATH)/simd/arm/jidctint-neon.c \
    $(LOCAL_PATH)/simd/arm/jidctred-neon.c \
    $(LOCAL_PATH)/simd/arm/jquanti-neon.c \

LOCAL_CFLAGS += \
	-DSIZEOF_SIZE_T=8 \

endif

# libjpeg_la_SOURCES from Makefile.am
LOCAL_SRC_FILES += \
	$(LOCAL_PATH)/jcapimin.c \
	$(LOCAL_PATH)/jcapistd.c \
	$(LOCAL_PATH)/jccoefct.c \
	$(LOCAL_PATH)/jccolor.c \
	$(LOCAL_PATH)/jcdctmgr.c \
	$(LOCAL_PATH)/jchuff.c \
	$(LOCAL_PATH)/jcinit.c \
	$(LOCAL_PATH)/jcmainct.c \
	$(LOCAL_PATH)/jcmarker.c \
	$(LOCAL_PATH)/jcmaster.c \
	$(LOCAL_PATH)/jcomapi.c \
	$(LOCAL_PATH)/jcparam.c \
	$(LOCAL_PATH)/jcphuff.c \
	$(LOCAL_PATH)/jcprepct.c \
	$(LOCAL_PATH)/jcsample.c \
	$(LOCAL_PATH)/jctrans.c \
	$(LOCAL_PATH)/jdapimin.c \
	$(LOCAL_PATH)/jdapistd.c \
	$(LOCAL_PATH)/jdatadst.c \
	$(LOCAL_PATH)/jdatasrc.c \
	$(LOCAL_PATH)/jdcoefct.c \
	$(LOCAL_PATH)/jdcolor.c \
	$(LOCAL_PATH)/jddctmgr.c \
	$(LOCAL_PATH)/jdhuff.c \
	$(LOCAL_PATH)/jdinput.c \
	$(LOCAL_PATH)/jdmainct.c \
	$(LOCAL_PATH)/jdmarker.c \
	$(LOCAL_PATH)/jdmaster.c \
	$(LOCAL_PATH)/jdmerge.c \
	$(LOCAL_PATH)/jdphuff.c \
	$(LOCAL_PATH)/jdpostct.c \
	$(LOCAL_PATH)/jdsample.c \
	$(LOCAL_PATH)/jdtrans.c \
	$(LOCAL_PATH)/jerror.c \
	$(LOCAL_PATH)/jfdctflt.c \
	$(LOCAL_PATH)/jfdctfst.c \
	$(LOCAL_PATH)/jfdctint.c \
	$(LOCAL_PATH)/jidctflt.c \
	$(LOCAL_PATH)/jidctfst.c \
	$(LOCAL_PATH)/jidctint.c \
	$(LOCAL_PATH)/jidctred.c \
	$(LOCAL_PATH)/jquant1.c \
	$(LOCAL_PATH)/jquant2.c \
	$(LOCAL_PATH)/jutils.c \
	$(LOCAL_PATH)/jmemmgr.c \
	$(LOCAL_PATH)/jmemnobs.c \

# if WITH_ARITH_ENC from Makefile.am
LOCAL_SRC_FILES += \
	$(LOCAL_PATH)/jaricom.c \
	$(LOCAL_PATH)/jcarith.c \
	$(LOCAL_PATH)/jdarith.c \

# libturbojpeg_la_SOURCES from Makefile.am
LOCAL_SRC_FILES += \
	$(LOCAL_PATH)/turbojpeg.c \
	$(LOCAL_PATH)/transupp.c \
	$(LOCAL_PATH)/jdatadst-tj.c \
	$(LOCAL_PATH)/jdatasrc-tj.c \

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/ \
	$(LOCAL_PATH)/include \
    $(LOCAL_PATH)/simd \
    $(LOCAL_PATH)/simd/arm \
    $(LOCAL_PATH)/simd/nasm

LOCAL_EXPORT_C_INCLUDES := \
	$(LOCAL_PATH)/$(LOCAL_PATH) \

LOCAL_CFLAGS += \
	-DC_ARITH_CODING_SUPPORTED=1 \
	-DD_ARITH_CODING_SUPPORTED=1 \
	-DHAVE_DLFCN_H=1 \
	-DHAVE_INTTYPES_H=1 \
	-DHAVE_LOCALE_H=1 \
	-DHAVE_MEMCPY=1 \
	-DHAVE_MEMORY_H=1 \
	-DHAVE_MEMSET=1 \
	-DHAVE_STDDEF_H=1 \
	-DHAVE_STDINT_H=1 \
	-DHAVE_STDLIB_H=1 \
	-DHAVE_STRINGS_H=1 \
	-DHAVE_STRING_H=1 \
	-DHAVE_SYS_STAT_H=1 \
	-DHAVE_SYS_TYPES_H=1 \
	-DHAVE_UNISTD_H=1 \
	-DHAVE_UNSIGNED_CHAR=1 \
	-DHAVE_UNSIGNED_SHORT=1 \
	-DMEM_SRCDST_SUPPORTED=1 \
	-DNEED_SYS_TYPES_H=1 \
	-DSTDC_HEADERS=1 \
	-DWITH_SIMD=1 \

LOCAL_CPPFLAGS += -O3
LOCAL_CFLAGS += -O3

include $(BUILD_STATIC_LIBRARY)