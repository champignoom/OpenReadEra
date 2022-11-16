LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := erapdf
LOCAL_STATIC_LIBRARIES := orebridge orecrop freetype jpeg-turbo jbig2dec openjpeg
LOCAL_LDLIBS := -lz -llog

LOCAL_CPPFLAGS := $(APP_CPPFLAGS)
LOCAL_CFLAGS := $(APP_CFLAGS) -DAA_BITS=8 -DNDEBUG
#LOCAL_CFLAGS += -DMUPDF_LOG_WARN_ENABLED
ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS += -DARCH_ARM -DARCH_THUMB -DARCH_ARM_CAN_LOAD_UNALIGNED
endif # TARGET_ARCH == armeabi

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../orebridge/include \
	$(LOCAL_PATH)/../../orecrop/include \
	$(LOCAL_PATH)/../../jpeg-turbo/jpeg-turbo/include \
	$(LOCAL_PATH)/../freetype/overlay \
	$(LOCAL_PATH)/../freetype/include \
	$(LOCAL_PATH)/../jbig2dec/include \
	$(LOCAL_PATH)/../openjpeg/openjpeg/include \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/generated

LOCAL_SRC_FILES := \
	EraPdfMain.cpp \
	EraPdfBridge.cpp \
	MuPdfLinks.cpp \
	MuPdfOutline.cpp \
	MuPdfText.cpp \
	MuPdfConfig.cpp \
	MuPdfSearch.cpp \
	EraPdfReflow.cpp \
	EraPdfReflowText.cpp

LOCAL_SRC_FILES += \
	pdf/js/pdf-js-none.c \
	pdf/pdf-annot.c \
	pdf/pdf-annot-edit.c \
	pdf/pdf-appearance.c \
	pdf/pdf-clean-file.c \
	pdf/pdf-clean.c \
	pdf/pdf-cmap.c \
	pdf/pdf-cmap-load.c \
	pdf/pdf-cmap-parse.c \
	pdf/pdf-cmap-table.c \
	pdf/pdf-colorspace.c \
	pdf/pdf-crypt.c \
	pdf/pdf-device.c \
	pdf/pdf-encoding.c \
	pdf/pdf-event.c \
	pdf/pdf-field.c \
	pdf/pdf-font.c \
	pdf/pdf-fontfile.c \
	pdf/pdf-form.c \
	pdf/pdf-function.c \
	pdf/pdf-image.c \
	pdf/pdf-interpret.c \
	pdf/pdf-lex.c \
	pdf/pdf-metrics.c \
	pdf/pdf-nametree.c \
	pdf/pdf-object.c \
	pdf/pdf-op-buffer.c \
	pdf/pdf-op-filter.c \
	pdf/pdf-op-run.c \
	pdf/pdf-outline.c \
	pdf/pdf-page.c \
	pdf/pdf-parse.c \
	pdf/pdf-pattern.c \
	pdf/pdf-pkcs7.c \
	pdf/pdf-run.c \
	pdf/pdf-repair.c \
	pdf/pdf-shade.c \
	pdf/pdf-store.c \
	pdf/pdf-stream.c \
	pdf/pdf-type3.c \
	pdf/pdf-unicode.c \
	pdf/pdf-write.c \
	pdf/pdf-xobject.c \
	pdf/pdf-xref.c \

LOCAL_SRC_FILES += \
	xps/xps-common.c \
	xps/xps-doc.c \
	xps/xps-glyphs.c \
	xps/xps-gradient.c \
	xps/xps-image.c \
	xps/xps-outline.c \
	xps/xps-path.c \
	xps/xps-resource.c \
	xps/xps-tile.c \
	xps/xps-util.c \
	xps/xps-zip.c

LOCAL_SRC_FILES += \
	fitz/bbox-device.c \
	fitz/bitmap.c \
	fitz/buffer.c \
	fitz/colorspace.c \
	fitz/compressed-buffer.c \
	fitz/context.c \
	fitz/crypt-aes.c \
	fitz/crypt-arc4.c \
	fitz/crypt-md5.c \
	fitz/crypt-sha2.c \
	fitz/device.c \
	fitz/document.c \
	fitz/document-all.c \
	fitz/draw-affine.c \
	fitz/draw-blend.c \
	fitz/draw-device.c \
	fitz/draw-edge.c \
	fitz/draw-glyph.c \
	fitz/draw-mesh.c \
	fitz/draw-paint.c \
	fitz/draw-path.c \
	fitz/draw-scale-simple.c \
	fitz/draw-unpack.c \
	fitz/error.c \
	fitz/filter-basic.c \
	fitz/filter-dct.c \
	fitz/filter-fax.c \
	fitz/filter-flate.c \
	fitz/filter-jbig2.c \
	fitz/filter-leech.c \
	fitz/filter-lzw.c \
	fitz/filter-predict.c \
	fitz/font.c \
	fitz/ftoa.c \
	fitz/function.c \
	fitz/geometry.c \
	fitz/getopt.c \
	fitz/glyph.c \
	fitz/halftone.c \
	fitz/hash.c \
	fitz/image.c \
	fitz/jmemcust.c \
	fitz/link.c \
	fitz/list-device.c \
	fitz/load-jpeg.c \
	fitz/load-jpx.c \
	fitz/load-jxr.c \
	fitz/load-png.c \
	fitz/load-tiff.c \
	fitz/memento.c \
	fitz/memory.c \
	fitz/outline.c \
	fitz/output.c \
	fitz/output-pcl.c \
	fitz/output-pwg.c \
	fitz/path.c \
	fitz/pixmap.c \
	fitz/printf.c \
	fitz/shade.c \
	fitz/stext-device.c \
	fitz/stext-output.c \
	fitz/stext-paragraph.c \
	fitz/stext-search.c \
	fitz/store.c \
	fitz/stream-open.c \
	fitz/stream-prog.c \
	fitz/stream-read.c \
	fitz/strtod.c \
	fitz/string.c \
	fitz/svg-device.c \
	fitz/text.c \
	fitz/time.c \
	fitz/trace-device.c \
	fitz/transition.c \
	fitz/ucdn.c \
	fitz/unzip.c \
	fitz/xml.c

include $(BUILD_EXECUTABLE)
