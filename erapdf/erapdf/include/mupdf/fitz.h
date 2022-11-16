#ifndef MUDPF_FITZ_H
#define MUDPF_FITZ_H

// EraPDF: unset debug flags >>>
#undef  NDK_PROFILER
#undef  MEMENTO
#undef  MEMENTO_ANDROID

#undef  TEST_BUFFER_WRITE
#undef  DUMP_GROUP_BLENDS
#undef  DUMP_STACK_CHANGES
#undef  DEBUG_SCALING
#undef  USE_OUTPUT_DEBUG_STRING
#undef  FITZ_DEBUG_LOCKING
#undef  FITZ_DEBUG_LOCKING_TIMES
#undef  DEBUG_ALIGN
#undef  DEBUG_INTERNALS
#undef  DEBUG_LINE_HEIGHTS
#undef  SPOT_LINE_NUMBERS
#undef  DEBUG_MASKS
#undef  DEBUG_INDENTS
#undef  DEBUG_SCAVENGING

#undef SHARE_JPEG
// EraPDF: unset debug flags <<<

#include "mupdf/fitz/version.h"
#include "mupdf/fitz/system.h"
#include "mupdf/fitz/context.h"

#include "mupdf/fitz/crypt.h"
#include "mupdf/fitz/getopt.h"
#include "mupdf/fitz/hash.h"
#include "mupdf/fitz/math.h"
#include "mupdf/fitz/string.h"
#include "mupdf/fitz/tree.h"
#include "mupdf/fitz/xml.h"

/* I/O */
#include "mupdf/fitz/buffer.h"
#include "mupdf/fitz/stream.h"
#include "mupdf/fitz/compressed-buffer.h"
#include "mupdf/fitz/filter.h"
#include "mupdf/fitz/output.h"
#include "mupdf/fitz/unzip.h"

/* Resources */
#include "mupdf/fitz/store.h"
#include "mupdf/fitz/colorspace.h"
#include "mupdf/fitz/pixmap.h"
#include "mupdf/fitz/glyph.h"
#include "mupdf/fitz/bitmap.h"
#include "mupdf/fitz/image.h"
#include "mupdf/fitz/function.h"
#include "mupdf/fitz/shade.h"
#include "mupdf/fitz/font.h"
#include "mupdf/fitz/path.h"
#include "mupdf/fitz/text.h"

#include "mupdf/fitz/device.h"
#include "mupdf/fitz/display-list.h"
#include "mupdf/fitz/structured-text.h"

#include "mupdf/fitz/transition.h"
#include "mupdf/fitz/glyph-cache.h"

/* Document */
#include "mupdf/fitz/link.h"
#include "mupdf/fitz/outline.h"
#include "mupdf/fitz/document.h"
#include "mupdf/fitz/annotation.h"

#include "mupdf/fitz/write-document.h"

/* Output formats */
#include "mupdf/fitz/output-pnm.h"
#include "mupdf/fitz/output-png.h"
#include "mupdf/fitz/output-pwg.h"
#include "mupdf/fitz/output-pcl.h"
#include "mupdf/fitz/output-svg.h"
#include "mupdf/fitz/output-tga.h"

#endif
