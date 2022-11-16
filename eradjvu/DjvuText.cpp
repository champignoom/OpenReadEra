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

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "ore_log.h"
#include "StProtocol.h"

#include "EraDjvuBridge.h"

#define PAGETEXT_MODE_WORD "word"
#define PAGETEXT_MODE_CHAR "char"
#define PAGETEXT_MODE_WORD_INT 0
#define PAGETEXT_MODE_CHAR_INT 1
#define PAGETEXT_HEIGHT_THRESHOLD_MULTIPLIER 4.0f

constexpr static bool LOG = false;

static int pagenum;
static int expnum;
bool djvu_get_djvu_words(miniexp_t expr, const char* pattern, ddjvu_pageinfo_t *pi, CmdResponse& response, int mode)
{
    if (!miniexp_consp(expr))
    {
        return false;
    }

    miniexp_t head = miniexp_car(expr);
    expr = miniexp_cdr(expr);
    if (!miniexp_symbolp(head))
    {
        return false;
    }

    int coords[4];
    float width = pi->width;
    float height = pi->height;

    if(width  <= 0.0f || height <= 0.0f)
    {
        LE("width  <= 0.0f || height <= 0.0f");
        return false;
    }

    int i;
    for (i = 0; i < 4 && miniexp_consp(expr); i++)
    {
        head = miniexp_car(expr);
        expr = miniexp_cdr(expr);

        if (!miniexp_numberp(head))
        {
            return false;
        }
        coords[i] = miniexp_to_int(head);
    }

    while (miniexp_consp(expr))
    {
        head = miniexp_car(expr);

        if (miniexp_stringp(head))
        {
            const char* text = miniexp_to_str(head);
            float t = 1.0f - (float)coords[1] / height;
            float b = 1.0f - (float)coords[3] / height;

            float t_temp = (t < b) ? t : b;
            float b_temp = (b > t) ? b : t;

            // bottom value must be BIGGER than top value

            t = t_temp;
            b = b_temp;

            if(t >= b)
            {
                LE("djvu_get_djvu_words error: t <= b!");
                return false;
            }

            float charheight = b - t;

            std::wstring str = djvu_stringToWstring(std::string(text,strlen(text)));
            float charnum = str.length();
            if(charnum == 0)
            {
                //LE("charnum <=0");
                expr = miniexp_cdr(expr);
                continue;
            }
            float charwidth = ((float)coords[2] - (float)coords[0])/charnum;
            float lastleft = coords[0];

            if(charwidth  <= 0.0f || lastleft <= 0.0f )
            {
                charwidth = 10;
                //break;
            }

            if(mode == PAGETEXT_MODE_WORD_INT && charheight >= (charwidth/width) * PAGETEXT_HEIGHT_THRESHOLD_MULTIPLIER)
            {
                LDD(LOG,"char is too high, retrying in \"char\" mode!");
                return false;
            }

            for (int j = 0; j < str.length() ; j++)
            {
                wchar_t ch[2] = {str.at(j),0};
                response.addFloat(lastleft / width);
                response.addFloat(t);
                response.addFloat((lastleft + charwidth) / width);
                response.addFloat(b);
                DjvuBridge::responseAddString(response,ch);

                lastleft = lastleft + charwidth;
                //LDD(LOG, "DjvuText: processText: [%s]", djvu_wstringToString(ch).c_str());
            }
            if(mode == PAGETEXT_MODE_WORD_INT)
            {
                wchar_t space[2] = {' ', 0};
                response.addFloat(lastleft / width);
                response.addFloat(t);
                response.addFloat((lastleft + (charwidth / 4)) / width);
                response.addFloat(b);
                DjvuBridge::responseAddString(response, space);
            }
            // MISLEADING LOG!
            //LDD(LOG, "DjvuText: processText: %d, %d, %d, %d: %s", coords[0], coords[1], coords[2], coords[3], text);
        }
        else if (miniexp_consp(head))
        {
            djvu_get_djvu_words(head, pattern, pi, response, mode);
        }
        expr = miniexp_cdr(expr);
    }
    return true;
}

