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

#include "include/fb3fmt.h"
#include "include/FootnotesPrinter.h"
#include "include/lvstream.h"
#include "include/lvtinydom.h"
#include "include/epubfmt.h"

lString16 GetFb3CoverImagePath(LVContainerRef container)
{
    LVStreamRef result;

    lString16 thumbnail_file_name = L"cover.jpg";
    result = container->OpenStream(thumbnail_file_name.c_str(), LVOM_READ);
    if (!result.isNull())
    {
        return thumbnail_file_name;
    }
    thumbnail_file_name = L"fb3/cover.jpg";
    result = container->OpenStream(thumbnail_file_name.c_str(), LVOM_READ);
    if (!result.isNull())
    {
        return thumbnail_file_name;
    }
    thumbnail_file_name = L"fb3/img/cover.jpg";
    result = container->OpenStream(thumbnail_file_name.c_str(), LVOM_READ);
    if (!result.isNull())
    {
        return thumbnail_file_name;
    }
    //search for files containing "cover" in its name
    LVContainer* c = container.get();
    for (int i = 0; i < c->GetObjectCount(); i++)
    {
        lString16 path = c->GetObjectInfo(i)->GetName();
        path.lowercase();
        if(!c->GetObjectInfo(i)->IsContainer() && path.pos("cover")!=-1)
        {
            if (path.endsWith(".jpg") || path.endsWith(".png") || path.endsWith(".bmp"))
            {
                thumbnail_file_name = c->GetObjectInfo(i)->GetName();
                result = container->OpenStream(thumbnail_file_name.c_str(), LVOM_READ);
                if (!result.isNull())
                {
                    return thumbnail_file_name;
                }
            }
        }
    }

    //search for files containing "thumb" in its name
    for (int i = 0; i < c->GetObjectCount(); i++)
    {
        lString16 path = c->GetObjectInfo(i)->GetName();
        path.lowercase();
        if(!c->GetObjectInfo(i)->IsContainer() && path.pos("thumb")!=-1)
        {
            if (path.endsWith(".jpg") || path.endsWith(".png") || path.endsWith(".bmp"))
            {
                thumbnail_file_name = c->GetObjectInfo(i)->GetName();
                result = container->OpenStream(thumbnail_file_name.c_str(), LVOM_READ);
                if (!result.isNull())
                {
                    return thumbnail_file_name;
                }
            }
        }
    }

    CRLog::error("GetFb3CoverImagePath: no cover found");
    return lString16::empty_str;
}

