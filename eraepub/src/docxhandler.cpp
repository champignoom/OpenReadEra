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

#include "../include/docxhandler.h"
#include "../include/lvstring.h"
#include "../include/lvstream.h"
#include "../include/epubfmt.h"
#include "../include/FootnotesPrinter.h"

//extract main document path
lString16 DocxGetMainFilePath(LVContainerRef m_arc)
{
    LVStreamRef container_stream = m_arc->OpenStream(L"[Content_Types].xml", LVOM_READ);
    if (!container_stream.isNull())
    {
        CrDom *doc = LVParseXMLStream(container_stream);
        if (doc)
        {
            for (int i = 1; i < 50; i++)
            {
                ldomNode *item = doc->nodeFromXPath(lString16("Types/Override[") << fmt::decimal(i) << "]");
                if (!item)
                {
                    break;
                }
                lString16 partname = item->getAttributeValue("PartName");
                lString16 contentType = item->getAttributeValue("ContentType");
                if (contentType.endsWith("document.main+xml"))
                {
                    return partname;
                }
            }
            delete doc;
        }
    }
    return lString16::empty_str;
}
//extract footnotes document path
lString16 DocxGetFootnotesFilePath(LVContainerRef m_arc)
{
    LVStreamRef container_stream = m_arc->OpenStream(L"[Content_Types].xml", LVOM_READ);
    if (!container_stream.isNull())
    {
        CrDom *doc = LVParseXMLStream(container_stream);
        if (doc)
        {
            for (int i = 1; i < 50; i++)
            {
                ldomNode *item = doc->nodeFromXPath(lString16("Types/Override[") << fmt::decimal(i) << "]");
                if (!item)
                {
                    break;
                }
                lString16 partname = item->getAttributeValue("PartName");
                lString16 contentType = item->getAttributeValue("ContentType");
                if (contentType.endsWith("footnotes+xml"))
                {
                    return partname;
                }
            }
            delete doc;
        }
    }
    return lString16::empty_str;
}
//extract links from relationships
DocxLinks DocxGetRelsLinks(LVContainerRef m_arc)
{
    DocxLinks linkslist;
    LVStreamRef container_stream = m_arc->OpenStream(L"word/_rels/document.xml.rels", LVOM_READ);
    if (!container_stream.isNull())
    {
        CrDom *doc = LVParseXMLStream(container_stream);
        if (doc)
        {

            for (int i = 1; i < 300; i++)
            {
                ldomNode *item = doc->nodeFromXPath(lString16("Relationships/Relationship[") << fmt::decimal(i) << "]");
                if (!item)
                {
                    break;
                }
                lString16 id = item->getAttributeValue("Id");
                lString16 type = item->getAttributeValue("Type");
                lString16 target = item->getAttributeValue("Target");
                lString16 targetmode = item->getAttributeValue("TargetMode");

                if (type.endsWith("hyperlink") && targetmode == "External")
                {
                    DocxLink *link = new DocxLink;
                    link->id_=id;
                    link->type_=type;
                    link->target_=target;
                    link->targetmode_=targetmode;
                    linkslist.add(link);
                }
            }
            delete doc;
        }
    }
    return linkslist;
}

//left that method for toc or other usage implementetion
DocxItems DocxParseContentTypes(LVContainerRef m_arc)
{
    LVStreamRef container_stream = m_arc->OpenStream(L"[Content_Types].xml", LVOM_READ);
    DocxItems docxItems_empty;
    if (!container_stream.isNull())
    {
        CrDom *doc = LVParseXMLStream(container_stream);
        if (doc)
        {
            DocxItems docxItems;
            for (int i = 1; i < 50; i++)
            {
                ldomNode *item = doc->nodeFromXPath(lString16("Types/Override[") << fmt::decimal(i) << "]");
                if (!item)
                {
                    break;
                }
                lString16 partname = item->getAttributeValue("PartName");
                lString16 contentType = item->getAttributeValue("ContentType");

                DocxItem *docxItem = new DocxItem;
                docxItem->href = partname;
                docxItem->mediaType = contentType;
                docxItems.add(docxItem);
            }
            return docxItems;
        }
        delete doc;

    }
    return docxItems_empty;
}

int DocxGetStyleNodeFontSize(ldomNode * node)
{
    if(node->isNodeName("sz"))
    {
        lString16 val = node->getAttributeValue("val");
        if(val!=lString16::empty_str)
        {
            return atoi(LCSTR(val));
        }
    }

    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode * child = node->getChildNode(i);
        int val = DocxGetStyleNodeFontSize(child);
        if(val!=-1)
        {
            return val;
        }
    }
    return -1;
}

lString16 DocxGetStyleName(ldomNode * node)
{
    if(node->isNodeName("name"))
    {
        lString16 name = node->getAttributeValue("val");
        if(name!=lString16::empty_str)
        {
            return name;
        }
    }

    for (int i = 0; i < node->getChildCount(); i++)
    {
        ldomNode * child = node->getChildNode(i);
        lString16 name = DocxGetStyleName(child);
        if(!name.empty())
        {
            return name;
        }
    }
    return lString16::empty_str;
}

