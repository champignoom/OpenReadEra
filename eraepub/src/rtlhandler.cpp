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

// RTL arabic like texts related functions

#include "include/lvtextfm.h"
#include "include/rtlhandler.h"
#include "include/arabic_tables.h"
#include "include/lvstring.h"

//shared

int TextRect::getWidthRTL(LVFont * font)
{
    lvRect this_rect = this->getRect();
    lString16 this_text = this->getText();
    int rect_width = this_rect.width();
    int rect_height = this_rect.height();
    int font_width = font->getCharWidth(this_text.firstChar());
    int font_height = font->getHeight();
    int width = (rect_width > 0 && rect_height <= font_height) ? rect_width : font_width;
    return width;
}

//hitboxes side
class TextRectGroup{
public:
    TextRectGroup(){};
    LVArray<TextRect> list_;
    bool is_rtl_ = false;
    int getRectWidth()
    {
        int result = 0;
        if(this->list_.empty())
        {
            return result;
        }
        for (int i = 0; i < list_.length(); i++)
        {
            TextRect curr = list_.get(i);
            int rect_width = curr.getRect().width();
            result += rect_width;
        }
        return result;
    }

    int getFontWidth(LVFont *font)
    {
        int result = 0;
        if(this->list_.empty())
        {
            return result;
        }
        for (int i = 0; i < list_.length(); i++)
        {
            TextRect curr = list_.get(i);
            int font_width = font->getCharWidth(curr.getText().firstChar());
            result += font_width;
        }
        return result;
    }

    int getWidth(LVFont *font)
    {
        int result = 0;
        if(this->list_.empty())
        {
            return result;
        }
        for (int i = 0; i < list_.length(); i++)
        {
            TextRect curr = list_.get(i);
            int rect_width = curr.getRect().width();
            int font_width = font->getCharWidth(curr.getText().firstChar());
            int width = (rect_width > 0 && rect_width < 100 ) ? rect_width : font_width;
            result += width;
        }
        return result;
    }

    lString16 getText()
    {
        lString16 text;
        for (int i = 0; i < this->list_.length(); i++)
        {
            text += this->list_.get(i).getText();
        }
        return text;
    }

    bool hasPunct(){
        if(list_.empty())
        {
            return false;
        }
        if(list_.length()>1)
        {
            return false;
        }
        lChar16 ch = list_[0].getText().firstChar();
        return char_isPunct(ch);
    }

    void addTextRect(TextRect textRect)
    {
        list_.add(textRect);
    }

    bool checkLineRTL()
    {
        if (list_.empty())
        {
            return false;
        }
        //CRLog::error("line length = %d",line.length());

        for (int i = 0; i < list_.length(); i++)
        {
            lChar16 ch = list_.get(i).getText().firstChar();
            if(char_isRTL(ch))
            {
                return true;
            }
        }
        return false;
    }
};

int getSpaceWidth(LVArray<TextRectGroup> words)
{
    if(words.empty())
    {
        return -1;
    }
    int words_len = words.length();
    if(words.get(words_len-1).getText() == " ")
    {
        words_len --;
    }
    LVFont * font = words.get(0).list_.get(0).getNode()->getParentNode()->getFont().get();

    int gaps = 0;
    int space_counter = 0;
    int offset = words.get(0).list_.get(0).getRect().left;
    int start = 0;
    if(words.get(0).getText() == " ")
    {
        start ++;
    }
    int gapfix = 0;
    int orig_line_left = words.get(0).list_.get(0).getRect().left;
    int orig_line_right = words.get(words_len-1).list_.get(0).getRect().left + words.get(words_len-1).getRectWidth();
    int orig_line_width = orig_line_right-orig_line_left;
    //CRLog::error("orig width = %d",orig_line_width);
    for (int i = start; i < words_len; i++)
    {
        TextRectGroup curr = words.get(i);
        if(curr.getText().firstChar() == ' ' )
        {
            //CRLog::trace("SPACE COUNTER ++");
            space_counter ++;
            int rect_width = curr.getRectWidth();
            int font_width = curr.getFontWidth(font);
            //CRLog::error("space counter = %d",space_counter);
            //CRLog::error("space rect width = %d",curr.getRectWidth());
            //CRLog::error("space font width = %d",curr.getFontWidth(font));
            if(rect_width < font_width)
            {
                gapfix += font_width;
            }
            continue;
        }
        int width = curr.getFontWidth(font);
        int fontheight = font->getHeight();
        int rectheight = curr.list_.get(0).getRect().height();
        int len =   curr.list_.length();
        int left =  curr.list_.get(0).getRect().left;
        int right = curr.list_.get(len-1).getRect().right;
        //CRLog::error("curr = [%s]",LCSTR(curr.getText()));
        //CRLog::error("real left = %d , real right = %d",left,right);
        //CRLog::error("width = %d, rectwidth %d",width,curr.list_.get(0).getRect().width());
        //CRLog::error("l/r1 = [%d:%d], width = %d, offset = %d gaps = %d",left,right,width, offset, gaps);

        left = (rectheight > fontheight)? right : left;
        int gap = left - offset;
        //CRLog::error("hitbox word left = %d, text = [%s]",left+100,LCSTR(curr.getText()));
        //CRLog::error("gap = left - offset = %d = %d - %d",left-offset,left,offset);
        offset += width + gap;
        gaps += gap;
        //CRLog::error("l/r2 = [%d:%d], width = %d, gap = %d , offset = %d gaps = %d",left,right,width,gap, offset, gaps);
    }

    if (space_counter > 0)
    {
        int spacewidth = (gaps + gapfix) / space_counter;
        //CRLog::info("gaps = %d,gapfix = %d, space_counter = %d , spacewidth = %d",gaps, gapfix,space_counter, spacewidth);
        return spacewidth;
    }
    //CRLog::error("gaps = %d, space_counter = %d , spacewidth = -1",gaps,space_counter);
    //CRLog::error("space_counter = %d , spacewidth = -1",space_counter);
    return -1;
}