bool ImportFb3Document(LVStreamRef in_stream, CrDom *m_doc, bool firstpage_thumb)
{
    LVContainerRef arc = LVOpenArchive(in_stream);
    if (arc.isNull())
    {
        CRLog::error("This FB3 is corrupted: not a ZIP archive");
        return false; // not a ZIP archive
    }
    // check if there is document.xml
    lString16 rootfilePath = L"fb3/body.xml";
    //lString16 stylesPath = L"styles.xml";

    EncryptedDataContainer *decryptor = new EncryptedDataContainer(arc);
    if (decryptor->open())
    {
        CRLog::debug("Fb3: encrypted items detected");
        return false;
    }

    LVContainerRef m_arc = LVContainerRef(decryptor);

    if (decryptor->hasUnsupportedEncryption())
    {
        // DRM!!!
        return false;
    }

    m_doc->setDocParentContainer(m_arc);

    LVStreamRef content_stream = m_arc->OpenStream(rootfilePath.c_str(), LVOM_READ);

    if (content_stream.isNull())
    {
        CRLog::error("ImportFb3Document : failed to open rootfile %s ", LCSTR(rootfilePath));
        CRLog::error("ImportFb3Document : Retrying...");
        LVStreamRef ctypes = m_arc->OpenStream(L"[Content_Types].xml", LVOM_READ);
        CrDom *dom = LVParseXMLStream(ctypes);
        if (dom)
        {
            for (int i = 1; i < 500; i++)
            {
                lString16 xpath = lString16("Types/Override[") << fmt::decimal(i) << "]";
                ldomNode *item = dom->nodeFromXPath(xpath);
                if (!item)
                {
                    break;
                }
                if(item->getAttributeValue("ContentType") == "application/fb3-body+xml")
                {
                    rootfilePath = item->getAttributeValue("PartName");
                    content_stream = m_arc->OpenStream(rootfilePath.c_str(), LVOM_READ);
                    if (content_stream.isNull())
                    {
                        CRLog::error("ImportFb3Document : Retry failed!");
                        return false;
                    }
                    else
                    {
                        CRLog::error("ImportFb3Document : Retry succeeded!!");
                    }
                }
            }
        }
    }
    content_stream.Clear();

    LvDomWriter writer(m_doc);


    lString16 root_file_path = L"fb3/description.xml";
    content_stream = arc->OpenStream(root_file_path.c_str(), LVOM_READ);

    lString16 annotation;
    lString16 date;
    lString16 coverPath;

    if (!content_stream.isNull())
    {
        CrDom *dom = LVParseXMLStream(content_stream);
        if (!dom)
        {
            CRLog::error("processMeta: malformed FB3 (2)");
        }
        else
        {
            annotation = dom->textFromXPath(lString16("fb3-description/annotation")).trim();
            date = dom->textFromXPath(lString16("fb3-description/written/date")).trim();
            coverPath = GetFb3CoverImagePath(arc);
        }
    }
    writer.setFlags( 0 );
    writer.OnStart(NULL);

    bool pagebreak = false;
    if(!annotation.empty())
    {
        writer.OnTagOpen(L"", L"annotation");
        writer.OnText(annotation.c_str(), annotation.length(), 0);
        writer.OnTagClose(L"", L"annotation");
        pagebreak = true;
    }
    if(!date.empty())
    {
        writer.OnTagOpen(L"", L"date");
        writer.OnText(date.c_str(), date.length(), 0);
        writer.OnTagClose(L"", L"date");
        pagebreak = true;
    }
    if(!coverPath.empty())
    {
        writer.OnTagOpen(L"", L"img");
        writer.OnAttribute(L"",L"src",coverPath.c_str());
        writer.OnTagClose(L"", L"img");
        pagebreak = true;
    }
    if(pagebreak )
    {
        writer.OnTagOpenAndClose(L"", L"pagebreak");
    }

    LVArray<LinkStruct> LinksList;
    LinksMap LinksMap;
    Epub3Notes epub3Notes;

    std::map<lUInt32,lString16> relsMap = BuildFb3RelsMap(arc);

    //parse main document
    LVStreamRef stream = m_arc->OpenStream(rootfilePath.c_str(), LVOM_READ);
    if (!stream.isNull())
    {
        //LvHtmlParser parser(stream2, &writer, firstpage_thumb);
        LvXmlParser  parser(stream, &writer, false, false, firstpage_thumb);

        parser.setLinksList(LinksList);
        parser.setLinksMap(LinksMap);
        parser.setEpub3Notes(epub3Notes);
        parser.setFb3Relationships(relsMap);
        if (parser.Parse())
        {
            LinksList = parser.getLinksList();
            LinksMap = parser.getLinksMap();
            epub3Notes = parser.getEpub3Notes();
            relsMap = parser.getFb3Relationships();
            // valid
        }
        else
        {
            CRLog::error("Unable to parse fb3 file at [%s]", LCSTR(rootfilePath));
        }
    }
    writer.OnTagClose(L"", L"body");

    if(!LinksList.empty())
    {
        //CRLog::error("Linkslist length = %d",LinksList.length());
        //for (int i = 0; i < LinksList.length(); i++)
        //{
        //	CRLog::error("LinksList %d = %s = %s",i,LCSTR(LinksList.get(i).id_),LCSTR(LinksList.get(i).href_));
        //}


        //visible footnotes
        Epub3NotesPrinter visible_printer(m_doc,epub3Notes);
        visible_printer.PrintLinksList(LinksList);

        //invisible footnotes
        FootnotesPrinter invisible_printer(m_doc);
        invisible_printer.PrintLinksList(LinksList);
    }

    writer.OnStop();

    if (m_doc->getToc()->getChildCount() == 0)
    {
        GetTOC(m_doc, m_doc->getToc(), false,true);
        if (m_doc->getToc()->getChildCount() == 0)
        {
            CRLog::info("Deeper TOC search");
            GetTOC(m_doc, m_doc->getToc(), true,true);
        }
    }
    return true;
}