bool DjvuBridge::retryProcessTextCharMode(int pageNo, const char* pattern, CmdResponse& response)
{
    response.reset();
    response.cmd = CMD_RES_PAGE_TEXT;

    ddjvu_pageinfo_t *pi = getPageInfo(pageNo);
    if (pi == NULL)
    {
        LDD(LOG, "DjvuText: processText: no page info %d", pageNo);
        return false;
    }
    miniexp_t r = miniexp_nil;

    while ((r = ddjvu_document_get_pagetext(doc, pageNo, PAGETEXT_MODE_CHAR )) == miniexp_dummy )
    {
        waitAndHandleMessages();
    }

    if (r == miniexp_nil || !miniexp_consp(r))
    {
        LDD(LOG, "DjvuText: processText: no text on page %d", pageNo);
        return false;
    }
    return djvu_get_djvu_words(r, pattern, pi, response, PAGETEXT_MODE_CHAR_INT);
}

void DjvuBridge::processText(int pageNo, const char* pattern, CmdResponse& response)
{
    ddjvu_pageinfo_t *pi = getPageInfo(pageNo);
    if (pi == NULL)
    {
        LDD(LOG, "DjvuText: processText: no page info %d", pageNo);
        return;
    }

    miniexp_t r = miniexp_nil;
    while ((r = ddjvu_document_get_pagetext(doc, pageNo, PAGETEXT_MODE_WORD )) == miniexp_dummy )
    {
        waitAndHandleMessages();
    }

    if (r == miniexp_nil || !miniexp_consp(r))
    {
        LDD(LOG, "DjvuText: processText: no text on page %d", pageNo);
        return;
    }

    LDD(LOG, "DjvuText: processText: text found on page %d", pageNo);

    bool res = djvu_get_djvu_words(r, pattern, pi, response, PAGETEXT_MODE_WORD_INT);
    if(!res)
    {
        res = retryProcessTextCharMode(pageNo,pattern,response);
        if(!res)
            LE("DjvuText: processText: \"char\" mode retry failed!");
    }
    ddjvu_miniexp_release(doc, r);
}

std::vector<Hitbox> djvu_get_djvu_words_to_array(miniexp_t expr, ddjvu_pageinfo_t *pi, int mode)
{
    std::vector<Hitbox> result;
    if (!miniexp_consp(expr))
    {
        return std::vector<Hitbox>();
    }

    miniexp_t head = miniexp_car(expr);
    expr = miniexp_cdr(expr);
    if (!miniexp_symbolp(head))
    {
        return std::vector<Hitbox>();
    }

    int coords[4];
    float width = pi->width;
    float height = pi->height;

    if(width  <= 0.0f || height <= 0.0f)
    {
        LE("width  <= 0.0f || height <= 0.0f");
        return std::vector<Hitbox>();
    }

    for (int i = 0; i < 4 && miniexp_consp(expr); i++)
    {
        head = miniexp_car(expr);
        expr = miniexp_cdr(expr);

        if (!miniexp_numberp(head))
        {
            return std::vector<Hitbox>();
        }
        coords[i] = miniexp_to_int(head);
    }

    while (miniexp_consp(expr))
    {
        head = miniexp_car(expr);

        if (miniexp_stringp(head))
        {
            const char* text = miniexp_to_str(head);
            float t = 1.0f - (float)coords[1] / height;
            float b = 1.0f - (float)coords[3] / height;

            float t_temp = (t < b) ? t : b;
            float b_temp = (b > t) ? b : t;

            // bottom value must be BIGGER than top value

            t = t_temp;
            b = b_temp;

            if(t >= b)
            {
                LE("djvu_get_djvu_words_to_array error: t <= b!");
                return std::vector<Hitbox>();
            }
            float charheight = b - t;

            std::wstring str =  djvu_stringToWstring(std::string(text,strlen(text)));
            int charnum = str.length();
            if(charnum  == 0)
            {
                //LE("charnum <=0");
                expr = miniexp_cdr(expr);
                continue;
            }
            float charwidth = ((float)coords[2] - (float)coords[0])/(float)charnum;
            float lastleft = coords[0];

            if(charwidth  <= 0.0f || lastleft <= 0.0f )
            {
                charwidth = 10;
                //break;
            }

            if(mode == PAGETEXT_MODE_WORD_INT && charheight >= (charwidth/width) * PAGETEXT_HEIGHT_THRESHOLD_MULTIPLIER)
            {
                LDD(LOG,"char is too high, retrying in \"char\" mode!");
                return std::vector<Hitbox>();
            }

            for (int i = 0; i < charnum ; ++i)
            {
                wchar_t ch[2] = {str.at(i),0};
                char path[100];
                sprintf(path, "/page[%d]/word[%d]/char[%d]", pagenum, expnum, i);
                //LDD(LOG, "DjvuText: djvu_get_djvu_words: char path: %s", path);

                float l_ = lastleft / width;
                float r_ = (lastleft + charwidth) / width;
                float t_ = t < b ? t : b;
                float b_ = t > b ? t : b;
                std::string xpath = std::string(path);
                Hitbox hb = Hitbox(l_,r_,t_,b_,ch,xpath);
                result.push_back(hb);

                lastleft = lastleft + charwidth;
                //LDD(LOG, "DjvuText: processText HB: %f, %f, %f, %f: [%s] : %s", l_,r_,t_,b_, djvu_wstringToString(ch).c_str(),xpath.c_str());

            }
            if (mode == PAGETEXT_MODE_WORD_INT)
            {
                wchar_t space[2] = {' ', 0};

                float l = (lastleft / width);
                float r = ((lastleft + (charwidth / 4)) / width);
                float t_ = (t < b ? t : b);
                float b_ = (t > b ? t : b);

                char xpath[100];
                sprintf(xpath, "/page[%d]/word[%d]/char[%d]", pagenum, expnum, charnum);

                Hitbox hb = Hitbox(l, r, t_, b_, space, xpath);
                result.push_back(hb);
            }
            //LDD(LOG, "DjvuText: processText: %d, %d, %d, %d: %s", coords[0], coords[1], coords[2], coords[3], text);
            expnum++;
        }
        else if (miniexp_consp(head))
        {
            std::vector<Hitbox> temp = djvu_get_djvu_words_to_array(head, pi, mode);
            result.insert(result.end(), temp.begin(), temp.end());
        }
        expr = miniexp_cdr(expr);
    }
    return result;
}

