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

#include <ore_log.h>
#include "include/EpubItems.h"

void CssStyle::parseShorthandMargin(const lString16& in)
{
    if(in.empty())
    {
        return;
    }
    lString16Collection coll;
    coll.parse(in, L" ", true);
    switch (coll.length())
    {
        case 0:
            LE("no elements in margin attribute for class %s",LCSTR(name_));
            break;
        case 1:
            margin_top_    = coll.at(0);
            margin_right_  = coll.at(0);
            margin_bottom_ = coll.at(0);
            margin_left_   = coll.at(0);
            break;
        case 2:
            margin_top_    = coll.at(0);
            margin_right_  = coll.at(1);
            margin_bottom_ = coll.at(0);
            margin_left_   = coll.at(1);
            break;
        case 3:
            margin_top_    = coll.at(0);
            margin_right_  = coll.at(1);
            margin_bottom_ = coll.at(2);
            margin_left_   = coll.at(1);
            break;
        case 4:
            margin_top_    = coll.at(0);
            margin_right_  = coll.at(1);
            margin_bottom_ = coll.at(2);
            margin_left_   = coll.at(3);
            break;
        default:
            margin_top_    = coll.at(0);
            margin_right_  = coll.at(1);
            margin_bottom_ = coll.at(2);
            margin_left_   = coll.at(3);
            CRLog::error("malformed margin css tag for [%s]",LCSTR(this->name_));
            break;
    }
}

void CssStyle::parseShorthandPadding(const lString16& in)
{
    if(in.empty())
    {
        return;
    }
    lString16Collection coll;
    coll.parse(in,L" ",true);
    switch (coll.length())
    {
        case 0:
            LE("no elements in padding attribute for class %s",LCSTR(name_));
            break;
        case 1:
            padding_top_    = coll.at(0);
            padding_right_  = coll.at(0);
            padding_bottom_ = coll.at(0);
            padding_left_   = coll.at(0);
            break;
        case 2:
            padding_top_    = coll.at(0);
            padding_right_  = coll.at(1);
            padding_bottom_ = coll.at(0);
            padding_left_   = coll.at(1);
            break;
        case 3:
            padding_top_    = coll.at(0);
            padding_right_  = coll.at(1);
            padding_bottom_ = coll.at(2);
            padding_left_   = coll.at(1);
            break;
        case 4:
            padding_top_    = coll.at(0);
            padding_right_  = coll.at(1);
            padding_bottom_ = coll.at(2);
            padding_left_   = coll.at(3);
            break;
        default:
            padding_top_    = coll.at(0);
            padding_right_  = coll.at(1);
            padding_bottom_ = coll.at(2);
            padding_left_   = coll.at(3);
            CRLog::error("malformed padding css tag for [%s]",LCSTR(this->name_));
            break;
    }
}

