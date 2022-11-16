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

#include "include/lvtinydom.h"
#include "include/epubfmt.h"
#include "include/FootnotesPrinter.h"
#include "include/odthandler.h"
#include "include/lvstring.h"
#include "include/lvstream.h"

//main docx document importing routine
/*
bool ImportDocxDocument(LVStreamRef stream, CrDom *m_doc, bool firstpage_thumb)
{
    LVContainerRef arc = LVOpenArchive(stream);
    if (arc.isNull())
    {
        CRLog::error("This Docx is corrupted: not a ZIP archive");
        return false; // not a ZIP archive
    }
    // check if there is document.xml
    lString16 rootfilePath = DocxGetMainFilePath(arc);
    if (rootfilePath.empty())
    {
        CRLog::error("No main document file found! This is either not a docx file, or the file is corrupted!");
        return false;
    }
    lString16 footnotesFilePath = DocxGetFootnotesFilePath(arc);
    EncryptedDataContainer *decryptor = new EncryptedDataContainer(arc);
    if (decryptor->open())
    {
        CRLog::debug("DOCX: encrypted items detected");
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
        return false;
    }

    LVStreamRef content_stream2 = m_arc->OpenStream(L"/word/_rels/document.xml.rels", LVOM_READ);

    if (content_stream2.isNull())
    {
        return false;
    }
    //parse relationships
    CrDom *doc2 = LVParseXMLStream(content_stream2);
    if (!doc2)
    {
        return false;
    }
        // for debug
        //{
        //    LVStreamRef out = LVOpenFileStream("/data/data/org.readera/files/document.xml.rels.xml", LVOM_WRITE);
        //    doc2->saveToStream(out, NULL, true);
        //}

    //images handling
    DocxItems docxItems;
    int counter =0;
    while (1)
    {
        ldomNode *item = doc2->nodeFromXPath(lString16("Relationships/Relationship[") << fmt::decimal(counter) << "]");
        if (!item)
        {
            break;
        }
        lString16 id = item->getAttributeValue("Id");
        lString16 mediaType = item->getAttributeValue("Type");
        lString16 target = L"word/";
        target.append(item->getAttributeValue("Target"));
        if (mediaType.endsWith("/image"))
        {
            DocxItem *docxItem = new DocxItem;
            docxItem->href = target;
            docxItem->id = id;
            docxItem->mediaType = mediaType;
            docxItems.add(docxItem);
        }
        counter++;
    }
    CRPropRef m_doc_props = m_doc->getProps();

    LvDomWriter writer(m_doc);

    class TrDocxWriter: public LvDocFragmentWriter {
    public:
        TrDocxWriter(LvXMLParserCallback *parentWriter)
        //: LvDocFragmentWriter(parentWriter, cs16("body"), cs16("DocFragment"), lString16::empty_str)
                : LvDocFragmentWriter(parentWriter, cs16("body"), lString16::empty_str, lString16::empty_str)
        {
        }
    };

    TrDocxWriter appender(&writer);
    writer.setFlags( TXTFLG_TRIM | TXTFLG_PRE_PARA_SPLITTING | TXTFLG_KEEP_SPACES | TXTFLG_TRIM_ALLOW_END_SPACE | TXTFLG_TRIM_ALLOW_START_SPACE );
    writer.OnStart(NULL);
    writer.OnTagOpenNoAttr(L"", L"body");
    writer.OnTagOpenNoAttr(L"", L"body");

    DocxStyles docxStyles = DocxParseStyles(m_arc);

    //CRLog::error("styles.legnth = %d",docxStyles.length());
    //   CRLog::error("min = %d",docxStyles.min_);
    //   CRLog::error("def = %d",docxStyles.default_size_);
    //   CRLog::error("max = %d",docxStyles.max_);
    //   for (int i = 0; i < docxStyles.length(); i++)
    //   {
    //       DocxStyle* style = docxStyles.get(i);
    //       CRLog::error("style = [%s] [%s] [%d] [%d]",LCSTR(style->type_),LCSTR(style->styleId_),style->fontSize_,style->isDefault_?1:0);
    //   }

    DocxLinks docxLinks = DocxGetRelsLinks(m_arc);
    LVArray<LinkStruct> LinksList;
    LinksMap LinksMap;
    //parse main document
    LVStreamRef stream2 = m_arc->OpenStream(rootfilePath.c_str(), LVOM_READ);
    if (!stream2.isNull())
    {
        LvHtmlParser parser(stream2, &appender, firstpage_thumb);
        parser.setLinksList(LinksList);
        parser.setLinksMap(LinksMap);
        if (parser.ParseDocx(docxItems,docxLinks,docxStyles))
        {
            LinksList = parser.getLinksList();
            LinksMap = parser.getLinksMap();
            // valid
        }
        else
        {
            CRLog::error("Unable to parse docx file at [%s]", LCSTR(rootfilePath));
        }
    }
    writer.OnTagClose(L"", L"body");
    writer.OnTagOpen(L"", L"body");
    writer.OnAttribute(L"", L"name", L"notes");
    //parse footnotes document if exists
    LVStreamRef stream3;
    if(!footnotesFilePath.empty())
    {
        stream3 = m_arc->OpenStream(footnotesFilePath.c_str(), LVOM_READ);
        if (!stream3.isNull())
        {
            LvHtmlParser parser(stream3, &appender, firstpage_thumb);
            parser.setLinksMap(LinksMap);
            if (parser.ParseDocx(docxItems,docxLinks,docxStyles))
            {
                // valid
            }
            else
            {
                CRLog::error("Unable to parse footnotes file at [%s]", LCSTR(footnotesFilePath));
            }
        }
        else
        {
            CRLog::error("Failed opening footnotes file at [%s]",LCSTR(footnotesFilePath));
        }
    }
    else
    {
        CRLog::trace("No footnotes found in docx package.");
    }
    writer.OnTagClose(L"", L"body");
    //footnotes
    if(!LinksList.empty())
    {
        //CRLog::error("Linkslist length = %d",LinksList.length());
        //for (int i = 0; i < LinksList.length(); i++)
        //{
        //	CRLog::error("LinksList %d = %s = %s",i,LCSTR(LinksList.get(i).id_),LCSTR(LinksList.get(i).href_));
        //}

        writer.OnTagOpen(L"", L"body");
        writer.OnAttribute(L"", L"name", L"notes_hidden");
        FootnotesPrinter printer(m_doc);
        printer.PrintLinksList(LinksList);
        writer.OnTagClose(L"", L"body");
    }
    writer.OnTagClose(L"", L"body"); //main body closed
    writer.OnStop();

    m_doc->getToc()->clear();
    GetTOC(m_doc,m_doc->getToc());
#if 0 // set stylesheet
    //m_doc->getStylesheet()->clear();
	m_doc->setStylesheet( NULL, true );
	//m_doc->getStylesheet()->parse(m_stylesheet.c_str());
	if (!css.empty() && m_doc->getDocFlag(DOC_FLAG_ENABLE_INTERNAL_STYLES)) {
		m_doc->setStylesheet( "p.p { text-align: justify }\n"
			"svg { text-align: center }\n"
			"i { display: inline; font-style: italic }\n"
			"b { display: inline; font-weight: bold }\n"
			"abbr { display: inline }\n"
			"acronym { display: inline }\n"
			"address { display: inline }\n"
			"p.title-p { hyphenate: none }\n", false);
		m_doc->setStylesheet(UnicodeToUtf8(css).c_str(), false);
		//m_doc->getStylesheet()->parse(UnicodeToUtf8(css).c_str());
	} else {
		//m_doc->getStylesheet()->parse(m_stylesheet.c_str());
		//m_doc->setStylesheet( m_stylesheet.c_str(), false );
	}
#endif
    return true;
}
 */

