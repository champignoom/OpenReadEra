#ifndef MUPDF_PDF_H
#define MUPDF_PDF_H

// EraPDF: unset debug flags >>>
#undef DEBUG_HEAP_SORT
#undef DEBUG_LINEARIZATION
#undef DEBUG_WRITING
#undef DEBUG_PROGESSIVE_ADVANCE
// EraPDF: unset debug flags <<<

#include "mupdf/fitz.h"

#include "mupdf/pdf/name-table.h"
#include "mupdf/pdf/object.h"
#include "mupdf/pdf/document.h"
#include "mupdf/pdf/parse.h"
#include "mupdf/pdf/xref.h"
#include "mupdf/pdf/crypt.h"

#include "mupdf/pdf/page.h"
#include "mupdf/pdf/resource.h"
#include "mupdf/pdf/cmap.h"
#include "mupdf/pdf/font.h"
#include "mupdf/pdf/interpret.h"

#include "mupdf/pdf/annot.h"
#include "mupdf/pdf/field.h"
#include "mupdf/pdf/widget.h"
#include "mupdf/pdf/appearance.h"
#include "mupdf/pdf/event.h"
#include "mupdf/pdf/javascript.h"

#include "mupdf/pdf/output-pdf.h"

#include "mupdf/pdf/clean.h"

#endif
