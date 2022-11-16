/*
 * Copyright (C) 2013 The MuPDF CLI viewer interface Project
 * Copyright (C) 2013-2020 READERA LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MUPDF_BRIDGE_H__
#define __MUPDF_BRIDGE_H__

#include <string>
#include <set>
#include <vector>

extern "C" {
#include <mupdf/pdf.h>
#include <mupdf/xps.h>
#include <../erapdf/include/mupdf/fitz/output.h>
};

#define FORMAT_PDF 1
#define FORMAT_XPS 2

#include "StBridge.h"
#include "StSearchUtils.h"
#include "openreadera.h"

#define CURRENT_MAX_VERSION 2

class EraConfig{
public:
    int erapdf_twilight_mode = 0;
    float f_font_color[3] = {1,1,1};
    float f_bg_color[3]   = {0,0,0};

/*
    #if OREDEBUG
    EraConfig()
    {
        erapdf_twilight_mode = 1;
        const float FLOAT_COEFF = 0.00390625f; // == 1/256

        unsigned int c = 0x2b2b2b;
        f_bg_color[0] = ((c >> 16) & 0xFF) * FLOAT_COEFF ; //r
        f_bg_color[1] = ((c >> 8 ) & 0xFF) * FLOAT_COEFF ; //g
        f_bg_color[2] = ( c        & 0xFF) * FLOAT_COEFF ; //b

        //c = 0xC7BAAA; //жёлтый
        c = 0xA9B7C6; //синий
        //c = 0xC5C6C7; //серый
        f_font_color[0] = ((c >> 16) & 0xFF) * FLOAT_COEFF ; //r
        f_font_color[1] = ((c >> 8 ) & 0xFF) * FLOAT_COEFF ; //g
        f_font_color[2] = ( c        & 0xFF) * FLOAT_COEFF ; //b
    }
    #endif
*/

    void applyToCtx(fz_context *ctx)
    {
        ctx->erapdf_twilight_mode = erapdf_twilight_mode;

        ctx->f_font_color[0] = f_font_color[0];
        ctx->f_font_color[1] = f_font_color[1];
        ctx->f_font_color[2] = f_font_color[2];

        ctx->f_bg_color[0] = f_bg_color[0];
        ctx->f_bg_color[1] = f_bg_color[1];
        ctx->f_bg_color[2] = f_bg_color[2];
    };
};

class ReflowManager;

class SearchResult{
public:
    SearchResult(){};
    SearchResult(std::wstring pre, std::vector<std::string> xps)
    {
        preview_ = pre;
        xpointers_ = xps;
    }
    std::wstring preview_;
    std::vector<std::string> xpointers_;
};

class SearchWindow
{
public:
    int start_ = -1;
    int pos_   = -1;
    int end_   = -1;
    int startpos_   = -1;
    std::vector<Hitbox> text_;
    std::wstring query_;

    std::vector<std::string> xp_array_;
    SearchWindow() {};

    SearchWindow(std::vector<Hitbox> & text, std::wstring& query, int startpos)
    {
        startpos_ = startpos;
        text_  = text;
        query_ = query;
    }

    void addpos(int pos, std::string xp)
    {
        pos_ = pos;
        xp_array_.push_back(xp);
        if( start_==-1 && end_ == -1) // if not initialised
        {
            init();
        }
        else
        {
            run();
        }
    }

    void init()
    {
        int counter = 0;
        int backoffset = pos_;
        int frontoffset = pos_ + query_.length();

        int addcount = 0;
        while(counter < TEXT_SEARCH_PREVIEW_WORD_NUM)
        {
            backoffset--;
            if(backoffset <= 0)// || text.at(backoffset)==L'\n')
            {
                backoffset = 0;
                addcount = TEXT_SEARCH_PREVIEW_WORD_NUM - counter;
                break;
            }
            if(backoffset <= startpos_)
            {
                addcount = TEXT_SEARCH_PREVIEW_WORD_NUM - counter;
                break;
            }
            if(text_.at(backoffset).text_.at(0)==L' ')
            {
                counter++;
            }
        }
        counter = 0;
        while (counter < TEXT_SEARCH_PREVIEW_WORD_NUM + addcount)
        {
            frontoffset++;
            if(frontoffset>=text_.size())// || text.at(frontoffset)==L'\n')
            {
                frontoffset = text_.size();
                break;
            }
            if(text_.at(frontoffset).text_.at(0)==L' ')
            {
                counter++;
            }
        }
        start_ = backoffset;
        end_ = frontoffset;
    }

    void run(){
        if(pos_ + query_.length() > end_)
        {
            end_ = pos_ + query_.length();
        }
        if(end_< text_.size())
        {
            for (int i = end_ ; i <  text_.size(); i++)
            {
                if(text_.at(i).text_.at(0) == L' ')
                {
                    end_ = i;
                    break;
                }
            }
        }
    };

    bool contains(int pos)
    {
        if( start_==-1 && end_ == -1) // if not initialised
        {
            return  true;
        }
        return (pos >= start_ && pos <= end_);
    }
};

class PageHitboxesCash
{
private:
    int page_ = -1;
    std::vector<Hitbox> PageHitboxes_;
public:
    PageHitboxesCash(){};
    PageHitboxesCash(int page, std::vector<Hitbox> Hitboxes){
        page_ = page;
        PageHitboxes_ = Hitboxes;
    }