LVArray<TextRect> reverseWord(LVArray<TextRect> in_word)
{
    LVArray<TextRect> result;
    if(in_word.empty())
    {
        return result;
    }
    if(in_word.length()==1)
    {
        return in_word;
    }
    int first_left = in_word.get(0).getRect().left;
    for (int i = in_word.length()-1; i >=0 ; i--)
    {
        TextRect curr = in_word.get(i);
        lvRect curr_rect = curr.getRect();
        int width = curr_rect.width();
        lvRect new_rect(first_left,curr_rect.top,first_left+width,curr_rect.bottom);
        curr.setRect(new_rect);
        result.add(curr);
        first_left += width;
    }
    return result;
}

LVArray<TextRectGroup> reverseWordsOrder(LVArray<TextRectGroup> words, int spacewidth, int clip_width)
{
    LVArray<TextRectGroup> result;
    if(words.empty())
    {
        CRLog::error("words empty");
        return result;
    }
    result.reserve(words.length()+1);

    //use first word in line that contains symbols
    int count = 0;
    for (int i = 0; i < words.length() ; i++)
    {
        if (!words.get(i).list_.empty())
        {
            count=i;
            break;
        }
    }

    TextRect  firstword_txrect = words.get(count).list_.get(0);
    lString16 first_text = firstword_txrect.getText();
    lvRect    first_rect = firstword_txrect.getRect();
    //CRLog::error("first text = [%s], left = %d",LCSTR(first_text),first_rect.left);

    LVFont * font = firstword_txrect.getNode()->getParentNode()->getFont().get();

    int linewidth = 0;
    for (int i = 0; i < words.length(); i++)
    {
        TextRectGroup curr = words.get(i);
        if(curr.getText().firstChar() == ' ')
        {
            linewidth += spacewidth;
        }
        else
        {
            linewidth += curr.getFontWidth(font);
        }
    }
    int startx = first_rect.left;
    if(gFlgFloatingPunctuationEnabled)
    {
        startx = startx-(font->getVisualAligmentWidth()/2);
    }

    int leftspace = startx ; // startx - margin.left = startx - 0 = startx
    int rightspace = clip_width - (startx + linewidth);
    startx = startx - leftspace + rightspace;
    TextRectGroup last_word = words.get(words.length()-1);
    TextRect last = last_word.list_.get(last_word.list_.length()-1);

    bool line_isRTL = false;
    for (int i = 0; i < words.length(); i++)
    {
        if(words.get(i).is_rtl_)
        {
            line_isRTL = true;
            break;
        }
    }

    if(line_isRTL && last.getText().lastChar() == ' ')
    {
        startx -= font->getCharWidth(' ');
    }
    //CRLog::error("first left = %d",startx);
    //CRLog::error("rfirst left = %d",first_rect.left);
    //CRLog::error("rfirst right = %d",first_rect.right);
    LVArray<TextRectGroup> nonRTLBuffer;

    int buffwidth = 0;
    bool prev_state = words.get(words.length() - 1).is_rtl_;

    for (int w = words.length()-1; w >= 0; w--)
    {
        TextRectGroup currword = words.get(w);

        if(currword.is_rtl_ || (currword.hasPunct() && prev_state ))
        {
            int startx_nonrtlbuff=0;
            if(nonRTLBuffer.length()>0)
            {
                /*
                CRLog::trace("nonrtlbuff start 1");
                for (int i = 0; i < nonRTLBuffer.length(); i++)
                {
                    CRLog::error("nonrtlbuffer %d , [%s]",i, LCSTR(nonRTLBuffer.get(i).getText()));
                }
                 */
                TextRectGroup firstitem = nonRTLBuffer.get(0);
                TextRectGroup lastitem = nonRTLBuffer.get(nonRTLBuffer.length()-1);

                if(firstitem.getText() == " ")
                {
                    nonRTLBuffer.remove(0);
                    nonRTLBuffer.add(firstitem);
                }
                if(lastitem.getText() == " ")
                {
                    nonRTLBuffer.remove(nonRTLBuffer.length()-1);
                    nonRTLBuffer.insert(0,lastitem);
                }
                startx_nonrtlbuff = startx - buffwidth;
                for (int b = nonRTLBuffer.length()-1; b >=0  ; b--)
                {
                    TextRectGroup buff_word = nonRTLBuffer.get(b);
                    //CRLog::error("word from buff = [%s], is rtl = %d, startx_nonrtlbuff = %d", LCSTR(buff_word.getText()), (int) buff_word.is_rtl_, startx_nonrtlbuff);
                    LVFont * font = buff_word.list_[0].getNode()->getParentNode()->getFont().get();
                    for (int c = 0; c < buff_word.list_.length(); c++)
                    {
                        TextRect  curr = buff_word.list_.get(c);
                        lvRect    curr_rect = curr.getRect();
                        lString16 curr_text = curr.getText();

                        int height = font->getHeight();
                        int width = curr_rect.width();
                        if(height<=font->getHeight())
                        {
                            width =  font->getCharWidth(curr.getText().firstChar());
                        }
                        if(curr_text == " ")
                        {
                            width = spacewidth;
                        }
                        lvRect new_rect(startx_nonrtlbuff,curr_rect.top,startx_nonrtlbuff + width,curr_rect.top+height);
                        curr.setRect(new_rect);
                        buff_word.list_.set(c, curr);
                        startx_nonrtlbuff += width;
                    }
                    result.add(buff_word);
                }
                buffwidth = 0;
                nonRTLBuffer.clear();
            }
            //CRLog::error("rtl word after buff = [%s], is rtl = %d, startx = %d, startx nonrtlbuff = %d", LCSTR(currword.getText()), (int) currword.is_rtl_, startx, startx_nonrtlbuff);
            if(startx<startx_nonrtlbuff)
            {
                //CRLog::trace("moving startx right by %d pixels",startx_nonrtlbuff-startx);
                startx = startx_nonrtlbuff;
            }

            for (int c = 0; c < currword.list_.length(); c++)
            {
                TextRect curr = currword.list_.get(c);
                lvRect curr_rect = curr.getRect();
                lString16 curr_text = curr.getText();
                int height = font->getHeight();
                int width = curr_rect.width();
                //int width = 0;
                if(height<=font->getHeight())
                {
                    width =  font->getCharWidth(curr.getText().firstChar());
                }
                if(curr_text == " ")
                {
                    width = spacewidth;
                }
                lvRect new_rect(startx, curr_rect.top, startx + width, curr_rect.top + height);
                curr.setRect(new_rect);
                currword.list_.set(c, curr);
                startx += width;
            }
            result.add(currword);
        }
        else //!currword.is_rtl_
        {
            // CRLog::error("word to buff = [%s], is rtl = %d, startx = %d", LCSTR(currword.getText()), (int) currword.is_rtl_, startx);
            nonRTLBuffer.add(currword);
            buffwidth +=currword.getWidth(font);
            startx+=currword.getWidth(font);
        }
        prev_state = currword.is_rtl_;
    }
    if(nonRTLBuffer.length()>0)
    {
        /*
        CRLog::trace("nonrtlbuff start 2");
        for (int i = 0; i < nonRTLBuffer.length(); i++)
        {
            CRLog::error("nonrtlbuffer %d , [%s]",i, LCSTR(nonRTLBuffer.get(i).getText()));
        }
         */
        TextRectGroup firstitem = nonRTLBuffer.get(0);
        TextRectGroup lastitem = nonRTLBuffer.get(nonRTLBuffer.length()-1);

        if(line_isRTL && firstitem.getText() == " ")
        {
            nonRTLBuffer.remove(0);
            nonRTLBuffer.add(firstitem);
        }
        if(line_isRTL && lastitem.getText() == " ")
        {
            nonRTLBuffer.remove(nonRTLBuffer.length()-1);
            nonRTLBuffer.insert(0,lastitem);
        }
        int startx_nonrtlbuff = startx - buffwidth;
        for (int b = nonRTLBuffer.length()-1; b >=0  ; b--)
        {
            TextRectGroup buff_word = nonRTLBuffer.get(b);
            //CRLog::error("nonrtl word in end from buff = [%s], is rtl = %d, startx = %d", LCSTR(buff_word.getText()), (int) buff_word.is_rtl_, startx_nonrtlbuff);

            for (int c = 0; c < buff_word.list_.length(); c++)
            {
                TextRect  curr = buff_word.list_.get(c);
                lvRect    curr_rect = curr.getRect();
                lString16 curr_text = curr.getText();
                int height = font->getHeight();
                int width = curr_rect.width();
                if(height<=font->getHeight())
                {
                    width =  font->getCharWidth(curr.getText().firstChar());
                }

                if(curr_text == " " )
                {
                    width = spacewidth;
                }
                lvRect new_rect(startx_nonrtlbuff,curr_rect.top,startx_nonrtlbuff + width,curr_rect.top+height);
                curr.setRect(new_rect);
                buff_word.list_.set(c, curr);
                startx_nonrtlbuff += width;
            }
            result.add(buff_word);
        }
        buffwidth = 0;
        nonRTLBuffer.clear();
    }
    //CRLog::error("revvwordsorder END");

    return result;
}

