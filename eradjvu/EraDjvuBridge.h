/*
 * Copyright (C) 2013 The DjVU CLI viewer interface Project
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

#ifndef __DJVU_BRIDGE_H__
#define __DJVU_BRIDGE_H__

#include <ddjvuapi.h>

#include "DjvuOutline.h"
#include "ore_log.h"

#include "StBridge.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <codecvt>
#include "StSearchUtils.h"
#include "openreadera.h"

std::wstring djvu_stringToWstring(const std::string& t_str);

std::string  djvu_wstringToString(const std::wstring& t_str);

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

class DjvuBridge : public StBridge
{
private:
    ddjvu_context_t *context;
    ddjvu_document_t *doc;

    uint32_t pageCount;
    ddjvu_pageinfo_t **info;
    ddjvu_page_t **pages;

    DjvuOutline* outline;
    int searchPackCounter = 0;

public:
    DjvuBridge();
    ~DjvuBridge();

    void process(CmdRequest& request, CmdResponse& response);
    static void responseAddString(CmdResponse &response, std::wstring str16);

protected:
    void processOpen(CmdRequest& request, CmdResponse& response);
    void processQuit(CmdRequest& request, CmdResponse& response);
    void processPageInfo(CmdRequest& request, CmdResponse& response);
    void processPage(CmdRequest& request, CmdResponse& response);
    void processPageLinks(CmdRequest& request, CmdResponse& response);
    void processPageRender(CmdRequest& request, CmdResponse& response);
    void processSmartCrop(CmdRequest& request, CmdResponse& response);
    void processPageFree(CmdRequest& request, CmdResponse& response);
    void processOutline(CmdRequest& request, CmdResponse& response);
    void processPageText(CmdRequest& request, CmdResponse& response);
    void processXPathByRectId(CmdRequest &request, CmdResponse &response);
    void processTextSearchPreviews(CmdRequest &request, CmdResponse &response);
    void processTextSearchHitboxes(CmdRequest &request, CmdResponse &response);
    void processRtlText(CmdRequest &request, CmdResponse &response);

    ddjvu_pageinfo_t* getPageInfo(uint32_t pageNo);
    ddjvu_page_t* getPage(uint32_t pageNo, bool decode);

    void processLinks(int pageNo, CmdResponse& response);
    void processText(int pageNo, const char* pattern, CmdResponse& response);
    bool retryProcessTextCharMode(int pageNo, const char *pattern, CmdResponse &response);

    void processSearchCounter(CmdRequest &request, CmdResponse &response);
    void processPageRangeText(CmdRequest &request, CmdResponse &response);

    void waitAndHandleMessages();

    std::vector<Hitbox> processTextToArray(int pageNo);
    std::vector<Hitbox> retryProcessTextToArrayCharMode(int pageNo);

    std::string GetXpathFromPageById(int page, int id, bool addcoords);
    std::string GetXpathFromPageById(std::vector<Hitbox> hitboxes, int id, bool addcoords);
    std::vector<std::string> GetXpathFromPageById(std::vector<Hitbox> hitboxes, std::vector<int> pos_arr, bool addcoords, int qlen);

    std::wstring getPageText(int page);

    //search functions
    std::vector<SearchResult> SearchForTextPreviews(int page, std::wstring query);

    std::vector<SearchResult> FindAndTrim(std::wstring query, int page, int end);

    std::vector<Hitbox> SearchForTextHitboxes(int page, std::wstring query);

    std::vector<Hitbox> GetSearchHitboxes(std::vector<Hitbox> hitboxes, int page, std::wstring query);

    std::vector<Hitbox> GetHitboxesBetweenXpaths(uint32_t page, std::string basic_string, std::string basicString, int startPage);

    /*
    int GetSearchHitboxesNextPageLite(int page, std::wstring query);

    std::vector<Hitbox> GetSearchHitboxesPrevPage(std::vector<Hitbox> base, std::wstring query, int last_len);

    SearchResult FindAndTrimNextPage(std::wstring query, int page, int start_last);

    std::vector<Hitbox> GetSearchHitboxesNextPage(std::vector<Hitbox> base, int page, std::wstring query);
    */
};

#endif
