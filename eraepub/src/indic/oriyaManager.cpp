//
// Created by Tarasus on 16.10.2020.
//

#include "include/indic/oriyaManager.h"
#include "ore_log.h"
#include <vector>

LigMap     gOriyaLigMap;
LigMapRev  gOriyaLigMapRev;
FastLigMap gOriyaFastLigMap;

bool CharIsOriya(int ch)
{
    return (ch >= 0x0B00 && ch <= 0x0B7F);
}

bool lString16::CheckOriya()
{
    if(gDocumentOriya == 1)
    {
        return true;
    }
    int increment = 5; // default
    if (this->length() <= 10)
    {
        increment = 2;
    }
    for (int i = 0; i < this->length(); i += increment)
    {
        int ch = this->at(i);
        if (CharIsOriya(ch))
        {
            gDocumentOriya = 1;
            gDocumentINDIC = 1;

            if(gOriyaLigMapRev.empty())
            {
                gOriyaLigMapRev = OriyaLigMapReversed();
            }
            return true;
        }
    }
    return false;
}

dvngLig findOriyaLig(lChar16 ligature)
{
    if(gOriyaLigMap.empty())
    {
        gOriyaLigMap = GetOriyaLigMap();
    }
    auto it = gOriyaLigMap.find(ligature);
    if(it==gOriyaLigMap.end())
    {
        return dvngLig();
    }
    return it->second;
}

lChar16 findOriyaLigGlyphIndex(lChar16 ligature)
{
    auto it = gOriyaLigMap.find(ligature);
    if(it==gOriyaLigMap.end())
    {
        return 0;
    }
    return it->second.glyphindex;
}

lChar16 findOriyaLigRev(dvngLig combo)
{
    if(gOriyaLigMapRev.empty())
    {
        gOriyaLigMapRev = OriyaLigMapReversed();
    }
    if(combo.len < 2 || combo.len > 10 )
    {
        return 0;
    }
    auto it = gOriyaLigMapRev.find(combo);
    if(it==gOriyaLigMapRev.end())
    {
        return 0;
    }
    //LE("findOriyaLigRev return %d", it->second);
    return it->second;
}

std::map <dvngLig, lChar16, Comparator> OriyaLigMapReversed()
{
    if(!gOriyaLigMapRev.empty())
    {
        return gOriyaLigMapRev;
    }
    if(gOriyaLigMap.empty())
    {
        gOriyaLigMap = GetOriyaLigMap();
    }
    gOriyaLigMapRev = makeReverseLigMap(gOriyaLigMap,&gOriyaFastLigMap);
    return gOriyaLigMapRev;
}