/*TextRectGroup LigatureCheck(TextRectGroup word, LVFont * font)
{
    if (!word.is_rtl_)
    {
        return word;
    }

    if(!char_isRTL(word.getText().firstChar()))
    {
        return word;
    }


    if(word.list_.length() == 2)
    {
        if(word.list_.get(0).getText().firstChar() == L'\u0644')
        {
            if(word.list_.get(1).getText().firstChar() == L'\u0627')
            {
                TextRect lam_alef = word.list_.get(0);
                lString16 lig;
                lig.append(1,L'\uFEFB');
                lam_alef.setText(lig);
                int lig_width = font->getCharWidth(L'\uFEFB');
                lvRect orig_rect = word.list_.get(0).getRect();
                lvRect lig_rect = lvRect(orig_rect.left,orig_rect.top,orig_rect.left+lig_width,orig_rect.bottom);
                lam_alef.setRect(lig_rect);
                word.list_.clear();
                word.list_.add(lam_alef);
            }
        }
        return word;
    }
    bool lam_flag = false;
    int lam_index = -1;
    for (int i = 0; i < word.list_.length(); i++)
    {
        TextRect curr = word.list_.get(i);
        if(curr.getText().firstChar() == L'\u0644')
        {
            lam_flag = true;
            lam_index = i;
            continue;
        }
        if(lam_flag && curr.getText().firstChar() == L'\u0627' && lam_index>0)
        {
            TextRect lam_alef = word.list_.get(lam_index);
            lString16 lig;
            lig.append(1,L'\uFEFB');
            lam_alef.setText(lig);
            int lig_width = font->getCharWidth(L'\uFEFB');
            lvRect orig_rect = word.list_.get(lam_index).getRect();
            lvRect lig_rect = lvRect(orig_rect.left,orig_rect.top,orig_rect.left+lig_width,orig_rect.bottom);
            lam_alef.setRect(lig_rect);
            word.list_.set(lam_index,lam_alef);
            word.list_.remove(i);
        }
        else
        {
            lam_flag = false;
        }
    }

    return word;
}
*/

