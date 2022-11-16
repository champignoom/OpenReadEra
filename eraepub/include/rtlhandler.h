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
 * Developers: ReadEra Team (2013-2020), Tarasus (2018-2020).
 */

#ifndef _OPENREADERA_RTLHANDLER_H_
#define _OPENREADERA_RTLHANDLER_H_

#include "include/lvtinydom.h"

//shared
bool char_isRTL(const lChar16 c);

bool char_isPunct(const lChar16 c);

//hitboxes side
LVArray<TextRect> RTL_mix(LVArray<TextRect> in_list, int clip_width, bool rtl_space);

//txtfmt side
class WordItem
{
public:
    int x_;
    int y_;
    const lChar16* text_=0;
    int len_;
    bool flgHyphen_;
    src_text_fragment_t * srcline_;
    bool is_rtl_ = false;
    int width_ = 0;
    WordItem(){}
    WordItem(int x, int y, const lChar16* text, int len,bool flgHyphen,src_text_fragment_t * srcline ):
            x_(x),
            y_(y),
            text_(text),
            len_(len),
            flgHyphen_(flgHyphen),
            srcline_(srcline) {}

    WordItem(int x, int y, const lChar16* text, int len,bool flgHyphen,src_text_fragment_t * srcline, bool is_rtl , int width ):
            x_(x),
            y_(y),
            text_(text),
            len_(len),
            flgHyphen_(flgHyphen),
            srcline_(srcline),
            is_rtl_ (is_rtl),
            width_(width){}

    bool hasPunct(){
        if(this->len_>1)
        {
            return false;
        }
        lChar16 ch = this->text_[0];
        return char_isPunct(ch);
    }

    lString16 getText()
    {
        return lString16(text_,len_);
    }
};

void PrintRTL(LVArray<WordItem> WordItems, LVDrawBuf * buf, LVFont* font , int spacewidth);


#endif //_OPENREADERA_RTLHANDLER_H_