LigMap GetOriyaLigMap()
{
    if(!gOriyaLigMap.empty())
    {
        return gOriyaLigMap;
    }
    //ORIYA_START 0xEC80
    {
        //from Noto Sans Oriya.ttf
        gOriyaLigMap.insert(std::make_pair( 0xEC80, dvngLig( 69 ,"0x0B21 0x0B3C"))); // 0x0B5C
        gOriyaLigMap.insert(std::make_pair( 0xEC81, dvngLig( 70 ,"0x0B22 0x0B3C"))); // 0x0B5D
        gOriyaLigMap.insert(std::make_pair( 0xEC82, dvngLig( 94 ,"0x0B15 0x0B3C"))); // 0x10001
        gOriyaLigMap.insert(std::make_pair( 0xEC83, dvngLig( 95 ,"0x0B16 0x0B3C"))); // 0x10002
        gOriyaLigMap.insert(std::make_pair( 0xEC84, dvngLig( 96 ,"0x0B17 0x0B3C"))); // 0x10003
        gOriyaLigMap.insert(std::make_pair( 0xEC85, dvngLig( 97 ,"0x0B18 0x0B3C"))); // 0x10004
        gOriyaLigMap.insert(std::make_pair( 0xEC86, dvngLig( 98 ,"0x0B19 0x0B3C"))); // 0x10005
        gOriyaLigMap.insert(std::make_pair( 0xEC87, dvngLig( 99 ,"0x0B1A 0x0B3C"))); // 0x10006
        gOriyaLigMap.insert(std::make_pair( 0xEC88, dvngLig( 100 ,"0x0B1B 0x0B3C"))); // 0x10007
        gOriyaLigMap.insert(std::make_pair( 0xEC89, dvngLig( 101 ,"0x0B1C 0x0B3C"))); // 0x10008
        gOriyaLigMap.insert(std::make_pair( 0xEC8A, dvngLig( 102 ,"0x0B1D 0x0B3C"))); // 0x10009
        gOriyaLigMap.insert(std::make_pair( 0xEC8B, dvngLig( 103 ,"0x0B1E 0x0B3C"))); // 0x1000A
        gOriyaLigMap.insert(std::make_pair( 0xEC8C, dvngLig( 104 ,"0x0B1F 0x0B3C"))); // 0x1000B
        gOriyaLigMap.insert(std::make_pair( 0xEC8D, dvngLig( 105 ,"0x0B20 0x0B3C"))); // 0x1000C
        gOriyaLigMap.insert(std::make_pair( 0xEC8E, dvngLig( 106 ,"0x0B23 0x0B3C"))); // 0x1000D
        gOriyaLigMap.insert(std::make_pair( 0xEC8F, dvngLig( 107 ,"0x0B24 0x0B3C"))); // 0x1000E
        gOriyaLigMap.insert(std::make_pair( 0xEC90, dvngLig( 108 ,"0x0B25 0x0B3C"))); // 0x1000F
        gOriyaLigMap.insert(std::make_pair( 0xEC91, dvngLig( 109 ,"0x0B26 0x0B3C"))); // 0x10010
        gOriyaLigMap.insert(std::make_pair( 0xEC92, dvngLig( 110 ,"0x0B27 0x0B3C"))); // 0x10011
        gOriyaLigMap.insert(std::make_pair( 0xEC93, dvngLig( 111 ,"0x0B28 0x0B3C"))); // 0x10012
        gOriyaLigMap.insert(std::make_pair( 0xEC94, dvngLig( 112 ,"0x0B2A 0x0B3C"))); // 0x10013
        gOriyaLigMap.insert(std::make_pair( 0xEC95, dvngLig( 113 ,"0x0B2B 0x0B3C"))); // 0x10014
        gOriyaLigMap.insert(std::make_pair( 0xEC96, dvngLig( 114 ,"0x0B2C 0x0B3C"))); // 0x10015
        gOriyaLigMap.insert(std::make_pair( 0xEC97, dvngLig( 115 ,"0x0B2D 0x0B3C"))); // 0x10016
        gOriyaLigMap.insert(std::make_pair( 0xEC98, dvngLig( 116 ,"0x0B2E 0x0B3C"))); // 0x10017
        gOriyaLigMap.insert(std::make_pair( 0xEC99, dvngLig( 117 ,"0x0B2F 0x0B3C"))); // 0x10018
        gOriyaLigMap.insert(std::make_pair( 0xEC9A, dvngLig( 118 ,"0x0B30 0x0B3C"))); // 0x10019
        gOriyaLigMap.insert(std::make_pair( 0xEC9B, dvngLig( 119 ,"0x0B32 0x0B3C"))); // 0x1001A
        gOriyaLigMap.insert(std::make_pair( 0xEC9C, dvngLig( 120 ,"0x0B33 0x0B3C"))); // 0x1001B
        gOriyaLigMap.insert(std::make_pair( 0xEC9D, dvngLig( 121 ,"0x0B35 0x0B3C"))); // 0x1001C
        gOriyaLigMap.insert(std::make_pair( 0xEC9E, dvngLig( 122 ,"0x0B36 0x0B3C"))); // 0x1001D
        gOriyaLigMap.insert(std::make_pair( 0xEC9F, dvngLig( 123 ,"0x0B37 0x0B3C"))); // 0x1001E
        gOriyaLigMap.insert(std::make_pair( 0xECA0, dvngLig( 124 ,"0x0B38 0x0B3C"))); // 0x1001F
        gOriyaLigMap.insert(std::make_pair( 0xECA1, dvngLig( 125 ,"0x0B39 0x0B3C"))); // 0x10020
        gOriyaLigMap.insert(std::make_pair( 0xECA2, dvngLig( 126 ,"0x0B5F 0x0B3C"))); // 0x10021
        gOriyaLigMap.insert(std::make_pair( 0xECA3, dvngLig( 127 ,"0x0B71 0x0B3C"))); // 0x10022
        gOriyaLigMap.insert(std::make_pair( 0xECA4, dvngLig( 128 ,"0x0B30 0x0B4D"))); // 0x10023 //ra virama reph
        //gOriyaLigMap.insert(std::make_pair( 0xECA5, dvngLig( 129 ,"0x0B4D 0x0B15"))); // 0x10024
        gOriyaLigMap.insert(std::make_pair( 0xECA6, dvngLig( 129 ,"0x0B15 0x0B4D"))); // 0x10024
        //gOriyaLigMap.insert(std::make_pair( 0xECA7, dvngLig( 130 ,"0x0B4D 0x0B16"))); // 0x10025
        gOriyaLigMap.insert(std::make_pair( 0xECA8, dvngLig( 130 ,"0x0B16 0x0B4D"))); // 0x10025
        //gOriyaLigMap.insert(std::make_pair( 0xECA9, dvngLig( 131 ,"0x0B4D 0x0B17"))); // 0x10026
        gOriyaLigMap.insert(std::make_pair( 0xECAA, dvngLig( 131 ,"0x0B17 0x0B4D"))); // 0x10026
        gOriyaLigMap.insert(std::make_pair( 0xECAB, dvngLig( 132 ,"0x0B4D 0x0B18"))); // 0x10027
        gOriyaLigMap.insert(std::make_pair( 0xECAC, dvngLig( 132 ,"0x0B18 0x0B4D"))); // 0x10027
        //gOriyaLigMap.insert(std::make_pair( 0xECAD, dvngLig( 133 ,"0x0B4D 0x0B19"))); // 0x10028
        gOriyaLigMap.insert(std::make_pair( 0xECAE, dvngLig( 133 ,"0x0B19 0x0B4D"))); // 0x10028
        gOriyaLigMap.insert(std::make_pair( 0xECAF, dvngLig( 134 ,"0x0B4D 0x0B1A"))); // 0x10029
        gOriyaLigMap.insert(std::make_pair( 0xECB0, dvngLig( 134 ,"0x0B1A 0x0B4D"))); // 0x10029
        //gOriyaLigMap.insert(std::make_pair( 0xECB1, dvngLig( 135 ,"0x0B4D 0x0B1B"))); // 0x1002A
        gOriyaLigMap.insert(std::make_pair( 0xECB2, dvngLig( 135 ,"0x0B1B 0x0B4D"))); // 0x1002A
        //gOriyaLigMap.insert(std::make_pair( 0xECB3, dvngLig( 136 ,"0x0B4D 0x0B1C"))); // 0x1002B
        gOriyaLigMap.insert(std::make_pair( 0xECB4, dvngLig( 136 ,"0x0B1C 0x0B4D"))); // 0x1002B
        //gOriyaLigMap.insert(std::make_pair( 0xECB5, dvngLig( 137 ,"0x0B4D 0x0B1D"))); // 0x1002C
        gOriyaLigMap.insert(std::make_pair( 0xECB6, dvngLig( 137 ,"0x0B1D 0x0B4D"))); // 0x1002C
        gOriyaLigMap.insert(std::make_pair( 0xECB7, dvngLig( 138 ,"0x0B4D 0x0B1F"))); // 0x1002D
        gOriyaLigMap.insert(std::make_pair( 0xECB8, dvngLig( 138 ,"0x0B1F 0x0B4D"))); // 0x1002D
        //gOriyaLigMap.insert(std::make_pair( 0xECB9, dvngLig( 139 ,"0x0B4D 0x0B20"))); // 0x1002E
        gOriyaLigMap.insert(std::make_pair( 0xECBA, dvngLig( 139 ,"0x0B20 0x0B4D"))); // 0x1002E
        gOriyaLigMap.insert(std::make_pair( 0xECBB, dvngLig( 140 ,"0x0B4D 0x0B21"))); // 0x1002F
        //gOriyaLigMap.insert(std::make_pair( 0xECBC, dvngLig( 140 ,"0x0B21 0x0B4D"))); // 0x1002F
        //gOriyaLigMap.insert(std::make_pair( 0xECBD, dvngLig( 141 ,"0x0B4D 0x0B22"))); // 0x10030
        gOriyaLigMap.insert(std::make_pair( 0xECBE, dvngLig( 141 ,"0x0B22 0x0B4D"))); // 0x10030
        //gOriyaLigMap.insert(std::make_pair( 0xECBF, dvngLig( 142 ,"0x0B4D 0x0B23"))); // 0x10031
        //gOriyaLigMap.insert(std::make_pair( 0xECC0, dvngLig( 142 ,"0x0B23 0x0B4D"))); // 0x10031
        //gOriyaLigMap.insert(std::make_pair( 0xECC1, dvngLig( 143 ,"0x0B4D 0x0B24"))); // 0x10032
        gOriyaLigMap.insert(std::make_pair( 0xECC2, dvngLig( 143 ,"0x0B24 0x0B4D"))); // 0x10032
        //gOriyaLigMap.insert(std::make_pair( 0xECC3, dvngLig( 144 ,"0x0B4D 0x0B25"))); // 0x10033
        gOriyaLigMap.insert(std::make_pair( 0xECC4, dvngLig( 144 ,"0x0B25 0x0B4D"))); // 0x10033
        //gOriyaLigMap.insert(std::make_pair( 0xECC5, dvngLig( 145 ,"0x0B4D 0x0B26"))); // 0x10034
        //gOriyaLigMap.insert(std::make_pair( 0xECC6, dvngLig( 145 ,"0x0B26 0x0B4D"))); // 0x10034
        //gOriyaLigMap.insert(std::make_pair( 0xECC7, dvngLig( 146 ,"0x0B4D 0x0B27"))); // 0x10035
        gOriyaLigMap.insert(std::make_pair( 0xECC8, dvngLig( 146 ,"0x0B27 0x0B4D"))); // 0x10035
        gOriyaLigMap.insert(std::make_pair( 0xECC9, dvngLig( 147 ,"0x0B4D 0x0B28"))); // 0x10036
        gOriyaLigMap.insert(std::make_pair( 0xECCA, dvngLig( 147 ,"0x0B28 0x0B4D"))); // 0x10036
        gOriyaLigMap.insert(std::make_pair( 0xECCB, dvngLig( 148 ,"0x0B4D 0x0B2A"))); // 0x10037
        //gOriyaLigMap.insert(std::make_pair( 0xECCC, dvngLig( 148 ,"0x0B2A 0x0B4D"))); // 0x10037
        //gOriyaLigMap.insert(std::make_pair( 0xECCD, dvngLig( 149 ,"0x0B4D 0x0B2B"))); // 0x10038
        //gOriyaLigMap.insert(std::make_pair( 0xECCE, dvngLig( 149 ,"0x0B2B 0x0B4D"))); // 0x10038
        gOriyaLigMap.insert(std::make_pair( 0xECCF, dvngLig( 150 ,"0x0B4D 0x0B71"))); // 0x10039
        gOriyaLigMap.insert(std::make_pair( 0xECD0, dvngLig( 150 ,"0x0B4D 0x0B35"))); // 0x10039
        gOriyaLigMap.insert(std::make_pair( 0xECD1, dvngLig( 150 ,"0x0B4D 0x0B2C"))); // 0x10039
        gOriyaLigMap.insert(std::make_pair( 0xECD2, dvngLig( 150 ,"0x0B71 0x0B4D"))); // 0x10039
        gOriyaLigMap.insert(std::make_pair( 0xECD3, dvngLig( 150 ,"0x0B35 0x0B4D"))); // 0x10039
        gOriyaLigMap.insert(std::make_pair( 0xECD4, dvngLig( 150 ,"0x0B2C 0x0B4D"))); // 0x10039
        //gOriyaLigMap.insert(std::make_pair( 0xECD5, dvngLig( 151 ,"0x0B4D 0x0B2D"))); // 0x1003A
        gOriyaLigMap.insert(std::make_pair( 0xECD6, dvngLig( 151 ,"0x0B2D 0x0B4D"))); // 0x1003A
        gOriyaLigMap.insert(std::make_pair( 0xECD7, dvngLig( 152 ,"0x0B4D 0x0B2E"))); // 0x1003B
        //gOriyaLigMap.insert(std::make_pair( 0xECD8, dvngLig( 152 ,"0x0B2E 0x0B4D"))); // 0x1003B
        gOriyaLigMap.insert(std::make_pair( 0xECD9, dvngLig( 153 ,"0x0B4D 0x0B30"))); // 0x1003C //ra virama underline
        //gOriyaLigMap.insert(std::make_pair( 0xECDA, dvngLig( 153 ,"0x0B30 0x0B4D"))); // 0x1003C //ra virama underline
        gOriyaLigMap.insert(std::make_pair( 0xECDB, dvngLig( 154 ,"0x0B4D 0x0B32"))); // 0x1003D
        //gOriyaLigMap.insert(std::make_pair( 0xECDC, dvngLig( 154 ,"0x0B32 0x0B4D"))); // 0x1003D
        gOriyaLigMap.insert(std::make_pair( 0xECDD, dvngLig( 155 ,"0x0B4D 0x0B33"))); // 0x1003E
        gOriyaLigMap.insert(std::make_pair( 0xECDE, dvngLig( 155 ,"0x0B33 0x0B4D"))); // 0x1003E
        //gOriyaLigMap.insert(std::make_pair( 0xECDF, dvngLig( 156 ,"0x0B4D 0x0B36"))); // 0x1003F
        gOriyaLigMap.insert(std::make_pair( 0xECE0, dvngLig( 156 ,"0x0B36 0x0B4D"))); // 0x1003F
        //gOriyaLigMap.insert(std::make_pair( 0xECE1, dvngLig( 157 ,"0x0B4D 0x0B37"))); // 0x10040
        gOriyaLigMap.insert(std::make_pair( 0xECE2, dvngLig( 157 ,"0x0B37 0x0B4D"))); // 0x10040
        gOriyaLigMap.insert(std::make_pair( 0xECE3, dvngLig( 158 ,"0x0B4D 0x0B38"))); // 0x10041
        gOriyaLigMap.insert(std::make_pair( 0xECE4, dvngLig( 158 ,"0x0B38 0x0B4D"))); // 0x10041
        //gOriyaLigMap.insert(std::make_pair( 0xECE5, dvngLig( 159 ,"0x0B4D 0x0B39"))); // 0x10042
        gOriyaLigMap.insert(std::make_pair( 0xECE6, dvngLig( 159 ,"0x0B39 0x0B4D"))); // 0x10042
        gOriyaLigMap.insert(std::make_pair( 0xECE7, dvngLig( 160 ,"0x0B4D 0x0B15 0x0B4D 0x0B37"))); // 0x10043
        gOriyaLigMap.insert(std::make_pair( 0xECE8, dvngLig( 160 ,"0x0B4D 0x0B15 0x0B37 0x0B4D"))); // 0x10043
        gOriyaLigMap.insert(std::make_pair( 0xECE9, dvngLig( 160 ,"0x0B15 0x0B4D 0x0B4D 0x0B37"))); // 0x10043
        //gOriyaLigMap.insert(std::make_pair( 0xECEA, dvngLig( 160 ,"0x0B15 0x0B4D 0x0B37 0x0B4D"))); // 0x10043
        gOriyaLigMap.insert(std::make_pair( 0xECEB, dvngLig( 161 ,"0x0B4D 0x0B24 0x0B4D 0x0B30"))); // 0x10044
        gOriyaLigMap.insert(std::make_pair( 0xECEC, dvngLig( 161 ,"0x0B4D 0x0B24 0x0B30 0x0B4D"))); // 0x10044
        gOriyaLigMap.insert(std::make_pair( 0xECED, dvngLig( 161 ,"0x0B24 0x0B4D 0x0B4D 0x0B30"))); // 0x10044
        gOriyaLigMap.insert(std::make_pair( 0xECEE, dvngLig( 161 ,"0x0B24 0x0B4D 0x0B30 0x0B4D"))); // 0x10044
        gOriyaLigMap.insert(std::make_pair( 0xECEF, dvngLig( 162 ,"0x0B4D 0x0B5F"))); // 0x10045
        gOriyaLigMap.insert(std::make_pair( 0xECF0, dvngLig( 162 ,"0x0B5F 0x0B4D"))); // 0x10045
        gOriyaLigMap.insert(std::make_pair( 0xECF1, dvngLig( 162 ,"0x0B2F 0x0B4D"))); // 0x10045
        //gOriyaLigMap.insert(std::make_pair( 0xECF2, dvngLig( 162 ,"0x0B4D 0x0B2F"))); // 0x10045
        gOriyaLigMap.insert(std::make_pair( 0xECF3, dvngLig( 163 ,"0x0B15 0x0B4D 0x0B15"))); // 0x10046
        gOriyaLigMap.insert(std::make_pair( 0xECF4, dvngLig( 163 ,"0x0B15 0x0B15 0x0B4D"))); // 0x10046
        gOriyaLigMap.insert(std::make_pair( 0xECF5, dvngLig( 164 ,"0x0B15 0x0B4D 0x0B1F"))); // 0x10047
        gOriyaLigMap.insert(std::make_pair( 0xECF6, dvngLig( 164 ,"0x0B15 0x0B1F 0x0B4D"))); // 0x10047
        gOriyaLigMap.insert(std::make_pair( 0xECF7, dvngLig( 165 ,"0x0B15 0x0B4D 0x0B24"))); // 0x10048
        gOriyaLigMap.insert(std::make_pair( 0xECF8, dvngLig( 165 ,"0x0B15 0x0B24 0x0B4D"))); // 0x10048
        gOriyaLigMap.insert(std::make_pair( 0xECF9, dvngLig( 166 ,"0x0B15 0x0B4D 0x0B33"))); // 0x10049
        gOriyaLigMap.insert(std::make_pair( 0xECFA, dvngLig( 166 ,"0x0B15 0x0B33 0x0B4D"))); // 0x10049
        gOriyaLigMap.insert(std::make_pair( 0xECFB, dvngLig( 167 ,"0x0B15 0x0B4D 0x0B37"))); // 0x1004A
        gOriyaLigMap.insert(std::make_pair( 0xECFC, dvngLig( 167 ,"0x0B15 0x0B37 0x0B4D"))); // 0x1004A
        gOriyaLigMap.insert(std::make_pair( 0xECFD, dvngLig( 168 ,"0x0B15 0x0B4D 0x0B37 0x0B4D 0x0B23"))); // 0x1004B
        gOriyaLigMap.insert(std::make_pair( 0xECFE, dvngLig( 168 ,"0x0B15 0x0B4D 0x0B37 0x0B23 0x0B4D"))); // 0x1004B
        gOriyaLigMap.insert(std::make_pair( 0xECFF, dvngLig( 168 ,"0x0B15 0x0B37 0x0B4D 0x0B4D 0x0B23"))); // 0x1004B
        gOriyaLigMap.insert(std::make_pair( 0xED00, dvngLig( 168 ,"0x0B15 0x0B37 0x0B4D 0x0B23 0x0B4D"))); // 0x1004B
        gOriyaLigMap.insert(std::make_pair( 0xED01, dvngLig( 169 ,"0x0B15 0x0B4D 0x0B38"))); // 0x1004C
        gOriyaLigMap.insert(std::make_pair( 0xED02, dvngLig( 169 ,"0x0B15 0x0B38 0x0B4D"))); // 0x1004C
        gOriyaLigMap.insert(std::make_pair( 0xED03, dvngLig( 170 ,"0x0B17 0x0B4D 0x0B26"))); // 0x1004D
        gOriyaLigMap.insert(std::make_pair( 0xED04, dvngLig( 170 ,"0x0B17 0x0B26 0x0B4D"))); // 0x1004D
        gOriyaLigMap.insert(std::make_pair( 0xED05, dvngLig( 171 ,"0x0B17 0x0B4D 0x0B27"))); // 0x1004E
        gOriyaLigMap.insert(std::make_pair( 0xED06, dvngLig( 171 ,"0x0B17 0x0B27 0x0B4D"))); // 0x1004E
        gOriyaLigMap.insert(std::make_pair( 0xED07, dvngLig( 172 ,"0x0B17 0x0B4D 0x0B33"))); // 0x1004F
        gOriyaLigMap.insert(std::make_pair( 0xED08, dvngLig( 172 ,"0x0B17 0x0B33 0x0B4D"))); // 0x1004F
        gOriyaLigMap.insert(std::make_pair( 0xED09, dvngLig( 173 ,"0x0B19 0x0B4D 0x0B15"))); // 0x10050
        gOriyaLigMap.insert(std::make_pair( 0xED0A, dvngLig( 173 ,"0x0B19 0x0B15 0x0B4D"))); // 0x10050
        gOriyaLigMap.insert(std::make_pair( 0xED0B, dvngLig( 174 ,"0x0B19 0x0B4D 0x0B16"))); // 0x10051
        gOriyaLigMap.insert(std::make_pair( 0xED0C, dvngLig( 174 ,"0x0B19 0x0B16 0x0B4D"))); // 0x10051
        gOriyaLigMap.insert(std::make_pair( 0xED0D, dvngLig( 175 ,"0x0B19 0x0B4D 0x0B17"))); // 0x10052
        gOriyaLigMap.insert(std::make_pair( 0xED0E, dvngLig( 175 ,"0x0B19 0x0B17 0x0B4D"))); // 0x10052
        gOriyaLigMap.insert(std::make_pair( 0xED0F, dvngLig( 176 ,"0x0B19 0x0B4D 0x0B18"))); // 0x10053
        gOriyaLigMap.insert(std::make_pair( 0xED10, dvngLig( 176 ,"0x0B19 0x0B18 0x0B4D"))); // 0x10053
        gOriyaLigMap.insert(std::make_pair( 0xED11, dvngLig( 177 ,"0x0B19 0x0B4D 0x0B15 0x0B4D 0x0B37"))); // 0x10054
        gOriyaLigMap.insert(std::make_pair( 0xED12, dvngLig( 177 ,"0x0B19 0x0B4D 0x0B15 0x0B37 0x0B4D"))); // 0x10054
        gOriyaLigMap.insert(std::make_pair( 0xED13, dvngLig( 177 ,"0x0B19 0x0B15 0x0B4D 0x0B4D 0x0B37"))); // 0x10054
        gOriyaLigMap.insert(std::make_pair( 0xED14, dvngLig( 177 ,"0x0B19 0x0B15 0x0B4D 0x0B37 0x0B4D"))); // 0x10054
        gOriyaLigMap.insert(std::make_pair( 0xED15, dvngLig( 178 ,"0x0B1A 0x0B4D 0x0B1A"))); // 0x10055
        gOriyaLigMap.insert(std::make_pair( 0xED16, dvngLig( 178 ,"0x0B1A 0x0B1A 0x0B4D"))); // 0x10055
        gOriyaLigMap.insert(std::make_pair( 0xED17, dvngLig( 179 ,"0x0B1A 0x0B4D 0x0B1B"))); // 0x10056
        gOriyaLigMap.insert(std::make_pair( 0xED18, dvngLig( 179 ,"0x0B1A 0x0B1B 0x0B4D"))); // 0x10056
        gOriyaLigMap.insert(std::make_pair( 0xED19, dvngLig( 180 ,"0x0B1C 0x0B4D 0x0B1C"))); // 0x10057
        gOriyaLigMap.insert(std::make_pair( 0xED1A, dvngLig( 180 ,"0x0B1C 0x0B1C 0x0B4D"))); // 0x10057
        gOriyaLigMap.insert(std::make_pair( 0xED1B, dvngLig( 181 ,"0x0B1C 0x0B4D 0x0B1E"))); // 0x10058
        gOriyaLigMap.insert(std::make_pair( 0xED1C, dvngLig( 182 ,"0x0B1E 0x0B4D 0x0B1A"))); // 0x10059
        gOriyaLigMap.insert(std::make_pair( 0xED1D, dvngLig( 182 ,"0x0B1E 0x0B1A 0x0B4D"))); // 0x10059
        gOriyaLigMap.insert(std::make_pair( 0xED1E, dvngLig( 183 ,"0x0B1E 0x0B4D 0x0B1B"))); // 0x1005A
        gOriyaLigMap.insert(std::make_pair( 0xED1F, dvngLig( 183 ,"0x0B1E 0x0B1B 0x0B4D"))); // 0x1005A
        gOriyaLigMap.insert(std::make_pair( 0xED20, dvngLig( 184 ,"0x0B1E 0x0B4D 0x0B1C"))); // 0x1005B
        gOriyaLigMap.insert(std::make_pair( 0xED21, dvngLig( 184 ,"0x0B1E 0x0B1C 0x0B4D"))); // 0x1005B
        gOriyaLigMap.insert(std::make_pair( 0xED22, dvngLig( 185 ,"0x0B1E 0x0B4D 0x0B1D"))); // 0x1005C
        gOriyaLigMap.insert(std::make_pair( 0xED23, dvngLig( 185 ,"0x0B1E 0x0B1D 0x0B4D"))); // 0x1005C
        gOriyaLigMap.insert(std::make_pair( 0xED24, dvngLig( 186 ,"0x0B1F 0x0B4D 0x0B1F"))); // 0x1005D
        gOriyaLigMap.insert(std::make_pair( 0xED25, dvngLig( 186 ,"0x0B1F 0x0B1F 0x0B4D"))); // 0x1005D
        gOriyaLigMap.insert(std::make_pair( 0xED26, dvngLig( 187 ,"0x0B21 0x0B4D 0x0B17"))); // 0x1005E
        gOriyaLigMap.insert(std::make_pair( 0xED27, dvngLig( 187 ,"0x0B21 0x0B17 0x0B4D"))); // 0x1005E
        gOriyaLigMap.insert(std::make_pair( 0xED28, dvngLig( 188 ,"0x0B21 0x0B4D 0x0B21"))); // 0x1005F
        gOriyaLigMap.insert(std::make_pair( 0xED29, dvngLig( 188 ,"0x0B21 0x0B21 0x0B4D"))); // 0x1005F
        gOriyaLigMap.insert(std::make_pair( 0xED2A, dvngLig( 189 ,"0x0B23 0x0B4D 0x0B1F"))); // 0x10060
        gOriyaLigMap.insert(std::make_pair( 0xED2B, dvngLig( 189 ,"0x0B23 0x0B1F 0x0B4D"))); // 0x10060
        gOriyaLigMap.insert(std::make_pair( 0xED2C, dvngLig( 190 ,"0x0B23 0x0B4D 0x0B20"))); // 0x10061
        gOriyaLigMap.insert(std::make_pair( 0xED2D, dvngLig( 190 ,"0x0B23 0x0B20 0x0B4D"))); // 0x10061
        gOriyaLigMap.insert(std::make_pair( 0xED2E, dvngLig( 191 ,"0x0B23 0x0B4D 0x0B21"))); // 0x10062
        gOriyaLigMap.insert(std::make_pair( 0xED2F, dvngLig( 191 ,"0x0B23 0x0B21 0x0B4D"))); // 0x10062
        gOriyaLigMap.insert(std::make_pair( 0xED30, dvngLig( 192 ,"0x0B23 0x0B4D 0x0B22"))); // 0x10063
        gOriyaLigMap.insert(std::make_pair( 0xED31, dvngLig( 192 ,"0x0B23 0x0B22 0x0B4D"))); // 0x10063
        gOriyaLigMap.insert(std::make_pair( 0xED32, dvngLig( 193 ,"0x0B23 0x0B4D 0x0B23"))); // 0x10064
        gOriyaLigMap.insert(std::make_pair( 0xED33, dvngLig( 193 ,"0x0B23 0x0B23 0x0B4D"))); // 0x10064
        gOriyaLigMap.insert(std::make_pair( 0xED34, dvngLig( 194 ,"0x0B24 0x0B4D 0x0B15"))); // 0x10065
        gOriyaLigMap.insert(std::make_pair( 0xED35, dvngLig( 194 ,"0x0B24 0x0B15 0x0B4D"))); // 0x10065
        gOriyaLigMap.insert(std::make_pair( 0xED36, dvngLig( 195 ,"0x0B24 0x0B4D 0x0B24"))); // 0x10066
        gOriyaLigMap.insert(std::make_pair( 0xED37, dvngLig( 195 ,"0x0B24 0x0B24 0x0B4D"))); // 0x10066
        gOriyaLigMap.insert(std::make_pair( 0xED38, dvngLig( 196 ,"0x0B24 0x0B4D 0x0B25"))); // 0x10067
        //gOriyaLigMap.insert(std::make_pair( 0xED39, dvngLig( 196 ,"0x0B24 0x0B25 0x0B4D"))); // 0x10067
        gOriyaLigMap.insert(std::make_pair( 0xED3A, dvngLig( 197 ,"0x0B24 0x0B4D 0x0B28"))); // 0x10068
        gOriyaLigMap.insert(std::make_pair( 0xED3B, dvngLig( 197 ,"0x0B24 0x0B28 0x0B4D"))); // 0x10068
        gOriyaLigMap.insert(std::make_pair( 0xED3C, dvngLig( 198 ,"0x0B24 0x0B4D 0x0B2A"))); // 0x10069
        gOriyaLigMap.insert(std::make_pair( 0xED3D, dvngLig( 198 ,"0x0B24 0x0B2A 0x0B4D"))); // 0x10069
        gOriyaLigMap.insert(std::make_pair( 0xED3E, dvngLig( 199 ,"0x0B24 0x0B4D 0x0B2E"))); // 0x1006A
        gOriyaLigMap.insert(std::make_pair( 0xED3F, dvngLig( 199 ,"0x0B24 0x0B2E 0x0B4D"))); // 0x1006A
        gOriyaLigMap.insert(std::make_pair( 0xED40, dvngLig( 200 ,"0x0B24 0x0B4D 0x0B38"))); // 0x1006B
        gOriyaLigMap.insert(std::make_pair( 0xED41, dvngLig( 200 ,"0x0B24 0x0B38 0x0B4D"))); // 0x1006B
        gOriyaLigMap.insert(std::make_pair( 0xED42, dvngLig( 201 ,"0x0B26 0x0B4D 0x0B17"))); // 0x1006C
        gOriyaLigMap.insert(std::make_pair( 0xED43, dvngLig( 201 ,"0x0B26 0x0B17 0x0B4D"))); // 0x1006C
        gOriyaLigMap.insert(std::make_pair( 0xED44, dvngLig( 202 ,"0x0B26 0x0B4D 0x0B26"))); // 0x1006D
        gOriyaLigMap.insert(std::make_pair( 0xED45, dvngLig( 202 ,"0x0B26 0x0B26 0x0B4D"))); // 0x1006D
        gOriyaLigMap.insert(std::make_pair( 0xED46, dvngLig( 203 ,"0x0B26 0x0B4D 0x0B27"))); // 0x1006E
        gOriyaLigMap.insert(std::make_pair( 0xED47, dvngLig( 203 ,"0x0B26 0x0B27 0x0B4D"))); // 0x1006E
        gOriyaLigMap.insert(std::make_pair( 0xED48, dvngLig( 204 ,"0x0B26 0x0B4D 0x0B2D"))); // 0x1006F
        gOriyaLigMap.insert(std::make_pair( 0xED49, dvngLig( 204 ,"0x0B26 0x0B2D 0x0B4D"))); // 0x1006F
        gOriyaLigMap.insert(std::make_pair( 0xED4A, dvngLig( 205 ,"0x0B27 0x0B4D 0x0B5F"))); // 0x10070
        //gOriyaLigMap.insert(std::make_pair( 0xED4B, dvngLig( 205 ,"0x0B27 0x0B5F 0x0B4D"))); // 0x10070
        //gOriyaLigMap.insert(std::make_pair( 0xED4C, dvngLig( 205 ,"0x0B27 0x0B2F 0x0B4D"))); // 0x10070
        gOriyaLigMap.insert(std::make_pair( 0xED4D, dvngLig( 205 ,"0x0B27 0x0B4D 0x0B2F"))); // 0x10070
        gOriyaLigMap.insert(std::make_pair( 0xED4E, dvngLig( 206 ,"0x0B28 0x0B4D 0x0B24"))); // 0x10071
        gOriyaLigMap.insert(std::make_pair( 0xED4F, dvngLig( 206 ,"0x0B28 0x0B24 0x0B4D"))); // 0x10071
        gOriyaLigMap.insert(std::make_pair( 0xED50, dvngLig( 207 ,"0x0B28 0x0B4D 0x0B24 0x0B4D 0x0B30"))); // 0x10072
        //gOriyaLigMap.insert(std::make_pair( 0xED51, dvngLig( 207 ,"0x0B28 0x0B4D 0x0B24 0x0B30 0x0B4D"))); // 0x10072
        //gOriyaLigMap.insert(std::make_pair( 0xED52, dvngLig( 207 ,"0x0B28 0x0B24 0x0B4D 0x0B4D 0x0B30"))); // 0x10072
        //gOriyaLigMap.insert(std::make_pair( 0xED53, dvngLig( 207 ,"0x0B28 0x0B24 0x0B4D 0x0B30 0x0B4D"))); // 0x10072
        gOriyaLigMap.insert(std::make_pair( 0xED54, dvngLig( 208 ,"0x0B28 0x0B4D 0x0B25"))); // 0x10073
        gOriyaLigMap.insert(std::make_pair( 0xED55, dvngLig( 208 ,"0x0B28 0x0B25 0x0B4D"))); // 0x10073
        gOriyaLigMap.insert(std::make_pair( 0xED56, dvngLig( 209 ,"0x0B28 0x0B4D 0x0B26"))); // 0x10074
        gOriyaLigMap.insert(std::make_pair( 0xED57, dvngLig( 209 ,"0x0B28 0x0B26 0x0B4D"))); // 0x10074
        gOriyaLigMap.insert(std::make_pair( 0xED58, dvngLig( 210 ,"0x0B28 0x0B4D 0x0B27"))); // 0x10075
        gOriyaLigMap.insert(std::make_pair( 0xED59, dvngLig( 210 ,"0x0B28 0x0B27 0x0B4D"))); // 0x10075
        gOriyaLigMap.insert(std::make_pair( 0xED5A, dvngLig( 211 ,"0x0B28 0x0B4D 0x0B28"))); // 0x10076
        //gOriyaLigMap.insert(std::make_pair( 0xED5B, dvngLig( 211 ,"0x0B28 0x0B28 0x0B4D"))); // 0x10076
        gOriyaLigMap.insert(std::make_pair( 0xED5C, dvngLig( 212 ,"0x0B2A 0x0B4D 0x0B24"))); // 0x10077
        //gOriyaLigMap.insert(std::make_pair( 0xED5D, dvngLig( 212 ,"0x0B2A 0x0B24 0x0B4D"))); // 0x10077
        gOriyaLigMap.insert(std::make_pair( 0xED5E, dvngLig( 213 ,"0x0B2A 0x0B4D 0x0B2A"))); // 0x10078
        gOriyaLigMap.insert(std::make_pair( 0xED5F, dvngLig( 213 ,"0x0B2A 0x0B2A 0x0B4D"))); // 0x10078
        gOriyaLigMap.insert(std::make_pair( 0xED60, dvngLig( 214 ,"0x0B2A 0x0B4D 0x0B33"))); // 0x10079
        gOriyaLigMap.insert(std::make_pair( 0xED61, dvngLig( 214 ,"0x0B2A 0x0B33 0x0B4D"))); // 0x10079
        gOriyaLigMap.insert(std::make_pair( 0xED62, dvngLig( 215 ,"0x0B2A 0x0B4D 0x0B38"))); // 0x1007A
        gOriyaLigMap.insert(std::make_pair( 0xED63, dvngLig( 215 ,"0x0B2A 0x0B38 0x0B4D"))); // 0x1007A
        gOriyaLigMap.insert(std::make_pair( 0xED64, dvngLig( 216 ,"0x0B2C 0x0B4D 0x0B26"))); // 0x1007B
        gOriyaLigMap.insert(std::make_pair( 0xED65, dvngLig( 216 ,"0x0B2C 0x0B26 0x0B4D"))); // 0x1007B
        gOriyaLigMap.insert(std::make_pair( 0xED66, dvngLig( 217 ,"0x0B2C 0x0B4D 0x0B27"))); // 0x1007C
        gOriyaLigMap.insert(std::make_pair( 0xED67, dvngLig( 217 ,"0x0B2C 0x0B27 0x0B4D"))); // 0x1007C
        gOriyaLigMap.insert(std::make_pair( 0xED68, dvngLig( 218 ,"0x0B2C 0x0B4D 0x0B71"))); // 0x1007D
        gOriyaLigMap.insert(std::make_pair( 0xED69, dvngLig( 218 ,"0x0B2C 0x0B4D 0x0B35"))); // 0x1007D
        gOriyaLigMap.insert(std::make_pair( 0xED6A, dvngLig( 218 ,"0x0B2C 0x0B4D 0x0B2C"))); // 0x1007D
        gOriyaLigMap.insert(std::make_pair( 0xED6B, dvngLig( 218 ,"0x0B2C 0x0B71 0x0B4D"))); // 0x1007D
        gOriyaLigMap.insert(std::make_pair( 0xED6C, dvngLig( 218 ,"0x0B2C 0x0B35 0x0B4D"))); // 0x1007D
        gOriyaLigMap.insert(std::make_pair( 0xED6D, dvngLig( 218 ,"0x0B2C 0x0B2C 0x0B4D"))); // 0x1007D
        gOriyaLigMap.insert(std::make_pair( 0xED6E, dvngLig( 219 ,"0x0B2E 0x0B4D 0x0B2A"))); // 0x1007E
        //gOriyaLigMap.insert(std::make_pair( 0xED6F, dvngLig( 219 ,"0x0B2E 0x0B2A 0x0B4D"))); // 0x1007E
        gOriyaLigMap.insert(std::make_pair( 0xED70, dvngLig( 220 ,"0x0B2E 0x0B4D 0x0B2B"))); // 0x1007F
        //gOriyaLigMap.insert(std::make_pair( 0xED71, dvngLig( 220 ,"0x0B2E 0x0B2B 0x0B4D"))); // 0x1007F
        gOriyaLigMap.insert(std::make_pair( 0xED72, dvngLig( 221 ,"0x0B2E 0x0B4D 0x0B2D"))); // 0x10080
        //gOriyaLigMap.insert(std::make_pair( 0xED73, dvngLig( 221 ,"0x0B2E 0x0B2D 0x0B4D"))); // 0x10080
        gOriyaLigMap.insert(std::make_pair( 0xED74, dvngLig( 222 ,"0x0B2E 0x0B4D 0x0B2E"))); // 0x10081
        //gOriyaLigMap.insert(std::make_pair( 0xED75, dvngLig( 222 ,"0x0B2E 0x0B2E 0x0B4D"))); // 0x10081
        gOriyaLigMap.insert(std::make_pair( 0xED76, dvngLig( 223 ,"0x0B32 0x0B4D 0x0B15"))); // 0x10082
        //gOriyaLigMap.insert(std::make_pair( 0xED77, dvngLig( 223 ,"0x0B32 0x0B15 0x0B4D"))); // 0x10082
        gOriyaLigMap.insert(std::make_pair( 0xED78, dvngLig( 224 ,"0x0B32 0x0B4D 0x0B17"))); // 0x10083
        //gOriyaLigMap.insert(std::make_pair( 0xED79, dvngLig( 224 ,"0x0B32 0x0B17 0x0B4D"))); // 0x10083
        gOriyaLigMap.insert(std::make_pair( 0xED7A, dvngLig( 225 ,"0x0B32 0x0B4D 0x0B32"))); // 0x10084
        //gOriyaLigMap.insert(std::make_pair( 0xED7B, dvngLig( 225 ,"0x0B32 0x0B32 0x0B4D"))); // 0x10084
        gOriyaLigMap.insert(std::make_pair( 0xED7C, dvngLig( 226 ,"0x0B33 0x0B4D 0x0B15"))); // 0x10085
        //gOriyaLigMap.insert(std::make_pair( 0xED7D, dvngLig( 226 ,"0x0B33 0x0B15 0x0B4D"))); // 0x10085
        gOriyaLigMap.insert(std::make_pair( 0xED7E, dvngLig( 227 ,"0x0B33 0x0B4D 0x0B2A"))); // 0x10086
        //gOriyaLigMap.insert(std::make_pair( 0xED7F, dvngLig( 227 ,"0x0B33 0x0B2A 0x0B4D"))); // 0x10086
        gOriyaLigMap.insert(std::make_pair( 0xED80, dvngLig( 228 ,"0x0B33 0x0B4D 0x0B2B"))); // 0x10087
        //gOriyaLigMap.insert(std::make_pair( 0xED81, dvngLig( 228 ,"0x0B33 0x0B2B 0x0B4D"))); // 0x10087
        gOriyaLigMap.insert(std::make_pair( 0xED82, dvngLig( 229 ,"0x0B33 0x0B4D 0x0B33"))); // 0x10088
        //gOriyaLigMap.insert(std::make_pair( 0xED83, dvngLig( 229 ,"0x0B33 0x0B33 0x0B4D"))); // 0x10088
        gOriyaLigMap.insert(std::make_pair( 0xED84, dvngLig( 230 ,"0x0B36 0x0B4D 0x0B1A"))); // 0x10089
        //gOriyaLigMap.insert(std::make_pair( 0xED85, dvngLig( 230 ,"0x0B36 0x0B1A 0x0B4D"))); // 0x10089
        gOriyaLigMap.insert(std::make_pair( 0xED86, dvngLig( 231 ,"0x0B36 0x0B4D 0x0B1B"))); // 0x1008A
        //gOriyaLigMap.insert(std::make_pair( 0xED87, dvngLig( 231 ,"0x0B36 0x0B1B 0x0B4D"))); // 0x1008A
        gOriyaLigMap.insert(std::make_pair( 0xED88, dvngLig( 232 ,"0x0B36 0x0B4D 0x0B33"))); // 0x1008B
        //gOriyaLigMap.insert(std::make_pair( 0xED89, dvngLig( 232 ,"0x0B36 0x0B33 0x0B4D"))); // 0x1008B
        gOriyaLigMap.insert(std::make_pair( 0xED8A, dvngLig( 233 ,"0x0B37 0x0B4D 0x0B15"))); // 0x1008C
        //gOriyaLigMap.insert(std::make_pair( 0xED8B, dvngLig( 233 ,"0x0B37 0x0B15 0x0B4D"))); // 0x1008C
        gOriyaLigMap.insert(std::make_pair( 0xED8C, dvngLig( 234 ,"0x0B37 0x0B4D 0x0B1F"))); // 0x1008D
        //gOriyaLigMap.insert(std::make_pair( 0xED8D, dvngLig( 234 ,"0x0B37 0x0B1F 0x0B4D"))); // 0x1008D
        gOriyaLigMap.insert(std::make_pair( 0xED8E, dvngLig( 235 ,"0x0B37 0x0B4D 0x0B20"))); // 0x1008E
        //gOriyaLigMap.insert(std::make_pair( 0xED8F, dvngLig( 235 ,"0x0B37 0x0B20 0x0B4D"))); // 0x1008E
        gOriyaLigMap.insert(std::make_pair( 0xED90, dvngLig( 236 ,"0x0B37 0x0B4D 0x0B23"))); // 0x1008F
        //gOriyaLigMap.insert(std::make_pair( 0xED91, dvngLig( 236 ,"0x0B37 0x0B23 0x0B4D"))); // 0x1008F
        gOriyaLigMap.insert(std::make_pair( 0xED92, dvngLig( 237 ,"0x0B37 0x0B4D 0x0B2A"))); // 0x10090
        //gOriyaLigMap.insert(std::make_pair( 0xED93, dvngLig( 237 ,"0x0B37 0x0B2A 0x0B4D"))); // 0x10090
        gOriyaLigMap.insert(std::make_pair( 0xED94, dvngLig( 238 ,"0x0B37 0x0B4D 0x0B2B"))); // 0x10091
        //gOriyaLigMap.insert(std::make_pair( 0xED95, dvngLig( 238 ,"0x0B37 0x0B2B 0x0B4D"))); // 0x10091
        gOriyaLigMap.insert(std::make_pair( 0xED96, dvngLig( 239 ,"0x0B38 0x0B4D 0x0B15"))); // 0x10092
        //gOriyaLigMap.insert(std::make_pair( 0xED97, dvngLig( 239 ,"0x0B38 0x0B15 0x0B4D"))); // 0x10092
        gOriyaLigMap.insert(std::make_pair( 0xED98, dvngLig( 240 ,"0x0B38 0x0B4D 0x0B16"))); // 0x10093
        //gOriyaLigMap.insert(std::make_pair( 0xED99, dvngLig( 240 ,"0x0B38 0x0B16 0x0B4D"))); // 0x10093
        gOriyaLigMap.insert(std::make_pair( 0xED9A, dvngLig( 241 ,"0x0B38 0x0B4D 0x0B24"))); // 0x10094
        //gOriyaLigMap.insert(std::make_pair( 0xED9B, dvngLig( 241 ,"0x0B38 0x0B24 0x0B4D"))); // 0x10094
        gOriyaLigMap.insert(std::make_pair( 0xED9C, dvngLig( 242 ,"0x0B38 0x0B4D 0x0B24 0x0B4D 0x0B30"))); // 0x10095
        //gOriyaLigMap.insert(std::make_pair( 0xED9D, dvngLig( 242 ,"0x0B38 0x0B4D 0x0B24 0x0B30 0x0B4D"))); // 0x10095
        gOriyaLigMap.insert(std::make_pair( 0xED9E, dvngLig( 242 ,"0x0B38 0x0B24 0x0B4D 0x0B4D 0x0B30"))); // 0x10095
        //gOriyaLigMap.insert(std::make_pair( 0xED9F, dvngLig( 242 ,"0x0B38 0x0B24 0x0B4D 0x0B30 0x0B4D"))); // 0x10095
        gOriyaLigMap.insert(std::make_pair( 0xEDA0, dvngLig( 243 ,"0x0B38 0x0B4D 0x0B25"))); // 0x10096
        //gOriyaLigMap.insert(std::make_pair( 0xEDA1, dvngLig( 243 ,"0x0B38 0x0B25 0x0B4D"))); // 0x10096
        gOriyaLigMap.insert(std::make_pair( 0xEDA2, dvngLig( 244 ,"0x0B38 0x0B4D 0x0B2A"))); // 0x10097
        //gOriyaLigMap.insert(std::make_pair( 0xEDA3, dvngLig( 244 ,"0x0B38 0x0B2A 0x0B4D"))); // 0x10097
        gOriyaLigMap.insert(std::make_pair( 0xEDA4, dvngLig( 245 ,"0x0B38 0x0B4D 0x0B2B"))); // 0x10098
        //gOriyaLigMap.insert(std::make_pair( 0xEDA5, dvngLig( 245 ,"0x0B38 0x0B2B 0x0B4D"))); // 0x10098
        gOriyaLigMap.insert(std::make_pair( 0xEDA6, dvngLig( 246 ,"0x0B30 0x0B4D 0x0B01"))); // 0x10099
        //gOriyaLigMap.insert(std::make_pair( 0xEDA7, dvngLig( 246 ,"0x0B01 0x0B30 0x0B4D"))); // 0x10099
        gOriyaLigMap.insert(std::make_pair( 0xEDA8, dvngLig( 247 ,"0x0B10 0x0B01"))); // 0x1009A
        gOriyaLigMap.insert(std::make_pair( 0xEDA9, dvngLig( 248 ,"0x0B10 0x0B30 0x0B4D"))); // 0x1009B
        gOriyaLigMap.insert(std::make_pair( 0xEDAA, dvngLig( 249 ,"0x0B13 0x200D 0x0B01"))); // 0x1009C
        gOriyaLigMap.insert(std::make_pair( 0xEDAB, dvngLig( 250 ,"0x0B14 0x0B01"))); // 0x1009D
        gOriyaLigMap.insert(std::make_pair( 0xEDAC, dvngLig( 251 ,"0x0B14 0x0B30 0x0B4D"))); // 0x1009E
        gOriyaLigMap.insert(std::make_pair( 0xEDAD, dvngLig( 252 ,"0x0B3F 0x0B01"))); // 0x1009F
        //gOriyaLigMap.insert(std::make_pair( 0xEDAE, dvngLig( 253 ,"0x0B3F 0x0B30 0x0B4D"))); // 0x100A0
        gOriyaLigMap.insert(std::make_pair( 0xEDAF, dvngLig( 254 ,"0x0B3F 0x0B30 0x0B4D 0x0B01"))); // 0x100A1
        gOriyaLigMap.insert(std::make_pair( 0xEDB0, dvngLig( 254 ,"0x0B3F 0x0B01 0x0B30 0x0B4D"))); // 0x100A1
        gOriyaLigMap.insert(std::make_pair( 0xEDB1, dvngLig( 255 ,"0x0B56 0x0B01"))); // 0x100A2
        gOriyaLigMap.insert(std::make_pair( 0xEDB2, dvngLig( 256 ,"0x0B30 0x0B4D 0x0B56"))); // 0x100A3
        gOriyaLigMap.insert(std::make_pair( 0xEDB3, dvngLig( 256 ,"0x0B56 0x0B30 0x0B4D"))); // 0x100A3
        gOriyaLigMap.insert(std::make_pair( 0xEDB4, dvngLig( 257 ,"0x0B30 0x0B4D 0x0B56 0x0B01"))); // 0x100A4
        gOriyaLigMap.insert(std::make_pair( 0xEDB5, dvngLig( 257 ,"0x0B56 0x0B30 0x0B4D 0x0B01"))); // 0x100A4
        gOriyaLigMap.insert(std::make_pair( 0xEDB6, dvngLig( 257 ,"0x0B56 0x0B01 0x0B30 0x0B4D"))); // 0x100A4
        gOriyaLigMap.insert(std::make_pair( 0xEDB7, dvngLig( 258 ,"0x0B57 0x0B01"))); // 0x100A5
        gOriyaLigMap.insert(std::make_pair( 0xEDB8, dvngLig( 259 ,"0x0B30 0x0B4D 0x0B57"))); // 0x100A6
        gOriyaLigMap.insert(std::make_pair( 0xEDB9, dvngLig( 260 ,"0x0B30 0x0B4D 0x0B57 0x0B01"))); // 0x100A7
        gOriyaLigMap.insert(std::make_pair( 0xEDBA, dvngLig( 292 ,"0x0B15 0x0B4D"))); // 0x100C7
        gOriyaLigMap.insert(std::make_pair( 0xEDBB, dvngLig( 293 ,"0x0B16 0x0B4D"))); // 0x100C8
        gOriyaLigMap.insert(std::make_pair( 0xEDBC, dvngLig( 294 ,"0x0B17 0x0B4D"))); // 0x100C9
        gOriyaLigMap.insert(std::make_pair( 0xEDBD, dvngLig( 295 ,"0x0B18 0x0B4D"))); // 0x100CA
        gOriyaLigMap.insert(std::make_pair( 0xEDBE, dvngLig( 296 ,"0x0B19 0x0B4D"))); // 0x100CB
        gOriyaLigMap.insert(std::make_pair( 0xEDBF, dvngLig( 297 ,"0x0B1A 0x0B4D"))); // 0x100CC
        gOriyaLigMap.insert(std::make_pair( 0xEDC0, dvngLig( 298 ,"0x0B1B 0x0B4D"))); // 0x100CD
        gOriyaLigMap.insert(std::make_pair( 0xEDC1, dvngLig( 299 ,"0x0B1C 0x0B4D"))); // 0x100CE
        gOriyaLigMap.insert(std::make_pair( 0xEDC2, dvngLig( 300 ,"0x0B1D 0x0B4D"))); // 0x100CF
        gOriyaLigMap.insert(std::make_pair( 0xEDC3, dvngLig( 301 ,"0x0B1E 0x0B4D"))); // 0x100D0
        gOriyaLigMap.insert(std::make_pair( 0xEDC4, dvngLig( 302 ,"0x0B1F 0x0B4D"))); // 0x100D1
        gOriyaLigMap.insert(std::make_pair( 0xEDC5, dvngLig( 303 ,"0x0B20 0x0B4D"))); // 0x100D2
        gOriyaLigMap.insert(std::make_pair( 0xEDC6, dvngLig( 304 ,"0x0B21 0x0B4D"))); // 0x100D3
        gOriyaLigMap.insert(std::make_pair( 0xEDC7, dvngLig( 305 ,"0x0B22 0x0B4D"))); // 0x100D4
        gOriyaLigMap.insert(std::make_pair( 0xEDC8, dvngLig( 306 ,"0x0B23 0x0B4D"))); // 0x100D5
        gOriyaLigMap.insert(std::make_pair( 0xEDC9, dvngLig( 307 ,"0x0B24 0x0B4D"))); // 0x100D6
        gOriyaLigMap.insert(std::make_pair( 0xEDCA, dvngLig( 308 ,"0x0B25 0x0B4D"))); // 0x100D7
        //gOriyaLigMap.insert(std::make_pair( 0xEDCB, dvngLig( 309 ,"0x0B26 0x0B4D"))); // 0x100D8
        gOriyaLigMap.insert(std::make_pair( 0xEDCC, dvngLig( 310 ,"0x0B27 0x0B4D"))); // 0x100D9
        gOriyaLigMap.insert(std::make_pair( 0xEDCD, dvngLig( 311 ,"0x0B28 0x0B4D"))); // 0x100DA
        gOriyaLigMap.insert(std::make_pair( 0xEDCE, dvngLig( 312 ,"0x0B2A 0x0B4D"))); // 0x100DB
        //gOriyaLigMap.insert(std::make_pair( 0xEDCF, dvngLig( 313 ,"0x0B2B 0x0B4D"))); // 0x100DC
        gOriyaLigMap.insert(std::make_pair( 0xEDD0, dvngLig( 314 ,"0x0B2C 0x0B4D"))); // 0x100DD
        gOriyaLigMap.insert(std::make_pair( 0xEDD1, dvngLig( 315 ,"0x0B2D 0x0B4D"))); // 0x100DE
        //gOriyaLigMap.insert(std::make_pair( 0xEDD2, dvngLig( 316 ,"0x0B2E 0x0B4D"))); // 0x100DF
        gOriyaLigMap.insert(std::make_pair( 0xEDD3, dvngLig( 317 ,"0x0B2F 0x0B4D"))); // 0x100E0
        gOriyaLigMap.insert(std::make_pair( 0xEDD4, dvngLig( 318 ,"0x0B30 0x0B4D"))); // 0x100E1 //ra virama full

        //gOriyaLigMap.insert(std::make_pair( 0xEDD5, dvngLig( 319 ,"0x0B32 0x0B4D"))); // 0x100E2
        gOriyaLigMap.insert(std::make_pair( 0xEDD6, dvngLig( 320 ,"0x0B33 0x0B4D"))); // 0x100E3
        gOriyaLigMap.insert(std::make_pair( 0xEDD7, dvngLig( 321 ,"0x0B35 0x0B4D"))); // 0x100E4
        gOriyaLigMap.insert(std::make_pair( 0xEDD8, dvngLig( 322 ,"0x0B36 0x0B4D"))); // 0x100E5
        gOriyaLigMap.insert(std::make_pair( 0xEDD9, dvngLig( 323 ,"0x0B37 0x0B4D"))); // 0x100E6
        gOriyaLigMap.insert(std::make_pair( 0xEDDA, dvngLig( 324 ,"0x0B38 0x0B4D"))); // 0x100E7
        gOriyaLigMap.insert(std::make_pair( 0xEDDB, dvngLig( 325 ,"0x0B39 0x0B4D"))); // 0x100E8
        gOriyaLigMap.insert(std::make_pair( 0xEDDC, dvngLig( 328 ,"0x0B5F 0x0B4D"))); // 0x100EB
        gOriyaLigMap.insert(std::make_pair( 0xEDDD, dvngLig( 329 ,"0x0B71 0x0B4D"))); // 0x100EC
        //gOriyaLigMap.insert(std::make_pair( 0xEDDE, dvngLig( 364 ,"0x0B15 0x0B4D 0x0B37 0x0B4D"))); // 0x1010F
        gOriyaLigMap.insert(std::make_pair( 0xEDDF, dvngLig( 364 ,"0x0B15 0x0B37 0x0B4D 0x0B4D"))); // 0x1010F
        gOriyaLigMap.insert(std::make_pair( 0xEDE0, dvngLig( 421 ,"0x0B3C 0x0B4D 0x0B15"))); // 0x10110
        gOriyaLigMap.insert(std::make_pair( 0xEDE1, dvngLig( 421 ,"0x0B3C 0x0B15 0x0B4D"))); // 0x10110
        gOriyaLigMap.insert(std::make_pair( 0xEDE2, dvngLig( 422 ,"0x0B3C 0x0B4D 0x0B16"))); // 0x10111
        gOriyaLigMap.insert(std::make_pair( 0xEDE3, dvngLig( 422 ,"0x0B3C 0x0B16 0x0B4D"))); // 0x10111
        gOriyaLigMap.insert(std::make_pair( 0xEDE4, dvngLig( 423 ,"0x0B3C 0x0B4D 0x0B17"))); // 0x10112
        gOriyaLigMap.insert(std::make_pair( 0xEDE5, dvngLig( 423 ,"0x0B3C 0x0B17 0x0B4D"))); // 0x10112
        gOriyaLigMap.insert(std::make_pair( 0xEDE6, dvngLig( 424 ,"0x0B3C 0x0B4D 0x0B18"))); // 0x10113
        gOriyaLigMap.insert(std::make_pair( 0xEDE7, dvngLig( 424 ,"0x0B3C 0x0B18 0x0B4D"))); // 0x10113
        gOriyaLigMap.insert(std::make_pair( 0xEDE8, dvngLig( 425 ,"0x0B3C 0x0B4D 0x0B19"))); // 0x10114
        gOriyaLigMap.insert(std::make_pair( 0xEDE9, dvngLig( 425 ,"0x0B3C 0x0B19 0x0B4D"))); // 0x10114
        gOriyaLigMap.insert(std::make_pair( 0xEDEA, dvngLig( 426 ,"0x0B3C 0x0B4D 0x0B1A"))); // 0x10115
        gOriyaLigMap.insert(std::make_pair( 0xEDEB, dvngLig( 426 ,"0x0B3C 0x0B1A 0x0B4D"))); // 0x10115
        gOriyaLigMap.insert(std::make_pair( 0xEDEC, dvngLig( 427 ,"0x0B3C 0x0B4D 0x0B1B"))); // 0x10116
        gOriyaLigMap.insert(std::make_pair( 0xEDED, dvngLig( 427 ,"0x0B3C 0x0B1B 0x0B4D"))); // 0x10116
        gOriyaLigMap.insert(std::make_pair( 0xEDEE, dvngLig( 428 ,"0x0B3C 0x0B4D 0x0B1C"))); // 0x10117
        gOriyaLigMap.insert(std::make_pair( 0xEDEF, dvngLig( 428 ,"0x0B3C 0x0B1C 0x0B4D"))); // 0x10117
        gOriyaLigMap.insert(std::make_pair( 0xEDF0, dvngLig( 429 ,"0x0B3C 0x0B4D 0x0B1D"))); // 0x10118
        gOriyaLigMap.insert(std::make_pair( 0xEDF1, dvngLig( 429 ,"0x0B3C 0x0B1D 0x0B4D"))); // 0x10118
        gOriyaLigMap.insert(std::make_pair( 0xEDF2, dvngLig( 430 ,"0x0B3C 0x0B4D 0x0B1F"))); // 0x10119
        gOriyaLigMap.insert(std::make_pair( 0xEDF3, dvngLig( 430 ,"0x0B3C 0x0B1F 0x0B4D"))); // 0x10119
        gOriyaLigMap.insert(std::make_pair( 0xEDF4, dvngLig( 431 ,"0x0B3C 0x0B4D 0x0B20"))); // 0x1011A
        gOriyaLigMap.insert(std::make_pair( 0xEDF5, dvngLig( 431 ,"0x0B3C 0x0B20 0x0B4D"))); // 0x1011A
        gOriyaLigMap.insert(std::make_pair( 0xEDF6, dvngLig( 432 ,"0x0B3C 0x0B4D 0x0B21"))); // 0x1011B
        gOriyaLigMap.insert(std::make_pair( 0xEDF7, dvngLig( 432 ,"0x0B3C 0x0B21 0x0B4D"))); // 0x1011B
        gOriyaLigMap.insert(std::make_pair( 0xEDF8, dvngLig( 433 ,"0x0B3C 0x0B4D 0x0B22"))); // 0x1011C
        gOriyaLigMap.insert(std::make_pair( 0xEDF9, dvngLig( 433 ,"0x0B3C 0x0B22 0x0B4D"))); // 0x1011C
        gOriyaLigMap.insert(std::make_pair( 0xEDFA, dvngLig( 434 ,"0x0B3C 0x0B4D 0x0B23"))); // 0x1011D
        gOriyaLigMap.insert(std::make_pair( 0xEDFB, dvngLig( 434 ,"0x0B3C 0x0B23 0x0B4D"))); // 0x1011D
        gOriyaLigMap.insert(std::make_pair( 0xEDFC, dvngLig( 435 ,"0x0B3C 0x0B4D 0x0B24"))); // 0x1011E
        gOriyaLigMap.insert(std::make_pair( 0xEDFD, dvngLig( 435 ,"0x0B3C 0x0B24 0x0B4D"))); // 0x1011E
        gOriyaLigMap.insert(std::make_pair( 0xEDFE, dvngLig( 436 ,"0x0B3C 0x0B4D 0x0B25"))); // 0x1011F
        gOriyaLigMap.insert(std::make_pair( 0xEDFF, dvngLig( 436 ,"0x0B3C 0x0B25 0x0B4D"))); // 0x1011F
        gOriyaLigMap.insert(std::make_pair( 0xEE00, dvngLig( 437 ,"0x0B3C 0x0B4D 0x0B26"))); // 0x10120
        gOriyaLigMap.insert(std::make_pair( 0xEE01, dvngLig( 437 ,"0x0B3C 0x0B26 0x0B4D"))); // 0x10120
        gOriyaLigMap.insert(std::make_pair( 0xEE02, dvngLig( 438 ,"0x0B3C 0x0B4D 0x0B27"))); // 0x10121
        gOriyaLigMap.insert(std::make_pair( 0xEE03, dvngLig( 438 ,"0x0B3C 0x0B27 0x0B4D"))); // 0x10121
        gOriyaLigMap.insert(std::make_pair( 0xEE04, dvngLig( 439 ,"0x0B3C 0x0B4D 0x0B28"))); // 0x10122
        gOriyaLigMap.insert(std::make_pair( 0xEE05, dvngLig( 439 ,"0x0B3C 0x0B28 0x0B4D"))); // 0x10122
        gOriyaLigMap.insert(std::make_pair( 0xEE06, dvngLig( 440 ,"0x0B3C 0x0B4D 0x0B2A"))); // 0x10123
        gOriyaLigMap.insert(std::make_pair( 0xEE07, dvngLig( 440 ,"0x0B3C 0x0B2A 0x0B4D"))); // 0x10123
        gOriyaLigMap.insert(std::make_pair( 0xEE08, dvngLig( 441 ,"0x0B3C 0x0B4D 0x0B2B"))); // 0x10124
        gOriyaLigMap.insert(std::make_pair( 0xEE09, dvngLig( 441 ,"0x0B3C 0x0B2B 0x0B4D"))); // 0x10124
        gOriyaLigMap.insert(std::make_pair( 0xEE0A, dvngLig( 442 ,"0x0B3C 0x0B4D 0x0B71"))); // 0x10125
        gOriyaLigMap.insert(std::make_pair( 0xEE0B, dvngLig( 442 ,"0x0B3C 0x0B4D 0x0B35"))); // 0x10125
        gOriyaLigMap.insert(std::make_pair( 0xEE0C, dvngLig( 442 ,"0x0B3C 0x0B4D 0x0B2C"))); // 0x10125
        gOriyaLigMap.insert(std::make_pair( 0xEE0D, dvngLig( 442 ,"0x0B3C 0x0B71 0x0B4D"))); // 0x10125
        gOriyaLigMap.insert(std::make_pair( 0xEE0E, dvngLig( 442 ,"0x0B3C 0x0B35 0x0B4D"))); // 0x10125
        gOriyaLigMap.insert(std::make_pair( 0xEE0F, dvngLig( 442 ,"0x0B3C 0x0B2C 0x0B4D"))); // 0x10125
        gOriyaLigMap.insert(std::make_pair( 0xEE10, dvngLig( 443 ,"0x0B3C 0x0B4D 0x0B2D"))); // 0x10126
        gOriyaLigMap.insert(std::make_pair( 0xEE11, dvngLig( 443 ,"0x0B3C 0x0B2D 0x0B4D"))); // 0x10126
        gOriyaLigMap.insert(std::make_pair( 0xEE12, dvngLig( 444 ,"0x0B3C 0x0B4D 0x0B2E"))); // 0x10127
        gOriyaLigMap.insert(std::make_pair( 0xEE13, dvngLig( 444 ,"0x0B3C 0x0B2E 0x0B4D"))); // 0x10127
        gOriyaLigMap.insert(std::make_pair( 0xEE14, dvngLig( 445 ,"0x0B3C 0x0B4D 0x0B30"))); // 0x10128
        gOriyaLigMap.insert(std::make_pair( 0xEE15, dvngLig( 445 ,"0x0B3C 0x0B30 0x0B4D"))); // 0x10128
        gOriyaLigMap.insert(std::make_pair( 0xEE16, dvngLig( 446 ,"0x0B3C 0x0B4D 0x0B32"))); // 0x10129
        gOriyaLigMap.insert(std::make_pair( 0xEE17, dvngLig( 446 ,"0x0B3C 0x0B32 0x0B4D"))); // 0x10129
        gOriyaLigMap.insert(std::make_pair( 0xEE18, dvngLig( 447 ,"0x0B3C 0x0B4D 0x0B33"))); // 0x1012A
        gOriyaLigMap.insert(std::make_pair( 0xEE19, dvngLig( 447 ,"0x0B3C 0x0B33 0x0B4D"))); // 0x1012A
        gOriyaLigMap.insert(std::make_pair( 0xEE1A, dvngLig( 448 ,"0x0B3C 0x0B4D 0x0B36"))); // 0x1012B
        gOriyaLigMap.insert(std::make_pair( 0xEE1B, dvngLig( 448 ,"0x0B3C 0x0B36 0x0B4D"))); // 0x1012B
        gOriyaLigMap.insert(std::make_pair( 0xEE1C, dvngLig( 449 ,"0x0B3C 0x0B4D 0x0B37"))); // 0x1012C
        gOriyaLigMap.insert(std::make_pair( 0xEE1D, dvngLig( 449 ,"0x0B3C 0x0B37 0x0B4D"))); // 0x1012C
        gOriyaLigMap.insert(std::make_pair( 0xEE1E, dvngLig( 450 ,"0x0B3C 0x0B4D 0x0B38"))); // 0x1012D
        gOriyaLigMap.insert(std::make_pair( 0xEE1F, dvngLig( 450 ,"0x0B3C 0x0B38 0x0B4D"))); // 0x1012D
        gOriyaLigMap.insert(std::make_pair( 0xEE20, dvngLig( 451 ,"0x0B3C 0x0B4D 0x0B39"))); // 0x1012E
        gOriyaLigMap.insert(std::make_pair( 0xEE21, dvngLig( 451 ,"0x0B3C 0x0B39 0x0B4D"))); // 0x1012E
        gOriyaLigMap.insert(std::make_pair( 0xEE22, dvngLig( 452 ,"0x0B3C 0x0B4D 0x0B15 0x0B4D 0x0B37"))); // 0x1012F
        gOriyaLigMap.insert(std::make_pair( 0xEE23, dvngLig( 452 ,"0x0B3C 0x0B4D 0x0B15 0x0B37 0x0B4D"))); // 0x1012F
        gOriyaLigMap.insert(std::make_pair( 0xEE24, dvngLig( 452 ,"0x0B3C 0x0B15 0x0B4D 0x0B4D 0x0B37"))); // 0x1012F
        gOriyaLigMap.insert(std::make_pair( 0xEE25, dvngLig( 452 ,"0x0B3C 0x0B15 0x0B4D 0x0B37 0x0B4D"))); // 0x1012F
        gOriyaLigMap.insert(std::make_pair( 0xEE26, dvngLig( 453 ,"0x0B3C 0x0B41"))); // 0x10130
        gOriyaLigMap.insert(std::make_pair( 0xEE27, dvngLig( 454 ,"0x0B3C 0x0B42"))); // 0x10131
        gOriyaLigMap.insert(std::make_pair( 0xEE28, dvngLig( 455 ,"0x0B3C 0x0B43"))); // 0x10132
        gOriyaLigMap.insert(std::make_pair( 0xEE29, dvngLig( 456 ,"0x0B3C 0x0B44"))); // 0x10133
        gOriyaLigMap.insert(std::make_pair( 0xEE2A, dvngLig( 457 ,"0x0B3C 0x0B62"))); // 0x10134
        gOriyaLigMap.insert(std::make_pair( 0xEE2B, dvngLig( 458 ,"0x0B3C 0x0B63"))); // 0x10135
        gOriyaLigMap.insert(std::make_pair( 0xEE2C, dvngLig( 459 ,"0x0B15 0x0B4D 0x0B15"))); // 0x10136
        //gOriyaLigMap.insert(std::make_pair( 0xEE2D, dvngLig( 459 ,"0x0B15 0x0B15 0x0B4D"))); // 0x10136
        gOriyaLigMap.insert(std::make_pair( 0xEE2E, dvngLig( 460 ,"0x0B15 0x0B4D 0x0B24"))); // 0x10137
        //gOriyaLigMap.insert(std::make_pair( 0xEE2F, dvngLig( 460 ,"0x0B15 0x0B24 0x0B4D"))); // 0x10137
        gOriyaLigMap.insert(std::make_pair( 0xEE30, dvngLig( 461 ,"0x0B15 0x0B4D 0x0B33"))); // 0x10138
        //gOriyaLigMap.insert(std::make_pair( 0xEE31, dvngLig( 461 ,"0x0B15 0x0B33 0x0B4D"))); // 0x10138
        gOriyaLigMap.insert(std::make_pair( 0xEE32, dvngLig( 462 ,"0x0B17 0x0B4D 0x0B33"))); // 0x10139
        //gOriyaLigMap.insert(std::make_pair( 0xEE33, dvngLig( 462 ,"0x0B17 0x0B33 0x0B4D"))); // 0x10139
        gOriyaLigMap.insert(std::make_pair( 0xEE34, dvngLig( 463 ,"0x0B1C 0x0B4D 0x0B1C"))); // 0x1013A
        //gOriyaLigMap.insert(std::make_pair( 0xEE35, dvngLig( 463 ,"0x0B1C 0x0B1C 0x0B4D"))); // 0x1013A
        gOriyaLigMap.insert(std::make_pair( 0xEE36, dvngLig( 464 ,"0x0B23 0x0B4D 0x0B1F"))); // 0x1013B
        //gOriyaLigMap.insert(std::make_pair( 0xEE37, dvngLig( 464 ,"0x0B23 0x0B1F 0x0B4D"))); // 0x1013B
        gOriyaLigMap.insert(std::make_pair( 0xEE38, dvngLig( 465 ,"0x0B23 0x0B4D 0x0B20"))); // 0x1013C
        //gOriyaLigMap.insert(std::make_pair( 0xEE39, dvngLig( 465 ,"0x0B23 0x0B20 0x0B4D"))); // 0x1013C
        gOriyaLigMap.insert(std::make_pair( 0xEE3A, dvngLig( 469 ,"0x0B28 0x0B4D 0x0B24"))); // 0x10140
        //gOriyaLigMap.insert(std::make_pair( 0xEE3B, dvngLig( 469 ,"0x0B28 0x0B24 0x0B4D"))); // 0x10140
        gOriyaLigMap.insert(std::make_pair( 0xEE3C, dvngLig( 471 ,"0x0B28 0x0B4D 0x0B28"))); // 0x10142
        //gOriyaLigMap.insert(std::make_pair( 0xEE3D, dvngLig( 471 ,"0x0B28 0x0B28 0x0B4D"))); // 0x10142
        gOriyaLigMap.insert(std::make_pair( 0xEE3E, dvngLig( 472 ,"0x0B2A 0x0B4D 0x0B24"))); // 0x10143
        //gOriyaLigMap.insert(std::make_pair( 0xEE3F, dvngLig( 472 ,"0x0B2A 0x0B24 0x0B4D"))); // 0x10143
        gOriyaLigMap.insert(std::make_pair( 0xEE40, dvngLig( 474 ,"0x0B32 0x0B4D 0x0B15"))); // 0x10145
        //gOriyaLigMap.insert(std::make_pair( 0xEE41, dvngLig( 474 ,"0x0B32 0x0B15 0x0B4D"))); // 0x10145
        gOriyaLigMap.insert(std::make_pair( 0xEE42, dvngLig( 475 ,"0x0B32 0x0B4D 0x0B32"))); // 0x10146
        //gOriyaLigMap.insert(std::make_pair( 0xEE43, dvngLig( 475 ,"0x0B32 0x0B32 0x0B4D"))); // 0x10146
        gOriyaLigMap.insert(std::make_pair( 0xEE44, dvngLig( 476 ,"0x0B33 0x0B4D 0x0B15"))); // 0x10147
        //gOriyaLigMap.insert(std::make_pair( 0xEE45, dvngLig( 476 ,"0x0B33 0x0B15 0x0B4D"))); // 0x10147
        gOriyaLigMap.insert(std::make_pair( 0xEE46, dvngLig( 477 ,"0x0B33 0x0B4D 0x0B33"))); // 0x10148
        //gOriyaLigMap.insert(std::make_pair( 0xEE47, dvngLig( 477 ,"0x0B33 0x0B33 0x0B4D"))); // 0x10148
        gOriyaLigMap.insert(std::make_pair( 0xEE48, dvngLig( 479 ,"0x0B36 0x0B4D 0x0B33"))); // 0x1014A
        //gOriyaLigMap.insert(std::make_pair( 0xEE49, dvngLig( 479 ,"0x0B36 0x0B33 0x0B4D"))); // 0x1014A
        gOriyaLigMap.insert(std::make_pair( 0xEE4A, dvngLig( 480 ,"0x0B37 0x0B4D 0x0B1F"))); // 0x1014B
        //gOriyaLigMap.insert(std::make_pair( 0xEE4B, dvngLig( 480 ,"0x0B37 0x0B1F 0x0B4D"))); // 0x1014B
        gOriyaLigMap.insert(std::make_pair( 0xEE4C, dvngLig( 481 ,"0x0B37 0x0B4D 0x0B20"))); // 0x1014C
        //gOriyaLigMap.insert(std::make_pair( 0xEE4D, dvngLig( 481 ,"0x0B37 0x0B20 0x0B4D"))); // 0x1014C
        gOriyaLigMap.insert(std::make_pair( 0xEE4E, dvngLig( 483 ,"0x0B38 0x0B4D 0x0B16"))); // 0x1014E
        //gOriyaLigMap.insert(std::make_pair( 0xEE4F, dvngLig( 483 ,"0x0B38 0x0B16 0x0B4D"))); // 0x1014E
        gOriyaLigMap.insert(std::make_pair( 0xEE50, dvngLig( 484 ,"0x0B38 0x0B4D 0x0B24"))); // 0x1014F
        //gOriyaLigMap.insert(std::make_pair( 0xEE51, dvngLig( 484 ,"0x0B38 0x0B24 0x0B4D"))); // 0x1014F
        gOriyaLigMap.insert(std::make_pair( 0xEE52, dvngLig( 486 ,"0x0B38 0x0B4D 0x0B2A"))); // 0x10151
        //gOriyaLigMap.insert(std::make_pair( 0xEE53, dvngLig( 486 ,"0x0B38 0x0B2A 0x0B4D"))); // 0x10151
        gOriyaLigMap.insert(std::make_pair( 0xEE54, dvngLig( 487 ,"0x0B38 0x0B4D 0x0B2B"))); // 0x10152
        //gOriyaLigMap.insert(std::make_pair( 0xEE55, dvngLig( 487 ,"0x0B38 0x0B2B 0x0B4D"))); // 0x10152
        gOriyaLigMap.insert(std::make_pair( 0xEE56, dvngLig( 582 ,"0x0B39 0x0B41"))); // 0x101B1
        gOriyaLigMap.insert(std::make_pair( 0xEE57, dvngLig( 583 ,"0x0B39 0x0B42"))); // 0x101B2
        gOriyaLigMap.insert(std::make_pair( 0xEE58, dvngLig( 584 ,"0x0B39 0x0B43"))); // 0x101B3
        gOriyaLigMap.insert(std::make_pair( 0xEE59, dvngLig( 585 ,"0x0B39 0x0B4D 0x0B15"))); // 0x101B4
        gOriyaLigMap.insert(std::make_pair( 0xEE5A, dvngLig( 585 ,"0x0B39 0x0B15 0x0B4D"))); // 0x101B4
        gOriyaLigMap.insert(std::make_pair( 0xEE5B, dvngLig( 586 ,"0x0B39 0x0B4D 0x0B1C"))); // 0x101B5
        gOriyaLigMap.insert(std::make_pair( 0xEE5C, dvngLig( 586 ,"0x0B39 0x0B1C 0x0B4D"))); // 0x101B5
        gOriyaLigMap.insert(std::make_pair( 0xEE5D, dvngLig( 587 ,"0x0B39 0x0B4D 0x0B28"))); // 0x101B6
        gOriyaLigMap.insert(std::make_pair( 0xEE5E, dvngLig( 587 ,"0x0B39 0x0B28 0x0B4D"))); // 0x101B6
        gOriyaLigMap.insert(std::make_pair( 0xEE5F, dvngLig( 588 ,"0x0B39 0x0B4D 0x0B71"))); // 0x101B7
        gOriyaLigMap.insert(std::make_pair( 0xEE60, dvngLig( 588 ,"0x0B39 0x0B4D 0x0B35"))); // 0x101B7
        gOriyaLigMap.insert(std::make_pair( 0xEE61, dvngLig( 588 ,"0x0B39 0x0B4D 0x0B2C"))); // 0x101B7
        gOriyaLigMap.insert(std::make_pair( 0xEE62, dvngLig( 588 ,"0x0B39 0x0B71 0x0B4D"))); // 0x101B7
        gOriyaLigMap.insert(std::make_pair( 0xEE63, dvngLig( 588 ,"0x0B39 0x0B35 0x0B4D"))); // 0x101B7
        gOriyaLigMap.insert(std::make_pair( 0xEE64, dvngLig( 588 ,"0x0B39 0x0B2C 0x0B4D"))); // 0x101B7
        gOriyaLigMap.insert(std::make_pair( 0xEE65, dvngLig( 589 ,"0x0B39 0x0B4D 0x0B2E"))); // 0x101B8
        gOriyaLigMap.insert(std::make_pair( 0xEE66, dvngLig( 589 ,"0x0B39 0x0B2E 0x0B4D"))); // 0x101B8
        gOriyaLigMap.insert(std::make_pair( 0xEE67, dvngLig( 590 ,"0x0B39 0x0B4D 0x0B30"))); // 0x101B9
        gOriyaLigMap.insert(std::make_pair( 0xEE68, dvngLig( 590 ,"0x0B39 0x0B30 0x0B4D"))); // 0x101B9
        gOriyaLigMap.insert(std::make_pair( 0xEE69, dvngLig( 591 ,"0x0B39 0x0B4D 0x0B32"))); // 0x101BA
        gOriyaLigMap.insert(std::make_pair( 0xEE6A, dvngLig( 591 ,"0x0B39 0x0B32 0x0B4D"))); // 0x101BA
        gOriyaLigMap.insert(std::make_pair( 0xEE6B, dvngLig( 592 ,"0x0B39 0x0B4D 0x0B33"))); // 0x101BB
        gOriyaLigMap.insert(std::make_pair( 0xEE6C, dvngLig( 592 ,"0x0B39 0x0B33 0x0B4D"))); // 0x101BB
        //gOriyaLigMap.insert(std::make_pair( 0xEE6D, dvngLig( 593 ,"0x0B5F 0x0B41"))); // 0x101BC
        gOriyaLigMap.insert(std::make_pair( 0xEE6E, dvngLig( 594 ,"0x0B5F 0x0B42"))); // 0x101BD
        gOriyaLigMap.insert(std::make_pair( 0xEE6F, dvngLig( 595 ,"0x0B5F 0x0B43"))); // 0x101BE
        gOriyaLigMap.insert(std::make_pair( 0xEE70, dvngLig( 596 ,"0x0B5F 0x0B4D 0x0B15"))); // 0x101BF
        //gOriyaLigMap.insert(std::make_pair( 0xEE71, dvngLig( 596 ,"0x0B5F 0x0B15 0x0B4D"))); // 0x101BF
        gOriyaLigMap.insert(std::make_pair( 0xEE72, dvngLig( 597 ,"0x0B5F 0x0B4D 0x0B1C"))); // 0x101C0
        gOriyaLigMap.insert(std::make_pair( 0xEE73, dvngLig( 597 ,"0x0B5F 0x0B1C 0x0B4D"))); // 0x101C0
        gOriyaLigMap.insert(std::make_pair( 0xEE74, dvngLig( 598 ,"0x0B5F 0x0B4D 0x0B28"))); // 0x101C1
        gOriyaLigMap.insert(std::make_pair( 0xEE75, dvngLig( 598 ,"0x0B5F 0x0B28 0x0B4D"))); // 0x101C1
        gOriyaLigMap.insert(std::make_pair( 0xEE76, dvngLig( 599 ,"0x0B5F 0x0B4D 0x0B71"))); // 0x101C2
        gOriyaLigMap.insert(std::make_pair( 0xEE77, dvngLig( 599 ,"0x0B5F 0x0B4D 0x0B35"))); // 0x101C2
        gOriyaLigMap.insert(std::make_pair( 0xEE78, dvngLig( 599 ,"0x0B5F 0x0B4D 0x0B2C"))); // 0x101C2
        gOriyaLigMap.insert(std::make_pair( 0xEE79, dvngLig( 599 ,"0x0B5F 0x0B71 0x0B4D"))); // 0x101C2
        gOriyaLigMap.insert(std::make_pair( 0xEE7A, dvngLig( 599 ,"0x0B5F 0x0B35 0x0B4D"))); // 0x101C2
        gOriyaLigMap.insert(std::make_pair( 0xEE7B, dvngLig( 599 ,"0x0B5F 0x0B2C 0x0B4D"))); // 0x101C2
        gOriyaLigMap.insert(std::make_pair( 0xEE7C, dvngLig( 600 ,"0x0B5F 0x0B4D 0x0B2E"))); // 0x101C3
        //gOriyaLigMap.insert(std::make_pair( 0xEE7D, dvngLig( 600 ,"0x0B5F 0x0B2E 0x0B4D"))); // 0x101C3
        gOriyaLigMap.insert(std::make_pair( 0xEE7E, dvngLig( 601 ,"0x0B5F 0x0B4D 0x0B30"))); // 0x101C4
        gOriyaLigMap.insert(std::make_pair( 0xEE7F, dvngLig( 601 ,"0x0B5F 0x0B30 0x0B4D"))); // 0x101C4
        gOriyaLigMap.insert(std::make_pair( 0xEE80, dvngLig( 602 ,"0x0B5F 0x0B4D 0x0B32"))); // 0x101C5
        gOriyaLigMap.insert(std::make_pair( 0xEE81, dvngLig( 602 ,"0x0B5F 0x0B32 0x0B4D"))); // 0x101C5
        gOriyaLigMap.insert(std::make_pair( 0xEE82, dvngLig( 603 ,"0x0B5F 0x0B4D 0x0B33"))); // 0x101C6
        gOriyaLigMap.insert(std::make_pair( 0xEE83, dvngLig( 603 ,"0x0B5F 0x0B33 0x0B4D"))); // 0x101C6
        gOriyaLigMap.insert(std::make_pair( 0xEE84, dvngLig( 609 ,"0x0B4D 0x0B71 0x0B4D 0x0B30"))); // 0x101CC
        gOriyaLigMap.insert(std::make_pair( 0xEE85, dvngLig( 609 ,"0x0B4D 0x0B71 0x0B30 0x0B4D"))); // 0x101CC
        gOriyaLigMap.insert(std::make_pair( 0xEE86, dvngLig( 609 ,"0x0B4D 0x0B35 0x0B4D 0x0B30"))); // 0x101CC
        gOriyaLigMap.insert(std::make_pair( 0xEE87, dvngLig( 609 ,"0x0B4D 0x0B35 0x0B30 0x0B4D"))); // 0x101CC
        gOriyaLigMap.insert(std::make_pair( 0xEE88, dvngLig( 609 ,"0x0B4D 0x0B2C 0x0B4D 0x0B30"))); // 0x101CC
        gOriyaLigMap.insert(std::make_pair( 0xEE89, dvngLig( 609 ,"0x0B4D 0x0B2C 0x0B30 0x0B4D"))); // 0x101CC
        gOriyaLigMap.insert(std::make_pair( 0xEE8A, dvngLig( 609 ,"0x0B71 0x0B4D 0x0B4D 0x0B30"))); // 0x101CC
        gOriyaLigMap.insert(std::make_pair( 0xEE8B, dvngLig( 609 ,"0x0B71 0x0B4D 0x0B30 0x0B4D"))); // 0x101CC
        gOriyaLigMap.insert(std::make_pair( 0xEE8C, dvngLig( 609 ,"0x0B35 0x0B4D 0x0B4D 0x0B30"))); // 0x101CC
        gOriyaLigMap.insert(std::make_pair( 0xEE8D, dvngLig( 609 ,"0x0B35 0x0B4D 0x0B30 0x0B4D"))); // 0x101CC
        gOriyaLigMap.insert(std::make_pair( 0xEE8E, dvngLig( 609 ,"0x0B2C 0x0B4D 0x0B4D 0x0B30"))); // 0x101CC
        gOriyaLigMap.insert(std::make_pair( 0xEE8F, dvngLig( 609 ,"0x0B2C 0x0B4D 0x0B30 0x0B4D"))); // 0x101CC
        gOriyaLigMap.insert(std::make_pair( 0xEE90, dvngLig( 610 ,"0x0B23 0x0B4D 0x0B21 0x0B4D 0x0B30"))); // 0x101CD
        gOriyaLigMap.insert(std::make_pair( 0xEE91, dvngLig( 610 ,"0x0B23 0x0B4D 0x0B21 0x0B30 0x0B4D"))); // 0x101CD
        gOriyaLigMap.insert(std::make_pair( 0xEE92, dvngLig( 610 ,"0x0B23 0x0B21 0x0B4D 0x0B4D 0x0B30"))); // 0x101CD
        gOriyaLigMap.insert(std::make_pair( 0xEE93, dvngLig( 610 ,"0x0B23 0x0B21 0x0B4D 0x0B30 0x0B4D"))); // 0x101CD
        gOriyaLigMap.insert(std::make_pair( 0xEE94, dvngLig( 611 ,"0x0B24 0x0B4D 0x0B15 0x0B4D 0x0B30"))); // 0x101CE
        gOriyaLigMap.insert(std::make_pair( 0xEE95, dvngLig( 611 ,"0x0B24 0x0B4D 0x0B15 0x0B30 0x0B4D"))); // 0x101CE
        gOriyaLigMap.insert(std::make_pair( 0xEE96, dvngLig( 611 ,"0x0B24 0x0B15 0x0B4D 0x0B4D 0x0B30"))); // 0x101CE
        gOriyaLigMap.insert(std::make_pair( 0xEE97, dvngLig( 611 ,"0x0B24 0x0B15 0x0B4D 0x0B30 0x0B4D"))); // 0x101CE

        // for (0x0B16 0x0B3f), (0x0B25 0x0B3f), (0x0B27 0x0B3f) ligatures, replacing 0x0B3f with 0xEE98
        gOriyaLigMap.insert(std::make_pair( 0xEE98, dvngLig( 261 ,"0xFFFF"))); // 0x100A8

        gOriyaLigMap.insert(std::make_pair( 0xEE99, dvngLig( 318 ,"0x0B30 0x0B4D"))); // 0x100E1 //ra virama full MANUALLY ADDED
    }
    return gOriyaLigMap;
}