LVArray<TextRect> reverseLine(TextRectGroup group, int clip_width)
{
    LVArray<TextRect> result;
    LVArray<TextRect> line = group.list_;
    LVArray<TextRectGroup> words;
    words.reserve(line.length());

    if (line.empty())
    {
        return result;
    }
    //CRLog::error("line length = %d",line.length());


    //CRLog::error("WORDS BREAKUP START");
    int start = 0;
    bool last_space = false;
    bool last_punct = false;

    lString16 last_text = line.get(0).getText();
    bool last_state = char_isRTL(last_text.firstChar());

    for (int c = 0; c < line.length(); c++)
    {
        TextRect curr = line.get(c);
        lString16 curr_text = curr.getText();

        lChar16 ch = curr_text.firstChar();
        bool is_space = ch == ' ';
        bool is_punct = char_isPunct(ch);

        //bool curr_state = (is_space)? last_state : char_isRTL(ch);
        bool curr_state;
        if(is_space)
        {
            curr_state = last_state;
        }
        else if(is_punct)
        {
            curr_state = (last_space)? false : last_state ;
        }
        else
        {
            curr_state = char_isRTL(ch);
        }
        //curr_state = (is_punct && last_space)? : curr_state
        bool break_char = (is_space || last_space || is_punct || last_punct);

        //CRLog::error("letter = [%s]",LCSTR(curr_text));
        if (curr_state != last_state || break_char )
        {
            int len = c-start;
            if(len>0)
            {
                TextRectGroup word;
                word.list_.reserve(c - start + 1);
                for (int i = start; i < c; i++)
                {
                    word.list_.add(line.get(i));
                }

                //word.is_rtl_ = (is_punct)? ( (last_space)? true : curr_state) : last_state;
                word.is_rtl_ = last_state;
                words.add(word);
                start = c;
            }
        }
        last_state = curr_state;
        last_space = is_space;
        last_punct = is_punct;
    }
    TextRectGroup word;
    word.list_.reserve(line.length() - start + 1);
    for (int i = start; i < line.length(); i++)
    {
        word.list_.add(line.get(i));
    }
    word.is_rtl_ = last_state;
    words.add(word);
    //CRLog::error("added word = [%s]  (%s)",LCSTR(word.getText()),(word.is_rtl_)?"RTL":"NOT RTL");
    //CRLog::error("WORDS BREAKUP END");

/*    for (int i = 0; i < words.length(); i++)
    {
        TextRectGroup curr = words.get(i);
        LVFont * font = curr.list_.get(0).getNode()->getParentNode()->getFont().get();
        words[i] = LigatureCheck(curr,font);
    }
    */
    //for (int i = 0; i < words.length(); i++)
    //{
    //    TextRectGroup curr = words.get(i);
    //    int left =  curr.list_.get(0).getRect().left;
    //    CRLog::error("IN hitbox word left = %d, text = [%s]",left,LCSTR(curr.getText()));
    //}
    int space_width = getSpaceWidth(words);
    //to avoid first space problems with text formatter
    if(words.get(0).getText() == " ")
    {
        words.add(words.get(0));
        words.remove(0);
    }
    words = reverseWordsOrder(words, space_width, clip_width);

    for (int w = 0; w < words.length(); w++)
    {
        if(words[w].is_rtl_)// rtl word
        {
            words[w].list_ = reverseWord(words[w].list_);
        }
    }

    for (int i = 0; i < words.length(); i++)
    {
        for (int j = 0; j < words.get(i).list_.length(); j++)
        {
            TextRect curr = words.get(i).list_.get(j);
            //if(char_isRTL(curr.getText().firstChar()))
            //{
            //    curr.setText(lString16("\a"));
            //}
            result.add(curr);
        }
    }

    return result;
}