DocxStyles DocxParseStyles(LVContainerRef m_arc)
{
    LVStreamRef container_stream = m_arc->OpenStream(L"word/styles.xml", LVOM_READ);
    DocxStyles DocxStyles_empty;
    if (!container_stream.isNull())
    {
        CrDom *doc = LVParseXMLStream(container_stream);
        if (doc)
        {
            DocxStyles DocxStyles;
            int i = 0;
            while (i<500)
            {
                ldomNode *item = doc->nodeFromXPath(lString16("styles/style[") << fmt::decimal(i) << "]");
                if (!item)
                {
                    break;
                }
                lString16 type = item->getAttributeValue("type");

                if (type == "paragraph")
                {
                    lString16 styleId   = item->getAttributeValue("styleId");
                    lString16 isDefault = item->getAttributeValue("default");

                    int size = DocxGetStyleNodeFontSize(item);
                    lString16 name = DocxGetStyleName(item);
                    if (size != -1)
                    {
                        if (isDefault == "1" && DocxStyles.default_size_ == -1)
                        {
                            DocxStyle *style = new DocxStyle(type, size, styleId, true, name);
                            DocxStyles.default_size_ = size;
                            DocxStyles.add(style);
                        }
                        else if (isDefault != "1")
                        {
                            DocxStyle *style = new DocxStyle(type, size, styleId, false, name);
                            DocxStyles.add(style);
                        }
                    }
                }
                i++;
            }
            if(DocxStyles.default_size_<0)
            {
                ldomNode *item = doc->nodeFromXPath(lString16("styles/docDefaults"));
                if(item!=NULL)
                {
                    DocxStyles.default_size_ = DocxGetStyleNodeFontSize(item);
                }
            }
            if (DocxStyles.generateHeaderFontSizes())
            {
                return DocxStyles;
            }
        }
        delete doc;
    }
    return DocxStyles_empty;
}

bool DocxStyles::updateMiniMax()
{
    for (int i = 0; i < this->length(); i++)
    {
        DocxStyle* curr = this->get(i);
        if(curr->fontSize_>max_)
            max_ = curr->fontSize_;
        if(curr->fontSize_<min_)
            min_ = curr->fontSize_;
    }
    return !(this->min_ == 500 || this->max_ == -1);
};

void DocxStyles::checkForHeaders()
{
    for (int i = 0; i < this->length(); i++)
    {
        DocxStyle* curr = this->get(i);
        lString16 curr_name = curr->name_;
        if(curr_name.pos("head")!=-1)
        {
            //CRLog::error("curr name = [%s]",LCSTR(curr_name));
            lString16Collection namefrags;
            namefrags.parse(curr_name,' ', false);
            for (int i = 0; i < namefrags.length(); i++)
            {
                lString16 namefrag  = namefrags.at(i);
                if(namefrag.DigitsOnly())
                {
                    int headnum = namefrag.atoi();
                    switch (headnum)
                    {
                        case 1: h1id_ = (h1isset)? h1id_: curr->styleId_; break;
                        case 2: h2id_ = (h2isset)? h2id_: curr->styleId_; break;
                        case 3: h3id_ = (h3isset)? h3id_: curr->styleId_; break;
                        case 4: h4id_ = (h4isset)? h4id_: curr->styleId_; break;
                        case 5: h5id_ = (h5isset)? h5id_: curr->styleId_; break;
                        case 6: h6id_ = (h6isset)? h6id_: curr->styleId_; break;
                        default:
                            CRLog::error("Error: Unknown header style found! [%s]",LCSTR(curr_name));
                            break;
                    }
                }
            }
        }
    }
}

bool DocxStyles::generateHeaderFontSizes()
{
    checkForHeaders();
    updateMiniMax();
    if (this->default_size_ < 0 )
    {
        return false;
    }
    int range_size = this->max_ - this->default_size_;
    if (range_size < 0)
    {
        return false;
    }
    int step = range_size / 6;
    h6min_ = default_size_;
    h5min_ = default_size_ + step;
    h4min_ = default_size_ + (step * 2);
    h3min_ = default_size_ + (step * 3);
    h2min_ = default_size_ + (step * 4);
    h1min_ = default_size_ + (step * 5);
    return true;
}

DocxStyle * DocxStyles::findById(const lString16 &id)
{
    if (id.empty())
    {
        return NULL;
    }
    for (int i = 0; i < length(); i++)
    {
        if (get(i)->styleId_ == id)
        {
            return get(i);
        }
    }
    return NULL;
}

DocxStyles & DocxStyles::operator=(const DocxStyles &v)
{
    for (int i = 0; i < this->length(); ++i)
    {
        this->set(i,v.get(i));
    }
    return *this;
}

int DocxStyles::getSizeById(const lString16 &id)
{
    if (id.empty())
    {
        return -1;
    }
    for (int i = 0; i < length(); i++)
    {
        if (get(i)->styleId_ == id)
        {
            return get(i)->fontSize_;
        }
    }
    return -1;
}

//main docx document importing routine
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
/*        // for debug
        {
            LVStreamRef out = LVOpenFileStream("/data/data/org.readera/files/document.xml.rels.xml", LVOM_WRITE);
            doc2->saveToStream(out, NULL, true);
        }
*/
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
 /*
 CRLog::error("styles.legnth = %d",docxStyles.length());
    CRLog::error("min = %d",docxStyles.min_);
    CRLog::error("def = %d",docxStyles.default_size_);
    CRLog::error("max = %d",docxStyles.max_);
    for (int i = 0; i < docxStyles.length(); i++)
    {
        DocxStyle* style = docxStyles.get(i);
        CRLog::error("style = [%s] [%s] [%d] [%d]",LCSTR(style->type_),LCSTR(style->styleId_),style->fontSize_,style->isDefault_?1:0);
    }
*/
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
    PrettyPrintDocx(m_doc->getRootNode());
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