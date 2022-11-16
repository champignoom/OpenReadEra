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

#ifndef _OPENREADERA_FOOTNOTESPRINTER_H_
#define _OPENREADERA_FOOTNOTESPRINTER_H_

#include "lvtinydom.h"
#include "fb2def.h"
#include <map>
typedef std::map<lUInt32,lString16> StrMap;

class FootnotesPrinter
{
protected:
    CrDom *doc_ = NULL;
    LvDomWriter* writer_ = NULL;
    lString16 title_ = lString16("Footnotes");
    bool hidden_ = true;
public:
    int textcounter_ = 0;
    lString16 main_href_;
    FootnotesPrinter(){}

    FootnotesPrinter(CrDom *m_doc){
        writer_ = new LvDomWriter(m_doc);
        doc_ = m_doc;
    }

    ldomNode *FindTextInNode(ldomNode *node);

    ldomNode *FindTextInParents(ldomNode *node);

    bool NodeContainsNextNote(ldomNode *node, lString16 nextId);

    bool NodeIsBreak(ldomNode *node, lString16 nextId);

    void recurseNodesToPrint(ldomNode *node);

    bool PrintLinksList(LVArray<LinkStruct> LinksList);

    virtual void PrintHeader();

    virtual void PrintNum(lString16 num , lString16 id);

    virtual bool PrintIsAllowed(lString16 href){ return true;};

    void PrintLinkNode(ldomNode *node);

    void PrintTextNode(ldomNode *node);
};

class Epub3NotesPrinter : public  FootnotesPrinter
{
private:
    LinksMap AsidesMap_;
public:
    Epub3NotesPrinter(){}

    Epub3NotesPrinter(CrDom *m_doc, Epub3Notes Epub3Notes){
        AsidesMap_ = Epub3Notes.AsidesMap_;
        title_ = Epub3Notes.FootnotesTitle_;
        writer_ = new LvDomWriter(m_doc);
        doc_ = m_doc;
        hidden_ = false;
    }
    bool PrintIsAllowed(lString16 href);

    void PrintNum(lString16 num , lString16 id);

    void PrintHeader();
};

#endif //_OPENREADERA_FOOTNOTESPRINTER_H_