void GetFb3Metadata(CrDom *dom, lString16 *res_title, lString16 *res_authors, lString16 *res_lang,
        lString16 *res_series, int *res_series_number, lString16 *res_genre, lString16 *res_annotation)
{
    lString16 authors;
    lString16 lang;
    lString16 series;
    lString16 genre;
    int series_number = 0;
    for (int i = 1; i < 20; i++)
    {
        lString16 xpath = lString16("fb3-description/fb3-relations/subject[") << fmt::decimal(i) << "]";
        ldomNode *item = dom->nodeFromXPath(xpath);
        if (!item)
        {
            break;
        }
        if(item->getAttributeValue("link") == "author")
        {
            for (int i = 0; i <item->getChildCount() ; i++)
            {
                ldomNode *child = item->getChildNode(i);
                if(child->isNodeName("title"))
                {
                    authors += child->getText().trim() + lString16("|");
                }
            }
        }
    }
    if(authors.endsWith("|"))
    {
        authors= authors.substr(0,authors.length()-1);
    }

    *res_authors = authors;

    for (int i = 1; i < 20; i++)
    {
        lString16 xpath = lString16("fb3-description/fb3-classification/subject[") << fmt::decimal(i) << "]";
        ldomNode *item = dom->nodeFromXPath(xpath);
        if (!item)
        {
            break;
        }
        genre += item->getText().trim() + lString16("|");
    }
    if(genre.endsWith("|"))
    {
        genre = genre.substr(0,genre.length()-1);
    }

    *res_genre = genre;


    *res_title      = dom->textFromXPath(lString16("fb3-description/title/main")).trim();
    *res_title      += " " + dom->textFromXPath(lString16("fb3-description/title/sub")).trim();
    *res_lang       = dom->textFromXPath(lString16("fb3-description/lang")).trim();
    *res_annotation = dom->textFromXPath(lString16("fb3-description/annotation")).trim();

    *res_series = series; // empty
    *res_series_number = series_number ; // 0
    return;
}

LVStreamRef GetFb3CoverImage(LVContainerRef container)
{
    lString16 thumbnail_file_name = GetFb3CoverImagePath(container);
    LVStreamRef result = container->OpenStream(thumbnail_file_name.c_str(), LVOM_READ);
    if (result.isNull())
    {
        CRLog::error("GetFb3CoverImage: no cover found");
        return LVStreamRef();
    }
    return result;
}

std::map<lUInt32,lString16> BuildFb3RelsMap(LVContainerRef container)
{
    std::map<lUInt32,lString16> map;
    LVArray<lString16> names;
    for (int i = 0; i < container->GetObjectCount(); i++)
    {
        lString16 path = container->GetObjectInfo(i)->GetName();
        names.add(path);
    }

    for (int i = 0; i < container->GetObjectCount(); i++)
    {
        lString16 path = container->GetObjectInfo(i)->GetName();
        lString16 path_low = path;
        path_low.lowercase();
        if(!container->GetObjectInfo(i)->IsContainer() && path_low.endsWith(".rels"))
        {
            LVStreamRef content_stream = container->OpenStream(path.c_str(), LVOM_READ);

            if (content_stream.isNull())
            {
                continue;
            }
            CrDom *dom = LVParseXMLStream(content_stream);
            if (!dom)
            {
                continue;
            }

            for (int i = 1; i < 5000; i++)
            {
                lString16 xpath = lString16("Relationships/Relationship[") << fmt::decimal(i) << "]";
                ldomNode *item = dom->nodeFromXPath(xpath);
                if (!item)
                {
                    break;
                }
                lString16 id     = item->getAttributeValue("Id");
                lString16 type   = item->getAttributeValue("Type");
                lString16 target = item->getAttributeValue("Target");
                if(target.startsWith("./")) {target = target.substr(2);};
                if(target.startsWith("/")) {target = target.substr(1);};

                if(type.pos("image")!=-1 || type.pos("thumbnail")!=-1)
                {
                    for (int i = 0; i < names.length(); i++)
                    {
                        if(names.get(i).pos(target)!=-1)
                        {
                            //CRLog::error("id = [%s] old target = [%s] target = [%s]", LCSTR(id), LCSTR(target), LCSTR(names.get(i)));
                            map.insert(std::make_pair(id.getHash(), DecodeHTMLUrlString(names.get(i))));
                            break;
                        }
                    }
                }
            }
        }
    }
    return map;
}