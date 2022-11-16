/*
 * Copyright (C) 2013-2020 READERA LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Developers: ReadEra Team (2013-2020), Tarasus (2018-2020).
 */

///TODO: Rename this source file to EraPdfSearch.cpp


#include <string>
#include <vector>
#include <sstream>

#include "ore_log.h"
#include "StProtocol.h"

#include "EraPdfBridge.h"
#include "StSearchUtils.h"

std::string MuPdfBridge::GetXpathFromPageById(int page, int id, bool addcoords, bool reverse)
{
    std::vector<Hitbox> hitboxes = processTextToArray(page);
    return GetXpathFromPageById(hitboxes, id, addcoords, reverse);
}

std::string MuPdfBridge::GetXpathFromPageById(std::vector<Hitbox> hitboxes, int id, bool addcoords, bool reverse)
{
    std::wstring slashn(L"\n");
    if (id >= 0 && id < hitboxes.size())
    {
        if (reverse)
        {
            while (hitboxes.at(id).text_.compare(slashn) == 0 && id >= 0)
            {
                id--;
            }
        }
        else
        {
            while (hitboxes.at(id).text_.compare(slashn) == 0 && id < hitboxes.size())
            {
                id++;
            }
        }
        std::string xpath = hitboxes.at(id).getXPointer();
        if(xpath.empty())
        {
            return std::string("-");
        }
        if(addcoords)
        {
            float x = hitboxes.at(id).left_;
            float y = hitboxes.at(id).top_;

            std::string xstr = std::to_string(x).substr(0, 6);
            std::string ystr = std::to_string(y).substr(0, 6);

            std::string coords = "@" + xstr + ":" + ystr;

            xpath += coords;
        }
        return xpath;
    }
    return std::string("-");
}

std::vector<std::string> MuPdfBridge::GetXpathFromPageById(std::vector<Hitbox> hitboxes, std::vector<int> pos_arr, bool addcoords, int qlen)
{
    std::vector<std::string> result;
    std::wstring slashn = std::wstring('\n',1);
    for (int i = 0; i < pos_arr.size(); i++)
    {
        std::string posresult;
        int pos = pos_arr.at(i);
        if (pos >= 0 && pos <= hitboxes.size())
        {
            if(hitboxes.at(pos).text_ == slashn && pos > 0)
            {
                pos--;
            }

            std::string xp = hitboxes.at(pos).getXPointer();
            posresult = xp;
            float startx = -1;
            float starty = -1;
            if(addcoords)
            {
                startx = hitboxes.at(pos).left_;
                starty = hitboxes.at(pos).top_;

                std::string xstr = std::to_string(startx).substr(0, 6);
                std::string ystr = std::to_string(starty).substr(0, 6);

                std::string coords = "@" + xstr + ":" + ystr;
                posresult += coords;
            }

            pos += qlen;
            if (pos >= 0 && pos <= hitboxes.size())
            {
                if (addcoords)
                {
                    float endx = hitboxes.at(pos).left_;
                    float endy = hitboxes.at(pos).top_;
                    if (startx <0.5f && endx > 0.5f)
                    {
                        std::string xstr = std::to_string(endx).substr(0, 6);
                        std::string ystr = std::to_string(endy).substr(0, 6);

                        std::string coords = "@" + xstr + ":" + ystr;
                        posresult += "<=>" + xp + coords;
                    }
                }
            }
            result.push_back(posresult);
        }
        else
        {
            result.push_back(std::string("-"));
        }
    }
    return result;
}

