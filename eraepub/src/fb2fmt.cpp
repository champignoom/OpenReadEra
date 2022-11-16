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

#include "include/fb2fmt.h"
#include "include/FootnotesPrinter.h"


bool ImportFb2Document(LVStreamRef stream, CrDom *m_doc, bool firstpage_thumb)
{
    LvDomWriter writer(m_doc);
    LvXmlParser parser(stream, &writer, false, true, firstpage_thumb);
    LVArray<LinkStruct> LinksList;
    EpubItems epubItems;

    if (!parser.CheckFormat())
    {
        CRLog::trace("!parser->CheckFormat()");
        // delete parser;
        return false;
    }
    parser.setLinksList(LinksList);
    parser.setEpubNotes(epubItems);
    if (!parser.Parse())
    {
        CRLog::trace("!parser->Parse()");
        // delete parser;
        return false;
    }
    LinksList = parser.getLinksList();
    //CRLog::error("Linkslist length = %d",LinksList.length());
    //for (int i = 0; i < LinksList.length(); i++)
    //{
    //	CRLog::error("LinksList %d = %s = %s",LinksList.get(i).num_,LCSTR(LinksList.get(i).id_),LCSTR(LinksList.get(i).href_));
    //}
    if (LinksList.length() > 0)
    {
        writer.OnStart(&parser);
        FootnotesPrinter printer(m_doc);
        printer.PrintLinksList(LinksList);
        writer.OnStop();
    }

    LE("m_doc->getToc()->getChildCount() = %d",m_doc->getToc()->getChildCount());
    if (m_doc->getToc()->getChildCount() == 0)
    {
        GetTOC(m_doc, m_doc->getToc(), false);
        if (m_doc->getToc()->getChildCount() == 0)
        {
            CRLog::info("Deeper TOC search");
            GetTOC(m_doc, m_doc->getToc(), true);
        }
        if (m_doc->getToc()->getChildCount() == 0)
        {
            CRLog::info("Superdeep TOC search");
            GetTOC(m_doc, m_doc->getToc(), true, true);
        }
    }
    else
    {
        CRLog::trace("TOC already exists. No TOC generation for now.");
    }
    return true;
}

