#ifndef __CHMFMT_H__
#define __CHMFMT_H__

#include "lvtinydom.h"

bool DetectCHMFormat(LVStreamRef stream);
bool ImportCHMDocument(LVStreamRef stream, CrDom* doc, bool cfg_firstpage_thumb_);

/// Opens CHM container
LVContainerRef LVOpenCHMContainer(LVStreamRef stream);

#endif // __CHMFMT_H__
