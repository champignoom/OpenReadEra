/** \file lvxml.h
    \brief XML parser

   CoolReader Engine

   (c) Vadim Lopatin, 2000-2006
   Copyright (C) 2013-2020 READERA LLC

   This source code is distributed under the terms of
   GNU General Public License.

   See LICENSE file for details.

*/

#ifndef __LVXML_H_INCLUDED__
#define __LVXML_H_INCLUDED__

#include <time.h>
#include <map>
#include "lvstring.h"
#include "lvstream.h"
#include "crtxtenc.h"
#include "dtddef.h"
#include "docxhandler.h"
#include "EpubItems.h"
#include "odthandler.h"

#define XML_CHAR_BUFFER_SIZE 4096
#define XML_FLAG_NO_SPACE_TEXT 1

typedef std::map<lUInt32,int> Tagmap;
typedef std::map<lUInt32,int>::iterator iter;
typedef std::map<lUInt32,lString16> LinksMap;
typedef std::map<lUInt32,lString16>::iterator LinksIter;
typedef std::map<lUInt32,int> Headermap;

//class LvXmlParser;
class LVFileFormatParser;

class tinyNode;
class ldomNode;

/// XML parser callback interface
class LvXMLParserCallback {
protected:
    LVFileFormatParser * _parser;
public:
    virtual bool fb2_cover_done() {return false;}
    virtual bool reading_binary() {return false;}
    virtual bool getRTLflag() { return 0;}
    virtual void setRTLflag(bool RTLflag) { }
    /// returns flags
    virtual lUInt32 getFlags() { return 0; }
    /// sets flags
    virtual void setFlags(lUInt32) { }
    /// called on document encoding definition
    virtual void OnEncoding(const lChar16*, const lChar16*) { }
    /// called on parsing start
    virtual void OnStart(LVFileFormatParser* parser) { _parser = parser; }
    /// called on parsing end
    virtual void OnStop() = 0;
    /// called on opening tag <
    virtual ldomNode* OnTagOpen(const lChar16* nsname, const lChar16* tagname) = 0;
    /// called after > of opening tag (when entering tag body)
    virtual void OnTagBody() = 0;
    /// calls OnTagOpen & OnTagBody
    virtual void OnTagOpenNoAttr(const lChar16* nsname, const lChar16* tagname)
    {
        OnTagOpen(nsname, tagname);
        OnTagBody();
    }
    /// calls OnTagOpen & OnTagClose
    virtual void OnTagOpenAndClose(const lChar16* nsname, const lChar16* tagname)
    {
        OnTagOpen(nsname, tagname);
        OnTagBody();
        OnTagClose(nsname, tagname);
    }
    /// called on tag close
    virtual void OnTagClose(const lChar16* nsname, const lChar16* tagname) = 0;
    /// called on element attribute
    virtual void OnAttribute(const lChar16* nsname, const lChar16* attrname, const lChar16* attrvalue) = 0;
    /// called on text
    virtual void OnText(const lChar16 * text, int len, lUInt32 flags) = 0;
    /// add named BLOB data to document
    virtual bool OnBlob(lString16 name, const lUInt8 * data, int size) = 0;
    /// call to set document property
    virtual void OnDocProperty(const char* name, lString8 value) { }
    virtual lString16 convertId( lString16 id )   { return lString16::empty_str;}
    virtual lString16 convertHref( lString16 id ) { return lString16::empty_str;}
    virtual ~LvXMLParserCallback() {}
};

/// don't treat CR/LF and TAB characters as space nor remove duplicate spaces
#define TXTFLG_PRE        1
/// last character of previous text was space
#define TXTFLG_LASTSPACE  2

#define TXTFLG_TRIM                         4
#define TXTFLG_TRIM_ALLOW_START_SPACE       8
#define TXTFLG_TRIM_ALLOW_END_SPACE         16
#define TXTFLG_TRIM_REMOVE_EOL_HYPHENS      32
#define TXTFLG_RTF                          64
#define TXTFLG_PRE_PARA_SPLITTING           128
#define TXTFLG_KEEP_SPACES                  256
#define TXTFLG_IN_NOTES                     512
#define TXTFLG_ENCODING_MASK                0xFF00
#define TXTFLG_ENCODING_SHIFT               8
#define TXTFLG_CONVERT_8BIT_ENTITY_ENCODING 0x10000