#if 0
void SwitchOriyaVirama(lString16* str)
{
/*
 * ( ? ' ) -> ( ' ? )
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0B4D)
        {
            str->at(i + 0) = str->at(i - 1);
            str->at(i - 1) = 0x0B4D;
        }
    }
}

void SwitchOriyaViarama_reverse(lString16* str)
{
/*
 *  ( ' ? ) -> ( ? ' )
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0B4D)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0x0B4D;
        }
    }
}
#endif

bool isBLWF(lChar16 ch)
{
    // BLWF == below base form == (0x0b4d + ??)
    // BLWF == 0xECD1 0xECB7 0xECEF 0xECC9 0xECA4 0xECDB ///this list would increase

    return ( (ch >= 0xECA5 && ch <= 0xECEF) ||
             (ch >= 0xEDDF && ch <= 0xEE2B) ||
             (ch >= 0xEDDF && ch <= 0xEE2B) ||
            ch == 0xECEF || //ya postform
            ch == 0xECA4    //ra virama repha form
            );
}

void StripZWNJ_oriya(lString16 *word)
{
    for (int i = 0; i < word->size(); ++i)
    {
        //if (word->at(i) == 0x200C) word->erase(i);
        if (word->at(i) == 0x200C || word->at(i) == 0x200D) word->at(i) = 0x200B;
    }
}

void StripZWNJ_oriya_reverse(lString16 *word)
{
    for (int i = 0; i < word->size(); ++i)
    {
        if (word->at(i) == 0x200B) word->at(i) = 0x200D;
    }
}

void ChangeOriyaRaViramaLigature(lString16* str)
{
/*
 * RPH  = 0xECA4
 * RV   = 0xEDD4 //last character in word ONLY
 * ( 0x0B30 0x0B4D ) -> ( 0xECA4 )
 * ( 0x0B30 0x0B4D ) -> ( 0xEDD4 ) // full form on the end of a word
 */
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 0; i < str->length()-1; i++)
    {
        int ch = str->at(i);
        int nextch = str->at(i + 1);
        int nextch2 = 0;

        if(i <= str->length()-3)
        {
            nextch2 = str->at(i + 2);
        }
        if (ch == 0x0B30 && nextch == 0x0B4D)
        {
            if( nextch2 == 0x200D)
            {
                //before any letter
                str->at(i) = 0xEE99;
                str->erase(i + 1);
            }
            else if (i < str->length() - 2)
            {
                //before any letter
                str->at(i) = 0xECA4;
                str->erase(i + 1);
            }
            else
            {
                //no letters after this combo
                str->at(i) = 0xEDD4;
                str->erase(i + 1);
            }
        }
    }
}

