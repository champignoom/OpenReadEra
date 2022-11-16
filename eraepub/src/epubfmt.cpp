#include "include/epubfmt.h"
#include "include/docxhandler.h"
#include "include/EpubItems.h"
#include "include/FootnotesPrinter.h"




bool DetectEpubFormat(LVStreamRef stream)
{
	LVContainerRef m_arc = LVOpenArchive(stream);
	if (m_arc.isNull())
	{
	    LE("failed to open stream %s",LCSTR(stream.get()->GetName()));
		// Not a ZIP archive
		return false;
	}
	// Read "mimetype" file contents from root of archive
	lString16 mimeType;
	LVStreamRef mtStream = m_arc->OpenStream(L"mimetype", LVOM_READ);
	if (!mtStream.isNull())
	{
		int size = mtStream->GetSize();
        if (size == 0)
        {
            size = 20;
        }
		if (size > 4 && size < 100)
		{
			LVArray<char> buf(size + 1, '\0');
			if (mtStream->Read(buf.get(), size, NULL) == LVERR_OK)
			{
				for (int i = 0; i < size; i++)
				{
					if (buf[i] < 32 || ((unsigned char) buf[i]) > 127)
					{
						buf[i] = 0;
					}
				}
				buf[size] = 0;
				if (buf[0])
				{
					mimeType = Utf8ToUnicode(lString8(buf.get()));
				}
			}
		}
	}
	return mimeType == L"application/epub+zip";
}

void ReadEpubToc(CrDom *doc, ldomNode *mapRoot, LvTocItem *baseToc, LvDocFragmentWriter &appender)
{
	if (!mapRoot || !baseToc)
		return;
	lUInt16 navPoint_id = mapRoot->getCrDom()->getElementNameIndex(L"navPoint");
	lUInt16 navLabel_id = mapRoot->getCrDom()->getElementNameIndex(L"navLabel");
	lUInt16 content_id = mapRoot->getCrDom()->getElementNameIndex(L"content");
	lUInt16 text_id = mapRoot->getCrDom()->getElementNameIndex(L"text");
	for (int i = 0; i < 5000; i++)
	{
		ldomNode *navPoint = mapRoot->findChildElement(LXML_NS_ANY, navPoint_id, i);
		if (!navPoint)
			break;
		ldomNode *navLabel = navPoint->findChildElement(LXML_NS_ANY, navLabel_id, -1);
		if (!navLabel)
			continue;
		ldomNode *text = navLabel->findChildElement(LXML_NS_ANY, text_id, -1);
		if (!text)
			continue;
		ldomNode *content = navPoint->findChildElement(LXML_NS_ANY, content_id, -1);
		if (!content)
			continue;
		lString16 href = content->getAttributeValue("src");
		lString16 title = text->getText(' ');
		title.trimDoubleSpaces(false, false, false);
		if (href.empty() || title.empty())
			continue;
		//LV("TOC href before convert: %s", LCSTR(href));
		href = DecodeHTMLUrlString(href);
		href = appender.convertHref(href);
		//LV("TOC href after convert: %s", LCSTR(href));
		if (href.empty() || href[0] != '#')
			continue;
		ldomNode *target = doc->getNodeById(doc->getAttrValueIndex(href.substr(1).c_str()));
		if (!target)
			continue;
		ldomXPointer ptr(target, 0);
		LvTocItem *tocItem = baseToc->addChild(title, ptr, lString16::empty_str);
		ReadEpubToc(doc, navPoint, tocItem, appender);
	}
}

lString16 EpubGetRootFilePath(LVContainerRef m_arc)
{
	// check root media type
	lString16 rootfilePath;
	lString16 rootfileMediaType;
	// read container.xml
	{
		LVStreamRef container_stream = m_arc->OpenStream(L"META-INF/container.xml", LVOM_READ);
		if (!container_stream.isNull())
		{
			CrDom *doc = LVParseXMLStream(container_stream);
			if (doc)
			{
				ldomNode *rootfile = doc->nodeFromXPath(cs16("container/rootfiles/rootfile"));
				if (rootfile && rootfile->isElement())
				{
					rootfilePath = rootfile->getAttributeValue("full-path");
					rootfileMediaType = rootfile->getAttributeValue("media-type");
				}
				delete doc;
			}
		}
	}

	if (rootfilePath.empty() || rootfileMediaType != "application/oebps-package+xml")
		return lString16::empty_str;
	return rootfilePath;
}

/// encrypted font demangling proxy: XORs first 1024 bytes of source stream with key
class FontDemanglingStream : public StreamProxy
{
	LVArray<lUInt8> &_key;
public:
	FontDemanglingStream(LVStreamRef baseStream, LVArray<lUInt8> &key)
			: StreamProxy(baseStream), _key(key) {}

	virtual lverror_t Read(void *buf, lvsize_t count, lvsize_t *nBytesRead)
	{
		lvpos_t pos = _base->GetPos();
		lverror_t res = _base->Read(buf, count, nBytesRead);
		if (pos < 1024 && _key.length() == 16)
		{
			for (int i = 0; i + pos < 1024; i++)
			{
				int keyPos = (i + pos) & 15;
				((lUInt8 *) buf)[i] ^= _key[keyPos];
			}
		}
		return res;
	}

};

class EncryptedItem
{
public:
	lString16 _uri;
	lString16 _method;

	EncryptedItem(lString16 uri, lString16 method) : _uri(uri), _method(method) {}
};

class EncCallback : public LvXMLParserCallback
{
	bool insideEncryption;
	bool insideEncryptedData;
	bool insideEncryptionMethod;
	bool insideCipherData;
	bool insideCipherReference;
public:
	/// called on opening tag <
	virtual ldomNode *OnTagOpen(const lChar16 *nsname, const lChar16 *tagname)
	{
		CR_UNUSED(nsname);
		if (!lStr_cmp(tagname, "encryption"))
			insideEncryption = true;
		else if (!lStr_cmp(tagname, "EncryptedData"))
			insideEncryptedData = true;
		else if (!lStr_cmp(tagname, "EncryptionMethod"))
			insideEncryptionMethod = true;
		else if (!lStr_cmp(tagname, "CipherData"))
			insideCipherData = true;
		else if (!lStr_cmp(tagname, "CipherReference"))
			insideCipherReference = true;
		return NULL;
	}

