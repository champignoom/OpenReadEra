/*
 * Copyright (C) 2013-2020 READERA LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Developers: ReadEra Team (2013-2020), Playful Curiosity (2013-2020),
 * Tarasus (2018-2020).
 */

#ifndef _ERAEPUB_BRIDGE_H_
#define _ERAEPUB_BRIDGE_H_

#include "openreadera.h"
#include "StBridge.h"
#include "include/lvdocview.h"

typedef std::map<int, ldomWord> ldomWordMap;

class CreBridge : public StBridge {
private:
    LVDocView* doc_view_;
public:
    CreBridge();

    ~CreBridge();

    void process(CmdRequest& request, CmdResponse& response);

protected:
    uint32_t ExportPagesCount(int columns, int pages);

    int ExportPage(int columns, int page);

    int ImportPage(int columns, int page);

    void responseAddString(CmdResponse& response, const lString16& str16);

    void convertBitmap(LVColorDrawBuf* bitmap);

    void responseAddLinkUnknown(CmdResponse& response, const lString16& href,
                                float l, float t, float r, float b);

    void processFontsConfig(CmdRequest& request, CmdResponse& response);

    void processConfig(CmdRequest& request, CmdResponse& response);

    void processOpen(CmdRequest& request, CmdResponse& response);

    void processQuit(CmdRequest& request, CmdResponse& response);

    void processOutline(CmdRequest& request, CmdResponse& response);

    void processPageLinks(CmdRequest& request, CmdResponse& response);

    void processPageText(CmdRequest& request, CmdResponse& response);

    void processPageRender(CmdRequest& request, CmdResponse& response);

    void processPageByXPath(CmdRequest& request, CmdResponse& response);

    void processPageByXPathMultiple(CmdRequest& request, CmdResponse& response);

    void processPageXPath(CmdRequest& request, CmdResponse& response);

    void processMeta(CmdRequest &request, CmdResponse &response);

    void processXPathByHitbox(CmdRequest& request, CmdResponse& response);

    void processXPathByRectId(CmdRequest& request, CmdResponse& response);

    void processPageRangeText(CmdRequest& request, CmdResponse& response);

    void processTextSearchHitboxes(CmdRequest& request, CmdResponse& response);

    void processRtlText(CmdRequest& request, CmdResponse& response);

    void processTextSearchPreviews(CmdRequest& request, CmdResponse& response);

    void processTextSearchGetSuggestionsIndex(CmdRequest& request, CmdResponse& response);

    void processImagesXpaths(CmdRequest& request, CmdResponse& response);

    void processImageByXpath(CmdRequest& request, CmdResponse& response);

    void processImageHitbox(CmdRequest& request, CmdResponse& response);

    void processFontNames(CmdRequest &request, CmdResponse &response);

    void processFontLigmap(CmdRequest &request, CmdResponse &response);
};

#endif //_ERAEPUB_BRIDGE_H_
