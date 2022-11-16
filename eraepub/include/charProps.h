//
// Created by Tarasus on 09.09.2020.
//

#ifndef _CHARPROPS_H
#define _CHARPROPS_H

#include "lvtypes.h"

#define CH_PROP_UPPER       0x0001 ///< uppercase alpha character flag
#define CH_PROP_LOWER       0x0002 ///< lowercase alpha character flag
#define CH_PROP_ALPHA       0x0003 ///< alpha flag is combination of uppercase and lowercase flags
#define CH_PROP_DIGIT       0x0004 ///< digit character flag
#define CH_PROP_PUNCT       0x0008 ///< pubctuation character flag
#define CH_PROP_SPACE       0x0010 ///< space character flag
#define CH_PROP_HYPHEN      0x0020 ///< hyphenation character flag
#define CH_PROP_VOWEL       0x0040 ///< vowel character flag
#define CH_PROP_CONSONANT   0x0080 ///< consonant character flag
#define CH_PROP_SIGN        0x0100 ///< sign character flag
#define CH_PROP_ALPHA_SIGN  0x0200 ///< alpha sign character flag
#define CH_PROP_DASH        0x0400 ///< minus, emdash, endash, ... (- signs)
#define CH_PROP_HIEROGLYPH  0x0800 ///< all symbols that are not in upper tags


extern const char * russian_capital[32];
extern const char * russian_small[32];
extern const char * latin_1[64];
extern const lUInt16 char_props[];
extern const size_t char_props_count;

extern const lUInt16 char_props_1f00[];
extern const size_t char_props_1f00_count;

#endif //_CHARPROPS_H