/// converts XML text: decode character entities, convert space chars
void PreProcessXmlString( lString16 & s, lUInt32 flags, const lChar16 * enc_table=NULL );
/// converts XML text in-place: decode character entities, convert space chars, returns new length of string
int PreProcessXmlString(lChar16 * str, int len, lUInt32 flags, const lChar16 * enc_table = NULL);

#define MAX_PERSISTENT_BUF_SIZE 16384

/// base class for all document format parsers
class LVFileFormatParser
{
public:
    /// returns true if format is recognized by parser
    virtual bool CheckFormat() = 0;
    /// parses input stream
    virtual bool Parse() = 0;
    /// parses input stream
    virtual bool ParseDocx(DocxItems docxItems,DocxLinks docxLinks,DocxStyles docxStyles) = 0;
    /// resets parsing, moves to beginning of stream
    virtual void Reset() = 0;
    /// stops parsing in the middle of file, to read header only
    virtual void Stop() = 0;
    /// sets charset by name
    virtual void SetCharset( const lChar16 * name ) = 0;
    /// sets 8-bit charset conversion table (128 items, for codes 128..255)
    virtual void SetCharsetTable( const lChar16 * table ) = 0;
    /// returns 8-bit charset conversion table (128 items, for codes 128..255)
    virtual lChar16 * GetCharsetTable() = 0;
    /// changes space mode
    virtual void SetSpaceMode(bool) { }
    /// returns space mode
    virtual bool GetSpaceMode() { return false; }
    /// virtual destructor
    virtual ~LVFileFormatParser();
    virtual void FullDom() = 0;
};

class LVFileParserBase : public LVFileFormatParser
{
protected:
    LVStreamRef m_stream;
    lUInt8* m_buf;
    int      m_buf_size;
    lvsize_t m_stream_size;
    int      m_buf_len;
    int      m_buf_pos;
    lvpos_t  m_buf_fpos;
    bool     m_stopped; // true if Stop() is called
    /// fills buffer, to provide specified number of bytes for read
    bool FillBuffer(int bytesToRead);
    /// seek to specified stream position
    bool Seek( lvpos_t pos, int bytesToPrefetch=0 );
    /// override to return file reading position percent
    virtual int getProgressPercent();
public:
    /// constructor
    LVFileParserBase(LVStreamRef stream);
    /// virtual destructor
    virtual ~LVFileParserBase();
    /// returns source stream
    LVStreamRef getStream() { return m_stream; }
    /// return stream file name
    lString16 getFileName();
    /// returns true if end of fle is reached, and there is no data left in buffer
    virtual bool Eof() { return m_buf_fpos + m_buf_pos >= m_stream_size; }
    /// resets parsing, moves to beginning of stream
    virtual void Reset();
    /// stops parsing in the middle of file, to read header only
    virtual void Stop();
};

class LVTextFileBase : public LVFileParserBase
{
protected:
    char_encoding_type m_enc_type;
    lString16 m_txt_buf;
    lString16 m_encoding_name;
    lString16 m_lang_name;
    lChar16 * m_conv_table; // charset conversion table for 8-bit encodings

    lChar16 m_read_buffer[XML_CHAR_BUFFER_SIZE];
    int m_read_buffer_len;
    int m_read_buffer_pos;
    bool eof_;

    void checkEof();

    inline lChar16 ReadCharFromBuffer()
    {
        if (m_read_buffer_pos >= m_read_buffer_len) {
            if (!fillCharBuffer()) {
                eof_ = true;
                return 0;
            }
        }
        return m_read_buffer[m_read_buffer_pos++];
    }

    inline lChar16 PeekCharFromBuffer()
    {
        if ( m_read_buffer_pos >= m_read_buffer_len ) {
            if ( !fillCharBuffer() ) {
                eof_ = true;
                return 0;
            }
        }
        return m_read_buffer[m_read_buffer_pos];
    }