std::vector<SearchResult> MuPdfBridge::FindAndTrim(std::wstring query, int page, int &end)
{
    //LE("FindAndTrim START");
    std::vector<SearchResult> result;
    std::vector<Hitbox> hitboxes = processTextToArray(page);

    std::vector<int> pos_arr;
    std::vector<std::string> xp_arr;
    //LE("FindAndTrim query = %s",query.c_str());

    if (hitboxes.empty())
    {
        LE("FindAndTrim hitboxes empty!");
        return result;
    }
    //LE("FindAndTrim pos arr gen start");

    int startpos = 0;
    //int tempcount = 0;
    while (startpos < hitboxes.size())
    {
        int pos = pos_f_arr(hitboxes, query, startpos);
        if (pos == -1)
        {
            break;
        }
        pos_arr.push_back(pos);
        //tempcount++;
        startpos = pos + query.length();
    }
    if (pos_arr.empty())
    {
        LE("FindAndTrim pos_arr empty!");
        return result;
    }
    //LE("FindAndTrim pos arr gen end num = %d",tempcount);
    xp_arr = GetXpathFromPageById(hitboxes, pos_arr, true, query.length());

    if (pos_arr.size() != xp_arr.size())
    {
        LD("pos arr len != xp arr len!!!!!11");
        return result;
    }


    SearchWindow window = SearchWindow(hitboxes, query, 0);
    for (int i = 0; i < xp_arr.size(); i++)
    {
        int curr_pos = pos_arr.at(i);
        std::string curr_xpointer = xp_arr.at(i);
        if (window.contains(curr_pos))
        {
            window.addpos(curr_pos, curr_xpointer);
        }
        else
        {
            std::wstring str;
            std::vector<Hitbox> subset(&hitboxes[window.start_], &hitboxes[window.end_]);
            for (int i = 0; i < subset.size(); i++)
            {
                str = str.append(subset.at(i).text_);
            }
            replaceAll(str, std::wstring(L"\n"), std::wstring(L" "));
            //LE("str in = %s",str.c_str());

            SearchResult sr = SearchResult(str, window.xp_array_);
            result.push_back(sr);

            window = SearchWindow(hitboxes, query, window.end_ + 1);
            window.addpos(curr_pos, curr_xpointer);
        }
    }
    //last iteration
    std::wstring str;
    std::vector<Hitbox> subset(&hitboxes[window.start_], &hitboxes[window.end_]);
    for (int i = 0; i < subset.size(); i++)
    {
        str = str.append(subset.at(i).text_);
    }
    replaceAll(str, std::wstring(L"\n"), std::wstring(L" "));
    //LE("str end = %s",str.c_str());
    end = window.end_;
    SearchResult sr = SearchResult(str, window.xp_array_);
    result.push_back(sr);
    //LE("FindAndTrim END");
    return result;
}

std::vector<Hitbox> MuPdfBridge::GetSearchHitboxes(std::vector<Hitbox> hitboxes, int page, std::wstring query)
{
    std::vector<Hitbox> result;

    if (hitboxes.empty())
    {
        LE("GetSearchHitboxes hitboxes empty");
        return std::vector<Hitbox>();
    }

    int qlen = query.length();

    int startpos = 0;
    int pos = 0;
    while (pos < hitboxes.size())
    {
        pos = pos_f_arr(hitboxes, query, startpos);
        if (pos == -1)
        {
            break;
        }
        std::wstring xp = stringToWstring(GetXpathFromPageById(hitboxes, pos, false, false));
        for (int i = pos; i < pos + qlen; i++)
        {
            Hitbox hb = hitboxes.at(i);
            hb.text_ = xp;
            hb.xpointer_ = wstringToString(xp);
            result.push_back(hb);
        }
        startpos = pos + qlen;
    }

    return result;
}

std::vector<SearchResult> MuPdfBridge::SearchForTextPreviews(int page, std::wstring query)
{
    std::vector<SearchResult> result;
    if (page < -1 || page > this->pageCount || query.empty())
    {
        return result;
    }

    int start_last = 0;
    std::string spagetext = getPageText(page);
    if(spagetext.length()<=0)
    {
        return result;
    }
    std::wstring pagetext = ReplaceUnusualSpaces(stringToWstring(spagetext));

    searchPackCounter = searchPackCounter + spagetext.length();
    //LD("page = %d",page);
    //LE("pagetext = %s",wstringToString(pagetext).c_str());
    //LE("pagetext len = %d",pagetext.length());

    pagetext = lowercase(pagetext);

    if (pos_f(pagetext, query) != -1)
    {
        std::vector<SearchResult> curr = FindAndTrim(query, page, start_last);
        for (int i = 0; i < curr.size(); i++)
        {
            result.push_back(curr.at(i));
        }
    }

    /*
    if (page < this->pageCount - 1 &&
    CheckNextPagePreviews(pagetext,stringToWstring(query),start_last))
    {
        std::wstring query_temp = stringToWstring(query);
        SearchResult nextpage = FindAndTrimNextPage(query_temp, page, start_last);
        if (!nextpage.xpointers_.empty())
        {
            result.push_back(nextpage);
        }
    }
     */
    return result;
}