void SwitchOriyaNV_BLWF(lString16 *str)
{
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 0; i < str->length()-1; i++)
    {
        int ch = str->at(i);
        int nextch = str->at(i+1);

        if (ch == 0xECEF)
        {
            if (nextch == 0x0B01 ||
                nextch == 0x0B02 ||
                nextch == 0x0B41 ||
                nextch == 0x0B42 ||
                nextch == 0x0B43 ||
                nextch == 0x0B44 ||
                nextch == 0x0B62 ||
                nextch == 0x0B63)
            {
                str->at(i + 0) = nextch;
                str->at(i + 1) = 0xECEF;
            }
        }
    }
}

void SwitchOriyaNV_BLWF_reverse(lString16 *str)
{
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 0; i < str->length()-1; i++)
    {
        if(str->at(i+1) == 0xECEF &&
           (
                   str->at(i) == 0x0B01 ||
                   str->at(i) == 0x0B02 ||
                   str->at(i) == 0x0B41 ||
                   str->at(i) == 0x0B42 ||
                   str->at(i) == 0x0B43 ||
                   str->at(i) == 0x0B44 ||
                   str->at(i) == 0x0B62 ||
                   str->at(i) == 0x0B63 )
                )
        {
            str->at(i + 1) = str->at(i+0);
            str->at(i + 0) = 0xECEF;
        }
    }
}