    int getPage()
    {
        return page_;
    }

    std::vector<Hitbox> getArray()
    {
        return PageHitboxes_;
    };

    Hitbox get(int num)
    {
        return PageHitboxes_.at(num);
    }

    int length() {
        return PageHitboxes_.size();
    }

    bool isEmpty()
    {
        return ((page_ == -1) || (PageHitboxes_.empty()));
    }

    void reset()
    {
        page_ = -1;
        PageHitboxes_.clear();
    }
};
class ReflowManager;
class MuPdfBridge : public StBridge
{
private:
    int config_format = 0;
    int config_invert_images = 0;
	int fd;
    char* password;

    fz_context *ctx;
    fz_document *document;
    fz_outline *outline;

    uint32_t pageCount;
    fz_page **pages;
    fz_display_list **pageLists;

    int storememory;
    int format;
    int layersmask;

    int searchPackCounter = 0;
    std::set<std::string> fonts;
    std::vector<PageHitboxesCash> pagesCache;
    ReflowManager* reflowManager;
    EraConfig eraConfig;
public:
    MuPdfBridge();
    ~MuPdfBridge();

    void process(CmdRequest& request, CmdResponse& response);


protected:
    void processOpen(CmdRequest& request, CmdResponse& response);
    void processQuit(CmdRequest& request, CmdResponse& response);
    void processPageInfo(CmdRequest& request, CmdResponse& response);
    void processPage(CmdRequest& request, CmdResponse& response);
	void processPageLinks(CmdRequest& request, CmdResponse& response);
    void processPageRender(CmdRequest& request, CmdResponse& response);
    void processPageFree(CmdRequest& request, CmdResponse& response);
    void processOutline(CmdRequest& request, CmdResponse& response);
    void processPageText(CmdRequest& request, CmdResponse& response);
    void processFontsConfig(CmdRequest& request, CmdResponse& response);
    void processConfig(CmdRequest& request, CmdResponse& response);
    void processStorage(CmdRequest& request, CmdResponse& response);
    void processSystemFont(CmdRequest& request, CmdResponse& response);
    void processGetMissedFonts(CmdRequest& request, CmdResponse& response);
    void processGetLayersList(CmdRequest& request, CmdResponse& response);
    void processSetLayersMask(CmdRequest& request, CmdResponse& response);
	void processSmartCrop(CmdRequest& request, CmdResponse& response);
	void processXPathByRectId(CmdRequest &request, CmdResponse &response);

    void processAnalyzeDocForReflow(CmdRequest& request, CmdResponse& response);
    void processDocToFB2(CmdRequest& request, CmdResponse& response);
    void processXpathByCoords(CmdRequest& request, CmdResponse& response);

    friend class ReflowManager;


    fz_page* getPage(uint32_t index, bool decode);
    bool renderPage(uint32_t index, int w, int h, unsigned char* pixels, const fz_matrix_s* ctm);
    bool restart();
    void release();
    void resetFonts();
    void processLinks(int pageNo, CmdResponse& response);
    void processOutline(fz_outline *outline, int level, int index, CmdResponse& response);
    void processTextSearchPreviews(CmdRequest &request, CmdResponse &response);
    void processTextSearchHitboxes(CmdRequest &request, CmdResponse &response);
    void processSearchCounter(CmdRequest &request, CmdResponse &response);
    void processPageRangeText(CmdRequest &request, CmdResponse &response);
    void applyLayersMask();

    std::string GetXpathFromPageById(std::vector<Hitbox> hitboxes, int id, bool addcoords, bool reverse);
    std::string GetXpathFromPageById(int page, int id, bool addcoords, bool reverse);
    std::vector<std::string> GetXpathFromPageById(std::vector<Hitbox> hitboxes, std::vector<int> pos_arr, bool addcoords, int qlen);
    std::vector<Hitbox> processTextToArray(int pageNo, int version = CURRENT_MAX_VERSION);
    std::vector<Hitbox> processTextToArray_imp(int pageNo, int version = CURRENT_MAX_VERSION);
    std::string getPageText(int pageNo);
    // Search functions
    std::vector<SearchResult> SearchForTextPreviews(int page, std::wstring query);
    std::vector<SearchResult> FindAndTrim(std::wstring query, int page, int &end);
    std::vector<Hitbox> GetSearchHitboxes(std::vector<Hitbox> hitboxes, int page, std::wstring query);
    std::vector<Hitbox> SearchForTextHitboxes(int page, std::wstring query);
    std::vector<Hitbox> GetHitboxesBetweenXpaths(int page, std::string xpStart, std::string xpEnd, int startPage, int version);
    /*
    SearchResult FindAndTrimNextPage(std::wstring query, int page, int start_last);
    int GetSearchHitboxesNextPageLite(int page, std::wstring query);
    std::vector<Hitbox> GetSearchHitboxesPrevPage(std::vector<Hitbox> base, std::wstring query, int last_len);
    std::vector<Hitbox> GetSearchHitboxesNextPage(std::vector<Hitbox> base, int page, std::wstring query);
    */
    void analyzePageForDarkMode(fz_context *ctx, int index, int w, int h);

    std::string GetXpathFromPageByCoords(int page, float x, float y, bool addcoords, bool reverse);

    void processRtlText(CmdRequest &request, CmdResponse &response);
};

bool isQuote(int ch);

#endif