    inline lChar16 PeekCharFromBuffer( int offset )
    {
        if ( m_read_buffer_pos + offset >= m_read_buffer_len ) {
            if ( !fillCharBuffer() ) {
                eof_ = true;
                return 0;
            }
            if ( m_read_buffer_pos + offset >= m_read_buffer_len )
                return 0;
        }
        return m_read_buffer[m_read_buffer_pos + offset];
    }

    // skip current char (was already peeked), peek next
    inline lChar16 PeekNextCharFromBuffer()
    {
        if (m_read_buffer_pos + 1 >= m_read_buffer_len) {
            if (!fillCharBuffer()) {
                eof_ = true;
                return 0;
            }
        }
        return m_read_buffer[++m_read_buffer_pos];
    }

    // skip current char (was already peeked), peek next
    inline lChar16 PeekNextCharFromBuffer(int offset)
    {
        if (m_read_buffer_pos+offset >= m_read_buffer_len) {
            if (!fillCharBuffer()) {
                eof_ = true;
                return 0;
            }
            if (m_read_buffer_pos + offset >= m_read_buffer_len)
                return 0;
        }
        m_read_buffer_pos += offset + 1;
        return m_read_buffer[m_read_buffer_pos];
    }

    void clearCharBuffer();
    /// returns number of available characters in buffer
    int fillCharBuffer();

    /// reads one character from buffer
    //lChar16 ReadChar();
    /// reads several characters from buffer
    int ReadChars( lChar16 * buf, int maxsize );
    /// reads one character from buffer in RTF format
    lChar16 ReadRtfChar( int enc_type, const lChar16 * conv_table );
    /// reads specified number of bytes, converts to characters and saves to buffer, returns number of chars read
    int ReadTextBytes( lvpos_t pos, int bytesToRead, lChar16 * buf, int buf_size, int flags );
#if 0
    /// reads specified number of characters and saves to buffer, returns number of chars read
    int ReadTextChars( lvpos_t pos, int charsToRead, lChar16 * buf, int buf_size, int flags );
#endif
public:
    /// returns true if end of fle is reached, and there is no data left in buffer
    virtual bool Eof() { return eof_; /* m_buf_fpos + m_buf_pos >= m_stream_size;*/ }
    virtual void Reset();
    /// tries to autodetect text encoding
    bool AutodetectEncoding( bool utfOnly=false );
    /// reads next text line, tells file position and size of line, sets EOL flag
    lString16 ReadLine( int maxLineSize, lUInt32 & flags );
    //lString16 ReadLine( int maxLineSize, lvpos_t & fpos, lvsize_t & fsize, lUInt32 & flags );
    /// returns name of character encoding
    lString16 GetEncodingName() { return m_encoding_name; }
    /// returns name of language
    lString16 GetLangName() { return m_lang_name; }

    // overrides
    /// sets charset by name
    virtual void SetCharset( const lChar16 * name );
    /// sets 8-bit charset conversion table (128 items, for codes 128..255)
    virtual void SetCharsetTable( const lChar16 * table );
    /// returns 8-bit charset conversion table (128 items, for codes 128..255)
    virtual lChar16 * GetCharsetTable( ) { return m_conv_table; }

    /// constructor
    LVTextFileBase( LVStreamRef stream );
    /// destructor
    virtual ~LVTextFileBase();
};

class LVTextParser : public LVTextFileBase {
protected:
    LvXMLParserCallback* m_callback;
    bool smart_format_;
    bool firstpage_thumb_;
public:
	/// constructor
    LVTextParser(LVStreamRef stream, LvXMLParserCallback* callback, bool smart_format,
                 bool firstpage_thumb);
    /// descructor
    virtual ~LVTextParser();
    /// returns true if format is recognized by parser
    virtual bool CheckFormat();
    /// parses input stream
    virtual bool Parse();

    virtual bool ParseDocx(DocxItems docxItems,DocxLinks docxLinks, DocxStyles docxStyles) { return false; };

