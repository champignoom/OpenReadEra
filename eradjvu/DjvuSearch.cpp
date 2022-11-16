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

#include "EraDjvuBridge.h"
#include "StProtocol.h"
#include <string>
#include <codecvt>

std::wstring djvu_get_djvu_page_text(miniexp_t expr, ddjvu_pageinfo_t *pi)
{
    std::wstring result;

    if (!miniexp_consp(expr))
    {
        return std::wstring();
    }

    miniexp_t head = miniexp_car(expr);
    expr = miniexp_cdr(expr);
    if (!miniexp_symbolp(head))
    {
        return std::wstring();
    }

    int coords[4];

    int i;
    for (i = 0; i < 4 && miniexp_consp(expr); i++)
    {
        head = miniexp_car(expr);
        expr = miniexp_cdr(expr);

        if (!miniexp_numberp(head))
        {
            return std::wstring();
        }
        coords[i] = miniexp_to_int(head);
    }

    while (miniexp_consp(expr))
    {
        head = miniexp_car(expr);

        if (miniexp_stringp(head))
        {
            const char *text = miniexp_to_str(head);

            std::wstring str = djvu_stringToWstring(std::string(text, strlen(text)));
            uint charnum = str.length();
            if (charnum == 0)
            {
                //LE("charnum <=0");
                expr = miniexp_cdr(expr);
                continue;
            }
            result+=str;

        }
        else if (miniexp_consp(head))
        {
            result += djvu_get_djvu_page_text(head, pi);
        }
        expr = miniexp_cdr(expr);
    }
    return result;
}


std::wstring DjvuBridge::getPageText(int pageNo)
{
    std::wstring result;

    ddjvu_pageinfo_t *pi = getPageInfo(pageNo);
    if (pi == NULL)
    {
        LE("processText: no page info %d", pageNo);
        return std::wstring();
    }

    miniexp_t r = miniexp_nil;

    //LEAVE "page" MODE AS IS: NO NEED TO HAVE COORDINATES IN TEXT ONLY MODE
    while ((r = ddjvu_document_get_pagetext(doc, pageNo, "page")) == miniexp_dummy )
    {
        waitAndHandleMessages();
    }

    if (r == miniexp_nil || !miniexp_consp(r))
    {
        LE("processText: no text on page %d", pageNo);
        return std::wstring();
    }

    //LE("processText: text found on page %d", pageNo);

    result = djvu_get_djvu_page_text(r, pi);

    //LE("page %d text len = %d",pageNo,text.size());
    //LE("page %d text = %s",pageNo,wstringToString(text).c_str());

    ddjvu_miniexp_release(doc, r);

    return result;
}

std::vector<SearchResult> DjvuBridge::FindAndTrim(std::wstring query, int page, int end)
{
    std::vector<SearchResult> result;
    std::vector<Hitbox> hitboxes = processTextToArray(page);

    std::vector<int> pos_arr;
    std::vector<std::string> xp_arr;

    if (hitboxes.empty())
    {
        LE("FindAndTrim hitboxes empty!");
        return result;
    }

    int startpos = 0;
    while (startpos < hitboxes.size())
    {
        int pos = pos_f_arr(hitboxes, query, startpos);
        if (pos == -1)
        {
            break;
        }
        pos_arr.push_back(pos);
        startpos = pos + query.length();

    }
    if (pos_arr.empty())
    {
        LE("FindAndTrim pos_arr empty!");
        return result;
    }
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
    return result;
}

std::vector<SearchResult> DjvuBridge::SearchForTextPreviews(int page, std::wstring query)
{
    std::vector<SearchResult> result;
    if (page < -1 || page > this->pageCount || query.empty())
    {
        return result;
    }

    int start_last = 0;
    std::wstring pagetext = ReplaceUnusualSpaces(getPageText(page));
    if(pagetext.length()<=0)
    {
        return result;
    }
    searchPackCounter = searchPackCounter + pagetext.length();

    pagetext = lowercase(pagetext);
    if (pos_f(pagetext, query) != -1)
    {
        std::vector<SearchResult> curr = FindAndTrim(query, page, start_last);
        for (int i = 0; i < curr.size(); i++)
        {
            result.push_back(curr.at(i));
        }
    }

    //if (page < this->pageCount - 1)
    //{
    //    if (CheckNextPagePreviews(pagetext, djvu_stringToWstring(query), start_last))
    //    {
    //        LE("LOOKING AT NEXT PAGE");
    //        std::wstring query_temp =  djvu_stringToWstring(query);
    //        SearchResult nextpage = FindAndTrimNextPage(query_temp, page, start_last);
    //        if (!nextpage.xpointers_.empty())
    //        {
    //            result.push_back(nextpage);
    //        }
    //    }
    //}

    return result;
}

void DjvuBridge::processTextSearchPreviews(CmdRequest &request, CmdResponse &response)
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

    std::string inputstr = std::string(val);

    //LE("processTextSearchPreviews inputstr [%s]",inputstr.c_str());
    std::wstring query = djvu_stringToWstring(inputstr);

    int pagestart = 0;
    int pageend = this->pageCount;
    int sep = pos_f( djvu_stringToWstring(inputstr), std::wstring(L":"));

    if (sep != -1)
    {

        std::stringstream pages_s = std::stringstream(inputstr.substr(0, sep));

        std::string key;
        std::vector<std::string> pages_c;

        while (std::getline(pages_s, key, '-'))
        {
            pages_c.push_back(key);
        }

        pagestart = std::atoi(pages_c.at(0).c_str());
        pageend = std::atoi(pages_c.at(1).c_str());

        query =  djvu_stringToWstring(inputstr.substr(sep + 1, inputstr.length() - sep + 1));
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
            response.addIpcString(djvu_wstringToString(text).c_str(), true);

            //LE("processTextSearchPreviews found [%d]^[%d] = [%s][%s]",p,curr.xpointers_.size(),wstringToString(text).c_str(),xpointers_str.c_str());
        }
    }

}

void DjvuBridge::processTextSearchHitboxes(CmdRequest &request, CmdResponse &response)
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
    std::string query_s = keys.at(1).c_str();
    std::wstring query =  djvu_stringToWstring(query_s);

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

std::vector<Hitbox> DjvuBridge::SearchForTextHitboxes(int page, std::wstring query)
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

std::vector<Hitbox> DjvuBridge::GetSearchHitboxes(std::vector<Hitbox> hitboxes, int page, std::wstring query)
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
        std::wstring xp =  djvu_stringToWstring(GetXpathFromPageById(hitboxes, pos, false));
        for (int i = pos; i < pos + qlen; i++)
        {
            Hitbox hb = hitboxes.at(i);
            hb.text_ = xp;
            hb.xpointer_ = djvu_wstringToString(xp);
            result.push_back(hb);
        }
        startpos = pos + qlen;
    }

    return result;
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
    std::string query_temp = djvu_wstringToString(query);
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