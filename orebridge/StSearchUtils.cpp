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

typedef unsigned int uint;

#include "StSearchUtils.h"

std::wstring uppercase(std::wstring str)
{
    for ( int i=0; i<str.length(); i++ ) {
        wchar_t ch = str[i];
        if ( ch>='a' && ch<='z' ) {
            str[i] = ch - 0x20;
        } else if ( ch>=0xE0 && ch<=0xFF ) {
            str[i] = ch - 0x20;
        } else if ( ch>=0x430 && ch<=0x44F ) {// cyrillic
            str[i] = ch - 0x20;
        } else if( ch == 0x451) { //cyrillic "Ё"
            str[i] = 0x401;
        } else if ( ch>=0x3b0 && ch<=0x3cF ) {
            str[i] = ch - 0x20;
        } else if ( (ch >> 8)==0x1F ) { // greek
            wchar_t n = ch & 255;
            if (n<0x70) {
                str[i] = ch | 8;
            } else if (n<0x80) {

            } else if (n<0xF0) {
                str[i] = ch | 8;
            }
        } else if(ch >= 0x561 && ch <= 0x586) { // armenian
            str[i] = ch - 0x30;
        } else if ((ch >= 0x10D0 && ch <= 0x10F5)|| ch == 0x10F7 || ch == 0x10FD ) {  // georgian
            str[i] = ch - 0x30;
        }
    }
    return str;
}

std::wstring lowercase( std::wstring str)
{
    for ( int i=0; i<str.length(); i++ ) {
        wchar_t ch = str[i];
        if ( ch>='A' && ch<='Z' ) {
            str[i] = ch + 0x20;
        } else if ( ch>=0xC0 && ch<=0xDF ) {
            str[i] = ch + 0x20;
        } else if ( ch>=0x410 && ch<=0x42F ) {  //cyrillic
            str[i] = ch + 0x20;
        } else if( ch == 0x401) { //cyrillic "Ё"
            str[i] = 0x451;
        } else if ( ch>=0x390 && ch<=0x3aF ) { // Greek
            str[i] = ch + 0x20;
        } else if ( (ch >> 8)==0x1F ) { // greek
            wchar_t n = ch & 255;
            if (n<0x70) {
                str[i] = ch & (~8);
            } else if (n<0x80) {

            } else if (n<0xF0) {
                str[i] = ch & (~8);
            }
        }
        else if(ch >= 0x531 && ch <= 0x556) {  // armenian
            str[i] = ch + 0x30;
        } else if ((ch >= 0x10A0 && ch <= 0x10C5)|| ch == 0x10C7 || ch == 0x10CD ) {  // georgian
            str[i] = ch + 0x30;
        }
    }
    return str;
}

int pos_f_arr(std::vector<Hitbox> in, std::wstring subStr_in, int startPos)
{
    if (startPos > in.size()-1)
    {
        return -1;
    }
    if (subStr_in.length() > in.size() - startPos)
    {
        return -1;
    }
    int s_len = subStr_in.length();
    int diff_len = in.size() - s_len;
    for (uint i = startPos; i <= diff_len; i++)
    {
        int flg = 1;
        for (uint j = 0; j < s_len; j++)
            if (lowercase(in.at(i + j).text_).at(0) != subStr_in.at(j))
            {
                flg = 0;
                break;
            }
        if (flg)
        {
            return i;
        }
    }
    return -1;
}

int pos_f(std::wstring in, std::wstring subStr)
{
    //return in.find(subStr);

    if (subStr.length() > in.length())
    {
        return -1;
    }
    int s_len = subStr.length();
    int diff_len = in.length() - s_len;
    for (int i = 0; i <= diff_len; i++)
    {
        int flg = 1;
        for (int j = 0; j < s_len; j++)
            if (in.at(i + j) != subStr.at(j))
            {
                flg = 0;
                break;
            }
        if (flg)
        {
            return i;
        }
    }
    return -1;
}

int pos_f(std::wstring in, std::wstring subStr, int startpos)
{
    //return in.find(subStr);

    if (subStr.length() > in.length())
    {
        return -1;
    }
    int s_len = subStr.length();
    int diff_len = in.length() - s_len;
    for (int i = startpos; i <= diff_len; i++)
    {
        int flg = 1;
        for (int j = 0; j < s_len; j++)
            if (in.at(i + j) != subStr.at(j))
            {
                flg = 0;
                break;
            }
        if (flg)
        {
            return i;
        }
    }
    return -1;
}

std::wstring stringToWstring(const std::string& t_str)
{
    //setup converter
    typedef std::codecvt_utf8<wchar_t> convert_type;
    std::wstring_convert<convert_type, wchar_t> converter;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    return converter.from_bytes(t_str);
}

std::string wstringToString(const std::wstring& t_str)
{
    //setup converter
    typedef std::codecvt_utf8<wchar_t> convert_type;
    std::wstring_convert<convert_type, wchar_t> converter;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    return converter.to_bytes(t_str);
}

void replaceAll(std::wstring &source, const std::wstring &from, const std::wstring &to)
{
    std::wstring newString;
    newString.reserve(source.length());  // avoids a few memory allocations

    std::wstring::size_type lastPos = 0;
    std::wstring::size_type findPos;

    while ((findPos = pos_f(source, from, lastPos)) != -1)
    {
        newString.append(source, lastPos, findPos - lastPos);
        newString += to;
        lastPos = findPos + from.length();
    }

    // Care for the rest after last occurrence
    newString += source.substr(lastPos);

    source.swap(newString);
}

