LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := eraepub
LOCAL_STATIC_LIBRARIES := orebridge orelibjpeg orelibpng webp
LOCAL_LDLIBS += -llog -lz
LOCAL_CPP_FEATURES += exceptions

LOCAL_CPPFLAGS := $(APP_CPPFLAGS)
LOCAL_CFLAGS := $(APP_CFLAGS)
LOCAL_CFLAGS += -DHAVE_CONFIG_H -DLINUX=1 -D_LINUX=1 -DFT2_BUILD_LIBRARY=1
LOCAL_CFLAGS += -DCR3_ANTIWORD_PATCH=1 -DENABLE_ANTIWORD=1
LOCAL_CFLAGS += -DJPEG_LIB_VERSION=80

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../orebridge/include \
    $(LOCAL_PATH)/../ \
    $(LOCAL_PATH)/libpng \
    $(LOCAL_PATH)/freetype/include

LOCAL_SRC_FILES := \
	EraEpubMain.cpp \
	EraEpubBridge.cpp \
	EraEpubConf.cpp \
	EraEpubMeta.cpp \
	EraEpubOutline.cpp \
	EraEpubLinks.cpp \
	EraEpubSearch.cpp \
	EraEpubTexts.cpp \
	EraEpubImages.cpp

LOCAL_SRC_FILES += \
	src/erae_log.cpp \
	src/bookmark.cpp \
	src/lvtoc.cpp \
	src/chmfmt.cpp \
    src/cp_stats.cpp \
    src/crtxtenc.cpp \
    src/epubfmt.cpp \
    src/hyphman.cpp \
    src/lstridmap.cpp \
    src/lvbmpbuf.cpp \
    src/lvdocview.cpp \
    src/crcss.cpp \
    src/lvdrawbuf.cpp \
    src/lvfnt.cpp \
    src/lvfntman.cpp \
    src/lvimg.cpp \
    src/lvpagesplitter.cpp \
    src/lvrend.cpp \
    src/lvstream.cpp \
    src/lvstring.cpp \
    src/lStringCollection.cpp \
    src/lvstsheet.cpp \
    src/lvstyles.cpp \
    src/lvtextfm.cpp \
    src/lvtinydom.cpp \
    src/lvxml.cpp \
    src/pdbfmt.cpp \
    src/props.cpp \
    src/rtfimp.cpp \
    src/txtselector.cpp \
    src/wordfmt.cpp \
    src/docxhandler.cpp \
    src/odthandler.cpp \
    src/RectHelper.cpp \
    src/fb2fmt.cpp \
    src/fb3fmt.cpp \
    src/FootnotesPrinter.cpp \
    src/rtlhandler.cpp \
    src/EpubItems.cpp \
    src/crconfig.cpp \
    src/charProps.cpp \
    src/dvngLig.cpp \
    src/serialBuf.cpp \
    src/indicUtils.cpp

LOCAL_SRC_FILES += \
    src/indic/devanagariManager.cpp \
    src/indic/banglaManager.cpp \
    src/indic/malayalamManager.cpp \
    src/indic/kannadaManager.cpp \
    src/indic/tamilManager.cpp \
    src/indic/teluguManager.cpp \
    src/indic/gujaratiManager.cpp \
    src/indic/oriyaManager.cpp

LOCAL_SRC_FILES += \
    freetype/src/autofit/autofit.c \
    freetype/src/bdf/bdf.c \
    freetype/src/cff/cff.c \
    freetype/src/base/ftbase.c \
    freetype/src/base/ftbbox.c \
    freetype/src/base/ftbdf.c \
    freetype/src/base/ftbitmap.c \
    freetype/src/base/ftgasp.c \
    freetype/src/cache/ftcache.c \
    freetype/src/base/ftglyph.c \
    freetype/src/base/ftgxval.c \
    freetype/src/gzip/ftgzip.c \
    freetype/src/base/ftinit.c \
    freetype/src/lzw/ftlzw.c \
    freetype/src/base/ftmm.c \
    freetype/src/base/ftpatent.c \
    freetype/src/base/ftotval.c \
    freetype/src/base/ftpfr.c \
    freetype/src/base/ftstroke.c \
    freetype/src/base/ftsynth.c \
    freetype/src/base/ftsystem.c \
    freetype/src/base/fttype1.c \
    freetype/src/base/ftwinfnt.c \
    freetype/src/base/ftxf86.c \
    freetype/src/winfonts/winfnt.c \
    freetype/src/pcf/pcf.c \
    freetype/src/pfr/pfr.c \
    freetype/src/psaux/psaux.c \
    freetype/src/pshinter/pshinter.c \
    freetype/src/psnames/psmodule.c \
    freetype/src/raster/raster.c \
    freetype/src/sfnt/sfnt.c \
    freetype/src/smooth/smooth.c \
    freetype/src/truetype/truetype.c \
    freetype/src/type1/type1.c \
    freetype/src/cid/type1cid.c \
    freetype/src/type42/type42.c

LOCAL_SRC_FILES += \
    chmlib/src/chm_lib.c \
    chmlib/src/lzx.c 

LOCAL_SRC_FILES += \
    antiword/asc85enc.c \
    antiword/blocklist.c \
    antiword/chartrans.c \
    antiword/datalist.c \
    antiword/depot.c \
    antiword/doclist.c \
    antiword/fail.c \
    antiword/finddata.c \
    antiword/findtext.c \
    antiword/fontlist.c \
    antiword/fonts.c \
    antiword/fonts_u.c \
    antiword/hdrftrlist.c \
    antiword/imgexam.c \
    antiword/listlist.c \
    antiword/misc.c \
    antiword/notes.c \
    antiword/options.c \
    antiword/out2window.c \
    antiword/pdf.c \
    antiword/pictlist.c \
    antiword/prop0.c \
    antiword/prop2.c \
    antiword/prop6.c \
    antiword/prop8.c \
    antiword/properties.c \
    antiword/propmod.c \
    antiword/rowlist.c \
    antiword/sectlist.c \
    antiword/stylelist.c \
    antiword/stylesheet.c \
    antiword/summary.c \
    antiword/tabstop.c \
    antiword/unix.c \
    antiword/utf8.c \
    antiword/word2text.c \
    antiword/worddos.c \
    antiword/wordlib.c \
    antiword/wordmac.c \
    antiword/wordole.c \
    antiword/wordwin.c \
    antiword/xmalloc.c

include $(BUILD_EXECUTABLE)