    virtual void FullDom();
};

/// XML parser
class LvXmlParser : public LVTextFileBase
{
private:
    LvXMLParserCallback* callback_;
    bool m_trimspaces;
    int  m_state;
    bool SkipSpaces();
    bool SkipTillChar( lChar16 ch );
    bool ReadIdent( lString16 & ns, lString16 & str );
    bool ReadText();
    bool need_coverpage_;
    Tagmap m_;
    bool tags_init_ = false;
    EpubItems * EpubNotes_;
    LVArray<LinkStruct> LinksList_;
    LinksMap LinksMap_;
    Epub3Notes Epub3Notes_;
    bool Notes_exists = false;
    EpubStylesManager EpubStylesManager_;
    std::map<lUInt32,lString16> fb3RelsMap_;
protected:
    bool possible_capitalized_tags_;
    bool m_allowHtml;
    bool m_fb2Only;
public:
    bool fb2_meta_only = false;
    /// Returns true if format is recognized by parser
    virtual bool CheckFormat();
    //parse
    virtual bool Parse();
    //highly modified xml parser for docx parsing
    virtual bool ParseDocx(DocxItems docxItems, DocxLinks docxLinks,DocxStyles docxStyles);
    //highly modified xml parser for odt parsing
    virtual bool ParseOdt(OdtStyles styles);
    //highly modified xml parser for epub footnotes parsing
    virtual bool ParseEpubFootnotes();
    //add epub notes list for parser
    void setEpubNotes(EpubItems epubItems);

    void setLinksList(LVArray<LinkStruct> LinksList);

    LVArray<LinkStruct> getLinksList();

    /// sets charset by name
    virtual void SetCharset(const lChar16* name);
    /// resets parsing, moves to beginning of stream
    virtual void Reset();
    /// constructor
    LvXmlParser( LVStreamRef stream, LvXMLParserCallback * callback, bool allowHtml=true, bool fb2Only=false, bool coverpage_needed= false );
    /// changes space mode
    virtual void SetSpaceMode( bool flgTrimSpaces );
    /// returns space mode
    bool GetSpaceMode() { return m_trimspaces; }
    /// destructor
    virtual ~LvXmlParser();

    void FullDom();

    //docx tag filterting
    bool docxTagAllowed(lString16 tagname);
    //docx tags to filter initialization
    void initDocxTagsFilter();
    //odt tags to filter
    void initOdtTagsFilter();

    bool ReadTextToString(lString16 &output, bool write_to_tree, bool rtl_force_check = false);

    void setLinksMap(LinksMap LinksMap);

    LinksMap getLinksMap();

    void setEpub3Notes(Epub3Notes Epub3Notes);

    Epub3Notes getEpub3Notes();

    void setStylesManager(EpubStylesManager manager);

    EpubStylesManager getStylesManager();

    bool odtTagAllowed(lString16 tagname);

    std::map<lUInt32,lString16> getFb3Relationships();

    void setFb3Relationships(std::map<lUInt32,lString16> map);
};

extern const char * * HTML_AUTOCLOSE_TABLE[];

class LvHtmlParser : public LvXmlParser
{
public:
    /// Returns true if format is recognized by parser
    virtual bool CheckFormat();
    virtual bool Parse();
    virtual bool ParseDocx(DocxItems docxItems, DocxLinks docxLinks,DocxStyles docxStyles);
    //virtual bool ParseDocx(DocxItems docxItems);
    virtual bool ParseEpubFootnotes();
    LvHtmlParser(LVStreamRef stream, LvXMLParserCallback * callback);
    LvHtmlParser(LVStreamRef stream, LvXMLParserCallback * callback, bool need_coverpage);
    bool need_coverpage_;
    virtual ~LvHtmlParser();

    bool ParseOdt(OdtStyles odtStyles);
};

/// read stream contents to string
lString16 LVReadCssText( LVStreamRef stream );
/// read file contents to string
lString16 LVReadCssText( lString16 filename );

LVStreamRef GetFB2Coverpage(LVStreamRef stream);

#endif // __LVXML_H_INCLUDED__