CssStyle::CssStyle(lString16 in, lString16 codebase)
{
    source_line_ = in;
    lString16 rest;
    for (int i = 0; i < in.length(); i++)
    {
        lChar16 curr = in.at(i);
        if(curr!='{')
        {
            continue;
        }
        else
        {
            name_ = in.substr(0,i).trimDoubleSpaces(false,false,false);
            rest = in.substr(i,in.length()-i);
            break;
        }
    }
    //LE("name = [%s]",LCSTR(name_));
    if(rest.empty())
    {
        //CRLog::error("rest empty");
        style_string_filtered      = lString16::empty_str;
        style_string_fonts         = lString16::empty_str;
        style_string_margins       = lString16::empty_str;
        style_string_fonts_margins = lString16::empty_str;
        style_string_no_filtering  = lString16::empty_str;
        return;
    }
    //CRLog::error("rest = %s", LCSTR(rest));
    style_string_no_filtering = name_ + rest;

    direction_ = getAttrval(rest, lString16("direction"));
    if(direction_.empty())
    {
        direction_ = getAttrval(rest, lString16("dir"));
    }
    lString16 t_a = getAttrval(rest, lString16("text-align"));
    if(!t_a.empty())
    {
        if      (t_a == lString16("left"))   { text_align_ = ta_left;}
        else if (t_a == lString16("right"))  { text_align_ = ta_right;}
        else if (t_a == lString16("center")) { text_align_ = ta_center;}
        else if (t_a == lString16("justify")){ text_align_ = ta_justify;}
        else                                     { text_align_ = ta_inherit;}
    }

    //forbid padding and margins for <body> tags
    if(name_ != "body")
    {
        margin_top_      = getAttrval(rest, lString16("margin-top"));
        margin_bottom_   = getAttrval(rest, lString16("margin-bottom"));
        margin_left_     = getAttrval(rest, lString16("margin-left"));
        margin_right_    = getAttrval(rest, lString16("margin-right"));

        if (margin_top_.empty()    &&
            margin_bottom_.empty() &&
            margin_left_.empty()   &&
            margin_right_.empty())
        {
            lString16 m = getAttrval(rest, lString16("margin"));
            parseShorthandMargin(m);
        }

        padding_top_      = getAttrval(rest, lString16("padding-top"));
        padding_bottom_   = getAttrval(rest, lString16("padding-bottom"));
        padding_left_     = getAttrval(rest, lString16("padding-left"));
        padding_right_    = getAttrval(rest, lString16("padding-right"));

        if (padding_top_.empty()    &&
            padding_bottom_.empty() &&
            padding_left_.empty()   &&
            padding_right_.empty())
        {
            lString16 p = getAttrval(rest, lString16("padding"));
            parseShorthandPadding(p);
        }

        text_indent_ = getAttrval(rest,lString16("text-indent"));
    }

    font_weight_     = getAttrval(rest, lString16("font-weight"));
    font_style_      = getAttrval(rest, lString16("font-style"));
    text_decoration_ = getAttrval(rest, lString16("text-decoration"));
    list_style_type_ = getAttrval(rest, lString16("list-style-type"));
    display_         = getAttrval(rest, lString16("display"));

    font_family_     = getAttrval(rest, lString16("font-family"));
    while (font_family_.pos("\'")!=-1)
    {
        font_family_.replace(L"\'", L"\"");
    }
    int pos = font_family_.pos("\"");
    while (pos !=-1)
    {
        font_family_.erase(pos);
        pos = font_family_.pos("\"");
    }

    lString16 bg = getAttrval(rest, lString16("background-image"));
    if(bg.empty())
    {
        bg = getAttrval(rest, lString16("background"));
        if((bg.pos("url") == -1) && (bg.pos("src") == -1))
        {
            bg.clear();
        }
    }
    if(!bg.empty())
    {
        bg = bg.substr(bg.pos("(") + 1);
        bg = bg.substr(0, bg.pos(")"));
        if (!bg.empty())
        {
            if (!bg.startsWith("..") && !bg.startsWith("/"))
            {
                //adding tilde to override convertHref() path conversion
                bg = "~" + bg;
            }
            background_image_ = bg;
        }
    }

    vertical_align_ = getAttrval(rest, lString16("vertical-align"));
    if( vertical_align_ != L"sub" && vertical_align_ != L"super") { vertical_align_.clear(); }

    style_string_filtered      = formatCSSstring(false, false);
    style_string_fonts         = formatCSSstring(true , false);
    style_string_margins       = formatCSSstring(false, true);
    style_string_fonts_margins = formatCSSstring(true , true);

    if(name_ == "@font-face")
    {
        lString16 src = getAttrval(rest, lString16("src"));
        if(src.startsWith("url"))
        {
            src = src.substr(3);
        }
        else if(src.startsWith("local"))
        {
            src = src.substr(5);
        }
        int pos2 = src.pos("format");
        if(pos2!=-1)
        {
            src = src.substr(0,pos2);
        }
        while(src.startsWith(" "))
        {
            src = src.substr(1);
        }
        while(src.endsWith(" "))
        {
            src = src.substr(0,src.length()-1);
        }

        if(src.pos(" ")!= -1)
        {
            int pos3 = src.pos(",");
            while (pos3 !=-1)
            {
                src.replace(L",",L" ");
                pos3 = src.pos(",",pos3);
            }

            lString16Collection coll;
            coll.parse(src, ' ', false);
            for (int i = 0; i < coll.length(); i++)
            {
                lString16 frag = coll.at(i);
                if(frag.empty())
                {
                    continue;
                }

                if(frag.startsWith("url"))
                {
                    frag = frag.substr(3);
                    src = frag;
                    break;
                }
            }
        }
        while(src.startsWith(" ") || src.startsWith("(") || src.startsWith("\""))
        {
            src = src.substr(1);
        }
        while(src.endsWith(" ") || src.endsWith(")") || src.endsWith("\""))
        {
            src = src.substr(0,src.length()-1);
        }
        if(codebase != lString16::empty_str && (!src.startsWith(codebase)))
        {
            while(src.startsWith(".") || src.startsWith("/"))
            {
                src = src.substr(1);
            }
            src = LVCombinePaths(codebase, src);
        }
        font_src_ = src;
    }

    //CRLog::error("style string = %s", LCSTR(style_string_));
};