	/// called on tag close
	virtual void OnTagClose(const lChar16 *nsname, const lChar16 *tagname)
	{
		CR_UNUSED(nsname);
		if (!lStr_cmp(tagname, "encryption"))
			insideEncryption = false;
		else if (!lStr_cmp(tagname, "EncryptedData") && insideEncryptedData)
		{
			if (!algorithm.empty() && !uri.empty())
			{
				_container->addEncryptedItem(new EncryptedItem(uri, algorithm));
			}
			insideEncryptedData = false;
		}
		else if (!lStr_cmp(tagname, "EncryptionMethod"))
			insideEncryptionMethod = false;
		else if (!lStr_cmp(tagname, "CipherData"))
			insideCipherData = false;
		else if (!lStr_cmp(tagname, "CipherReference"))
			insideCipherReference = false;
	}

	/// called on element attribute
	virtual void OnAttribute(const lChar16 *nsname, const lChar16 *attrname, const lChar16 *attrvalue)
	{
		CR_UNUSED2(nsname, attrvalue);
		if (!lStr_cmp(attrname, "URI") && insideCipherReference)
		{
			insideEncryption = false;
		}
		else if (!lStr_cmp(attrname, "Algorithm") && insideEncryptionMethod)
		{
			insideEncryptedData = false;
		}
	}

	/// called on text
	virtual void OnText(const lChar16 *text, int len, lUInt32 flags)
	{
		CR_UNUSED3(text, len, flags);
	}

	/// add named BLOB data to document
	virtual bool OnBlob(lString16 name, const lUInt8 *data, int size)
	{
		CR_UNUSED3(name, data, size);
		return false;
	}

	virtual void OnStop() {}

	/// called after > of opening tag (when entering tag body)
	virtual void OnTagBody() {}

	EncryptedItemCallback *_container;
	lString16 algorithm;
	lString16 uri;

	/// destructor
	EncCallback(EncryptedItemCallback *container) : _container(container)
	{
		insideEncryption = false;
		insideEncryptedData = false;
		insideEncryptionMethod = false;
		insideCipherData = false;
		insideCipherReference = false;
	}

	virtual ~EncCallback() {}
};

EncryptedDataContainer::EncryptedDataContainer(LVContainerRef baseContainer)
		: _container(baseContainer) {}

LVContainer *EncryptedDataContainer::GetParentContainer()
{
	return _container->GetParentContainer();
}

//virtual const LVContainerItemInfo * GetObjectInfo(const wchar_t * pname);
const LVContainerItemInfo *EncryptedDataContainer::GetObjectInfo(int index)
{
	return _container->GetObjectInfo(index);
}

int EncryptedDataContainer::GetObjectCount() const { return _container->GetObjectCount(); }

/// returns object size (file size or directory entry count)
lverror_t EncryptedDataContainer::GetSize(lvsize_t *pSize) { return _container->GetSize(pSize); }

LVStreamRef EncryptedDataContainer::OpenStream(const lChar16 *fname, lvopen_mode_t mode)
{
	LVStreamRef res = _container->OpenStream(fname, mode);
	if (res.isNull())
		return res;
	if (isEncryptedItem(fname))
		return LVStreamRef(new FontDemanglingStream(res, _fontManglingKey));
	return res;
}

LVStreamRef EncryptedDataContainer::OpenStreamByPackedSize(uint32_t size)
{
	return LVStreamRef();
}

/// returns stream/container name, may be NULL if unknown
const lChar16 *EncryptedDataContainer::GetName()
{
	return _container->GetName();
}

/// sets stream/container name, may be not implemented for some objects
void EncryptedDataContainer::SetName(const lChar16 *name)
{
	_container->SetName(name);
}

void EncryptedDataContainer::addEncryptedItem(EncryptedItem *item)
{
	_list.add(item);
}

EncryptedItem *EncryptedDataContainer::findEncryptedItem(const lChar16 *name)
{
	lString16 n;
	if (name[0] != '/' && name[0] != '\\')
		n << "/";
	n << name;
	for (int i = 0; i < _list.length(); i++)
	{
		lString16 s = _list[i]->_uri;
		if (s[0] != '/' && s[i] != '\\')
			s = "/" + s;
		if (_list[i]->_uri == s)
			return _list[i];
	}
	return NULL;
}

bool EncryptedDataContainer::isEncryptedItem(const lChar16 *name)
{
	return findEncryptedItem(name) != NULL;
}

bool EncryptedDataContainer::setManglingKey(lString16 key)
{
	if (key.startsWith("urn:uuid:"))
		key = key.substr(9);
	_fontManglingKey.clear();
	_fontManglingKey.reserve(16);
	lUInt8 b = 0;
	int n = 0;
	for (int i = 0; i < key.length(); i++)
	{
		int d = hexDigit(key[i]);
		if (d >= 0)
		{
			b = (b << 4) | d;
			if (++n > 1)
			{
				_fontManglingKey.add(b);
				n = 0;
				b = 0;
			}
		}
	}
	return _fontManglingKey.length() == 16;
}

bool EncryptedDataContainer::hasUnsupportedEncryption()
{
	for (int i = 0; i < _list.length(); i++)
	{
		lString16 method = _list[i]->_method;
		if (method != "http://ns.adobe.com/pdf/enc#RC")
		{
			LD("unsupported encryption method: %s", LCSTR(method));
			return true;
		}
	}
	return false;
}

bool EncryptedDataContainer::open()
{
	LVStreamRef stream = _container->OpenStream(L"META-INF/encryption.xml", LVOM_READ);
	if (stream.isNull())
		return false;
	EncCallback enccallback(this);
	LvXmlParser parser(stream, &enccallback, false, false);
	if (!parser.Parse())
		return false;
	if (_list.length())
		return true;
	return false;
}

void EncryptedDataContainer::createEncryptedEpubWarningDocument(CrDom *m_doc)
{
	LE("EPUB document contains encrypted items");
	LvDomWriter writer(m_doc);
	writer.OnTagOpenNoAttr(NULL, L"body");
	writer.OnTagOpenNoAttr(NULL, L"body");
	writer.OnTagOpenNoAttr(NULL, L"h3");
	lString16 hdr("Encrypted content");
	writer.OnText(hdr.c_str(), hdr.length(), 0);
	writer.OnTagClose(NULL, L"h3");

	writer.OnTagOpenAndClose(NULL, L"hr");

	writer.OnTagOpenNoAttr(NULL, L"p");
	lString16 txt("This document is encrypted (has DRM protection).");
	writer.OnText(txt.c_str(), txt.length(), 0);
	writer.OnTagClose(NULL, L"p");

	writer.OnTagOpenNoAttr(NULL, L"p");
	lString16 txt2("Reading of DRM protected books is not supported.");
	writer.OnText(txt2.c_str(), txt2.length(), 0);
	writer.OnTagClose(NULL, L"p");

	writer.OnTagOpenNoAttr(NULL, L"p");
	lString16 txt3("To read this book, please use software recommended by book seller.");
	writer.OnText(txt3.c_str(), txt3.length(), 0);
	writer.OnTagClose(NULL, L"p");

	writer.OnTagOpenAndClose(NULL, L"hr");

	writer.OnTagOpenNoAttr(NULL, L"p");
	lString16 txt4("");
	writer.OnText(txt4.c_str(), txt4.length(), 0);
	writer.OnTagClose(NULL, L"p");

	writer.OnTagClose(NULL, L"body");
}