bool OdtCheckAutospacing(LVStreamRef style_content_stream)
{
    // reading styles stream
    CrDom *styles = LVParseXMLStream(style_content_stream);
    if (styles)
    {
        CRPropRef m_doc_props = styles->getProps();

        for (int i = 1; i < 50000; i++)
        {
            ldomNode *item = styles->nodeFromXPath(lString16("document-styles/styles/default-style[") << fmt::decimal(i) << "]");
            if (!item)
            {
                break;
            }
            for (int i = 0; i < item->getChildCount(); i++)
            {
                ldomNode * child = item->getChildNode(i);
                if (child->getNodeName() != "paragraph-properties")
                {
                    continue;
                }
                lString16 value = child->getAttributeValue(L"text-autospace");
                if (!value.empty() && value != L"none")
                {
                    return true;
                    //CRLog::error("autospacing = true! (val = %s)", LCSTR(value));
                    break;
                }
            }
        }
    }
    return false;
}

OdtStyles OdtGetHeadersStyles(LVStreamRef style_content_stream)
{
    //CRLog::error("OdtGetHeadersStyles");
    OdtStyles result;
    // reading styles stream
    CrDom *styles = LVParseXMLStream(style_content_stream);
    if (styles)
    {
        CRPropRef m_doc_props = styles->getProps();

        for (int i = 1; i < 50000; i++)
        {
            ldomNode *item = styles->nodeFromXPath(lString16("document-styles/styles/style[") << fmt::decimal(i) << "]");
            if (!item || result.setfields>=6 )
            {
                break;
            }
            lString16 name = item->getAttributeValue(L"name");
            lString16 level = item->getAttributeValue(L"default-outline-level");
            if (!name.empty() && !level.empty())
            {
                result.addHeader(level.atoi(),name);
            }
        }
    }
    return result;
}