void SwitchOriyaNaVirama(lString16* str)
{
    /*
    * ( RPH ? ) -> ( ? RPH )
    */
    if (str->length() < 2)
    {
        return;
    }
    for (int i = str->length() - 1; i >= 1; i--)
    {
        if (str->at(i) == 0x0B28 && str->at(i + 1) == 0x0B4D)
        {
            if (i == str->length() - 2)
            {
                str->at(i) = 0xEDCD;
            }
            else
            {
                str->at(i) = 0xECCA;
            }
            str->erase(i + 1);
        }
    }
}

void SwitchOriyaRaVirama(lString16* str)
{
 /*
 * RPH  = 0xECA4
 * ( RPH ? ) -> ( ? RPH )
 */
    if (str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-1; i >= 1 ; i--)
    {
        if(str->at(i - 1) == 0xECA4)
        {
            str->at(i - 1) = str->at(i - 0);
            str->at(i - 0) = 0xECA4;
        }
    }
}

void SwitchOriyaRaVirama_reverse(lString16* str)
{
/*
 * RV_ == 0xECDA == 0x0B30 + 0x0B4D
 * RPH == 0xECA4 == 0x0B30 + 0x0B4D
 * RV  == 0xEDD4 == 0x0B30 + 0x0B4D
 * ( ? RPH  ) -> ( 0x0B30 0x0B4D ? )
 * ( ? RPH2 ) -> ( 0x0B30 0x0B4D ? )
 * ( ? RV   ) -> ( 0x0B30 0x0B4D ? )
 */

    if (str->length() < 2)
    {
        return;
    }

    for (int i = 0; i < str->length()-1; i++)
    {
        if ( str->at(i + 1) == 0xECA4)
        {
            int ch = str->at(i);
            str->at(i + 0) = 0x0B30;
            str->at(i + 1) = 0x0B4D;
            str->insert(i+2, 1, ch);
            i+=2;
        }
        if ( str->at(i + 1) == 0xEDD4)
        {
            str->at(i + 1) = 0x0B30;
            str->insert(i+2, 1, 0x0B4D);
            i+=2;
        }
    }
}