lString16 CssStyle::getAttrval(lString16 in, lString16 attrname)
{
    lString16 result;
    int attr_start = in.pos(attrname);
    if(attr_start == -1)
    {
        //CRLog::error("getAttrval [%s] return empty str",LCSTR(attrname));
        return result;
    }

    int attr_end = -1;
    for (int i = attr_start; i < in.length(); i++)
    {
        lChar16 curr = in.at(i);
        if (curr == ';' || curr == '}')
        {
            attr_end = i;
            break;
        }
    }

    lString16 attrstr = in.substr(attr_start, attr_end - attr_start);
    //CRLog::error("attrstr = [%s]", LCSTR(attrstr));

    if(attrstr.pos(":")!=-1)
    {
        int colonpos = attrstr.pos(":")+1;
        result = attrstr.substr(colonpos,attrstr.length()-colonpos);
    }

    lString16 imp(L"!important");
    int pos = result.pos(imp);
    while(pos!=-1)
    {
        result = result.replace(pos, imp.length(),lString16::empty_str);
        pos = result.pos(imp);
    }
    //CRLog::error("getAttrval [%s] return [%s]",LCSTR(attrname),LCSTR(result.trimDoubleSpaces(false, false, false)));
    return result.trimDoubleSpaces(false, false, false);
}

lString16 CssStyle::formatCSSstring(bool fonts, bool margins)
{
    lString16 text_align;
    switch (text_align_)
    {
        case ta_left    : text_align = lString16("left");    break;
        case ta_right   : text_align = lString16("right");   break;
        case ta_center  : text_align = lString16("center");  break;
        case ta_justify : text_align = lString16("justify"); break;
        case ta_inherit : text_align = lString16::empty_str;     break;
    }

    lString16 str;
    if(!display_.empty())         { str += "display:" + display_ + "; ";}
    if(!font_weight_.empty())     { str += "font-weight:" + font_weight_ + "; ";}
    if(!font_style_.empty())      { str += "font-style:" + font_style_ + "; ";}
    if(!text_decoration_.empty()) { str += "text-decoration:" + text_decoration_ + "; ";}
    if(!list_style_type_.empty()) {
        if(list_style_type_ == "upper-latin") {list_style_type_ = "upper-alpha";}
        if(list_style_type_ == "lower-latin") {list_style_type_ = "lower-alpha";}
        str += "list-style-type:" + list_style_type_ + "; ";
    }
    if(fonts &&   !font_family_.empty())   { str += "font-family:"    + font_family_   + "; "; }
    if (margins)
    {
        if (!text_align.empty())     { str += "text-align: "    + text_align     + "; ";}
        if (!margin_top_.empty()     && margin_top_     != L"0") { str += "margin-top: "     + margin_top_     + "; ";}
        if (!margin_bottom_.empty()  && margin_bottom_  != L"0") { str += "margin-bottom: "  + margin_bottom_  + "; ";}
        if (!margin_left_.empty()    && margin_left_    != L"0") { str += "margin-left: "    + margin_left_    + "; ";}
        if (!margin_right_.empty()   && margin_right_   != L"0") { str += "margin-right: "   + margin_right_   + "; ";}

        if (!padding_top_.empty()    && padding_top_    != L"0") { str += "padding-top: "    + padding_top_    + "; ";}
        if (!padding_bottom_.empty() && padding_bottom_ != L"0") { str += "padding-bottom: " + padding_bottom_ + "; ";}
        if (!padding_left_.empty()   && padding_left_   != L"0") { str += "padding-left: "   + padding_left_   + "; ";}
        if (!padding_right_.empty()  && padding_right_  != L"0") { str += "padding-right: "  + padding_right_  + "; ";}

        if (!text_indent_.empty()) { str += "text-indent: "  + text_indent_  + "; ";}
    }

    if(!vertical_align_.empty()) { str += "vertical-align: " + vertical_align_ + "; ";}

    if(str.empty())
    {
        //CRLog::error(" str.empty() class [%s], sourceline = [%s]",LCSTR(name_),LCSTR(source_line_));
        return lString16::empty_str;
    }
    if(!name_.empty())
    {
        str = name_ + " { " + str + "}";
    }
    else
    {
        str = "{" + str + "}";
    }
    //LE("class = [%s]", LCSTR(str));
    return str;
}

