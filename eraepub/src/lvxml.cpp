/** CoolReader Engine

   lvxml.cpp:  XML parser implementation

   (c) Vadim Lopatin, 2000-2006
   Copyright (C) 2013-2020 READERA LLC

   This source code is distributed under the terms of
   GNU General Public License
   See LICENSE file for details
*/

#include "include/FootnotesPrinter.h"
#include "include/lvxml.h"
#include "include/crtxtenc.h"
#include "include/fb2def.h"
#include "include/lvdocview.h"
#include "include/crconfig.h"
#include "include/epubfmt.h"

typedef struct {
   unsigned short indx; /* index into big table */
   unsigned short used; /* bitmask of used entries */
} Summary16;

typedef unsigned int ucs4_t;
#if GBK_ENCODING_SUPPORT == 1
#include "include/encodings/gbkext1.h"
#include "include/encodings/gbkext2.h"
#include "include/encodings/gb2312.h"
#include "include/encodings/cp936ext.h"
#endif
#if JIS_ENCODING_SUPPORT == 1
#include "include/encodings/jisx0213.h"
#endif
#if BIG5_ENCODING_SUPPORT == 1
#include "include/encodings/big5.h"
#include "include/encodings/big5_2003.h"
#endif
#if EUC_KR_ENCODING_SUPPORT == 1
#include "include/encodings/ksc5601.h"
#endif

#define BUF_SIZE_INCREMENT 4096
#define MIN_BUF_DATA_SIZE 4096
#define CP_AUTODETECT_BUF_SIZE 0x20000

int CalcTabCount(const lChar16 * str, int nlen);
void ExpandTabs(lString16 & s);
void ExpandTabs(lString16 & buf, const lChar16 * str, int len);

/// virtual destructor
LVFileFormatParser::~LVFileFormatParser()
{
}

LVFileParserBase::LVFileParserBase( LVStreamRef stream )
    : m_stream(stream)
    , m_buf(NULL)
    , m_buf_size(0)
    , m_stream_size(0)
    , m_buf_len(0)
    , m_buf_pos(0)
    , m_buf_fpos(0)
    , m_stopped(false)
{
    m_stream_size = stream.isNull()?0:stream->GetSize();
}

/// override to return file reading position percent
int LVFileParserBase::getProgressPercent()
{
    if ( m_stream_size<=0 )
        return 0;
    return (int)((lInt64)100 * (m_buf_pos + m_buf_fpos) / m_stream_size);
}

lString16 LVFileParserBase::getFileName()
{
    if ( m_stream.isNull() )
        return lString16::empty_str;
    lString16 name( m_stream->GetName() );
    int lastPathDelim = -1;
    for ( int i=0; i<name.length(); i++ ) {
        if ( name[i]=='\\' || name[i]=='/' ) {
            lastPathDelim = i;
        }
    }
    name = name.substr( lastPathDelim+1, name.length()-lastPathDelim-1 );
    return name;
}

LVTextFileBase::LVTextFileBase( LVStreamRef stream )
    : LVFileParserBase(stream)
    , m_enc_type( ce_8bit_cp )
    , m_conv_table(NULL)
    , eof_(false)
{
    clearCharBuffer();
}


/// stops parsing in the middle of file, to read header only
void LVFileParserBase::Stop()
{
    //CRLog::trace("LVTextFileBase::Stop() is called!");
    m_stopped = true;
}

/// destructor
LVFileParserBase::~LVFileParserBase()
{
    if (m_buf)
        free( m_buf );
}

/// destructor
LVTextFileBase::~LVTextFileBase()
{
    if (m_conv_table)
        delete[] m_conv_table;
}

static int charToHex( lUInt8 ch )
{
    if ( ch>='0' && ch<='9' )
        return ch-'0';
    if ( ch>='a' && ch<='f' )
        return ch-'a'+10;
    if ( ch>='A' && ch<='F' )
        return ch-'A'+10;
    return -1;
}


/// reads one character from buffer in RTF format
lChar16 LVTextFileBase::ReadRtfChar( int, const lChar16 * conv_table )
{
    lChar16 ch = m_buf[m_buf_pos++];
    lChar16 ch2 = m_buf[m_buf_pos];
    if ( ch=='\\' && ch2!='\'' ) {
    } else if (ch=='\\' ) {
        m_buf_pos++;
        int digit1 = charToHex( m_buf[0] );
        int digit2 = charToHex( m_buf[1] );
        m_buf_pos+=2;
        if ( digit1>=0 && digit2>=0 ) {
            ch = ( (lChar8)((digit1 << 4) | digit2) );
            if ( ch&0x80 )
                return conv_table[ch&0x7F];
            else
                return ch;
        } else {
            return '?';
        }
    } else {
        if ( ch>=' ' ) {
            if ( ch&0x80 )
                return conv_table[ch&0x7F];
            else
                return ch;
        }
    }
    return ' ';
}

void LVTextFileBase::checkEof()
{
    if ( m_buf_fpos+m_buf_len >= this->m_stream_size-4 )
        m_buf_pos = m_buf_len = m_stream_size - m_buf_fpos; //force eof
        //m_buf_pos = m_buf_len = m_stream_size - (m_buf_fpos+m_buf_len);
}

#if GBK_ENCODING_SUPPORT == 1
// based on code from libiconv
static lChar16 cr3_gb2312_mbtowc(const unsigned char *s)
{
    unsigned char c1 = s[0];
    if ((c1 >= 0x21 && c1 <= 0x29) || (c1 >= 0x30 && c1 <= 0x77)) {
        unsigned char c2 = s[1];
        if (c2 >= 0x21 && c2 < 0x7f) {
            unsigned int i = 94 * (c1 - 0x21) + (c2 - 0x21);
            if (i < 1410) {
                if (i < 831)
                    return gb2312_2uni_page21[i];
            } else {
                if (i < 8178)
                    return gb2312_2uni_page30[i-1410];
            }
        }
    }
    return 0;
}

// based on code from libiconv
static lChar16 cr3_cp936ext_mbtowc (const unsigned char *s)
{
    unsigned char c1 = s[0];
    if ((c1 == 0xa6) || (c1 == 0xa8)) {
        unsigned char c2 = s[1];
        if ((c2 >= 0x40 && c2 < 0x7f) || (c2 >= 0x80 && c2 < 0xff)) {
            unsigned int i = 190 * (c1 - 0x81) + (c2 - (c2 >= 0x80 ? 0x41 : 0x40));
            if (i < 7410) {
                if (i >= 7189 && i < 7211)
                    return cp936ext_2uni_pagea6[i-7189];
            } else {
                if (i >= 7532 && i < 7538)
                    return cp936ext_2uni_pagea8[i-7532];
            }
        }
    }
    return 0;
}

// based on code from libiconv
static lChar16 cr3_gbkext1_mbtowc (lChar16 c1, lChar16 c2)
{
    if ((c1 >= 0x81 && c1 <= 0xa0)) {
        if ((c2 >= 0x40 && c2 < 0x7f) || (c2 >= 0x80 && c2 < 0xff)) {
        unsigned int i = 190 * (c1 - 0x81) + (c2 - (c2 >= 0x80 ? 0x41 : 0x40));
        if (i < 6080)
            return gbkext1_2uni_page81[i];
        }
    }
    return 0;
}

// based on code from libiconv
static lChar16 cr3_gbkext2_mbtowc(lChar16 c1, lChar16 c2)
{
    if ((c1 >= 0xa8 && c1 <= 0xfe)) {
        if ((c2 >= 0x40 && c2 < 0x7f) || (c2 >= 0x80 && c2 < 0xa1)) {
            unsigned int i = 96 * (c1 - 0x81) + (c2 - (c2 >= 0x80 ? 0x41 : 0x40));
            if (i < 12016)
                return gbkext2_2uni_pagea8[i-3744];
        }
    }
    return 0;
}
#endif

#if JIS_ENCODING_SUPPORT == 1
// based on code from libiconv
static lChar16 cr3_jisx0213_to_ucs4(unsigned int row, unsigned int col)
{
    lChar16 val;

    if (row >= 0x121 && row <= 0x17e)
        row -= 289;
    else if (row == 0x221)
        row -= 451;
    else if (row >= 0x223 && row <= 0x225)
        row -= 452;
    else if (row == 0x228)
        row -= 454;
    else if (row >= 0x22c && row <= 0x22f)
        row -= 457;
    else if (row >= 0x26e && row <= 0x27e)
        row -= 519;
    else
        return 0x0000;

    if (col >= 0x21 && col <= 0x7e)
        col -= 0x21;
    else
        return 0x0000;

    val = (lChar16)jisx0213_to_ucs_main[row * 94 + col];
    val = (lChar16)jisx0213_to_ucs_pagestart[val >> 8] + (val & 0xff);
    if (val == 0xfffd)
        val = 0x0000;
    return val;
}
#endif

#if BIG5_ENCODING_SUPPORT == 1
// based on code from libiconv
static lUInt16 cr3_big5_mbtowc(lChar16 c1, lChar16 c2)
{
    if ((c1 >= 0xa1 && c1 <= 0xc7) || (c1 >= 0xc9 && c1 <= 0xf9)) {
        if ((c2 >= 0x40 && c2 < 0x7f) || (c2 >= 0xa1 && c2 < 0xff)) {
            unsigned int i = 157 * (c1 - 0xa1) + (c2 - (c2 >= 0xa1 ? 0x62 : 0x40));
            unsigned short wc = 0xfffd;
            if (i < 6280) {
                if (i < 6121)
                    wc = big5_2uni_pagea1[i];
            } else {
                if (i < 13932)
                    wc = big5_2uni_pagec9[i-6280];
            }
            if (wc != 0xfffd) {
                return wc;
            }
        }
    }
    return 0;
}

#endif

#if EUC_KR_ENCODING_SUPPORT == 1
// based on code from libiconv
static lChar16 cr3_ksc5601_mbtowc(lChar16 c1, lChar16 c2)
{
    if ((c1 >= 0x21 && c1 <= 0x2c) || (c1 >= 0x30 && c1 <= 0x48) || (c1 >= 0x4a && c1 <= 0x7d)) {
        if (c2 >= 0x21 && c2 < 0x7f) {
            unsigned int i = 94 * (c1 - 0x21) + (c2 - 0x21);
            unsigned short wc = 0xfffd;
            if (i < 1410) {
                if (i < 1115)
                    wc = ksc5601_2uni_page21[i];
            } else if (i < 3854) {
                if (i < 3760)
                    wc = ksc5601_2uni_page30[i-1410];
            } else {
                if (i < 8742)
                    wc = ksc5601_2uni_page4a[i-3854];
            }
            if (wc != 0xfffd) {
                return wc;
            }
        }
    }
    return 0;
}
#endif


/// reads several characters from buffer
int LVTextFileBase::ReadChars( lChar16 * buf, int maxsize )
{
    if (m_buf_pos >= m_buf_len)
        return 0;
    int count = 0;
    switch ( m_enc_type ) {
    case ce_8bit_cp:
    case ce_utf8:
        if ( m_conv_table!=NULL ) {
            for ( ; count<maxsize && m_buf_pos<m_buf_len; count++ ) {
                lUInt16 ch = m_buf[m_buf_pos++];
                buf[count] = ( (ch & 0x80) == 0 ) ? ch : m_conv_table[ch&0x7F];
            }
            return count;
        } else  {
            int srclen = m_buf_len - m_buf_pos;
            int dstlen = maxsize;
            Utf8ToUnicode(m_buf + m_buf_pos, srclen, buf, dstlen);
            m_buf_pos += srclen;
            if (dstlen == 0) {
                checkEof();
            }
            return dstlen;
        }
    case ce_utf16_be:
        {
            for ( ; count<maxsize; count++ ) {
                if ( m_buf_pos+1>=m_buf_len ) {
                    checkEof();
                    return count;
                }
                lUInt16 ch = m_buf[m_buf_pos++];
                lUInt16 ch2 = m_buf[m_buf_pos++];
                buf[count] = (ch << 8) | ch2;
            }
            return count;
        }

#if GBK_ENCODING_SUPPORT == 1
    case ce_gbk:
    {
        // based on ICONV code, gbk.h
        for ( ; count<maxsize; count++ ) {
            if (m_buf_pos >= m_buf_len) {
                checkEof();
                return count;
            }
            lUInt16 ch = m_buf[m_buf_pos++];
            int twoBytes = ch >= 0x81 && ch < 0xFF ? 1 : 0;
            if ( m_buf_pos + twoBytes>=m_buf_len ) {
                checkEof();
                return count;
            }
            lUInt16 ch2 = 0;
            if (twoBytes)
                ch2 = m_buf[m_buf_pos++];
            lUInt16 res = twoBytes ? 0 : ch;
            if (res == 0 && ch >= 0xa1 && ch <= 0xf7) {
                if (ch == 0xa1) {
                    if (ch2 == 0xa4) {
                        res = 0x00b7;
                    }
                    if (ch2 == 0xaa) {
                        res = 0x2014;
                    }
                }
                if (ch2 >= 0xa1 && ch2 < 0xff) {
                    unsigned char buf[2];
                    buf[0] = (lUInt8)(ch - 0x80);
					buf[1] = (lUInt8)(ch2 - 0x80);
                    res = cr3_gb2312_mbtowc(buf);
                    if (!res)
                        res = cr3_cp936ext_mbtowc(buf);
                }
            }
            if (res == 0 && ch >= 0x81 && ch <= 0xa0)
                res = cr3_gbkext1_mbtowc(ch, ch2);
            if (res == 0 && ch >= 0xa8 && ch <= 0xfe)
                res = cr3_gbkext2_mbtowc(ch, ch2);
            if (res == 0 && ch == 0xa2) {
                if (ch2 >= 0xa1 && ch2 <= 0xaa) {
                    res = 0x2170 + (ch2 - 0xa1);
                }
            }
            if (res == 0)
                res = '?'; // replace invalid chars with ?
            buf[count] = res;
        }
        return count;
    }
#endif
#if JIS_ENCODING_SUPPORT == 1
    case ce_shift_jis:
    {
        // based on ICONV code, gbk.h
        for ( ; count < maxsize - 1; count++ ) {
            if (m_buf_pos >= m_buf_len) {
                checkEof();
                return count;
            }
            lUInt16 ch = m_buf[m_buf_pos++];
            lUInt16 res = 0;
            if (ch < 0x80) {
                /* Plain ISO646-JP character. */
                if (ch == 0x5c)
                    res = 0x00a5;
                else if (ch == 0x7e)
                    res = 0x203e;
                else
                    res = ch;
            } else if (ch >= 0xa1 && ch <= 0xdf) {
                res = ch + 0xfec0;
            } else {
                if ((ch >= 0x81 && ch <= 0x9f) || (ch >= 0xe0 && ch <= 0xfc)) {
                    /* Two byte character. */
                    if (m_buf_pos + 1 >= m_buf_len) {
                        checkEof();
                        return count;
                    }
                    lUInt16 ch2 = 0;
                    ch2 = m_buf[m_buf_pos++];
                    if ((ch2 >= 0x40 && ch2 <= 0x7e) || (ch2 >= 0x80 && ch2 <= 0xfc)) {
                        lChar16 ch1;
                        /* Convert to row and column. */
                        if (ch < 0xe0)
                            ch -= 0x81;
                        else
                            ch -= 0xc1;
                        if (ch2 < 0x80)
                            ch2 -= 0x40;
                        else
                            ch2 -= 0x41;
                        /* Now 0 <= ch <= 0x3b, 0 <= ch2 <= 0xbb. */
                        ch1 = 2 * ch;
                        if (ch2 >= 0x5e)
                            ch2 -= 0x5e, ch1++;
                        ch2 += 0x21;
                        if (ch1 >= 0x5e) {
                            /* Handling of JISX 0213 plane 2 rows. */
                            if (ch1 >= 0x67)
                                ch1 += 230;
                            else if (ch1 >= 0x63 || ch1 == 0x5f)
                                ch1 += 168;
                            else
                                ch1 += 162;
                        }
                        lChar16 wc = cr3_jisx0213_to_ucs4(0x121+ch1, ch2);
                        if (wc) {
                            if (wc < 0x80) {
                                /* It's a combining character. */
                                lChar16 wc1 = jisx0213_to_ucs_combining[wc - 1][0];
                                lChar16 wc2 = jisx0213_to_ucs_combining[wc - 1][1];
                                buf[count++] = wc1;
                                res = wc2;
                            } else
                                res = wc;
                        }
                    }
                }
            }


            if (res == 0)
                res = '?'; // replace invalid chars with ?
            buf[count] = res;
        }
        return count;
    }
    case ce_euc_jis:
    {
        // based on ICONV code, gbk.h
        for ( ; count < maxsize-1; count++ ) {
            lUInt16 ch = m_buf[m_buf_pos++];
            lUInt16 res = 0;
            if (ch < 0x80) {
                /* Plain ASCII character. */
                res = ch;
            } else {
                if ((ch >= 0xa1 && ch <= 0xfe) || ch == 0x8e || ch == 0x8f) {
                    /* Two byte character. */
                    if (m_buf_pos + 1 >= m_buf_len) {
                        checkEof();
                        return count;
                    }
                    lUInt16 ch2 = m_buf[m_buf_pos++];
                    if (ch2 >= 0xa1 && ch2 <= 0xfe && ch == 0x8f && m_buf_pos + 2 >= m_buf_len) {
                        checkEof();
                        return count;
                    }

                    if (ch2 >= 0xa1 && ch2 <= 0xfe) {
                        if (ch == 0x8e) {
                            /* Half-width katakana. */
                            if (ch2 <= 0xdf) {
                              res = ch2 + 0xfec0;
                            }
                        } else {
                            lChar16 wc;
                            if (ch == 0x8f) {
                                /* JISX 0213 plane 2. */
                                lUInt16 ch3 = m_buf[m_buf_pos++];
                                wc = cr3_jisx0213_to_ucs4(0x200-0x80+ch2,ch3^0x80);
                            } else {
                                /* JISX 0213 plane 1. */
                                wc = cr3_jisx0213_to_ucs4(0x100-0x80+ch,ch2^0x80);
                            }
                            if (wc) {
                                if (wc < 0x80) {
                                    /* It's a combining character. */
                                    ucs4_t wc1 = jisx0213_to_ucs_combining[wc - 1][0];
                                    ucs4_t wc2 = jisx0213_to_ucs_combining[wc - 1][1];
                                    /* We cannot output two Unicode characters at once. So,
                                       output the first character and buffer the second one. */
                                    buf[count++] = (lChar16)wc1;
                                    res = (lChar16)wc2;
                                } else
                                    res = (lChar16)wc;
                            }
                        }
                    }
                }
            }

            if (res == 0)
                res = '?'; // replace invalid chars with ?
            buf[count] = res;
        }
        return count;
    }
#endif
#if BIG5_ENCODING_SUPPORT == 1
    case ce_big5:
    {
        // based on ICONV code, gbk.h
        for ( ; count < maxsize - 1; count++ ) {
            if (m_buf_pos >= m_buf_len) {
                checkEof();
                return count;
            }
            lUInt16 ch = m_buf[m_buf_pos++];
            lUInt16 res = 0;
            /* Code set 0 (ASCII) */
            if (ch < 0x80) {
                res = ch;
            } else if (ch >= 0x81 && ch < 0xff) {
                /* Code set 1 (BIG5 extended) */
                {
                    if (m_buf_pos + 1 >= m_buf_len) {
                        checkEof();
                        return count;
                    }
                    lUInt16 ch2 = m_buf[m_buf_pos++];
                    if ((ch2 >= 0x40 && ch2 < 0x7f) || (ch2 >= 0xa1 && ch2 < 0xff)) {
                        if (ch >= 0xa1) {
                            if (ch < 0xa3) {
                                unsigned int i = 157 * (ch - 0xa1) + (ch2 - (ch2 >= 0xa1 ? 0x62 : 0x40));
                                lChar16 wc = big5_2003_2uni_pagea1[i];
                                if (wc != 0xfffd) {
                                    res = wc;
                                }
                            }
                            if (!((ch == 0xc6 && ch2 >= 0xa1) || ch == 0xc7)) {
                                if (!(ch == 0xc2 && ch2 == 0x55)) {
                                    res = cr3_big5_mbtowc(ch, ch2);
                                    if (!res) {
                                        if (ch == 0xa3) {
                                            if (ch2 >= 0xc0 && ch2 <= 0xe1) {
                                                res = (ch2 == 0xe1 ? 0x20ac : ch2 == 0xe0 ? 0x2421 : 0x2340 + ch2);
                                            }
                                        } else if (ch == 0xf9) {
                                            if (ch2 >= 0xd6) {
                                                res = big5_2003_2uni_pagef9[ch2-0xd6];
                                            }
                                        } else if (ch >= 0xfa) {
                                            res = 0xe000 + 157 * (ch - 0xfa) + (ch2 - (ch2 >= 0xa1 ? 0x62 : 0x40));
                                        }
                                    }
                                } else {
                                    /* c == 0xc2 && c2 == 0x55. */
                                    res = 0x5f5e;
                                }
                            } else {
                                /* (c == 0xc6 && c2 >= 0xa1) || c == 0xc7. */
                                unsigned int i = 157 * (ch - 0xc6) + (ch2 - (ch2 >= 0xa1 ? 0x62 : 0x40));
                                if (i < 133) {
                                    /* 63 <= i < 133. */
                                    lChar16 wc = big5_2003_2uni_pagec6[i-63];
                                    if (wc != 0xfffd) {
                                        res = wc;
                                    }
                                } else if (i < 216) {
                                    /* 133 <= i < 216. Hiragana. */
                                    res = (lChar16)(0x3041 - 133 + i);
                                } else if (i < 302) {
                                    /* 216 <= i < 302. Katakana. */
                                    res = (lChar16)(0x30a1 - 216 + i);
                                }
                            }
                        } else {
                            /* 0x81 <= c < 0xa1. */
                            res = (ch >= 0x8e ? 0xdb18 : 0xeeb8) + 157 * (ch - 0x81)
                                    + (ch2 - (ch2 >= 0xa1 ? 0x62 : 0x40));
                        }
                    }
                }
            }


            if (res == 0)
                res = '?'; // replace invalid chars with ?
            buf[count] = res;
        }
        return count;
    }
#endif
#if EUC_KR_ENCODING_SUPPORT == 1
    case ce_euc_kr:
    {
        // based on ICONV code, gbk.h
        for ( ; count < maxsize - 1; count++ ) {
            if (m_buf_pos >= m_buf_len) {
                checkEof();
                return count;
            }
            lUInt16 ch = m_buf[m_buf_pos++];
            lUInt16 res = 0;

            /* Code set 0 (ASCII or KS C 5636-1993) */
            if (ch < 0x80)
                res = ch;
            else if (ch >= 0xa1 && ch < 0xff) {
                if (m_buf_pos + 1 >= m_buf_len) {
                    checkEof();
                    return count;
                }
                /* Code set 1 (KS C 5601-1992, now KS X 1001:2002) */
                lUInt16 ch2 = m_buf[m_buf_pos++];
                if (ch2 >= 0xa1 && ch2 < 0xff) {
                    res = cr3_ksc5601_mbtowc(ch-0x80, ch2-0x80);
                }
            }

            if (res == 0)
                res = '?'; // replace invalid chars with ?
            buf[count] = res;
        }
        return count;
    }
#endif

    case ce_utf16_le:
        {
            for ( ; count<maxsize; count++ ) {
                if ( m_buf_pos+1>=m_buf_len ) {
                    checkEof();
                    return count;
                }
                lUInt16 ch = m_buf[m_buf_pos++];
                lUInt16 ch2 = m_buf[m_buf_pos++];
                buf[count] = (ch2 << 8) | ch;
            }
            return count;
        }
    case ce_utf32_be:
        // support 24 bits only
        {
            for ( ; count<maxsize; count++ ) {
                if ( m_buf_pos+3>=m_buf_len ) {
                    checkEof();
                    return count;
                }
                m_buf_pos++; //lUInt16 ch = m_buf[m_buf_pos++];
                lUInt16 ch2 = m_buf[m_buf_pos++];
                lUInt16 ch3 = m_buf[m_buf_pos++];
                lUInt16 ch4 = m_buf[m_buf_pos++];
                buf[count] = (ch2 << 16) | (ch3 << 8) | ch4;
            }
            return count;
        }
    case ce_utf32_le:
        // support 24 bits only
        {
            for ( ; count<maxsize; count++ ) {
                if ( m_buf_pos+3>=m_buf_len ) {
                    checkEof();
                    return count;
                }
                lUInt16 ch = m_buf[m_buf_pos++];
                lUInt16 ch2 = m_buf[m_buf_pos++];
                lUInt16 ch3 = m_buf[m_buf_pos++];
                m_buf_pos++; //lUInt16 ch4 = m_buf[m_buf_pos++];
                buf[count] = (ch3 << 16) | (ch2 << 8) | ch;
            }
            return count;
        }
    default:
        return 0;
    }
}

/// tries to autodetect text encoding
bool LVTextFileBase::AutodetectEncoding( bool utfOnly )
{
    char enc_name[32];
    char lang_name[32];
    lvpos_t oldpos = m_stream->GetPos();
    unsigned sz = CP_AUTODETECT_BUF_SIZE;
    m_stream->SetPos( 0 );
    if ( sz>m_stream->GetSize() )
        sz = m_stream->GetSize();
    if ( sz < 16 )
    {
        CRLog::error("LVTextFileBase::AutodetectEncoding: sz<16! Trying to continue...");
        //return false;
    }
    unsigned char * buf = new unsigned char[ sz ];
    lvsize_t bytesRead = 0;
    if (m_stream->Read(buf, sz, &bytesRead) != LVERR_OK)
    {
        CRLog::error("LVTextFileBase::AutodetectEncoding failed to read");
        delete[] buf;
        m_stream->SetPos( oldpos );
        return false;
    }

    int res = 0;
    bool hasTags = hasXmlTags(buf, sz);
    if ( utfOnly )
        res = AutodetectCodePageUtf(buf, sz, enc_name, lang_name);
    else
        res = AutodetectCodePage(buf, sz, enc_name, lang_name, hasTags);
    delete[] buf;
    m_stream->SetPos( oldpos );
    if ( res) {
        //CRLog::debug("Code page decoding results: encoding=%s, lang=%s", enc_name, lang_name);
        m_lang_name = lString16( lang_name );
        SetCharset( lString16( enc_name ).c_str() );
    }

    // restore state
    return res!=0  || utfOnly;
}

/// seek to specified stream position
bool LVFileParserBase::Seek( lvpos_t pos, int bytesToPrefetch )
{
    if ( pos >= m_buf_fpos && pos+bytesToPrefetch <= (m_buf_fpos+m_buf_len) ) {
        m_buf_pos = (pos - m_buf_fpos);
        return true;
    }
    if ( pos>=m_stream_size )
        return false;
    unsigned bytesToRead = (bytesToPrefetch > m_buf_size) ? bytesToPrefetch : m_buf_size;
    if ( bytesToRead < BUF_SIZE_INCREMENT )
        bytesToRead = BUF_SIZE_INCREMENT;
    if ( bytesToRead > (m_stream_size - pos) )
        bytesToRead = (m_stream_size - pos);
    if ( (unsigned)m_buf_size < bytesToRead ) {
        m_buf_size = bytesToRead;
        m_buf = cr_realloc( m_buf, m_buf_size );
    }
    m_buf_fpos = pos;
    m_buf_pos = 0;
    m_buf_len = m_buf_size;
    // TODO: add error handing
    if ( m_stream->SetPos( m_buf_fpos ) != m_buf_fpos ) {
        CRLog::error("cannot set stream position to %d", (int)m_buf_pos );
        return false;
    }
    lvsize_t bytesRead = 0;
    if ( m_stream->Read( m_buf, bytesToRead, &bytesRead ) != LVERR_OK ) {
        CRLog::error("error while reading %d bytes from stream", (int)bytesToRead);
        return false;
    }
    return true;
}

/// reads specified number of bytes, converts to characters and saves to buffer
int LVTextFileBase::ReadTextBytes( lvpos_t pos, int bytesToRead, lChar16 * buf, int buf_size, int flags)
{
    if ( !Seek( pos, bytesToRead ) ) {
        CRLog::error("LVTextFileBase::ReadTextBytes seek error! cannot set pos to %d to read %d bytes", (int)pos, (int)bytesToRead);
        return 0;
    }
    int chcount = 0;
    int max_pos = m_buf_pos + bytesToRead;
    if ( max_pos > m_buf_len )
        max_pos = m_buf_len;
    if ( (flags & TXTFLG_RTF)!=0 ) {
        char_encoding_type enc_type = ce_utf8;
        lChar16 * conv_table = NULL;
        if ( flags & TXTFLG_ENCODING_MASK ) {
        // set new encoding
            int enc_id = (flags & TXTFLG_ENCODING_MASK) >> TXTFLG_ENCODING_SHIFT;
            if ( enc_id >= ce_8bit_cp ) {
                conv_table = (lChar16 *)GetCharsetByte2UnicodeTableById( enc_id );
                enc_type = ce_8bit_cp;
            } else {
                conv_table = NULL;
                enc_type = (char_encoding_type)enc_id;
            }
        }
        while ( m_buf_pos<max_pos && chcount < buf_size ) {
            *buf++ = ReadRtfChar(enc_type, conv_table);
            chcount++;
        }
    } else {
        return ReadChars( buf, buf_size );
    }
    return chcount;
}

