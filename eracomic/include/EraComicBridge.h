/*
 * Copyright (C) 2013-2021 READERA LLC
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
 * Developers: ReadEra Team (2013-2021), Playful Curiosity (2013-2021),
 * Tarasus (2018-2021).
 */
//
// Created by Tarasus on 22.12.2020.
//

#ifndef ERACOMICBRIDGE_H
#define ERACOMICBRIDGE_H

#include "../../orebridge/include/StBridge.h"
#include "../../orebridge/include/StSearchUtils.h"
#include "../../orebridge/include/openreadera.h"
#include "include/EraComicManager.h"

typedef struct matrix_s matrix;
struct matrix_s
{
    float a; // sliceX
    float b; // sliceY
    float c; // sliceWidth
    float d; // sliceHeight
    float e; // (unused) slice right
    float f; // (unused) slice bottom
};
const matrix default_matrix = { 0, 0, 1, 1, 0, 0 };

class EraComicBridge : public StBridge
{
private:
    int format;
    ComicManager* comicManager;
public:
    EraComicBridge();
    ~EraComicBridge();
    void process(CmdRequest& request, CmdResponse& response);
protected:

    void processOpen(CmdRequest& request, CmdResponse& response);
    void processQuit(CmdRequest& request, CmdResponse& response);
    void processPage(CmdRequest &request, CmdResponse &response);
    void processPageFree(CmdRequest &request, CmdResponse &response);
    void processPageRender(CmdRequest& request, CmdResponse& response);
    void processPageInfo(CmdRequest& request, CmdResponse& response);
    void processSmartCrop(CmdRequest& request, CmdResponse& response);

    /*
    void processPageText(CmdRequest& request, CmdResponse& response);
    void processFontsConfig(CmdRequest& request, CmdResponse& response);
    void processConfig(CmdRequest& request, CmdResponse& response);
    void processStorage(CmdRequest& request, CmdResponse& response);
    void processSystemFont(CmdRequest& request, CmdResponse& response);
    void processGetMissedFonts(CmdRequest& request, CmdResponse& response);
    void processGetLayersList(CmdRequest& request, CmdResponse& response);
    void processSetLayersMask(CmdRequest& request, CmdResponse& response);
    void processXPathByRectId(CmdRequest &request, CmdResponse &response);
    void processAnalyzeDocForReflow(CmdRequest& request, CmdResponse& response);
    void processDocToFB2(CmdRequest& request, CmdResponse& response);
    void processXpathByCoords(CmdRequest& request, CmdResponse& response);
    void processTextSearchHitboxes(CmdRequest &request, CmdResponse &response);
    void processPageRangeText(CmdRequest &request, CmdResponse &response);
    */

    //Stubs for compatibility
    void processOutline(CmdRequest& request, CmdResponse& response);
    void processPageLinks(CmdRequest &request, CmdResponse &response);
    void processPageText(CmdRequest &request, CmdResponse &response);
    void processSearchCounter(CmdRequest &request, CmdResponse &response);
    void processTextSearchPreviews(CmdRequest &request, CmdResponse &response);


    bool renderPage(uint32_t page_index, uint32_t w, uint32_t h, uint8_t *pixels, matrix_s transform_matrix, bool preview);

    void processConfig(CmdRequest &request, CmdResponse &response);

    int checkArchiveFormat(std::string, int fd);

    void processImageHitbox(CmdRequest &request, CmdResponse &response);

    void processImageByXpath(CmdRequest &request, CmdResponse &response);

    void processRarInfo(CmdRequest &request, CmdResponse &response);

    void processRarExtract(CmdRequest &request, CmdResponse &response);
};

#endif //ERACOMICBRIDGE_H
