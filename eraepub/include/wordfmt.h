#ifndef WORDFMT_H
#define WORDFMT_H

#if ENABLE_ANTIWORD==1

#include "lvtinydom.h"

// MS WORD format support using AntiWord library
bool DetectWordFormat(LVStreamRef stream);
// Check if it is RTF using AntiWord library
bool DetectRTFFormat(LVStreamRef stream);

bool ImportWordDocument(LVStreamRef stream, CrDom* dom, bool need_coverpage);

#endif // ENABLE_ANTIWORD==1

#endif // WORDFMT_H
