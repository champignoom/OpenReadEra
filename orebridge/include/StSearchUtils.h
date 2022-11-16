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

#ifndef _OPENREADERA_STSEARCHUTILS_H_
#define _OPENREADERA_STSEARCHUTILS_H_

#include <codecvt>
#include <string>
#include <vector>
#include <locale>
#include <algorithm>

class Hitbox
{
public:
    float left_;
    float right_;
    float top_;
    float bottom_;
    std::string xpointer_;

    std::wstring text_;
    Hitbox() {};
    Hitbox(float left, float right, float top, float bottom, std::wstring text, std::string xpointer)
    {
        left_ = left;
        right_ = right;
        top_ = top;
        bottom_ = bottom;
        text_ = text;
        xpointer_ = xpointer;
    };

    std::string getXPointer()
    {
        return xpointer_;
    }

    ~Hitbox(){};
};

std::wstring uppercase(std::wstring str);
std::wstring lowercase( std::wstring str);

int pos_f_arr(std::vector<Hitbox> in, std::wstring subStr_in, int startPos);
int pos_f(std::wstring in, std::wstring subStr);
int pos_f(std::wstring in, std::wstring subStr, int startpos);

std::wstring stringToWstring(const std::string& t_str);
std::string wstringToString(const std::wstring& t_str);

void replaceAll(std::wstring &source, const std::wstring &from, const std::wstring &to);

std::vector<Hitbox> unionRects(std::vector<Hitbox> rects, bool glueLast = true);
std::vector<Hitbox> unionRectsTextCheck(std::vector<Hitbox> rects);
bool checkBeforePrevPage(std::vector<Hitbox> base, std::wstring query);
std::wstring ReplaceUnusualSpaces(std::wstring in);
std::vector<Hitbox> ReplaceUnusualSpaces(std::vector<Hitbox> in);
bool char_isPunct(int c);
bool char_isSpace(int ch);

#endif //_OPENREADERA_STSEARCHUTILS_H_