bool LVFileParserBase::FillBuffer( int bytesToRead )
{
    lvoffset_t bytesleft = (lvoffset_t) (m_stream_size - (m_buf_fpos+m_buf_len));
    if (bytesleft<=0)
        return true; //FIX
    if (bytesToRead > bytesleft)
        bytesToRead = (int)bytesleft;
    int space = m_buf_size - m_buf_len;
    if (space < bytesToRead)
    {
        if ( m_buf_pos>bytesToRead || m_buf_pos>((m_buf_len*3)>>2) )
        {
            // just move
            int sz = (int)(m_buf_len -  m_buf_pos);
            for (int i=0; i<sz; i++)
            {
                m_buf[i] = m_buf[i+m_buf_pos];
            }
            m_buf_len = sz;
            m_buf_fpos += m_buf_pos;
            m_buf_pos = 0;
            space = m_buf_size - m_buf_len;
        }
        if (space < bytesToRead)
        {
            m_buf_size = m_buf_size + (bytesToRead - space + BUF_SIZE_INCREMENT);
            m_buf = cr_realloc( m_buf, m_buf_size );
        }
    }
    lvsize_t n = 0;
    if ( m_stream->Read(m_buf+m_buf_len, bytesToRead, &n) != LVERR_OK )
        return false;
//    if ( CRLog::isTraceEnabled() ) {
//        const lUInt8 * s = m_buf + m_buf_len;
//        const lUInt8 * s2 = m_buf + m_buf_len + (int)n - 8;
//        CRLog::trace("fpos=%06x+%06x, sz=%04x, data: %02x %02x %02x %02x %02x %02x %02x %02x .. %02x %02x %02x %02x %02x %02x %02x %02x",
//                     m_buf_fpos, m_buf_len, (int) n,
//                     s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
//                     s2[0], s2[1], s2[2], s2[3], s2[4], s2[5], s2[6], s2[7]
//                     );
//    }
    m_buf_len += (int)n;
    return (n>0);
}

void LVFileParserBase::Reset()
{
    m_stream->SetPos(0);
    m_buf_fpos = 0;
    m_buf_pos = 0;
    m_buf_len = 0;
    m_stream_size = m_stream->GetSize();
}

void LVTextFileBase::Reset()
{
    LVFileParserBase::Reset();
    eof_ = false;
    clearCharBuffer();
    // Remove Byte Order Mark from beginning of file
    if (PeekCharFromBuffer() == 0xFEFF)
        ReadCharFromBuffer();
}

void LVTextFileBase::SetCharset( const lChar16 * name )
{
    m_encoding_name = lString16( name );
    if ( m_encoding_name == "utf-8" ) {
        m_enc_type = ce_utf8;
        SetCharsetTable( NULL );
    } else if ( m_encoding_name == "utf-16" ) {
        m_enc_type = ce_utf16_le;
        SetCharsetTable( NULL );
#if GBK_ENCODING_SUPPORT == 1
    } else if ( m_encoding_name == "gbk" || m_encoding_name == "cp936" || m_encoding_name == "cp-936") {
        m_enc_type = ce_gbk;
        SetCharsetTable( NULL );
#endif
#if JIS_ENCODING_SUPPORT == 1
    } else if ( m_encoding_name == "shift-jis" || m_encoding_name == "shift_jis" || m_encoding_name == "sjis" || m_encoding_name == "ms_kanji" || m_encoding_name == "csshiftjis" || m_encoding_name == "shift_jisx0213" || m_encoding_name == "shift_jis-2004" || m_encoding_name == "cp932") {
        m_enc_type = ce_shift_jis;
        SetCharsetTable( NULL );
    } else if (m_encoding_name == "euc-jisx0213" ||  m_encoding_name == "euc-jis-2004" ||  m_encoding_name == "euc-jis" ||  m_encoding_name == "euc-jp" ||  m_encoding_name == "eucjp") {
        m_enc_type = ce_euc_jis;
        SetCharsetTable( NULL );
#endif
#if BIG5_ENCODING_SUPPORT == 1
    } else if ( m_encoding_name == "big5" || m_encoding_name == "big5-2003" || m_encoding_name == "big-5" || m_encoding_name == "big-five" || m_encoding_name == "bigfive" || m_encoding_name == "cn-big5" || m_encoding_name == "csbig5" || m_encoding_name == "cp950") {
        m_enc_type = ce_big5;
        SetCharsetTable( NULL );
#endif
#if EUC_KR_ENCODING_SUPPORT == 1
    } else if ( m_encoding_name == "euc_kr" || m_encoding_name == "euc-kr" || m_encoding_name == "euckr" || m_encoding_name == "cseuckr" || m_encoding_name == "cp51949" || m_encoding_name == "cp949") {
        m_enc_type = ce_euc_kr;
        SetCharsetTable( NULL );
#endif
    } else if ( m_encoding_name == "utf-16le" ) {
        m_enc_type = ce_utf16_le;
        SetCharsetTable( NULL );
    } else if ( m_encoding_name == "utf-16be" ) {
        m_enc_type = ce_utf16_be;
        SetCharsetTable( NULL );
    } else if ( m_encoding_name == "utf-32" ) {
        m_enc_type = ce_utf32_le;
        SetCharsetTable( NULL );
    } else if ( m_encoding_name == "utf-32le" ) {
        m_enc_type = ce_utf32_le;
        SetCharsetTable( NULL );
    } else if ( m_encoding_name == "utf-32be" ) {
        m_enc_type = ce_utf32_be;
        SetCharsetTable( NULL );
    } else {
        m_enc_type = ce_8bit_cp;
        //CRLog::trace("charset: %s", LCSTR(lString16(name)));
        const lChar16 * table = GetCharsetByte2UnicodeTable( name );
        if ( table )
            SetCharsetTable( table );
    }
}

void LVTextFileBase::SetCharsetTable( const lChar16 * table )
{
    if (!table)
    {
        if (m_conv_table)
        {
            delete[] m_conv_table;
            m_conv_table = NULL;
        }
        return;
    }
    m_enc_type = ce_8bit_cp;
    if (!m_conv_table)
        m_conv_table = new lChar16[128];
    lStr_memcpy( m_conv_table, table, 128 );
}


static const lChar16 * heading_volume[] = {
    L"volume",
    L"vol",
    L"\x0442\x043e\x043c", // tom
    NULL
};

static const lChar16 * heading_part[] = {
    L"part",
    L"\x0447\x0430\x0441\x0442\x044c", // chast'
    NULL
};

static const lChar16 * heading_chapter[] = {
    L"chapter",
    L"\x0433\x043B\x0430\x0432\x0430", // glava
    NULL
};

static bool startsWithOneOf( const lString16 & s, const lChar16 * list[] )
{
    lString16 str = s;
    str.lowercase();
    const lChar16 * p = str.c_str();
    for ( int i=0; list[i]; i++ ) {
        const lChar16 * q = list[i];
        int j=0;
        for ( ; q[j]; j++ ) {
            if ( !p[j] ) {
                return (!q[j] || q[j]==' ');
            }
            if ( p[j] != q[j] )
                break;
        }
        if ( !q[j] )
            return true;
    }
    return false;
}

int DetectHeadingLevelByText(const lString16& str) {
    if (!TXT_SMART_HEADERS) {
        return 0;
    }
    if (str.empty()) {
        return 0;
    }
    if (startsWithOneOf(str, heading_volume)) {
        return 1;
    }
    if (startsWithOneOf(str, heading_part)) {
        return 2;
    }
    if (startsWithOneOf(str, heading_chapter)) {
        return 3;
    }
    lChar16 ch = str[0];
    if (ch >= '0' && ch <= '9') {
        int i;
        int point_count = 0;
        for (i = 1; i < str.length(); i++) {
            ch = str[i];
            if (ch >= '0' && ch <= '9') {
                continue;
            }
            if (ch != '.') {
                return 0;
            }
            point_count++;
        }
        return (str.length() < 80) ? 5 + point_count : 0;
    }
    if (ch == 'I' || ch == 'V' || ch == 'X') {
        static const char* romeNumbers[] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII",
                                            "IX", "X", "XI", "XII", "XIII", "XIV", "XV", "XVI",
                                            "XVII", "XVIII", "XIX", "XX", "XXI", "XXII", "XXIII",
                                            "XXIV", "XXV", "XXVI", "XXVII", "XXVIII", "XXIX",
                                            "XXX", "XXXI", "XXXII", "XXXIII", "XXXIV", "XXXV",
                                            "XXXVI", "XXXVII", "XXXVIII", "XXXIX", NULL};
        int i = 0;
        for (i = 0; romeNumbers[i]; i++) {
            if (!lStr_cmp(str.c_str(), romeNumbers[i])) {
                return 4;
            }
        }
    }
    return 0;
}

#define LINE_IS_HEADER 0x2000
#define LINE_HAS_EOLN 1

typedef enum {
    la_unknown,  // not detected
    la_empty,    // empty line
    la_left,     // left aligned
    la_indent,   // right aligned
    la_centered, // centered
    la_right,    // right aligned
    la_width     // justified width
} lineAlign_t;

class LVTextFileLine {
public:
    //lvpos_t fpos;   // position of line in file
    //lvsize_t fsize;  // size of data in file
    lUInt32 flags; // flags. 1=eoln
    lString16 text; // line text
    lUInt16 lpos; // left non-space char position
    lUInt16 rpos; // right non-space char posision + 1
    lineAlign_t align;

    bool empty() { return rpos == 0; }

    bool isHeading() {
        return TXT_SMART_HEADERS ? (flags & LINE_IS_HEADER) != 0 : false;
    }

    LVTextFileLine(LVTextFileBase* file, int maxsize)
            : flags(0), lpos(0), rpos(0), align(la_unknown)
    {
        text = file->ReadLine(maxsize, flags);
        //CRLog::debug("    line read: %s", UnicodeToUtf8(text).c_str());

        if (!text.empty())
        {
            if(file->Eof())
            {
                text = text.TrimEndQuestionChar(text);
                //CRLog::debug("    line fixed: %s", UnicodeToUtf8(text).c_str());
            }

            const lChar16* s = text.c_str();
            for (int p = 0; *s; s++)
            {
                if (*s == '\t')
                {
                    p = (p + 8) % 8;
                }
                else
                {
                    if (*s != ' ') {
                        if (rpos == 0 && p > 0)
                        {
                            //CRLog::debug("    lpos = %d", p);
                            lpos = (lUInt16) p;
                        }
                        rpos = (lUInt16) (p + 1);
                    }
                    p++;
                }
            }
        }
    }
};

// returns char like '*' in "* * *"
lChar16 getSingleLineChar( const lString16 & s) {
    lChar16 nonSpace = 0;
    for ( const lChar16 * p = s.c_str(); *p; p++ ) {
        lChar16 ch = *p;
        if ( ch!=' ' && ch!='\t' && ch!='\r' && ch!='\n' ) {
            if ( nonSpace==0 )
                nonSpace = ch;
            else if ( nonSpace!=ch )
                return 0;
        }
    }
    return nonSpace;
}

#define MAX_HEADING_CHARS 48
#define MAX_PARA_LINES 30
#define MAX_BUF_LINES  200
#define MIN_MULTILINE_PARA_WIDTH 45

class LVTextLineQueue : public LVPtrVector<LVTextFileLine>
{
private:
    LVTextFileBase* file;
    int maxLineSize;
    int first_line_index;
    lString16 bookTitle;
    lString16 bookAuthors;
    lString16 seriesName;
    lString16 seriesNumber;
    int smart_format_flags_;
    int min_left;
    int max_right;
    int avg_left;
    int avg_right;
    int avg_center;
    int blocks_count_;
    int para_count_;
    int linesToSkip;
    bool lastParaWasTitle;
    bool inSubSection;
    int max_left_stats_pos;
    int max_left_second_stats_pos;
    int max_right_stats_pos;
    bool firstpage_thumb_;
    enum {
        tftParaPerLine = 1,
        tftParaIdents = 2,
        tftParaEmptyLineDelim = 4,
        tftHeadersCentered = 8,
        tftHeadersEmptyLineDelim = 16,
        tftFormatted = 32, // text lines are wrapped and formatted
        tftJustified = 64, // right bound is justified
        tftHeadersDoubleEmptyLineBefore = 128,
        tftPreFormatted = 256,
        tftPML = 512 // Palm Markup Language
    } smart_format_flags__t;
public:
    LVTextLineQueue(LVTextFileBase* file, bool firstpage_thumb, int maxLineLen)
            : file(file),
              firstpage_thumb_(firstpage_thumb),
              maxLineSize(maxLineLen),
              first_line_index(0),
              lastParaWasTitle(false),
              inSubSection(false) {
        min_left = -1;
        max_right = -1;
        avg_left = 0;
        avg_right = 0;
        avg_center = 0;
        blocks_count_ = 0;
        para_count_ = 0;
        linesToSkip = 0;
        smart_format_flags_ = tftPreFormatted;
    }

    void reinit()
    {
        clear();
        smart_format_flags_ = tftParaPerLine | tftHeadersEmptyLineDelim; //default for txtSmartFormat()
    }

    // get index of first line of queue
    int  GetFirstLineIndex() { return first_line_index; }
    // get line count read from file. Use length() instead to get count of lines queued.
    int  GetLineCount() { return first_line_index + length(); }
    // get line by line file index
    LVTextFileLine * GetLine( int index )
    {
        return get(index - first_line_index);
    }
    // remove lines from head of queue
    void RemoveLines(int lineCount)
    {
        if ((unsigned)lineCount > (unsigned)length())
            lineCount = length();
        erase(0, lineCount);
        first_line_index += lineCount;
    }
    // read lines and place to tail of queue
    bool ReadLines( int lineCount )
    {
        for ( int i=0; i<lineCount; i++ ) {
            if ( file->Eof() ) {
                if ( i==0 )
                    return false;
                break;
            }
            LVTextFileLine * line = new LVTextFileLine( file, maxLineSize );
            if ( min_left>=0 )
                line->align = getFormat( line );
            add( line );
        }
        return true;
    }
    inline static int absCompare( int v1, int v2 )
    {
        if ( v1<0 )
            v1 = -v1;
        if ( v2<0 )
            v2 = -v2;
        if ( v1>v2 )
            return 1;
        else if ( v1==v2 )
            return 0;
        else
            return -1;
    }
    lineAlign_t getFormat( LVTextFileLine * line )
    {
        if ( line->lpos>=line->rpos )
            return la_empty;
        int center_dist = (line->rpos + line->lpos) / 2 - avg_center;
        int right_dist = line->rpos - avg_right;
        int left_dist = line->lpos - max_left_stats_pos;
        if ( (smart_format_flags_ & tftJustified) || (smart_format_flags_ & tftFormatted) ) {
            if ( line->lpos==min_left && line->rpos==max_right )
                return la_width;
            if ( line->lpos==min_left )
                return la_left;
            if ( line->rpos==max_right )
                return la_right;
            if ( line->lpos==max_left_second_stats_pos )
                return la_indent;
            if ( line->lpos > max_left_second_stats_pos && absCompare( center_dist, left_dist )<0
                    && absCompare( center_dist, right_dist )<0 )
                return la_centered;
            if ( absCompare( right_dist, left_dist )<0 )
                return la_right;
            if ( line->lpos > min_left )
                return la_indent;
            return la_left;
        } else {
            if ( line->lpos == min_left )
                return la_left;
            else
                return la_indent;
        }
    }
    static bool isCentered( LVTextFileLine * line )
    {
        return line->align == la_centered;
    }
    void TxtSmartFormat()
    {
        smart_format_flags_ = tftParaPerLine | tftHeadersEmptyLineDelim; // default format
        if (length() < MAX_PARA_LINES) {
            return;
        }
        smart_format_flags_ = 0;
        avg_center = 0;
        int empty_lines = 0;
        int ident_lines = 0;
        int center_lines = 0;
        min_left = -1;
        max_right = -1;
        avg_left = 0;
        avg_right = 0;
        int pmlTagCount = 0;
        int pmlNotTagCount = 0;
        int i;
#define MAX_PRE_STATS 1000
        int left_stats[MAX_PRE_STATS];
        int right_stats[MAX_PRE_STATS];
        for ( i=0; i<MAX_PRE_STATS; i++ )
            left_stats[i] = right_stats[i] = 0;
        for ( i=0; i<length(); i++ ) {
            LVTextFileLine * line = get(i);
            //CRLog::debug("   LINE: %d .. %d", line->lpos, line->rpos);
            if ( line->lpos == line->rpos ) {
                empty_lines++;
            } else {
                if ( line->lpos < MAX_PRE_STATS )
                    left_stats[line->lpos]++;
                if ( line->rpos < MAX_PRE_STATS )
                    right_stats[line->rpos]++;
                if ( min_left==-1 || line->lpos<min_left )
                    min_left = line->lpos;
                if ( max_right==-1 || line->rpos>max_right )
                    max_right = line->rpos;
                avg_left += line->lpos;
                avg_right += line->rpos;
                for (int j=line->lpos; j<line->rpos-1; j++ ) {
                    lChar16 ch = line->text[j];
                    lChar16 ch2 = line->text[j+1];
                    if ( ch=='\\' ) {
                        switch ( ch2 ) {
                        case 'p':
                        case 'x':
                        case 'X':
                        case 'C':
                        case 'c':
                        case 'r':
                        case 'u':
                        case 'o':
                        case 'v':
                        case 't':
                        case 'n':
                        case 's':
                        case 'b':
                        case 'l':
                        case 'a':
                        case 'U':
                        case 'm':
                        case 'q':
                        case 'Q':
                            pmlTagCount++;
                            break;
                        default:
                            pmlNotTagCount++;
                        }
                    }
                }
            }
        }
        // pos stats
        int max_left_stats = 0;
        max_left_stats_pos = 0;
        int max_left_second_stats = 0;
        max_left_second_stats_pos = 0;
        int max_right_stats = 0;
        max_right_stats_pos = 0;
        for ( i=0; i<MAX_PRE_STATS; i++ ) {
            if ( left_stats[i] > max_left_stats ) {
                max_left_stats = left_stats[i];
                max_left_stats_pos = i;
            }
            if ( right_stats[i] > max_right_stats ) {
                max_right_stats = right_stats[i];
                max_right_stats_pos = i;
            }
        }
        for ( i=max_left_stats_pos + 1; i<MAX_PRE_STATS; i++ ) {
            if ( left_stats[i] > max_left_second_stats ) {
                max_left_second_stats = left_stats[i];
                max_left_second_stats_pos = i;
            }
        }


        int non_empty_lines = length() - empty_lines;
        if ( non_empty_lines < 10 )
            return;
        avg_left /= non_empty_lines;
        avg_right /= non_empty_lines;
        avg_center = (avg_left + avg_right) / 2;

        //int best_left_align_percent = max_left_stats * 100 / length();
        int best_right_align_percent = max_right_stats * 100 / length();
        //int best_left_second_align_percent = max_left_second_stats * 100 / length();

        int fw = max_right_stats_pos - max_left_stats_pos;
        for ( i=0; i<length(); i++ ) {
            LVTextFileLine * line = get(i);
            int lw = line->rpos - line->lpos;
            if ( line->lpos > min_left+1 ) {
                int center_dist = (line->rpos + line->lpos) / 2 - avg_center;
                //int right_dist = line->rpos - avg_right;
                int left_dist = line->lpos - max_left_stats_pos;
                //if ( absCompare( center_dist, right_dist )<0 )
                if ( absCompare( center_dist, left_dist )<0 ) {
                    if ( line->lpos > min_left+fw/10 && line->lpos < max_right-fw/10 && lw < 9*fw/10 ) {
                        center_lines++;
                    }
                } else
                    ident_lines++;
            }
        }
        for ( i=0; i<length(); i++ ) {
            get(i)->align = getFormat( get(i) );
        }
        if ( avg_right >= 80 ) {
            if ( empty_lines>non_empty_lines && empty_lines<non_empty_lines*110/100 ) {
                smart_format_flags_ = tftParaPerLine | tftHeadersDoubleEmptyLineBefore; // default format
                return;
            }
            if ( empty_lines>non_empty_lines*2/3 ) {
                smart_format_flags_ = tftParaEmptyLineDelim; // default format
                return;
            }
            //tftHeadersDoubleEmptyLineBefore
            return;
        }
        smart_format_flags_ = 0;
        int ident_lines_percent = ident_lines * 100 / non_empty_lines;
        int center_lines_percent = center_lines * 100 / non_empty_lines;
        int empty_lines_percent = empty_lines * 100 / length();
        if (empty_lines_percent > 5 && max_right < 80) {
            smart_format_flags_ |= tftParaEmptyLineDelim;
        }
        if (ident_lines_percent > 5 && ident_lines_percent < 55) {
            smart_format_flags_ |= tftParaIdents;
            if (empty_lines_percent < 7) {
                smart_format_flags_ |= tftHeadersEmptyLineDelim;
            }
        }
        if (center_lines_percent > 1) {
            smart_format_flags_ |= tftHeadersCentered;
        }
        if (max_right < 80) {
            // text lines are wrapped and formatted
            smart_format_flags_ |= tftFormatted;
        }
        if (max_right_stats_pos == max_right && best_right_align_percent > 30) {
            // right bound is justified
            smart_format_flags_ |= tftJustified;
        }
        if (!smart_format_flags_ && pmlTagCount>20  && pmlNotTagCount == 0) {
            smart_format_flags_ = tftPML; // Palm markup
            return;
        }
        CRLog::debug("TxtSmartFormat() min_left=%d, max_right=%d, ident=%d, empty=%d, flags=%d",
                     min_left, max_right, ident_lines_percent, empty_lines_percent, smart_format_flags_);
        if (!smart_format_flags_) {
            // default format
            smart_format_flags_ = tftParaPerLine | tftHeadersEmptyLineDelim;
        }
    }

    bool TxtSmartDescriptionProjectGutenberg() {
        int i = 0;
        for (; i < length() && get(i)->rpos == 0; i++) {
        }
        if (i >= length()) {
            return false;
        }
        bookTitle.clear();
        bookAuthors.clear();
        lString16 firstLine = get(i)->text;
        lString16 pgPrefix("The Project Gutenberg Etext of ");
        if (firstLine.length() < pgPrefix.length()) {
            return false;
        }
        if (firstLine.substr(0, pgPrefix.length()) != pgPrefix) {
            return false;
        }
        firstLine = firstLine.substr(pgPrefix.length(), firstLine.length() - pgPrefix.length());
        int byPos = firstLine.pos(", by ");
        if (byPos <= 0) {
            return false;
        }
        bookTitle = firstLine.substr(0, byPos);
        bookAuthors = firstLine.substr(byPos + 5, firstLine.length() - byPos - 5);
        for (; i < length() && i < 500 && get(i)->text.pos("*END*") != 0; i++) {
        }
        if (i < length() && i < 500) {
            for (i++; i < length() && i < 500 && get(i)->text.empty(); i++) {
            }
            linesToSkip = i;
        }
        return true;
    }
    /// check beginning of file for book title, author and series
    void TxtSmartDescription(LvXMLParserCallback* callback) {
        if (!TXT_SMART_DESCRIPTION) {
            return;
        }
        if (!TxtSmartDescriptionProjectGutenberg()) {
            bookTitle.clear();
            bookAuthors.clear();
        }
        lString16Collection author_list;
        if (!bookAuthors.empty()) {
            author_list.parse(bookAuthors, ',', true);
        }
        int i;
        for (i = 0; i < author_list.length(); i++) {
            lString16Collection name_list;
            name_list.parse(author_list[i], ' ', true);
            if (name_list.length() > 0) {
                lString16 firstName = name_list[0];
                lString16 lastName;
                lString16 middleName;
                if (name_list.length() == 2) {
                    lastName = name_list[1];
                } else if (name_list.length() > 2) {
                    middleName = name_list[1];
                    lastName = name_list[2];
                }
                callback->OnTagOpenNoAttr(NULL, L"author");
                callback->OnTagOpenNoAttr(NULL, L"first-name");
                lUInt32 text_flags = TXTFLG_TRIM | TXTFLG_TRIM_REMOVE_EOL_HYPHENS;
                if (!firstName.empty()) {
                    callback->OnText(firstName.c_str(), firstName.length(), text_flags);
                }
                callback->OnTagClose(NULL, L"first-name");
                callback->OnTagOpenNoAttr(NULL, L"middle-name");
                if (!middleName.empty()) {
                    callback->OnText(middleName.c_str(), middleName.length(), text_flags);
                }
                callback->OnTagClose(NULL, L"middle-name");
                callback->OnTagOpenNoAttr(NULL, L"last-name");
                if (!lastName.empty()) {
                    callback->OnText(lastName.c_str(), lastName.length(), text_flags);
                }
                callback->OnTagClose(NULL, L"last-name");
                callback->OnTagClose(NULL, L"author");
            }
        }
        callback->OnTagOpenNoAttr(NULL, L"book-title");
        if (!bookTitle.empty()) {
            callback->OnText(bookTitle.c_str(), bookTitle.length(), 0);
        }
        callback->OnTagClose(NULL, L"book-title");
        if (!seriesName.empty() || !seriesNumber.empty()) {
            callback->OnTagOpenNoAttr(NULL, L"sequence");
            if (!seriesName.empty()) {
                callback->OnAttribute(NULL, L"name", seriesName.c_str());
            }
            if (!seriesNumber.empty()) {
                callback->OnAttribute(NULL, L"number", seriesNumber.c_str());
            }
            callback->OnTagClose(NULL, L"sequence");
        }
        // remove description lines
        if (linesToSkip > 0) {
            RemoveLines(linesToSkip);
        }
    }
    /// add one paragraph
    void AddEmptyLine(LvXMLParserCallback* callback) {
        callback->OnTagOpenAndClose(NULL, L"empty-line");
    }
    /// add one paragraph
    void AddPara(int startline, int endline, LvXMLParserCallback* callback) {
        // TODO: remove pos, sz tracking
        lString16 str;
        //lvpos_t pos = 0;
        //lvsize_t sz = 0;
        for (int i = startline; i <= endline; i++) {
            LVTextFileLine* item = get(i);
            //if ( i==startline )
            //    pos = item->fpos;
            //sz = (item->fpos + item->fsize) - pos;
            str += item->text + "\n";
        }
        str.trimDoubleSpaces(false, false, true);
        bool isHeader = false;
        lChar16 singleChar = 0;
        if (TXT_SMART_HEADERS) {
            singleChar = getSingleLineChar(str);
            if (singleChar != 0 && singleChar >= 'A') {
                singleChar = 0;
            }
            bool singleLineFollowedByEmpty = false;
            bool singleLineFollowedByTwoEmpty = false;
            if (startline == endline && endline < length() - 1) {
                if (!(smart_format_flags_ & tftParaIdents) || get(startline)->lpos > 0) {
                    if (get(endline + 1)->rpos == 0
                        && (startline == 0 || get(startline - 1)->rpos == 0)) {
                        singleLineFollowedByEmpty = get(startline)->text.length() < MAX_HEADING_CHARS;
                        if ((startline <= 1 || get(startline - 2)->rpos == 0)) {
                            singleLineFollowedByTwoEmpty = get(startline)->text.length() < MAX_HEADING_CHARS;
                        }
                    }
                }
            }
            isHeader = singleChar != 0;
            if (smart_format_flags_ & tftHeadersDoubleEmptyLineBefore) {
                isHeader = singleLineFollowedByTwoEmpty;
                if (singleLineFollowedByEmpty && startline < 3 && str.length() < MAX_HEADING_CHARS) {
                    isHeader = true;
                } else if (startline < 2 && str.length() < MAX_HEADING_CHARS) {
                    isHeader = true;
                }
                if (str.length() == 0) {
                    return;
                } // no empty lines
            } else {
                if ((startline == endline && str.length() < 4)
                    || (para_count_ < 2 && str.length() < 50 && startline < length() - 2
                        && (get(startline + 1)->rpos == 0 || get(startline + 2)->rpos == 0))) {
                    isHeader = true;
                }
                if (startline == endline && get(startline)->isHeading()) {
                    isHeader = true;
                }
                if (smart_format_flags_ & tftHeadersCentered) {
                    if (startline == endline && isCentered(get(startline))) {
                        isHeader = true;
                    }
                }
                int hlevel = DetectHeadingLevelByText(str);
                if (hlevel > 0) {
                    isHeader = true;
                }
                if (singleLineFollowedByEmpty && !(smart_format_flags_ & tftParaEmptyLineDelim)) {
                    isHeader = true;
                }
            }
            if (str.length() > MAX_HEADING_CHARS) {
                isHeader = false;
            }
        }
        if (str.empty()) {
            if (!(smart_format_flags_ & tftParaEmptyLineDelim) || !isHeader) {
                callback->OnTagOpenAndClose(NULL, L"empty-line");
            }
            return;
        }
        const lChar16* title_tag = L"title";
        if (isHeader) {
            if (singleChar) {
                title_tag = L"subtitle";
                lastParaWasTitle = false;
            } else {
                if (!lastParaWasTitle) {
                    if (inSubSection) {
                        callback->OnTagClose(NULL, L"section");
                    }
                    callback->OnTagOpenNoAttr(NULL, L"section");
                    inSubSection = true;
                }
                lastParaWasTitle = true;
            }
            callback->OnTagOpenNoAttr(NULL, title_tag);
        } else {
            lastParaWasTitle = false;
        }
        callback->OnTagOpenNoAttr(NULL, L"p");
        if(str.CheckRTL())
        {
            callback->setRTLflag(true);
            callback->OnAttribute(L"",L"dir",L"rtl");
            gDocumentRTL = 1;
        }
        callback->OnText(str.c_str(), str.length(), TXTFLG_TRIM | TXTFLG_TRIM_REMOVE_EOL_HYPHENS);
        callback->OnTagClose(NULL, L"p");
        if (isHeader) {
            callback->OnTagClose(NULL, title_tag);
        }
        para_count_++;
    }