void SwitchOriyaI(lString16* str)
{
/* i = 0x0B3f
 * , = 0xEE98
 * (kha I) -> (kha ,)  // (0x0B16 0x0B3f) -> (0x0B16 0xEE98)
 * (tha I) -> (tha ,)  // (0x0B25 0x0B3f) -> (0x0B25 0xEE98)
 * (dha I) -> (dha ,)  // (0x0B27 0x0B3f) -> (0x0B27 0xEE98)
*/
    for (int i = 0; i < str->length()-1; i++)
    {
        if(str->at(i+1) == 0x0B3f )
        {
            if(str->at(i) == 0x0B16 ||
               str->at(i) == 0x0B25 ||
               str->at(i) == 0x0B27 )
            {
                str->at(i+1) = 0xEE98;
                i++;
            }
        }
    }
}

void SwitchOriyaE(lString16* str)
{
/*  E == 0x0B47
 *
 *  ( ? 0xECEF E)   -> ( E ? 0xECEF)
 *  ( ? 0xECF0 E)   -> ( E ? 0xECF0)
 *  ( ? 0xECF1 E)   -> ( E ? 0xECF1)
 *  ( ? 0xECF2 E)   -> ( E ? 0xECF2)
 *  ( ? 0xECD9 E)   -> ( E ? 0xECD9)
 *  ( ? 0xECDA E)   -> ( E ? 0xECDA)
 *  ( ? 0xECA4 E)   -> ( E ? 0xECA4 )
 *  ( ? BLWF E)     -> ( E ? BLWF)
 *  ( ? BLWF1 BLWF2 E) -> ( E ? BLWF1 BLWF2)
 *  ( ? E )  -> ( E ? )
 */
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0B47)
        {
            if (i >= 3 && isBLWF(str->at(i-1)) && isBLWF(str->at(i-2)))
            {
                //pos    -3 -2 -1 0
                //before A  B  C  E
                //after  E  A  B  C

                int ch1 = str->at(i - 3);
                int ch2 = str->at(i - 2);
                int ch3 = str->at(i - 1);

                str->at(i - 3) = 0x0B47;
                str->at(i - 2) = ch1;
                str->at(i - 1) = ch2;
                str->at(i - 0) = ch3;
            }
            else if( str->at(i - 1) == 0xECEF ||  //ya postform
                     str->at(i - 1) == 0xECF0 ||  //ya postform
                     str->at(i - 1) == 0xECF1 ||  //ya postform
                     str->at(i - 1) == 0xECF2 ||  //ya postform
                     str->at(i - 1) == 0xECD9 ||  //ra virama underline
                     str->at(i - 1) == 0xECDA ||  //ra virama underline
                     isBLWF(str->at(i - 1)))
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x0B47;
            }
            else
            {
                //default
                str->at(i + 0) = str->at(i - 1);
                str->at(i - 1) = 0x0B47;
            }
        }
    }
}

