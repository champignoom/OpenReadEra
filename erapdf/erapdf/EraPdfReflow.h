/******************************************************************************
	Copyright (C) 2020 READERA LLC OpenReadEra - Ebook renderer

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
******************************************************************************/
//
// Created by Tarasus on 23/10/2019.
//

#ifndef CODE_THORNYREADER_PURE_MUPDFREFLOW_H
#define CODE_THORNYREADER_PURE_MUPDFREFLOW_H

#include <ore_log.h>
#include "EraPdfBridge.h"
#include <map>
#include <iostream>

#include "../freetype/include/ft2build.h"
#include FT_FREETYPE_H

extern "C" {
#include "include/mupdf/pdf.h"
#include "include/mupdf/fitz.h"
#include "fitz/draw-imp.h"
}

#define SUBSCRIPT_OFFSET 0.2F
#define SUPERSCRIPT_OFFSET -0.2F

#ifdef LCTX
    #undef LCTX
#endif
#define LCTX "MuPdfReflow.h"

using namespace std;

typedef map<float,int> HeightsMap;

class PageStats
{
public:
    int   pagenum       = -1;
    int   charnum       = 0;
    float h1            = -1;
    float h2            = -1;
    int   bannedBlock1  = -1;
    int   bannedBlock2  = -1;
    float width         = 0;
    float height        = 0;
    fz_rect bounds;
    bool hasFullPageImage = false;

    HeightsMap heightsMap;

    PageStats(){};

    PageStats(fz_text_page *page_, int pagenum)
    {
        this->pagenum = pagenum;
        empty = false;
        page = page_;
        analyzePage();
        bounds = page->mediabox;
        width  = bounds.x1 - bounds.x0;
        height = bounds.y1 - bounds.y0;
    }

    void printStats()
    {
        LE("==============================================");
        LE("| PAGE STATS:"                                );
        LE("| blocknum           = %d", page->len         );
        LE("| charnum            = %d", charnum           );
        LE("| h1                 = %f", h1                );
        LE("| h2                 = %f", h2                );
        LE("| top_span_y         = %f", top_span_y        );
        LE("| bottom_span_y      = %f", bottom_span_y     );
        LE("| topSpanBlockNum    = %d", topSpanBlockNum   );
        LE("| bottomSpanBlockNum = %d", bottomSpanBlockNum);
        LE("| bannedBlock1       = %d", bannedBlock1      );
        LE("| bannedBlock2       = %d", bannedBlock2      );
        LE("==============================================");
    }

    bool isEmpty() { return empty;}

private:
    bool empty = true;
    float top_span_y         = 65536;
    float bottom_span_y      = -1;
    int   topSpanBlockNum    = -1;
    int   bottomSpanBlockNum = -1;
    fz_text_page *page;

    void analyzePage();

    void banBlocks();
};

class ReflowManager
{
public:
    ReflowManager(fz_context *ctx, MuPdfBridge *muPdfBridge);
    bool analyzed = false;
    int doctype = REFLOW_UNSUPPORTED;

    void analyzeDocument();

    bool reflowPageMain(int page_index);

    inline void setOutput(fz_output *output) {this->out = output;};

    inline fz_output *getOutput(){return this->out;};
private:
    fz_context *ctx;
    fz_output *out;
    vector<PageStats> pageStatsArray;
    unsigned int pageCount = 0;
    MuPdfBridge* muPdfBridge;

    void freePage(int index);

    void reflowPage(fz_text_page *page, int pagenum, fz_display_list_s *displayList);

    void reflowPageText(fz_text_page *page, int pagenum, fz_display_list_s *displayList);

    void send_img_base64(fz_buffer *buffer);
};

class PageBlock
{
public:
    int index;
    int type;
    fz_rect rect;
    std::wstring text;
    fz_image_block *imgBlock;
    bool draw = true;

    PageBlock() {
        index = -1;
        type = -1;
        rect = fz_empty_rect;
        imgBlock = nullptr;
    }

    template<typename ... Args>
        void printInside(const std::wstring &format, Args ... args)
        {

            wchar_t buf[1024];
            int size = 0;
            size = swprintf(buf, 1024, format.data(), args ...);
            if (size <= 0)
            {
                return;
            }
            std::wstring a = std::wstring(buf, 1024);
            this->text.append(buf, size);
        }
};

#endif //CODE_THORNYREADER_PURE_MUPDFREFLOW_H