CssStyle CssStyle::OverwriteClass(CssStyle add)
{
    if (this->source_line_.empty() || add.source_line_.empty())
    {
        return *this;
    }

    if (add.name_ != this->name_)
    {
        return *this;
    }
    source_line_ = source_line_ + "\n" + add.source_line_;

    //if (margin_top_!= add.margin_top_ && !add.margin_top_.empty())
    //    margin_top_ = add.margin_top_;

    //if (margin_bottom_!= add.margin_bottom_ && !add.margin_bottom_.empty())
    //    margin_bottom_ = add.margin_bottom_;

    //if (margin_left_!= add.margin_left_ && !add.margin_left_.empty())
    //    margin_left_ = add.margin_left_;

    //if (margin_right_!= add.margin_right_ && !add.margin_right_.empty())
    //    margin_right_ = add.margin_right_;

    //if (padding_top_!= add.padding_top_ && !add.padding_top_.empty())
    //    padding_top_ = add.padding_top_;

    //if (padding_bottom_!= add.padding_bottom_ && !add.padding_bottom_.empty())
    //    padding_bottom_ = add.padding_bottom_;

    //if (padding_left_!= add.padding_left_ && !add.padding_left_.empty())
    //    padding_left_ = add.padding_left_;

    //if (padding_right_!= add.padding_right_ && !add.padding_right_.empty())
    //    padding_right_ = add.padding_right_;

    if (text_indent_ != add.text_indent_ && !add.text_indent_.empty())
        text_indent_ = add.text_indent_;

    if (direction_ != add.direction_ && !add.direction_.empty())
        direction_ = add.direction_;

    if (font_weight_ != add.font_weight_ && !add.font_weight_.empty())
        font_weight_ = add.font_weight_;

    if (font_style_ != add.font_style_ && !add.font_style_.empty())
        font_style_ = add.font_style_;

    if (text_decoration_ != add.text_decoration_ && !add.text_decoration_.empty())
        text_decoration_ = add.text_decoration_;

    if (background_image_ != add.background_image_ && !add.background_image_.empty())
        background_image_ = add.background_image_;

    if (list_style_type_ != add.list_style_type_ && !add.list_style_type_.empty())
        list_style_type_ = add.list_style_type_;

    if (display_ != add.display_ && !add.display_.empty())
        display_ = add.display_;

    //if (text_align_ != add.text_align_ && add.text_align_ > ta_left)
    //    text_align_ = add.text_align_;

    if (font_family_ != add.font_family_ && !add.font_family_.empty())
        font_family_ = add.font_family_;

    if (font_src_ != add.font_src_ && !add.font_src_.empty())
        font_src_ = add.font_src_;

    if (vertical_align_ != add.vertical_align_ && !add.vertical_align_.empty())
        vertical_align_ = add.vertical_align_;

    style_string_filtered      = formatCSSstring(false, false);
    style_string_fonts         = formatCSSstring(true , false);
    style_string_margins       = formatCSSstring(false, true);
    style_string_fonts_margins = formatCSSstring(true , true);

    //CRLog::error("merged style_string_fonts_margins is[%s]",LCSTR(style_string_fonts_margins));

    return *this;
}

LVEmbeddedFontDef *CssStyle::getfontDef()
{
    //LE("CssStyle::getfontDef [%s], [%s], %d %d",LCSTR(font_src_),LCSTR(font_family_),isBold(),isItalic());
    return new LVEmbeddedFontDef(font_src_, UnicodeToUtf8(font_family_), isBold(), isItalic());
}