void trimFirstSpace(TextRectGroup *line)
{
    for (int i = 0; i < line->list_.length(); i++)
    {
        TextRect curr = line->list_.get(i);
        if (curr.getText().firstChar() != ' ')
        {
            return;
        }
        int spacewidth = curr.getRect().width();
        line->list_.remove(i);
        for (int c = 0; c < line->list_.length(); c++)
        {
            lvRect newrect = line->list_.get(c).getRect();
            newrect.left -= spacewidth;
            newrect.right -= spacewidth;
            line->list_[c].setRect(newrect);
        }
    }
}

void trimLastSpace(TextRectGroup *line)
{
    for (int i = line->list_.length()-1; i >= 0 ; i--)
    {
        TextRect curr = line->list_.get(i);
        if (curr.getText().firstChar() != ' ')
        {
            return;
        }
        line->list_.remove(i);
    }
}

TextRect getZeroTxRect(TextRectGroup *line)
{
    TextRect result;
    if(line->list_.empty())
    {
        return result;
    }
    TextRect first = line->list_.get(0);
    int firstleft = first.getRect().left;
    LVFont * font = first.getNode()->getParentNode()->getFont().get();
    int spacewidth = font->getCharWidth(' ');
    lvRect firstrect = first.getRect();
    firstrect.right = firstleft;
    firstrect.left = firstleft - spacewidth;
    firstrect.top = firstrect.top;
    firstrect.bottom = firstrect.top + font->getHeight();
    result=first;
    result.setRect(firstrect);
    result.setText(lString16(" "));
    //CRLog::error("zero l/r = %d / %d, height = %d fontheight = %d",zerorect.left,zerorect.right,zerorect.height(),font->getHeight());
    return result;
}

LVArray<TextRect> RTL_mix(LVArray<TextRect> in_list, int clip_width, bool rtl_space)
{
    LVArray<TextRect> result_list;
    if(in_list.empty())
    {
        return result_list;
    }
    LVArray<TextRectGroup> lines;
    TextRect first = in_list.get(0);
    lvRect first_rect = first.getRect();

    TextRectGroup line;
    ldomNode * last_node = first.getNode();
    bool last_state = last_node->isRTL();
    bool curr_state;

    for (int i = 0; i < in_list.length(); i++)
    {
        TextRect curr = in_list[i];
        curr.setIndex(i);
        lvRect curr_rect = curr.getRect();
        ldomNode * curr_node = curr.getNode();

        curr_state = ( curr_node != last_node)? curr_node->isRTL() : last_state;

        if(curr_rect.top > first_rect.top && curr_rect.bottom > first_rect.bottom)
        {
            lines.add(line);
            line = TextRectGroup();
            first_rect = curr_rect;
        }
        line.addTextRect(curr);
        last_node = curr_node;
        last_state = curr_state;
        if(curr_state)
        {
            line.is_rtl_ = true;
        }
    }
    lines.add(line);

    for (int l = 0; l < lines.length(); l++)
    {
        if(lines[l].is_rtl_ && lines[l].checkLineRTL())
        {
            trimFirstSpace(&lines[l]);
            trimLastSpace(&lines[l]);
            //CRLog::trace("reverse line text = [%s]",LCSTR(lines[l].getText()));
            lines[l].list_ = reverseLine(lines[l],clip_width);

            //reverse addition to result for rtl lines
            for (int c = lines[l].list_.length() -1 ; c >= 0 ; c--)
            {
                result_list.add(lines[l].list_.get(c));
            }
            if(rtl_space)
            {
                TextRect space;
                space = getZeroTxRect(&lines[l]);
                result_list.add(space);
            }
        }
        else
        {
           // TextRect space;
           // space = getZeroTxRect(&lines[l]);
           // result_list.add(space);
            for (int c = 0; c < lines[l].list_.length(); c++)
            {
                result_list.add(lines[l].list_.get(c));
            }
        }
    }
    return result_list;
}