    class PMLTextImport {
        LvXMLParserCallback * callback;
        bool insideInvisibleText;
        const lChar16 * cp1252;
        int align; // 0, 'c' or 'r'
        lString16 line;
        int chapterIndent;
        bool insideChapterTitle;
        lString16 chapterTitle;
        int sectionId;
        bool inSection;
        bool inParagraph;
        bool indented;
        bool inLink;
        lString16 styleTags;
    public:
        PMLTextImport( LvXMLParserCallback * cb )
        : callback(cb), insideInvisibleText(false), align(0)
        , chapterIndent(0)
        , insideChapterTitle(false)
        , sectionId(0)
        , inSection(false)
        , inParagraph(false)
        , indented(false)
        , inLink(false)
        {
            cp1252 = GetCharsetByte2UnicodeTable(L"windows-1252");
        }
        void addChar( lChar16 ch ) {
            if ( !insideInvisibleText )
                line << ch;
        }
        const lChar16 * getStyleTagName( lChar16 ch ) {
            switch ( ch ) {
            case 'b':
            case 'B':
                return L"b";
            case 'i':
                return L"i";
            case 'u':
                return L"u";
            case 's':
                return L"strikethrough";
            case 'a':
                return L"a";
            default:
                return NULL;
            }
        }
        int styleTagPos(lChar16 ch) {
            for ( int i=0; i<styleTags.length(); i++ )
                if ( styleTags[i]==ch )
                    return i;
            return -1;
        }
        void closeStyleTag( lChar16 ch, bool updateStack ) {
            int pos = ch ? styleTagPos( ch ) : 0;
            if ( updateStack && pos<0 )
                return;
            //if ( updateStack )
            //if ( !line.empty() )
                postText();
            for ( int i=styleTags.length()-1; i>=pos; i-- ) {
                const lChar16 * tag = getStyleTagName(styleTags[i]);
                if ( updateStack )
                    styleTags.erase(styleTags.length()-1, 1);
                if ( tag ) {
                    callback->OnTagClose(L"", tag);
                }
            }
        }
        void openStyleTag( lChar16 ch, bool updateStack ) {
            int pos = styleTagPos( ch );
            if ( updateStack && pos>=0 )
                return;
            if ( updateStack )
            //if ( !line.empty() )
                postText();
            const lChar16 * tag = getStyleTagName(ch);
            if ( tag ) {
                callback->OnTagOpenNoAttr(L"", tag);
                if ( updateStack )
                    styleTags.append( 1,  ch );
            }
        }
        void openStyleTags() {
            for ( int i=0; i<styleTags.length(); i++ )
                openStyleTag(styleTags[i], false);
        }
        void closeStyleTags() {
            for ( int i=styleTags.length()-1; i>=0; i-- )
                closeStyleTag(styleTags[i], false);
        }
        void onStyleTag(lChar16 ch ) {
            int pos = ch!=0 ? styleTagPos( ch ) : 0;
            if ( pos<0 ) {
                openStyleTag(ch, true);
            } else {
                closeStyleTag(ch, true);
            }

        }
        void onImage( lString16 url ) {
            //url = cs16("book_img/") + url;
            callback->OnTagOpen(L"", L"img");
            callback->OnAttribute(L"", L"src", url.c_str());
            callback->OnTagBody();
            callback->OnTagClose(L"", L"img");
        }
        void startParagraph() {
            if ( !inParagraph ) {
                callback->OnTagOpen(L"", L"p");
                lString16 style;
                if ( indented )
                    style<< L"left-margin: 15%; ";
                if ( align ) {
                    if ( align=='c' ) {
                        style << L"text-align: center; ";
                        if ( !indented )
                            style << L"text-indent: 0px; ";
                    } else if ( align=='r' )
                        style << L"text-align: right; ";
                }
                if ( !style.empty() )
                    callback->OnAttribute(L"", L"style", style.c_str() );
                callback->OnTagBody();
                openStyleTags();
                inParagraph = true;
            }
        }
        void postText() {
            startParagraph();
            if ( !line.empty() ) {
                callback->OnText(line.c_str(), line.length(), 0);
                line.clear();
            }
        }
        void startPage() {
            if ( inSection )
                return;
            sectionId++;
            callback->OnTagOpen(NULL, L"section");
            callback->OnAttribute(NULL, L"id", (cs16("_section") + fmt::decimal(sectionId)).c_str() );
            callback->OnTagBody();
            inSection = true;
            endOfParagraph();
        }
        void endPage() {
            if ( !inSection )
                return;
            indented = false;
            endOfParagraph();
            callback->OnTagClose(NULL, L"section");
            inSection = false;
        }
        void newPage() {
            endPage();
            startPage();
        }
        void endOfParagraph() {
//            if ( line.empty() )
//                return;
            // post text
            //startParagraph();
            if ( !line.empty() )
                postText();
            // clear current text
            line.clear();
            if ( inParagraph ) {
                //closeStyleTag(0);
                closeStyleTags();
                callback->OnTagClose(L"", L"p");
                inParagraph = false;
            }
        }
        void addSeparator( int /*width*/ ) {
            endOfParagraph();
            callback->OnTagOpenAndClose(L"", L"hr");
        }
        void startOfChapterTitle( bool startNewPage, int level ) {
            endOfParagraph();
            if ( startNewPage )
                newPage();
            chapterTitle.clear();
            insideChapterTitle = true;
            chapterIndent = level;
            callback->OnTagOpenNoAttr(NULL, L"title");
        }
        void addChapterTitle( int /*level*/, lString16 title ) {
            // add title, invisible, for TOC only
        }
        void endOfChapterTitle() {
            chapterTitle.clear();
            if ( !insideChapterTitle )
                return;
            endOfParagraph();
            insideChapterTitle = false;
            callback->OnTagClose(NULL, L"title");
        }
        void addAnchor( lString16 ref ) {
            startParagraph();
            callback->OnTagOpen(NULL, L"a");
            callback->OnAttribute(NULL, L"name", ref.c_str());
            callback->OnTagBody();
            callback->OnTagClose(NULL, L"a");
        }
        void startLink( lString16 ref ) {
            if ( !inLink ) {
                postText();
                callback->OnTagOpen(NULL, L"a");
                callback->OnAttribute(L"",L"class",L"link_valid");
                callback->OnAttribute(NULL, L"href", ref.c_str());
                callback->OnTagBody();
                styleTags << "a";
                inLink = true;
            }
        }
        void endLink() {
            if ( inLink ) {
                inLink = false;
                closeStyleTag('a', true);
                //callback->OnTagClose(NULL, L"a");
            }
        }
        lString16 readParam( const lChar16 * str, int & j ) {
            lString16 res;
            if ( str[j]!='=' || str[j+1]!='\"' )
                return res;
            for ( j=j+2; str[j] && str[j]!='\"'; j++ )
                res << str[j];
            return res;
        }
        void processLine( lString16 text ) {
            int len = text.length();
            const lChar16 * str = text.c_str();
            for ( int j=0; j<len; j++ ) {
                //bool isStartOfLine = (j==0);
                lChar16 ch = str[j];
                lChar16 ch2 = str[j+1];
                if ( ch=='\\' ) {
                    if ( ch2=='a' ) {
                        // \aXXX	Insert non-ASCII character whose Windows 1252 code is decimal XXX.
                        int n = decodeDecimal( str + j + 2, 3 );
                        bool use1252 = true;
                        if ( n>=128 && n<=255 && use1252 ) {
                            addChar( cp1252[n-128] );
                            j+=4;
                            continue;
                        } else if ( n>=1 && n<=255 ) {
                            addChar((lChar16)n);
                            j+=4;
                            continue;
                        }
                    } else if ( ch2=='U' ) {
                        // \UXXXX	Insert non-ASCII character whose Unicode code is hexidecimal XXXX.
                        int n = decodeHex( str + j + 2, 4 );
                        if ( n>0 ) {
                            addChar((lChar16)n);
                            j+=5;
                            continue;
                        }
                    } else if ( ch2=='\\' ) {
                        // insert '\'
                        addChar( ch2 );
                        j++;
                        continue;
                    } else if ( ch2=='-' ) {
                        // insert '\'
                        addChar( UNICODE_SOFT_HYPHEN_CODE );
                        j++;
                        continue;
                    } else if ( ch2=='T' ) {
                        // Indents the specified percentage of the screen width, 50% in this case.
                        // If the current drawing position is already past the specified screen location, this tag is ignored.
                        j+=2;
                        lString16 w = readParam( str, j );
                        // IGNORE
                        continue;
                    } else if ( ch2=='m' ) {
                        // Insert the named image.
                        j+=2;
                        lString16 image = readParam( str, j );
                        onImage( image );
                        continue;
                    } else if ( ch2=='Q' ) {
                        // \Q="linkanchor" - Specify a link anchor in the document.
                        j+=2;
                        lString16 anchor = readParam( str, j );
                        addAnchor(anchor);
                        continue;
                    } else if ( ch2=='q' ) {
                        // \q="#linkanchor"Some text\q	Reference a link anchor which is at another spot in the document.
                        // The string after the anchor specification and before the trailing \q is underlined
                        // or otherwise shown to be a link when viewing the document.
                        if ( !inLink ) {
                            j+=2;
                            lString16 ref = readParam( str, j );
                            startLink(ref);
                        } else {
                            j+=1;
                            endLink();
                        }
                        continue;
                    } else if ( ch2=='w' ) {
                        // Embed a horizontal rule of a given percentage width of the screen, in this case 50%.
                        // This tag causes a line break before and after it. The rule is centered. The percent sign is mandatory.
                        j+=2;
                        lString16 w = readParam( str, j );
                        addSeparator( 50 );
                        continue;
                    } else if ( ch2=='C' ) {
                        // \Cn="Chapter title"
                        // Insert "Chapter title" into the chapter listing, with level n (like \Xn).
                        // The text is not shown on the page and does not force a page break.
                        // This can sometimes be useful to insert a chapter mark at the beginning of an introduction to the chapter, for example.
                        if ( str[2] && str[3]=='=' && str[4]=='\"' ) {
                            int level = hexDigit(str[2]);
                            if ( level<0 || level>4 )
                                level = 0;
                            j+=5; // skip \Cn="
                            lString16 title;
                            for ( ;str[j] && str[j]!='\"'; j++ )
                                title << str[j];
                            addChapterTitle( level, title );
                            continue;
                        } else {
                            j++;
                            continue;
                        }
                    } else {
                        bool unknown = false;
                        switch( ch2 ) {
                        case 'v':
                            insideInvisibleText = !insideInvisibleText;
                            break;
                        case 'c':
                            //if ( isStartOfLine ) {
                                endOfParagraph();
                                align = (align==0) ? 'c' : 0;
                            //}
                            break;
                        case 'r':
                            //if ( isStartOfLine ) {
                                endOfParagraph();
                                align = (align==0) ? 'r' : 0;
                            //}
                            break;
                        case 't':
                            indented = !indented;
                            break;
                        case 'i':
                            onStyleTag('i');
                            break;
                        case 'u':
                            onStyleTag('u');
                            break;
                        case 'o':
                            onStyleTag('s');
                            break;
                        case 'b':
                            onStyleTag('b');
                            break;
                        case 'd':
                            break;
                        case 'B':
                            onStyleTag('B');
                            break;
                        case 'p': //New page
                            newPage();
                            break;
                        case 'n':
                            // normal font
                            break;
                        case 's':
                            // small font
                            break;
//                        case 'b':
//                            // bold font
//                            break;
                        case 'l':
                            // large font
                            break;
                        case 'x': // New chapter; also causes a new page break.
                                  // Enclose chapter title (and any style codes) with \x and \x
                        case 'X': // New chapter, indented n levels (n between 0 and 4 inclusive)
                                  // in the Chapter dialog; doesn't cause a page break.
                                  // Enclose chapter title (and any style codes) with \Xn and \Xn
                            {
                                int level = 0;
                                if ( ch2=='X' ) {
                                    switch( str[j+2] ) {
                                    case '1':
                                        level = 1;
                                        break;
                                    case '2':
                                        level = 2;
                                        break;
                                    case '3':
                                        level = 3;
                                        break;
                                    case '4':
                                        level = 4;
                                        break;
                                    }
                                    j++;
                                }
                                if ( !insideChapterTitle ) {
                                    startOfChapterTitle( ch2=='x', level );
                                } else {
                                    endOfChapterTitle();
                                }
                                break;
                            }
                            break;
                        default:
                            unknown = true;
                            break;
                        }
                        if ( !unknown ) {
                            j++; // 2 chars processed
                            continue;
                        }
                    }
                }
                addChar( ch );
            }
            endOfParagraph();
        }
    };
    /// one line per paragraph
    bool DoPMLImport(LvXMLParserCallback* callback) {
        CRLog::debug("DoPMLImport()");
        RemoveLines(length());
        file->Reset();
        file->SetCharset(L"windows-1252");
        ReadLines(100);
        if(length() == 0)
        {
            return false;
        }
        int remainingLines = 0;
        PMLTextImport importer(callback);
        do {
            for (int i = remainingLines; i < length(); i++) {
                LVTextFileLine* item = get(i);
                lString16 temp = item->text;
                if(temp.CheckRTL())
                {
                    callback->setRTLflag(true);
                    callback->OnAttribute(L"",L"dir",L"rtl");
                    gDocumentRTL = 1;
                    temp.PrepareRTL();
                }
                importer.processLine(temp);
            }
            RemoveLines(length() - 3);
            remainingLines = 3;
        } while (ReadLines(100));
        importer.endPage();
        return true;
    }
    /// one line per paragraph
    bool DoParaPerLineImport(LvXMLParserCallback* callback) {
        CRLog::debug("DoParaPerLineImport()");
        bool done = false;
        int remaining_lines = 0;
        do {
            for (int i = remaining_lines; i < length(); i++) {
                LVTextFileLine* item = get(i);
                if (TXT_SMART_HEADERS && (smart_format_flags_ & tftHeadersDoubleEmptyLineBefore)) {
                    if (!item->empty()) {
                        AddPara(i, i, callback);
                        blocks_count_++;
                    }
                } else {
                    if (!item->empty()) {
                        AddPara(i, i, callback);
                    } else {
                        AddEmptyLine(callback);
                    }
                    blocks_count_++;
                }
                if (firstpage_thumb_ && blocks_count_ >= FIRSTPAGE_BLOCKS_MAX) {
                    done = true;
                    break;
                }
            }
            if (done) {
                break;
            }
            RemoveLines(length() - 3);
            remaining_lines = 3;
        } while (ReadLines(100));
        if (inSubSection) {
            callback->OnTagClose(NULL, L"section");
        }
        return true;
    }
    /// delimited by empty lines
    bool DoPreFormattedImport(LvXMLParserCallback* callback) {
        CRLog::debug("DoPreFormattedImport()");
        bool done = false;
        int remaining_lines = 0;
        do {
            for (int i = remaining_lines; i < length(); i++) {
                LVTextFileLine* item = get(i);
                if (item->rpos > item->lpos) {
                    callback->OnTagOpenNoAttr(NULL, L"pre");
                    if(item->text.CheckRTL())
                    {
                        callback->setRTLflag(true);
                        callback->OnAttribute(L"",L"dir",L"rtl");
                    }
                    callback->OnText(item->text.c_str(), item->text.length(), item->flags);
                    callback->OnTagClose(NULL, L"pre");
                } else {
                    callback->OnTagOpenAndClose(NULL, L"empty-line");
                }
                blocks_count_++;
                if (firstpage_thumb_ && blocks_count_ >= FIRSTPAGE_BLOCKS_MAX) {
                    done = true;
                    break;
                }
            }
            if (done) {
                break;
            }
            RemoveLines(length() - 3);
            remaining_lines = 3;
        } while (ReadLines(100));
        if (inSubSection) {
            callback->OnTagClose(NULL, L"section");
        }
        return true;
    }
    /// delimited by first line ident
    bool DoParaPerIdentImport(LvXMLParserCallback* callback) {
        CRLog::debug("DoParaPerIdentImport()");
        int pos = 0;
        for (;;) {
            if (length() - pos <= MAX_PARA_LINES) {
                if (pos) {
                    RemoveLines(pos);
                }
                ReadLines(MAX_BUF_LINES);
                pos = 0;
            }
            if (pos >= length()) {
                break;
            }
            int i = pos + 1;
            bool emptyLineFlag = false;
            if (pos >= length() || DetectHeadingLevelByText(get(pos)->text) == 0) {
                for (; i < length() && i < pos + MAX_PARA_LINES; i++) {
                    LVTextFileLine* item = get(i);
                    if (item->lpos > min_left) {
                        // ident
                        break;
                    }
                    if (item->lpos == item->rpos) {
                        // empty line
                        i++;
                        emptyLineFlag = true;
                        break;
                    }
                }
            }
            if (i > pos + 1 || !emptyLineFlag) {
                AddPara(pos, i - 1 - (emptyLineFlag ? 1 : 0), callback);
            } else {
                AddEmptyLine(callback);
            }
            blocks_count_++;
            pos = i;
            if (firstpage_thumb_ && blocks_count_ >= FIRSTPAGE_BLOCKS_MAX) {
                break;
            }
        }
        if (inSubSection) {
            callback->OnTagClose(NULL, L"section");
        }
        return true;
    }
    /// delimited by empty lines
    bool DoParaPerEmptyLinesImport(LvXMLParserCallback* callback) {
        CRLog::debug("DoParaPerEmptyLinesImport()");
        int pos = 0;   //position of a string in a paragraph
        int shortLineCount = 0;
        int emptyLineCount = 0;
        for (;;) {
            if (length() - pos <= MAX_PARA_LINES) {
                if (pos) {
                    RemoveLines(pos - 1);
                }
                ReadLines(MAX_BUF_LINES);
                pos = 1;
            }
            if (pos >= length()) {
                break;
            }
            // skip starting empty lines
            while (pos < length()) {
                LVTextFileLine* item = get(pos);
                if (item->lpos != item->rpos) {
                    break;    //Deleteing string here
                }
                pos++;        //Aaand deleting string here
            }
            int i = pos;
            if (pos >= length() || DetectHeadingLevelByText(get(pos)->text) == 0) {
                for (; i < length() && i < pos + MAX_PARA_LINES; i++) {
                    LVTextFileLine* item = get(i);
                    if (item->lpos == item->rpos) {
                        // empty line
                        emptyLineCount++;
                        break;
                    }
                    if (item->rpos - item->lpos < MIN_MULTILINE_PARA_WIDTH) {
                        // next line is very short, possible paragraph start
                        shortLineCount++;
                        break;
                    }
                    shortLineCount = 0;
                    emptyLineCount = 0;
                }
            }
            if (i == length()) {
                i--;
            }
            if (i >= pos) {
                AddPara(pos, i, callback);
                blocks_count_++;
                if (emptyLineCount) {
                    if (shortLineCount > 1) {
                        AddEmptyLine(callback);
                        blocks_count_++;
                    }
                    shortLineCount = 0;
                    emptyLineCount = 0;
                }
            }
            pos = i + 1;
            if (firstpage_thumb_ && blocks_count_ >= FIRSTPAGE_BLOCKS_MAX) {
                break;
            }
        }
        if (inSubSection) {
            callback->OnTagClose(NULL, L"section");
        }
        return true;
    }
    /// import document body
    bool DoTextImport(LvXMLParserCallback* callback) {
        if (smart_format_flags_ & tftPML) {
            CRLog::trace("tftPML");
            return DoPMLImport(callback);
        } else if (smart_format_flags_ & tftPreFormatted) {
            CRLog::trace("tftPreFormatted");
            return DoPreFormattedImport(callback);
        } else if (smart_format_flags_ & tftParaIdents) {
            CRLog::trace("tftParaIdents");
            return DoParaPerIdentImport(callback);
        } else if (smart_format_flags_ & tftParaEmptyLineDelim) {
            CRLog::trace("tftParaEmptyLineDelim");
            return DoParaPerEmptyLinesImport(callback);
        } else {
            CRLog::trace("DoParaPerLineImport");
            return DoParaPerLineImport(callback);
        }
    }
};

/// reads next text line, tells file position and size of line, sets EOL flag
lString16 LVTextFileBase::ReadLine(int maxLineSize, lUInt32& flags) {
    //fsize = 0;
    flags = 0;
    lString16 res;
    res.reserve(80);
    //FillBuffer( maxLineSize*3 );
    lChar16 ch = 0;
    int debug_sanitizedsymbolcounter =0;
    for (;;)
    {
        if (eof_)
        {
            // EOF: treat as EOLN
            flags |= LINE_HAS_EOLN; // EOLN flag
            break;
        }
        ch = ReadCharFromBuffer();
        /*ch== L'\u00B1' ||*/
        //lString16 temp;
        //temp.append(1, ch);
        //res.append(1, ch);
        //CRLog::trace("%s %08x %d", UnicodeToUtf8(temp).c_str(), ch, ch);
        if (ch != '\r' && ch != '\n')
        {
            if ( ch<=32 )
            {
                switch (ch)
                {
                    case ' ':
                    case '\t':
                    case 10:        //cr
                    case 13:        //lf
                    case 12:
                        //case 9:
                    case 8:
                    case 7:
                    case 30:
                    case 0x14:
                    case 0x15:

                        res.append(1, ch);
                        break;
                    default:  //case 0: here
                        CRLog::trace("Found illegal character. Replacing with u/FFFD.");
                        res.append(1, L'\ufffd');
                        debug_sanitizedsymbolcounter++;
                }
            }

            else
            {
                res.append(1, ch);
            }
            if (ch == ' ' || ch == '\t')
            {
                if (res.length() >= maxLineSize) {
                    break;
                }
            }
        }
        else
        {
            // eoln
            if (!eof_)
            {
                lChar16 ch2 = PeekCharFromBuffer();
                if (ch2 != ch && (ch2 == '\r' || ch2 == '\n'))
                {
                    ReadCharFromBuffer();
                }
            }
            flags |= LINE_HAS_EOLN; // EOLN flag
            break;
        }
    }
    if (!res.empty())
    {int firstNs = 0;
        lChar16 ch = 0;
        for (;; firstNs++)
        {ch = res[firstNs];
            if (!ch) {
                break;
            }
            if (ch != ' ' && ch != '\t')
            {
                break;
            }
        }
        if (ch == 0x14)
        {
            if (res[res.length() - 1] == 0x15)
            {
                // LIB.RU header flags
                res.erase(res.length() - 1, 1);
                res.erase(0, firstNs + 1);
                flags |= LINE_IS_HEADER;
            }
        }
        else
        {
            if (ch == '-' || ch == '*' || ch == '=')
            {
                bool sameChars = true;
                for (int i = firstNs; i < res.length(); i++)
                {
                    lChar16 ch2 = res[i];
                    if (ch2 != ' ' && ch2 != '\t' && ch2 != ch) {
                        sameChars = false;
                        break;
                    }
                }
                if (sameChars) {
                    res = "* * *"; // hline
                    flags |= LINE_IS_HEADER;
                }
            }}
    }
    res.pack();
    return res;
}

LVTextParser::LVTextParser(LVStreamRef stream, LvXMLParserCallback* callback, bool smart_format,
                           bool firstpage_thumb)
        : LVTextFileBase(stream),
          m_callback(callback),
          smart_format_(smart_format),
          firstpage_thumb_(firstpage_thumb) {}

LVTextParser::~LVTextParser() {}

/// returns true if format is recognized by parser
bool LVTextParser::CheckFormat()
{
    Reset();
    // encoding test
    if ( !AutodetectEncoding() )
        return false;
    #define TEXT_PARSER_DETECT_SIZE 16384
    Reset();
    lChar16 * chbuf = new lChar16[TEXT_PARSER_DETECT_SIZE];
    FillBuffer( TEXT_PARSER_DETECT_SIZE );
    int charsDecoded = ReadTextBytes( 0, m_buf_len, chbuf, TEXT_PARSER_DETECT_SIZE-1, 0 );
    delete[] chbuf;
    Reset();
    return true;
}

/// parses input stream
bool LVTextParser::Parse() {
    LVTextLineQueue queue(this, firstpage_thumb_, 2000);
    queue.ReadLines(2000);
    if (smart_format_) {
        queue.TxtSmartFormat();
    }
    // make fb2 document structure
    m_callback->OnTagOpen( NULL, L"?xml" );
    m_callback->OnAttribute( NULL, L"version", L"1.0" );
    m_callback->OnAttribute( NULL, L"encoding", GetEncodingName().c_str() );
    m_callback->OnEncoding( GetEncodingName().c_str(), GetCharsetTable( ) );
    m_callback->OnTagBody();
    m_callback->OnTagClose( NULL, L"?xml" );
    m_callback->OnTagOpenNoAttr( NULL, L"FictionBook" );
      // DESCRIPTION
      m_callback->OnTagOpenNoAttr( NULL, L"description" );
        m_callback->OnTagOpenNoAttr( NULL, L"title-info" );
          queue.TxtSmartDescription( m_callback );
        m_callback->OnTagClose( NULL, L"title-info" );
      m_callback->OnTagClose( NULL, L"description" );
      // BODY
      m_callback->OnTagOpenNoAttr( NULL, L"body" );
        //callback_->OnTagOpen( NULL, L"section" );
        bool res = queue.DoTextImport(m_callback);
        if(!res)
        {
            //fallback
            Reset();
            queue.reinit();
            queue.ReadLines(2000);
            queue.DoTextImport(m_callback);
        }
        //callback_->OnTagClose( NULL, L"section" );
      m_callback->OnTagClose( NULL, L"body" );
    m_callback->OnTagClose( NULL, L"FictionBook" );
    return true;
}

void LVTextParser::FullDom(){
    //do nothing
};
/*******************************************************************************/
// XML parser
/*******************************************************************************/


/// states of XML parser
enum parser_state_t {
    ps_bof,
    ps_lt,
    ps_attr,     // waiting for attributes or end of tag
    ps_text
};


void LvXmlParser::SetCharset(const lChar16 * name)
{
    LVTextFileBase::SetCharset(name);
    callback_->OnEncoding(name, m_conv_table);
}

void LvXmlParser::Reset()
{
    LVTextFileBase::Reset();
    m_state = ps_bof;
}

inline bool IsSpaceChar(lChar16 ch)
{
    return (ch == ' ')
        || (ch == '\t')
        || (ch == '\r')
        || (ch == '\n');
}