void SwitchOriyaAI(lString16* str)
{
/*
 * AI = 0x0B48
 * ai_length_mark = 0x0B56
 * (? AI)             -> (e ? ai_length_mark)
 * (? BLWF AI         -> (e ? BLWF ai_length_mark)
 * (? BLWF1 BLWF2 AI) -> (e ? BLWF1 BLWF2 ai_length_mark)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if(str->at(i) == 0x0B48)
        {
            if (i >= 3 && isBLWF(str->at(i - 1)) && isBLWF(str->at(i - 2)))
            {
                //pos    -3 -2 -1 0 +1
                //before A  B  C  AU
                //after  e  A  B  C au

                int ch1 = str->at(i - 3);
                int ch2 = str->at(i - 2);
                int ch3 = str->at(i - 1);

                str->at(i - 3) = 0x0B47;
                str->at(i - 2) = ch1;
                str->at(i - 1) = ch2;
                str->at(i - 0) = ch3;
                str->insert(i + 1, 1, 0x0B56);
            }
            else if ( i >= 2 && isBLWF(str->at(i - 1)))
            {
                int ch0 = str->at(i - 2);
                int ch1 = str->at(i - 1);

                str->at(i - 2) = 0x0B47;
                str->at(i - 1) = ch0;
                str->at(i - 0) = ch1;
                str->insert(i + 1, 1, 0x0B56);
            }
            else
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x0B47;
                str->insert(i + 1, 1, 0x0B56);
            }
        }
    }
}

void SwitchOriyaO(lString16 *str)
{
/*
 * RV_  == 0xECD9 == 0x0B30 + 0x0B4D
 * RV_1 == 0xECDA == 0x0B30 + 0x0B4D
 * RV_2 == 0xECA4 == 0x0B30 + 0x0B4D
 *
 * (? RV_  O) -> (e ? RV_  aa)
 * (? RV_1 O) -> (e ? RV_1 aa)
 * (? RV_2 O) -> (e ? RV_2 aa)
 * (? BLWF O) -> (e ? BLWF aa)
 * (? BLWF1 BLWF2 O) -> (e ? BLWF1 BLWF2 aa)
 *
 *
 * (? O )     -> (e ? aa)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0B4B)
        {
            if (i >= 3 && isBLWF(str->at(i - 1)) && isBLWF(str->at(i - 2)))
            {
                //pos    -3 -2 -1 0 +1
                //before A  B  C  O
                //after  e  A  B  C oo

                int ch1 = str->at(i - 3);
                int ch2 = str->at(i - 2);
                int ch3 = str->at(i - 1);

                str->at(i - 3) = 0x0B47;
                str->at(i - 2) = ch1;
                str->at(i - 1) = ch2;
                str->at(i - 0) = ch3;
                str->insert(i + 1, 1, 0x0B3E);
            }
            else if(i >= 2 && (str->at(i-1)== 0xECD9 ||str->at(i-1)== 0xECDA))
            {
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x0B47;
                str->at(i - 0) = 0xECD9;
                str->insert(i + 1, 1, 0x0B3E);
            }

            else if (i >= 2 && isBLWF(str->at(i-1)))
            {
                int ch = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x0B47;
                str->at(i - 0) = ch;
                str->insert(i + 1, 1, 0x0B3E);
            }
            else
            {
                //default
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x0B47;
                str->insert(i + 1, 1, 0x0B3E);
            }
        }
    }
}

void SwitchOriyaAU(lString16 *str)
{
/*
 * (? AU)      -> (e ? au_length_mark)
 * (? BLWF AU) -> (e ? BLWF au_length_mark)
 * (? BLWF1 BLWF2 AU) -> (e ? BLWF1 BLWF2 au_length_mark)
 *
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0B4C)
        {
            if (i >= 3 && isBLWF(str->at(i - 1)) && isBLWF(str->at(i - 2)))
            {
                //pos    -3 -2 -1 0 +1
                //before A  B  C  AU
                //after  e  A  B  C au

                int ch1 = str->at(i - 3);
                int ch2 = str->at(i - 2);
                int ch3 = str->at(i - 1);

                str->at(i - 3) = 0x0B47;
                str->at(i - 2) = ch1;
                str->at(i - 1) = ch2;
                str->at(i - 0) = ch3;
                str->insert(i + 1, 1, 0x0B57);
            }
            else if (i >= 2 && isBLWF(str->at(i - 1)))
            {
                int ch = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x0B47;
                str->at(i - 0) = ch;
                str->insert(i + 1, 1, 0x0B57);
            }
            else
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x0B47;
                str->insert(i + 1, 1, 0x0B57);
            }
        }
    }
}

void SwitchOriyaI_reverse(lString16* str)
{
/* i = 0x0B3f
 * , = 0xEE98
 * (kha ,) -> (kha I) // (0x0B16 0xEE98) -> (0x0B16 0x0B3f)
 * (tha ,) -> (tha I) // (0x0B25 0xEE98) -> (0x0B25 0x0B3f)
 * (dha ,) -> (dha I) // (0x0B27 0xEE98) -> (0x0B27 0x0B3f)
*/
    for (int i = 0; i < str->length()-1; i++)
    {
        if(str->at(i+1) == 0xEE98 )
        {
            if(str->at(i) == 0x0B16 ||
               str->at(i) == 0x0B25 ||
               str->at(i) == 0x0B27 )
            {
                str->at(i+1) = 0x0B3f;
                i++;
            }
        }
    }
}