//textfmt side

void PrintRTL(LVArray<WordItem> WordItems, LVDrawBuf * buf, LVFont * font, int space_width)
{
    if(WordItems.empty() || buf == NULL || font == NULL)
    {
        return;
    }

    int linewidth = 0;
    for (int i = 0; i < WordItems.length(); i++)
    {
        WordItem curr = WordItems.get(i);
        if(curr.getText().firstChar() == ' ')
        {
            linewidth += space_width;
        }
        else
        {
            linewidth += curr.width_;
        }
    }
    lvRect clip;
    buf->GetClipRect(&clip);
    int startx = WordItems.get(0).x_;
    int leftspace = startx - clip.left;
    int rightspace = clip.right - (startx + linewidth);
    startx = startx - leftspace + rightspace;
    bool line_isRTL = false;
    for (int i = 0; i < WordItems.length(); i++)
    {
        if(WordItems.get(i).is_rtl_)
        {
            line_isRTL = true;
            break;
        }
    }
    WordItem last = WordItems.get(WordItems.length()-1);
    if(line_isRTL && last.getText().lastChar() == ' ')
    {
        startx -= font->getCharWidth(' ');
    }
    LVArray<WordItem> nonRTLBuffer;
    int buffwidth = 0;
    bool prev_state = WordItems.get(WordItems.length() - 1).is_rtl_;

    /* lString16 text;
    for (int w = 0 ; w <  WordItems.length(); w++)
    {
        WordItem curr = WordItems.get(w);
        text += lString16(curr.text_,curr.len_);
    }
    CRLog::error("fmt line = [%s]",LCSTR(text));
    */

    for (int w = WordItems.length() - 1; w >= 0; w--)
    {
        WordItem curr = WordItems.get(w);
        int width = (curr.getText() == " ") ? space_width : curr.width_ ;

        if( curr.is_rtl_ || (curr.hasPunct() && prev_state))
        {
            if(nonRTLBuffer.length()>0)
            {
                WordItem firstitem = nonRTLBuffer.get(0);
                WordItem lastitem = nonRTLBuffer.get(nonRTLBuffer.length()-1);

                if(firstitem.getText() == " ")
                {
                    nonRTLBuffer.remove(0);
                    nonRTLBuffer.add(firstitem);
                }
                if(lastitem.getText() == " ")
                {
                    nonRTLBuffer.remove(nonRTLBuffer.length()-1);
                    nonRTLBuffer.insert(0,lastitem);
                }
                int startx_nonrtlbuff = startx - buffwidth;
                for (int k =nonRTLBuffer.length()-1; k >=0 ; k--)
                {
                    WordItem curr_buff = nonRTLBuffer.get(k);
                    int curr_buff_width = (curr_buff.getText() == " ") ? space_width : curr_buff.width_;

                    font->DrawTextString(buf,
                            startx_nonrtlbuff,
                            curr_buff.y_,
                            curr_buff.text_,
                            curr_buff.len_,
                            '?',
                            NULL,
                            curr_buff.flgHyphen_,
                            curr_buff.srcline_->flags & 0x0F00,
                            curr_buff.srcline_->letter_spacing,
                            false,
                            0);
                    startx_nonrtlbuff += curr_buff_width;
                }
                buffwidth = 0;
                nonRTLBuffer.clear();
            }

            font->DrawTextString(buf,
                    startx,
                    curr.y_,
                    curr.text_,
                    curr.len_,
                    '?',
                    NULL,
                    curr.flgHyphen_,
                    curr.srcline_->flags & 0x0F00,
                    curr.srcline_->letter_spacing,
                    true,
                    0);
            startx += width;
        }
        else
        {
            nonRTLBuffer.add(WordItem(startx,
                    curr.y_,
                    curr.text_,
                    curr.len_,
                    curr.flgHyphen_,
                    curr.srcline_,
                    false,
                    curr.width_)
            );
            startx += width;
            buffwidth +=width;
        }
        prev_state = curr.is_rtl_;
    }
    if(nonRTLBuffer.length()>0)
    {
        WordItem firstitem = nonRTLBuffer.get(0);
        WordItem lastitem = nonRTLBuffer.get(nonRTLBuffer.length()-1);

        if(line_isRTL && firstitem.getText() == " ")
        {
            nonRTLBuffer.remove(0);
            nonRTLBuffer.add(firstitem);
        }
        if(line_isRTL && lastitem.getText() == " ")
        {
            nonRTLBuffer.remove(nonRTLBuffer.length()-1);
            nonRTLBuffer.insert(0,lastitem);
        }
        int startx_nonrtlbuff = startx - buffwidth;
        for (int k =nonRTLBuffer.length()-1; k >=0 ; k--)
        {
            WordItem curr_buff = nonRTLBuffer.get(k);
            font->DrawTextString(buf,
                    startx_nonrtlbuff,
                    curr_buff.y_,
                    curr_buff.text_,
                    curr_buff.len_,
                    '?',
                    NULL,
                    curr_buff.flgHyphen_,
                    curr_buff.srcline_->flags & 0x0F00,
                    curr_buff.srcline_->letter_spacing,
                    false,
                    0);
            int width = (curr_buff.getText() == " ") ? space_width : curr_buff.width_;
            startx_nonrtlbuff += width;
        }
        nonRTLBuffer.clear();
    }
}

