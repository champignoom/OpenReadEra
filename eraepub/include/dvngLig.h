//
// Created by Tarasus on 27/8/2020.
//

#ifndef _DVNGLIG_H
#define _DVNGLIG_H

#include <map>

//virtual charcodes range helpers
#define DEVANAGARI_START 0xE001
#define DEVANAGARI_END   0xE1C5
//from 0xE1C6 to 0xE200 is not reserved space
#define BANGLA_START     0xE201
#define BANGLA_END       0xE50F
//from 0xE50F to 0xE520 is not reserved space
#define MALAY_START     0xE520
#define MALAY_END       0xE5DF
//from 0xE5DF to 0xE600 is not reserved space
#define KANNADA_START   0xE600
#define KANNADA_END     0xE72F
//no gap
#define TAMIL_START     0xE730
#define TAMIL_END       0xE780
//from 0xE780 to 0xE790 is not reserved space
#define TELUGU_START    0xE790
#define TELUGU_END      0xE99F
//no gap
#define GUJARATI_START  0xEA00
#define GUJARATI_END    0xEC7F
//no gap
#define ORIYA_START     0xEC80
#define ORIYA_END       0xEEA0

class dvngLig
{
public:
    lChar16 a = 0;
    lChar16 b = 0;
    lChar16 c = 0;
    lChar16 d = 0;
    lChar16 e = 0;
    lChar16 f = 0;
    lChar16 g = 0;
    lChar16 h = 0;
    lChar16 i = 0;
    lChar16 j = 0;
    lUInt32 key_hash;
    int len = 0;
    int glyphindex = -1; //index of glyph in current font

    bool banglaRa = false; //last char of ligature is 0x09CD virama (in bengali it is Ra)

    dvngLig(){};

    dvngLig(int glyphIndex, std::string str);

    dvngLig(std::string str);

    dvngLig(lString16 str);

    bool operator == (const dvngLig& rhs);

    bool isNull();

    lString16 restoreToChars();
private:
    std::vector<std::string> parse(const std::string &s, char delimeter);

    void parseStdStr(std::string str);

};

struct Comparator
{
    using is_transparent = std::true_type;

    // standard comparison (between two instances of dvngLig)
    bool operator()(const dvngLig& lhs, const dvngLig& rhs) const
    {
        //LE("l = %d r = %d",lhs.key_hash, rhs.key_hash);
        return lhs.key_hash < rhs.key_hash;
    }
};

typedef std::map<lChar16 , dvngLig> LigMap;
typedef std::map<dvngLig , lChar16, Comparator> LigMapRev;
typedef std::map<lUInt32 , int> FastLigMap;

extern LigMap     gCurrentLigMap;
extern LigMapRev  gCurrentLigMapRev;
extern FastLigMap gCurrentFastLigMap;

LigMapRev makeReverseLigMap(LigMap in, FastLigMap* in_fast);
int findCurrMapLigGlyphIndex(int ligature);

#endif //_DVNGLIG_H