std::vector<Hitbox> DjvuBridge::retryProcessTextToArrayCharMode(int pageNo)
{
    pagenum = pageNo;
    ddjvu_pageinfo_t *pi = getPageInfo(pageNo);
    if (pi == NULL)
    {
        LDD(LOG, "DjvuText: processText: no page info %d", pageNo);
        return std::vector<Hitbox>();
    }

    miniexp_t r = miniexp_nil;

    while ((r = ddjvu_document_get_pagetext(doc, pageNo, PAGETEXT_MODE_CHAR )) == miniexp_dummy )
    {
        waitAndHandleMessages();
    }

    if (r == miniexp_nil || !miniexp_consp(r))
    {
        LDD(LOG, "DjvuText: processText: no text on page %d", pageNo);
        return std::vector<Hitbox>();
    }

    LDD(LOG, "DjvuText: processText: text found on page %d", pageNo);
    expnum = 0;

    return  djvu_get_djvu_words_to_array(r, pi, PAGETEXT_MODE_CHAR_INT);
}

std::vector<Hitbox> DjvuBridge::processTextToArray(int pageNo)
{
    pagenum = pageNo;
    ddjvu_pageinfo_t *pi = getPageInfo(pageNo);
    if (pi == NULL)
    {
        LDD(LOG, "DjvuText: processTextToArray: no page info %d", pageNo);
        return std::vector<Hitbox>();
    }

    miniexp_t r = miniexp_nil;

    while ((r = ddjvu_document_get_pagetext(doc, pageNo, PAGETEXT_MODE_WORD )) == miniexp_dummy )
    {
        waitAndHandleMessages();
    }

    if (r == miniexp_nil || !miniexp_consp(r))
    {
        LDD(LOG, "DjvuText: processTextToArray: no text on page %d", pageNo);
        return std::vector<Hitbox>();
    }

    LDD(LOG, "DjvuText: processTextToArray: text found on page %d", pageNo);
    expnum = 0;
    std::vector<Hitbox> result = djvu_get_djvu_words_to_array(r, pi, PAGETEXT_MODE_WORD_INT);

    if(result.empty())
    {
        result = retryProcessTextToArrayCharMode(pageNo);
        if(result.empty())
        {
            LE("DjvuText: processTextToArray: \"char\" mode retry failed!");
            return result;
        }
    }

    ddjvu_miniexp_release(doc, r);

    return ReplaceUnusualSpaces(result);
}
std::string DjvuBridge::GetXpathFromPageById(int page, int id, bool addcoords)
{
    std::vector<Hitbox> hitboxes = processTextToArray(page);
    return GetXpathFromPageById(hitboxes,id,addcoords);
}