//text side

LetterMap arabicLetterMap = ArabicLetterMap();

ReverseLetterMap reverseLetterMap = ArabicReverseLetterMap();


bool char_isPunct(const lChar16 c){
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

bool char_isRTL(const lChar16 c){
    if(c < 0x0590)
    {
        return false;
    }
    return ( //(c==0x00A6) ||  // UNICODE "BROKEN BAR"
           (c>=0x0590)&&(c<=0x05FF)) ||
           (c == 0x05BE) || (c == 0x05C0) || (c == 0x05C3) || (c == 0x05C6) ||
           ((c>=0x05D0)&&(c<=0x05F4)) ||
           (c==0x0608) || (c==0x060B) ||
           (c==0x060D) ||
           ((c>=0x0600)&&(c<=0x06FF)) ||
           ((c>=0x06FF)&&(c<=0x0710)) ||
           ((c>=0x0712)&&(c<=0x072F)) ||
           ((c>=0x074D)&&(c<=0x07A5)) ||
           ((c>=0x07B1)&&(c<=0x07EA)) ||
           ((c>=0x07F4)&&(c<=0x07F5)) ||
           ((c>=0x07FA)&&(c<=0x0815)) ||
           (c==0x081A) || (c==0x0824) ||
           (c==0x0828) ||
           ((c>=0x0830)&&(c<=0x0858)) ||
           ((c>=0x085E)&&(c<=0x08AC)) ||
           (c==0x200F) || (c==0xFB1D) ||
           ((c>=0xFB1F)&&(c<=0xFB28)) ||
           ((c>=0xFB2A)&&(c<=0xFD3D)) ||
           ((c>=0xFD50)&&(c<=0xFDFC)) ||
           ((c>=0xFE70)&&(c<=0xFEFC)) ||
           ((c>=0x10800)&&(c<=0x1091B)) ||
           ((c>=0x10920)&&(c<=0x10A00)) ||
           ((c>=0x10A10)&&(c<=0x10A33)) ||
           ((c>=0x10A40)&&(c<=0x10B35)) ||
           ((c>=0x10B40)&&(c<=0x10C48)) ||
           ((c>=0x1EE00)&&(c<=0x1EEBB));
}

lString16 lString16::LigatureCheck(lString16 text)
{
    for (int i = 0 ; i < LAM_ALEF_COMBOS_LENGTH; i++) {
        text = LigatureCheck(text, lamAlefCombos[i][0], lamAlefCombos[i][1], lamAlefCombos[i][2]);
    }
    return text;
}

lString16 lString16::LigatureCheck(lString16 text, lChar16 lam, lChar16 alef, lChar16 lig)
{
    if(text.length() < 2)
    {
        return text;
    }

    lChar16 * alef_a = &alef;
    lChar16 * lam_a = &lam;
    lChar16 * lig_a = &lig;
    lString16 lamstr = lString16(lam_a,1);
    lString16 alefstr = lString16(alef_a,1);
    lString16 ligstr = lString16(lig_a,1);
    lString16 search = lamstr + alefstr;

    if(text.pos(search)==-1)
    {
        return text;
    }

    while (text.pos(search) !=-1)
    {
        text = text.replace(text.pos(search),2,ligstr);
    }

    return text;
}

bool char_is_RTL_left_unjoinable(lChar16 ch)
{
    return ((ch == 0x0627) ||
            (ch == 0x062F) ||
            (ch == 0x0630) ||
            (ch == 0x0631) ||
            (ch == 0x0632) ||
            (ch == 0x0648) ||
            (ch == 0x0621) || //Hamza
            (ch == 0x0623) || //alef + hamza above
            (ch == 0x0625) || //alef + hamza below
            (ch == 0x0623) || //alef + wavy hamza above
            (ch == 0x0672) || //alef + wavy hamza below
            (ch == 0x0673) || //alef + madda
            (ch == 0x0671) || //alef + wasla
            (ch == 0xFEFC) || //Lam-alif final
            (ch == 0xFEFB)); //Lam-alif isolated
}

lString16 lString16::PrettyLetters(lString16 text)
{
    if (text.empty())
    {
        return text;
    }

    lString16 restext;
    for (int i = 0; i < text.length(); i++)
    {
        lChar16 ch = text.at(i);
        if( arabicLetterMap.find(ch) != arabicLetterMap.end() )
        {
            unsigned int pos = arabic_isolated;
            if (text.length() == 1)
            {
                pos = arabic_isolated;
            }
            else if (i == 0)
            {
                pos = arabic_start;
            }
            else if (i > 0)
            {
                if (char_is_RTL_left_unjoinable(text.at(i - 1)))
                {
                    if (i == text.length() - 1)
                    {
                        pos = arabic_isolated;
                    }
                    else
                    {
                        pos = arabic_start;
                    }
                }
                else
                {
                    if (i == text.length() - 1)
                    {
                        pos = arabic_end;
                    }
                    else
                    {
                        pos = arabic_mid;
                    }
                }
            }

            lChar16 replace = arabicLetterMap.at(ch).at(pos);
            lChar16 * replace_a = &replace;
            lString16 replacestr = lString16(replace_a,1);
            restext += replacestr;
        }
        else
        {
            lChar16 * ch_a = &ch;
            lString16 ch_str = lString16(ch_a,1);
            restext += ch_str;
        }
        //restext += lString16::itoa(i);
    }
    return restext;
}

std::vector<lString16> lString16::ArabicSplitToWords()
{
    std::vector<lString16> result;

    lString16 line = *this;

    int start = 0;
    bool last_space = false;
    bool last_punct = false;

    lChar16 last_text = line.at(0);
    bool last_state = char_isRTL(last_text);

    for (int c = 0; c < line.length(); c++)
    {
        lChar16 ch = line.at(c);

        bool is_space = ch == ' ';
        bool is_punct = char_isPunct(ch);

        //bool curr_state = (is_space)? last_state : char_isRTL(ch);
        bool curr_state;
        if(is_space)
        {
            curr_state = last_state;
        }
        else if(is_punct)
        {
            curr_state = (last_space)? false : last_state ;
        }
        else
        {
            curr_state = char_isRTL(ch);
        }
        //curr_state = (is_punct && last_space)? : curr_state
        bool break_char = (is_space || last_space || is_punct || last_punct);

        //CRLog::error("letter = [%s]",LCSTR(curr_text));
        if (curr_state != last_state || break_char )
        {
            int len = c-start;
            if(len>0)
            {
                lString16 word;
                for (int i = start; i < c; i++)
                {
                    word += line.at(i);
                }
                result.push_back(word);
                start = c;
            }
        }
        last_state = curr_state;
        last_space = is_space;
        last_punct = is_punct;
    }
    lString16 word;
    for (int i = start; i < line.length(); i++)
    {
        word += line.at(i);
    }
    result.push_back(word);
    return result;
}

bool lString16::CheckRTL()
{
    for (int i = 0; i < this->length(); i++)
    {
        if (char_isRTL(this->at(i)))
        {
            return true;
        }
    }
    return false;
}

lString16 lString16::PrepareRTL()
{
    //CRLog::error("preparertl");
    if(this->length() < 0)
    {
        return *this;
    }
    if(!this->CheckRTL())
    {
        return *this;
    }
    std::vector<lString16> words = this->ArabicSplitToWords();
    lString16 result;
    for (int i = 0; i < words.size(); i++)
    {
        lString16 word = PrettyLetters(LigatureCheck(words.at(i)));
        result += word;
    }
    *this = result;
    return *this;
}

lString16 lString16::ReversePrettyLetters()
{
    if (this->empty())
    {
        return *this;
    }

    lString16 result;
    for (int i = 0; i < this->length(); i++)
    {
        lChar16 lam = 0x0644;
        lChar16 alef = 0x003F; //question mark "?"

        lChar16 ch = this->at(i);
        if( reverseLetterMap.find(ch) != reverseLetterMap.end() )
        {
            lChar16 replace = reverseLetterMap.at(ch);
            lString16 replacestr;

            bool lamAlefFound = (ch == 0xFEFA || ch == 0xFEFB || ch == 0xFEFC || ch == 0xFEF5 ||
                                 ch == 0xFEF6 || ch == 0xFEF7 || ch == 0xFEF8 || ch == 0xFEF9) ;
            if(lamAlefFound)
            {
                if      (replace == 0x0627) { alef = 0x0627; }
                else if (replace == 0x0622) { alef = 0x0622; }
                else if (replace == 0x0623) { alef = 0x0623; }
                else if (replace == 0x0625) { alef = 0x0625; }
                replacestr = lString16(&lam,1)+  lString16(&alef,1);
            }
            else
            {
                replacestr = lString16(&replace,1);
            }
            result += replacestr;
        }
        else
        {
            result += lString16(&ch,1);
        }
    }
    return result;
}