bool LvXmlParser::CheckFormat() {
    Reset();
    AutodetectEncoding();
    Reset();
	#define XML_PARSER_DETECT_SIZE 8192
    lChar16* chbuf = new lChar16[XML_PARSER_DETECT_SIZE];
    FillBuffer(XML_PARSER_DETECT_SIZE);
    int charsDecoded = ReadTextBytes(0, m_buf_len, chbuf, XML_PARSER_DETECT_SIZE-1, 0);
    chbuf[charsDecoded] = 0;
    bool res = false;
    if (charsDecoded > 30) {
        lString16 s( chbuf, charsDecoded );
        res = s.pos("<FictionBook") >= 0;
        if ( s.pos("<?xml") >= 0 && s.pos("version=") >= 6 ) {
            res = res || !m_fb2Only;
            int encpos;
            if ( res && (encpos=s.pos("encoding=\"")) >= 0 ) {
                lString16 encname = s.substr( encpos+10, 20 );
                int endpos = s.pos("\"");
                if ( endpos>0 ) {
                    encname.erase( endpos, encname.length() - endpos );
                    SetCharset( encname.c_str() );
                }
            }
        } else if ( !res && s.pos("<html xmlns=\"http://www.w3.org/1999/xhtml\"") >= 0) {
            res = m_allowHtml;
        } else if (!res && !m_fb2Only) {
            // not XML or XML without declaration;
            int lt_pos = s.pos("<");
            if ( lt_pos >= 0 && s.pos("xmlns") > lt_pos ) {
                // contains xml namespace declaration probably XML
                res = true;
                // check that only whitespace chars before <
                for ( int i=0; i<lt_pos && res; i++)
                {
                    // 0xFEFF is the Byte Order Mark (BOM), char at start of stream that:
                    // signals the byte order of the stream,
                    // marks text as a unicode text,
                    // signals the encoding of the stream.
                    res = (IsSpaceChar(chbuf[i]) || chbuf[i] == 0xFEFF);
                }
            }
        }
    }
    else
    {
        CRLog::error("LvXmlParser::CheckFormat() : Failed to check format. Chars decoded = %d",charsDecoded);
    }
    delete[] chbuf;
    Reset();
    return res;
}

void LvXmlParser::FullDom()
{
    need_coverpage_ = false;
}
bool LvXmlParser::Parse()
{
	Reset();
	EpubStylesManager stylesManager = getStylesManager();
    callback_->OnStart(this);
    //int txt_count = 0;
    int flags = callback_->getFlags();
    bool error = false;
    bool in_xml_tag = false;
    bool close_flag = false;
    bool q_flag = false;
    bool body_started = false;
    bool in_blockquote = false;
	bool firstpage_thumb_num_reached = false;
    int fragments_counter = 0;
    lString16 tagname;
    lString16 tagns;
    lString16 attrname;
    lString16 attrns;
    lString16 attrvalue;

    bool in_head = false;
    bool in_style = false;

    bool in_body = false;
    bool in_body_notes = false;
    bool in_note_section = false;
    bool in_sup = false;
    bool in_a = false;
    bool in_a_sup = false;
    bool in_aside = false;
    bool in_section = false;
    bool in_rearnote = false;
    bool in_rearnotes = false;
    bool in_div = false;
    bool a_is_fb2_note = false;
    bool in_image = false;
    bool in_font = false;
    bool in_font_header1 = false;
    bool in_font_header2 = false;
    bool in_font_header3 = false;

    bool in_video = false;

    bool in_note = false;
    bool save_a_content= false;
    bool save_notes_title = false;
    lString16 link_href;
    lString16 buffer;
    lString16 link_id;
    lString16 section_id;
    lString16 aside_old_id;
    lString16 rtl_holder;
    int rtl_holder_counter = 0;
	for (; !eof_ && !error && !firstpage_thumb_num_reached ;)
    {
	    if (m_stopped)
             break;
        // Load next portion of data if necessary
        lChar16 ch = PeekCharFromBuffer();
        switch (m_state)
        {
        case ps_bof:
            {
            	//CRLog::trace("LvXmlParser::Parse() ps_bof");
                // Skip file beginning until '<'
                for ( ; !eof_ && ch!='<'; ch = PeekNextCharFromBuffer())
                    ;
                if (!eof_)
                {
                    m_state = ps_lt;
                    ReadCharFromBuffer();
                }
                //CRLog::trace("LvXmlParser::Parse() ps_bof ret");
            }
            break;
        case ps_lt:  //look for tags
            {
            	//CRLog::trace("LvXmlParser::Parse() ps_lt");
                if (!SkipSpaces())
                    break;
                close_flag = false;
                q_flag = false;
                if (ch=='/')
                {
                    ch = ReadCharFromBuffer();
                    close_flag = true;
                }
                else if (ch=='?')
                {
                    // <?xml?>
                    ch = ReadCharFromBuffer();
                    q_flag = true;
                }
                else if (ch=='!')
                {
                    // comments etc...
                    if (PeekCharFromBuffer(1) == '-' && PeekCharFromBuffer(2) == '-') {
                        // skip comments
                        ch = PeekNextCharFromBuffer(2);
                        while (!eof_ && (ch != '-' || PeekCharFromBuffer(1) != '-'
                                || PeekCharFromBuffer(2) != '>') ) {
                            ch = PeekNextCharFromBuffer();
                        }
                        if (ch=='-' && PeekCharFromBuffer(1)=='-'
                                && PeekCharFromBuffer(2)=='>' )
                            ch = PeekNextCharFromBuffer(2);
                        m_state = ps_text;
                        break;
                    }
                }
                if (!ReadIdent(tagns, tagname) || PeekCharFromBuffer()=='=')
                {
                    // Error
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }
                if (possible_capitalized_tags_) {
                    tagns.lowercase();
                    tagname.lowercase();
                }
                if(in_body_notes && tagname == "empty-line")
                {
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }

                /*if(tagname=="style") //|| tagname=="table" || tagname=="tr" || tagname=="td") // skipping all <style> tags and <table> <tr> <td> tags
                {
                    //if (attrname=="name")
                    //{ if(attrvalue.pos("override")!=-1 || attrvalue.pos("GramE")!=-1 )// || attrvalue.pos("")!=-1 ){
                    //callback_->OnTagClose(tagns.c_str(), tagname.c_str());
                    //CRLog::trace("</%s>", LCSTR(tagname));
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                    //}}
                }*/

                if(tagname == "video")
                {
                    if(!close_flag)
                    {
                        in_video = true;
                    }
                    else
                    {
                        in_video = false;
                    }
                }

                if(tagname == "ruby")
                {
                    gRubyDetected = 1;
                }

                if(tagname == "head")
                {
                    in_head = !close_flag;
                }
                if(tagname == "style")
                {
                    in_style = !close_flag;
                }

                if(tagname == "description" && fb2_meta_only && close_flag)
                {
                    firstpage_thumb_num_reached = true;
                }

                if(tagname == "img" || tagname == "image")
                {
                    in_image = true;
                }

                if(tagname == "underline")
                {
                    tagname = "u";
                }
                if(tagname == "fb3-body")
                {
                    tagname = "body";
                }
                if(tagname=="div")
                {
                    if (!close_flag)
                    {
                        in_div = true;
                    }
                    else
                    {
                        in_div = false;
                    }
                }

                if (tagname=="br")
                {
                    callback_->OnTagOpen(L"",L"p");
                    callback_->OnText(L"\u200b", 1, flags);
                    callback_->OnTagClose(L"",L"p");
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }
                if(tagname=="blockquote")
                {
                    if (!in_blockquote && !close_flag)
                    {
                        in_blockquote = true;
                    }
                    else if (in_blockquote && !close_flag)
                    {
                        if (SkipTillChar('>'))
                        {
                            m_state = ps_text;
                            ReadCharFromBuffer();
                        }
                        break;
                    }
                    else if (in_blockquote && close_flag)
                    {
                        in_blockquote = false;
                    }
                    else if (!in_blockquote && close_flag)
                    {
                        if (SkipTillChar('>'))
                        {
                            m_state = ps_text;
                            ReadCharFromBuffer();
                        }
                        break;
                    }
                }

                if(tagname=="pagebreak")
                {
                    tagns = "";
                    if (close_flag)
                    {
                        callback_->OnText(L"\u200B", 1, flags);
                    }
                }

                if(tagname == "sup")
                {
                    if (!close_flag)
                    {
                        in_sup = true;
                        if(in_a)
                        {
                            in_a_sup = true;
                        }
                    }
                    else
                    {
                        in_sup = false;
                    }
                }
                //epub3
                if (tagname == "aside" )
                {
                    if(!close_flag)
                    {
                        in_aside = true;
                    }
                    else
                    {
                        //lString16 aside_new_id = lString16("aside_") + lString16::itoa(AsidesList_.links_.length());

                        //this->AsidesList_.links_.add(LinkStruct(0,lString16::empty_str,callback_->convertId(aside_old_id)));

//                        CRLog::error("aside old = %s, new = %s",LCSTR(aside_old_id),LCSTR(aside_new_id));
                        //callback_->OnAttribute(L"",L"id",aside_new_id.c_str());
                        callback_->OnAttribute(L"",L"id",L"~");
                        callback_->OnAttribute(L"",L"class",L"hidden");
                        in_aside = false;
                    }
                }

                if(in_rearnotes && (tagname =="h1"|| tagname =="h2"|| tagname =="h3"||
                                    tagname =="h4"|| tagname =="h5"|| tagname =="h6"|| tagname =="title"))
                {
                    if(!close_flag)
                    {
                        save_notes_title = true;
                    }
                    else
                    {
                        save_notes_title = false;
                    }
                }

                if(tagname=="section")
                {
                    if(!close_flag)
                    {
                        in_section = true;
                    }
                    else
                    {
                        in_section = false;
                        in_rearnote = false;
                        in_rearnotes = false;
                    }
                }

                if(tagname == "note")
                {
                    in_note = !close_flag;
                    tagname = "a";
                }

                if (tagname == "a")
                {
                    if (!close_flag)
                    {
                        save_a_content = true;
                        in_a = true;
                    }
                    else
                    {
                        if(!link_href.empty())
                        {
                            callback_->OnAttribute(L"",L"class",L"link_valid");
                        }
                        if(!buffer.empty())
                        {
                            bool flag1 = false;
                            bool flag2 = false;
                            if(buffer.startsWith("[")|| buffer.startsWith("{"))
                            {
                                buffer = buffer.substr(1);
                                flag1 = true;
                            }
                            if(buffer.endsWith("]")|| buffer.endsWith("}"))
                            {
                                buffer = buffer.substr(0,buffer.length()-1);
                                flag2 = true;
                            }
                            int bufnum = -1;
                            if(buffer == L"*")
                            {
                                bufnum = LinksList_.size() + 1;
                            }
                            else
                            {
                                bufnum = buffer.atoi();
                            }

                            //LE("%d && %d && %d && ((%d && %d) || %d || %d || %d)",!in_rearnote, !in_aside, bufnum>0, flag1, flag2 , in_sup, in_a_sup ,a_is_fb2_note );
                            if(!in_rearnote && !in_aside && bufnum>0
                            && ((flag1 && flag2) || in_sup || in_a_sup || a_is_fb2_note))
                            {
                                lString16 tmp_search;
                                if(link_id.empty())
                                {
                                    lString16 temp = lString16("back_") + lString16::itoa(LinksList_.length());
                                    callback_->OnAttribute(L"", L"id", temp.c_str());
                                    link_id = lString16("#") + callback_->convertId(temp);
                                    tmp_search = /*L"#" +*/ link_id;
                                    //LE("A tmp_search = [%s]",LCSTR(tmp_search));
                                }
                                else
                                {
                                    tmp_search = (link_id.startsWith("#")) ? link_id : L"#" + link_id;;
                                    //LE("B tmp_search = [%s]",LCSTR(tmp_search));
                                }

                                lString16 tmp_href = callback_->convertHref(link_href);
                                //LE("tmp_href = [%s]",LCSTR(tmp_href));
                                if (LinksMap_.find(tmp_search.getHash()) == LinksMap_.end())
                                {
                                    callback_->OnAttribute(L"", L"nref", (link_href + lString16("_note")).c_str());
                                    callback_->OnAttribute(L"", L"type", L"note");
                                    if(!in_a_sup && !in_sup)
                                    {
                                        callback_->OnAttribute(L"", L"class", L"note_class");
                                    }
                                    LinksList_.add(LinkStruct(bufnum, link_id, tmp_href));
                                }
                                LinksMap_[tmp_href.getHash()] = link_id;
                                //CRLog::error("LIST added [%s] to [%s]",LCSTR(link_id),LCSTR(link_href));
                                buffer = lString16::empty_str;

                            }
                        }
                        else
                        {
                            callback_->OnText(L"\u200B", 1, flags);
                        }
                        save_a_content = false;
                        link_href = lString16::empty_str;
                        link_id = lString16::empty_str;
                        in_a = false;
                        in_a_sup = false;
                        a_is_fb2_note = false;
                    }
                }

                if(tagname == "body")
                {
                    if (!close_flag)
                    {
                        in_body = true;
                    }
                    else
                    {
                        in_body_notes = false;
                        in_body = false;
                    }
                }

                if(tagname == "section" && in_body_notes)
                {
                    if (!close_flag)
                    {
                        in_note_section = true;
                    }
                    else
                    {
                        in_note_section = false;
                        section_id = lString16::empty_str;
                    }
                }
                if(tagname == "font")
                {
                    if (!close_flag)
                    {
                        in_font = true;
                    }
                    else
                    {
                        if(in_font_header1)
                        {
                            callback_->OnTagClose(L"",L"h1");
                        }
                        if(in_font_header2)
                        {
                            callback_->OnTagClose(L"",L"h2");
                        }
                        if(in_font_header3)
                        {
                            callback_->OnTagClose(L"",L"h3");
                        }
                        in_font = false;
                        in_font_header1 = false;
                        in_font_header2 = false;
                        in_font_header3 = false;
                    }
                }

                if(tagname == "title" && in_note_section)
                {
                    if (!close_flag)
                    {
                        //in_note_section = true

                        callback_->OnTagOpenNoAttr(L"",L"title");
                        if(LinksMap_.find(callback_->convertId(lString16("#") + section_id).getHash())!=LinksMap_.end())
                        {
                            callback_->OnTagOpen(L"",L"a");
                            lString16 href = LinksMap_.at(callback_->convertId(lString16("#") + section_id).getHash());
                            callback_->OnAttribute(L"",L"href",href.c_str());
                            callback_->OnAttribute(L"",L"class",L"link_valid");
                        }
                        break;
                    }
                    else
                    {
                        callback_->OnTagClose(L"",L"a");
                    }
                }
                if(!rtl_holder.empty() && tagname == rtl_holder)
                {
                    if(!close_flag)
                    {
                        rtl_holder_counter++;
                    }
                    else
                    {
                        rtl_holder_counter--;
                    }
                }

                if(gEmbeddedStylesLVL > 0 && !stylesManager.rtl_map_empty())
                {
                    if(stylesManager.ClassIsRTL(tagname))
                    {
                        callback_->OnAttribute(L"",L"dir",L"rtl");
                        callback_->setRTLflag(true);
                        gDocumentRTL = 1;
                    }
                }

                if (close_flag)
                {
                    if(tagname == rtl_holder && RTL_DISPLAY_ENABLE && ( rtl_holder_counter == 0 ) )
                    {
                        callback_->setRTLflag(false);
                        rtl_holder.clear();
                    }
                    callback_->OnTagClose(tagns.c_str(), tagname.c_str());
                    //CRLog::trace("</%s>", LCSTR(tagname));
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }

                if (q_flag) {
                    tagname.insert(0, 1, '?');
                    in_xml_tag = (tagname == "?xml");
                } else {
                    in_xml_tag = false;
                }
                callback_->OnTagOpen(tagns.c_str(), tagname.c_str());
                //CRLog::trace("<%s>", LCSTR(tagname));

                m_state = ps_attr;
                //CRLog::trace("LvXmlParser::Parse() ps_lt ret");
            }
            break;
        case ps_attr: //read tags
            {
                //CRLog::trace("LvXmlParser::Parse() ps_attr");
                if (!SkipSpaces())
                    break;
                ch = PeekCharFromBuffer();
                lChar16 nch = PeekCharFromBuffer(1);
                if (ch == '>' || ((ch == '/' || ch == '?') && nch == '>'))
                {
                    callback_->OnTagBody();
                    // end of tag
                    if (ch != '>')
                        callback_->OnTagClose(tagns.c_str(), tagname.c_str());
                    if (ch == '>')
                        ch = PeekNextCharFromBuffer();
                    else
                        ch = PeekNextCharFromBuffer(1);
                    m_state = ps_text;
                    break;
                }
                if (!ReadIdent(attrns, attrname))
                {
                    // error: skip rest of tag
                    SkipTillChar('<');
                    ch = PeekNextCharFromBuffer(1);
                    callback_->OnTagBody();
                    m_state = ps_lt;
                    break;
                }
                SkipSpaces();
                attrvalue.reset(16);
                ch = PeekCharFromBuffer();
                // Read attribute value
                if (ch == '=')
                {
                    // Skip '='
                    ReadCharFromBuffer();
                    SkipSpaces();
                    lChar16 qChar = 0;
                    ch = PeekCharFromBuffer();
                    if (ch == '\"' || ch == '\'')
                    {
                        qChar = ReadCharFromBuffer();
                    }
                    for (; !eof_;)
                    {
                        ch = PeekCharFromBuffer();
                        if (ch == '>')
                            break;
                        if (!qChar && IsSpaceChar(ch))
                            break;
                        if (qChar && ch == qChar)
                        {
                            ch = PeekNextCharFromBuffer();
                            break;
                        }
                        ch = ReadCharFromBuffer();
                        if (ch)
                            attrvalue += ch;
                        else
                            break;
                    }
                }
                if (possible_capitalized_tags_) {
                    attrns.lowercase();
                    attrname.lowercase();
                }
                if ((flags & TXTFLG_CONVERT_8BIT_ENTITY_ENCODING) && m_conv_table) {
                    PreProcessXmlString(attrvalue, 0, m_conv_table);
                }

                if(in_image && attrname == "src" && !fb2_meta_only)
                {
                    if(attrvalue.startsWith("http://") || attrvalue.startsWith("https://"))
                    {
                        callback_->OnTagClose(L"",L"img");
                        in_image = false;

                        callback_->OnTagOpen(L"",L"a");
                        callback_->OnAttribute(L"",L"href",attrvalue.c_str());
                        callback_->OnAttribute(L"",L"class",L"link_valid");
                        callback_->OnText(attrvalue.c_str(),attrvalue.length(),0);
                        callback_->OnTagClose(L"",L"a");
                        break;
                    }
                }

                if (in_video && attrname == "poster")
                {
                    attrvalue = callback_->convertHref(attrvalue);
                }

                if (attrname == "style")
                {
                    lString16Collection col;
                    col.parse(attrvalue, ' ', true);
                    lString16 stripped;
                    for (int i = 0; i < col.length(); i++)
                    {
                        stripped.append(col.at(i));
                    }
                    stripped.lowercase();

                    if (stripped.pos("display:none") != -1)
                    {
                        callback_->OnAttribute(L"", L"class", L"hidden");
                    }
                }

                //fb3 images
                if(!fb3RelsMap_.empty() && (tagname == "img" || tagname == "image"))
                {
                    if(attrname == "src" && fb3RelsMap_.find(attrvalue.getHash())!=fb3RelsMap_.end())
                    {
                        attrvalue = fb3RelsMap_[attrvalue.getHash()];
                    }
                }

                if(attrname == "class")
                {
                    lString16Collection classes;
                    classes.parse(attrvalue,' ',true);

                    bool bold = false;
                    bool italic = false;
                    bool underline = false;
                    lString16 img_source;
                    for (int i = 0; i < classes.length(); i++)
                    {
                        lString16 name = classes.at(i);
                        lString16 fullname = tagname +  "." + name;
                        if(stylesManager.classExists(fullname))
                        {
                            name = fullname;
                        }
                        if(stylesManager.classExists(name))
                        {
                            CssStyle CssClass = stylesManager.getClass(name);
                            bold = CssClass.isBold();
                            italic = CssClass.isItalic();
                            underline = CssClass.isUnderline();
                            img_source = CssClass.getBackgroundImageSrc();
                            if(!img_source.empty())
                            {
                                callback_->OnTagOpen(L"",L"img");
                                callback_->OnAttribute(L"",L"src",img_source.c_str());
                                callback_->OnTagClose(L"",L"img");
                            }
                        }
                    }
                    lString16 style;
                    if(bold)      style += "font-weight: bold;";
                    if(italic)    style += "font-style: italic;";
                    if(underline) style += "text-decoration: underline;";
                    if(!style.empty())
                    {
                        callback_->OnAttribute(L"", L"style", style.c_str());
                    }
                }

                if(gEmbeddedStylesLVL > 0 && !stylesManager.rtl_map_empty())
                {
                    if(attrname == "class" && stylesManager.ClassIsRTL(attrvalue))
                    {
                        callback_->OnAttribute(L"",L"dir",L"rtl");
                        callback_->setRTLflag(true);
                        gDocumentRTL = 1;
                    }
                }

                if(RTL_DISPLAY_ENABLE && rtl_holder.empty())
                {
                    if(attrname=="dir" || attrname=="direction" || attrname == "class")
                    {
                        if(attrvalue=="rtl")
                        {
                            rtl_holder = tagname;
                            callback_->setRTLflag(true);
                            rtl_holder_counter++;
                            gDocumentRTL = 1;
                        }
                    }
                }

                //epub3
                if(in_section && attrname == "type" && attrvalue == "rearnote")
                {
                    in_rearnote = true;
                }

                if(tagname=="section" && attrname == "type" && attrvalue == "rearnotes")
                {
                    in_rearnotes = true;
                    callback_->OnAttribute(L"",L"class",L"hidden");
                }

                if(attrns == "epub" && attrname == "type" && attrvalue == "noteref")
                {
                    attrns = "";
                    attrvalue = "note";
                }
                if(in_aside)
                {
                    if(attrname == "id")
                    {
                        //CRLog::error("attrval = %d",("#" + callback_->convertId(attrvalue)).getHash());
                        lString16 hrf = "#" + callback_->convertId(attrvalue);
                        Epub3Notes_.AddAside(hrf);
                        //aside_old_id = attrvalue;
                    }
                }
                if(in_rearnote )
                {
                    if(attrname == "id")
                    {
                        lString16 hrf = "#" + callback_->convertId(attrvalue);
                        Epub3Notes_.AddAside(hrf);
                    }
                }

                if(in_body && attrname == "name" && attrvalue == "notes")
                {
                    //CRLog::error("in_body_notes");
                    in_body_notes = true;
                }

                if(in_note_section && attrname == "id")
                {
                    section_id = attrvalue;
                }
                if (in_a || in_note)
                {
                    if ( (attrname == "type" && attrvalue == "note") ||
                         (attrname == "role" && attrvalue == "footnote") )
                    {
                        a_is_fb2_note = true;
                    }
                    if (attrname == "href")
                    {
                        if (in_note && !attrvalue.startsWith("#"))
                        {
                            attrvalue = "#" + attrvalue;
                        }
                        link_href = attrvalue;
                    }
                    if (attrname == "id")
                    {
                        link_id = callback_->convertId(attrvalue);
                    }
                }
                if(in_font && attrname == "size" )
                {
                    if( attrvalue == "6"|| attrvalue == "7")
                    {
                        callback_->OnTagOpen(L"",L"h1");
                        in_font_header1 = true;
                    }
                    else if ( attrvalue == "5")
                    {
                        callback_->OnTagOpen(L"",L"h2");
                        in_font_header2 = true;
                    }
                    else if( attrvalue == "4")
                    {
                        callback_->OnTagOpen(L"",L"h3");
                        in_font_header3 = true;
                    }
                }
                callback_->OnAttribute(attrns.c_str(), attrname.c_str(), attrvalue.c_str());
                if (in_xml_tag && attrname == "encoding")
                {
                    SetCharset(attrvalue.c_str());
                }
                //CRLog::trace("LvXmlParser::Parse() ps_attr ret");
            }
                break;
        case ps_text:
            {
            	//CRLog::trace("LvXmlParser::Parse() ps_text");
//                if ( dumpActive ) {
//                    lString16 s;
//                    s << PeekCharFromBuffer(0) << PeekCharFromBuffer(1) << PeekCharFromBuffer(2) << PeekCharFromBuffer(3)
//                      << PeekCharFromBuffer(4) << PeekCharFromBuffer(5) << PeekCharFromBuffer(6) << PeekCharFromBuffer(7);
//                    CRLog::trace("text: %s...", LCSTR(s) );
//                    dumpActive = true;
//                }
//                txt_count++;
//                if ( txt_count<121 ) {
//                    if ( txt_count>118 ) {
//                        CRLog::trace("Text[%d]:", txt_count);
//                    }
//                }
            if(save_notes_title)
            {
                this->ReadTextToString(Epub3Notes_.FootnotesTitle_,true);
            }
            else if(save_a_content)
            {
                this->ReadTextToString(buffer,true);
                //CRLog::error("buffer = %s",LCSTR(buffer));
            }
            else if( in_head && in_style)
            {
                lString16 cssBufffer;
                this->ReadTextToString(cssBufffer,true);
                EpubStylesManager_.parseString(cssBufffer);
                cssBufffer.clear();
            }
            else
            {
                ReadText();
            }

            fragments_counter++;

	        if(need_coverpage_)
			{
                //CRLog::trace("LvXmlParser: text fragments read : %d", fragments_counter);
				if (fragments_counter >= FIRSTPAGE_BLOCKS_MAX)
				{
					firstpage_thumb_num_reached = true;
				}
			}
			if(callback_->fb2_cover_done())
            {
                firstpage_thumb_num_reached = true;
            }
	            m_state = ps_lt;
                //CRLog::trace("LvXmlParser::Parse() ps_text ret");
            }
            break;
        default:
            {
            }
        }
    }
    callback_->OnStop();
    return !error;
}

