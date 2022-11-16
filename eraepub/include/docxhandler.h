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

#ifndef _OPENREADERA_DOCXHANDLER_H_
#define _OPENREADERA_DOCXHANDLER_H_

#include "lvstring.h"
#include "lvptrvec.h"

class DocxItem
{
public:
    lString16 href;
    lString16 mediaType;
    lString16 id;

    DocxItem() {}

    DocxItem(const DocxItem &v) : href(v.href), mediaType(v.mediaType), id(v.id) {}

    DocxItem &operator=(const DocxItem &v)
    {
        href = v.href;
        mediaType = v.mediaType;
        id = v.id;
        return *this;
    }

    ~DocxItem() {}
};

class DocxItems : public LVPtrVector<DocxItem>
{
public:
    DocxItem *findById(const lString16 &id)
    {
        if (id.empty())
        {return NULL;}
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->id == id){return get(i);}
        }
        return NULL;
    }

    DocxItems &operator=(const DocxItems &v)
    {
        for (int i = 0; i < this->length(); ++i)
        {
            this->set(i,v.get(i));
        }
        return *this;
    }

    lString16 findHrefById(const lString16 &id)
    {
        if (id.empty())
        {
            return lString16::empty_str;
        }
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->id == id){return get(i)->href;}
        }
        return lString16::empty_str;
    }
};

class DocxLink
{
public:
    lString16 id_;
    lString16 type_;
    lString16 target_;
    lString16 targetmode_;

    DocxLink() {}
    DocxLink(const DocxLink &v) : id_(v.id_), type_(v.type_), target_(v.target_),targetmode_(v.targetmode_) {}
    DocxLink &operator=(const DocxLink &v)
    {
        id_ = v.id_;
        type_ = v.type_;
        target_ = v.target_;
        targetmode_ = v.targetmode_;
        return *this;
    }

    ~DocxLink() {}
};

class DocxLinks : public LVPtrVector<DocxLink>
{
public:
    DocxLink *findById(const lString16 &id)
    {
        if (id.empty())
        {return NULL;}
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->id_ == id){return get(i);}
        }
        return NULL;
    }

    DocxLinks &operator=(const DocxLinks &v)
    {
        for (int i = 0; i < this->length(); ++i)
        {
            this->set(i,v.get(i));
        }
        return *this;
    }

    lString16 findTargetById(const lString16 &id)
    {
        if (id.empty())
        {return lString16::empty_str;}
        for (int i = 0; i < length(); i++)
        {
            if (get(i)->id_ == id){return get(i)->target_;}
        }
        return lString16::empty_str;
    }
};

class DocxStyle
{
public:
    lString16 type_;
    lString16 styleId_;
    int fontSize_ = -1;
    bool isDefault_ = false;
    lString16 name_;

    DocxStyle() {}

    DocxStyle(const DocxStyle &v) : type_(v.type_), fontSize_(v.fontSize_), styleId_(v.styleId_),isDefault_(v.isDefault_),name_(v.name_) {}
    DocxStyle(lString16 type,int fontSize,lString16 styleId,bool isDefault,lString16 name) : type_(type)   , fontSize_(fontSize)   , styleId_(styleId)   ,isDefault_(isDefault),name_(name) {}

    DocxStyle &operator=(const DocxStyle &v)
    {
        type_ = v.type_;
        styleId_ = v.styleId_;
        fontSize_ = v.fontSize_;
        isDefault_ = v.isDefault_;
        return *this;
    }

    ~DocxStyle() {}
};

class DocxStyles : public LVPtrVector<DocxStyle>
{
private:

    bool updateMiniMax();

    bool h1isset = false;
    bool h2isset = false;
    bool h3isset = false;
    bool h4isset = false;
    bool h5isset = false;
    bool h6isset = false;

public:
    int min_ = 500;
    int max_ = -1;
    int default_size_ = -1;
    int h6min_ = -1;
    int h5min_ = -1;
    int h4min_ = -1;
    int h3min_ = -1;
    int h2min_ = -1;
    int h1min_ = -1;

    lString16 h1id_;
    lString16 h2id_;
    lString16 h3id_;
    lString16 h4id_;
    lString16 h5id_;
    lString16 h6id_;

    void checkForHeaders();

    bool generateHeaderFontSizes();

    DocxStyle *findById(const lString16 &id);

    DocxStyles &operator=(const DocxStyles &v);

    int getSizeById(const lString16 &id);
};

#endif //_OPENREADERA_DOCXHANDLER_H_