void SwitchOriyaE_reverse(lString16* str)
{
/*  ( E ? 0xECEF) -> ( ? 0xECEF E )
 *  ( E ? 0xECF0) -> ( ? 0xECF0 E )
 *  ( E ? 0xECF1) -> ( ? 0xECF1 E )
 *  ( E ? 0xECF2) -> ( ? 0xECF2 E )
 *  ( E ? 0xECD9) -> ( ? 0xECD9 E )
 *  ( E ? 0xECDA) -> ( ? 0xECDA E )
 *  ( E ? 0xECA4) -> ( ? 0xECA4 E )
 *
 *  ( E ? BLWF1 BLWF2) -> ( ? BLWF1 BLWF2 E)
 *
 *  ( E ? )       -> ( ? E )
 */
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2 ; i >= 0 ; i--)
    {
        if (str->at(i) == 0x0B47)
        {
            if (i <= str->length() - 4 &&
            isBLWF(str->at(i + 2)) &&
            isBLWF(str->at(i + 3)))
            {
                //( E ? BLWF1 BLWF2) -> ( ? BLWF1 BLWF2 E)
                str->at(i+0) = str->at(i+1);
                str->at(i+1) = str->at(i+2);
                str->at(i+2) = str->at(i+3);
                str->at(i+3) = 0x0B47;
            }
            else if( i <= str->length()-3 &&
                ( str->at(i + 2) == 0xECEF ||  //ya postform
                  str->at(i + 2) == 0xECF0 ||  //ya postform
                  str->at(i + 2) == 0xECF1 ||  //ya postform
                  str->at(i + 2) == 0xECF2 ||  //ya postform
                  str->at(i + 2) == 0xECD9 ||  //ra virama underline
                  str->at(i + 2) == 0xECDA ||  //ra virama underline
                  str->at(i + 2) == 0xECA4 ||  //ra virama repha form
                  isBLWF(str->at(i+2))
                  ))
            {
                str->at(i + 0) = str->at(i+1);
                str->at(i + 1) = str->at(i+2);
                str->at(i + 2) = 0x0B47;
                i+=2;
            }
            else
            {
                //default
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = 0x0B47;
                i++;
            }
        }
    }
}

void SwitchOriyaAI_reverse(lString16* str)
{
/*
 *  (e ? ai_length_mark)   -> (? AI)
 *
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0B47)
        {
            if( i <= str->length()-4 &&
                str->at(i+4) == 0x0B48 &&
                isBLWF(str->at(i + 2)) &&
                isBLWF(str->at(i + 3))
                )
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = str->at(i + 3);
                str->at(i + 3) = 0x09CB;
                str->erase(i + 4);
            }
            else if (str->at(i + 2) == 0x0B48)
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x09CB;
                str->erase(i + 2);
            }
        }
    }
}

void SwitchOriyaO_reverse(lString16* str)
{
/*
 * RV_  == 0xECD9 == 0x0B30 + 0x0B4D
 * RV_1 == 0xECDA == 0x0B30 + 0x0B4D
 * RV_2 == 0xECA4 == 0x0B30 + 0x0B4D
 * (e ? RV_  aa) -> (? RV_  O)
 * (e ? RV_1 aa) -> (? RV_1 O)
 * (e ? RV_2 aa) -> (? RV_2 O)
 * (e ? aa)      -> (? O)
 * (e ? BLWF1 BLWF2 aa) -> (? BLWF1 BLWF2 O)
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0B47)
        {
            if (i<=str->length()-3 && str->at(i + 4) == 0x0B3E)
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = str->at(i + 3);
                str->at(i + 3) = 0x0B4B;
                str->erase(i + 4);
            }
            else if (i<=str->length()-3 && str->at(i + 3) == 0x0B3E &&
                      ( str->at(i + 2)  == 0xECD9 ||
                        str->at(i + 2)  == 0xECA4 ||
                        str->at(i + 2)  == 0xECDA ||
                        isBLWF(str->at(i + 2))
                      )
                    )
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = 0x0B4B;
                str->erase(i + 3);
            }
            else if ( i<=str->length()-3 && str->at(i + 3) == 0x0B3E &&  str->at(i+2) == 0xECA4)
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = 0xECA4;
                str->at(i + 2) = 0x0B4B;
                str->erase(i + 3);
            }
            if (str->at(i + 2) == 0x0B3E)
            {
                //default
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x0B4B;
                str->erase(i + 2);
            }
        }
    }
}

void SwitchOriyaAU_reverse(lString16* str)
{
/*
 *  e = 0x0B47
 *  au_length_mark = x0B57
 *  AU = 0x0B4C
 *  (e ? au_length_mark)   -> (? AU)
 *  (e ? BLWF  au_length_mark) -> (? BLWF AU)
 *  (e ? BLWF1 BLWF2 au_length_mark) -> (? BLWF1 BLWF2 AU)
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0B47)
        {
            if (i <= str->length() - 4 &&
            str->at(i + 4) == 0x0B57 &&
            isBLWF(str->at(i + 2)) &&
            isBLWF(str->at(i + 3)))
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = str->at(i + 3);
                str->at(i + 3) = 0x0B4C;
                str->erase(i + 4);
            }
            else if (i <= str->length()-3 &&
                    str->at(i+3) == 0x0B57)
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = 0x0B4C;
                str->erase(i + 3);
            }
            else if (str->at(i + 2) == 0x0B57)
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x0B4C;
                str->erase(i + 2);
            }
        }
    }
}

lString16 processOriyaLigatures(lString16 word)
{
    int j = (word.length() >= 7 ) ? 7 : word.length();
    for(;  j >= 2; j--)
    {
        for (int c = word.length() - j; c >= 0; c--)
        {
            lUInt32 fastkey = (word.at(c) << 16) + word.at(c + 1);
            if (gOriyaFastLigMap.find(fastkey) == gOriyaFastLigMap.end())
            {
                continue;
            }
            dvngLig lig(word.substr(c, j));
            lChar16 rep = findOriyaLigRev(lig);

            if (rep != 0)
            {
                word.replace(c, j, lString16(&rep, 1));
                c -= j - 2;
            }
        }
    }
    return word;
}


void SwitchDhaViramaYa_U_UU_R(lString16 *str)
{
    // dha virama ya U  -> dha U  virama ya
    // dha virama ya UU -> dha UU virama ya
    // dha virama ya R  -> dha R  virama ya
    if (str->length() < 4)
    {
        return;
    }
    for (int i = 0; i < str->length()-4; i++)
    {
        int ch0 = str->at(i+0);
        int ch1 = str->at(i+1);
        int ch2 = str->at(i+2);
        int ch3 = str->at(i+3);

        if(ch0 == 0x0B27 && ch1 == 0x0B4D && ch2 == 0x0B5F )
        {
            if (ch3 == 0x0B41 || ch3 == 0x0B42 || ch3 == 0x0B43)
            {
                str->at(i+1) = ch3;
                str->at(i+2) = ch1;
                str->at(i+3) = ch2;
            }
        }
    }
}

void SwitchDhaViramaYa_U_UU_R_reverse(lString16 *str)
{
    // dha U  virama ya ->  dha virama ya U
    // dha UU virama ya ->  dha virama ya UU
    // dha R  virama ya ->  dha virama ya R
    if (str->length() < 4)
    {
        return;
    }
    for (int i = 0; i < str->length()-4; i++)
    {
        int ch0 = str->at(i+0);
        int ch1 = str->at(i+1);
        int ch2 = str->at(i+2);
        int ch3 = str->at(i+3);

        if(ch0 == 0x0B27 && ch2 == 0x0B4D && ch3 == 0x0B5F )
        {
            if (ch1 == 0x0B41 || ch1 == 0x0B42 || ch1 == 0x0B43)
            {
                str->at(i+1) = ch2;
                str->at(i+2) = ch3;
                str->at(i+3) = ch1;
            }
        }
    }
}

lString16 lString16::processOriyaText()
{
    if(length() < 2)
    {
        return *this;
    }
    lString16 res;
    lString16Collection words;

    words.parse(*this,' ',true);
    for (int i = 0; i < words.length(); i++)
    {
        lString16 word = words.at(i);

        if(word.length()<2)
        {
            res.append(word);
            res.append(L" ");
            continue;
        }

        SwitchDhaViramaYa_U_UU_R(&word);
        word = processOriyaLigatures(word);
        ChangeOriyaRaViramaLigature(&word);
        SwitchOriyaNaVirama(&word);
        SwitchOriyaRaVirama(&word);
        SwitchOriyaI(&word);
        SwitchOriyaE(&word);
        SwitchOriyaAI(&word);
        SwitchOriyaO(&word);
        SwitchOriyaAU(&word);
        SwitchOriyaNV_BLWF(&word);
        StripZWNJ_oriya(&word);

        res.append(word);
        res.append(L" ");
    }
    res.substr(0,res.size()-1);
    return res;
}

lString16 restoreOriyaWord(lString16 in)
{
    if(ORIYA_DISPLAY_ENABLE == 0 || gDocumentOriya == 0)
    {
        return in;
    }
    StripZWNJ_oriya_reverse(&in);
    SwitchOriyaNV_BLWF_reverse(&in);
    SwitchOriyaAU_reverse(&in);
    SwitchOriyaO_reverse(&in);
    SwitchOriyaAI_reverse(&in);
    SwitchOriyaE_reverse(&in);
    SwitchOriyaI_reverse(&in);
    SwitchOriyaRaVirama_reverse(&in);
    for (int i = 0; i < in.length(); i++)
    {
        if (in[i] < ORIYA_START || in[i] > ORIYA_END)
        {
            continue;
        }

        dvngLig lig = findOriyaLig(in[i]);
        if (lig.isNull() || lig.len == 0 || lig.len > 10)
        {
            continue;
        }
        //LE("lig for [in[%d] [0x%X]] = 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X ",i, in[i], lig.a,lig.b,lig.c,lig.d,lig.e,lig.f,lig.g,lig.h,lig.i,lig.j);
        lString16 rep = lig.restoreToChars();
        lString16 firsthalf = in.substr(0,i);
        lString16 lasthalf  = in.substr(i+1,in.length()-i);
        in = firsthalf + rep + lasthalf;
    }
    SwitchDhaViramaYa_U_UU_R_reverse(&in);
    return in;
}