void LvXmlParser::initDocxTagsFilter(){
    if(tags_init_){
        return;
    }
    lString16Collection tags;
    tags.add(lString16("proofErr"));
    tags.add(lString16("bcs"));
    tags.add(lString16("ics"));
    tags.add(lString16("lang"));
    tags.add(lString16("highlight"));
    tags.add(lString16("anchor"));
    tags.add(lString16("simplepos"));
    tags.add(lString16("positionh"));
    tags.add(lString16("positionv"));
    tags.add(lString16("extent"));
    tags.add(lString16("effectextent"));
    tags.add(lString16("wraptopandbottom"));
    tags.add(lString16("docpr"));
    tags.add(lString16("graphicframelocks"));
    tags.add(lString16("cnv"));
    tags.add(lString16("cnvpr"));
    tags.add(lString16("cnvpicpr"));
    tags.add(lString16("piclocks"));
    tags.add(lString16("cnvgraphicframepr"));
    tags.add(lString16("graphic"));
    tags.add(lString16("graphicdata"));
    tags.add(lString16("pic"));
    tags.add(lString16("nvpicpr"));
    tags.add(lString16("blipfill"));
    tags.add(lString16("stretch"));
    tags.add(lString16("fillrect"));
    tags.add(lString16("sppr"));
    tags.add(lString16("xfrm"));
    tags.add(lString16("off"));
    tags.add(lString16("ext"));
    tags.add(lString16("prstgeom"));
    tags.add(lString16("avlst"));
    tags.add(lString16("drawing"));
    tags.add(lString16("wrapsquare"));
    tags.add(lString16("spacing"));
    tags.add(lString16("rfonts"));
    tags.add(lString16("sz"));
    tags.add(lString16("szcs"));
    tags.add(lString16("rstyle"));
    tags.add(lString16("noproof"));
    tags.add(lString16("extlst"));
    tags.add(lString16("uselocaldpi"));
    tags.add(lString16("tab"));
    //tags.add(lString16("tcpr"));
    tags.add(lString16("trpr"));
    tags.add(lString16("tblpr"));
    tags.add(lString16("tcw"));
    tags.add(lString16("tcborders"));
    tags.add(lString16("shd"));
    tags.add(lString16("top"));
    tags.add(lString16("left"));
    tags.add(lString16("right"));
    tags.add(lString16("bottom"));
    tags.add(lString16("insideh"));
    tags.add(lString16("insidev"));
    tags.add(lString16("tblgrid"));
    tags.add(lString16("gridcol"));
    tags.add(lString16("tblw"));
    tags.add(lString16("tblind"));
    tags.add(lString16("tblborders"));
    tags.add(lString16("tbllayout"));
    tags.add(lString16("tbllook"));
    //tags.add(lString16("jc"));
    tags.add(lString16("ind"));
    tags.add(lString16("numpr"));
    tags.add(lString16("tblcellmar"));
    tags.add(lString16("prooferr"));
    tags.add(lString16("separator"));
    tags.add(lString16("continuationseparator"));
    tags.add(lString16("autospacede"));
    tags.add(lString16("autospacedn"));
    tags.add(lString16("adjustrightind"));
    tags.add(lString16("formulas"));
    tags.add(lString16("f"));
    tags.add(lString16("stroke"));
    tags.add(lString16("shapetype"));
    tags.add(lString16("object"));
    tags.add(lString16("path"));
    tags.add(lString16("lock"));
    tags.add(lString16("shape"));
    tags.add(lString16("oleobject"));
    tags.add(lString16("bookmarkend"));
    tags.add(lString16("contextualspacing"));
    tags.add(lString16("color"));
    tags.add(lString16("imagedata"));
    tags.add(lString16("tabs"));
    tags.add(lString16("keepnext"));
    tags.add(lString16("keeplines"));
    tags.add(lString16("widowcontrol"));
    tags.add(lString16("snaptogrid"));
    tags.add(lString16("nofill"));
    tags.add(lString16("inline"));
    tags.add(lString16("ln"));
    tags.add(lString16("srcrect"));
    tags.add(lString16("lastrenderedpagebreak"));
    //tags.add(lString16("vmerge"));
    tags.add(lString16("style"));
    tags.add(lString16("sdtpr"));
    tags.add(lString16("docpartobj"));
    tags.add(lString16("docpartgallery"));
    tags.add(lString16("docpartunique"));
    tags.add(lString16("sdtendpr"));
    tags.add(lString16("fldchar"));
    tags.add(lString16("sdtendpr"));
    tags.add(lString16("sdt"));
    tags.add(lString16("id"));
    tags.add(lString16("caps"));
    tags.add(lString16("strike"));
    tags.add(lString16("pbdr"));
    tags.add(lString16("bdr"));
    tags.add(lString16("tblstyle"));
    tags.add(lString16("tblppr"));
    tags.add(lString16("tbloverlap"));
    tags.add(lString16("cnfstyle"));
    tags.add(lString16("nowrap"));
    tags.add(lString16("wafter"));
    tags.add(lString16("trheight"));
    //tags.add(lString16("gridspan"));
    tags.add(lString16("tcmar"));
    tags.add(lString16("valign"));
    tags.add(lString16("framepr"));
    tags.add(lString16("textalignment"));
    tags.add(lString16("position"));
    tags.add(lString16("sectpr"));
    tags.add(lString16("pgsz"));
    tags.add(lString16("pgmar"));
    tags.add(lString16("cols"));
    tags.add(lString16("docgrid"));
    tags.add(lString16("outlinelvl"));
    tags.add(lString16("suppressautohyphens"));
    tags.add(lString16("sizerelh"));
    tags.add(lString16("sizerelv"));
    //tags.add(lString16("pctheight"));
    //tags.add(lString16("pctwidth"));
    tags.add(lString16("miter"));
    tags.add(lString16("headend"));
    tags.add(lString16("tailend"));
    tags.add(lString16("formprot"));
    tags.add(lString16("hidemark"));
    tags.add(lString16("tblcellspacing"));
    tags.add(lString16("kern"));
    tags.add(lString16("ssub"));
    tags.add(lString16("ssubpr"));
    tags.add(lString16("ctrlpr"));
    tags.add(lString16("e"));
    tags.add(lString16("sty"));
    tags.add(lString16("fpr"));
    tags.add(lString16("omath"));
    //tags.add(lString16("omathpara"));
    tags.add(lString16("den"));
    tags.add(lString16("num"));
    tags.add(lString16("pict"));
    tags.add(lString16("fill"));
    tags.add(lString16("rect"));
    tags.add(lString16("textbox"));
    tags.add(lString16("txbxcontent"));
    tags.add(lString16("ssup"));
    tags.add(lString16("ssuppr"));
    tags.add(lString16("dpr"));
    tags.add(lString16("m"));
    tags.add(lString16("mpr"));
    tags.add(lString16("mcs"));
    tags.add(lString16("mc"));
    tags.add(lString16("mcpr"));
    tags.add(lString16("count"));
    tags.add(lString16("mcjc"));
    tags.add(lString16("mr"));
    //tags.add(lString16("alternatecontent"));
    tags.add(lString16("choice"));
    tags.add(lString16("wrapnone"));
    tags.add(lString16("wsp"));
    tags.add(lString16("cnvsppr"));
    tags.add(lString16("splocks"));
    tags.add(lString16("solidfill"));
    tags.add(lString16("srgbclr"));
    tags.add(lString16("hiddenline"));
    tags.add(lString16("bodypr"));
    tags.add(lString16("noautofit"));
    tags.add(lString16("choice"));
    tags.add(lString16("fallback"));
    tags.add(lString16("shadowobscured"));
    tags.add(lString16("cnvgrpsppr"));
    tags.add(lString16("grpsppr"));
    tags.add(lString16("choff"));
    tags.add(lString16("chext"));
    tags.add(lString16("cnvcnpr"));
    tags.add(lString16("schemeclr"));
    tags.add(lString16("lnref"));
    tags.add(lString16("fillref"));
    tags.add(lString16("effectref"));
    tags.add(lString16("fontref"));
    tags.add(lString16("txbx"));
    tags.add(lString16("grpsp"));
    tags.add(lString16("line"));
    tags.add(lString16("wgp"));
    tags.add(lString16("anchorlock"));
    tags.add(lString16("document"));
    tags.add(lString16("xml"));
    tags.add(lString16("footnotes"));
    tags.add(lString16("smallcaps"));
    tags.add(lString16("rtl"));
    tags.add(lString16("softhyphen"));
    for (int i = 0; i < tags.length(); i++)
    {
        m_[tags.at(i).getHash()] = 1;
    }
    tags.clear();
    tags_init_ = true;
    CRLog::trace("DOCX tag filtering array initialized");
    return;
}

bool LvXmlParser::docxTagAllowed(lString16 tagname){
    if(!tags_init_){
        initDocxTagsFilter();
    }
    iter it;
    it = m_.find(tagname.getHash());
    if (it != m_.end())
    {
        return false;
    }
    return true;
}

bool LvXmlParser::ParseDocx(DocxItems docxItems, DocxLinks docxLinks, DocxStyles docxStyles)
{
    Reset();
    callback_->OnStart(this);
    //int txt_count = 0;
    int flags = callback_->getFlags();
    bool error = false;
    bool in_xml_tag = false;
    bool close_flag = false;
    bool q_flag = false;
    bool body_started = false;
    bool in_blockquote = false;
    bool firstpage_thumb_num_reached = false;
    bool last_tag_was_t = false;

    int fragments_counter = 0;
    lString16 tagname;
    lString16 tagns;
    lString16 attrname;
    lString16 attrns;
    lString16 attrvalue;

    //flag for image node
    bool in_blip_img = false;
    //flag for run properties
    bool in_rpr = false;
    //flag for run node
    bool in_r = false;
    //flag for paragraph properties
    bool in_ppr = false;
    //flag for paragrahh style
    bool in_pstyle = false;
    //flag for text node
    bool in_t = false;
    //flag for headers
    bool in_header= false;
    //flags for tables
    bool in_table= false;
    //flag for footnotes
    bool in_footnoteref= false;
    bool in_footnote = false;
    //flag for hyperlinks
    bool in_a = false;
    //flags for table of contents
    bool in_tocref = false;
    bool in_toc = false;
    bool in_sdt_a = false;
    //flags for pagebreaks
    bool allow_footnote_pbr= false;
    bool just_r= false;

    //flags for bold italic underline
    bool rpr_b=false;
    bool rpr_i=false;
    bool rpr_u=false;
    //flags for no element drawing
    bool rpr_webhidden=false;
    bool nodraw = false;
    bool nodraw_group = false;
    //flags for superscript and subscript
    bool rpr_vertalign=false;
    bool rpr_superscript = false;
    bool rpr_subscript = false;
    //flag for lists
    bool ilvl=false;
    //flag for no attributes drawing
    bool noattrib = false;

    int pstyle_value= 0;

    bool in_p = false;
    bool been_in_t = false;

    bool first_bilp_in_p = false;
    bool separate_img = false;

    bool in_td = false;
    bool in_tcpr = false;
    bool flag_colspan = false;
    lString16 colspan_val;

    int empty_p_counter = 0;

    ldomNode* curr_p = NULL;

    bool save_text = false;
    lString16 str_buffer;
    lString16 a_href;

    int default_size = docxStyles.default_size_;
    int h1min = docxStyles.h1min_;
    int h2min = docxStyles.h2min_;
    int h3min = docxStyles.h3min_;
    int h4min = docxStyles.h4min_;
    int h5min = docxStyles.h5min_;
    int h6min = docxStyles.h6min_;

    Headermap headermap;
    headermap[docxStyles.h1id_.getHash()] = 1;
    headermap[docxStyles.h2id_.getHash()] = 2;
    headermap[docxStyles.h3id_.getHash()] = 3;
    headermap[docxStyles.h4id_.getHash()] = 4;
    headermap[docxStyles.h5id_.getHash()] = 5;
    headermap[docxStyles.h6id_.getHash()] = 6;

    LinksMap LinksMap = LinksMap_;
    for (; !eof_ && !error && !firstpage_thumb_num_reached ;)
    {
        if (m_stopped)
            break;
        // Load next portion of data if necessary
        lChar16 ch = PeekCharFromBuffer();
        switch (m_state)
        {
            case ps_bof:
            {
                //CRLog::trace("LvXmlParser::Parse() ps_bof");
                // Skip file beginning until '<'
                for ( ; !eof_ && ch!='<'; ch = PeekNextCharFromBuffer())
                    ;
                if (!eof_)
                {
                    m_state = ps_lt;
                    ReadCharFromBuffer();
                }
                //CRLog::trace("LvXmlParser::Parse() ps_bof ret");
            }
                break;
            case ps_lt:  //look for tags
            {
                //CRLog::trace("LvXmlParser::Parse() ps_lt");
                if (!SkipSpaces())
                    break;
                close_flag = false;
                q_flag = false;
                if (ch=='/')
                {
                    ch = ReadCharFromBuffer();
                    close_flag = true;
                }
                else if (ch=='?')
                {
                    // <?xml?>
                    ch = ReadCharFromBuffer();
                    q_flag = true;
                }
                else if (ch=='!')
                {
                    // comments etc...
                    if (PeekCharFromBuffer(1) == '-' && PeekCharFromBuffer(2) == '-') {
                        // skip comments
                        ch = PeekNextCharFromBuffer(2);
                        while (!eof_ && (ch != '-' || PeekCharFromBuffer(1) != '-'
                                         || PeekCharFromBuffer(2) != '>') ) {
                            ch = PeekNextCharFromBuffer();
                        }
                        if (ch=='-' && PeekCharFromBuffer(1)=='-'
                            && PeekCharFromBuffer(2)=='>' )
                            ch = PeekNextCharFromBuffer(2);
                        m_state = ps_text;
                        break;
                    }
                }
                if (!ReadIdent(tagns, tagname) || PeekCharFromBuffer()=='=')
                {
                    // Error
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }
                tagns.lowercase();
                tagname.lowercase();
                //CRLog::error("%s:%s",LCSTR(tagns),LCSTR(tagname));

                tagns = "";

                //removing OpenXML tags from tree
                if(!docxTagAllowed(tagname))
                {
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }
                if(tagname == "bidi")
                {
                    gDocumentRTL = true;
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }

                if(tagname == "br")
                {
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    callback_->OnTagOpenAndClose(L"", L"br");
                    break;
                }

                //<r> (runs) handling
                if(tagname == "r")
                {
                    first_bilp_in_p = false;
                    if(!close_flag)
                    {
                        in_r = true;
                        just_r = false;
                    }
                    if(close_flag)
                    {
                        just_r = true;
                        if(nodraw)
                        {
                            callback_->OnTagClose(L"", lString16("span").c_str());
                        }
                        in_r = false;
                        nodraw = false;
                    }
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }
                //removing bookmarkstart that appears right after run
                //seems no need in this filtering
                /*if(just_r)
                {
                    if(tagname == "bookmarkstart")
                    {
                        if (SkipTillChar('>'))
                        {
                            m_state = ps_text;
                            ch = ReadCharFromBuffer();
                        }
                        break;
                    }
                    else
                    {
                        just_r = false;
                    }
                }*/
                //remove runs that contain webhidden tag
                if (in_r && tagname == "webhidden")
                {
                    nodraw = true;
                    callback_->OnTagClose(L"", lString16("a").c_str());
                    callback_->OnTagOpen(L"", lString16("span").c_str());
                    callback_->OnAttribute(L"", lString16("class").c_str(), L"hidden");
                    break;
                }

                if ((tagname == "pctwidth" || tagname == "pctheight") && !close_flag)
                {
                    callback_->OnTagOpen(L"", lString16("span").c_str());
                    callback_->OnAttribute(L"", lString16("class").c_str(), L"hidden");
                    break;
                }
                if ((tagname == "pctwidth" || tagname == "pctheight") && close_flag)
                {
                    callback_->OnTagClose(L"", lString16("span").c_str());
                    break;
                }

                //table handling
                if (tagname == "tbl" && !close_flag)
                {
                    tagname = "table";
                    in_table = true;
                }

                if (tagname == "tbl" && close_flag)
                {
                    tagname = "table";
                    in_table = false;
                }

                if (in_table)
                {
                    if (tagname == "pstyle" || tagname == "p")
                    {
                        if (SkipTillChar('>'))
                        {
                            m_state = ps_text;
                            ch = ReadCharFromBuffer();
                        }
                        break;
                    }
                }

                if (tagname == "tc")
                {
                    tagname = "td";
                }

                if (tagname == "td" && !close_flag)
                {
                    in_td = true;
                }

                if (tagname == "td" && close_flag)
                {
                    in_td = false;
                    if(flag_colspan)
                    {
                        //CRLog::error("flag_colspan, val = %s ",LCSTR(colspan_val));
                        callback_->OnAttribute(L"",L"colspan",colspan_val.c_str());
                        flag_colspan = false;
                    }
                }

                if(in_td)
                {
                    if(tagname == "tcpr" && !close_flag)
                    {
                        in_tcpr = true;
                    }
                    if(tagname == "tcpr" && close_flag)
                    {
                        in_tcpr = false;
                    }
                }

                //footnotes handling

                if (tagname == "footnotereference")
                {
                    in_footnoteref = true;
                    tagname = "a";
                    m_state = ps_attr;
                }

                if (tagname == "footnote" && !close_flag)
                {
                    tagname = "section";
                    in_footnote = true;
                    m_state = ps_attr;
                    //break;
                }
                if (tagname == "footnote" && close_flag && in_footnote)
                {
                    tagname = "section";
                    allow_footnote_pbr = true;
                }

                //bold italic underlined text handling
                if (in_rpr)
                {
                    bool remove = false;
                    if (tagname == "b")
                    {
                        //CRLog::error(" b closeflag = %d",close_flag?1:0);
                        //callback_->OnTagOpen(L"",L"span");
                        rpr_b = true;
                        remove = true;
                    }
                    if (tagname == "u")
                    {
                        rpr_u = true;
                        remove = true;
                    }
                    if (tagname == "i")
                    {
                        rpr_i = true;
                        remove = true;
                    }
                    if (tagname == "vertalign")
                    {
                        rpr_vertalign = true;
                        m_state = ps_attr;
                        break;
                    }

                    //if(remove)
                    //{
                    //    if (SkipTillChar('>'))
                    //    {
                    //        m_state = ps_text;
                    //        ch = ReadCharFromBuffer();
                    //    }
                    //    break;
                    //}
                }

                    //header styles handling
                if (tagname == "ppr" && !close_flag)
                {
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    in_ppr = true;
                    break;
                }
                if (tagname == "ppr" && close_flag)
                {
                    in_ppr = false;
                }

                if( in_ppr)
                {
                    if (tagname == "pstyle")
                    {
                        in_pstyle = true;
                        m_state = ps_attr;
                        break;
                    }
                }

                if (tagname == "rpr" && !close_flag)
                {
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    in_rpr = true;
                    break;
                }
                if (tagname == "rpr" && close_flag)
                {
                    in_rpr = false;
                }

                //image embedding handling
                if (tagname == "blip")
                {
                    tagname = "img";
                    in_blip_img = true;
                    if (in_r && in_p)
                    {
                        if (!first_bilp_in_p)
                        {
                            callback_->OnTagClose(L"",L"p");
                            callback_->OnAttribute(L"",L"class",L"section_image");
                            separate_img = true;
                        }
                        first_bilp_in_p = true;
                    }
                }
                //different kind of filtering
                if (tagname == "posoffset"|| tagname == "align" )
                {
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_lt;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }

                //unmarked lists handling
                if (tagname == "ilvl")
                {
                    ilvl = true;
                }

                if(tagname == "p" && !close_flag)
                {
                    in_p = true;
                }
                //empty paragraphs handling
                if(tagname == "p" && close_flag)
                {
                    if (!been_in_t && empty_p_counter < 3)
                    {
                        callback_->OnText(L"\u200B", 1, flags);
                        //callback_->OnText(L"+",1,flags);
                        empty_p_counter++;
                    }
                    been_in_t = false;
                    in_p = false;
                    first_bilp_in_p = false;
                    rpr_b =false;
                    rpr_i =false;
                    rpr_u =false;

                }

                if(tagname == "t")
                {
                    m_state = ps_attr;
                    if(ilvl)
                    {
                        callback_->OnTagOpen(L"", lString16("li").c_str());
                        ilvl = false;
                    }
                    empty_p_counter =0;
                    been_in_t = true;
                    in_t = true;
                    break;
                }
                if(tagname == "t" && close_flag)
                {
                    rpr_b = false;
                    rpr_i = false;
                    rpr_u = false;
                    rpr_subscript = false;
                    rpr_superscript = false;
                    in_t = false;
                }
                //pagebreaks handling
                if(tagname=="pagebreak")
                {
                    tagns = "";
                    if (close_flag)
                    {
                        callback_->OnText(L"\u200B", 1, flags);
                    }
                }
                //hyperlinks handling
                if (tagname == "hyperlink")
                {
                    tagname = "a";
                    if(!close_flag)
                    {
                        save_text = true;
                        in_a = true;
                    }
                    if(close_flag)
                    {
                        if(!a_href.empty())
                        {
                            callback_->OnAttribute(L"",L"class",L"link_valid");
                        }
                        if(a_href.pos("://")==-1 && str_buffer.pos("://")!=-1)
                        {
                            //trimming spaces in href
                            while(str_buffer.startsWith(" "))
                            {
                                str_buffer = str_buffer.substr(1);
                            }
                            while (str_buffer.endsWith(" "))
                            {
                                str_buffer = str_buffer.substr(0,str_buffer.length()-1);
                            }
                            callback_->OnAttribute(L"",L"href",str_buffer.c_str());
                        }
                        str_buffer = lString16::empty_str;
                        save_text = false;
                        in_a = false;
                    }
                }

                if(tagname == "a")
                {
                    in_sdt_a = true;
                    if(close_flag)
                    {
                        callback_->OnAttribute(L"",L"class",L"link_valid");
                        save_text = false;
                    }
                }
                //another filtering
                if(tagname== "instrtext")
                {
                    if (SkipTillChar('<'))
                    {
                        m_state = ps_text;
                        //ch = ReadCharFromBuffer();
                    }
                    break;
                }
                //bookmarks handling
                if(tagname== "bookmarkstart")
                {
                    in_tocref = true;
                }
                if (tagname == "group"
                    || tagname == "omathpara"
                    || tagname == "alternatecontent")
                {
                    nodraw_group = true;
                    tagname = "img";
                }
                //all other closing tags handling
                if (close_flag)
                {
                    callback_->OnTagClose(tagns.c_str(), tagname.c_str());
                    //CRLog::trace("</%s:%s>", LCSTR(tagns),LCSTR(tagname));
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }

                if (q_flag) {
                    tagname.insert(0, 1, '?');
                    in_xml_tag = (tagname == "?xml");
                } else {
                    in_xml_tag = false;
                }

                if(tagname == "jc")
                {
                    m_state = ps_attr;
                    break;
                }

                if(!(rpr_b || rpr_i || rpr_u ))
                {
                    if(tagname == "p")
                    {
                        curr_p = callback_->OnTagOpen(tagns.c_str(), tagname.c_str());
                    }
                    else
                    {
                        callback_->OnTagOpen(tagns.c_str(), tagname.c_str());
                    }
                }
                //CRLog::trace("<%s:%s>", LCSTR(tagns),LCSTR(tagname));

                m_state = ps_attr;
                //CRLog::trace("LvXmlParser::Parse() ps_lt ret");
            }
                break;
            case ps_attr: //read tags
            {
                //CRLog::trace("LvXmlParser::Parse() ps_attr");
                if (!SkipSpaces())
                    break;
                ch = PeekCharFromBuffer();
                lChar16 nch = PeekCharFromBuffer(1);
                if (ch == '>' || ((ch == '/' || ch == '?') && nch == '>'))
                {
                    callback_->OnTagBody();
                    // end of tag
                    if (ch != '>')
                        callback_->OnTagClose(tagns.c_str(), tagname.c_str());
                    if (ch == '>')
                        ch = PeekNextCharFromBuffer();
                    else
                        ch = PeekNextCharFromBuffer(1);
                    m_state = ps_text;
                    break;
                }
                if (!ReadIdent(attrns, attrname))
                {
                    // error: skip rest of tag
                    SkipTillChar('<');
                    ch = PeekNextCharFromBuffer(1);
                    callback_->OnTagBody();
                    m_state = ps_lt;
                    break;
                }
                SkipSpaces();
                attrvalue.reset(16);
                ch = PeekCharFromBuffer();
                // Read attribute value
                if (ch == '=')
                {
                    // Skip '='
                    ReadCharFromBuffer();
                    SkipSpaces();
                    lChar16 qChar = 0;
                    ch = PeekCharFromBuffer();
                    if (ch == '\"' || ch == '\'')
                    {
                        qChar = ReadCharFromBuffer();
                    }
                    for (; !eof_;)
                    {
                        ch = PeekCharFromBuffer();
                        if (ch == '>')
                            break;
                        if (!qChar && IsSpaceChar(ch))
                            break;
                        if (qChar && ch == qChar)
                        {
                            ch = PeekNextCharFromBuffer();
                            break;
                        }
                        ch = ReadCharFromBuffer();
                        if (ch)
                            attrvalue += ch;
                        else
                            break;
                    }
                }
                if (possible_capitalized_tags_) {
                    attrns.lowercase();
                    attrname.lowercase();
                }
                attrns = "";
                //attribute filtering
                if (tagname == "document"
                    || tagname == "body"
                    || tagname == "footnotes"
                    || attrname == "rsidr"
                    || attrname == "rsidrdefault"
                    || attrname == "rsidp"
                    || attrname == "rsidrpr"
                    || attrname == "space"
                    || attrname == "gfxdata")
                {
                    noattrib = true;
                }
                else
                {
                    noattrib = false;
                }

                //colspan handling
                if (in_ppr && tagname == "jc") //jc is self-closed, so we can't look at close_flag
                {
                    if(attrname == "val" && curr_p != NULL)
                    {
                        if (attrvalue == "start" || attrvalue == "left")
                        {
                            curr_p->setAttribute(L"", L"class", L"left");
                        }
                        else if (attrvalue == "end" || attrvalue == "right")
                        {
                            curr_p->setAttribute(L"", L"class", L"right");
                        }
                        else if (attrvalue == "both" || attrvalue == "distribute")
                        {
                            curr_p->setAttribute(L"", L"class", L"justify");
                        }
                        else if (attrvalue == "center")
                        {
                            curr_p->setAttribute(L"", L"class", L"center");
                        }
                        break;
                    }
                }

                if(in_tcpr)
                {
                    if(tagname == "gridspan" && attrname == "val")
                    {
                        colspan_val = attrvalue;
                        //CRLog::error("in_gridspan , [val] = [%s]",LCSTR(colspan_val));
                        flag_colspan = true;
                    }
                }
                //header style handling
                if(in_pstyle)
                {
                    if(attrname == "val")
                    {
                        in_header = true;
                        if(headermap.find(attrvalue.getHash())!=headermap.end())
                        {
                            pstyle_value = headermap[attrvalue.getHash()];
                            //CRLog::error("pstyle = %d",pstyle_value);
                        }
                        else
                        {
                            int currfontsize = docxStyles.getSizeById(attrvalue);
                            if(currfontsize > default_size)
                            {
                                if ( currfontsize >  h6min )  pstyle_value = 6;
                                if ( currfontsize >= h5min )  pstyle_value = 5;
                                if ( currfontsize >= h4min )  pstyle_value = 4;
                                if ( currfontsize >= h3min )  pstyle_value = 3;
                                if ( currfontsize >= h2min )  pstyle_value = 2;
                                if ( currfontsize >= h1min )  pstyle_value = 1;
                            }
                            else if(currfontsize <= default_size)
                            {
                                pstyle_value = -1;
                            }
                            //CRLog::error("pstyle generated = %d",pstyle_value);
                        }
                        // can be val="Normal"
                    }
                    in_pstyle = false;
                }
                //footnote handling
                if(in_footnoteref)
                {
                    callback_->OnAttribute(L"",L"class",L"link_valid");
                    callback_->OnAttribute(attrns.c_str(), lString16("type").c_str(), lString16("note").c_str());
                    lString16 nref = lString16("#") + attrvalue + lString16("_note");
                    lString16 id   = attrvalue + lString16("_back");
                    lString16 href = lString16("#") + attrvalue;
                    callback_->OnAttribute(attrns.c_str(), lString16("nref").c_str(), nref.c_str());
                    callback_->OnAttribute(attrns.c_str(), lString16("id").c_str(), id.c_str());
                    lString16 mark = "[" + attrvalue + "]";
                    callback_->OnText(mark.c_str(), mark.length(),0);
                    attrname= "href";
                    LinksList_.add(LinkStruct(attrvalue.atoi(),id,href));
                    LinksMap_[href.getHash()] = id;
                    //CRLog::error("linksmap add = %s %s", LCSTR(href),LCSTR(id));
                    attrvalue = href;
                    in_footnoteref = false;
                }
                if(in_footnote){
                    if(attrname == "id")
                    {
                        lString16 currlink_id;
                        if(LinksMap.size() !=0)
                        {
                            if(LinksMap.find((lString16("#") + attrvalue).getHash())!=LinksMap.end())
                            {
                                currlink_id = LinksMap[(lString16("#") + attrvalue).getHash()];
                                //CRLog::error("found [%s] at [%s]",LCSTR(currlink_id),LCSTR(temp_section_id));
                                currlink_id = lString16("#") + currlink_id ;
                            }
                        }
                        if(!currlink_id.empty())
                        {
                            callback_->OnTagOpen(L"", L"title");
                            callback_->OnTagOpen(L"", L"p");
                            callback_->OnTagOpen(L"", L"a");
                            callback_->OnAttribute(L"",L"class",L"link_valid");
                            callback_->OnAttribute(L"",L"href",(lString16("~") + currlink_id).c_str());
                            callback_->OnText(attrvalue.c_str(), attrvalue.length(), flags);
                            callback_->OnTagClose(L"", L"a");
                            callback_->OnTagClose(L"", L"p");
                            callback_->OnTagClose(L"", L"title");
                        }
                        else
                        {
                            //callback_->OnTagOpen(L"", L"title");
                            //callback_->OnTagOpen(L"", L"p");
                            //callback_->OnText(attrvalue.c_str(), attrvalue.length(), flags);
                            //callback_->OnTagClose(L"", L"p");
                            //callback_->OnTagClose(L"", L"title");
                        }

                    }
                    if(attrname == "type" && (attrvalue == "separator" || attrvalue =="continuationSeparator"))
                    {
                        callback_->OnTagClose(L"",L"section");
                        in_footnote = false;
                        allow_footnote_pbr = false;
                        break;
                    }
                }
                //embedded image handling
                if(in_blip_img)
                {
                    if (attrname == "embed")
                    {
                        attrname = "src";
                        lString16 rID = attrvalue;
                        attrvalue = docxItems.findHrefById(rID);
                    }
                    in_blip_img =false;
                    if(separate_img)
                    {
                        callback_->OnTagClose(L"",L"p");
                    }
                }

                //hyperlinks handling
                if(in_a)
                {
                    if (attrname == "id")
                    {
                        attrvalue = docxLinks.findTargetById(attrvalue);
                        a_href=attrvalue;
                        attrname = "href";
                    }
                    //in_a = false;
                }
                //toc handling
                if(in_tocref)
                {
                    if(attrname == "id")
                    {
                        m_state = ps_attr;
                        break;
                    }
                    else if (attrname == "name")
                    {
                        attrname = "id";
                    }
                }
                if(in_sdt_a)
                {
                    if(attrname == "anchor")
                    {
                        attrname = "href";
                        attrvalue = lString16("#") + attrvalue;
                    }
                    in_sdt_a = false;
                }
                //superscript and subscript handling
                if(rpr_vertalign)
                {
                    if (attrname == "val" && attrvalue == "superscript")
                    {
                        rpr_superscript = true;
                        rpr_subscript = false;
                    }
                    else if (attrname == "val" && attrvalue == "subscript")
                    {
                        rpr_subscript = true;
                        rpr_superscript = false;
                    }
                    rpr_vertalign = false;
                }
                if (rpr_b && tagname == "b" && attrname == "val" && (attrvalue == "0" || attrvalue == "none"))
                {
                    rpr_b = false;
                }
                if (rpr_i && tagname == "i" && attrname == "val" && (attrvalue == "0" || attrvalue == "none"))
                {
                    rpr_i = false;
                }
                if (rpr_u && tagname == "u" && attrname == "val" && (attrvalue == "0" || attrvalue == "none"))
                {
                    rpr_u = false;
                }
                //<group> removing procedure
                if(nodraw_group)
                {
                    callback_->OnAttribute(attrns.c_str(),lString16("src").c_str(), lString16("Intentional_error").c_str());
                    nodraw_group = false;
                }
                if ((flags & TXTFLG_CONVERT_8BIT_ENTITY_ENCODING) && m_conv_table) {
                    PreProcessXmlString(attrvalue, 0, m_conv_table);
                }
                if(!noattrib)
                {
                    //CRLog::error("OnAttrib [%s:%s = \"%s\"]",LCSTR(attrns), LCSTR(attrname), LCSTR(attrvalue));
                    callback_->OnAttribute(attrns.c_str(), attrname.c_str(), attrvalue.c_str());
                }
                if(separate_img)
                {
                    callback_->OnAttribute(L"",L"class",L"section_image");
                    separate_img = false;
                }
                if (in_xml_tag && attrname == "encoding")
                {
                    SetCharset(attrvalue.c_str());
                }
                //CRLog::trace("LvXmlParser::Parse() ps_attr ret");
            }
                break;
            case ps_text:
            {
                //footnotes page-to-page separation handling
                if (allow_footnote_pbr)
                {
                    //callback_->OnTagOpen(L"", L"pagebreak");
                    //callback_->OnText(L"\u200B", 1, flags);
                    //callback_->OnTagClose(L"", L"pagebreak");

                    in_footnote = false;
                    allow_footnote_pbr = false;
                }

                //bold italic underline list tag insertion
                if (in_t)
                {
                    if(in_header)
                    {
                        switch (pstyle_value){
                            case 1:
                                callback_->OnTagOpen(L"", lString16("h1").c_str());
                                break;
                            case 2:
                                callback_->OnTagOpen(L"", lString16("h2").c_str());
                                break;
                            case 3:
                                callback_->OnTagOpen(L"", lString16("h3").c_str());
                                break;
                            case 4:
                                callback_->OnTagOpen(L"", lString16("h4").c_str());
                                break;
                            case 5:
                                callback_->OnTagOpen(L"", lString16("h5").c_str());
                                break;
                            case 6:
                                //callback_->OnTagOpen(L"", lString16("h6").c_str());
                                break;
                            default:
                                //val="Normal"
                                in_header = false;
                                break;
                        }
                        //in_header = false;
                        //pstyle_value = 0;
                    }
                    if (rpr_b)
                    {
                        callback_->OnTagOpen(L"", lString16("b").c_str());
                    }
                    if (rpr_i)
                    {
                        callback_->OnTagOpen(L"", lString16("i").c_str());
                    }
                    if (rpr_u)
                    {
                        callback_->OnTagOpen(L"", lString16("u").c_str());
                    }
                    if (rpr_superscript)
                    {
                        callback_->OnTagOpen(L"", lString16("sup").c_str());
                    }
                    else if (rpr_subscript)
                    {
                        callback_->OnTagOpen(L"", lString16("sub").c_str());
                    }
                }
                if(save_text)
                {
                    this->ReadTextToString(str_buffer,true,true);
                }
                else
                {
                    lString16 temp;
                    this->ReadTextToString(temp,true,true);
                    temp.clear();
                }
                fragments_counter++;
                //bold italic underline list tag insertion closing tags
                if (in_t)
                {
                    if (rpr_u)
                    {
                        callback_->OnTagClose(L"", lString16("u").c_str());
                        rpr_u = false;
                    }
                    if (rpr_i)
                    {
                        callback_->OnTagClose(L"", lString16("i").c_str());
                        rpr_i = false;
                    }
                    if (rpr_b)
                    {
                        callback_->OnTagClose(L"", lString16("b").c_str());
                        rpr_b = false;
                    }

                    if (rpr_superscript)
                    {
                        callback_->OnTagClose(L"", lString16("sup").c_str());
                        rpr_superscript=false;
                    }
                    else if (rpr_subscript)
                    {
                        callback_->OnTagClose(L"", lString16("sub").c_str());
                        rpr_subscript=false;
                    }

                    if(in_header)
                    {
                        switch (pstyle_value)
                        {
                            case 1:
                                callback_->OnTagClose(L"", lString16("h1").c_str());
                                break;
                            case 2:
                                callback_->OnTagClose(L"", lString16("h2").c_str());
                                break;
                            case 3:
                                callback_->OnTagClose(L"", lString16("h3").c_str());
                                break;
                            case 4:
                                callback_->OnTagClose(L"", lString16("h4").c_str());
                                break;
                            case 5:
                                callback_->OnTagClose(L"", lString16("h5").c_str());
                                break;
                            case 6:
                                callback_->OnTagClose(L"", lString16("h6").c_str());
                                break;
                            default:
                                //val="Normal"
                                in_header = false;
                                break;
                        }
                        in_header = false;
                        pstyle_value = 0;
                    }
                    in_t = false;
                }

                if(need_coverpage_)
                {
                    //CRLog::trace("LvXmlParser: text fragments read : %d", fragments_counter);
                    if (fragments_counter >= FIRSTPAGE_BLOCKS_MAX_DOCX)
                    {
                        firstpage_thumb_num_reached = true;
                    }
                }
                m_state = ps_lt;
            }
                break;
            default:
            {
            }
        }
    }
    callback_->OnStop();
    return !error;
}