std::string DjvuBridge::GetXpathFromPageById(std::vector<Hitbox> hitboxes, int id, bool addcoords)
{
    std::wstring slashn;
    slashn += '\n';

    if (id >= 0 && id <= hitboxes.size())
    {
        if(hitboxes.at(id).text_ == slashn && id > 0)
        {
            id--;
        }
        //LDD(LOG, "DjvuText: hitbox [%d:%d] = [%s] = {%s}",page,id,hitboxes.at(id).text_.c_str(),hitboxes.at(id).getXPointer().c_str());
        std::string xpath = hitboxes.at(id).getXPointer();
        if(xpath.empty())
        {
            //DEBUG_L(true, "Djvu", "hitbox 1 [-]" );
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

std::vector<std::string> DjvuBridge::GetXpathFromPageById(std::vector<Hitbox> hitboxes, std::vector<int> pos_arr, bool addcoords, int qlen)
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

std::wstring djvu_stringToWstring(const std::string& input)
{
    try
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(input);
    }
    catch(std::range_error& e)
    {
        std::size_t length = input.length();
        std::wstring result;
        result.reserve(length);
        for(std::size_t i = 0; i < length; i++)
        {
            result.push_back(input[i] & 0xFF);
        }
        return result;
    }
}

std::string djvu_wstringToString(const std::wstring& t_str)
{
    typedef std::codecvt_utf8<wchar_t> convert_type;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    std::wstring_convert<convert_type, wchar_t> converter;

    std::string result;
    try
    {
        return converter.to_bytes(t_str);
    }
    catch (std::range_error rangeError)
    {
        return std::string();
    }
}

void SwitchDvngI_reverse(std::wstring* str)
{
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x093F)
        {
            str->at(i) = str->at(i+1);
            str->at(i+1) = 0x093f;
        }
    }
}

void SwitchBanglaLetters_reverse(std::wstring* str)
{
    // reverse
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x09BF || str->at(i) == 0x09C7 || str->at(i) == 0x09C8 )
        {
            str->at(i) = str->at(i+1);
            str->at(i+1) = 0x09BF;
        }
    }
}

void SwitchIndicChars(std::wstring* str)
{
    std::wistringstream in_stream = std::wistringstream(*str);

    std::wstring key;
    std::wstring res;
    while (std::getline(in_stream, key, L' '))
    {
        std::wstring *curr = &key;
        SwitchDvngI_reverse(curr);
        SwitchBanglaLetters_reverse(curr);
        res.append(*curr);
        res.append(L" ");
    }
    res.substr(0,res.length()-1);
    *str = res;
}

void DjvuBridge::processRtlText(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_RTL_TEXT;

    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        LE("processRtlText bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    std::string inputstr = std::string(val);

    //page:startindex:endindex
    //"1:0:40"

    std::stringstream in_stream = std::stringstream(inputstr);

    std::string key;
    std::vector<std::string> in_arr;

    while (std::getline(in_stream, key, ':'))
    {
        in_arr.push_back(key);
    }

    if (in_arr.at(0).empty() || in_arr.at(1).empty() || in_arr.at(2).empty() )
    {
        LE("processRtlText bad request data : invalid key string [%s]",inputstr.c_str());
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    uint32_t page = std::atoi(in_arr.at(0).c_str());
    uint32_t startindex = std::atoi(in_arr.at(1).c_str());
    uint32_t endindex = std::atoi(in_arr.at(2).c_str());

    std::vector<Hitbox> selection = processTextToArray(page);

    std::wstring respstr;
    for (int i = startindex; i <= endindex && i < selection.size(); i++)
    {
        Hitbox currHitbox = selection.at(i);
        respstr += currHitbox.text_;
    }

    SwitchIndicChars(&respstr);

    response.addIpcString(wstringToString(respstr).c_str(), true);
}