bool EpubStylesManager::CheckClassName(lString16 name)
{
    //CRLog::error("check classname = [%s]",LCSTR(name));
    if(name.empty())
    {
        return false;
    }
    for (int i = 0; i < name.length(); i++)
    {
        lChar16 ch = name.at(i);
        if ((ch >= 45 && ch <= 57)     || //0-9
            (ch >= 65 && ch <= 90)     || //A-Z
            (ch >= 97 && ch <= 122)    || //a-z
            (ch == ' ') || (ch == '.') ||
            (ch == ',') || (ch == '=') ||
            (ch == '_') || (ch == '"') ||
            (ch == '<') || (ch == '>') ||
            (ch == '[') || (ch == ']'))
        {
            continue;
        }
        else
        {
            //CRLog::error("Found illegal character in CSS class name: [%s] -> [%lc]",LCSTR(name),ch);
            return false;
        }
    }

    lChar16 last = 0;
    int q_count = 0;
    bool br_open = false;
    for (int i = 0; i < name.length(); i++)
    {
        lChar16 ch = name.at(i);
        if (last == '.' || last == ' ' || last == ',')
        {
            if ((ch >= 45 && ch <= 57) || ch == '-')
            {
                //CRLog::error("Illegal character combination in css class name: [%s] -> [%lc][%lc]",LCSTR(name),last,ch);
                return false;
            }
        }
        if(ch == '[')
        {
            if(br_open)
            {
                //CRLog::error("brackets error 1");
                return false;
            }
            br_open = true;
        }
        else if(ch == ']')
        {
            if(!br_open)
            {
                //CRLog::error("brackets error 2");
                return false;
            }
            br_open = false;
        }
        else if(ch=='"')
        {
            q_count++;
        }

        last = ch;
    }
    if(q_count %2 != 0 || br_open)
    {
        //CRLog::error("unpaired quotes or br_open");
        return false;
    }

    return true;
}

lString16Collection EpubStylesManager::splitSelectorToClasses(lString16 selector)
{
    lString16Collection result;
    selector = selector.trimDoubleSpaces(false,false,false);
    //CRLog::error("selector in = %s",LCSTR(selector));
    if(selector.pos(" ")!=-1||selector.pos(",")!=-1)
    {
        lString16 buf;
        bool save = false;
        for (int i = 0; i < selector.length(); i++)
        {
            lChar16 curr = selector.at(i);
            if(curr == '.')
            {
                save = true;
            }
            else if(curr == ',')
            {
                result.add(buf);
                buf.clear();
                save = false;
                continue;
            }
            else if( i == selector.length()-1)
            {
                buf +=curr;
                result.add(buf);
                buf.clear();
                save = false;
                break;
            }
            if(save)
            {
                buf += curr;
            }
        }
        return result;
    }
    else
    {
        result.add(selector);
        return result;
    }
}

void EpubStylesManager::addCssRTLClass(CssStyle css)
{
    if (css.isRTL())
    {
        lString16Collection names = splitSelectorToClasses(css.name_);
        for (int i = 0; i < names.length(); i++)
        {
            lString16 name = names.at(i);
            name = (name.startsWith("."))? name.substr(1,css.name_.length()-1) : name ;
            rtl_map_.insert(std::make_pair(name.getHash(), css));
            //CRLog::error("rtl class in map = %s",LCSTR(name));
        }
    }

}

void EpubStylesManager::addCSSClass(CssStyle css , EpubCSSMap *map)
{
    addCssRTLClass(css);

    if (css.name_.empty())
    {
        //LE("name_ empty");
        return;
    }
    if( css.source_line_.empty())
    {
        //LE("source_line_ empty");
        return;
    }
    if(css.style_string_fonts_margins.empty() &&
       css.style_string_filtered.empty() &&
       css.background_image_.empty())
    {
        //LE("style_string_ empty");
        return;
    }
    if(!CheckClassName(css.name_))
    {
        //LE("class check failed");
        return;
    }
    int hash = css.name_.getHash();
    if (map->find(hash) != map->end())
    {
        CssStyle base = map->at(hash);
        //LE("overwriting \n[%s]\n over \n[%s]",  LCSTR(css.style_string_fonts_margins), LCSTR(base.style_string_fonts_margins));
        map->operator[](hash) = base.OverwriteClass(css);
        return;
    }
    //LT("EpubCSSclass added [%s] ",LCSTR(css.name_));
    map->operator[](hash) = css;

    //classes_array_.add(css);
}

lString16Collection EpubStylesManager::SplitToClasses(lString16 in)
{
    lString16Collection result;
    bool comment_skip = false;
    lString16 cssStr;
    for (int i = 0; i < in.length(); i++)
    {
        lChar16 curr = in.at(i);
        lChar16 next = (i < in.length() - 1) ? in.at(i + 1) : 0;

        if (curr == '*' && next == '/')
        {
            comment_skip = false;
            i++;
            continue;
        }
        if (comment_skip)
        {
            continue;
        }
        if (curr == '/' && next == '*')
        {
            comment_skip = true;
            i++;
            continue;
        }
        cssStr += curr;

        if (curr == '}')
        {
            //LE("cssStr add = [%s]", LCSTR(cssStr));
            result.add(cssStr.trim());
            cssStr.clear();
        }
    }
    for (int i = 0; i < result.length(); i++)
    {
        //remove each element that has no "{" or "}"
        if(result.at(i).pos("{")==-1 || result.at(i).pos("}")==-1)
        {
            result.erase(i,1);
        }
    }
    return result;
}