bool ImportOdtDocument(LVStreamRef stream, CrDom *m_doc, bool firstpage_thumb)
{
    LVContainerRef arc = LVOpenArchive(stream);
    if (arc.isNull())
    {
        CRLog::error("This ODT is corrupted: not a ZIP archive");
        return false; // not a ZIP archive
    }
    // check if there is document.xml
    //lString16 rootfilePath = OdtGetMainFilePath(arc);
    lString16 rootfilePath = L"content.xml";
    lString16 stylesPath = L"styles.xml";
    if (rootfilePath.empty())
    {
        CRLog::error("No main document file found! This is either not a ODT file, or the file is corrupted!");
        return false;
    }
    //lString16 footnotesFilePath = DocxGetFootnotesFilePath(arc);
    EncryptedDataContainer *decryptor = new EncryptedDataContainer(arc);
    if (decryptor->open())
    {
        CRLog::debug("DOCX: encrypted items detected");
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
        CRLog::error("ImportOdtDocument : failed to open rootfile %s",LCSTR(rootfilePath));
        return false;
    }



    LvDomWriter writer(m_doc);

    class TrOdtWriter: public LvDocFragmentWriter {
    public:
        TrOdtWriter(LvXMLParserCallback *parentWriter): LvDocFragmentWriter(parentWriter, cs16("body"), lString16::empty_str, lString16::empty_str)
        {
        }
    };

    TrOdtWriter appender(&writer);
    writer.setFlags( TXTFLG_TRIM | TXTFLG_PRE_PARA_SPLITTING | TXTFLG_KEEP_SPACES | TXTFLG_TRIM_ALLOW_END_SPACE | TXTFLG_TRIM_ALLOW_START_SPACE );
    writer.OnStart(NULL);
    writer.OnTagOpenNoAttr(L"", L"body");

/*
    //CRLog::error("styles.legnth = %d",docxStyles.length());
    //   CRLog::error("min = %d",docxStyles.min_);
    //   CRLog::error("def = %d",docxStyles.default_size_);
    //   CRLog::error("max = %d",docxStyles.max_);
    //   for (int i = 0; i < docxStyles.length(); i++)
    //   {
    //       DocxStyle* style = docxStyles.get(i);
    //       CRLog::error("style = [%s] [%s] [%d] [%d]",LCSTR(style->type_),LCSTR(style->styleId_),style->fontSize_,style->isDefault_?1:0);
    //   }
*/

    LVStreamRef style_content_stream = m_arc->OpenStream(stylesPath.c_str(), LVOM_READ);
    bool autospacing = OdtCheckAutospacing(style_content_stream);
    OdtStyles odtStyles = OdtGetHeadersStyles(style_content_stream);
    //CRLog::error("odtstyles setfields = %d",odtStyles.setfields);
    //CRLog::error("odtstyles 1 = %s",LCSTR(odtStyles.h1id_));
    //CRLog::error("odtstyles 2 = %s",LCSTR(odtStyles.h2id_));
    //CRLog::error("odtstyles 3 = %s",LCSTR(odtStyles.h3id_));
    //CRLog::error("odtstyles 4 = %s",LCSTR(odtStyles.h4id_));
    //CRLog::error("odtstyles 5 = %s",LCSTR(odtStyles.h5id_));
    //CRLog::error("odtstyles 6 = %s",LCSTR(odtStyles.h6id_));
    LVArray<LinkStruct> LinksList;
    LinksMap LinksMap;
    Epub3Notes epub3Notes;

    //parse main document
    LVStreamRef stream2 = m_arc->OpenStream(rootfilePath.c_str(), LVOM_READ);
    if (!stream2.isNull())
    {
        LvHtmlParser parser(stream2, &appender, firstpage_thumb);
        parser.setLinksList(LinksList);
        parser.setLinksMap(LinksMap);
        parser.setEpub3Notes(epub3Notes);
        if (parser.ParseOdt(/*docxItems,docxLinks,*/odtStyles))
        {
            LinksList = parser.getLinksList();
            LinksMap = parser.getLinksMap();
            epub3Notes = parser.getEpub3Notes();
            // valid
        }
        else
        {
            CRLog::error("Unable to parse odt file at [%s]", LCSTR(rootfilePath));
        }
    }

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
    m_doc->getToc()->clear();
    GetTOC(m_doc,m_doc->getToc());

    if(autospacing)
    {
        ldomNode * root = m_doc->getRootNode();
        FixOdtSpaces(root);
    }
    return true;
}