std::vector<Hitbox> MuPdfBridge::SearchForTextHitboxes(int page, std::wstring query)
{
    std::vector<Hitbox> result;

    if (page < -1 || query.empty())
    {
        return result;
    }

    std::vector<Hitbox> base = processTextToArray(page);

    /*
    if (page != 0)
    {
        if (checkBeforePrevPage(base, query))
        {
            int last_len = GetSearchHitboxesNextPageLite(page - 1, query);
            if (last_len > 0)
            {
                std::vector<Hitbox> first = GetSearchHitboxesPrevPage(base, query, last_len);
                for (int i = 0; i < first.size(); i++)
                {
                    result.push_back(first.at(i));
                }
            }
        }
    }

    */
    std::vector<Hitbox> main = GetSearchHitboxes(base, page, query);

    for (int i = 0; i < main.size(); i++)
    {
        result.push_back(main.at(i));
    }

    /*
    if (page < pageCount-2)
    {
        std::vector<Hitbox> last = GetSearchHitboxesNextPage(base, page, query);
        for (int i = 0; i < last.size(); i++)
        {
            result.push_back(last.at(i));
        }
    }

    */
    if (result.empty())
    {
        return result;
    }
    return unionRectsTextCheck(result);
}

//searches for the specified rectangle on page (by number), returns xpath if found.
void MuPdfBridge::processXPathByRectId(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_XPATH_BY_RECT_ID;
    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        LE("processXPathByHitbox bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    const char *val = reinterpret_cast<const char *>(temp_val);

    //std::stringstream input_str("2:10:0"); //page:id:reverse

    std::stringstream input_str(val);
    std::string key;
    std::vector<std::string> keys;

    while (std::getline(input_str, key, ':'))
    {
        keys.push_back(key);
    }

    int page = atoi(keys.at(0).c_str());
    int id = atoi(keys.at(1).c_str());
    bool reverse = atoi(keys.at(2).c_str()) > 0;

    std::string xpath = GetXpathFromPageById(page, id, true, reverse);
    response.addIpcString(xpath.c_str(), true);
}

void MuPdfBridge::processTextSearchPreviews(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_SEARCH_PREVIEWS;

    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        LE("processTextSearchPreviews bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);

    std::wstring inputstr = stringToWstring(std::string(val));

    //LE("processTextSearchPreviews inputstr [%s]",inputstr.c_str());
    std::wstring query = inputstr;

    int pagestart = 0;
    int pageend = this->pageCount;
    int sep = pos_f(inputstr, std::wstring(L":"));

    if (sep != -1)
    {

        std::stringstream pages_s = std::stringstream(wstringToString(inputstr.substr(0, sep)));

        std::string key;
        std::vector<std::string> pages_c;

        while (std::getline(pages_s, key, '-'))
        {
            pages_c.push_back(key);
        }

        pagestart = std::atoi(pages_c.at(0).c_str());
        pageend = std::atoi(pages_c.at(1).c_str());

        query = inputstr.substr(sep + 1, inputstr.length() - sep + 1);
    }

    if (query.empty())
    {
        LE("processTextSearchPreviews bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    query = lowercase(query);
    for (int p = pagestart; p <= pageend; p++)
    {
        std::vector<SearchResult> pagestrings;
        pagestrings = SearchForTextPreviews(p, query);

        //LE("found %d items",pagestrings.size());
        for (int i = 0; i < pagestrings.size(); i++)
        {
            SearchResult curr = pagestrings.at(i);

            std::wstring text = curr.preview_;
            replaceAll(text, std::wstring(L"\n"), std::wstring(L" "));

            std::string xpointers_str;
            for (int i = 0; i < curr.xpointers_.size(); i++)
            {
                xpointers_str += curr.xpointers_.at(i);
                xpointers_str += ";";
            }
            xpointers_str = xpointers_str.substr(0, xpointers_str.length() - 1);


            response.addInt(p);
            response.addIpcString(xpointers_str.c_str(), true);
            response.addIpcString(wstringToString(text).c_str(), true);

            //LE("processTextSearchPreviews found [%d]^[%d] = [%s][%s]",p,curr.xpointers_.size(),wstringToString(text).c_str(),xpointers_str.c_str());
        }
    }

}

void MuPdfBridge::processTextSearchHitboxes(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_SEARCH_HITBOXES;
    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        LE("processTextSearchHitboxes bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    //std::stringstream input_str("3:the"); //page:text
    std::stringstream input_str(val);

    std::string key;
    std::vector<std::string> keys;
    //LE("processTextSearchHitbox inputstr =  %s",val);

    while (std::getline(input_str, key, ':'))
    {
        keys.push_back(key);
    }

    int page = atoi(keys.at(0).c_str());
    std::string query_s  = keys.at(1).c_str();
    std::wstring query = stringToWstring(query_s);

    if (!query.empty())
    {
        query = lowercase(query);
        std::vector<Hitbox> hitboxes = SearchForTextHitboxes(page, query);

        for (int i = 0; i < hitboxes.size(); i++)
        {
            Hitbox currHitbox = hitboxes.at(i);
            //LE("processTextSearchHitbox found %d = [%s]",i,hitboxes.at(i).xpointer_.c_str());

            response.addFloat(currHitbox.left_);
            response.addFloat(currHitbox.top_);
            response.addFloat(currHitbox.right_);
            response.addFloat(currHitbox.bottom_);
            response.addIpcString(currHitbox.xpointer_.c_str(), true);
        }
    }
}

void MuPdfBridge::processSearchCounter(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_SEARCH_COUNTER;
    uint32_t flag;

    CmdDataIterator iter(request.first);
    iter.getInt(&flag);

    if (!iter.isValid())
    {
        LE("processSearchCounter Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    if (flag == 1)     // 1 == on // 0 = off
    {
        response.addInt(searchPackCounter);
    }
    else if(flag == 0)
    {
        response.addInt(searchPackCounter);
        searchPackCounter = 0;
    }
}

/*

SearchResult StBridge::FindAndTrimNextPage(std::wstring query, int page, int start_last);
{
    SearchResult result;
    std::vector<Hitbox> base1 = processTextToArray(page);

    if (base1.empty())
    {
        return result;
    }
    std::wstring text1;
    for (int i = 0; i < base1.size(); i++)
    {
        text1+=base1.at(i).text_;
    }

    query = lowercase(query);
    text1 = lowercase(text1);

    int tlen1 = text1.length();
    int qlen = query.length();
    int start = tlen1 - qlen;

    if(pos_f_arr(base1,query,start) == start)
    {
        return result;
    }

    int qlen1 = -1;

    int pos = pos_f_arr(base1,std::wstring(query.c_str(), 1), start);
    //LE("FindAndTrimNextPage tlen1 = %d,start = %d",tlen1,start);
    //LE("FindAndTrimNextPage pos == %d",pos);

    while (pos > -1)
    {
        std::wstring qtemp = query.substr(0, tlen1 - pos);
        std::wstring ttemp = text1.substr(pos, tlen1 - pos);
        //LE("FindAndTrimNextPage  qtemp =[%ls] [0:%d]",qtemp.c_str(),tlen1-pos);
        //LE("FindAndTrimNextPage  ttemp =[%ls] [%d:%d]",ttemp.c_str(),pos, tlen1-pos);
        if (qtemp == ttemp)
        {
            qlen1 = tlen1 - pos;
            break;
        }
        pos = pos_f_arr(base1,std::wstring(query.c_str(), 1), pos + 1);
    }
    if (pos == -1)
    {
        return result;
    }

    std::vector<Hitbox> base2 = processTextToArray(page+1);

    int qlen2 = qlen - qlen1;

    std::wstring q2 = query.substr(qlen1,qlen2);

    std::vector<Hitbox> base2sub(&base2[0], &base2[qlen2]);
    std::wstring text2;
    for (int i = 0; i < base2sub.size(); i++)
    {
        text2.append(base2sub.at(i).text_);
    }
    text2 = lowercase(text2);
    if(text2 != q2)
    {
        return result;
    }

    std::string xp1 = base1.at(pos).xpointer_;
    std::string xp2 = base2.at(qlen2).xpointer_;
    if (xp1.empty() || xp2.empty())
    {
        return result;
    }

    int pos2 = pos_f_arr(base2,query,0);

    if (pos2 != -1)
    {
        base2 = std::vector<Hitbox>(&base2[0], &base2[pos2]);
    }

    std::vector<Hitbox> globaltext;
    for (int i = 0; i < base1.size(); i++)
    {
        globaltext.push_back(base1.at(i));
    }
    for (int i = 0; i < base2.size(); i++)
    {
        globaltext.push_back(base2.at(i));
    }
    std::string query_temp = wstringToString(query);
    SearchWindow window = SearchWindow(globaltext,query_temp,start_last);

    int curr_pos = pos;

    std::string xpRange = xp1 + "<=>" + xp2;
    //LE("FindAndTrim xprange = %s",xpRange.c_str());
    if (window.contains(curr_pos))
    {
        window.addpos(curr_pos, xpRange);

        std::vector<Hitbox> prev_arr(&globaltext[window.start_], &globaltext[window.end_]);
        std::wstring str;
        for (int i = 0; i < prev_arr.size(); i++)
        {
            str += prev_arr.at(i).text_;
        }
        SearchResult sr = SearchResult(str, window.xp_array_);

        return sr;
    }

    return result;
}

int GetSearchHitboxesNextPageLite(int page, std::wstring query)
{
    std::vector<Hitbox> base1 = processTextToArray(page);

    if (base1.empty())
    {
        return 0;
    }

    std::wstring text1;
    for (int i = 0; i < base1.size(); i++)
    {
        text1 += base1.at(i).text_;
    }

    text1 = lowercase(text1);
    query = lowercase(query);

    int tlen = base1.size();
    int qlen = query.length();
    int start = base1.size() - qlen;

    if(pos_f_arr(base1,query,start) == start)
    {
        return 0;
    }

    int qlen1 = -1;
    int pos = pos_f_arr(base1,std::wstring(query.c_str(), 1), start);

    while (pos > -1)
    {
        std::wstring qtemp = query.substr(0, tlen - pos);
        std::wstring ttemp = text1.substr(pos, tlen - pos);
        if (qtemp == ttemp)
        {
            qlen1 = tlen - pos;
            break;
        }
        pos = pos_f_arr(base1,std::wstring(query.c_str(), 1), pos + 1);
    }

    if(pos == -1)
    {
        return 0;
    }

    return qlen1;
}

std::vector<Hitbox> GetSearchHitboxesPrevPage(std::vector<Hitbox> base, std::wstring query, int last_len)
{
    std::vector<Hitbox> result;
    if (base.empty())
    {
        return result;
    }
    query = lowercase(query);

    int qlen = query.length();

    int qlen1 = qlen - last_len;
    std::wstring qfrag1 = query.substr(last_len,qlen1);

    int lastIndex = qlen1;
    //LE("GetSearchHitboxesPrevPage lastIndex [%d] = [%d-%d-1]",lastIndex,qlen,last_len);

    std::vector<Hitbox> text1(&base[0], &base[qlen]);

    std::wstring temp;
    for (int i = 0; i < text1.size(); i++)
    {
        temp += text1.at(i).text_;
    }

    //CRLog::error("temp = %s , qfrag = %s",LCSTR(temp),LCSTR(qfrag1));
    int pos = pos_f_arr(text1,qfrag1,0);
    if(pos==-1 || qlen1==qlen || qlen1 <=0)
    {
        //CRLog::error("GetSearchHitboxesPrevPage not found");
        return result;
    }

    std::string xp1 = base.at(lastIndex).xpointer_;
    //LE("GetSearchHitboxesPrevPage xp = %s",xp1.c_str());
    if(xp1.empty())
    {
        return result;
    }

    for (int i = 0; i < qlen1; i++)
    {
        Hitbox hb = base.at(i);
        hb.xpointer_ = xp1;
        result.push_back(hb);
    }

    return result;
}

std::vector<Hitbox> GetSearchHitboxesNextPage(std::vector<Hitbox> base1, int page, std::wstring query)
{
    std::vector<Hitbox> result;
    if (base1.empty())
    {
        return result;
    }

    std::wstring text1;
    for (int i = 0; i < base1.size(); i++)
    {
        text1 += base1.at(i).text_;
    }

    text1 = lowercase(text1);
    query = lowercase(query);

    int tlen1 = base1.size();
    int qlen = query.length();
    int start = tlen1 - qlen;

    if(pos_f_arr(base1,query,start) == start)
    {
        return result;
    }

    int pos = pos_f_arr(base1,std::wstring(query.c_str(), 1), start);
    int qlen1 = -1;
    //LE("GetSearchHitboxesNextPage page = %d",page);
    //LE("GetSearchHitboxesNextPage tlen1 = %d,start = %d",tlen1,start);
    //LE("GetSearchHitboxesNextPage pos == %d",pos);
    while (pos > -1)
    {
        qlen1 = tlen1 - pos;
        std::wstring qtemp = query.substr(0, qlen1);
        std::wstring ttemp = text1.substr(pos, qlen1);
        //LE("GetSearchHitboxesNextPage  qtemp =[%ls] [0:%d]",qtemp.c_str(),tlen1-pos);
        //LE("GetSearchHitboxesNextPage  ttemp =[%ls] [%d:%d]",ttemp.c_str(),pos, tlen1-pos);
        if (qtemp == ttemp)
        {
            break;
        }
        pos = pos_f_arr(base1,std::wstring(query.c_str(), 1), pos + 1);
    }
    if(pos == -1)
    {
        return result;
    }

    std::vector<Hitbox> base2 = processTextToArray(page + 1);

    int qlen2 = qlen - qlen1;

    std::wstring q2 = query.substr(qlen1,qlen2);

    std::wstring text2;
    for (int i = 0; i < qlen2; i++)
    {
        text2 += base2.at(i).text_;
    }
    text2 = lowercase(text2);
    //LE("GetSearchHitboxesNextPage  q2   =[%ls] [%d:%d]",q2.c_str(),qlen1, qlen2);
    //LE("GetSearchHitboxesNextPage  text2 =[%ls] [0:%d]",text2.c_str(),qlen2);
    if(text2 != q2)
    {
        return result;
    }
    std::string xp1 = base1.at(pos).xpointer_;
    //LE("GetSearchHitboxesNextPage xp == %s",xp1.c_str());

    if(xp1.empty())
    {
        return result;
    }

    for (int i = pos; i < pos+qlen1; i++)
    {
        Hitbox hb = base1.at(i);
        hb.xpointer_ = xp1;
        result.push_back(hb);
    }

    return result;
}

bool CheckNextPagePreviews(std::wstring text_in, std::wstring query_in, int end)
{
    std::wstring query = lowercase(query_in);
    std::wstring text1 = lowercase(text_in);

    int tlen1 = text1.length();
    int qlen = query.length();
    int start = tlen1 - qlen;

    if(pos_f(text1,query,start) == start)
    {
        return false;
    }

    int pos = pos_f(text1,std::wstring(query.c_str(), 1), end);
    while (pos > -1)
    {
        std::wstring qtemp = query.substr(0, tlen1 - pos);
        std::wstring ttemp = text1.substr(pos, tlen1 - pos);
        LE("CheckNextPagePreviews  pos   = [%d]",pos);
        LE("CheckNextPagePreviews  qtemp = [0:%d]  [%ls]",tlen1-pos     ,qtemp.c_str());
        LE("CheckNextPagePreviews  ttemp = [%d:%d] [%ls]",pos, tlen1-pos,ttemp.c_str());
        if (qtemp == ttemp)
        {
            break;
        }
        pos = pos_f(text1,std::wstring(query.c_str(), 1), pos + 1);
    }

    return (pos!=-1);
}
*/