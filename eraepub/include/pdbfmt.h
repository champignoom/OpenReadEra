#ifndef PDBFMT_H
#define PDBFMT_H

#include "lvtinydom.h"

bool DetectMOBIFormat(LVStreamRef stream, doc_format_t& contentFormat);
bool ImportMOBIDoc(LVStreamRef& stream, CrDom* doc, doc_format_t& doc_format, bool need_coverpage);
LVStreamRef GetMOBICover(LVStreamRef stream);

#endif // PDBFMT_H