void EpubStylesManager::parseString(lString16 in)
{
    {
        in = in.trimDoubleSpaces(false,false,false);
        lString16Collection classes_str_coll = SplitToClasses(in);
        for (int i = 0; i < classes_str_coll.length(); i++)
        {
            CssStyle cssClass = CssStyle(classes_str_coll.at(i),codeBase);
            this->addCSSClass(cssClass, &classes_map_);
            if(!cssClass.font_src_.empty())
            {
                embedded_font_classes.add(cssClass);
            }
        }
    }
}

void EpubStylesManager::Finalize()
{
    EpubCSSMap newMap;
    std::map<lUInt32 , CssStyle>::iterator it = classes_map_.begin();
    while (it != classes_map_.end())
    {
        //lUInt32 hash = it->first; //key
        CssStyle curr = it->second; //value

        if(!curr.style_string_filtered.empty())
        {
            all_classes_filtered.append(curr.style_string_filtered);
            all_classes_filtered.append("\n");
        }

        if(!curr.style_string_fonts.empty())
        {
            all_classes_fonts.append(curr.style_string_fonts);
            all_classes_fonts.append("\n");
        }

        if(!curr.style_string_margins.empty())
        {
            all_classes_margins.append(curr.style_string_margins);
            all_classes_margins.append("\n");
        }

        if( !curr.style_string_fonts_margins.empty())
        {
            all_classes_fonts_margins.append(curr.style_string_fonts_margins);
            all_classes_fonts_margins.append("\n");
        }

        if(!curr.style_string_no_filtering.empty())
        {
            all_classes_not_filtered.append(curr.style_string_no_filtering);
            all_classes_not_filtered.append("\n");
        }
        lString16 className = curr.name_;
        while (className.pos(L".")!=-1)
        {
            className = className.substr(className.pos(L".")+1);
        }
        curr.name_ = className;
        addCSSClass(curr,&classes_map_);
        it++;
    }
    //classes_map_.clear();
    //classes_map_ = newMap;
    //classes_map_.insert(newMap.begin(), newMap.end());
}

bool EpubStylesManager::ClassIsRTL(lString16 name) //array scan
{
    if(rtl_map_.empty())
    {
        return false;
    }
    if(rtl_map_.find(name.getHash())!=rtl_map_.end())
    {
        return true;
    }
    return false;
}

lString16 EpubStylesManager::as_string()
{
    switch (gEmbeddedStylesLVL)
    {
        case 1: return all_classes_filtered;
        case 2: return all_classes_fonts;
        case 3: return all_classes_margins;
        case 4: return all_classes_fonts_margins;
        case 5: return all_classes_not_filtered;
        default: CRLog::error("embedded styles level = 0!");
            return lString16::empty_str;
    }
}

bool EpubStylesManager::classExists(lString16 className)
{
    if(classes_map_.empty())
    {
        return false;
    }
    if(classes_map_.find(className.getHash())!=classes_map_.end())
    {
        return true;
    }
    return false;
}

CssStyle EpubStylesManager::getClass(lString16 className)
{
    if(classes_map_.empty())
    {
        return CssStyle();
    }
    if(classes_map_.find(className.getHash())!=classes_map_.end())
    {
        return classes_map_.at(className.getHash());
    }
    return CssStyle();
}

bool EpubStylesManager::classIsBold(lString16 className)
{
    if(classes_map_.find(className.getHash())!=classes_map_.end())
    {
        return classes_map_[className.getHash()].isBold();
    }
    return false;
}

bool EpubStylesManager::classIsItalic(lString16 className)
{
    if(classes_map_.find(className.getHash())!=classes_map_.end())
    {
        return classes_map_[className.getHash()].isItalic();
    }
    return false;
}

bool EpubStylesManager::classIsUnderline(lString16 className)
{
    if(classes_map_.find(className.getHash())!=classes_map_.end())
    {
        return classes_map_[className.getHash()].isUnderline();
    }
    return false;
}