class EmbeddedFontStyleParser
{
	LVEmbeddedFontList &_fontList;
	lString16 _basePath;
	int _state;
	lString8 _face;
	bool _italic;
	bool _bold;
	lString16 _url;
	lString8 islocal;
public:
	EmbeddedFontStyleParser(LVEmbeddedFontList &fontList) : _fontList(fontList) {}

	void onToken(char token)
	{
		// 4,5:  font-family:
		// 6,7:  font-weight:
		// 8,9:  font-style:
		//10,11: src:
		//   10   11    12   13
		//   src   :   url    (
		//LV("state==%d: %c ", _state, token);
		switch (token)
		{
			case ':':
				if (_state < 2)
				{
					_state = 0;
				}
				else if (_state == 4 || _state == 6 || _state == 8 || _state == 10)
				{
					_state++;
				}
				else if (_state != 3)
				{
					_state = 2;
				}
				break;
			case ';':
				if (_state < 2)
				{
					_state = 0;
				}
				else if (_state != 3)
				{
					_state = 2;
				}
				break;
			case '{':
				if (_state == 1)
				{
					_state = 2; // inside @font {
					_face.clear();
					_italic = false;
					_bold = false;
					_url.clear();
				}
				else
				{
					_state = 3;
				} // inside other {
				break;
			case '}':
				if (_state == 2)
				{
					if (!_url.empty())
					{
						//LV("@font { face: %s; bold: %s; italic: %s; url: %s",
						//    _face.c_str(), _bold ? "yes" : "no", _italic ? "yes" : "no", LCSTR(_url));
						if (islocal.length() == 5)
						{
							_url = (_url.substr((_basePath.length() + 1), (_url.length() - _basePath.length())));
						}
						_fontList.add(_url, _face, _bold, _italic);
					}
				}
				_state = 0;
				break;
			case ',':
				if (_state == 2)
				{
					if (!_url.empty())
					{
						if (islocal.length() == 5)
							_url = (_url.substr((_basePath.length() + 1), (_url.length() - _basePath.length())));
						_fontList.add(_url, _face, _bold, _italic);
					}
					_state = 11;
				}
				break;
			case '(':
				if (_state == 12)
				{
					_state = 13;
				}
				else
				{
					if (_state > 3)
						_state = 2;
				}
				break;
		}
	}

	void onToken(lString8 &token)
	{
		if (token.empty())
			return;
		lString8 t = token;
		token.clear();
		//LV("state==%d: %s", _state, t.c_str());
		if (t == "@font-face")
		{
			if (_state == 0)
				_state = 1; // right after @font
			return;
		}
		if (_state == 1)
			_state = 0;
		if (_state == 2)
		{
			if (t == "font-family")
				_state = 4;
			else if (t == "font-weight")
				_state = 6;
			else if (t == "font-style")
				_state = 8;
			else if (t == "src")
				_state = 10;
		}
		else if (_state == 5)
		{
			_face = t;
			_state = 2;
		}
		else if (_state == 7)
		{
			if (t == "bold")
				_bold = true;
			_state = 2;
		}
		else if (_state == 9)
		{
			if (t == "italic")
				_italic = true;
			_state = 2;
		}
		else if (_state == 11)
		{
			if (t == "url")
			{
				_state = 12;
				islocal = t;
			}
			else if (t == "local")
			{
				_state = 12;
				islocal = t;
			}
			else
				_state = 2;
		}
	}

	void onQuotedText(lString8 &token)
	{
		//LV("state==%d: \"%s\"", _state, token.c_str());
		if (_state == 11 || _state == 13)
		{
			if (!token.empty())
			{
				lString16 ltoken = Utf8ToUnicode(token);
				if (ltoken.startsWithNoCase(lString16("res://")) || ltoken.startsWithNoCase(lString16("file://")))
				{
					_url = ltoken;
				}
				else
				{
					_url = LVCombinePaths(_basePath, ltoken);
				}
			}
			_state = 2;
		}
		else if (_state == 5)
		{
			if (!token.empty())
			{
				_face = token;
			}
			_state = 2;
		}
		token.clear();
	}