void LvXmlParser::initOdtTagsFilter(){
    if(tags_init_){
        return;
    }
    lString16Collection tags;

    tags.add(lString16("table-columns"));
    tags.add(lString16("table-column"));
    tags.add(lString16("frame"));
    tags.add(lString16("title"));
    tags.add(lString16("desc"));
    tags.add(lString16("font-face-decls"));
    tags.add(lString16("font-face"));
    //tags.add(lString16("style"));
    tags.add(lString16("automatic-styles"));
    tags.add(lString16("list-level-properties"));
    tags.add(lString16("list-level-label-alignment"));
    tags.add(lString16("paragraph-properties"));
    tags.add(lString16("tab-stops"));
    tags.add(lString16("tab-stop"));
    //tags.add(lString16("text-properties"));
    tags.add(lString16("table-properties"));
    tags.add(lString16("table-row-properties"));
    tags.add(lString16("table-column-properties"));
    tags.add(lString16("table-cell-properties"));
    tags.add(lString16("graphic-properties"));
    //tags.add(lString16("table-of-content"));
    tags.add(lString16("index-body"));
    tags.add(lString16("tab"));
    tags.add(lString16("table-of-content-source"));
    tags.add(lString16("table-of-content-entry-template"));
    tags.add(lString16("index-entry-link-start"));
    tags.add(lString16("index-entry-text"));
    tags.add(lString16("index-entry-tab-stop"));
    tags.add(lString16("index-entry-page-number"));
    tags.add(lString16("index-entry-link-end"));
    //tags.add(lString16("note"));
    tags.add(lString16("s"));
    tags.add(lString16("text"));
    //tags.add(lString16("span"));
    tags.add(lString16("document-content"));
    tags.add(lString16("bookmark-start"));
    tags.add(lString16("bookmark-end"));
    tags.add(lString16("table-header-rows"));
    tags.add(lString16("scripts"));
    tags.add(lString16("sequence-decls"));
    tags.add(lString16("sequence-decl"));

    for (int i = 0; i < tags.length(); i++)
    {
        m_[tags.at(i).getHash()] = 1;
    }
    tags.clear();
    tags_init_ = true;
    CRLog::trace("ODT tag filtering array initialized");
    return;
}

bool LvXmlParser::odtTagAllowed(lString16 tagname){
    if(!tags_init_){
        initOdtTagsFilter();
    }
    iter it;
    it = m_.find(tagname.getHash());
    if (it != m_.end())
    {
        return false;
    }
    return true;
}

class OdtTextStyle
{
private:
    bool b_ = false;
    bool i_ = false;
    bool u_ = false;
    bool nodraw_ = false;
    bool sub_ = false;
    bool sup_ = false;
    int  list_type_ = none;
public:
    enum list_style {none = 0, bullet = 1, numeric = 2};

    lString16 name_;
    bool valid = false;

    OdtTextStyle(){};

    OdtTextStyle(lString16 name, bool b, bool i, bool u)
    {
        valid = true;
        name_ = name;
        b_ = b;
        i_ = i;
        u_ = u;
    }

    void set_b(bool val){ valid = true; b_ = val;}
    void set_i(bool val){ valid = true; i_ = val;}
    void set_u(bool val){ valid = true; u_ = val;}
    void set_nodraw(bool val){ valid = true; nodraw_ = val;}
    void set_sub(bool val){ valid = true; sub_ = val;}
    void set_sup(bool val){ valid = true; sup_ = val;}
    void set_list_type(int val){ valid = true; list_type_ = val;}

    bool get_b(){ return b_;}
    bool get_i(){ return i_;}
    bool get_u(){ return u_;}
    bool get_nodraw(){ return nodraw_;}
    bool get_sub(){ return sub_;}
    bool get_sup(){ return sup_;}
    int  get_list_type(){ return list_type_;}
};

class OdtTextStyles
{
public:
    LVArray<OdtTextStyle> arr;
    OdtTextStyles(){};
    OdtTextStyle findByName(lString16 name)
    {
        for (int i = 0; i < arr.length(); i++)
        {
            if(arr.get(i).name_ == name)
            {
                return arr.get(i);
            }
        }
        return OdtTextStyle();
    }
    void add(OdtTextStyle style)
    {
        arr.add(style);
    }
};

bool LvXmlParser::ParseOdt(/*DocxItems docxItems, DocxLinks docxLinks, */OdtStyles odtStyles)
{
    Reset();
    callback_->OnStart(this);
    //int txt_count = 0;
    int flags = callback_->getFlags();
    bool error = false;
    bool in_xml_tag = false;
    bool close_flag = false;
    bool q_flag = false;
    bool firstpage_thumb_num_reached = false;
    int fragments_counter = 0;
    lString16 tagname;
    lString16 tagns;
    lString16 attrname;
    lString16 attrns;
    lString16 attrvalue;

    bool in_list = false;
    bool in_style = false;
    bool in_style_t_props = false;

    bool in_b = false;
    bool in_i = false;
    bool in_u = false;
    bool in_nodraw = false;
    bool in_sub = false;
    bool in_sup = false;
    bool in_list_b = false;
    bool in_list_n = false;

    bool in_note = false;
    bool in_noteref = false;
    bool in_notebody = false;
    bool in_toc = false;
    //bool in_span = false;

    bool in_h = false;
    bool in_h_printed = false;



    OdtTextStyles styles;
    OdtTextStyle currstyle;

    lString16 footnote_head;
    lString16 note_id;

    int hlevel = 0;
    int hlevel_backup = 0;
    LinksMap LinksMap = LinksMap_;

    for (; !eof_ && !error && !firstpage_thumb_num_reached ;)
    {
        if (m_stopped)
            break;
        // Load next portion of data if necessary
        lChar16 ch = PeekCharFromBuffer();
        switch (m_state)
        {
            case ps_bof:
            {
                //CRLog::trace("LvXmlParser::Parse() ps_bof");
                // Skip file beginning until '<'
                for ( ; !eof_ && ch!='<'; ch = PeekNextCharFromBuffer())
                    ;
                if (!eof_)
                {
                    m_state = ps_lt;
                    ReadCharFromBuffer();
                }
                //CRLog::trace("LvXmlParser::Parse() ps_bof ret");
            }
                break;
            case ps_lt:  //look for tags
            {
                //CRLog::trace("LvXmlParser::Parse() ps_lt");
                if (!SkipSpaces())
                    break;
                close_flag = false;
                q_flag = false;
                if (ch=='/')
                {
                    ch = ReadCharFromBuffer();
                    close_flag = true;
                }
                else if (ch=='?')
                {
                    // <?xml?>
                    ch = ReadCharFromBuffer();
                    q_flag = true;
                }
                else if (ch=='!')
                {
                    // comments etc...
                    if (PeekCharFromBuffer(1) == '-' && PeekCharFromBuffer(2) == '-') {
                        // skip comments
                        ch = PeekNextCharFromBuffer(2);
                        while (!eof_ && (ch != '-' || PeekCharFromBuffer(1) != '-'
                                         || PeekCharFromBuffer(2) != '>') ) {
                            ch = PeekNextCharFromBuffer();
                        }
                        if (ch=='-' && PeekCharFromBuffer(1)=='-'
                            && PeekCharFromBuffer(2)=='>' )
                            ch = PeekNextCharFromBuffer(2);
                        m_state = ps_text;
                        break;
                    }
                }
                if (!ReadIdent(tagns, tagname) || PeekCharFromBuffer()=='=')
                {
                    // Error
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }
                tagns.lowercase();
                tagname.lowercase();
                //CRLog::error("%s:%s",LCSTR(tagns),LCSTR(tagname));

                tagns = "";

                //removing OpenXML tags from tree
                if(!odtTagAllowed(tagname))
                {
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }

                if(tagname == "line-break")
                {
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    callback_->OnTagOpenAndClose(L"",L"br");
                    break;
                }

                if(tagname=="table-row")
                {
                    tagname = "tr";
                }

                if(tagname=="table-cell")
                {
                    tagname = "td";
                }

                if(tagname == "list"  )
                {
                    in_list = !close_flag;
                }

                if (tagname == "list-item")
                {
                    tagname = "li";
                }

                if(in_list && tagname == "p" )
                {
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }

                if(tagname=="a" && close_flag)
                {
                    callback_->OnAttribute(L"",L"class",L"link_valid");
                }

                if((tagname=="h" && close_flag) || (tagname=="h" && !close_flag && in_h))
                {
                    if(hlevel!=0)
                    {
                        //CRLog::error("hlevel close = %d", hlevel);
                        switch (hlevel)
                        {
                            case 1: callback_->OnTagClose(L"", L"h1"); break;
                            case 2: callback_->OnTagClose(L"", L"h2"); break;
                            case 3: callback_->OnTagClose(L"", L"h3"); break;
                            case 4: callback_->OnTagClose(L"", L"h4"); break;
                            case 5: callback_->OnTagClose(L"", L"h5"); break;
                            case 6: callback_->OnTagClose(L"", L"h6"); break;
                            default:callback_->OnTagClose(L"", L"h1"); break;
                        }
                        hlevel = 0;
                    }
                    else
                    {
                        //CRLog::error("hlevel_backup close = %d", hlevel_backup);
                        switch (hlevel_backup)
                        {
                            case 1: callback_->OnTagClose(L"", L"h1"); break;
                            case 2: callback_->OnTagClose(L"", L"h2"); break;
                            case 3: callback_->OnTagClose(L"", L"h3"); break;
                            case 4: callback_->OnTagClose(L"", L"h4"); break;
                            case 5: callback_->OnTagClose(L"", L"h5"); break;
                            case 6: callback_->OnTagClose(L"", L"h6"); break;
                            default:callback_->OnTagClose(L"", L"h1"); break;
                        }
                        hlevel_backup = 0;
                    }
                    in_h = false;
                    in_h_printed = false;
                }

                if(tagname=="h" && !close_flag)
                {
                    in_h = true;
                }

                if(in_h && tagname == "span")
                {
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }

                if(tagname == "soft-page-break")
                {
                    tagname = "pagebreak";
                }

                if((tagname=="style" || tagname=="list-style") && close_flag)
                {
                    in_style = false;
                    if(currstyle.valid)
                    {
                        CRLog::error("styles add %s",LCSTR(currstyle.name_));
                        styles.add(currstyle);
                    }
                    currstyle = OdtTextStyle();
                }
                if(tagname=="style" || tagname=="list-style")
                {
                    in_style = true;
                }

                if(in_style)
                {
                    if(tagname == "list-level-style-number")
                    {
                        currstyle.set_list_type(currstyle.numeric);
                    }
                    if(tagname == "list-level-style-bullet")
                    {
                        currstyle.set_list_type(currstyle.bullet);
                    }
                }

                if(tagname == "text-properties" && in_style && close_flag )
                {
                    in_style_t_props = false;
                }
                if(tagname == "text-properties" && in_style)
                {
                    in_style_t_props = true;
                }

                if (tagname == "note" && !close_flag)
                {
                    tagname = "hidden";
                    in_note = true;
                }
                if (tagname == "note" && close_flag)
                {
                    callback_->OnTagClose(L"", L"hidden");
                    callback_->OnTagOpen(L"", L"sup");
                    callback_->OnTagOpen(L"", L"a");
                    lString16 href = "#" + note_id;
                    lString16 nref = lString16("#") + note_id + lString16("_note");
                    lString16 id   = note_id + lString16("_back");
                    lString16 head = "[" + footnote_head + "]";

                    callback_->OnAttribute(L"",L"class",L"link_valid");
                    callback_->OnAttribute(L"",L"type",L"note");
                    callback_->OnAttribute(L"",L"href", href.c_str());
                    callback_->OnAttribute(L"",L"nref", nref.c_str());
                    callback_->OnAttribute(L"",L"id",   id.c_str());
                    callback_->OnText(head.c_str(),16,flags);
                    callback_->OnTagClose(L"", L"a");
                    callback_->OnTagClose(L"", L"sup");

                    LinksList_.add(LinkStruct(footnote_head.atoi(),id,href));
                    LinksMap_[href.getHash()] = id;

                    lString16 hrf = "#" + callback_->convertId(note_id);
                    Epub3Notes_.AddAside(hrf);

                    in_note = false;
                    break;
                }

                if (tagname == "note-citation" && !close_flag)
                {
                    tagname = "p";
                    in_noteref = true;
                }

                if (tagname == "note-citation" && close_flag)
                {
                    tagname = "p";
                    in_noteref = false;
                }


                if ((tagname == "note-body" || tagname == "notebody") && !close_flag)
                {
                    tagname = "p";
                    in_notebody = true;
                }
                if ((tagname == "note-body" || tagname == "notebody") && close_flag)
                {
                    tagname = "p";
                    callback_->OnAttribute(L"",L"class",L"hidden");
                    in_notebody = false;
                }


                if(tagname == "table-of-content" && !close_flag)
                {
                    tagname = "hidden";
                    in_toc = true;
                }

                if(tagname == "table-of-content" && close_flag)
                {
                    tagname = "hidden";
                    in_toc = false;
                }


                //all other closing tags handling
                if (close_flag)
                {
                    if(in_b)
                    {
                        callback_->OnTagClose(L"", L"b");
                        in_b = false;
                    }
                    if(in_i)
                    {
                        callback_->OnTagClose(L"", L"i");
                        in_i = false;
                    }
                    if(in_u)
                    {
                        callback_->OnTagClose(L"", L"u");
                        in_u = false;
                    }
                    if(in_nodraw)
                    {
                        callback_->OnTagClose(L"", L"hidden");
                        in_nodraw = false;
                    }
                    if(in_sub)
                    {
                        callback_->OnTagClose(L"", L"sub");
                        in_sub = false;
                    }
                    if(in_sup)
                    {
                        callback_->OnTagClose(L"", L"sup");
                        in_sup = false;
                    }
                    callback_->OnTagClose(tagns.c_str(), tagname.c_str());
                    //CRLog::trace("</%s:%s>", LCSTR(tagns),LCSTR(tagname));
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }

                if (q_flag) {
                    tagname.insert(0, 1, '?');
                    in_xml_tag = (tagname == "?xml");
                } else {
                    in_xml_tag = false;
                }
                callback_->OnTagOpen(tagns.c_str(), tagname.c_str());
                //CRLog::trace("<%s:%s>", LCSTR(tagns),LCSTR(tagname));

                m_state = ps_attr;
                //CRLog::trace("LvXmlParser::Parse() ps_lt ret");
            }
                break;
            case ps_attr: //read tags
            {
                //CRLog::trace("LvXmlParser::Parse() ps_attr");
                if (!SkipSpaces())
                    break;
                ch = PeekCharFromBuffer();
                lChar16 nch = PeekCharFromBuffer(1);
                if (ch == '>' || ((ch == '/' || ch == '?') && nch == '>'))
                {
                    callback_->OnTagBody();
                    // end of tag
                    if (ch != '>')
                        callback_->OnTagClose(tagns.c_str(), tagname.c_str());
                    if (ch == '>')
                        ch = PeekNextCharFromBuffer();
                    else
                        ch = PeekNextCharFromBuffer(1);
                    m_state = ps_text;
                    break;
                }
                if (!ReadIdent(attrns, attrname))
                {
                    // error: skip rest of tag
                    SkipTillChar('<');
                    ch = PeekNextCharFromBuffer(1);
                    callback_->OnTagBody();
                    m_state = ps_lt;
                    break;
                }
                SkipSpaces();
                attrvalue.reset(16);
                ch = PeekCharFromBuffer();
                // Read attribute value
                if (ch == '=')
                {
                    // Skip '='
                    ReadCharFromBuffer();
                    SkipSpaces();
                    lChar16 qChar = 0;
                    ch = PeekCharFromBuffer();
                    if (ch == '\"' || ch == '\'')
                    {
                        qChar = ReadCharFromBuffer();
                    }
                    for (; !eof_;)
                    {
                        ch = PeekCharFromBuffer();
                        if (ch == '>')
                            break;
                        if (!qChar && IsSpaceChar(ch))
                            break;
                        if (qChar && ch == qChar)
                        {
                            ch = PeekNextCharFromBuffer();
                            break;
                        }
                        ch = ReadCharFromBuffer();
                        if (ch)
                            attrvalue += ch;
                        else
                            break;
                    }
                }
                if (possible_capitalized_tags_) {
                    attrns.lowercase();
                    attrname.lowercase();
                }
                attrns = "";
                if(attrname == "number-columns-spanned")
                {
                    attrname = "colspan";
                }
                if(attrname == "number-rows-spanned")
                {
                    attrname = "rowspan";
                }
                if (in_note)
                {
                    if(tagname == "hidden" && attrname == "id")
                    {
                        note_id = attrvalue;
                        //CRLog::error("note_id = %s",LCSTR(note_id));
                    }
                }

                if (in_h)
                {
                    callback_->OnTagClose(L"", L"h");

                    //CRLog::error("in h");
                    if (attrname == "outline-level")
                    {
                        hlevel_backup = attrvalue.atoi();
                        //CRLog::error("hlevel_backup  = [%d]", hlevel_backup);
                    }

                    if (attrname == "style-name")
                    {
                        hlevel = odtStyles.GetHeaderById(attrvalue);
                        //CRLog::error("hlevel  [%s] => [%d]", LCSTR(attrvalue), hlevel);
                    }
                }

                if (in_style)
                {
                    if (attrname == "name")
                    {
                        currstyle.name_ = attrvalue;
                    }
                    //no need
                    //if (attrname == "master-page-name" && !attrvalue.empty())
                    //{
                    //    //currstyle.set_nodraw(true);
                    //}
                }

                if(in_style_t_props)
                {
                    if(attrname == "font-weight" && attrvalue == "bold")
                    {
                        currstyle.set_b(true);
                    }
                    if(attrname == "font-style" && attrvalue == "italic")
                    {
                        currstyle.set_i(true);
                    }
                    if ((attrname == "text-underline-type" && attrvalue == "single") || (attrname == "text-underline-style" && attrvalue == "solid"))
                    {
                        currstyle.set_u(true);
                    }
                    if(attrname == "text-position" && attrvalue.pos("sub")!=-1)
                    {
                        currstyle.set_sub(true);
                    }
                    if(attrname == "text-position" && attrvalue.pos("super")!=-1)
                    {
                        currstyle.set_sup(true);
                    }
                }

                if(attrname == "style-name")
                {
                    OdtTextStyle s = styles.findByName(attrvalue);
                    if(s.valid)
                    {
                        if (s.get_b())
                        {
                            callback_->OnTagOpen(L"", L"b");
                            in_b = true;
                        }
                        if (s.get_i())
                        {
                            callback_->OnTagOpen(L"", L"i");
                            in_i = true;
                        }
                        if (s.get_u())
                        {
                            callback_->OnTagOpen(L"", L"u");
                            in_u = true;
                        }
                        if(s.get_nodraw())
                        {
                            callback_->OnTagOpen(L"", L"hidden");
                            in_nodraw = true;
                        }
                        if(s.get_sub())
                        {
                            callback_->OnTagOpen(L"", L"sub");
                            in_sub = true;
                        }
                        if(s.get_sup())
                        {
                            callback_->OnTagOpen(L"", L"sup");
                            in_sup = true;
                        }
                        if(in_list)
                        {
                            if (s.get_list_type() != s.none)
                            {
                                if (s.get_list_type() == s.bullet)
                                {
                                    callback_->OnTagOpen(L"", L"ul");
                                    in_list_b = true;
                                }
                                else //s.numeric
                                {
                                    callback_->OnTagOpen(L"", L"ol");
                                    in_list_n = true;
                                }
                            }
                        }
                    }
                }

                callback_->OnAttribute(attrns.c_str(), attrname.c_str(), attrvalue.c_str());
                //CRLog::trace("LvXmlParser::Parse() ps_attr ret");
            }
                break;
            case ps_text:
            {
                lString16 temp;

                if(in_noteref || in_notebody )
                {
                    //CRLog::error("adding hidden %d %d",in_noteref,in_notebody);
                    callback_->OnAttribute(L"",L"class",L"hidden");
                }

                if(in_noteref)
                {
                    this->ReadTextToString(footnote_head,false,true);
                }
                else
                {
                    this->ReadTextToString(temp,true,true);
                }

                if(in_h)
                {
                    if (!in_h_printed)
                    {
                        //CRLog::error("!in h printed");
                        //CRLog::error("tagname = %s",LCSTR(tagname));
                        in_h_printed = true;
                        if (hlevel != 0)
                        {
                            //CRLog::error("hlevel open = %d", hlevel);
                            switch (hlevel)
                            {
                                case 1: callback_->OnTagOpen(L"", L"h1"); break;
                                case 2: callback_->OnTagOpen(L"", L"h2"); break;
                                case 3: callback_->OnTagOpen(L"", L"h3"); break;
                                case 4: callback_->OnTagOpen(L"", L"h4"); break;
                                case 5: callback_->OnTagOpen(L"", L"h5"); break;
                                case 6: callback_->OnTagOpen(L"", L"h6"); break;
                                default:callback_->OnTagOpen(L"", L"h1"); break;
                            }
                        }
                        else
                        {
                            //CRLog::error("hlevel_backup open = %d", hlevel_backup);
                            switch (hlevel_backup)
                            {
                                case 1: callback_->OnTagOpen(L"", L"h1"); break;
                                case 2: callback_->OnTagOpen(L"", L"h2"); break;
                                case 3: callback_->OnTagOpen(L"", L"h3"); break;
                                case 4: callback_->OnTagOpen(L"", L"h4"); break;
                                case 5: callback_->OnTagOpen(L"", L"h5"); break;
                                case 6: callback_->OnTagOpen(L"", L"h6"); break;
                                default:callback_->OnTagOpen(L"", L"h1"); break;
                            }
                        }
                    }
                }

                //if(!temp.DigitsOnly() && !temp.endsWith(" "))
                //{
                //    callback_->OnText(L" ", 1, flags);
                //}
                temp.clear();

                if(need_coverpage_)
                {
                    //CRLog::trace("LvXmlParser: text fragments read : %d", fragments_counter);
                    if (fragments_counter >= FIRSTPAGE_BLOCKS_MAX_DOCX)
                    {
                        firstpage_thumb_num_reached = true;
                    }
                }
                m_state = ps_lt;
            }
                break;
            default:
            {
            }
        }
    }
    callback_->OnStop();
    return !error;
}


