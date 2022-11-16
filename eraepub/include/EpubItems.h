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

#ifndef _OPENREADERA_EPUBITEMS_H_
#define _OPENREADERA_EPUBITEMS_H_

#include <map>
#include "lvstring.h"
#include "lvarray.h"
#include "lvptrvec.h"
#include "crconfig.h"
#include "lvfntman.h"

typedef std::map<lUInt32,lString16> LinksMap;

class EpubItem
{
public:
    lString16 href;
    lString16 mediaType;
    lString16 id;
    lString16 title;

    EpubItem() {}

    EpubItem(const EpubItem &v) : href(v.href), mediaType(v.mediaType), id(v.id) {}
    EpubItem(lString16 href_new,lString16 mediaType_new,lString16 id_new,lString16 title_new) : href(href_new), mediaType(mediaType_new), id(id_new), title(title_new) {}

    EpubItem &operator=(const EpubItem &v)
    {
        href = v.href;
        mediaType = v.mediaType;
        id = v.id;
        return *this;
    }
};

class EpubItems : public LVPtrVector<EpubItem>
{
public:
    EpubItem *findById(const lString16 &id)
    {
        if (id.empty())
        {return NULL;}
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->id == id){return get(i);}
        }
        return NULL;
    }

    EpubItem *findByHref(const lString16 &href)
    {
        if (href.empty())
        {return NULL;}
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->href == href){return get(i);}
        }
        return NULL;
    }

    bool hrefCheck(lString16 string)
    {
        for (int i = 0; i < this->length(); i++)
        {
            if (string.pos(this->get(i)->href.c_str()) != -1) {return true;}
        }
        return false;
    }
};

class LinkStruct
{
public:
    int num_;
    lString16 href_ = lString16::empty_str;
    lString16 id_ = lString16::empty_str;
    LinkStruct(){}
    LinkStruct(int num, lString16 id, lString16 href):num_(num), id_(id), href_(href) {}
    ~LinkStruct(){}
};

class Epub3Notes
{
public:
    LinksMap AsidesMap_;
    lString16 FootnotesTitle_ = L"Footnotes";
    void addTitle(lString16 title){FootnotesTitle_ = title; }
    void AddAside(lString16 href) {
        lUInt32 hash = href.getHash();
        AsidesMap_[hash] = "1";
    }
    int size() { return AsidesMap_.size(); }
};

enum epub_CSS_attr_t_align {ta_inherit = 0, ta_left, ta_right, ta_center, ta_justify};

class CssStyle
{
    friend class EpubStylesManager;
private:
    lString16 source_line_;
    lString16 name_;
    lString16 style_string_filtered;
    lString16 style_string_fonts;
    lString16 style_string_margins;
    lString16 style_string_fonts_margins;
    lString16 style_string_no_filtering;

    //attributes
    lString16 margin_top_;
    lString16 margin_bottom_;
    lString16 margin_left_;
    lString16 margin_right_;
    lString16 padding_top_;
    lString16 padding_bottom_;
    lString16 padding_left_;
    lString16 padding_right_;
    lString16 text_indent_;
    lString16 direction_;
    lString16 font_weight_ ;
    lString16 font_style_;
    lString16 text_decoration_;
    lString16 background_image_;
    //lString16 background_color_;  // not used or implemented
    lString16 list_style_type_;
    lString16 display_;
    int text_align_ = ta_inherit;
    lString16 font_family_;
    lString16 font_src_;
    lString16 vertical_align_;
    //
    lString16 getAttrval(lString16 in, lString16 attrname);
    lString16 formatCSSstring(bool fonts, bool margins);
    CssStyle OverwriteClass(CssStyle add);


public:
    CssStyle() {};

    CssStyle(lString16 in, lString16 codebase = lString16::empty_str);

    bool isRTL() { return direction_ == L"rtl";}
    bool isBold()              { return font_weight_ == L"bold"; }
    bool isItalic()            { return font_style_ == L"italic";}
    bool isUnderline()         { return text_decoration_ == L"underline";}
    lString16 getBackgroundImageSrc()  { return background_image_;}

    inline lString16 getStyleStringFonts() { return style_string_fonts;}

    void parseShorthandMargin(const lString16& in);

    void parseShorthandPadding(const lString16& in);

    LVEmbeddedFontDef* getfontDef();
};

typedef std::map<lUInt32,CssStyle> EpubCSSMap;

class EpubStylesManager
{
private:
    EpubCSSMap rtl_map_;
    LVArray<CssStyle> classes_array_;
    bool CheckClassName(lString16 name);

    void addCSSClass(CssStyle css, EpubCSSMap *map);

    lString16Collection SplitToClasses(lString16 in);

    void addCssRTLClass(CssStyle css);

    lString16Collection splitSelectorToClasses(lString16 selector);

    EpubCSSMap classes_map_;

    lString16 all_classes_filtered;
    lString16 all_classes_fonts;
    lString16 all_classes_margins;
    lString16 all_classes_fonts_margins;
    lString16 all_classes_not_filtered;

public:
    lString16 codeBase = lString16::empty_str;

    LVArray<CssStyle> embedded_font_classes;

    EpubStylesManager() {};

    void Finalize();

    inline bool rtl_map_empty() { return rtl_map_.empty();}

    void parseString(lString16 in);

    bool ClassIsRTL(lString16 name);

    lString16 as_string();

    bool classExists(lString16 classname);

    CssStyle getClass(lString16 classname);

    bool classIsBold(lString16 className);

    bool classIsItalic(lString16 className);

    bool classIsUnderline(lString16 className);
};
#endif //_OPENREADERA_EPUBITEMS_H_