	void parse(lString16 basePath, const lString8 &css)
	{
		_state = 0;
		_basePath = basePath;
		lString8 token;
		char insideQuotes = 0;
		for (int i = 0; i < css.length(); i++)
		{
			char ch = css[i];
			if (insideQuotes || _state == 13)
			{
				if (ch == insideQuotes || (_state == 13 && ch == ')'))
				{
					onQuotedText(token);
					insideQuotes = 0;
					if (_state == 13)
					{
						onToken(ch);
					}
				}
				else
				{
					if (_state == 13 && token.empty() && (ch == '\'' || ch == '\"'))
					{
						insideQuotes = ch;
					}
					else if (ch != ' ' || _state != 13)
					{
						token << ch;
					}
				}
				continue;
			}
			if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
			{
				onToken(token);
			}
			else if (ch == '@' || ch == '-' || ch == '_' || ch == '.' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
			{
				token << ch;
			}
			else if (ch == ':' || ch == '{' || ch == '}' || ch == '(' || ch == ')' || ch == ';' || ch == ',')
			{
				onToken(token);
				onToken(ch);
			}
			else if (ch == '\'' || ch == '\"')
			{
				onToken(token);
				insideQuotes = ch;
			}
		}
	}
};

void recurseNav(ldomNode *node, LvDocFragmentWriter &appender, CrDom *maindoc)
{
	for (int i = 0; i < node->getChildCount(); i++)
	{
		ldomNode *child = node->getChildNode(i);
		if (!child->isNodeName("a"))
		{
			recurseNav(child, appender, maindoc);
			continue;
		}
		lString16 text = child->getText();
		if (text.empty() || text == "-" || text.DigitsOnly())
		{
			continue;
		}
		if (text.length() > TOC_ITEM_LENGTH_MAX)
		{
			text = text.substr(0, TOC_ITEM_LENGTH_MAX);
			text = text + lString16("...");
		}

		lString16 path = appender.convertHref(child->getAttributeValue("href"));
		LvTocItem *item = new LvTocItem(ldomXPointer(), path, text);
		maindoc->getToc()->addItem(item, 1);
	}
}

void parseNav(CrDom *navdoc, CrDom *maindoc, LvDocFragmentWriter &appender)
{
	ldomNode *root = navdoc->getRootNode();
	if (root == NULL)
	{
		return;
	}
	recurseNav(root, appender, maindoc);
}

bool ImportEpubDocument(LVStreamRef stream, CrDom *m_doc, bool firstpage_thumb)
{
	LVContainerRef arc = LVOpenArchive(stream);
	if (arc.isNull())
    {
	    LE("ImportEpubDocument: Failed to open stream as archive");
        return false; // not a ZIP archive
    }
	// check root media type
	lString16 rootfilePath = EpubGetRootFilePath(arc);
	if (rootfilePath.empty())
    {
        LE("ImportEpubDocument failed to obtain rootfile path. Trying to use hardcoded one");
        //return false;
	    rootfilePath = lString16("OEBPS/content.opf");
    }

	EncryptedDataContainer *decryptor = new EncryptedDataContainer(arc);
	if (decryptor->open())
	{
		LD("ImportEpubDocument: encrypted items detected");
	}

	LVContainerRef m_arc = LVContainerRef(decryptor);

	if (decryptor->hasUnsupportedEncryption())
	{
		// DRM!!!
		EncryptedDataContainer::createEncryptedEpubWarningDocument(m_doc);
		return true;
	}

	m_doc->setDocParentContainer(m_arc);

	// read content.opf
	EpubItems epubItems;
	EpubItems NotesItems;
	LVArray<LinkStruct> LinksList;
	LinksMap LinksMap;
	Epub3Notes Epub3Notes;
	//EpubItem * epubToc = NULL; //TODO
	LVArray<EpubItem *> spineItems;
	lString16 codeBase;
	//lString16 css;
	{
		codeBase = LVExtractPath(rootfilePath, false);
		LV("codeBase=%s", LCSTR(codeBase));
	}

	LVStreamRef content_stream = m_arc->OpenStream(rootfilePath.c_str(), LVOM_READ);

	if (content_stream.isNull())
	{
		LE("ImportEpubDocument : failed to open rootfile %s",LCSTR(rootfilePath));
		return false;
	}

	lString16 ncxHref;
	lString16 navHref;
	lString16 coverId;
	bool CoverPageIsValid = false;

	LVEmbeddedFontList fontList;
	EmbeddedFontStyleParser styleParser(fontList);

	// reading content stream
    {
        CrDom *doc = LVParseXMLStream(content_stream);
        if (!doc)
        {
            LE("ImportEpubDocument: failed parsing rootfile %s",LCSTR(rootfilePath));
            return false;
        }
//        // for debug
//        {
//            LVStreamRef out = LVOpenFileStream("/tmp/content.xml", LVOM_WRITE);
//            doc->saveToStream(out, NULL, true);
//        }

		CRPropRef m_doc_props = m_doc->getProps();

		for (int i = 1; i < 50; i++)
		{
			ldomNode *item = doc->nodeFromXPath(lString16("package/metadata/identifier[") << fmt::decimal(i) << "]");
			if (!item)
			{
				break;
			}
			lString16 key = item->getText();
			if (decryptor->setManglingKey(key))
			{
				//LD("Using font mangling key %s", LCSTR(key));
				break;
			}
		}

		for (int i = 1; i < 20; i++)
		{
			ldomNode *item = doc->nodeFromXPath(lString16("package/metadata/meta[") << fmt::decimal(i) << "]");
			if (!item)
			{
				break;
			}
			lString16 name = item->getAttributeValue("name");
			lString16 content = item->getAttributeValue("content");
			if (name == "cover")
			{
				coverId = content;
			}
			else if (name == "calibre:series")
			{
				// Nothing to do
			}
			else if (name == "calibre:series_index")
			{
				// Nothing to do
			}
		}

		// items
		for (int i = 1; i < 50000; i++)
		{
			ldomNode *item = doc->nodeFromXPath(lString16("package/manifest/item[") << fmt::decimal(i) << "]");
			if (!item)
			{
				break;
			}
			lString16 href = item->getAttributeValue("href");
			lString16 mediaType = item->getAttributeValue("media-type");
			lString16 id = item->getAttributeValue("id");
			if (!href.empty() && !id.empty())
			{
				href = DecodeHTMLUrlString(href);

				lString16 props = item->getAttributeValue("properties");
				if(id == L"nav" && !props.empty() && props == L"nav" && mediaType == "application/xhtml+xml")
				{
					while (href.startsWith(".") || href.startsWith("/")){href = href.substr(1);}
					navHref = href;
					//continue;
				}
				if (id == coverId)
				{
					// coverpage file
					lString16 coverFileName = codeBase + href;
					LV("EPUB coverpage file: %s", LCSTR(coverFileName));
					LVStreamRef stream = m_arc->OpenStream(coverFileName.c_str(), LVOM_READ);
					if (!stream.isNull())
					{
						LVImageSourceRef img = LVCreateStreamImageSource(stream);
						if (!img.isNull())
						{
							LV("EPUB coverpage image is correct: %d x %d", img->GetWidth(), img->GetHeight());
							m_doc_props->setString(DOC_PROP_COVER_FILE, coverFileName);
							CoverPageIsValid = true;
						}
						else
						{
							LV("EPUB coverpage image incorrect! Generating own coverpage!");
							CoverPageIsValid = false;
						}
					}
					else
					{
						LV("EPUB coverpage image incorrect! Generating own coverpage!");
						CoverPageIsValid = false;
					}
				}
				EpubItem *epubItem = new EpubItem;
				epubItem->href = href;
				epubItem->id = id;
				epubItem->mediaType = mediaType;
				epubItems.add(epubItem);
				// register embedded document fonts
				if (mediaType == L"application/vnd.ms-opentype" ||
					mediaType == L"application/x-font-otf"      ||
					mediaType == L"application/x-font-ttf"      ||
					mediaType == L"font/ttf"                    ||
					mediaType == L"font/otf" )
				{
					// TODO: more media types?
				   fontList.add(codeBase + href);
				   //LE("fontlist add 1 [%s]",LCSTR(codeBase + href));
				}
			}
			if( gEmbeddedStylesLVL > 0)
            {
			    if (mediaType == "text/css")
                {
                    lString16 name = LVCombinePaths(codeBase, href);
                    LVStreamRef cssStream = m_arc->OpenStream(name.c_str(), LVOM_READ);
                    if (!cssStream.isNull())
                    {
                        lString8 cssFile = UnicodeToUtf8(LVReadCssText(cssStream));
                        lString16 base = name;
                        LVExtractLastPathElement(base);
                        //styleParser.parse(base, cssFile);
                        lString16 embedded_style;
                        m_doc->stylesManager.codeBase = codeBase;
                        m_doc->stylesManager.parseString(Utf8ToUnicode(cssFile));
                    }
                }
            }
		}

        if (epubItems.length() <= 0)
        {
            LE("ImportEpubDocument : failed reading package/manifest ");
            return false;
        }

		for (int i = 0; i < m_doc->stylesManager.embedded_font_classes.length(); ++i)
		{
			CssStyle c = m_doc->stylesManager.embedded_font_classes.get(i);
			fontList.add(c.getfontDef());
            //LE("fontlist add 2 [%s]",LCSTR(c.getfontDef()->getUrl()));
        }
        //we're counting xpaths from 1 NOT from 0
        int refcount = 1;
        while (1)
        {
            ldomNode *item = doc->nodeFromXPath(lString16("package/guide/reference[") << fmt::decimal(refcount) << "]");
            if (!item)
            {
                break;
            }
            lString16 type = item->getAttributeValue("type");
            lString16 href = item->getAttributeValue("href");
            lString16 id = item->getAttributeValue("id");
            lString16 title = item->getAttributeValue("title");
            if(type == "notes" || type == "glossary")
            {
                NotesItems.add(new EpubItem(href,type,id,title));
            }
            refcount++;
        }

		// spine == itemrefs

        ldomNode *spine = doc->nodeFromXPath(cs16("package/spine"));
        if (!spine)
        {
             LE("ImportEpubDocument : failed reading package/spine");
             return false;
        }
        {
            EpubItem *ncx = epubItems.findById(spine->getAttributeValue("toc")); //TODO
            //EpubItem * ncx = epubItems.findById(cs16("ncx"));
            if (ncx != NULL)
            {
                ncxHref = codeBase + ncx->href;
            }
            int nodes_to_parse = 50000;
            if (firstpage_thumb)
            {
                //LV("EPUB FIRSTPAGE SPINE COMPOSING");
                nodes_to_parse = 6; // irl 5: from 1 to 6
            }
            else
            {
                //LV("EPUB FULL SPINE COMPOSING");
                nodes_to_parse = 50000;
            }
            for (int i = 1; i < nodes_to_parse; i++)
            {
                ldomNode *item = doc->nodeFromXPath(lString16("package/spine/itemref[") << fmt::decimal(i) << "]");
                if (!item)
                {
                    break;
                }
                EpubItem *epubItem = epubItems.findById(item->getAttributeValue("idref"));
                if (epubItem && epubItem->mediaType == "application/xhtml+xml")
                {
                    bool allowed = true;
                    for (int i = 0; i < NotesItems.length(); i++)
                    {
                        if (epubItem->href == NotesItems.get(i)->href)
                            allowed = false;
                    }
                    if (allowed)
                    {
                        spineItems.add(epubItem);
                    }
                }
            }
        }
		delete doc;
	}
	if (spineItems.length() == 0)
	{
        LE("ImportEpubDocument : package/spine read successfully, but no valid items");
        return false;
	}

	lUInt32 saveFlags = m_doc->getDocFlags();
	m_doc->setDocFlags(saveFlags);
	m_doc->setDocParentContainer(m_arc);

	LvDomWriter writer(m_doc);
#if 0
	m_doc->setNodeTypes( fb2_elem_table );
	m_doc->setAttributeTypes( fb2_attr_table );
	m_doc->setNameSpaceTypes( fb2_ns_table );
#endif
	//m_doc->setCodeBase( codeBase );

	int fontList_nb_before_head_parsing = fontList.length();
	if (!fontList.empty()) {
		// set document font list, and register fonts
		m_doc->getEmbeddedFontList().set(fontList);
		m_doc->registerEmbeddedFonts();
	}

	LvDocFragmentWriter appender(&writer, cs16("body"), cs16("DocFragment"), lString16::empty_str);
    LvDocFragmentWriter appender2(&writer, cs16("body"), cs16("NoteFragment"), lString16::empty_str);
    LvDocFragmentWriter appender3(&writer, cs16("body"), cs16("NoteFragment"), lString16::empty_str);

    writer.OnStart(NULL);
	writer.OnTagOpenNoAttr(L"", L"body");
	int fragmentCount = 0;
	int itemcounter = 0;
	for (int i = 0; i < spineItems.length(); i++)
    {
        itemcounter++;
        lString16 name = codeBase + spineItems[i]->href;
        lString16 subst = cs16("_doc_fragment_") + fmt::decimal(i);
        appender.addPathSubstitution(name, subst);
        appender2.addPathSubstitution(name, subst);
        appender3.addPathSubstitution(name, subst);
        //LV("subst: %s => %s", LCSTR(name), LCSTR(subst));

	    //in case of "&amp;"-like symbols in href
	    lString16 saniName = codeBase + metaSanitize(spineItems[i]->href);
        if(saniName!=name)
        {
	        appender.addPathSubstitution(saniName, subst);
	        appender2.addPathSubstitution(saniName, subst);
	        appender3.addPathSubstitution(saniName, subst);
        }
    }
	for (int i = 0; i < NotesItems.length(); i++)
    {
        lString16 name = codeBase + NotesItems[i]->href;
        lString16 subst = cs16("_doc_fragment_") + fmt::decimal(itemcounter);
        appender.addPathSubstitution(name, subst);
        appender2.addPathSubstitution(name, subst);
        appender3.addPathSubstitution(name, subst);

	    //in case of "&amp;"-like symbols in href
	    lString16 saniName = codeBase + metaSanitize(NotesItems[i]->href);
	    if(saniName!=name)
	    {
		    appender.addPathSubstitution(saniName, subst);
		    appender2.addPathSubstitution(saniName, subst);
		    appender3.addPathSubstitution(saniName, subst);
	    }
        itemcounter++;
    }
	for (int i = 0; i < spineItems.length(); i++)
	{
		lString16 name = codeBase + spineItems[i]->href;
		//LV("        EPUB Checking fragment: %s", UnicodeToUtf8(name).c_str());
		LVStreamRef stream = m_arc->OpenStream(name.c_str(), LVOM_READ);
		if (stream.isNull())
		{
			LE("failed opening stream for [%s], retrying with sanitize",LCSTR(name));
			stream = m_arc->OpenStream(metaSanitize(name).c_str(), LVOM_READ);
		}

		if (stream.isNull())
		{
			LE("failed opening stream for [%s]",LCSTR(metaSanitize(name)));
			continue;
		}

		appender.setCodeBase(name);
		lString16 base = name;
		LVExtractLastPathElement(base);
		//LV("base: %s", UnicodeToUtf8(base).c_str());
		//LvXmlParser
		LvHtmlParser parser(stream, &appender, firstpage_thumb);
		parser.setEpubNotes(NotesItems);
		parser.setLinksList(LinksList);
		parser.setLinksMap(LinksMap);
		parser.setEpub3Notes(Epub3Notes);
		parser.setStylesManager(m_doc->stylesManager);
		if (parser.CheckFormat() && parser.Parse())
		{
			// valid
			fragmentCount++;
			//lString8 headCss = appender.getHeadStyleText();
			//styleParser.parse(base, headCss);
		}
		else
		{
			LE("Document type is not XML/XHTML for fragment %s", LCSTR(name));
		}
		LinksList = parser.getLinksList();
		LinksMap = parser.getLinksMap();
		Epub3Notes = parser.getEpub3Notes();
	}
	if(gEmbeddedStylesLVL > 0)
	{
		m_doc->stylesManager.Finalize();
	}

	if(LinksList.length()>0 && Epub3Notes.size() > 0)
	{
		//LE("FnotesTitle = %s",LCSTR(FnotesTitle));
		/*
		for (int i = 0; i < LinksList.length(); i++)
		{
		    LinkStruct link = LinksList.get(i);
			LE("List item #%d = %s , %s",i,LCSTR(link.id_),LCSTR(link.href_));
		}
		*/
        Epub3NotesPrinter printer(m_doc,Epub3Notes);
		printer.PrintLinksList(LinksList);
	}

	if(NotesItems.length()>0)
	{
		writer.OnTagOpen(L"", L"DocFragment");
		//this excessive notefragment is inserted for correct page separation
		writer.OnTagOpen(L"", L"NoteFragment");
		writer.OnText(L"\u200B",1,TXTFLG_KEEP_SPACES | TXTFLG_TRIM_ALLOW_END_SPACE | TXTFLG_TRIM_ALLOW_START_SPACE);
		writer.OnTagClose(L"", L"NoteFragment");

		//special footnotes parsing
		writer.setFlags(TXTFLG_IN_NOTES);
		for (int i = 0; i < NotesItems.length(); i++)
		{
			lString16 name = codeBase + NotesItems[i]->href;
			LVStreamRef stream = m_arc->OpenStream(name.c_str(), LVOM_READ);
			if (!stream.isNull())
			{
				appender3.setCodeBase(name);
				lString16 base = name;
				LVExtractLastPathElement(base);
				//LV("base: %s", UnicodeToUtf8(base).c_str());
				//LvXmlParser
				LvHtmlParser parser(stream, &appender3, firstpage_thumb);
				//parser.setLinksList(LinksList);
				parser.setLinksMap(LinksMap);
                parser.setStylesManager(m_doc->stylesManager);
                if (parser.CheckFormat() && parser.ParseEpubFootnotes())
				{
					// valid
					//fragmentCount++;
					//lString8 headCss = appender3.getHeadStyleText();
					//styleParser.parse(base, headCss);
				}
				else
				{
					LE("Document type is not XML/XHTML for fragment %s", LCSTR(name));
				}
			}
		}

		writer.OnTagClose(L"", L"DocFragment");
	}

    //LE("Linkslist length = %d",LinksList.length());
    //for (int i = 0; i < LinksList.length(); i++)
    //{
    //	LE("LinksList %d = #%d = %s = %s",i,LinksList.get(i).num_,LCSTR(LinksList.get(i).id_),LCSTR(LinksList.get(i).href_));
    //}
    if(LinksList.length()>0)
    {
	  //  writer.OnTagOpen(L"", L"NoteFragment");
	  //  writer.OnText(L"\u200B", 1, TXTFLG_KEEP_SPACES | TXTFLG_TRIM_ALLOW_END_SPACE | TXTFLG_TRIM_ALLOW_START_SPACE);
	  //  writer.OnTagClose(L"", L"NoteFragment");
        //LE("printing footnotes");
        FootnotesPrinter printer(m_doc);
	    printer.PrintLinksList(LinksList);
    }
    //writer.OnTagClose(L"", L"DocFragment");
    writer.OnTagClose(L"", L"body");
    writer.OnStop();

	if (!ncxHref.empty())
	{
		LVStreamRef stream = m_arc->OpenStream(ncxHref.c_str(), LVOM_READ);
		lString16 codeBase = LVExtractPath(ncxHref);
		if (codeBase.length() > 0 && codeBase.lastChar() != '/')
		{
			codeBase.append(1, L'/');
		}
		appender.setCodeBase(codeBase);
		if (!stream.isNull())
		{
			CrDom *ncxdoc = LVParseXMLStream(stream);
			if (ncxdoc != NULL)
			{
				ldomNode *navMap = ncxdoc->nodeFromXPath(cs16("ncx/navMap"));
				if (navMap != NULL)
					ReadEpubToc(m_doc, navMap, m_doc->getToc(), appender);
				delete ncxdoc;
			}
		}
	}
	LD("EPUB: %d documents merged", fragmentCount);

	//fallback for toc
	if (m_doc->getToc()->getChildCount() <= 1)
	{
		m_doc->getToc()->clear(); //clear if original toc.ncx is malformed
        GetTOC(m_doc, m_doc->getToc(), false);
        if (m_doc->getToc()->getChildCount() == 0)
        {
        	//deeper fallback for toc
            LI("Deeper TOC search");
            GetTOC(m_doc, m_doc->getToc(), true);
        }

		//fallback for fallback for toc
        if (m_doc->getToc()->getChildCount() == 0)
		{
			LI("No toc generated, falling back to navdoc");
			LI("Looking at [%s]",LCSTR(navHref));
			if (!navHref.empty())
			{
				LVStreamRef nav_stream = m_arc->OpenStream(navHref.c_str(), LVOM_READ);
				lString16 codeBase = LVExtractPath(ncxHref);
				if (codeBase.length() > 0 && codeBase.lastChar() != '/')
				{
					codeBase.append(1, L'/');
				}
				appender.setCodeBase(codeBase);
				if (!nav_stream.isNull())
				{
					CrDom *navdoc = LVParseXMLStream(nav_stream);

					if (navdoc != NULL)
					{
						parseNav(navdoc, m_doc, appender);
						delete navdoc;
					}
				}
			}
		}
	}
	else
	{
		LV("TOC already exists. No TOC generation for now.");
	}

	if ( fontList.length() != fontList_nb_before_head_parsing ) {
		// New fonts met when parsing <head><style> of some DocFragments
		// set document font list, and register fonts
		m_doc->getEmbeddedFontList().set(fontList);
		m_doc->registerEmbeddedFonts();
		LI("ImportEpubDocument: document styles re-init (cause: embedded fonts)");
		m_doc->forceReinitStyles();
	}
	if (fragmentCount == 0)
	{
        if(spineItems.length() > 0)
        {
            EncryptedDataContainer::createEncryptedEpubWarningDocument(m_doc);
            return true;
        }
		return false;
	}
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

void GetEpubMetadata(CrDom *dom, lString16 *res_title, lString16 *res_authors, lString16 *res_lang,
		lString16 *res_series, int *res_series_number, lString16 *res_genre, lString16 *res_annotation, int *fontcount)
{
    lString16 title;
    lString16 authors;
    lString16 lang;
    lString16 series;
    int series_number = 0;
    for (int i = 1; i < 1000; i++)
    {
        lString16 xpath = lString16("package/metadata/creator[") << fmt::decimal(i) << "]";
        ldomNode *item = dom->nodeFromXPath(xpath);
        if (!item)
        {
            break;
        }
        authors += item->getText().trim() + lString16("|");
    }
    if(authors.endsWith("|"))
    {
        authors= authors.substr(0,authors.length()-1);
    }

    title = dom->textFromXPath(lString16("package/metadata/title")).trim();
    lang = dom->textFromXPath(lString16("package/metadata/language")).trim();
    for (int i = 1; i < 1000; i++)
    {
        lString16 xpath = lString16("package/metadata/meta[") << fmt::decimal(i) << "]";
        ldomNode *item = dom->nodeFromXPath(xpath);
        if (!item)
        {
            break;
        }
        lString16 name = item->getAttributeValue("name");
        lString16 content = item->getAttributeValue("content");
        if (name == "calibre:series")
        {
            series = content.trim();
        }
        else if (name == "calibre:series_index")
        {
            series_number = content.trim().atoi();
        }
    }

    //epub3 series metadata
    if(series.empty())
    {
        struct titleitem
        {
            lString16 title;
            lString16 id;
        };

        LVArray<titleitem> titles;
        for (int i = 1; i < 1000; i++)
        {
            lString16 xpath = lString16("package/metadata/title[") << fmt::decimal(i) << "]";
            ldomNode *item = dom->nodeFromXPath(xpath);
            if (!item)
            {
                break;
            }
            titleitem curr;
            curr.id = item->getAttributeValue("id");
            curr.title = item->getText().trim();
            titles.add(curr);
        }
        lString16 series_id;
        for (int i = 1; i < 1000; i++)
        {
            lString16 xpath = lString16("package/metadata/meta[") << fmt::decimal(i) << "]";
            ldomNode *item = dom->nodeFromXPath(xpath);
            if (!item)
            {
                break;
            }
            lString16 property = item->getAttributeValue("property");
            lString16 content = item->getText().trim();
            if (property == "collection-type" && content == "series")
            {
                series_id = item->getAttributeValue("refines");
                if (series_id.startsWith("#"))
                {
                    series_id = series_id.substr(1, series_id.length() - 1);
                }
                break;
            }
        }
        if (!series_id.empty())
        {
            for (int i = 0; i < titles.length(); i++)
            {
                titleitem curr = titles.get(i);
                if (curr.id == series_id)
                {
                    series = curr.title;
                    break;
                }
            }
            for (int i = 1; i < 1000; i++)
            {
                lString16 xpath = lString16("package/metadata/meta[") << fmt::decimal(i) << "]";
                ldomNode *item = dom->nodeFromXPath(xpath);
                if (!item)
                {
                    break;
                }
                lString16 refines = item->getAttributeValue("refines");
                lString16 property = item->getAttributeValue("property");
                lString16 content = item->getText().trim();
                if (refines == series_id && property == "group-positon")
                {
                    series_number = content.atoi();
                    break;
                }
            }
        }
    }

    //if series still empty: last attempt, for property="belongs-to-collection"
    if(series.empty())
    {
        lString16 series_id;
        for (int i = 1; i < 1000; i++)
        {
            lString16 xpath = lString16("package/metadata/meta[") << fmt::decimal(i) << "]";
            ldomNode *item = dom->nodeFromXPath(xpath);
            if (!item)
            {
                break;
            }
            lString16 property = item->getAttributeValue("property");
            if (property == "belongs-to-collection")
            {
                series = item->getText().trim();
                series_id = item->getAttributeValue("id");
                break;
            }
        }
        if(!series_id.empty() && series_number == 0)
        {
            for (int i = 1; i < 1000; i++)
            {
                lString16 xpath = lString16("package/metadata/meta[") << fmt::decimal(i) << "]";
                ldomNode *item = dom->nodeFromXPath(xpath);
	            if (!item)
	            {
		            break;
	            }
                lString16 property = item->getAttributeValue("property");
                lString16 id = item->getAttributeValue("refines");
                if (id.startsWith("#"))
                {
                    id = id.substr(1, id.length() - 1);
                }
                if (id == series_id && property == "group-position")
                {
                    series_number = item->getText().trim().atoi();
                    break;
                }
            }
        }
    }

    lString16 genre;

	for (int i = 1; i < 1000; i++)
	{
		lString16 xpath = lString16("package/metadata/subject[") << fmt::decimal(i) << "]";
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

	int fontcount_inner = 0;
	for (int i = 1; i < 1000; i++)
	{
		lString16 xpath = lString16("package/manifest/item[") << fmt::decimal(i) << "]";
		ldomNode *item = dom->nodeFromXPath(xpath);
		if (!item)
		{
			break;
		}
		lString16 mediaType = item->getAttributeValue("media-type");
		if (mediaType == L"application/vnd.ms-opentype" ||
		    mediaType == L"application/x-font-otf"      ||
		    mediaType == L"application/x-font-ttf"      ||
		    mediaType == L"font/ttf"                    ||
		    mediaType == L"font/otf")
		{
			fontcount_inner++;
		}
	}

	*res_annotation = dom->textFromXPath(lString16("package/metadata/description")).trim();

	*fontcount = fontcount_inner;
	*res_genre = genre;
    *res_title = title;
    *res_authors = authors;
    *res_lang = lang;
    *res_series = series;
    *res_series_number = series_number ;
    return;
}


bool checkEpubJapaneseVertical(CrDom *dom, LVContainerRef container, lString16 code_base)
{
	for (int i = 1; i < 1000; i++)
	{
		lString16 xpath = lString16("package/manifest/item[") << fmt::decimal(i) << "]";
		ldomNode *item = dom->nodeFromXPath(xpath);
		if (!item)
		{
			break;
		}
		lString16 mediaType = item->getAttributeValue("media-type");
		lString16 href = item->getAttributeValue("href");

		if (mediaType == L"text/css" )
		{
			//LE("found css [%s]",LCSTR(href));

			LVStreamRef css_stream = container->OpenStream((code_base + href).c_str(),LVOM_READ);
			if (css_stream.isNull()) {
				LE("checkEpubJapaneseVertical: malformed stream for [%s]",LCSTR(code_base+href));
				continue;
			}

			const lvsize_t size = css_stream->GetSize();
			char* buf = (char*)malloc(size);
			lvsize_t read = 0;
			int err = css_stream->Read(buf,size,&read);
			if(err != LVERR_OK)
			{
				LE("checkEpubJapaneseVertical: failed reading stream for [%s]",LCSTR(code_base+href));
				continue;
			}
			if(read != size)
			{
				LE("checkEpubJapaneseVertical: failed reading stream: read [%d] bytes of asked [%d]",read,size);
			}

			std::string term("vertical-rl");
			std::string bufstr(buf, read);
			std::size_t n = bufstr.find(term);
			if (n != std::string::npos)
			{
				//LE("FOUND JAPANESE MODE!");
				return true;
			}
		}
	}
	return false;
}


LVStreamRef getThumbByAttrValueStrict(CrDom *dom, LVContainerRef container, lString16 code_base, lString16 attr, lString16 val)
{
    LVStreamRef stream;
    for (int i = 1; i < 5000; i++)
    {
        lString16 xpath = lString16("package/manifest/item[") << fmt::decimal(i) << "]";
        ldomNode *item = dom->nodeFromXPath(xpath);
        if (!item)
        {
            break;
        }
        lString16 value = item->getAttributeValue(LCSTR(attr));
        lString16 href = item->getAttributeValue("href");
        lString16 mediaType = item->getAttributeValue("media-type");
        if (value.empty()){
            continue;
        }
        if ((mediaType == lString16("image/jpeg")
            || mediaType == lString16("image/jpg")
            || mediaType == lString16("image/webp")
            || mediaType == lString16("image/png")) && value == val)
        {
            lString16 thumbnail_file_name = code_base + href;
            thumbnail_file_name = DecodeHTMLUrlString(thumbnail_file_name);
            stream = container->OpenStream(thumbnail_file_name.c_str(), LVOM_READ);
            break;
        }
    }
	return stream;
}

LVStreamRef getThumbByAttrValue(CrDom *dom, LVContainerRef container, lString16 code_base, lString16 attribute, lString16 prefix)
{
    lString16 strjpg  = prefix + ".jpg";
    lString16 strjpeg = prefix + ".jpeg";
    lString16 strpng  = prefix + ".png";
    LVStreamRef stream;
    for (int i = 1; i < 5000; i++)
    {
        lString16 xpath = lString16("package/manifest/item[") << fmt::decimal(i) << "]";
        ldomNode *item = dom->nodeFromXPath(xpath);
        if (!item)
        {
            break;
        }
        lString16 value = item->getAttributeValue(LCSTR(attribute));
        lString16 href = item->getAttributeValue("href");
        lString16 mediaType = item->getAttributeValue("media-type");

        if (value.empty()) {
            continue;
        }
        if (mediaType == lString16("image/jpg")  ||
            mediaType == lString16("image/jpeg") ||
            mediaType == lString16("image/webp") ||
            mediaType == lString16("image/png"))
        {
            if (value.endsWith(strjpg)  ||
                value.endsWith(strjpeg) ||
                value.endsWith(strpng))
            {
                lString16 thumbnail_file_name = code_base + href;
                thumbnail_file_name = DecodeHTMLUrlString(thumbnail_file_name);
                stream = container->OpenStream(thumbnail_file_name.c_str(), LVOM_READ);
                break;
            }
        }
    }
	return stream;
}

LVStreamRef FindEpubThumbStream(CrDom *dom, LVContainerRef container, lString16 code_base)
{
    LV("Epub coverpage fallback search attempt...");
    LVStreamRef stream;
    stream = getThumbByAttrValueStrict(dom,container,code_base,lString16("id"),lString16("cover"));
    if(!stream.isNull()){LV("Epub coverpage found by id == cover");return stream;}
    stream = getThumbByAttrValueStrict(dom,container,code_base,lString16("properties"),lString16("cover-image"));
    if(!stream.isNull()){LV("Epub coverpage found by properties == cover-image");return stream;}
    stream = getThumbByAttrValue(dom,container,code_base,lString16("id"),lString16("/cover"));
    if(!stream.isNull()){LV("Epub coverpage found by id containing \"/cover\"");return stream;}
    stream = getThumbByAttrValue(dom,container,code_base,lString16("id"),lString16("cover"));
    if(!stream.isNull()){LV("Epub coverpage found by id containing \"cover\"");return stream;}
    stream = getThumbByAttrValue(dom,container,code_base,lString16("id"),lString16("_cover"));
    if(!stream.isNull()){LV("Epub coverpage found by id containing \"_cover\"");return stream;}
    stream = getThumbByAttrValue(dom,container,code_base,lString16("href"),lString16("cover"));
    if(!stream.isNull()){LV("Epub coverpage found by href containing \"cover\"");return stream;}
    LE("Epub coverpage fallback search attempt FAILED");
    return stream;
}

LVStreamRef GetEpubCoverImage(CrDom *dom,LVContainerRef container,lString16 code_base)
{
    LVStreamRef thumb_stream;
    lString16 thumb_id;
    for (int i = 1; i < 5000; i++)
    {
        lString16 xpath = lString16("package/metadata/meta[") << fmt::decimal(i) << "]";
        ldomNode *item = dom->nodeFromXPath(xpath);
        if (!item)
        {
            break;
        }
        lString16 name = item->getAttributeValue("name");
        lString16 content = item->getAttributeValue("content");
        if (name == "cover")
        {
            thumb_id = content;
            break;
        }
    }
    if(!thumb_id.empty())
    {
        thumb_stream = getThumbByAttrValueStrict(dom,container,code_base,lString16("id"),thumb_id);
    }
    if(thumb_stream.isNull())
    {
        thumb_stream = FindEpubThumbStream(dom,container,code_base);
    }
    return thumb_stream;
}