std::vector<Hitbox> unionRects(std::vector<Hitbox> rects, bool glueLast)
{
    std::vector<Hitbox> result;
    if (rects.empty())
    {
        return result;
    }

    Hitbox curr = rects.at(0);
    int max = rects.size();
    if (!glueLast)
    {
        max--;
    }
    for (int i = 0; i < max; i++)
    {
        Hitbox rect = rects.at(i);
        if (curr.right_ >= rect.left_ && curr.top_ == rect.top_ && curr.bottom_ == rect.bottom_)
        {
            curr.right_ = rect.right_;
            continue;
        }
        result.push_back(curr);
        curr = rect;
    }
    result.push_back(curr);
    if (!glueLast)
    {
        result.push_back(rects.at(rects.size()-1));
    }
    return result;
}

std::vector<Hitbox> unionRectsTextCheck(std::vector<Hitbox> rects)
{
    std::vector<Hitbox> result;
    if (rects.empty())
    {
        return result;
    }

    Hitbox curr = rects.at(0);
    for (int i = 0; i < rects.size(); i++)
    {
        Hitbox rect = rects.at(i);
        if (curr.right_ >= rect.left_ &&
            curr.top_ == rect.top_ &&
            curr.bottom_ == rect.bottom_ &&
            curr.text_ == rect.text_)
        {
            curr.right_ = rect.right_;
            continue;
        }
        result.push_back(curr);
        curr = rect;
    }
    result.push_back(curr);
    return result;
}

std::wstring ReplaceUnusualSpaces(std::wstring in)
{
    if(in.length()<=0)
    {
        return in;
    }
    std::replace( in.begin(), in.end(), 0x0009, 0x0020);
    std::replace( in.begin(), in.end(), 0x00A0, 0x0020);
    std::replace( in.begin(), in.end(), 0x180E, 0x0020);
    std::replace( in.begin(), in.end(), 0x2000, 0x0020);
    std::replace( in.begin(), in.end(), 0x2001, 0x0020);
    std::replace( in.begin(), in.end(), 0x2002, 0x0020);
    std::replace( in.begin(), in.end(), 0x2003, 0x0020);
    std::replace( in.begin(), in.end(), 0x2004, 0x0020);
    std::replace( in.begin(), in.end(), 0x2005, 0x0020);
    std::replace( in.begin(), in.end(), 0x2006, 0x0020);
    std::replace( in.begin(), in.end(), 0x2007, 0x0020);
    std::replace( in.begin(), in.end(), 0x2008, 0x0020);
    std::replace( in.begin(), in.end(), 0x2009, 0x0020);
    std::replace( in.begin(), in.end(), 0x200A, 0x0020);
    std::replace( in.begin(), in.end(), 0x200B, 0x0020);
    std::replace( in.begin(), in.end(), 0x202F, 0x0020);
    std::replace( in.begin(), in.end(), 0x205F, 0x0020);
    std::replace( in.begin(), in.end(), 0x3000, 0x0020);
    std::replace( in.begin(), in.end(), 0xFEFF, 0x0020);
    return in;
}

std::vector<Hitbox> ReplaceUnusualSpaces(std::vector<Hitbox> in)
{
    for (int i = 0; i < in.size() ; i++)
    {
        Hitbox* curr = &in[i];
        curr->text_ = ReplaceUnusualSpaces(curr->text_);
    }
    return in;
}

bool checkBeforePrevPage(std::vector<Hitbox> base, std::wstring query)
{
    int qlen = query.length();
    std::vector<Hitbox> subset(&base[0], &base[qlen]);
    std::wstring chStr = query.substr(qlen-1,1);
    return (pos_f_arr(subset, chStr, 0) != -1);
}

bool char_isPunct(int c){
    return ((c>=33) && (c<=47)) ||
           ((c>=58) && (c<=64)) ||
           ((c>=91) && (c<=96)) ||
           ((c>=123)&& (c<=126))||
           (c == 0x00A6)||
           (c == 0x060C)||
           (c == 0x060D)||
           (c == 0x060E)||
           (c == 0x060F)||
           (c == 0x061F)||
           (c == 0x066D)||
           (c == 0x06DD)||
           (c == 0x06DE)||
           (c == 0x06E9)||
           (c == 0xFD3E)||
           (c == 0x0621)|| // hamza
           (c == 0xFD3F);
}

bool char_isSpace(int ch)
{
    return( ch == 0x0009 ||
            ch == 0x0020 ||
            ch == 0x00A0 ||
            ch == 0x180E ||
            ch == 0x2000 ||
            ch == 0x2001 ||
            ch == 0x2002 ||
            ch == 0x2003 ||
            ch == 0x2004 ||
            ch == 0x2005 ||
            ch == 0x2006 ||
            ch == 0x2007 ||
            ch == 0x2008 ||
            ch == 0x2009 ||
            ch == 0x200A ||
            ch == 0x200B ||
            ch == 0x202F ||
            ch == 0x205F ||
            ch == 0x3000 ||
            ch == 0xFEFF );
}