bool LvXmlParser::ParseEpubFootnotes()
{
    Reset();
    EpubStylesManager stylesManager = getStylesManager();
    lString16 name = lString16("notes");
    callback_->OnStart(this);
    callback_->OnTagOpen(L"",L"body");
    callback_->OnAttribute(L"", L"name", name.c_str());
    //int txt_count = 0;
    int flags = callback_->getFlags();
    bool error = false;
    bool in_xml_tag = false;
    bool close_flag = false;
    bool q_flag = false;
    bool body_started = false;
    bool firstpage_thumb_num_reached = false;

    bool in_section = false;
    bool in_section_inner = false;
    bool in_a = false;
    bool in_body = false;
    bool save_title_content = false;
    bool title_content_saved = false;
    bool title_section_check = false;
    bool reappend_title = false;
    bool title_is_text = false;
    bool in_head = false;

    int fragments_counter = 0;
    lString16 tagname;
    lString16 tagns;
    lString16 attrname;
    lString16 attrns;
    lString16 attrvalue;
    lString16 buffer;
    lString16 rtl_holder;
    int rtl_holder_counter = 0;

    int buffernum =-1;
    //LVArray<LinkStruct> LinksList = getLinksList();
    LinksMap LinksMap = getLinksMap();
    lString16 temp_section_id;

    for (; !eof_ && !error && !firstpage_thumb_num_reached ;)
    {
        if (m_stopped)
            break;
        // Load next portion of data if necessary
        lChar16 ch = PeekCharFromBuffer();
        switch (m_state)
        {
            case ps_bof:
            {
                //CRLog::trace("LvXmlParser::Parse() ps_bof");
                // Skip file beginning until '<'
                for ( ; !eof_ && ch!='<'; ch = PeekNextCharFromBuffer())
                    ;
                if (!eof_)
                {
                    m_state = ps_lt;
                    ReadCharFromBuffer();
                }
                //CRLog::trace("LvXmlParser::Parse() ps_bof ret");
            }
                break;

            case ps_lt:  //look for tags
            {
                //CRLog::trace("LvXmlParser::Parse() ps_lt");
                if (!SkipSpaces())
                    break;
                close_flag = false;
                q_flag = false;
                if (ch=='/')
                {
                    ch = ReadCharFromBuffer();
                    close_flag = true;
                }
                else if (ch=='?')
                {
                    // <?xml?>
                    ch = ReadCharFromBuffer();
                    q_flag = true;
                }
                else if (ch=='!')
                {
                    // comments etc...
                    if (PeekCharFromBuffer(1) == '-' && PeekCharFromBuffer(2) == '-') {
                        // skip comments
                        ch = PeekNextCharFromBuffer(2);
                        while (!eof_ && (ch != '-' || PeekCharFromBuffer(1) != '-'
                                         || PeekCharFromBuffer(2) != '>') ) {
                            ch = PeekNextCharFromBuffer();
                        }
                        if (ch=='-' && PeekCharFromBuffer(1)=='-'
                            && PeekCharFromBuffer(2)=='>' )
                            ch = PeekNextCharFromBuffer(2);
                        m_state = ps_text;
                        break;
                    }
                }
                if (!ReadIdent(tagns, tagname) || PeekCharFromBuffer()=='=')
                {
                    // Error
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }
                tagns.lowercase();
                tagname.lowercase();
                //if(!close_flag) CRLog::error(" <%s>",LCSTR(tagname));
                //if( close_flag) CRLog::error("</%s>",LCSTR(tagname));

                tagns = "";

                if(tagname == "html" || tagname == "body" || tagname == "meta"|| tagname == "link" || tagname == "xml")
                {
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }
                if (tagname == "head")
                {
                    if(!close_flag)
                    {
                        in_head = true;
                    }
                    else
                    {
                        in_head = false;
                    }
                }
                if ((tagname == "a" || tagname == "head") && !close_flag)
                {
                    if (SkipTillChar('/'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }

                if (tagname == "h1" ||
                    tagname == "h2" ||
                    tagname == "h3" ||
                    tagname == "h4" ||
                    tagname == "h5" ||
                    tagname == "h6")
                {
                    tagname = "title";
                }
                if(in_head)
                {
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }
                if(tagname == "title" && !in_section && !in_head)
                {
                    if (!close_flag) // set flags and remove title tag
                    {
                        save_title_content = true;
                        title_content_saved = false;
                    }
                    if (close_flag)
                    {

                        save_title_content = false;
                    }
                    if (!title_is_text)
                    {
                        if (SkipTillChar('>'))
                        {
                            m_state = ps_text;
                            ch = ReadCharFromBuffer();
                        }
                        break;
                    }
                    if (title_is_text)
                    {
                        callback_->OnTagOpen(L"", L"title");
                        callback_->OnText(buffer.c_str(), buffer.length(), flags);
                        callback_->OnTagClose(L"", L"title");
                        title_is_text = false;
                        break;
                    }
                }

                if (tagname == "div")
                {
                    tagname = "section";
                    temp_section_id = lString16::empty_str;
                }

                if (in_section)
                {
                    if (tagname == "title" && !close_flag && !title_content_saved)
                    {
                        save_title_content = true;
                        title_section_check = true;

                        //title tag exists in current section, no need to insert another one
                        //do nothing
                        if (SkipTillChar('>'))
                        {
                            m_state = ps_text;
                            ch = ReadCharFromBuffer();
                        }
                        break;
                    }
                    if (tagname == "title" && close_flag && title_section_check)
                    {
                        if (title_is_text)
                        {
                            callback_->OnTagClose(L"", L"section");
                            callback_->OnTagOpen(L"", L"title");
                            callback_->OnText(buffer.c_str(), buffer.length(), flags);
                            callback_->OnTagClose(L"", L"title");
                            callback_->OnTagClose(L"", L"title");
                        }
                        else
                        {
                            lString16 currlink_id;
                            if(LinksMap_.size() !=0 && buffernum != -1)
                            {
                                if(LinksMap_.find(temp_section_id.getHash())!=LinksMap_.end())
                                {
                                    currlink_id = LinksMap_[temp_section_id.getHash()];
                                    //CRLog::error("found [%s] at [%s]",LCSTR(currlink_id),LCSTR(temp_section_id));
                                    currlink_id = lString16("#") + currlink_id ;
                                }
                            }
                            if(!currlink_id.empty())
                            {
                                callback_->OnTagOpen(L"", L"title");
                                callback_->OnTagOpen(L"", L"a");
                                callback_->OnAttribute(L"",L"class",L"link_valid");
                                callback_->OnAttribute(L"",L"href",(lString16("~") + currlink_id).c_str());
                                callback_->OnText(buffer.c_str(), buffer.length(), flags);
                                callback_->OnTagClose(L"", L"a");
                                callback_->OnTagClose(L"", L"title");
                            }
                            else
                            {
                                callback_->OnTagOpen(L"", L"title");
                                callback_->OnText(buffer.c_str(), buffer.length(), flags);
                                callback_->OnTagClose(L"", L"title");
                            }
                        }
                        save_title_content = false;
                        title_section_check = false;
                    }
                    if(tagname == "title")
                    {
                        //title tag exists in current section, no need to insert another one
                        //do nothing
                        title_content_saved = false;
                    }
                    if(tagname != "title" && title_content_saved)
                    {

                        lString16 currlink_id;
                        if (LinksMap_.size() != 0 && buffernum != -1)
                        {
                            if (LinksMap_.find(temp_section_id.getHash()) != LinksMap_.end())
                            {
                                currlink_id = LinksMap_[temp_section_id.getHash()];
                                currlink_id = lString16("#") + currlink_id;
                            }
                        }
                        if (!currlink_id.empty())
                        {
                            callback_->OnTagOpen(L"", L"title");
                            callback_->OnTagOpen(L"", L"a");
                            callback_->OnAttribute(L"",
                                    L"href",
                                    (lString16("~") + currlink_id).c_str());
                            callback_->OnAttribute(L"",L"class",L"link_valid");
                            callback_->OnText(buffer.c_str(), buffer.length(), flags);
                            callback_->OnTagClose(L"", L"a");
                            callback_->OnTagClose(L"", L"title");
                        }
                        else
                        {
                            callback_->OnTagOpen(L"", L"title");
                            callback_->OnText(buffer.c_str(), buffer.length(), flags);
                            callback_->OnTagClose(L"", L"title");
                        }

                        title_content_saved = false;
                    }
                    if (tagname == "section" && in_section && !close_flag)
                    {
                        in_section_inner = true;
                        m_state = ps_text;
                    }
                    if (tagname == "section" && close_flag && in_section_inner)
                    {
                        in_section_inner = false;
                        m_state = ps_text;
                    }
                }

                if (tagname == "section")
                {
                    if (!close_flag)
                    {
                        in_section = true;
                    }
                    else if (close_flag && !in_section_inner)
                    {
                        in_section = false;
                    }
                }

                if(!rtl_holder.empty() && tagname == rtl_holder)
                {
                    if(!close_flag)
                    {
                        rtl_holder_counter++;
                    }
                    else
                    {
                        rtl_holder_counter--;
                    }
                }

                if (close_flag)
                {
                    if(tagname == rtl_holder && RTL_DISPLAY_ENABLE && ( rtl_holder_counter == 0 ) )
                    {
                        callback_->setRTLflag(false);
                        rtl_holder.clear();
                    }
                    callback_->OnTagClose(tagns.c_str(), tagname.c_str());
                    //CRLog::trace("</%s:%s>", LCSTR(tagns),LCSTR(tagname));
                    if (SkipTillChar('>'))
                    {
                        m_state = ps_text;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }

                if (q_flag) {
                    tagname.insert(0, 1, '?');
                    in_xml_tag = (tagname == "?xml");
                } else {
                    in_xml_tag = false;
                }

                    callback_->OnTagOpen(tagns.c_str(), tagname.c_str());

                //CRLog::trace("<%s:%s>", LCSTR(tagns),LCSTR(tagname));

                m_state = ps_attr;
                //CRLog::trace("LvXmlParser::Parse() ps_lt ret");
            }
                break;
            case ps_attr: //read tags
            {
                //CRLog::trace("LvXmlParser::Parse() ps_attr");
                if (!SkipSpaces())
                    break;
                ch = PeekCharFromBuffer();
                lChar16 nch = PeekCharFromBuffer(1);
                if (ch == '>' || ((ch == '/' || ch == '?') && nch == '>'))
                {
                    callback_->OnTagBody();
                    // end of tag
                    if (ch != '>')
                        callback_->OnTagClose(tagns.c_str(), tagname.c_str());
                    if (ch == '>')
                        ch = PeekNextCharFromBuffer();
                    else
                        ch = PeekNextCharFromBuffer(1);
                    m_state = ps_text;
                    break;
                }
                if (!ReadIdent(attrns, attrname))
                {
                    // error: skip rest of tag
                    SkipTillChar('<');
                    ch = PeekNextCharFromBuffer(1);
                    callback_->OnTagBody();
                    m_state = ps_lt;
                    break;
                }
                SkipSpaces();
                attrvalue.reset(16);
                ch = PeekCharFromBuffer();
                // Read attribute value
                if (ch == '=')
                {
                    // Skip '='
                    ReadCharFromBuffer();
                    SkipSpaces();
                    lChar16 qChar = 0;
                    ch = PeekCharFromBuffer();
                    if (ch == '\"' || ch == '\'')
                    {
                        qChar = ReadCharFromBuffer();
                    }
                    for (; !eof_;)
                    {
                        ch = PeekCharFromBuffer();
                        if (ch == '>')
                            break;
                        if (!qChar && IsSpaceChar(ch))
                            break;
                        if (qChar && ch == qChar)
                        {
                            ch = PeekNextCharFromBuffer();
                            break;
                        }
                        ch = ReadCharFromBuffer();
                        if (ch)
                            attrvalue += ch;
                        else
                            break;
                    }
                }
                if (possible_capitalized_tags_) {
                    attrns.lowercase();
                    attrname.lowercase();
                }
                attrns = "";

                if ((flags & TXTFLG_CONVERT_8BIT_ENTITY_ENCODING) && m_conv_table) {
                    PreProcessXmlString(attrvalue, 0, m_conv_table);
                }

                if((tagname == "section" || in_section ) && tagname != "title"  && attrname == "id")
                {
                    temp_section_id = lString16("#") + callback_->convertId(attrvalue);
                    //CRLog::error("temp_section_id = [%s]",LCSTR(temp_section_id));
                }

                if(attrname == "class" && attrvalue.pos(" ")!=-1)
                {
                    lString16Collection classes;
                    classes.parse(attrvalue,' ',true);

                    bool bold = false;
                    bool italic = false;
                    bool underline = false;

                    for (int i = 0; i < classes.length(); i++)
                    {
                        lString16 name = classes.at(i);
                        if(stylesManager.classExists(name))
                        {
                            CssStyle CssClass = stylesManager.getClass(name);
                            bold = CssClass.isBold();
                            italic = CssClass.isItalic();
                            underline = CssClass.isUnderline();
                        }
                    }
                    lString16 style;
                    if(bold)      style += "font-weight: bold;";
                    if(italic)    style += "font-style: italic;";
                    if(underline) style += "text-decoration: underline;";
                    callback_->OnAttribute(L"",L"style",style.c_str());
                }

                if(gEmbeddedStylesLVL > 0 && !stylesManager.rtl_map_empty())
                {
                    if(attrname == "class" && stylesManager.ClassIsRTL(attrvalue))
                    {
                        callback_->OnAttribute(L"",L"dir",L"rtl");
                        callback_->setRTLflag(true);
                        gDocumentRTL = 1;
                    }
                }

                if(RTL_DISPLAY_ENABLE && rtl_holder.empty())
                {
                    if(attrname=="dir" || attrname=="direction" || attrname == "class")
                    {
                        if(attrvalue=="rtl")
                        {
                            rtl_holder = tagname;
                            callback_->setRTLflag(true);
                            rtl_holder_counter++;
                            gDocumentRTL = 1;
                        }
                    }
                }
                //CRLog::error("OnAttrib [%s:%s = \"%s\"]",LCSTR(attrns), LCSTR(attrname), LCSTR(attrvalue));
                callback_->OnAttribute(attrns.c_str(), attrname.c_str(), attrvalue.c_str());


                if (in_xml_tag && attrname == "encoding")
                {
                    SetCharset(attrvalue.c_str());
                }
                //CRLog::trace("LvXmlParser::Parse() ps_attr ret");
            }
                break;
            case ps_text:
            {
                if(in_head)
                {
                    if (SkipTillChar('<'))
                    {
                        m_state = ps_lt;
                        ch = ReadCharFromBuffer();
                    }
                    break;
                }
                if (save_title_content)
                {
                    this->ReadTextToString(buffer,false);
                    //CRLog::error("saving to buffer = [%s]", LCSTR(buffer));
                    if (buffer.atoi() <= 0)
                    {
                        buffernum = -1;
                        title_is_text = true;
                    }
                    else
                    {
                        buffernum = buffer.atoi();
                        title_is_text = false;
                    }
                    title_content_saved = true;
                }
                else
                {
                    ReadText();
                }

                fragments_counter++;
                if(need_coverpage_)
                {
                    //CRLog::trace("LvXmlParser: text fragments read : %d", fragments_counter);
                    if (fragments_counter >= FIRSTPAGE_BLOCKS_MAX_DOCX)
                    {
                        firstpage_thumb_num_reached = true;
                    }
                }
                m_state = ps_lt;
            }
                break;
            default:
            {
            }
        }
    }
    callback_->OnTagClose(L"",L"body");
    callback_->OnStop();
    return !error;
}

#define TEXT_SPLIT_SIZE 8192

typedef struct  {
    const wchar_t * name;
    wchar_t code;
} ent_def_t;

static const ent_def_t def_entity_table[] = {
{L"nbsp", 160},
{L"iexcl", 161},
{L"cent", 162},
{L"pound", 163},
{L"curren", 164},
{L"yen", 165},
{L"brvbar", 166},
{L"sect", 167},
{L"uml", 168},
{L"copy", 169},
{L"ordf", 170},
{L"laquo", 171},
{L"not", 172},
{L"shy", 173},
{L"reg", 174},
{L"macr", 175},
{L"deg", 176},
{L"plusmn", 177},
{L"sup2", 178},
{L"sup3", 179},
{L"acute", 180},
{L"micro", 181},
{L"para", 182},
{L"middot", 183},
{L"cedil", 184},
{L"sup1", 185},
{L"ordm", 186},
{L"raquo", 187},
{L"frac14", 188},
{L"frac12", 189},
{L"frac34", 190},
{L"iquest", 191},
{L"Agrave", 192},
{L"Aacute", 193},
{L"Acirc", 194},
{L"Atilde", 195},
{L"Auml", 196},
{L"Aring", 197},
{L"AElig", 198},
{L"Ccedil", 199},
{L"Egrave", 200},
{L"Eacute", 201},
{L"Ecirc", 202},
{L"Euml", 203},
{L"Igrave", 204},
{L"Iacute", 205},
{L"Icirc", 206},
{L"Iuml", 207},
{L"ETH", 208},
{L"Ntilde", 209},
{L"Ograve", 210},
{L"Oacute", 211},
{L"Ocirc", 212},
{L"Otilde", 213},
{L"Ouml", 214},
{L"times", 215},
{L"Oslash", 216},
{L"Ugrave", 217},
{L"Uacute", 218},
{L"Ucirc", 219},
{L"Uuml", 220},
{L"Yacute", 221},
{L"THORN", 222},
{L"szlig", 223},
{L"agrave", 224},
{L"aacute", 225},
{L"acirc", 226},
{L"atilde", 227},
{L"auml", 228},
{L"aring", 229},
{L"aelig", 230},
{L"ccedil", 231},
{L"egrave", 232},
{L"eacute", 233},
{L"ecirc", 234},
{L"euml", 235},
{L"igrave", 236},
{L"iacute", 237},
{L"icirc", 238},
{L"iuml", 239},
{L"eth", 240},
{L"ntilde", 241},
{L"ograve", 242},
{L"oacute", 243},
{L"ocirc", 244},
{L"otilde", 245},
{L"ouml", 246},
{L"divide", 247},
{L"oslash", 248},
{L"ugrave", 249},
{L"uacute", 250},
{L"ucirc", 251},
{L"uuml", 252},
{L"yacute", 253},
{L"thorn", 254},
{L"yuml", 255},
{L"quot", 34},
{L"amp", 38},
{L"lt", 60},
{L"gt", 62},
{L"apos", '\''},
{L"OElig", 338},
{L"oelig", 339},
{L"Scaron", 352},
{L"scaron", 353},
{L"Yuml", 376},
{L"circ", 710},
{L"tilde", 732},
{L"ensp", 8194},
{L"emsp", 8195},
{L"thinsp", 8201},
{L"zwnj", 8204},
{L"zwj", 8205},
{L"lrm", 8206},
{L"rlm", 8207},
{L"ndash", 8211},
{L"mdash", 8212},
{L"lsquo", 8216},
{L"rsquo", 8217},
{L"sbquo", 8218},
{L"ldquo", 8220},
{L"rdquo", 8221},
{L"bdquo", 8222},
{L"dagger", 8224},
{L"Dagger", 8225},
{L"permil", 8240},
{L"lsaquo", 8249},
{L"rsaquo", 8250},
{L"euro", 8364},
{L"fnof", 402},
{L"Alpha", 913},
{L"Beta", 914},
{L"Gamma", 915},
{L"Delta", 916},
{L"Epsilon", 917},
{L"Zeta", 918},
{L"Eta", 919},
{L"Theta", 920},
{L"Iota", 921},
{L"Kappa", 922},
{L"Lambda", 923},
{L"Mu", 924},
{L"Nu", 925},
{L"Xi", 926},
{L"Omicron", 927},
{L"Pi", 928},
{L"Rho", 929},
{L"Sigma", 931},
{L"Tau", 932},
{L"Upsilon", 933},
{L"Phi", 934},
{L"Chi", 935},
{L"Psi", 936},
{L"Omega", 937},
{L"alpha", 945},
{L"beta", 946},
{L"gamma", 947},
{L"delta", 948},
{L"epsilon", 949},
{L"zeta", 950},
{L"eta", 951},
{L"theta", 952},
{L"iota", 953},
{L"kappa", 954},
{L"lambda", 955},
{L"mu", 956},
{L"nu", 957},
{L"xi", 958},
{L"omicron", 959},
{L"pi", 960},
{L"rho", 961},
{L"sigmaf", 962},
{L"sigma", 963},
{L"tau", 964},
{L"upsilon", 965},
{L"phi", 966},
{L"chi", 967},
{L"psi", 968},
{L"omega", 969},
{L"thetasym", 977},
{L"upsih", 978},
{L"piv", 982},
{L"bull", 8226},
{L"hellip", 8230},
{L"prime", 8242},
{L"Prime", 8243},
{L"oline", 8254},
{L"frasl", 8260},
{L"weierp", 8472},
{L"image", 8465},
{L"real", 8476},
{L"trade", 8482},
{L"alefsym", 8501},
{L"larr", 8592},
{L"uarr", 8593},
{L"rarr", 8594},
{L"darr", 8595},
{L"harr", 8596},
{L"crarr", 8629},
{L"lArr", 8656},
{L"uArr", 8657},
{L"rArr", 8658},
{L"dArr", 8659},
{L"hArr", 8660},
{L"forall", 8704},
{L"part", 8706},
{L"exist", 8707},
{L"empty", 8709},
{L"nabla", 8711},
{L"isin", 8712},
{L"notin", 8713},
{L"ni", 8715},
{L"prod", 8719},
{L"sum", 8721},
{L"minus", 8722},
{L"lowast", 8727},
{L"radic", 8730},
{L"prop", 8733},
{L"infin", 8734},
{L"ang", 8736},
{L"and", 8743},
{L"or", 8744},
{L"cap", 8745},
{L"cup", 8746},
{L"int", 8747},
{L"there4", 8756},
{L"sim", 8764},
{L"cong", 8773},
{L"asymp", 8776},
{L"ne", 8800},
{L"equiv", 8801},
{L"le", 8804},
{L"ge", 8805},
{L"sub", 8834},
{L"sup", 8835},
{L"nsub", 8836},
{L"sube", 8838},
{L"supe", 8839},
{L"oplus", 8853},
{L"otimes", 8855},
{L"perp", 8869},
{L"sdot", 8901},
{L"lceil", 8968},
{L"rceil", 8969},
{L"lfloor", 8970},
{L"rfloor", 8971},
{L"lang", 9001},
{L"rang", 9002},
{L"loz", 9674},
{L"spades", 9824},
{L"clubs", 9827},
{L"hearts", 9829},
{L"diams", 9830},
{NULL, 0},
};

/// In-place XML string decoding, don't expand tabs, returns new length (may be less than initial len)
int PreProcessXmlString(lChar16 *str, int len, lUInt32 flags, const lChar16 *enc_table)
{
    int state = 0;
    lChar16 nch = 0;
    lChar16 lch = 0;
    lChar16 nsp = 0;
    bool pre = (flags & TXTFLG_PRE);
    bool pre_para_splitting = (flags & TXTFLG_PRE_PARA_SPLITTING) != 0;
    if (pre_para_splitting)
    {
        pre = false;
    }
    //CRLog::trace("before: '%s' %s", LCSTR(s), pre ? "pre ":" ");
    int j = 0;
    for (int i = 0; i < len; ++i)
    {
        lChar16 ch = str[i];
        if (pre)
        {
            if (ch == '\r')
            {
                if ((i == 0 || lch != '\n') && (i == len - 1 || str[i + 1] != '\n'))
                {
                    str[j++] = '\n';
                    lch = '\n';
                }
                continue;
            }
            else if (ch == '\n')
            {
                str[j++] = '\n';
                lch = ch;
                continue;
            }
        }
        else
        {
            if( ch == L'â' && str[i+1] == 0x0402 && i < len-2)
            {
                switch (str[i+2])
                {
                    case 0x045A: i+=2; ch = 0x2019; break;
                    case 0x00a6: i+=2; ch = 0x2026; break;
                    case 0x201c: i+=2; ch = '-';    break;
                    case 0x0000:
                    case 0x00B2:
                    case 0x201d: i+=2; ch = ' ';    break;
                    case 0x045C:
                    case 0x2122: i+=2; ch = '`';    break;
                    default: break;
                }
            }
            if (ch == UNICODE_SOFT_HYPHEN_CODE)  // add filtered chars here
            {
                continue;
            }
            if (ch == '\r' || ch == '\n' || ch == '\t')
            {
                ch = ' ';
            }
        }
        if (ch == '&')
        {
            state = 1;
            nch = 0;
        }
        else if (state == 0)
        {
            if (ch == ' ')
            {
                if (pre || !nsp)
                {
                    str[j++] = ch;
                }
                nsp++;
            }
            else
            {
                str[j++] = ch;
                nsp = 0;
            }
        }
        else
        {
            if (state == 2 && ch == 'x')
            {
                state = 22;
            }
            else if (state == 22 && hexDigit(ch) >= 0)
            {
                nch = (lChar16) ((nch << 4) | hexDigit(ch));
            }
            else if (state == 2 && ch >= '0' && ch <= '9')
            {
                nch = (lChar16) (nch * 10 + (ch - '0'));
            }
            else if (ch == '#' && state == 1)
            {
                state = 2;
            }
            else if (state == 1 && ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')))
            {
                int k;
                lChar16 entname[16];
                // 16 - 1 to avoid buffer overflow crash
                for (k = i; str[k] && str[k] != ';' && str[k] != ' ' && k - i < 16 - 1; k++)
                    entname[k - i] = str[k];
                entname[k - i] = 0;
                int n;
                lChar16 code = 0;
                // TODO: optimize search
                if (str[k] == ';' || str[k] == ' ')
                {
                    for (n = 0; def_entity_table[n].name; n++)
                    {
                        if (!lStr_cmp(def_entity_table[n].name, entname))
                        {
                            code = def_entity_table[n].code;
                            break;
                        }
                    }
                }
                if (code)
                {
                    i = k;
                    state = 0;
                    if (enc_table && code < 256 && code >= 128)
                    {
                        code = enc_table[code - 128];
                    }
                    if (code == UNICODE_SOFT_HYPHEN_CODE) // 173 == &shy
                    {
                        //do nothing. that removes shy tags from output
                    }
                    else
                    {
                        str[j++] = code;
                    }
                    nsp = 0;
                }
                else
                {
                    // include & and rest of entity into output string
                    str[j++] = '&';
                    str[j++] = str[i];
                    state = 0;
                }

            }
            else if (ch == ';')
            {
                if (nch)
                {
                    if (nch == UNICODE_SOFT_HYPHEN_CODE)
                    {
                        //do nothing. that removes shy tags from output
                    }
                    else
                    {
                        str[j++] = nch;
                    }
                }
                state = 0;
                nsp = 0;
            }
            else
            {
                // error: return to normal mode
                state = 0;
            }
        }
        lch = ch;
    }
    return j;
}

int CalcTabCount(const lChar16 * str, int nlen) {
    int tabCount = 0;
    for (int i=0; i<nlen; i++) {
        if (str[i] == '\t')
            tabCount++;
    }
    return tabCount;
}

void ExpandTabs(lString16 & buf, const lChar16 * str, int len)
{
    // check for tabs
    int x = 0;
    for (int i = 0; i < len; i++) {
        lChar16 ch = str[i];
        if ( ch=='\r' || ch=='\n' )
            x = 0;
        if ( ch=='\t' ) {
            int delta = 8 - (x & 7);
            x += delta;
            while ( delta-- )
                buf << L' ';
        } else {
            buf << ch;
            x++;
        }
    }
}

void ExpandTabs(lString16 & s)
{
    // check for tabs
    int nlen = s.length();
    int tabCount = CalcTabCount(s.c_str(), nlen);
    if ( tabCount > 0 ) {
        // expand tabs
        lString16 buf;
        buf.reserve(nlen + tabCount * 8);
        ExpandTabs(buf, s.c_str(), s.length());
        s = buf;
    }
}

// returns new length
void PreProcessXmlString( lString16 & s, lUInt32 flags, const lChar16 * enc_table )
{
    lChar16 * str = s.modify();
    int len = s.length();
    int nlen = PreProcessXmlString(str, len, flags, enc_table);
    // remove extra characters from end of line
    if (nlen < len)
        s.limit(nlen);

    if (flags & TXTFLG_PRE)
        ExpandTabs(s);
    //CRLog::trace(" after: '%s'", LCSTR(s));
}

void LVTextFileBase::clearCharBuffer()
{
    m_read_buffer_len = m_read_buffer_pos = 0;
}

int LVTextFileBase::fillCharBuffer()
{
    int available = m_read_buffer_len - m_read_buffer_pos;
    // Don't update if more than 1/8 of buffer filled
    if (available > (XML_CHAR_BUFFER_SIZE>>3))
        return available;
    if (m_buf_len - m_buf_pos < MIN_BUF_DATA_SIZE)
        FillBuffer(MIN_BUF_DATA_SIZE * 2);
    if (m_read_buffer_len > (XML_CHAR_BUFFER_SIZE - (XML_CHAR_BUFFER_SIZE>>3)))
    {
        memcpy(m_read_buffer, m_read_buffer+m_read_buffer_pos, available * sizeof(lChar16));
        m_read_buffer_pos = 0;
        m_read_buffer_len = available;
    }
    int charsRead = ReadChars(m_read_buffer + m_read_buffer_len, XML_CHAR_BUFFER_SIZE - m_read_buffer_len);
    m_read_buffer_len += charsRead;
//#ifdef _DEBUG
//    	CRLog::trace("buf: %s\n", UnicodeToUtf8(lString16(m_read_buffer, m_read_buffer_len)).c_str() );
//#endif
//    	CRLog::trace("Buf:'%s'", LCSTR(lString16(m_read_buffer, m_read_buffer_len)) );
    return m_read_buffer_len - m_read_buffer_pos;
}

bool LvXmlParser::ReadText()
{
    lString16 temp;
    return this->ReadTextToString(temp,true);
    temp.clear();
}

bool LvXmlParser::ReadTextToString(lString16 & output, bool write_to_tree, bool rtl_force_check)
{
    // TODO: remove tracking of file pos
    //int text_start_pos = 0;
    //int ch_start_pos = 0;
    //int last_split_fpos = 0;
    int last_split_txtlen = 0;
    int tlen = 0;
    //text_start_pos = (int)(m_buf_fpos + m_buf_pos);
    m_txt_buf.reset(TEXT_SPLIT_SIZE+1);
    lUInt32 flags = callback_->getFlags();
    bool pre_para_splitting = ( flags & TXTFLG_PRE_PARA_SPLITTING )!=0;
    bool last_eol = false;

    bool flgBreak = false;
    bool splitParas = false;
    while ( !flgBreak ) {
        int i=0;
        if ( m_read_buffer_pos + 1 >= m_read_buffer_len ) {
            if ( !fillCharBuffer() ) {
                eof_ = true;
                return false;
            }
        }
        for ( ; m_read_buffer_pos+i<m_read_buffer_len; i++ ) {
            lChar16 ch = m_read_buffer[m_read_buffer_pos + i];
            lChar16 nextch = m_read_buffer_pos + i + 1 < m_read_buffer_len ? m_read_buffer[m_read_buffer_pos + i + 1] : 0;
            flgBreak = ch=='<' || eof_;
            if ( flgBreak && !tlen ) {
                m_read_buffer_pos++;
                return false;
            }
            splitParas = false;
            if (last_eol && pre_para_splitting && (ch==' ' || ch=='\t' || ch==160) && tlen>0 ) //!!!
                splitParas = true;
            if (!flgBreak && !splitParas)
            {
                tlen++;
            }
            if ( tlen > TEXT_SPLIT_SIZE || flgBreak || splitParas)
            {
                if ( last_split_txtlen==0 || flgBreak || splitParas )
                    last_split_txtlen = tlen;
                break;
            }
            else if (ch==' ' || (ch=='\r' && nextch!='\n')
                || (ch=='\n' && nextch!='\r') )
            {
                //last_split_fpos = (int)(m_buf_fpos + m_buf_pos);
                last_split_txtlen = tlen;
            }
            last_eol = (ch=='\r' || ch=='\n');
        }
        if ( i>0 ) {
            m_txt_buf.append( m_read_buffer + m_read_buffer_pos, i );
            m_read_buffer_pos += i;
        }
        if ( tlen > TEXT_SPLIT_SIZE || flgBreak || splitParas)
        {
            //=====================================================
            lChar16 * buf = m_txt_buf.modify();

            const lChar16 * enc_table = NULL;
            if ( flags & TXTFLG_CONVERT_8BIT_ENTITY_ENCODING )
                enc_table = this->m_conv_table;

            int nlen = PreProcessXmlString(buf, last_split_txtlen, flags, enc_table);
            if ( (flags & TXTFLG_TRIM) && (!(flags & TXTFLG_PRE) || (flags & TXTFLG_PRE_PARA_SPLITTING)) )
            {
                nlen = TrimDoubleSpaces(buf, nlen,
                    ((flags & TXTFLG_TRIM_ALLOW_START_SPACE) || pre_para_splitting)?true:false,
                    (flags & TXTFLG_TRIM_ALLOW_END_SPACE)?true:false,
                    (flags & TXTFLG_TRIM_REMOVE_EOL_HYPHENS)?true:false );
            }

            if(RTL_DISPLAY_ENABLE && rtl_force_check)
            {
                if (lString16(buf,nlen).CheckRTL())
                {
                    //CRLog::error("added rtl for docx");
                    callback_->OnAttribute(L"", L"dir", L"rtl");
                    callback_->setRTLflag(true);
                    gDocumentRTL = 1;
                }
            }
            bool indic = false;
            lString16 processed;

            if(!callback_->reading_binary())
            {
                processed = lString16(buf, nlen);
                bool needsCheck = false;
                for (int i = 0; i < processed.length(); i+=2)
                {
                    if (processed.at(i) > 0x08FF)
                    {
                        needsCheck = true;
                        break;
                    }
                }
                if(needsCheck)
                {
                    processed.checkCJK();
                    processed = processed.processIndicText(&indic);
                    if (indic)
                    {
                        buf = processed.modify();
                        nlen = processed.length();
                    }
                }
            }
            if (flags & TXTFLG_PRE) {
                // check for tabs
                int tabCount = CalcTabCount(buf, nlen);
                if ( tabCount > 0 ) {
                    // expand tabs
                    lString16 tmp;
                    tmp.reserve(nlen + tabCount * 8);
                    ExpandTabs(tmp, buf, nlen);
                    if(write_to_tree)
                    {
                        callback_->OnText(tmp.c_str(), tmp.length(), flags);
                    }
                    output = tmp;
                } else {
                    if(write_to_tree)
                    {
                        callback_->OnText(buf, nlen, flags);
                    }
                    output = (indic) ? processed : buf;
                }
            } else {
                if(write_to_tree)
                {
                    callback_->OnText(buf, nlen, flags);
                }
                output = (indic) ? processed : buf;
            }

            m_txt_buf.erase(0, last_split_txtlen);
            tlen = m_txt_buf.length();
            last_split_txtlen = 0;

            //=====================================================
            if (flgBreak)
            {
                // TODO:LVE???
                if ( PeekCharFromBuffer()=='<' )
                    m_read_buffer_pos++;
                //if ( m_read_buffer_pos < m_read_buffer_len )
                //    m_read_buffer_pos++;
                break;
            }
            //text_start_pos = last_split_fpos; //m_buf_fpos + m_buf_pos;
            //last_split_fpos = 0;
        }
    }


    //if (!Eof())
    //    m_buf_pos++;
    return (!eof_);
}

bool LvXmlParser::SkipSpaces()
{
    for ( lUInt16 ch = PeekCharFromBuffer(); !eof_; ch = PeekNextCharFromBuffer() ) {
        if ( !IsSpaceChar(ch) )
            break; // char found!
    }
    return (!eof_);
}

bool LvXmlParser::SkipTillChar(lChar16 charToFind)
{
    for (lUInt16 ch = PeekCharFromBuffer(); !eof_; ch = PeekNextCharFromBuffer()) {
        if (ch == charToFind)
            return true; // char found!
    }
    return false; // EOF
}

inline bool isValidIdentChar(lChar16 ch)
{
    return ((ch>='a' && ch<='z')
          || (ch>='A' && ch<='Z')
          || (ch>='0' && ch<='9')
          || (ch=='-')
          || (ch=='_')
          || (ch=='.')
          || (ch==':'));
}

inline bool isValidFirstIdentChar(lChar16 ch)
{
    return ((ch>='a' && ch<='z') || (ch>='A' && ch<='Z'));
}

// read identifier from stream
bool LvXmlParser::ReadIdent(lString16 & ns, lString16 & name)
{
    // clear string buffer
    ns.reset(16);
    name.reset(16);
    // check first char
    lChar16 ch0 = PeekCharFromBuffer();
    if (!isValidFirstIdentChar(ch0))
        return false;

    name += ReadCharFromBuffer();

    for ( lUInt16 ch = PeekCharFromBuffer(); !eof_; ch = PeekNextCharFromBuffer() ) {
        if (!isValidIdentChar(ch))
            break;
        if (ch == ':')
        {
            if (ns.empty())
                name.swap(ns); // add namespace
            else
                break; // error
        }
        else
        {
            name += ch;
        }
    }
    lChar16 ch = PeekCharFromBuffer();
    return (!name.empty()) && (ch==' ' || ch=='/' || ch=='>' || ch=='?' || ch=='=' || ch==0 || ch == '\r' || ch == '\n');
}

void LvXmlParser::SetSpaceMode(bool flgTrimSpaces)
{
    m_trimspaces = flgTrimSpaces;
}

LvXmlParser::LvXmlParser(LVStreamRef stream, LvXMLParserCallback* callback,bool allowHtml, bool fb2Only, bool need_coverpage)
        : LVTextFileBase(stream),
          callback_(callback),
          m_trimspaces(true),
          m_state(0),
          possible_capitalized_tags_(false),
          m_allowHtml(allowHtml),
          m_fb2Only(fb2Only) {
    this->need_coverpage_= need_coverpage;
}

LvXmlParser::~LvXmlParser() {}

void LvXmlParser::setEpubNotes(EpubItems epubItems)
{
    EpubNotes_ = new EpubItems(epubItems);
    if(epubItems.length()>0)
    {
        Notes_exists = true;
    }
}

void LvXmlParser::setLinksList(LVArray<LinkStruct> LinksList)
{
    LinksList_ = LinksList;
}

LVArray<LinkStruct> LvXmlParser::getLinksList()
{
    return LinksList_;
}

void LvXmlParser::setLinksMap(LinksMap LinksMap)
{
    LinksMap_ = LinksMap;
}

LinksMap LvXmlParser::getLinksMap()
{
    return LinksMap_;
}

void LvXmlParser::setEpub3Notes(Epub3Notes Epub3Notes)
{
    Epub3Notes_ = Epub3Notes;
}

Epub3Notes LvXmlParser::getEpub3Notes()
{
    return Epub3Notes_;
}

void LvXmlParser::setStylesManager(EpubStylesManager manager)
{
    EpubStylesManager_ = manager;
}

EpubStylesManager LvXmlParser::getStylesManager()
{
    return EpubStylesManager_;
}

lString16 htmlCharset(lString16 htmlHeader)
{
    // META HTTP-EQUIV
    htmlHeader.lowercase();
    int p1 = htmlHeader.pos("charset=");
    int p2 = htmlHeader.rpos("charset="); //search from behind
    if (p1 != p2)
    {
        //LE("multiple charsets detected!");
        return lString16::empty_str;
    }

    lString16 meta("meta http-equiv=\"content-type\"");
    int p = htmlHeader.pos( meta );
    if ( p<0 )
        return lString16::empty_str;
    htmlHeader = htmlHeader.substr( p + meta.length() );
    p = htmlHeader.pos(">");
    if ( p<0 )
        return lString16::empty_str;
    htmlHeader = htmlHeader.substr( 0, p );
    // Commented out because of spam
    //CRLog::trace("http-equiv content-type: %s", UnicodeToUtf8(htmlHeader).c_str());
    p = htmlHeader.pos("charset=");
    if ( p<0 )
        return lString16::empty_str;
    htmlHeader = htmlHeader.substr( p + 8 ); // skip "charset="
    lString16 enc;
    for ( int i=0; i<(int)htmlHeader.length(); i++ ) {
        lChar16 ch = htmlHeader[i];
        if ( (ch>='a' && ch<='z') || (ch>='0' && ch<='9') || (ch=='-') || (ch=='_') )
            enc += ch;
        else
            break;
    }
    if (enc == "utf-16")
        return lString16::empty_str;
    return enc;
}

bool LvHtmlParser::CheckFormat()
{
    Reset();
    if (!AutodetectEncoding(!this->m_encoding_name.empty()))
    {
        return false;
    }
    lChar16 *chbuf = new lChar16[XML_PARSER_DETECT_SIZE];
    FillBuffer(XML_PARSER_DETECT_SIZE);
    int charsDecoded = ReadTextBytes(0, m_buf_len, chbuf, XML_PARSER_DETECT_SIZE - 1, 0);
    chbuf[charsDecoded] = 0;
    bool res = false;
    if (charsDecoded > 30)
    {
        lString16 s(chbuf, charsDecoded);
        s.lowercase();
        if (s.pos("<html") >= 0 && (s.pos("<head") >= 0 || s.pos("<body") >=0))
        {
            res = true;
        }
        else
        {
            lString16 name = m_stream->GetName();
            name.lowercase();
            bool html_ext = name.endsWith(".htm")
                            || name.endsWith(".html")
                            || name.endsWith(".hhc")
                            || name.endsWith(".xhtml");
            if (html_ext && (s.pos("<!--") >= 0|| s.pos("<p>") >= 0 || s.pos("<UL>") >= 0 || s.pos("<ul>") >= 0))
            {
                res = true;
            }
        }
        lString16 enc = htmlCharset(s);
        if (!enc.empty())
        {
            SetCharset(enc.c_str());
        }
        //else if ( s.pos("<html xmlns=\"http://www.w3.org/1999/xhtml\"") >= 0 )
        //    res = true;
    }
    delete[] chbuf;
    Reset();
    return res;
}

LvHtmlParser::LvHtmlParser(LVStreamRef stream, LvXMLParserCallback* callback)
		: LvXmlParser(stream, callback) {
    possible_capitalized_tags_ = true;
}

LvHtmlParser::LvHtmlParser(LVStreamRef stream, LvXMLParserCallback* callback , bool need_coverpage)
		: LvXmlParser(stream, callback , true, false, need_coverpage) {
	possible_capitalized_tags_ = true;
    this-> need_coverpage_ = need_coverpage ;
}
LvHtmlParser::~LvHtmlParser()
{
}

bool LvHtmlParser::Parse()
{
    return LvXmlParser::Parse();
}

bool LvHtmlParser::ParseEpubFootnotes()
{
    return LvXmlParser::ParseEpubFootnotes();
}

bool LvHtmlParser::ParseDocx(DocxItems docxItems, DocxLinks docxLinks,DocxStyles docxStyles)
{
    return LvXmlParser::ParseDocx(docxItems, docxLinks, docxStyles);
}

bool LvHtmlParser::ParseOdt(/*DocxItems docxItems, DocxLinks docxLinks,*/OdtStyles odtStyles)
{
    return LvXmlParser::ParseOdt(/*docxItems, docxLinks,*/ odtStyles);
}

void LvXmlParser::setFb3Relationships(std::map<lUInt32,lString16> map)
{
    fb3RelsMap_ = map;
}

std::map<lUInt32,lString16> LvXmlParser::getFb3Relationships()
{
    return fb3RelsMap_;
}

/// read file contents to string
lString16 LVReadCssText(lString16 filename)
{
	LVStreamRef stream = LVOpenFileStream(filename.c_str(), LVOM_READ);
	return LVReadCssText(stream);
}

lString16 LVReadCssText(LVStreamRef stream)
{
	if (stream.isNull())
        return lString16::empty_str;
    lString16 buf;
    LVTextParser reader(stream, NULL, false, false);
    if (!reader.AutodetectEncoding())
        return buf;
    lUInt32 flags;
    while (!reader.Eof()) {
        lString16 line = reader.ReadLine(4096, flags);
        if (!buf.empty())
            buf << L'\n';
        if (!line.empty()) {
            buf << line;
        }
    }
    return buf;
}

static const char * AC_P[]  = {"p", "p", "hr", NULL};
static const char * AC_COL[] = {"col", NULL};
static const char * AC_LI[] = {"li", "li", "p", NULL};
static const char * AC_UL[] = {"ul", "li", "p", NULL};
static const char * AC_OL[] = {"ol", "li", "p", NULL};
static const char * AC_DD[] = {"dd", "dd", "p", NULL};
static const char * AC_DL[] = {"dl", "dt", "p", NULL};
static const char * AC_DT[] = {"dt", "dt", "dd", "p", NULL};
static const char * AC_BR[] = {"br", NULL};
static const char * AC_HR[] = {"hr", NULL};
static const char * AC_PARAM[] = {"param", "param", NULL};
static const char * AC_IMG[]= {"img", NULL};
static const char * AC_TD[] = {"td", "td", "th", NULL};
static const char * AC_TH[] = {"th", "th", "td", NULL};
static const char * AC_TR[] = {"tr", "tr", "thead", "tfoot", "tbody", NULL};
static const char * AC_DIV[] = {"div", "p", NULL};
static const char * AC_TABLE[] = {"table", "p", NULL};
static const char * AC_THEAD[] = {"thead", "tr", "thead", "tfoot", "tbody", NULL};
static const char * AC_TFOOT[] = {"tfoot", "tr", "thead", "tfoot", "tbody", NULL};
static const char * AC_TBODY[] = {"tbody", "tr", "thead", "tfoot", "tbody", NULL};
static const char * AC_OPTION[] = {"option", "option", NULL};
static const char * AC_PRE[] = {"pre", "pre", NULL};
static const char * AC_INPUT[] = {"input", NULL};
const char * *
HTML_AUTOCLOSE_TABLE[] = {
    AC_INPUT,
    AC_OPTION,
    AC_PRE,
    AC_P,
    AC_LI,
    AC_UL,
    AC_OL,
    AC_TD,
    AC_TH,
    AC_DD,
    AC_DL,
    AC_DT,
    AC_TR,
    AC_COL,
    AC_BR,
    AC_HR,
    AC_PARAM,
    AC_IMG,
    AC_DIV,
    AC_THEAD,
    AC_TFOOT,
    AC_TBODY,
    AC_TABLE,
    NULL
};


// base64 decode table
static const signed char base64_decode_table[] = {
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //0..15
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, //16..31   10
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63, //32..47   20
   52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1, //48..63   30
   -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14, //64..79   40
   15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1, //80..95   50
   -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40, //INDEX2..111  60
   41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1  //112..127 70
};

#define BASE64_BUF_SIZE 128
class LVBase64Stream : public LVNamedStream
{
private:
    lString8    m_curr_text;
    int         m_text_pos;
    lvsize_t    m_size;
    lvpos_t     m_pos;

    int         m_iteration;
    lUInt32     m_value;

    lUInt8      m_bytes[BASE64_BUF_SIZE];
    int         m_bytes_count;
    int         m_bytes_pos;

    int readNextBytes()
    {
        int bytesRead = 0;
        bool flgEof = false;
        while ( bytesRead == 0 && !flgEof )
        {
            while ( m_text_pos >= (int)m_curr_text.length() )
            {
                return bytesRead;
            }
            int len = m_curr_text.length();
            const lChar8 * txt = m_curr_text.c_str();
            for ( ; m_text_pos<len && m_bytes_count < BASE64_BUF_SIZE - 3; m_text_pos++ )
            {
                lChar16 ch = txt[ m_text_pos ];
                if ( ch < 128 )
                {
                    if ( ch == '=' )
                    {
                        // end of stream
                        if ( m_iteration == 2 )
                        {
                            m_bytes[m_bytes_count++] = (lUInt8)((m_value>>4) & 0xFF);
                            bytesRead++;
                        }
                        else if ( m_iteration == 3 )
                        {
                            m_bytes[m_bytes_count++] = (lUInt8)((m_value>>10) & 0xFF);
                            m_bytes[m_bytes_count++] = (lUInt8)((m_value>>2) & 0xFF);
                            bytesRead += 2;
                        }
                        // stop!!!
                        //m_text_pos--;
                        m_iteration = 0;
                        flgEof = true;
                        break;
                    }
                    else
                    {
                        int k = base64_decode_table[ch];
                        if ( !(k & 0x80) ) {
                            // next base-64 digit
                            m_value = (m_value << 6) | (k);
                            m_iteration++;
                            if (m_iteration==4)
                            {
                                //
                                m_bytes[m_bytes_count++] = (lUInt8)((m_value>>16) & 0xFF);
                                m_bytes[m_bytes_count++] = (lUInt8)((m_value>>8) & 0xFF);
                                m_bytes[m_bytes_count++] = (lUInt8)((m_value>>0) & 0xFF);
                                m_iteration = 0;
                                m_value = 0;
                                bytesRead+=3;
                            }
                        } else {
                            //m_text_pos++;
                        }
                    }
                }
            }
        }
        return bytesRead;
    }

    int bytesAvailable() { return m_bytes_count - m_bytes_pos; }

    bool rewind()
    {
        m_pos = 0;
        m_bytes_count = 0;
        m_bytes_pos = 0;
        m_iteration = 0;
        m_value = 0;
        m_text_pos = 0;
        return m_text_pos < m_curr_text.length();
    }

    bool skip( lvsize_t count )
    {
        while ( count )
        {
            if ( m_bytes_pos >= m_bytes_count )
            {
                m_bytes_pos = 0;
                m_bytes_count = 0;
                int bytesRead = readNextBytes();
                if ( bytesRead == 0 )
                    return false;
            }
            int diff = (int) (m_bytes_count - m_bytes_pos);
            if (diff > (int)count)
                diff = (int)count;
            m_pos += diff;
            count -= diff;
        }
        return true;
    }

public:
    virtual ~LVBase64Stream() { }
    LVBase64Stream(lString8 data)
        : m_curr_text(data), m_size(0), m_pos(0)
    {
        // calculate size
        rewind();
        m_size = bytesAvailable();
        for (;;) {
            int bytesRead = readNextBytes();
            if ( !bytesRead )
                break;
            m_bytes_count = 0;
            m_bytes_pos = 0;
            m_size += bytesRead;
        }
        // rewind
        rewind();
    }
    virtual bool Eof()
    {
        return m_pos >= m_size;
    }
    virtual lvsize_t  GetSize()
    {
        return m_size;
    }

    virtual lvpos_t GetPos()
    {
        return m_pos;
    }

    virtual lverror_t GetPos( lvpos_t * pos )
    {
        if (pos)
            *pos = m_pos;
        return LVERR_OK;
    }

    virtual lverror_t Seek(lvoffset_t offset, lvseek_origin_t origin, lvpos_t* newPos)
    {
        lvpos_t npos = 0;
        lvpos_t currpos = GetPos();
        switch (origin) {
        case LVSEEK_SET:
            npos = offset;
            break;
        case LVSEEK_CUR:
            npos = currpos + offset;
            break;
        case LVSEEK_END:
            npos = m_size + offset;
            break;
        }
        if (npos > m_size)
            return LVERR_FAIL;
        if ( npos != currpos )
        {
            if (npos < currpos)
            {
                if ( !rewind() || !skip(npos) )
                    return LVERR_FAIL;
            }
            else
            {
                skip( npos - currpos );
            }
        }
        if (newPos)
            *newPos = npos;
        return LVERR_OK;
    }
    virtual lverror_t Write(const void*, lvsize_t, lvsize_t*)
    {
        return LVERR_NOTIMPL;
    }
    virtual lverror_t Read(void* buf, lvsize_t size, lvsize_t* pBytesRead)
    {
        lvsize_t bytesRead = 0;
        //fprintf( stderr, "Read()\n" );

        lUInt8 * out = (lUInt8 *)buf;

        while (size>0)
        {
            int sz = bytesAvailable();
            if (!sz) {
                m_bytes_pos = m_bytes_count = 0;
                sz = readNextBytes();
                if (!sz) {
                    if ( !bytesRead || m_pos!=m_size) //
                        return LVERR_FAIL;
                    break;
                }
            }
            if (sz>(int)size)
                sz = (int)size;
            for (int i=0; i<sz; i++)
                *out++ = m_bytes[m_bytes_pos++];
            size -= sz;
            bytesRead += sz;
            m_pos += sz;
        }

        if (pBytesRead)
            *pBytesRead = bytesRead;
        //fprintf( stderr, "    %d bytes read...\n", (int)bytesRead );
        return LVERR_OK;
    }
    virtual lverror_t SetSize(lvsize_t)
    {
        return LVERR_NOTIMPL;
    }
};

/// XML parser callback interface
class FB2CoverpageParserCallback : public LvXMLParserCallback
{
protected:
    LVFileFormatParser * _parser;
    bool insideFictionBook;
    bool insideDescription;
    bool insideTitleInfo;
    bool insideCoverpage;
    bool insideImage;
    bool insideBinary;
    bool insideCoverBinary;
    bool coverDone;
    int tagCounter;
    lString16 binaryId;
    lString8 data;
public:
    ///
    bool fb2_cover_done() { return coverDone; }
    bool reading_binary() { return true; }

    FB2CoverpageParserCallback()
    {
        insideFictionBook = false;
        insideDescription = false;
        insideTitleInfo = false;
        insideCoverpage = false;
        insideImage = false;
        insideBinary = false;
        tagCounter = 0;
        insideCoverBinary = false;
        coverDone = false;
    }
    virtual lUInt32 getFlags() { return TXTFLG_PRE; }
    /// called on parsing start
    virtual void OnStart(LVFileFormatParser * parser)
    {
        _parser = parser;
        parser->SetSpaceMode(false);
    }
    /// called on parsing end
    virtual void OnStop()
    {
    }
    /// called on opening tag end
    virtual void OnTagBody()
    {
    }
    /// add named BLOB data to document
    virtual bool OnBlob(lString16 /*name*/, const lUInt8 * /*data*/, int /*size*/) { return true; }
    /// called on opening tag
    virtual ldomNode * OnTagOpen( const lChar16 * /*nsname*/, const lChar16 * tagname)
    {
        tagCounter++;
        if (!insideFictionBook && tagCounter > 5) {
            _parser->Stop();
            return NULL;
        }
        if ( lStr_cmp(tagname, "FictionBook")==0) {
            insideFictionBook = true;
        } else if ( lStr_cmp(tagname, "description")==0 && insideFictionBook) {
            insideDescription = true;
        } else if ( lStr_cmp(tagname, "title-info")==0 && insideDescription) {
            insideTitleInfo = true;
        } else if ( lStr_cmp(tagname, "coverpage")==0 && insideTitleInfo) {
            insideCoverpage =  true;
        } else if ( lStr_cmp(tagname, "image")==0 && insideCoverpage) {
            insideImage = true;
        } else if ( lStr_cmp(tagname, "binary")==0 && insideFictionBook) {
            insideBinary = true;
            return NULL;
        } else if ( lStr_cmp(tagname, "body")==0 && binaryId.empty()) {
            _parser->Stop();
            // NO Image ID specified
            return NULL;
        }
        insideCoverBinary = false;
        return NULL;
    }
    /// called on closing
    virtual void OnTagClose( const lChar16 * nsname, const lChar16 * tagname )
    {
        if ( lStr_cmp(nsname, "FictionBook")==0) {
            insideFictionBook = false;
        } else if ( lStr_cmp(tagname, "description")==0) {
            insideDescription = false;
        } else if ( lStr_cmp(tagname, "title-info")==0) {
            insideTitleInfo = false;
        } else if ( lStr_cmp(tagname, "coverpage")==0) {
            insideCoverpage =  false;
        } else if ( lStr_cmp(tagname, "image")==0) {
            insideImage = false;
        } else if ( lStr_cmp(tagname, "binary")==0) {
            insideBinary = false;
            insideCoverBinary = false;
        }
    }
    /// called on element attribute
    virtual void OnAttribute( const lChar16 * /*nsname*/, const lChar16 * attrname, const lChar16 * attrvalue )
    {
        if (lStr_cmp(attrname, "href")==0 && insideImage) {
            lString16 s(attrvalue);
            if (s.startsWith("#")) {
                binaryId = s.substr(1);
                //CRLog::trace("found FB2 cover ID");
            }
        } else if (lStr_cmp(attrname, "id")==0 && insideBinary) {
            lString16 id(attrvalue);
            if (!id.empty() && id == binaryId) {
                insideCoverBinary = true;
                //CRLog::trace("found FB2 cover data");
            }
        } else if (lStr_cmp(attrname, "page")==0) {
        }
    }
    /// called on text
    virtual void OnText( const lChar16 * text, int len, lUInt32 /*flags*/ )
    {
        if (!insideCoverBinary)
            return;
        lString16 txt( text, len );
        data.append(UnicodeToUtf8(txt));
        coverDone = true;
    }
    /// destructor
    virtual ~FB2CoverpageParserCallback()
    {
    }
    LVStreamRef getStream() {
        static lUInt8 fake_data[1] = {0};
        if (data.length() == 0) {
            return LVCreateMemoryStream(fake_data, 0, false);
        }
        LVStreamRef stream = LVStreamRef(new LVBase64Stream(data));
        LVStreamRef res = LVCreateMemoryStream(stream);
        return res;
    }
};

LVStreamRef GetFB2Coverpage(LVStreamRef stream)
{
    FB2CoverpageParserCallback callback;
    LvXmlParser parser(stream, &callback, false, true);
    if (!parser.CheckFormat()) {
        stream->SetPos(0);
		return LVStreamRef();
	}
    parser.Parse();
    LVStreamRef res = callback.getStream();
    stream->SetPos(0);
    return res;
}
