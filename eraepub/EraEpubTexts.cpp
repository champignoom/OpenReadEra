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

#include "EraEpubBridge.h"

void CreBridge::processPageText(CmdRequest& request, CmdResponse& response)
{
#ifdef OREDEBUG
#define DEBUG_TEXT
#endif //OREDEBUG
    response.cmd = CMD_RES_PAGE_TEXT;
    CmdDataIterator iter(request.first);
    uint32_t external_page = 0;
    iter.getInt(&external_page);
    if (!iter.isValid())
    {
        CRLog::error("processPageText bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    auto page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    doc_view_->GoToPage(page);
#ifdef DEBUG_TEXT
    CRLog::debug("processPageText external_page=%d page=%d page_width=%d page_height=%d",
            external_page, page, doc_view_->GetWidth(), doc_view_->GetHeight());
#endif
    LVArray<Hitbox> hitboxes = doc_view_->GetPageHitboxes();
    for (int i = 0; i < hitboxes.length(); i++)
    {
        Hitbox currHitbox = hitboxes.get(i);
        if(gJapaneseVerticalMode)
        {
            float t = currHitbox.top_;
            float b = currHitbox.bottom_;

            t -=0.5f;
            b -=0.5f;

            t = (t<0)? fabs(t): -t;
            b = (b<0)? fabs(b): -b;

            t +=0.5f;
            b +=0.5f;

            response.addFloat(b);
            response.addFloat(currHitbox.left_);
            response.addFloat(t);
            response.addFloat(currHitbox.right_);
        }
        else
        {
            response.addFloat(currHitbox.left_);
            response.addFloat(currHitbox.top_);
            response.addFloat(currHitbox.right_);
            response.addFloat(currHitbox.bottom_);
        }

        responseAddString(response, currHitbox.text_.restoreIndicText());
    }
#undef DEBUG_TEXT
}

struct SortStruct {
    lString16 text;
    long long int weight;
};

int compareByWeight(const void *a, const void *b)
{
    auto ia = (struct SortStruct*) a;
    auto ib = (struct SortStruct*) b;
    return (int) (ia->weight - ib->weight);
}

void CreBridge::processRtlText(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_RTL_TEXT;

    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        CRLog::error("processPageXpaths bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    lString16 key_input = lString16(val);

    //page:startindex:endindex
    //lString16 key_input = lString16("1:0:40");

    lString16Collection keys;
    keys.split(key_input,lString16(":"));

    uint32_t external_page = keys.at(0).atoi();
    uint32_t id_start = keys.at(1).atoi();
    uint32_t id_end   = keys.at(2).atoi();

    uint32_t page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    doc_view_->GoToPage(page);
    LVArray<Hitbox> hitboxes = doc_view_->GetPageHitboxes();
    LVArray<SortStruct> array;
    long long int lastindex;

    for (int i = id_start; i <= id_end; i++)
    {
        Hitbox hb = hitboxes.get(i);
        long long int index;
        if (hb.getNode()->isNull())
        {
            index = lastindex;
        }
        else
        {
            index = hb.getNode()->getDataIndex() * 1000000;
            index += hb.word_.getStartXPointer().getOffset();
            lastindex = index;
        }
        SortStruct str;
        str.text = hb.text_;
        str.weight = index;
        array.add(str);
    }

    qsort(array.get(), array.length(), sizeof(struct SortStruct), compareByWeight);

    lString16 result;

    for (int i = 0; i < array.length(); i++)
    {
        result+= array.get(i).text;
    }
    //CRLog::error("processRtlText in = [%s]",LCSTR(result));

    result = result.ReversePrettyLetters();
    result = result.restoreIndicText();
    //CRLog::error("processRtlText out = [%s]",LCSTR(result));

    //CRLog::error("str = [%s]",LCSTR(result.processOriyaText()));

    responseAddString(response,result);
}

//searches for the specified rectangle on page, returns xpath if found.
void CreBridge::processXPathByHitbox(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_XPATH_BY_HITBOX;

    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        CRLog::error("processXPathByHitbox bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    lString16 key_input = lString16(val);
    //lString16 key_input = lString16("3;0.10375:0.12000:0.81115:0.83306");
    lString16Collection keys;
    keys.split(key_input,lString16(";"));

    uint32_t external_page = keys.at(0).atoi();
    lString16 hitbox = keys.at(1);
    uint32_t page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    ldomWordMap m = doc_view_->GetldomWordMapFromPage(page);

    lString16 xp1 = doc_view_->GetXpathByRectCoords(hitbox, m);
    responseAddString(response, xp1);
}

//searches for the specified rectangle on page (by number), returns xpath if found.
void CreBridge::processXPathByRectId(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_XPATH_BY_RECT_ID;

    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        CRLog::error("processXPathByHitbox bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    lString16 key_input = lString16(val);

    //lString16 key_input = lString16("3:132:0"); //page:num:end

    lString16Collection keys;
    keys.split(key_input,lString16(":"));

    uint32_t external_page = keys.at(0).atoi();
    uint32_t id = keys.at(1).atoi();
    bool endXpath = false;
    if(keys.length()>2)
    {
        endXpath = (bool) keys.at(2).atoi();
    }
    uint32_t page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    lString16 xpath = doc_view_->GetXpathFromPageById(page, id, endXpath );

    responseAddString(response, xpath);
}

//returns hitboxes of letters between the specified xpointers
void CreBridge::processPageRangeText(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_RANGE_HITBOX;

    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        CRLog::error("processPageRangeText bad request data : invalid iterator");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    lString16 key_input = lString16(val);

    //"page;start;end"
    //lString16 key_input = lString16("0;"
    //                                "/body/body[1]/p[11]/h6/text().0;"
    //                                "/body/body[1]/p[44]/a/text().1");

    lString16Collection keys;
    keys.split(key_input,lString16(";"));

    if (keys.at(0).empty() || keys.at(1).empty() || keys.at(2).empty() )
    {
        CRLog::error("processPageRangeText bad request data : invalid key string [%s]",
                     LCSTR(key_input));
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    int external_page = keys.at(0).atoi();
    lString16 startstr  = keys.at(1);
    lString16 endstr    = keys.at(2);
    if( external_page < 0 || external_page >= doc_view_->GetPagesCount() )
    {
        CRLog::error("processPageRangeText: External page %d < 0 OR %d >= %d",
                     external_page,external_page,doc_view_->GetPagesCount());
        return;
    }

    int page = external_page;
    doc_view_->GoToPage(page);

    ldomXPointer start  = doc_view_->GetCrDom()->createXPointer(startstr);
    ldomXPointer end    = doc_view_->GetCrDom()->createXPointer(endstr);
    int startpage = doc_view_->GetPageForBookmark(start);
    int endpage = doc_view_->GetPageForBookmark(end);
    //CRLog::error("processPageRangeText key_input = %s",LCSTR(key_input));
    LVArray<Hitbox> BookmarkHitboxes;
    ldomXRange* range;

    // out of selection range
    if (page < startpage || page > endpage)
    {
        CRLog::error("processPageRangeTextSelection out of range : page (%d) < startpage(%d) OR page(%d) > endpage(%d)",
                     page,startpage,page,endpage);
        return;
    }
    //CRLog::error("processPageRangeText page (%d), startpage(%d), endpage(%d)",page,startpage,endpage);
    //exactly on one current page
    if (startpage == page && endpage == page)
    {
        range = new ldomXRange(start, end);
    }
    //selection goes lower
    if (startpage == page && endpage > page)
    {
        ldomXPointer page_end = doc_view_->GetPageDocRange(page,true).get()->getEnd();
        range = new ldomXRange(start, page_end);
    }
    //selection goes upper
    if (startpage < page && endpage == page)
    {
        ldomXPointer page_start = doc_view_->GetPageDocRange(page,true).get()->getStart();
        range = new ldomXRange(page_start, end);
    }
    //selection goes upper and lower
    if (startpage < page && endpage > page)
    {
        ldomXPointer page_start = doc_view_->GetPageDocRange(page,true).get()->getStart();
        ldomXPointer page_end = doc_view_->GetPageDocRange(page,true).get()->getEnd();
        range = new ldomXRange(page_start, page_end);
    }

    //CRLog::error("processPageRangeText range = %s : %s",LCSTR(range->getStart().toString()),LCSTR(range->getEnd().toString()));
    LVArray<Hitbox> FilteredHitboxes;
    if (gDocumentRTL)
    {
        FilteredHitboxes = doc_view_->GetPageHitboxesRTL(range, page);

        Hitbox smallFirst, smallLast;
        smallFirst = FilteredHitboxes.get(0);
        smallLast = FilteredHitboxes.get(FilteredHitboxes.length()-1);

        // making left == right for selection marks to work properly
        smallFirst.left_ = smallFirst.right_;
        smallLast.right_ = smallLast.left_;

        FilteredHitboxes = doc_view_->unionRects(FilteredHitboxes);

        // adding those hitboxes for rtl detection
        if (smallFirst.getNode()->isRTL()) {FilteredHitboxes.insert(0, smallFirst);}
        if (smallLast.getNode()->isRTL())  {FilteredHitboxes.add(smallLast);}
    }
    else
    {
        FilteredHitboxes = doc_view_->GetPageHitboxes(range);
        FilteredHitboxes = doc_view_->unionRects(FilteredHitboxes);
    }
    float offset = 0;
    if(doc_view_->GetColumns() > 1 && external_page % 2 == 1)
    {
        offset = 0.5f;
    }

    lString16 resp_key;
    resp_key.append(startstr);
    resp_key.append(":");
    resp_key.append(endstr);

    for (int i = 0; i < FilteredHitboxes.length(); i++)
    {
        Hitbox currHitbox = FilteredHitboxes.get(i);
        if(gJapaneseVerticalMode)
        {
            float t = currHitbox.top_;
            float b = currHitbox.bottom_;

            t -=0.5f;
            b -=0.5f;

            t = (t<0)? fabs(t): -t;
            b = (b<0)? fabs(b): -b;

            t +=0.5f;
            b +=0.5f;

            response.addFloat(b + offset);
            response.addFloat(currHitbox.left_);
            response.addFloat(t + offset);
            response.addFloat(currHitbox.right_);
        }
        else
        {
            response.addFloat(currHitbox.left_ + offset);
            response.addFloat(currHitbox.top_);
            response.addFloat(currHitbox.right_ + offset);
            response.addFloat(currHitbox.bottom_);
        }
        responseAddString(response, resp_key);
    }
}

//return page number by xpath / xpointer
void CreBridge::processPageByXPath(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_CRE_PAGE_BY_XPATH;
    CmdDataIterator iter(request.first);
    uint8_t* xpath_string;
    iter.getByteArray(&xpath_string);
    if (!iter.isValid()) {
        CRLog::error("processPageByXPath invalid iterator");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    lString16 xpath(reinterpret_cast<const char*>(xpath_string));
    ldomXPointer bm = doc_view_->GetCrDom()->createXPointer(xpath);
    if (bm.isNull()) {
        CRLog::error("processPageByXPath bad xpath bm.isNull()");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    doc_view_->GoToBookmark(bm);
    int current_page = doc_view_->GetCurrPage();
    if (current_page < 0) {
        CRLog::error("processPageByXPath bad xpath current_page < 0");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    response.addInt((uint32_t) ExportPage(doc_view_->GetColumns(), current_page));
}

//return page number by xpath / xpointer
void CreBridge::processPageByXPathMultiple(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_CRE_PAGE_BY_XPATH_MULT;
    CmdDataIterator iter(request.first);
    uint8_t* in_string;
    iter.getByteArray(&in_string);
    if (!iter.isValid()) {
        CRLog::error("processPageByXPathMultiple invalid iterator");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    lString16 in(reinterpret_cast<const char*>(in_string));
    lString16Collection paths;
    paths.parse(in,L";",true);

    lString16 pages_str;
    for (int i = 0; i < paths.length(); i++)
    {
        lString16 xpath = paths.at(i);
        ldomXPointer bm = doc_view_->GetCrDom()->createXPointer(xpath);
        if (bm.isNull())
        {
            pages_str += L"-1;";
            CRLog::error("processPageByXPathMultiple bad xpath bm.isNull()");
            continue;
        }
        doc_view_->GoToBookmark(bm);
        int current_page = doc_view_->GetCurrPage();
        if (current_page < 0)
        {
            pages_str += L"-1;";
            CRLog::error("processPageByXPathMultiple bad xpath current_page < 0");
            continue;
        }
        pages_str += lString16::itoa((uint32_t) ExportPage(doc_view_->GetColumns(), current_page));
        pages_str += L";";
    }
    if (!pages_str.empty())
    {
        pages_str = pages_str.substr(0, pages_str.length() - 1);
        //CRLog::error("pages_str = %s",LCSTR(pages_str));
        responseAddString(response, pages_str);
    }
}

//return page xpath by number
void CreBridge::processPageXPath(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_CRE_PAGE_XPATH;
    CmdDataIterator iter(request.first);
    uint32_t page;
    iter.getInt(&page);
    if (!iter.isValid()) {
        CRLog::error("processPageXPath invalid iterator");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    ldomXPointer xptr = doc_view_->getPageBookmark(ImportPage(page, doc_view_->GetColumns()));
    if (xptr.isNull()) {
        CRLog::error("processPageXPath null ldomXPointer");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    responseAddString(response, xptr.toString());
}

