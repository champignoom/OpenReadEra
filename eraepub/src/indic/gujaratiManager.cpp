//
// Created by Tarasus on 16.10.2020.
//

#include "include/indic/gujaratiManager.h"
#include "ore_log.h"
#include <vector>

LigMap     gGujaratiLigMap;
LigMapRev  gGujaratiLigMapRev;
FastLigMap gGujaratiFastLigMap;

bool CharIsGujarati(int ch)
{
    return (ch >= 0x0A80 && ch <= 0x0AFF);
}

bool lString16::CheckGujarati()
{
    if(gDocumentGujarati == 1)
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
        if (CharIsGujarati(ch))
        {
            gDocumentGujarati = 1;
            gDocumentINDIC = 1;

            if(gGujaratiLigMapRev.empty())
            {
                gGujaratiLigMapRev = GujaratiLigMapReversed();
            }
            return true;
        }
    }
    return false;
}

dvngLig findGujaratiLig(lChar16 ligature)
{
    if(gGujaratiLigMap.empty())
    {
        gGujaratiLigMap = GetGujaratiLigMap();
    }
    auto it = gGujaratiLigMap.find(ligature);
    if(it==gGujaratiLigMap.end())
    {
        return dvngLig();
    }
    return it->second;
}

lChar16 findGujaratiLigGlyphIndex(lChar16 ligature)
{
    auto it = gGujaratiLigMap.find(ligature);
    if(it==gGujaratiLigMap.end())
    {
        return 0;
    }
    return it->second.glyphindex;
}

lChar16 findGujaratiLigRev(dvngLig combo)
{
    if(gGujaratiLigMapRev.empty())
    {
        gGujaratiLigMapRev = GujaratiLigMapReversed();
    }
    if(combo.len < 2 || combo.len > 10 )
    {
        return 0;
    }
    auto it = gGujaratiLigMapRev.find(combo);
    if(it==gGujaratiLigMapRev.end())
    {
        return 0;
    }
    //LE("findGujaratiLigRev return %d", it->second);
    return it->second;
}

std::map <dvngLig, lChar16, Comparator> GujaratiLigMapReversed()
{
    if(!gGujaratiLigMapRev.empty())
    {
        return gGujaratiLigMapRev;
    }
    if(gGujaratiLigMap.empty())
    {
        gGujaratiLigMap = GetGujaratiLigMap();
    }
    gGujaratiLigMapRev = makeReverseLigMap(gGujaratiLigMap,&gGujaratiFastLigMap);
    return gGujaratiLigMapRev;
}

LigMap GetGujaratiLigMap()
{
    if(!gGujaratiLigMap.empty())
    {
        return gGujaratiLigMap;
    }
    //GUJARATI_START
    {
        //from Noto Sans Gujarati.ttf

        gGujaratiLigMap.insert(std::make_pair( 0xEA00, dvngLig( 92 ,"0x0A95 0x0ABC"))); // 0x10001
        gGujaratiLigMap.insert(std::make_pair( 0xEA01, dvngLig( 93 ,"0x0A96 0x0ABC"))); // 0x10002
        gGujaratiLigMap.insert(std::make_pair( 0xEA02, dvngLig( 94 ,"0x0A97 0x0ABC"))); // 0x10003
        gGujaratiLigMap.insert(std::make_pair( 0xEA03, dvngLig( 95 ,"0x0A98 0x0ABC"))); // 0x10004
        gGujaratiLigMap.insert(std::make_pair( 0xEA04, dvngLig( 96 ,"0x0A99 0x0ABC"))); // 0x10005
        gGujaratiLigMap.insert(std::make_pair( 0xEA05, dvngLig( 97 ,"0x0A9A 0x0ABC"))); // 0x10006
        gGujaratiLigMap.insert(std::make_pair( 0xEA06, dvngLig( 98 ,"0x0A9B 0x0ABC"))); // 0x10007
        gGujaratiLigMap.insert(std::make_pair( 0xEA07, dvngLig( 99 ,"0x0A9C 0x0ABC"))); // 0x10008
        gGujaratiLigMap.insert(std::make_pair( 0xEA08, dvngLig( 100 ,"0x0A9D 0x0ABC"))); // 0x10009
        gGujaratiLigMap.insert(std::make_pair( 0xEA09, dvngLig( 101 ,"0x0A9E 0x0ABC"))); // 0x1000A
        gGujaratiLigMap.insert(std::make_pair( 0xEA0A, dvngLig( 102 ,"0x0A9F 0x0ABC"))); // 0x1000B
        gGujaratiLigMap.insert(std::make_pair( 0xEA0B, dvngLig( 103 ,"0x0AA0 0x0ABC"))); // 0x1000C
        gGujaratiLigMap.insert(std::make_pair( 0xEA0C, dvngLig( 104 ,"0x0AA1 0x0ABC"))); // 0x1000D
        gGujaratiLigMap.insert(std::make_pair( 0xEA0D, dvngLig( 105 ,"0x0AA2 0x0ABC"))); // 0x1000E
        gGujaratiLigMap.insert(std::make_pair( 0xEA0E, dvngLig( 106 ,"0x0AA3 0x0ABC"))); // 0x1000F
        gGujaratiLigMap.insert(std::make_pair( 0xEA0F, dvngLig( 107 ,"0x0AA4 0x0ABC"))); // 0x10010
        gGujaratiLigMap.insert(std::make_pair( 0xEA10, dvngLig( 108 ,"0x0AA5 0x0ABC"))); // 0x10011
        gGujaratiLigMap.insert(std::make_pair( 0xEA11, dvngLig( 109 ,"0x0AA6 0x0ABC"))); // 0x10012
        gGujaratiLigMap.insert(std::make_pair( 0xEA12, dvngLig( 110 ,"0x0AA7 0x0ABC"))); // 0x10013
        gGujaratiLigMap.insert(std::make_pair( 0xEA13, dvngLig( 111 ,"0x0AA8 0x0ABC"))); // 0x10014
        gGujaratiLigMap.insert(std::make_pair( 0xEA14, dvngLig( 112 ,"0x0AAA 0x0ABC"))); // 0x10015
        gGujaratiLigMap.insert(std::make_pair( 0xEA15, dvngLig( 113 ,"0x0AAB 0x0ABC"))); // 0x10016
        gGujaratiLigMap.insert(std::make_pair( 0xEA16, dvngLig( 114 ,"0x0AAC 0x0ABC"))); // 0x10017
        gGujaratiLigMap.insert(std::make_pair( 0xEA17, dvngLig( 115 ,"0x0AAD 0x0ABC"))); // 0x10018
        gGujaratiLigMap.insert(std::make_pair( 0xEA18, dvngLig( 116 ,"0x0AAE 0x0ABC"))); // 0x10019
        gGujaratiLigMap.insert(std::make_pair( 0xEA19, dvngLig( 117 ,"0x0AAF 0x0ABC"))); // 0x1001A
        gGujaratiLigMap.insert(std::make_pair( 0xEA1A, dvngLig( 118 ,"0x0AB0 0x0ABC"))); // 0x1001B
        gGujaratiLigMap.insert(std::make_pair( 0xEA1B, dvngLig( 119 ,"0x0AB2 0x0ABC"))); // 0x1001C
        gGujaratiLigMap.insert(std::make_pair( 0xEA1C, dvngLig( 120 ,"0x0AB3 0x0ABC"))); // 0x1001D
        gGujaratiLigMap.insert(std::make_pair( 0xEA1D, dvngLig( 121 ,"0x0AB5 0x0ABC"))); // 0x1001E
        gGujaratiLigMap.insert(std::make_pair( 0xEA1E, dvngLig( 122 ,"0x0AB6 0x0ABC"))); // 0x1001F
        gGujaratiLigMap.insert(std::make_pair( 0xEA1F, dvngLig( 123 ,"0x0AB7 0x0ABC"))); // 0x10020
        gGujaratiLigMap.insert(std::make_pair( 0xEA20, dvngLig( 124 ,"0x0AB8 0x0ABC"))); // 0x10021
        gGujaratiLigMap.insert(std::make_pair( 0xEA21, dvngLig( 125 ,"0x0AB9 0x0ABC"))); // 0x10022
        gGujaratiLigMap.insert(std::make_pair( 0xEA22, dvngLig( 126 ,"0x0A95 0x0ACD 0x0AB7"))); // 0x10023
        gGujaratiLigMap.insert(std::make_pair( 0xEA23, dvngLig( 127 ,"0x0A9C 0x0ACD 0x0A9E"))); // 0x10024
        gGujaratiLigMap.insert(std::make_pair( 0xEA24, dvngLig( 128 ,"0x0AB0 0x0ACD"))); // 0x10025    //Ra Virama
        //gGujaratiLigMap.insert(std::make_pair( 0xEA25, dvngLig( 129 ,"0x0ACD 0x0AB0"))); // 0x10026
        //gGujaratiLigMap.insert(std::make_pair( 0xEA26, dvngLig( 129 ,"0x0AB0 0x0ACD"))); // 0x10026
        gGujaratiLigMap.insert(std::make_pair( 0xEA27, dvngLig( 130 ,"0x0A95 0x0ACD"))); // 0x10027
        gGujaratiLigMap.insert(std::make_pair( 0xEA28, dvngLig( 131 ,"0x0A96 0x0ACD"))); // 0x10028
        gGujaratiLigMap.insert(std::make_pair( 0xEA29, dvngLig( 132 ,"0x0A97 0x0ACD"))); // 0x10029
        gGujaratiLigMap.insert(std::make_pair( 0xEA2A, dvngLig( 133 ,"0x0A98 0x0ACD"))); // 0x1002A
        gGujaratiLigMap.insert(std::make_pair( 0xEA2B, dvngLig( 134 ,"0x0A99 0x0ACD"))); // 0x1002B
        gGujaratiLigMap.insert(std::make_pair( 0xEA2C, dvngLig( 135 ,"0x0A9A 0x0ACD"))); // 0x1002C
        gGujaratiLigMap.insert(std::make_pair( 0xEA2D, dvngLig( 136 ,"0x0A9B 0x0ACD"))); // 0x1002D
        gGujaratiLigMap.insert(std::make_pair( 0xEA2E, dvngLig( 137 ,"0x0A9C 0x0ACD"))); // 0x1002E
        gGujaratiLigMap.insert(std::make_pair( 0xEA2F, dvngLig( 138 ,"0x0A9D 0x0ACD"))); // 0x1002F
        gGujaratiLigMap.insert(std::make_pair( 0xEA30, dvngLig( 139 ,"0x0A9E 0x0ACD"))); // 0x10030
        gGujaratiLigMap.insert(std::make_pair( 0xEA31, dvngLig( 140 ,"0x0A9F 0x0ACD"))); // 0x10031
        gGujaratiLigMap.insert(std::make_pair( 0xEA32, dvngLig( 141 ,"0x0AA0 0x0ACD"))); // 0x10032
        gGujaratiLigMap.insert(std::make_pair( 0xEA33, dvngLig( 142 ,"0x0AA1 0x0ACD"))); // 0x10033
        gGujaratiLigMap.insert(std::make_pair( 0xEA34, dvngLig( 143 ,"0x0AA2 0x0ACD"))); // 0x10034
        gGujaratiLigMap.insert(std::make_pair( 0xEA35, dvngLig( 144 ,"0x0AA3 0x0ACD"))); // 0x10035
        gGujaratiLigMap.insert(std::make_pair( 0xEA36, dvngLig( 145 ,"0x0AA4 0x0ACD"))); // 0x10036
        gGujaratiLigMap.insert(std::make_pair( 0xEA37, dvngLig( 146 ,"0x0AA5 0x0ACD"))); // 0x10037
        gGujaratiLigMap.insert(std::make_pair( 0xEA38, dvngLig( 147 ,"0x0AA6 0x0ACD"))); // 0x10038
        gGujaratiLigMap.insert(std::make_pair( 0xEA39, dvngLig( 148 ,"0x0AA7 0x0ACD"))); // 0x10039
        gGujaratiLigMap.insert(std::make_pair( 0xEA3A, dvngLig( 149 ,"0x0AA8 0x0ACD"))); // 0x1003A
        gGujaratiLigMap.insert(std::make_pair( 0xEA3B, dvngLig( 150 ,"0x0AAA 0x0ACD"))); // 0x1003B
        gGujaratiLigMap.insert(std::make_pair( 0xEA3C, dvngLig( 151 ,"0x0AAB 0x0ACD"))); // 0x1003C
        gGujaratiLigMap.insert(std::make_pair( 0xEA3D, dvngLig( 152 ,"0x0AAC 0x0ACD"))); // 0x1003D
        gGujaratiLigMap.insert(std::make_pair( 0xEA3E, dvngLig( 153 ,"0x0AAD 0x0ACD"))); // 0x1003E
        gGujaratiLigMap.insert(std::make_pair( 0xEA3F, dvngLig( 154 ,"0x0AAE 0x0ACD"))); // 0x1003F
        gGujaratiLigMap.insert(std::make_pair( 0xEA40, dvngLig( 155 ,"0x0AAF 0x0ACD"))); // 0x10040
        //gGujaratiLigMap.insert(std::make_pair( 0xEA41, dvngLig( 156 ,"0x0AB0 0x0ACD"))); // 0x10041
        gGujaratiLigMap.insert(std::make_pair( 0xEA42, dvngLig( 157 ,"0x0AB2 0x0ACD"))); // 0x10042
        gGujaratiLigMap.insert(std::make_pair( 0xEA43, dvngLig( 158 ,"0x0AB3 0x0ACD"))); // 0x10043
        gGujaratiLigMap.insert(std::make_pair( 0xEA44, dvngLig( 159 ,"0x0AB5 0x0ACD"))); // 0x10044
        gGujaratiLigMap.insert(std::make_pair( 0xEA45, dvngLig( 160 ,"0x0AB6 0x0ACD"))); // 0x10045
        gGujaratiLigMap.insert(std::make_pair( 0xEA46, dvngLig( 161 ,"0x0AB7 0x0ACD"))); // 0x10046
        gGujaratiLigMap.insert(std::make_pair( 0xEA47, dvngLig( 162 ,"0x0AB8 0x0ACD"))); // 0x10047
        gGujaratiLigMap.insert(std::make_pair( 0xEA48, dvngLig( 163 ,"0x0AB9 0x0ACD"))); // 0x10048
        gGujaratiLigMap.insert(std::make_pair( 0xEA49, dvngLig( 164 ,"0x0A95 0x0ACD 0x0AB7 0x0ACD"))); // 0x10049
        gGujaratiLigMap.insert(std::make_pair( 0xEA4A, dvngLig( 165 ,"0x0A9C 0x0ACD 0x0A9E 0x0ACD"))); // 0x1004A
        gGujaratiLigMap.insert(std::make_pair( 0xEA4B, dvngLig( 166 ,"0x0A95 0x0ABC 0x0ACD"))); // 0x1004B
        gGujaratiLigMap.insert(std::make_pair( 0xEA4C, dvngLig( 167 ,"0x0A96 0x0ABC 0x0ACD"))); // 0x1004C
        gGujaratiLigMap.insert(std::make_pair( 0xEA4D, dvngLig( 168 ,"0x0A97 0x0ABC 0x0ACD"))); // 0x1004D
        gGujaratiLigMap.insert(std::make_pair( 0xEA4E, dvngLig( 169 ,"0x0A98 0x0ABC 0x0ACD"))); // 0x1004E
        gGujaratiLigMap.insert(std::make_pair( 0xEA4F, dvngLig( 170 ,"0x0A99 0x0ABC 0x0ACD"))); // 0x1004F
        gGujaratiLigMap.insert(std::make_pair( 0xEA50, dvngLig( 171 ,"0x0A9A 0x0ABC 0x0ACD"))); // 0x10050
        gGujaratiLigMap.insert(std::make_pair( 0xEA51, dvngLig( 172 ,"0x0A9B 0x0ABC 0x0ACD"))); // 0x10051
        gGujaratiLigMap.insert(std::make_pair( 0xEA52, dvngLig( 173 ,"0x0A9C 0x0ABC 0x0ACD"))); // 0x10052
        gGujaratiLigMap.insert(std::make_pair( 0xEA53, dvngLig( 174 ,"0x0A9D 0x0ABC 0x0ACD"))); // 0x10053
        gGujaratiLigMap.insert(std::make_pair( 0xEA54, dvngLig( 175 ,"0x0A9E 0x0ABC 0x0ACD"))); // 0x10054
        gGujaratiLigMap.insert(std::make_pair( 0xEA55, dvngLig( 176 ,"0x0A9F 0x0ABC 0x0ACD"))); // 0x10055
        gGujaratiLigMap.insert(std::make_pair( 0xEA56, dvngLig( 177 ,"0x0AA0 0x0ABC 0x0ACD"))); // 0x10056
        gGujaratiLigMap.insert(std::make_pair( 0xEA57, dvngLig( 178 ,"0x0AA1 0x0ABC 0x0ACD"))); // 0x10057
        gGujaratiLigMap.insert(std::make_pair( 0xEA58, dvngLig( 179 ,"0x0AA2 0x0ABC 0x0ACD"))); // 0x10058
        gGujaratiLigMap.insert(std::make_pair( 0xEA59, dvngLig( 180 ,"0x0AA3 0x0ABC 0x0ACD"))); // 0x10059
        gGujaratiLigMap.insert(std::make_pair( 0xEA5A, dvngLig( 181 ,"0x0AA4 0x0ABC 0x0ACD"))); // 0x1005A
        gGujaratiLigMap.insert(std::make_pair( 0xEA5B, dvngLig( 182 ,"0x0AA5 0x0ABC 0x0ACD"))); // 0x1005B
        gGujaratiLigMap.insert(std::make_pair( 0xEA5C, dvngLig( 183 ,"0x0AA6 0x0ABC 0x0ACD"))); // 0x1005C
        gGujaratiLigMap.insert(std::make_pair( 0xEA5D, dvngLig( 184 ,"0x0AA7 0x0ABC 0x0ACD"))); // 0x1005D
        gGujaratiLigMap.insert(std::make_pair( 0xEA5E, dvngLig( 185 ,"0x0AA8 0x0ABC 0x0ACD"))); // 0x1005E
        gGujaratiLigMap.insert(std::make_pair( 0xEA5F, dvngLig( 186 ,"0x0AAA 0x0ABC 0x0ACD"))); // 0x1005F
        gGujaratiLigMap.insert(std::make_pair( 0xEA60, dvngLig( 187 ,"0x0AAB 0x0ABC 0x0ACD"))); // 0x10060
        gGujaratiLigMap.insert(std::make_pair( 0xEA61, dvngLig( 188 ,"0x0AAC 0x0ABC 0x0ACD"))); // 0x10061
        gGujaratiLigMap.insert(std::make_pair( 0xEA62, dvngLig( 189 ,"0x0AAD 0x0ABC 0x0ACD"))); // 0x10062
        gGujaratiLigMap.insert(std::make_pair( 0xEA63, dvngLig( 190 ,"0x0AAE 0x0ABC 0x0ACD"))); // 0x10063
        gGujaratiLigMap.insert(std::make_pair( 0xEA64, dvngLig( 191 ,"0x0AAF 0x0ABC 0x0ACD"))); // 0x10064
        gGujaratiLigMap.insert(std::make_pair( 0xEA65, dvngLig( 192 ,"0x0AB0 0x0ABC 0x0ACD"))); // 0x10065
        gGujaratiLigMap.insert(std::make_pair( 0xEA66, dvngLig( 193 ,"0x0AB2 0x0ABC 0x0ACD"))); // 0x10066
        gGujaratiLigMap.insert(std::make_pair( 0xEA67, dvngLig( 194 ,"0x0AB3 0x0ABC 0x0ACD"))); // 0x10067
        gGujaratiLigMap.insert(std::make_pair( 0xEA68, dvngLig( 195 ,"0x0AB5 0x0ABC 0x0ACD"))); // 0x10068
        gGujaratiLigMap.insert(std::make_pair( 0xEA69, dvngLig( 196 ,"0x0AB6 0x0ABC 0x0ACD"))); // 0x10069
        gGujaratiLigMap.insert(std::make_pair( 0xEA6A, dvngLig( 197 ,"0x0AB7 0x0ABC 0x0ACD"))); // 0x1006A
        gGujaratiLigMap.insert(std::make_pair( 0xEA6B, dvngLig( 198 ,"0x0AB8 0x0ABC 0x0ACD"))); // 0x1006B
        gGujaratiLigMap.insert(std::make_pair( 0xEA6C, dvngLig( 199 ,"0x0AB9 0x0ABC 0x0ACD"))); // 0x1006C
        gGujaratiLigMap.insert(std::make_pair( 0xEA6D, dvngLig( 200 ,"0x0A95 0x0ACD 0x0AB0"))); // 0x1006D
        gGujaratiLigMap.insert(std::make_pair( 0xEA6E, dvngLig( 200 ,"0x0A95 0x0AB0 0x0ACD"))); // 0x1006D
        gGujaratiLigMap.insert(std::make_pair( 0xEA6F, dvngLig( 201 ,"0x0A96 0x0ACD 0x0AB0"))); // 0x1006E
        gGujaratiLigMap.insert(std::make_pair( 0xEA70, dvngLig( 201 ,"0x0A96 0x0AB0 0x0ACD"))); // 0x1006E
        gGujaratiLigMap.insert(std::make_pair( 0xEA71, dvngLig( 202 ,"0x0A97 0x0ACD 0x0AB0"))); // 0x1006F
        gGujaratiLigMap.insert(std::make_pair( 0xEA72, dvngLig( 202 ,"0x0A97 0x0AB0 0x0ACD"))); // 0x1006F
        gGujaratiLigMap.insert(std::make_pair( 0xEA73, dvngLig( 203 ,"0x0A98 0x0ACD 0x0AB0"))); // 0x10070
        gGujaratiLigMap.insert(std::make_pair( 0xEA74, dvngLig( 203 ,"0x0A98 0x0AB0 0x0ACD"))); // 0x10070
        gGujaratiLigMap.insert(std::make_pair( 0xEA75, dvngLig( 204 ,"0x0A99 0x0ACD 0x0AB0"))); // 0x10071
        gGujaratiLigMap.insert(std::make_pair( 0xEA76, dvngLig( 204 ,"0x0A99 0x0AB0 0x0ACD"))); // 0x10071
        gGujaratiLigMap.insert(std::make_pair( 0xEA77, dvngLig( 205 ,"0x0A9A 0x0ACD 0x0AB0"))); // 0x10072
        gGujaratiLigMap.insert(std::make_pair( 0xEA78, dvngLig( 205 ,"0x0A9A 0x0AB0 0x0ACD"))); // 0x10072
        gGujaratiLigMap.insert(std::make_pair( 0xEA79, dvngLig( 206 ,"0x0A9B 0x0ACD 0x0AB0"))); // 0x10073
        gGujaratiLigMap.insert(std::make_pair( 0xEA7A, dvngLig( 206 ,"0x0A9B 0x0AB0 0x0ACD"))); // 0x10073
        gGujaratiLigMap.insert(std::make_pair( 0xEA7B, dvngLig( 207 ,"0x0A9C 0x0ACD 0x0AB0"))); // 0x10074
        gGujaratiLigMap.insert(std::make_pair( 0xEA7C, dvngLig( 207 ,"0x0A9C 0x0AB0 0x0ACD"))); // 0x10074
        gGujaratiLigMap.insert(std::make_pair( 0xEA7D, dvngLig( 208 ,"0x0A9D 0x0ACD 0x0AB0"))); // 0x10075
        gGujaratiLigMap.insert(std::make_pair( 0xEA7E, dvngLig( 208 ,"0x0A9D 0x0AB0 0x0ACD"))); // 0x10075
        gGujaratiLigMap.insert(std::make_pair( 0xEA7F, dvngLig( 209 ,"0x0A9E 0x0ACD 0x0AB0"))); // 0x10076
        gGujaratiLigMap.insert(std::make_pair( 0xEA80, dvngLig( 209 ,"0x0A9E 0x0AB0 0x0ACD"))); // 0x10076
        gGujaratiLigMap.insert(std::make_pair( 0xEA81, dvngLig( 210 ,"0x0A9F 0x0ACD 0x0AB0"))); // 0x10077
        gGujaratiLigMap.insert(std::make_pair( 0xEA82, dvngLig( 210 ,"0x0A9F 0x0AB0 0x0ACD"))); // 0x10077
        gGujaratiLigMap.insert(std::make_pair( 0xEA83, dvngLig( 211 ,"0x0AA0 0x0ACD 0x0AB0"))); // 0x10078
        gGujaratiLigMap.insert(std::make_pair( 0xEA84, dvngLig( 211 ,"0x0AA0 0x0AB0 0x0ACD"))); // 0x10078
        gGujaratiLigMap.insert(std::make_pair( 0xEA85, dvngLig( 212 ,"0x0AA1 0x0ACD 0x0AB0"))); // 0x10079
        gGujaratiLigMap.insert(std::make_pair( 0xEA86, dvngLig( 212 ,"0x0AA1 0x0AB0 0x0ACD"))); // 0x10079
        gGujaratiLigMap.insert(std::make_pair( 0xEA87, dvngLig( 213 ,"0x0AA2 0x0ACD 0x0AB0"))); // 0x1007A
        gGujaratiLigMap.insert(std::make_pair( 0xEA88, dvngLig( 213 ,"0x0AA2 0x0AB0 0x0ACD"))); // 0x1007A
        gGujaratiLigMap.insert(std::make_pair( 0xEA89, dvngLig( 214 ,"0x0AA3 0x0ACD 0x0AB0"))); // 0x1007B
        gGujaratiLigMap.insert(std::make_pair( 0xEA8A, dvngLig( 214 ,"0x0AA3 0x0AB0 0x0ACD"))); // 0x1007B
        gGujaratiLigMap.insert(std::make_pair( 0xEA8B, dvngLig( 215 ,"0x0AA4 0x0ACD 0x0AB0"))); // 0x1007C
        gGujaratiLigMap.insert(std::make_pair( 0xEA8C, dvngLig( 215 ,"0x0AA4 0x0AB0 0x0ACD"))); // 0x1007C
        gGujaratiLigMap.insert(std::make_pair( 0xEA8D, dvngLig( 216 ,"0x0AA5 0x0ACD 0x0AB0"))); // 0x1007D
        gGujaratiLigMap.insert(std::make_pair( 0xEA8E, dvngLig( 216 ,"0x0AA5 0x0AB0 0x0ACD"))); // 0x1007D
        gGujaratiLigMap.insert(std::make_pair( 0xEA8F, dvngLig( 217 ,"0x0AA6 0x0ACD 0x0AB0"))); // 0x1007E
        gGujaratiLigMap.insert(std::make_pair( 0xEA90, dvngLig( 217 ,"0x0AA6 0x0AB0 0x0ACD"))); // 0x1007E
        gGujaratiLigMap.insert(std::make_pair( 0xEA91, dvngLig( 218 ,"0x0AA7 0x0ACD 0x0AB0"))); // 0x1007F
        gGujaratiLigMap.insert(std::make_pair( 0xEA92, dvngLig( 218 ,"0x0AA7 0x0AB0 0x0ACD"))); // 0x1007F
        gGujaratiLigMap.insert(std::make_pair( 0xEA93, dvngLig( 219 ,"0x0AA8 0x0ACD 0x0AB0"))); // 0x10080
        gGujaratiLigMap.insert(std::make_pair( 0xEA94, dvngLig( 219 ,"0x0AA8 0x0AB0 0x0ACD"))); // 0x10080
        gGujaratiLigMap.insert(std::make_pair( 0xEA95, dvngLig( 220 ,"0x0AAA 0x0ACD 0x0AB0"))); // 0x10081
        gGujaratiLigMap.insert(std::make_pair( 0xEA96, dvngLig( 220 ,"0x0AAA 0x0AB0 0x0ACD"))); // 0x10081
        gGujaratiLigMap.insert(std::make_pair( 0xEA97, dvngLig( 221 ,"0x0AAB 0x0ACD 0x0AB0"))); // 0x10082
        gGujaratiLigMap.insert(std::make_pair( 0xEA98, dvngLig( 221 ,"0x0AAB 0x0AB0 0x0ACD"))); // 0x10082
        gGujaratiLigMap.insert(std::make_pair( 0xEA99, dvngLig( 222 ,"0x0AAC 0x0ACD 0x0AB0"))); // 0x10083
        gGujaratiLigMap.insert(std::make_pair( 0xEA9A, dvngLig( 222 ,"0x0AAC 0x0AB0 0x0ACD"))); // 0x10083
        gGujaratiLigMap.insert(std::make_pair( 0xEA9B, dvngLig( 223 ,"0x0AAD 0x0ACD 0x0AB0"))); // 0x10084
        gGujaratiLigMap.insert(std::make_pair( 0xEA9C, dvngLig( 223 ,"0x0AAD 0x0AB0 0x0ACD"))); // 0x10084
        gGujaratiLigMap.insert(std::make_pair( 0xEA9D, dvngLig( 224 ,"0x0AAE 0x0ACD 0x0AB0"))); // 0x10085
        gGujaratiLigMap.insert(std::make_pair( 0xEA9E, dvngLig( 224 ,"0x0AAE 0x0AB0 0x0ACD"))); // 0x10085
        gGujaratiLigMap.insert(std::make_pair( 0xEA9F, dvngLig( 225 ,"0x0AAF 0x0ACD 0x0AB0"))); // 0x10086
        gGujaratiLigMap.insert(std::make_pair( 0xEAA0, dvngLig( 225 ,"0x0AAF 0x0AB0 0x0ACD"))); // 0x10086
        gGujaratiLigMap.insert(std::make_pair( 0xEAA1, dvngLig( 226 ,"0x0AB0 0x0ACD 0x0AB0"))); // 0x10087
        gGujaratiLigMap.insert(std::make_pair( 0xEAA2, dvngLig( 226 ,"0x0AB0 0x0AB0 0x0ACD"))); // 0x10087
        gGujaratiLigMap.insert(std::make_pair( 0xEAA3, dvngLig( 227 ,"0x0AB2 0x0ACD 0x0AB0"))); // 0x10088
        gGujaratiLigMap.insert(std::make_pair( 0xEAA4, dvngLig( 227 ,"0x0AB2 0x0AB0 0x0ACD"))); // 0x10088
        gGujaratiLigMap.insert(std::make_pair( 0xEAA5, dvngLig( 228 ,"0x0AB3 0x0ACD 0x0AB0"))); // 0x10089
        gGujaratiLigMap.insert(std::make_pair( 0xEAA6, dvngLig( 228 ,"0x0AB3 0x0AB0 0x0ACD"))); // 0x10089
        gGujaratiLigMap.insert(std::make_pair( 0xEAA7, dvngLig( 229 ,"0x0AB5 0x0ACD 0x0AB0"))); // 0x1008A
        //gGujaratiLigMap.insert(std::make_pair( 0xEAA8, dvngLig( 229 ,"0x0AB5 0x0AB0 0x0ACD"))); // 0x1008A
        gGujaratiLigMap.insert(std::make_pair( 0xEAA9, dvngLig( 230 ,"0x0AB6 0x0ACD 0x0AB0"))); // 0x1008B
        gGujaratiLigMap.insert(std::make_pair( 0xEAAA, dvngLig( 230 ,"0x0AB6 0x0AB0 0x0ACD"))); // 0x1008B
        gGujaratiLigMap.insert(std::make_pair( 0xEAAB, dvngLig( 231 ,"0x0AB7 0x0ACD 0x0AB0"))); // 0x1008C
        gGujaratiLigMap.insert(std::make_pair( 0xEAAC, dvngLig( 231 ,"0x0AB7 0x0AB0 0x0ACD"))); // 0x1008C
        gGujaratiLigMap.insert(std::make_pair( 0xEAAD, dvngLig( 232 ,"0x0AB8 0x0ACD 0x0AB0"))); // 0x1008D
        gGujaratiLigMap.insert(std::make_pair( 0xEAAE, dvngLig( 232 ,"0x0AB8 0x0AB0 0x0ACD"))); // 0x1008D
        gGujaratiLigMap.insert(std::make_pair( 0xEAAF, dvngLig( 233 ,"0x0AB9 0x0ACD 0x0AB0"))); // 0x1008E
        gGujaratiLigMap.insert(std::make_pair( 0xEAB0, dvngLig( 233 ,"0x0AB9 0x0AB0 0x0ACD"))); // 0x1008E
        gGujaratiLigMap.insert(std::make_pair( 0xEAB1, dvngLig( 234 ,"0x0A95 0x0ACD 0x0AB7 0x0ACD 0x0AB0"))); // 0x1008F
        gGujaratiLigMap.insert(std::make_pair( 0xEAB2, dvngLig( 234 ,"0x0A95 0x0ACD 0x0AB7 0x0AB0 0x0ACD"))); // 0x1008F
        gGujaratiLigMap.insert(std::make_pair( 0xEAB3, dvngLig( 235 ,"0x0A9C 0x0ACD 0x0A9E 0x0ACD 0x0AB0"))); // 0x10090
        gGujaratiLigMap.insert(std::make_pair( 0xEAB4, dvngLig( 235 ,"0x0A9C 0x0ACD 0x0A9E 0x0AB0 0x0ACD"))); // 0x10090
        gGujaratiLigMap.insert(std::make_pair( 0xEAB5, dvngLig( 236 ,"0x0A95 0x0ABC 0x0ACD 0x0AB0"))); // 0x10091
        gGujaratiLigMap.insert(std::make_pair( 0xEAB6, dvngLig( 236 ,"0x0A95 0x0ABC 0x0AB0 0x0ACD"))); // 0x10091
        gGujaratiLigMap.insert(std::make_pair( 0xEAB7, dvngLig( 237 ,"0x0A96 0x0ABC 0x0ACD 0x0AB0"))); // 0x10092
        gGujaratiLigMap.insert(std::make_pair( 0xEAB8, dvngLig( 237 ,"0x0A96 0x0ABC 0x0AB0 0x0ACD"))); // 0x10092
        gGujaratiLigMap.insert(std::make_pair( 0xEAB9, dvngLig( 238 ,"0x0A97 0x0ABC 0x0ACD 0x0AB0"))); // 0x10093
        gGujaratiLigMap.insert(std::make_pair( 0xEABA, dvngLig( 238 ,"0x0A97 0x0ABC 0x0AB0 0x0ACD"))); // 0x10093
        gGujaratiLigMap.insert(std::make_pair( 0xEABB, dvngLig( 239 ,"0x0A98 0x0ABC 0x0ACD 0x0AB0"))); // 0x10094
        gGujaratiLigMap.insert(std::make_pair( 0xEABC, dvngLig( 239 ,"0x0A98 0x0ABC 0x0AB0 0x0ACD"))); // 0x10094
        gGujaratiLigMap.insert(std::make_pair( 0xEABD, dvngLig( 240 ,"0x0A99 0x0ABC 0x0ACD 0x0AB0"))); // 0x10095
        gGujaratiLigMap.insert(std::make_pair( 0xEABE, dvngLig( 240 ,"0x0A99 0x0ABC 0x0AB0 0x0ACD"))); // 0x10095
        gGujaratiLigMap.insert(std::make_pair( 0xEABF, dvngLig( 241 ,"0x0A9A 0x0ABC 0x0ACD 0x0AB0"))); // 0x10096
        gGujaratiLigMap.insert(std::make_pair( 0xEAC0, dvngLig( 241 ,"0x0A9A 0x0ABC 0x0AB0 0x0ACD"))); // 0x10096
        gGujaratiLigMap.insert(std::make_pair( 0xEAC1, dvngLig( 242 ,"0x0A9B 0x0ABC 0x0ACD 0x0AB0"))); // 0x10097
        gGujaratiLigMap.insert(std::make_pair( 0xEAC2, dvngLig( 242 ,"0x0A9B 0x0ABC 0x0AB0 0x0ACD"))); // 0x10097
        gGujaratiLigMap.insert(std::make_pair( 0xEAC3, dvngLig( 243 ,"0x0A9C 0x0ABC 0x0ACD 0x0AB0"))); // 0x10098
        gGujaratiLigMap.insert(std::make_pair( 0xEAC4, dvngLig( 243 ,"0x0A9C 0x0ABC 0x0AB0 0x0ACD"))); // 0x10098
        gGujaratiLigMap.insert(std::make_pair( 0xEAC5, dvngLig( 244 ,"0x0A9D 0x0ABC 0x0ACD 0x0AB0"))); // 0x10099
        gGujaratiLigMap.insert(std::make_pair( 0xEAC6, dvngLig( 244 ,"0x0A9D 0x0ABC 0x0AB0 0x0ACD"))); // 0x10099
        gGujaratiLigMap.insert(std::make_pair( 0xEAC7, dvngLig( 245 ,"0x0A9E 0x0ABC 0x0ACD 0x0AB0"))); // 0x1009A
        gGujaratiLigMap.insert(std::make_pair( 0xEAC8, dvngLig( 245 ,"0x0A9E 0x0ABC 0x0AB0 0x0ACD"))); // 0x1009A
        gGujaratiLigMap.insert(std::make_pair( 0xEAC9, dvngLig( 246 ,"0x0A9F 0x0ABC 0x0ACD 0x0AB0"))); // 0x1009B
        gGujaratiLigMap.insert(std::make_pair( 0xEACA, dvngLig( 246 ,"0x0A9F 0x0ABC 0x0AB0 0x0ACD"))); // 0x1009B
        gGujaratiLigMap.insert(std::make_pair( 0xEACB, dvngLig( 247 ,"0x0AA0 0x0ABC 0x0ACD 0x0AB0"))); // 0x1009C
        gGujaratiLigMap.insert(std::make_pair( 0xEACC, dvngLig( 247 ,"0x0AA0 0x0ABC 0x0AB0 0x0ACD"))); // 0x1009C
        gGujaratiLigMap.insert(std::make_pair( 0xEACD, dvngLig( 248 ,"0x0AA1 0x0ABC 0x0ACD 0x0AB0"))); // 0x1009D
        gGujaratiLigMap.insert(std::make_pair( 0xEACE, dvngLig( 248 ,"0x0AA1 0x0ABC 0x0AB0 0x0ACD"))); // 0x1009D
        gGujaratiLigMap.insert(std::make_pair( 0xEACF, dvngLig( 249 ,"0x0AA2 0x0ABC 0x0ACD 0x0AB0"))); // 0x1009E
        gGujaratiLigMap.insert(std::make_pair( 0xEAD0, dvngLig( 249 ,"0x0AA2 0x0ABC 0x0AB0 0x0ACD"))); // 0x1009E
        gGujaratiLigMap.insert(std::make_pair( 0xEAD1, dvngLig( 250 ,"0x0AA3 0x0ABC 0x0ACD 0x0AB0"))); // 0x1009F
        gGujaratiLigMap.insert(std::make_pair( 0xEAD2, dvngLig( 250 ,"0x0AA3 0x0ABC 0x0AB0 0x0ACD"))); // 0x1009F
        gGujaratiLigMap.insert(std::make_pair( 0xEAD3, dvngLig( 251 ,"0x0AA4 0x0ABC 0x0ACD 0x0AB0"))); // 0x100A0
        gGujaratiLigMap.insert(std::make_pair( 0xEAD4, dvngLig( 251 ,"0x0AA4 0x0ABC 0x0AB0 0x0ACD"))); // 0x100A0
        gGujaratiLigMap.insert(std::make_pair( 0xEAD5, dvngLig( 252 ,"0x0AA5 0x0ABC 0x0ACD 0x0AB0"))); // 0x100A1
        gGujaratiLigMap.insert(std::make_pair( 0xEAD6, dvngLig( 252 ,"0x0AA5 0x0ABC 0x0AB0 0x0ACD"))); // 0x100A1
        gGujaratiLigMap.insert(std::make_pair( 0xEAD7, dvngLig( 253 ,"0x0AA6 0x0ABC 0x0ACD 0x0AB0"))); // 0x100A2
        gGujaratiLigMap.insert(std::make_pair( 0xEAD8, dvngLig( 253 ,"0x0AA6 0x0ABC 0x0AB0 0x0ACD"))); // 0x100A2
        gGujaratiLigMap.insert(std::make_pair( 0xEAD9, dvngLig( 254 ,"0x0AA7 0x0ABC 0x0ACD 0x0AB0"))); // 0x100A3
        gGujaratiLigMap.insert(std::make_pair( 0xEADA, dvngLig( 254 ,"0x0AA7 0x0ABC 0x0AB0 0x0ACD"))); // 0x100A3
        gGujaratiLigMap.insert(std::make_pair( 0xEADB, dvngLig( 255 ,"0x0AA8 0x0ABC 0x0ACD 0x0AB0"))); // 0x100A4
        gGujaratiLigMap.insert(std::make_pair( 0xEADC, dvngLig( 255 ,"0x0AA8 0x0ABC 0x0AB0 0x0ACD"))); // 0x100A4
        gGujaratiLigMap.insert(std::make_pair( 0xEADD, dvngLig( 256 ,"0x0AAA 0x0ABC 0x0ACD 0x0AB0"))); // 0x100A5
        gGujaratiLigMap.insert(std::make_pair( 0xEADE, dvngLig( 256 ,"0x0AAA 0x0ABC 0x0AB0 0x0ACD"))); // 0x100A5
        gGujaratiLigMap.insert(std::make_pair( 0xEADF, dvngLig( 257 ,"0x0AAB 0x0ABC 0x0ACD 0x0AB0"))); // 0x100A6
        gGujaratiLigMap.insert(std::make_pair( 0xEAE0, dvngLig( 257 ,"0x0AAB 0x0ABC 0x0AB0 0x0ACD"))); // 0x100A6
        gGujaratiLigMap.insert(std::make_pair( 0xEAE1, dvngLig( 258 ,"0x0AAC 0x0ABC 0x0ACD 0x0AB0"))); // 0x100A7
        gGujaratiLigMap.insert(std::make_pair( 0xEAE2, dvngLig( 258 ,"0x0AAC 0x0ABC 0x0AB0 0x0ACD"))); // 0x100A7
        gGujaratiLigMap.insert(std::make_pair( 0xEAE3, dvngLig( 259 ,"0x0AAD 0x0ABC 0x0ACD 0x0AB0"))); // 0x100A8
        gGujaratiLigMap.insert(std::make_pair( 0xEAE4, dvngLig( 259 ,"0x0AAD 0x0ABC 0x0AB0 0x0ACD"))); // 0x100A8
        gGujaratiLigMap.insert(std::make_pair( 0xEAE5, dvngLig( 260 ,"0x0AAE 0x0ABC 0x0ACD 0x0AB0"))); // 0x100A9
        gGujaratiLigMap.insert(std::make_pair( 0xEAE6, dvngLig( 260 ,"0x0AAE 0x0ABC 0x0AB0 0x0ACD"))); // 0x100A9
        gGujaratiLigMap.insert(std::make_pair( 0xEAE7, dvngLig( 261 ,"0x0AAF 0x0ABC 0x0ACD 0x0AB0"))); // 0x100AA
        gGujaratiLigMap.insert(std::make_pair( 0xEAE8, dvngLig( 261 ,"0x0AAF 0x0ABC 0x0AB0 0x0ACD"))); // 0x100AA
        gGujaratiLigMap.insert(std::make_pair( 0xEAE9, dvngLig( 262 ,"0x0AB0 0x0ABC 0x0ACD 0x0AB0"))); // 0x100AB
        gGujaratiLigMap.insert(std::make_pair( 0xEAEA, dvngLig( 262 ,"0x0AB0 0x0ABC 0x0AB0 0x0ACD"))); // 0x100AB
        gGujaratiLigMap.insert(std::make_pair( 0xEAEB, dvngLig( 263 ,"0x0AB2 0x0ABC 0x0ACD 0x0AB0"))); // 0x100AC
        gGujaratiLigMap.insert(std::make_pair( 0xEAEC, dvngLig( 263 ,"0x0AB2 0x0ABC 0x0AB0 0x0ACD"))); // 0x100AC
        gGujaratiLigMap.insert(std::make_pair( 0xEAED, dvngLig( 264 ,"0x0AB3 0x0ABC 0x0ACD 0x0AB0"))); // 0x100AD
        gGujaratiLigMap.insert(std::make_pair( 0xEAEE, dvngLig( 264 ,"0x0AB3 0x0ABC 0x0AB0 0x0ACD"))); // 0x100AD
        gGujaratiLigMap.insert(std::make_pair( 0xEAEF, dvngLig( 265 ,"0x0AB5 0x0ABC 0x0ACD 0x0AB0"))); // 0x100AE
        gGujaratiLigMap.insert(std::make_pair( 0xEAF0, dvngLig( 265 ,"0x0AB5 0x0ABC 0x0AB0 0x0ACD"))); // 0x100AE
        gGujaratiLigMap.insert(std::make_pair( 0xEAF1, dvngLig( 266 ,"0x0AB6 0x0ABC 0x0ACD 0x0AB0"))); // 0x100AF
        gGujaratiLigMap.insert(std::make_pair( 0xEAF2, dvngLig( 266 ,"0x0AB6 0x0ABC 0x0AB0 0x0ACD"))); // 0x100AF
        gGujaratiLigMap.insert(std::make_pair( 0xEAF3, dvngLig( 267 ,"0x0AB7 0x0ABC 0x0ACD 0x0AB0"))); // 0x100B0
        gGujaratiLigMap.insert(std::make_pair( 0xEAF4, dvngLig( 267 ,"0x0AB7 0x0ABC 0x0AB0 0x0ACD"))); // 0x100B0
        gGujaratiLigMap.insert(std::make_pair( 0xEAF5, dvngLig( 268 ,"0x0AB8 0x0ABC 0x0ACD 0x0AB0"))); // 0x100B1
        gGujaratiLigMap.insert(std::make_pair( 0xEAF6, dvngLig( 268 ,"0x0AB8 0x0ABC 0x0AB0 0x0ACD"))); // 0x100B1
        gGujaratiLigMap.insert(std::make_pair( 0xEAF7, dvngLig( 269 ,"0x0AB9 0x0ABC 0x0ACD 0x0AB0"))); // 0x100B2
        gGujaratiLigMap.insert(std::make_pair( 0xEAF8, dvngLig( 269 ,"0x0AB9 0x0ABC 0x0AB0 0x0ACD"))); // 0x100B2
        gGujaratiLigMap.insert(std::make_pair( 0xEAF9, dvngLig( 270 ,"0x0A95 0x0ACD 0x0ACD 0x0AB0"))); // 0x100B3
        gGujaratiLigMap.insert(std::make_pair( 0xEAFA, dvngLig( 270 ,"0x0A95 0x0ACD 0x0AB0 0x0ACD"))); // 0x100B3
        gGujaratiLigMap.insert(std::make_pair( 0xEAFB, dvngLig( 270 ,"0x0A95 0x0AB0 0x0ACD 0x0ACD"))); // 0x100B3
        gGujaratiLigMap.insert(std::make_pair( 0xEAFC, dvngLig( 271 ,"0x0A96 0x0ACD 0x0ACD 0x0AB0"))); // 0x100B4
        gGujaratiLigMap.insert(std::make_pair( 0xEAFD, dvngLig( 271 ,"0x0A96 0x0ACD 0x0AB0 0x0ACD"))); // 0x100B4
        gGujaratiLigMap.insert(std::make_pair( 0xEAFE, dvngLig( 271 ,"0x0A96 0x0AB0 0x0ACD 0x0ACD"))); // 0x100B4
        gGujaratiLigMap.insert(std::make_pair( 0xEAFF, dvngLig( 272 ,"0x0A97 0x0ACD 0x0ACD 0x0AB0"))); // 0x100B5
        gGujaratiLigMap.insert(std::make_pair( 0xEB00, dvngLig( 272 ,"0x0A97 0x0ACD 0x0AB0 0x0ACD"))); // 0x100B5
        gGujaratiLigMap.insert(std::make_pair( 0xEB01, dvngLig( 272 ,"0x0A97 0x0AB0 0x0ACD 0x0ACD"))); // 0x100B5
        gGujaratiLigMap.insert(std::make_pair( 0xEB02, dvngLig( 273 ,"0x0A98 0x0ACD 0x0ACD 0x0AB0"))); // 0x100B6
        gGujaratiLigMap.insert(std::make_pair( 0xEB03, dvngLig( 273 ,"0x0A98 0x0ACD 0x0AB0 0x0ACD"))); // 0x100B6
        gGujaratiLigMap.insert(std::make_pair( 0xEB04, dvngLig( 273 ,"0x0A98 0x0AB0 0x0ACD 0x0ACD"))); // 0x100B6
        gGujaratiLigMap.insert(std::make_pair( 0xEB05, dvngLig( 274 ,"0x0A99 0x0ACD 0x0AB0 0x0ACD"))); // 0x100B7
        gGujaratiLigMap.insert(std::make_pair( 0xEB06, dvngLig( 274 ,"0x0A99 0x0AB0 0x0ACD 0x0ACD"))); // 0x100B7
        gGujaratiLigMap.insert(std::make_pair( 0xEB07, dvngLig( 274 ,"0x0A99 0x0ACD 0x0ACD 0x0AB0"))); // 0x100B7
        gGujaratiLigMap.insert(std::make_pair( 0xEB08, dvngLig( 275 ,"0x0A9A 0x0ACD 0x0ACD 0x0AB0"))); // 0x100B8
        gGujaratiLigMap.insert(std::make_pair( 0xEB09, dvngLig( 275 ,"0x0A9A 0x0ACD 0x0AB0 0x0ACD"))); // 0x100B8
        gGujaratiLigMap.insert(std::make_pair( 0xEB0A, dvngLig( 275 ,"0x0A9A 0x0AB0 0x0ACD 0x0ACD"))); // 0x100B8
        gGujaratiLigMap.insert(std::make_pair( 0xEB0B, dvngLig( 276 ,"0x0A9B 0x0ACD 0x0AB0 0x0ACD"))); // 0x100B9
        gGujaratiLigMap.insert(std::make_pair( 0xEB0C, dvngLig( 276 ,"0x0A9B 0x0AB0 0x0ACD 0x0ACD"))); // 0x100B9
        gGujaratiLigMap.insert(std::make_pair( 0xEB0D, dvngLig( 276 ,"0x0A9B 0x0ACD 0x0ACD 0x0AB0"))); // 0x100B9
        gGujaratiLigMap.insert(std::make_pair( 0xEB0E, dvngLig( 277 ,"0x0A9C 0x0ACD 0x0ACD 0x0AB0"))); // 0x100BA
        gGujaratiLigMap.insert(std::make_pair( 0xEB0F, dvngLig( 277 ,"0x0A9C 0x0ACD 0x0AB0 0x0ACD"))); // 0x100BA
        gGujaratiLigMap.insert(std::make_pair( 0xEB10, dvngLig( 277 ,"0x0A9C 0x0AB0 0x0ACD 0x0ACD"))); // 0x100BA
        gGujaratiLigMap.insert(std::make_pair( 0xEB11, dvngLig( 278 ,"0x0A9D 0x0ACD 0x0ACD 0x0AB0"))); // 0x100BB
        gGujaratiLigMap.insert(std::make_pair( 0xEB12, dvngLig( 278 ,"0x0A9D 0x0ACD 0x0AB0 0x0ACD"))); // 0x100BB
        gGujaratiLigMap.insert(std::make_pair( 0xEB13, dvngLig( 278 ,"0x0A9D 0x0AB0 0x0ACD 0x0ACD"))); // 0x100BB
        gGujaratiLigMap.insert(std::make_pair( 0xEB14, dvngLig( 279 ,"0x0A9E 0x0ACD 0x0ACD 0x0AB0"))); // 0x100BC
        gGujaratiLigMap.insert(std::make_pair( 0xEB15, dvngLig( 279 ,"0x0A9E 0x0ACD 0x0AB0 0x0ACD"))); // 0x100BC
        gGujaratiLigMap.insert(std::make_pair( 0xEB16, dvngLig( 279 ,"0x0A9E 0x0AB0 0x0ACD 0x0ACD"))); // 0x100BC
        gGujaratiLigMap.insert(std::make_pair( 0xEB17, dvngLig( 280 ,"0x0A9F 0x0ACD 0x0AB0 0x0ACD"))); // 0x100BD
        gGujaratiLigMap.insert(std::make_pair( 0xEB18, dvngLig( 280 ,"0x0A9F 0x0AB0 0x0ACD 0x0ACD"))); // 0x100BD
        gGujaratiLigMap.insert(std::make_pair( 0xEB19, dvngLig( 280 ,"0x0A9F 0x0ACD 0x0ACD 0x0AB0"))); // 0x100BD
        gGujaratiLigMap.insert(std::make_pair( 0xEB1A, dvngLig( 281 ,"0x0AA0 0x0ACD 0x0AB0 0x0ACD"))); // 0x100BE
        gGujaratiLigMap.insert(std::make_pair( 0xEB1B, dvngLig( 281 ,"0x0AA0 0x0AB0 0x0ACD 0x0ACD"))); // 0x100BE
        gGujaratiLigMap.insert(std::make_pair( 0xEB1C, dvngLig( 281 ,"0x0AA0 0x0ACD 0x0ACD 0x0AB0"))); // 0x100BE
        gGujaratiLigMap.insert(std::make_pair( 0xEB1D, dvngLig( 282 ,"0x0AA1 0x0ACD 0x0AB0 0x0ACD"))); // 0x100BF
        gGujaratiLigMap.insert(std::make_pair( 0xEB1E, dvngLig( 282 ,"0x0AA1 0x0AB0 0x0ACD 0x0ACD"))); // 0x100BF
        gGujaratiLigMap.insert(std::make_pair( 0xEB1F, dvngLig( 282 ,"0x0AA1 0x0ACD 0x0ACD 0x0AB0"))); // 0x100BF
        gGujaratiLigMap.insert(std::make_pair( 0xEB20, dvngLig( 283 ,"0x0AA2 0x0ACD 0x0AB0 0x0ACD"))); // 0x100C0
        gGujaratiLigMap.insert(std::make_pair( 0xEB21, dvngLig( 283 ,"0x0AA2 0x0AB0 0x0ACD 0x0ACD"))); // 0x100C0
        gGujaratiLigMap.insert(std::make_pair( 0xEB22, dvngLig( 283 ,"0x0AA2 0x0ACD 0x0ACD 0x0AB0"))); // 0x100C0
        gGujaratiLigMap.insert(std::make_pair( 0xEB23, dvngLig( 284 ,"0x0AA3 0x0ACD 0x0ACD 0x0AB0"))); // 0x100C1
        gGujaratiLigMap.insert(std::make_pair( 0xEB24, dvngLig( 284 ,"0x0AA3 0x0ACD 0x0AB0 0x0ACD"))); // 0x100C1
        gGujaratiLigMap.insert(std::make_pair( 0xEB25, dvngLig( 284 ,"0x0AA3 0x0AB0 0x0ACD 0x0ACD"))); // 0x100C1
        gGujaratiLigMap.insert(std::make_pair( 0xEB26, dvngLig( 285 ,"0x0AA4 0x0ACD 0x0ACD 0x0AB0"))); // 0x100C2
        gGujaratiLigMap.insert(std::make_pair( 0xEB27, dvngLig( 285 ,"0x0AA4 0x0ACD 0x0AB0 0x0ACD"))); // 0x100C2
        gGujaratiLigMap.insert(std::make_pair( 0xEB28, dvngLig( 285 ,"0x0AA4 0x0AB0 0x0ACD 0x0ACD"))); // 0x100C2
        gGujaratiLigMap.insert(std::make_pair( 0xEB29, dvngLig( 286 ,"0x0AA5 0x0ACD 0x0ACD 0x0AB0"))); // 0x100C3
        gGujaratiLigMap.insert(std::make_pair( 0xEB2A, dvngLig( 286 ,"0x0AA5 0x0ACD 0x0AB0 0x0ACD"))); // 0x100C3
        gGujaratiLigMap.insert(std::make_pair( 0xEB2B, dvngLig( 286 ,"0x0AA5 0x0AB0 0x0ACD 0x0ACD"))); // 0x100C3
        gGujaratiLigMap.insert(std::make_pair( 0xEB2C, dvngLig( 287 ,"0x0AA6 0x0ACD 0x0AB0 0x0ACD"))); // 0x100C4
        gGujaratiLigMap.insert(std::make_pair( 0xEB2D, dvngLig( 287 ,"0x0AA6 0x0AB0 0x0ACD 0x0ACD"))); // 0x100C4
        gGujaratiLigMap.insert(std::make_pair( 0xEB2E, dvngLig( 287 ,"0x0AA6 0x0ACD 0x0ACD 0x0AB0"))); // 0x100C4
        gGujaratiLigMap.insert(std::make_pair( 0xEB2F, dvngLig( 288 ,"0x0AA7 0x0ACD 0x0ACD 0x0AB0"))); // 0x100C5
        gGujaratiLigMap.insert(std::make_pair( 0xEB30, dvngLig( 288 ,"0x0AA7 0x0ACD 0x0AB0 0x0ACD"))); // 0x100C5
        gGujaratiLigMap.insert(std::make_pair( 0xEB31, dvngLig( 288 ,"0x0AA7 0x0AB0 0x0ACD 0x0ACD"))); // 0x100C5
        gGujaratiLigMap.insert(std::make_pair( 0xEB32, dvngLig( 289 ,"0x0AA8 0x0ACD 0x0ACD 0x0AB0"))); // 0x100C6
        gGujaratiLigMap.insert(std::make_pair( 0xEB33, dvngLig( 289 ,"0x0AA8 0x0ACD 0x0AB0 0x0ACD"))); // 0x100C6
        gGujaratiLigMap.insert(std::make_pair( 0xEB34, dvngLig( 289 ,"0x0AA8 0x0AB0 0x0ACD 0x0ACD"))); // 0x100C6
        gGujaratiLigMap.insert(std::make_pair( 0xEB35, dvngLig( 290 ,"0x0AAA 0x0ACD 0x0ACD 0x0AB0"))); // 0x100C7
        gGujaratiLigMap.insert(std::make_pair( 0xEB36, dvngLig( 290 ,"0x0AAA 0x0ACD 0x0AB0 0x0ACD"))); // 0x100C7
        gGujaratiLigMap.insert(std::make_pair( 0xEB37, dvngLig( 290 ,"0x0AAA 0x0AB0 0x0ACD 0x0ACD"))); // 0x100C7
        gGujaratiLigMap.insert(std::make_pair( 0xEB38, dvngLig( 291 ,"0x0AAB 0x0ACD 0x0ACD 0x0AB0"))); // 0x100C8
        gGujaratiLigMap.insert(std::make_pair( 0xEB39, dvngLig( 291 ,"0x0AAB 0x0ACD 0x0AB0 0x0ACD"))); // 0x100C8
        gGujaratiLigMap.insert(std::make_pair( 0xEB3A, dvngLig( 291 ,"0x0AAB 0x0AB0 0x0ACD 0x0ACD"))); // 0x100C8
        gGujaratiLigMap.insert(std::make_pair( 0xEB3B, dvngLig( 292 ,"0x0AAC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100C9
        gGujaratiLigMap.insert(std::make_pair( 0xEB3C, dvngLig( 292 ,"0x0AAC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100C9
        gGujaratiLigMap.insert(std::make_pair( 0xEB3D, dvngLig( 292 ,"0x0AAC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100C9
        gGujaratiLigMap.insert(std::make_pair( 0xEB3E, dvngLig( 293 ,"0x0AAD 0x0ACD 0x0ACD 0x0AB0"))); // 0x100CA
        gGujaratiLigMap.insert(std::make_pair( 0xEB3F, dvngLig( 293 ,"0x0AAD 0x0ACD 0x0AB0 0x0ACD"))); // 0x100CA
        gGujaratiLigMap.insert(std::make_pair( 0xEB40, dvngLig( 293 ,"0x0AAD 0x0AB0 0x0ACD 0x0ACD"))); // 0x100CA
        gGujaratiLigMap.insert(std::make_pair( 0xEB41, dvngLig( 294 ,"0x0AAE 0x0ACD 0x0ACD 0x0AB0"))); // 0x100CB
        gGujaratiLigMap.insert(std::make_pair( 0xEB42, dvngLig( 294 ,"0x0AAE 0x0ACD 0x0AB0 0x0ACD"))); // 0x100CB
        gGujaratiLigMap.insert(std::make_pair( 0xEB43, dvngLig( 294 ,"0x0AAE 0x0AB0 0x0ACD 0x0ACD"))); // 0x100CB
        gGujaratiLigMap.insert(std::make_pair( 0xEB44, dvngLig( 295 ,"0x0AAF 0x0ACD 0x0ACD 0x0AB0"))); // 0x100CC
        gGujaratiLigMap.insert(std::make_pair( 0xEB45, dvngLig( 295 ,"0x0AAF 0x0ACD 0x0AB0 0x0ACD"))); // 0x100CC
        gGujaratiLigMap.insert(std::make_pair( 0xEB46, dvngLig( 295 ,"0x0AAF 0x0AB0 0x0ACD 0x0ACD"))); // 0x100CC
        gGujaratiLigMap.insert(std::make_pair( 0xEB47, dvngLig( 296 ,"0x0AB0 0x0ACD 0x0ACD 0x0AB0"))); // 0x100CD
        gGujaratiLigMap.insert(std::make_pair( 0xEB48, dvngLig( 296 ,"0x0AB0 0x0ACD 0x0AB0 0x0ACD"))); // 0x100CD
        gGujaratiLigMap.insert(std::make_pair( 0xEB49, dvngLig( 297 ,"0x0AB2 0x0ACD 0x0ACD 0x0AB0"))); // 0x100CE
        gGujaratiLigMap.insert(std::make_pair( 0xEB4A, dvngLig( 297 ,"0x0AB2 0x0ACD 0x0AB0 0x0ACD"))); // 0x100CE
        gGujaratiLigMap.insert(std::make_pair( 0xEB4B, dvngLig( 297 ,"0x0AB2 0x0AB0 0x0ACD 0x0ACD"))); // 0x100CE
        gGujaratiLigMap.insert(std::make_pair( 0xEB4C, dvngLig( 298 ,"0x0AB3 0x0ACD 0x0ACD 0x0AB0"))); // 0x100CF
        gGujaratiLigMap.insert(std::make_pair( 0xEB4D, dvngLig( 298 ,"0x0AB3 0x0ACD 0x0AB0 0x0ACD"))); // 0x100CF
        gGujaratiLigMap.insert(std::make_pair( 0xEB4E, dvngLig( 298 ,"0x0AB3 0x0AB0 0x0ACD 0x0ACD"))); // 0x100CF
        gGujaratiLigMap.insert(std::make_pair( 0xEB4F, dvngLig( 299 ,"0x0AB5 0x0ACD 0x0ACD 0x0AB0"))); // 0x100D0
        gGujaratiLigMap.insert(std::make_pair( 0xEB50, dvngLig( 299 ,"0x0AB5 0x0ACD 0x0AB0 0x0ACD"))); // 0x100D0
        gGujaratiLigMap.insert(std::make_pair( 0xEB51, dvngLig( 299 ,"0x0AB5 0x0AB0 0x0ACD 0x0ACD"))); // 0x100D0
        gGujaratiLigMap.insert(std::make_pair( 0xEB52, dvngLig( 300 ,"0x0AB6 0x0ACD 0x0ACD 0x0AB0"))); // 0x100D1
        gGujaratiLigMap.insert(std::make_pair( 0xEB53, dvngLig( 300 ,"0x0AB6 0x0ACD 0x0AB0 0x0ACD"))); // 0x100D1
        gGujaratiLigMap.insert(std::make_pair( 0xEB54, dvngLig( 300 ,"0x0AB6 0x0AB0 0x0ACD 0x0ACD"))); // 0x100D1
        gGujaratiLigMap.insert(std::make_pair( 0xEB55, dvngLig( 301 ,"0x0AB7 0x0ACD 0x0ACD 0x0AB0"))); // 0x100D2
        gGujaratiLigMap.insert(std::make_pair( 0xEB56, dvngLig( 301 ,"0x0AB7 0x0ACD 0x0AB0 0x0ACD"))); // 0x100D2
        gGujaratiLigMap.insert(std::make_pair( 0xEB57, dvngLig( 301 ,"0x0AB7 0x0AB0 0x0ACD 0x0ACD"))); // 0x100D2
        gGujaratiLigMap.insert(std::make_pair( 0xEB58, dvngLig( 302 ,"0x0AB8 0x0ACD 0x0ACD 0x0AB0"))); // 0x100D3
        gGujaratiLigMap.insert(std::make_pair( 0xEB59, dvngLig( 302 ,"0x0AB8 0x0ACD 0x0AB0 0x0ACD"))); // 0x100D3
        gGujaratiLigMap.insert(std::make_pair( 0xEB5A, dvngLig( 302 ,"0x0AB8 0x0AB0 0x0ACD 0x0ACD"))); // 0x100D3
        gGujaratiLigMap.insert(std::make_pair( 0xEB5B, dvngLig( 303 ,"0x0AB9 0x0ACD 0x0ACD 0x0AB0"))); // 0x100D4
        gGujaratiLigMap.insert(std::make_pair( 0xEB5C, dvngLig( 303 ,"0x0AB9 0x0ACD 0x0AB0 0x0ACD"))); // 0x100D4
        gGujaratiLigMap.insert(std::make_pair( 0xEB5D, dvngLig( 303 ,"0x0AB9 0x0AB0 0x0ACD 0x0ACD"))); // 0x100D4
        gGujaratiLigMap.insert(std::make_pair( 0xEB5E, dvngLig( 304 ,"0x0A95 0x0ACD 0x0AB7 0x0ACD 0x0ACD 0x0AB0"))); // 0x100D5
        gGujaratiLigMap.insert(std::make_pair( 0xEB5F, dvngLig( 304 ,"0x0A95 0x0ACD 0x0AB7 0x0ACD 0x0AB0 0x0ACD"))); // 0x100D5
        gGujaratiLigMap.insert(std::make_pair( 0xEB60, dvngLig( 304 ,"0x0A95 0x0ACD 0x0AB7 0x0AB0 0x0ACD 0x0ACD"))); // 0x100D5
        gGujaratiLigMap.insert(std::make_pair( 0xEB61, dvngLig( 305 ,"0x0A9C 0x0ACD 0x0A9E 0x0ACD 0x0ACD 0x0AB0"))); // 0x100D6
        gGujaratiLigMap.insert(std::make_pair( 0xEB62, dvngLig( 305 ,"0x0A9C 0x0ACD 0x0A9E 0x0ACD 0x0AB0 0x0ACD"))); // 0x100D6
        gGujaratiLigMap.insert(std::make_pair( 0xEB63, dvngLig( 305 ,"0x0A9C 0x0ACD 0x0A9E 0x0AB0 0x0ACD 0x0ACD"))); // 0x100D6
        gGujaratiLigMap.insert(std::make_pair( 0xEB64, dvngLig( 306 ,"0x0A95 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100D7
        gGujaratiLigMap.insert(std::make_pair( 0xEB65, dvngLig( 306 ,"0x0A95 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100D7
        gGujaratiLigMap.insert(std::make_pair( 0xEB66, dvngLig( 306 ,"0x0A95 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100D7
        gGujaratiLigMap.insert(std::make_pair( 0xEB67, dvngLig( 307 ,"0x0A96 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100D8
        gGujaratiLigMap.insert(std::make_pair( 0xEB68, dvngLig( 307 ,"0x0A96 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100D8
        gGujaratiLigMap.insert(std::make_pair( 0xEB69, dvngLig( 307 ,"0x0A96 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100D8
        gGujaratiLigMap.insert(std::make_pair( 0xEB6A, dvngLig( 308 ,"0x0A97 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100D9
        gGujaratiLigMap.insert(std::make_pair( 0xEB6B, dvngLig( 308 ,"0x0A97 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100D9
        gGujaratiLigMap.insert(std::make_pair( 0xEB6C, dvngLig( 308 ,"0x0A97 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100D9
        gGujaratiLigMap.insert(std::make_pair( 0xEB6D, dvngLig( 309 ,"0x0A98 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100DA
        gGujaratiLigMap.insert(std::make_pair( 0xEB6E, dvngLig( 309 ,"0x0A98 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100DA
        gGujaratiLigMap.insert(std::make_pair( 0xEB6F, dvngLig( 309 ,"0x0A98 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100DA
        gGujaratiLigMap.insert(std::make_pair( 0xEB70, dvngLig( 310 ,"0x0A99 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100DB
        gGujaratiLigMap.insert(std::make_pair( 0xEB71, dvngLig( 310 ,"0x0A99 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100DB
        gGujaratiLigMap.insert(std::make_pair( 0xEB72, dvngLig( 310 ,"0x0A99 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100DB
        gGujaratiLigMap.insert(std::make_pair( 0xEB73, dvngLig( 311 ,"0x0A9A 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100DC
        gGujaratiLigMap.insert(std::make_pair( 0xEB74, dvngLig( 311 ,"0x0A9A 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100DC
        gGujaratiLigMap.insert(std::make_pair( 0xEB75, dvngLig( 311 ,"0x0A9A 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100DC
        gGujaratiLigMap.insert(std::make_pair( 0xEB76, dvngLig( 312 ,"0x0A9B 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100DD
        gGujaratiLigMap.insert(std::make_pair( 0xEB77, dvngLig( 312 ,"0x0A9B 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100DD
        gGujaratiLigMap.insert(std::make_pair( 0xEB78, dvngLig( 312 ,"0x0A9B 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100DD
        gGujaratiLigMap.insert(std::make_pair( 0xEB79, dvngLig( 313 ,"0x0A9C 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100DE
        gGujaratiLigMap.insert(std::make_pair( 0xEB7A, dvngLig( 313 ,"0x0A9C 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100DE
        gGujaratiLigMap.insert(std::make_pair( 0xEB7B, dvngLig( 313 ,"0x0A9C 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100DE
        gGujaratiLigMap.insert(std::make_pair( 0xEB7C, dvngLig( 314 ,"0x0A9D 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100DF
        gGujaratiLigMap.insert(std::make_pair( 0xEB7D, dvngLig( 314 ,"0x0A9D 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100DF
        gGujaratiLigMap.insert(std::make_pair( 0xEB7E, dvngLig( 314 ,"0x0A9D 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100DF
        gGujaratiLigMap.insert(std::make_pair( 0xEB7F, dvngLig( 315 ,"0x0A9E 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100E0
        gGujaratiLigMap.insert(std::make_pair( 0xEB80, dvngLig( 315 ,"0x0A9E 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100E0
        gGujaratiLigMap.insert(std::make_pair( 0xEB81, dvngLig( 315 ,"0x0A9E 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100E0
        gGujaratiLigMap.insert(std::make_pair( 0xEB82, dvngLig( 316 ,"0x0A9F 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100E1
        gGujaratiLigMap.insert(std::make_pair( 0xEB83, dvngLig( 316 ,"0x0A9F 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100E1
        gGujaratiLigMap.insert(std::make_pair( 0xEB84, dvngLig( 316 ,"0x0A9F 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100E1
        gGujaratiLigMap.insert(std::make_pair( 0xEB85, dvngLig( 317 ,"0x0AA0 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100E2
        gGujaratiLigMap.insert(std::make_pair( 0xEB86, dvngLig( 317 ,"0x0AA0 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100E2
        gGujaratiLigMap.insert(std::make_pair( 0xEB87, dvngLig( 317 ,"0x0AA0 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100E2
        gGujaratiLigMap.insert(std::make_pair( 0xEB88, dvngLig( 318 ,"0x0AA1 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100E3
        gGujaratiLigMap.insert(std::make_pair( 0xEB89, dvngLig( 318 ,"0x0AA1 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100E3
        gGujaratiLigMap.insert(std::make_pair( 0xEB8A, dvngLig( 318 ,"0x0AA1 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100E3
        gGujaratiLigMap.insert(std::make_pair( 0xEB8B, dvngLig( 319 ,"0x0AA2 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100E4
        gGujaratiLigMap.insert(std::make_pair( 0xEB8C, dvngLig( 319 ,"0x0AA2 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100E4
        gGujaratiLigMap.insert(std::make_pair( 0xEB8D, dvngLig( 319 ,"0x0AA2 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100E4
        gGujaratiLigMap.insert(std::make_pair( 0xEB8E, dvngLig( 320 ,"0x0AA3 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100E5
        gGujaratiLigMap.insert(std::make_pair( 0xEB8F, dvngLig( 320 ,"0x0AA3 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100E5
        gGujaratiLigMap.insert(std::make_pair( 0xEB90, dvngLig( 320 ,"0x0AA3 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100E5
        gGujaratiLigMap.insert(std::make_pair( 0xEB91, dvngLig( 321 ,"0x0AA4 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100E6
        gGujaratiLigMap.insert(std::make_pair( 0xEB92, dvngLig( 321 ,"0x0AA4 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100E6
        gGujaratiLigMap.insert(std::make_pair( 0xEB93, dvngLig( 321 ,"0x0AA4 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100E6
        gGujaratiLigMap.insert(std::make_pair( 0xEB94, dvngLig( 322 ,"0x0AA5 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100E7
        gGujaratiLigMap.insert(std::make_pair( 0xEB95, dvngLig( 322 ,"0x0AA5 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100E7
        gGujaratiLigMap.insert(std::make_pair( 0xEB96, dvngLig( 322 ,"0x0AA5 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100E7
        gGujaratiLigMap.insert(std::make_pair( 0xEB97, dvngLig( 323 ,"0x0AA6 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100E8
        gGujaratiLigMap.insert(std::make_pair( 0xEB98, dvngLig( 323 ,"0x0AA6 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100E8
        gGujaratiLigMap.insert(std::make_pair( 0xEB99, dvngLig( 323 ,"0x0AA6 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100E8
        gGujaratiLigMap.insert(std::make_pair( 0xEB9A, dvngLig( 324 ,"0x0AA7 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100E9
        gGujaratiLigMap.insert(std::make_pair( 0xEB9B, dvngLig( 324 ,"0x0AA7 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100E9
        gGujaratiLigMap.insert(std::make_pair( 0xEB9C, dvngLig( 324 ,"0x0AA7 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100E9
        gGujaratiLigMap.insert(std::make_pair( 0xEB9D, dvngLig( 325 ,"0x0AA8 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100EA
        gGujaratiLigMap.insert(std::make_pair( 0xEB9E, dvngLig( 325 ,"0x0AA8 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100EA
        gGujaratiLigMap.insert(std::make_pair( 0xEB9F, dvngLig( 325 ,"0x0AA8 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100EA
        gGujaratiLigMap.insert(std::make_pair( 0xEBA0, dvngLig( 326 ,"0x0AAA 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100EB
        gGujaratiLigMap.insert(std::make_pair( 0xEBA1, dvngLig( 326 ,"0x0AAA 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100EB
        gGujaratiLigMap.insert(std::make_pair( 0xEBA2, dvngLig( 326 ,"0x0AAA 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100EB
        gGujaratiLigMap.insert(std::make_pair( 0xEBA3, dvngLig( 327 ,"0x0AAB 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100EC
        gGujaratiLigMap.insert(std::make_pair( 0xEBA4, dvngLig( 327 ,"0x0AAB 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100EC
        gGujaratiLigMap.insert(std::make_pair( 0xEBA5, dvngLig( 327 ,"0x0AAB 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100EC
        gGujaratiLigMap.insert(std::make_pair( 0xEBA6, dvngLig( 328 ,"0x0AAC 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100ED
        gGujaratiLigMap.insert(std::make_pair( 0xEBA7, dvngLig( 328 ,"0x0AAC 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100ED
        gGujaratiLigMap.insert(std::make_pair( 0xEBA8, dvngLig( 328 ,"0x0AAC 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100ED
        gGujaratiLigMap.insert(std::make_pair( 0xEBA9, dvngLig( 329 ,"0x0AAD 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100EE
        gGujaratiLigMap.insert(std::make_pair( 0xEBAA, dvngLig( 329 ,"0x0AAD 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100EE
        gGujaratiLigMap.insert(std::make_pair( 0xEBAB, dvngLig( 329 ,"0x0AAD 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100EE
        gGujaratiLigMap.insert(std::make_pair( 0xEBAC, dvngLig( 330 ,"0x0AAE 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100EF
        gGujaratiLigMap.insert(std::make_pair( 0xEBAD, dvngLig( 330 ,"0x0AAE 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100EF
        gGujaratiLigMap.insert(std::make_pair( 0xEBAE, dvngLig( 330 ,"0x0AAE 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100EF
        gGujaratiLigMap.insert(std::make_pair( 0xEBAF, dvngLig( 331 ,"0x0AAF 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100F0
        gGujaratiLigMap.insert(std::make_pair( 0xEBB0, dvngLig( 331 ,"0x0AAF 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100F0
        gGujaratiLigMap.insert(std::make_pair( 0xEBB1, dvngLig( 331 ,"0x0AAF 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100F0
        gGujaratiLigMap.insert(std::make_pair( 0xEBB2, dvngLig( 332 ,"0x0AB0 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100F1
        gGujaratiLigMap.insert(std::make_pair( 0xEBB3, dvngLig( 332 ,"0x0AB0 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100F1
        gGujaratiLigMap.insert(std::make_pair( 0xEBB4, dvngLig( 332 ,"0x0AB0 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100F1
        gGujaratiLigMap.insert(std::make_pair( 0xEBB5, dvngLig( 333 ,"0x0AB2 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100F2
        gGujaratiLigMap.insert(std::make_pair( 0xEBB6, dvngLig( 333 ,"0x0AB2 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100F2
        gGujaratiLigMap.insert(std::make_pair( 0xEBB7, dvngLig( 333 ,"0x0AB2 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100F2
        gGujaratiLigMap.insert(std::make_pair( 0xEBB8, dvngLig( 334 ,"0x0AB3 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100F3
        gGujaratiLigMap.insert(std::make_pair( 0xEBB9, dvngLig( 334 ,"0x0AB3 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100F3
        gGujaratiLigMap.insert(std::make_pair( 0xEBBA, dvngLig( 334 ,"0x0AB3 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100F3
        gGujaratiLigMap.insert(std::make_pair( 0xEBBB, dvngLig( 335 ,"0x0AB5 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100F4
        gGujaratiLigMap.insert(std::make_pair( 0xEBBC, dvngLig( 335 ,"0x0AB5 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100F4
        gGujaratiLigMap.insert(std::make_pair( 0xEBBD, dvngLig( 335 ,"0x0AB5 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100F4
        gGujaratiLigMap.insert(std::make_pair( 0xEBBE, dvngLig( 336 ,"0x0AB6 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100F5
        gGujaratiLigMap.insert(std::make_pair( 0xEBBF, dvngLig( 336 ,"0x0AB6 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100F5
        gGujaratiLigMap.insert(std::make_pair( 0xEBC0, dvngLig( 336 ,"0x0AB6 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100F5
        gGujaratiLigMap.insert(std::make_pair( 0xEBC1, dvngLig( 337 ,"0x0AB7 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100F6
        gGujaratiLigMap.insert(std::make_pair( 0xEBC2, dvngLig( 337 ,"0x0AB7 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100F6
        gGujaratiLigMap.insert(std::make_pair( 0xEBC3, dvngLig( 337 ,"0x0AB7 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100F6
        gGujaratiLigMap.insert(std::make_pair( 0xEBC4, dvngLig( 338 ,"0x0AB8 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100F7
        gGujaratiLigMap.insert(std::make_pair( 0xEBC5, dvngLig( 338 ,"0x0AB8 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100F7
        gGujaratiLigMap.insert(std::make_pair( 0xEBC6, dvngLig( 338 ,"0x0AB8 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100F7
        gGujaratiLigMap.insert(std::make_pair( 0xEBC7, dvngLig( 339 ,"0x0AB9 0x0ABC 0x0ACD 0x0ACD 0x0AB0"))); // 0x100F8
        gGujaratiLigMap.insert(std::make_pair( 0xEBC8, dvngLig( 339 ,"0x0AB9 0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x100F8
        gGujaratiLigMap.insert(std::make_pair( 0xEBC9, dvngLig( 339 ,"0x0AB9 0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x100F8
        gGujaratiLigMap.insert(std::make_pair( 0xEBCA, dvngLig( 340 ,"0x0A95 0x0ACD 0x0A95"))); // 0x100F9
        gGujaratiLigMap.insert(std::make_pair( 0xEBCB, dvngLig( 341 ,"0x0A95 0x0ACD 0x0AAF"))); // 0x100FA
        gGujaratiLigMap.insert(std::make_pair( 0xEBCC, dvngLig( 342 ,"0x0A97 0x0ACD 0x0AA8"))); // 0x100FB
        gGujaratiLigMap.insert(std::make_pair( 0xEBCD, dvngLig( 343 ,"0x0A99 0x0ACD 0x0A95"))); // 0x100FC
        gGujaratiLigMap.insert(std::make_pair( 0xEBCE, dvngLig( 344 ,"0x0A99 0x0ACD 0x0A97"))); // 0x100FD
        gGujaratiLigMap.insert(std::make_pair( 0xEBCF, dvngLig( 345 ,"0x0A99 0x0ACD 0x0A98"))); // 0x100FE
        gGujaratiLigMap.insert(std::make_pair( 0xEBD0, dvngLig( 346 ,"0x0A99 0x0ACD 0x0AAE"))); // 0x100FF
        gGujaratiLigMap.insert(std::make_pair( 0xEBD1, dvngLig( 347 ,"0x0A99 0x0ACD 0x0AAF"))); // 0x10100
        gGujaratiLigMap.insert(std::make_pair( 0xEBD2, dvngLig( 348 ,"0x0A9B 0x0ACD 0x0AAF"))); // 0x10101
        gGujaratiLigMap.insert(std::make_pair( 0xEBD3, dvngLig( 349 ,"0x0A9B 0x0ACD 0x0AB5"))); // 0x10102
        gGujaratiLigMap.insert(std::make_pair( 0xEBD4, dvngLig( 350 ,"0x0A9E 0x0ACD 0x0A9A"))); // 0x10103
        gGujaratiLigMap.insert(std::make_pair( 0xEBD5, dvngLig( 351 ,"0x0A9E 0x0ACD 0x0A9C"))); // 0x10104
        gGujaratiLigMap.insert(std::make_pair( 0xEBD6, dvngLig( 352 ,"0x0A9F 0x0ACD 0x0A9F"))); // 0x10105
        gGujaratiLigMap.insert(std::make_pair( 0xEBD7, dvngLig( 353 ,"0x0A9F 0x0ACD 0x0A9F 0x0AC1"))); // 0x10106
        gGujaratiLigMap.insert(std::make_pair( 0xEBD8, dvngLig( 354 ,"0x0A9F 0x0ACD 0x0A9F 0x0AC2"))); // 0x10107
        gGujaratiLigMap.insert(std::make_pair( 0xEBD9, dvngLig( 355 ,"0x0A9F 0x0ACD 0x0AA0"))); // 0x10108
        gGujaratiLigMap.insert(std::make_pair( 0xEBDA, dvngLig( 356 ,"0x0A9F 0x0ACD 0x0AA0 0x0AC2"))); // 0x10109
        gGujaratiLigMap.insert(std::make_pair( 0xEBDB, dvngLig( 357 ,"0x0A9F 0x0ACD 0x0AAF"))); // 0x1010A
        gGujaratiLigMap.insert(std::make_pair( 0xEBDC, dvngLig( 358 ,"0x0A9F 0x0ACD 0x0AB5"))); // 0x1010B
        gGujaratiLigMap.insert(std::make_pair( 0xEBDD, dvngLig( 359 ,"0x0AA0 0x0ACD 0x0AA0"))); // 0x1010C
        gGujaratiLigMap.insert(std::make_pair( 0xEBDE, dvngLig( 360 ,"0x0AA0 0x0ACD 0x0AA0 0x0AC1"))); // 0x1010D
        gGujaratiLigMap.insert(std::make_pair( 0xEBDF, dvngLig( 361 ,"0x0AA0 0x0ACD 0x0AAF"))); // 0x1010E
        gGujaratiLigMap.insert(std::make_pair( 0xEBE0, dvngLig( 362 ,"0x0AA1 0x0ACD 0x0AA1"))); // 0x1010F
        gGujaratiLigMap.insert(std::make_pair( 0xEBE1, dvngLig( 363 ,"0x0AA1 0x0ACD 0x0AA1 0x0AC1"))); // 0x10110
        gGujaratiLigMap.insert(std::make_pair( 0xEBE2, dvngLig( 364 ,"0x0AA1 0x0ACD 0x0AA2"))); // 0x10111
        gGujaratiLigMap.insert(std::make_pair( 0xEBE3, dvngLig( 365 ,"0x0AA1 0x0ACD 0x0AAF"))); // 0x10112
        gGujaratiLigMap.insert(std::make_pair( 0xEBE4, dvngLig( 366 ,"0x0AA2 0x0ACD 0x0AA2"))); // 0x10113
        gGujaratiLigMap.insert(std::make_pair( 0xEBE5, dvngLig( 367 ,"0x0AA2 0x0ACD 0x0AAF"))); // 0x10114
        gGujaratiLigMap.insert(std::make_pair( 0xEBE6, dvngLig( 368 ,"0x0AA4 0x0ACD 0x0AA4"))); // 0x10115
        gGujaratiLigMap.insert(std::make_pair( 0xEBE7, dvngLig( 369 ,"0x0AA6 0x0ACD 0x0A97"))); // 0x10116
        gGujaratiLigMap.insert(std::make_pair( 0xEBE8, dvngLig( 370 ,"0x0AA6 0x0ACD 0x0A98"))); // 0x10117
        gGujaratiLigMap.insert(std::make_pair( 0xEBE9, dvngLig( 371 ,"0x0AA6 0x0ACD 0x0AA6"))); // 0x10118
        gGujaratiLigMap.insert(std::make_pair( 0xEBEA, dvngLig( 372 ,"0x0AA6 0x0ACD 0x0AA7"))); // 0x10119
        gGujaratiLigMap.insert(std::make_pair( 0xEBEB, dvngLig( 373 ,"0x0AA6 0x0ACD 0x0AA8"))); // 0x1011A
        gGujaratiLigMap.insert(std::make_pair( 0xEBEC, dvngLig( 374 ,"0x0AA6 0x0ACD 0x0AAC"))); // 0x1011B
        gGujaratiLigMap.insert(std::make_pair( 0xEBED, dvngLig( 375 ,"0x0AA6 0x0ACD 0x0AAD"))); // 0x1011C
        gGujaratiLigMap.insert(std::make_pair( 0xEBEE, dvngLig( 376 ,"0x0AA6 0x0ACD 0x0AAE"))); // 0x1011D
        gGujaratiLigMap.insert(std::make_pair( 0xEBEF, dvngLig( 377 ,"0x0AA6 0x0ACD 0x0AAF"))); // 0x1011E
        gGujaratiLigMap.insert(std::make_pair( 0xEBF0, dvngLig( 378 ,"0x0AA6 0x0ACD 0x0AB5"))); // 0x1011F
        gGujaratiLigMap.insert(std::make_pair( 0xEBF1, dvngLig( 379 ,"0x0AA8 0x0ACD 0x0AA8"))); // 0x10120
        gGujaratiLigMap.insert(std::make_pair( 0xEBF2, dvngLig( 380 ,"0x0AAB 0x0ACD 0x0AAF"))); // 0x10121
        gGujaratiLigMap.insert(std::make_pair( 0xEBF3, dvngLig( 381 ,"0x0AB3 0x0ACD 0x0AAF"))); // 0x10122
        gGujaratiLigMap.insert(std::make_pair( 0xEBF4, dvngLig( 382 ,"0x0AB6 0x0ACD 0x0A9A"))); // 0x10123
        gGujaratiLigMap.insert(std::make_pair( 0xEBF5, dvngLig( 383 ,"0x0AB6 0x0ACD 0x0AA8"))); // 0x10124
        gGujaratiLigMap.insert(std::make_pair( 0xEBF6, dvngLig( 384 ,"0x0AB6 0x0ACD 0x0AB2"))); // 0x10125
        gGujaratiLigMap.insert(std::make_pair( 0xEBF7, dvngLig( 385 ,"0x0AB6 0x0ACD 0x0AB5"))); // 0x10126
        gGujaratiLigMap.insert(std::make_pair( 0xEBF8, dvngLig( 386 ,"0x0AB7 0x0ACD 0x0A9F"))); // 0x10127
        gGujaratiLigMap.insert(std::make_pair( 0xEBF9, dvngLig( 387 ,"0x0AB7 0x0ACD 0x0A9F 0x0ACD 0x0AB0"))); // 0x10128
        gGujaratiLigMap.insert(std::make_pair( 0xEBFA, dvngLig( 387 ,"0x0AB7 0x0ACD 0x0A9F 0x0AB0 0x0ACD"))); // 0x10128
        gGujaratiLigMap.insert(std::make_pair( 0xEBFB, dvngLig( 388 ,"0x0AB7 0x0ACD 0x0AA0"))); // 0x10129
        gGujaratiLigMap.insert(std::make_pair( 0xEBFC, dvngLig( 389 ,"0x0AB7 0x0ACD 0x0AA0 0x0ACD 0x0AB0"))); // 0x1012A
        gGujaratiLigMap.insert(std::make_pair( 0xEBFD, dvngLig( 389 ,"0x0AB7 0x0ACD 0x0AA0 0x0AB0 0x0ACD"))); // 0x1012A
        gGujaratiLigMap.insert(std::make_pair( 0xEBFE, dvngLig( 390 ,"0x0AB8 0x0ACD 0x0AA4 0x0ACD 0x0AB0"))); // 0x1012B
        gGujaratiLigMap.insert(std::make_pair( 0xEBFF, dvngLig( 390 ,"0x0AB8 0x0ACD 0x0AA4 0x0AB0 0x0ACD"))); // 0x1012B
        gGujaratiLigMap.insert(std::make_pair( 0xEC00, dvngLig( 391 ,"0x0AB9 0x0ACD 0x0AA3"))); // 0x1012C
        gGujaratiLigMap.insert(std::make_pair( 0xEC01, dvngLig( 392 ,"0x0AB9 0x0ACD 0x0AA8"))); // 0x1012D
        gGujaratiLigMap.insert(std::make_pair( 0xEC02, dvngLig( 393 ,"0x0AB9 0x0ACD 0x0AAE"))); // 0x1012E
        gGujaratiLigMap.insert(std::make_pair( 0xEC03, dvngLig( 394 ,"0x0AB9 0x0ACD 0x0AAF"))); // 0x1012F
        gGujaratiLigMap.insert(std::make_pair( 0xEC04, dvngLig( 395 ,"0x0AB9 0x0ACD 0x0AB2"))); // 0x10130
        gGujaratiLigMap.insert(std::make_pair( 0xEC05, dvngLig( 396 ,"0x0AB9 0x0ACD 0x0AB5"))); // 0x10131
        gGujaratiLigMap.insert(std::make_pair( 0xEC06, dvngLig( 397 ,"0x0AA4 0x0ACD 0x0AA4 0x0ACD"))); // 0x10132
        gGujaratiLigMap.insert(std::make_pair( 0xEC07, dvngLig( 403 ,"0x0AC0 0x0A82"))); // 0x10138
        gGujaratiLigMap.insert(std::make_pair( 0xEC08, dvngLig( 403 ,"0x0AC0 0x0A81"))); // 0x10138
        //gGujaratiLigMap.insert(std::make_pair( 0xEC09, dvngLig( 404 ,"0x0AC0 0x0AB0 0x0ACD"))); // 0x10139
        gGujaratiLigMap.insert(std::make_pair( 0xEC0A, dvngLig( 405 ,"0x0AC0 0x0AB0 0x0ACD 0x0A82"))); // 0x1013A
        gGujaratiLigMap.insert(std::make_pair( 0xEC0B, dvngLig( 405 ,"0x0AC0 0x0AB0 0x0ACD 0x0A81"))); // 0x1013A
        gGujaratiLigMap.insert(std::make_pair( 0xEC0C, dvngLig( 406 ,"0x0AC5 0x0A82"))); // 0x1013B
        gGujaratiLigMap.insert(std::make_pair( 0xEC0D, dvngLig( 406 ,"0x0AC5 0x0A81"))); // 0x1013B
        gGujaratiLigMap.insert(std::make_pair( 0xEC0E, dvngLig( 407 ,"0x0AC5 0x0AB0 0x0ACD"))); // 0x1013C
        gGujaratiLigMap.insert(std::make_pair( 0xEC0F, dvngLig( 408 ,"0x0AC5 0x0AB0 0x0ACD 0x0A82"))); // 0x1013D
        gGujaratiLigMap.insert(std::make_pair( 0xEC10, dvngLig( 408 ,"0x0AC5 0x0AB0 0x0ACD 0x0A81"))); // 0x1013D
        gGujaratiLigMap.insert(std::make_pair( 0xEC11, dvngLig( 409 ,"0x0AC7 0x0A82"))); // 0x1013E
        gGujaratiLigMap.insert(std::make_pair( 0xEC12, dvngLig( 409 ,"0x0AC7 0x0A81"))); // 0x1013E
        gGujaratiLigMap.insert(std::make_pair( 0xEC13, dvngLig( 410 ,"0x0AC7 0x0AB0 0x0ACD"))); // 0x1013F
        gGujaratiLigMap.insert(std::make_pair( 0xEC14, dvngLig( 411 ,"0x0AC7 0x0AB0 0x0ACD 0x0A82"))); // 0x10140
        gGujaratiLigMap.insert(std::make_pair( 0xEC15, dvngLig( 411 ,"0x0AC7 0x0AB0 0x0ACD 0x0A81"))); // 0x10140
        gGujaratiLigMap.insert(std::make_pair( 0xEC16, dvngLig( 412 ,"0x0AC8 0x0A82"))); // 0x10141
        gGujaratiLigMap.insert(std::make_pair( 0xEC17, dvngLig( 412 ,"0x0AC8 0x0A81"))); // 0x10141
        gGujaratiLigMap.insert(std::make_pair( 0xEC18, dvngLig( 413 ,"0x0AC8 0x0AB0 0x0ACD"))); // 0x10142
        gGujaratiLigMap.insert(std::make_pair( 0xEC19, dvngLig( 414 ,"0x0AC8 0x0AB0 0x0ACD 0x0A82"))); // 0x10143
        gGujaratiLigMap.insert(std::make_pair( 0xEC1A, dvngLig( 414 ,"0x0AC8 0x0AB0 0x0ACD 0x0A81"))); // 0x10143
        gGujaratiLigMap.insert(std::make_pair( 0xEC1B, dvngLig( 415 ,"0x0AC9 0x0A82"))); // 0x10144
        gGujaratiLigMap.insert(std::make_pair( 0xEC1C, dvngLig( 415 ,"0x0AC9 0x0A81"))); // 0x10144
        gGujaratiLigMap.insert(std::make_pair( 0xEC1D, dvngLig( 416 ,"0x0AC9 0x0AB0 0x0ACD"))); // 0x10145
        gGujaratiLigMap.insert(std::make_pair( 0xEC1E, dvngLig( 417 ,"0x0AC9 0x0AB0 0x0ACD 0x0A82"))); // 0x10146
        gGujaratiLigMap.insert(std::make_pair( 0xEC1F, dvngLig( 417 ,"0x0AC9 0x0AB0 0x0ACD 0x0A81"))); // 0x10146
        gGujaratiLigMap.insert(std::make_pair( 0xEC20, dvngLig( 418 ,"0x0ACB 0x0A82"))); // 0x10147
        gGujaratiLigMap.insert(std::make_pair( 0xEC21, dvngLig( 418 ,"0x0ACB 0x0A81"))); // 0x10147
        gGujaratiLigMap.insert(std::make_pair( 0xEC22, dvngLig( 419 ,"0x0ACB 0x0AB0 0x0ACD"))); // 0x10148
        gGujaratiLigMap.insert(std::make_pair( 0xEC23, dvngLig( 420 ,"0x0ACB 0x0AB0 0x0ACD 0x0A82"))); // 0x10149
        gGujaratiLigMap.insert(std::make_pair( 0xEC24, dvngLig( 420 ,"0x0ACB 0x0AB0 0x0ACD 0x0A81"))); // 0x10149
        gGujaratiLigMap.insert(std::make_pair( 0xEC25, dvngLig( 421 ,"0x0ACC 0x0A82"))); // 0x1014A
        gGujaratiLigMap.insert(std::make_pair( 0xEC26, dvngLig( 421 ,"0x0ACC 0x0A81"))); // 0x1014A
        gGujaratiLigMap.insert(std::make_pair( 0xEC27, dvngLig( 422 ,"0x0ACC 0x0AB0 0x0ACD"))); // 0x1014B
        gGujaratiLigMap.insert(std::make_pair( 0xEC28, dvngLig( 423 ,"0x0ACC 0x0AB0 0x0ACD 0x0A82"))); // 0x1014C
        gGujaratiLigMap.insert(std::make_pair( 0xEC29, dvngLig( 423 ,"0x0ACC 0x0AB0 0x0ACD 0x0A81"))); // 0x1014C
        gGujaratiLigMap.insert(std::make_pair( 0xEC2A, dvngLig( 424 ,"0x0AB0 0x0ACD 0x0A82"))); // 0x1014D
        gGujaratiLigMap.insert(std::make_pair( 0xEC2B, dvngLig( 424 ,"0x0AB0 0x0ACD 0x0A81"))); // 0x1014D
        gGujaratiLigMap.insert(std::make_pair( 0xEC2C, dvngLig( 425 ,"0x0A89 0x0A82"))); // 0x1014E
        gGujaratiLigMap.insert(std::make_pair( 0xEC2D, dvngLig( 425 ,"0x0A89 0x0A81"))); // 0x1014E
        gGujaratiLigMap.insert(std::make_pair( 0xEC2E, dvngLig( 426 ,"0x0A8A 0x0A82"))); // 0x1014F
        gGujaratiLigMap.insert(std::make_pair( 0xEC2F, dvngLig( 426 ,"0x0A8A 0x0A81"))); // 0x1014F
        gGujaratiLigMap.insert(std::make_pair( 0xEC30, dvngLig( 427 ,"0x0A87 0x0A82"))); // 0x10150
        gGujaratiLigMap.insert(std::make_pair( 0xEC31, dvngLig( 427 ,"0x0A87 0x0A81"))); // 0x10150
        gGujaratiLigMap.insert(std::make_pair( 0xEC32, dvngLig( 428 ,"0x0A88 0x0A82"))); // 0x10151
        gGujaratiLigMap.insert(std::make_pair( 0xEC33, dvngLig( 428 ,"0x0A88 0x0A81"))); // 0x10151
        gGujaratiLigMap.insert(std::make_pair( 0xEC34, dvngLig( 429 ,"0x0A8D 0x0A82"))); // 0x10152
        gGujaratiLigMap.insert(std::make_pair( 0xEC35, dvngLig( 429 ,"0x0A8D 0x0A81"))); // 0x10152
        gGujaratiLigMap.insert(std::make_pair( 0xEC36, dvngLig( 430 ,"0x0A8F 0x0A82"))); // 0x10153
        gGujaratiLigMap.insert(std::make_pair( 0xEC37, dvngLig( 430 ,"0x0A8F 0x0A81"))); // 0x10153
        gGujaratiLigMap.insert(std::make_pair( 0xEC38, dvngLig( 431 ,"0x0A90 0x0A82"))); // 0x10154
        gGujaratiLigMap.insert(std::make_pair( 0xEC39, dvngLig( 431 ,"0x0A90 0x0A81"))); // 0x10154
        gGujaratiLigMap.insert(std::make_pair( 0xEC3A, dvngLig( 432 ,"0x0A91 0x0A82"))); // 0x10155
        gGujaratiLigMap.insert(std::make_pair( 0xEC3B, dvngLig( 432 ,"0x0A91 0x0A81"))); // 0x10155
        gGujaratiLigMap.insert(std::make_pair( 0xEC3C, dvngLig( 433 ,"0x0A93 0x0A82"))); // 0x10156
        gGujaratiLigMap.insert(std::make_pair( 0xEC3D, dvngLig( 433 ,"0x0A93 0x0A81"))); // 0x10156
        gGujaratiLigMap.insert(std::make_pair( 0xEC3E, dvngLig( 434 ,"0x0A94 0x0A82"))); // 0x10157
        gGujaratiLigMap.insert(std::make_pair( 0xEC3F, dvngLig( 434 ,"0x0A94 0x0A81"))); // 0x10157
        gGujaratiLigMap.insert(std::make_pair( 0xEC40, dvngLig( 435 ,"0x0A9C 0x0ABE"))); // 0x10158
        gGujaratiLigMap.insert(std::make_pair( 0xEC41, dvngLig( 436 ,"0x0A9C 0x0AC0"))); // 0x10159
        gGujaratiLigMap.insert(std::make_pair( 0xEC42, dvngLig( 437 ,"0x0AA3 0x0AC1"))); // 0x1015A
        gGujaratiLigMap.insert(std::make_pair( 0xEC43, dvngLig( 438 ,"0x0AA3 0x0ACD 0x0AB0 0x0AC1"))); // 0x1015B
        gGujaratiLigMap.insert(std::make_pair( 0xEC44, dvngLig( 438 ,"0x0AA3 0x0AB0 0x0ACD 0x0AC1"))); // 0x1015B
        gGujaratiLigMap.insert(std::make_pair( 0xEC45, dvngLig( 439 ,"0x0AA6 0x0AC3"))); // 0x1015C
        gGujaratiLigMap.insert(std::make_pair( 0xEC46, dvngLig( 440 ,"0x0AB0 0x0AC1"))); // 0x1015D
        gGujaratiLigMap.insert(std::make_pair( 0xEC47, dvngLig( 441 ,"0x0AB0 0x0AC2"))); // 0x1015E
        gGujaratiLigMap.insert(std::make_pair( 0xEC48, dvngLig( 442 ,"0x0AB9 0x0AC3"))); // 0x1015F
        gGujaratiLigMap.insert(std::make_pair( 0xEC49, dvngLig( 445 ,"0x0A9C 0x0ABC 0x0ABE"))); // 0x10162
        gGujaratiLigMap.insert(std::make_pair( 0xEC4A, dvngLig( 446 ,"0x0A9C 0x0ABC 0x0AC0"))); // 0x10163
        gGujaratiLigMap.insert(std::make_pair( 0xEC4B, dvngLig( 447 ,"0x0AA3 0x0ABC 0x0AC1"))); // 0x10164
        gGujaratiLigMap.insert(std::make_pair( 0xEC4C, dvngLig( 448 ,"0x0AA3 0x0ABC 0x0ACD 0x0AB0 0x0AC1"))); // 0x10165
        gGujaratiLigMap.insert(std::make_pair( 0xEC4D, dvngLig( 448 ,"0x0AA3 0x0ABC 0x0AB0 0x0ACD 0x0AC1"))); // 0x10165
        gGujaratiLigMap.insert(std::make_pair( 0xEC4E, dvngLig( 449 ,"0x0AA6 0x0ABC 0x0AC3"))); // 0x10166
        gGujaratiLigMap.insert(std::make_pair( 0xEC4F, dvngLig( 450 ,"0x0AB0 0x0ABC 0x0AC1"))); // 0x10167
        gGujaratiLigMap.insert(std::make_pair( 0xEC50, dvngLig( 451 ,"0x0AB0 0x0ABC 0x0AC2"))); // 0x10168
        gGujaratiLigMap.insert(std::make_pair( 0xEC51, dvngLig( 452 ,"0x0AB9 0x0ABC 0x0AC3"))); // 0x10169
        gGujaratiLigMap.insert(std::make_pair( 0xEC52, dvngLig( 600 ,"0x0ACD 0x0AB0 0x0AC1"))); // 0x101FD
        gGujaratiLigMap.insert(std::make_pair( 0xEC53, dvngLig( 600 ,"0x0AB0 0x0ACD 0x0AC1"))); // 0x101FD
        gGujaratiLigMap.insert(std::make_pair( 0xEC54, dvngLig( 601 ,"0x0ABC 0x0ACD 0x0AB0 0x0AC1"))); // 0x101FE
        gGujaratiLigMap.insert(std::make_pair( 0xEC55, dvngLig( 601 ,"0x0ABC 0x0AB0 0x0ACD 0x0AC1"))); // 0x101FE
        gGujaratiLigMap.insert(std::make_pair( 0xEC56, dvngLig( 602 ,"0x0ACD 0x0AB0 0x0AC2"))); // 0x101FF
        gGujaratiLigMap.insert(std::make_pair( 0xEC57, dvngLig( 602 ,"0x0AB0 0x0ACD 0x0AC2"))); // 0x101FF
        gGujaratiLigMap.insert(std::make_pair( 0xEC58, dvngLig( 603 ,"0x0ABC 0x0ACD 0x0AB0 0x0AC2"))); // 0x10200
        gGujaratiLigMap.insert(std::make_pair( 0xEC59, dvngLig( 603 ,"0x0ABC 0x0AB0 0x0ACD 0x0AC2"))); // 0x10200
        gGujaratiLigMap.insert(std::make_pair( 0xEC5A, dvngLig( 604 ,"0x0ACD 0x0AB0 0x0AC3"))); // 0x10201
        gGujaratiLigMap.insert(std::make_pair( 0xEC5B, dvngLig( 604 ,"0x0AB0 0x0ACD 0x0AC3"))); // 0x10201
        gGujaratiLigMap.insert(std::make_pair( 0xEC5C, dvngLig( 605 ,"0x0ABC 0x0ACD 0x0AB0 0x0AC3"))); // 0x10202
        gGujaratiLigMap.insert(std::make_pair( 0xEC5D, dvngLig( 605 ,"0x0ABC 0x0AB0 0x0ACD 0x0AC3"))); // 0x10202
        gGujaratiLigMap.insert(std::make_pair( 0xEC5E, dvngLig( 606 ,"0x0ACD 0x0AB0 0x0AC4"))); // 0x10203
        gGujaratiLigMap.insert(std::make_pair( 0xEC5F, dvngLig( 606 ,"0x0AB0 0x0ACD 0x0AC4"))); // 0x10203
        gGujaratiLigMap.insert(std::make_pair( 0xEC60, dvngLig( 607 ,"0x0ACD 0x0AB0 0x0AE2"))); // 0x10204
        gGujaratiLigMap.insert(std::make_pair( 0xEC61, dvngLig( 607 ,"0x0AB0 0x0ACD 0x0AE2"))); // 0x10204
        gGujaratiLigMap.insert(std::make_pair( 0xEC62, dvngLig( 608 ,"0x0ACD 0x0AB0 0x0AE3"))); // 0x10205
        gGujaratiLigMap.insert(std::make_pair( 0xEC63, dvngLig( 608 ,"0x0AB0 0x0ACD 0x0AE3"))); // 0x10205
        gGujaratiLigMap.insert(std::make_pair( 0xEC64, dvngLig( 609 ,"0x0ACD 0x0AB0 0x0ACD"))); // 0x10206
        gGujaratiLigMap.insert(std::make_pair( 0xEC65, dvngLig( 609 ,"0x0AB0 0x0ACD 0x0ACD"))); // 0x10206
        gGujaratiLigMap.insert(std::make_pair( 0xEC66, dvngLig( 610 ,"0x0ABC 0x0ACD 0x0AB0 0x0ACD"))); // 0x10207
        gGujaratiLigMap.insert(std::make_pair( 0xEC67, dvngLig( 610 ,"0x0ABC 0x0AB0 0x0ACD 0x0ACD"))); // 0x10207
        gGujaratiLigMap.insert(std::make_pair( 0xEC68, dvngLig( 649 ,"0x0ABC 0x0AC1"))); // 0x1022E
        gGujaratiLigMap.insert(std::make_pair( 0xEC69, dvngLig( 650 ,"0x0ABC 0x0AC2"))); // 0x1022F
        gGujaratiLigMap.insert(std::make_pair( 0xEC6A, dvngLig( 651 ,"0x0ABC 0x0AC3"))); // 0x10230
        gGujaratiLigMap.insert(std::make_pair( 0xEC6B, dvngLig( 652 ,"0x0ABC 0x0AC4"))); // 0x10231
        gGujaratiLigMap.insert(std::make_pair( 0xEC6C, dvngLig( 653 ,"0x0ABC 0x0AE2"))); // 0x10232
        gGujaratiLigMap.insert(std::make_pair( 0xEC6D, dvngLig( 654 ,"0x0ABC 0x0AE3"))); // 0x10233
        gGujaratiLigMap.insert(std::make_pair( 0xEC6E, dvngLig( 655 ,"0x0ABC 0x0AC1"))); // 0x10234
        gGujaratiLigMap.insert(std::make_pair( 0xEC6F, dvngLig( 656 ,"0x0ABC 0x0AC2"))); // 0x10235
        gGujaratiLigMap.insert(std::make_pair( 0xEC70, dvngLig( 657 ,"0x0ABC 0x0AC3"))); // 0x10236
        gGujaratiLigMap.insert(std::make_pair( 0xEC71, dvngLig( 658 ,"0x0ABC 0x0AC4"))); // 0x10237
        gGujaratiLigMap.insert(std::make_pair( 0xEC72, dvngLig( 659 ,"0x0ABC 0x0AE2"))); // 0x10238
        gGujaratiLigMap.insert(std::make_pair( 0xEC73, dvngLig( 660 ,"0x0ABC 0x0AE3"))); // 0x10239
        gGujaratiLigMap.insert(std::make_pair( 0xEC74, dvngLig( 667 ,"0x0ABC 0x0ACD"))); // 0x10240
        gGujaratiLigMap.insert(std::make_pair( 0xEC75, dvngLig( 668 ,"0x0A95 0x0ABC 0x0AC1"))); // 0x10241
        gGujaratiLigMap.insert(std::make_pair( 0xEC76, dvngLig( 669 ,"0x0AA1 0x0ABC 0x0AC1"))); // 0x10242
        gGujaratiLigMap.insert(std::make_pair( 0xEC77, dvngLig( 670 ,"0x0AA2 0x0ABC 0x0AC1"))); // 0x10243
        gGujaratiLigMap.insert(std::make_pair( 0xEC78, dvngLig( 671 ,"0x0A95 0x0ABC 0x0ACD 0x0AB0 0x0AC1"))); // 0x10244
        gGujaratiLigMap.insert(std::make_pair( 0xEC79, dvngLig( 671 ,"0x0A95 0x0ABC 0x0AB0 0x0ACD 0x0AC1"))); // 0x10244
        gGujaratiLigMap.insert(std::make_pair( 0xEC7A, dvngLig( 672 ,"0x0A95 0x0ABC 0x0AC2"))); // 0x10245
        gGujaratiLigMap.insert(std::make_pair( 0xEC7B, dvngLig( 673 ,"0x0AA1 0x0ABC 0x0AC2"))); // 0x10246
        gGujaratiLigMap.insert(std::make_pair( 0xEC7C, dvngLig( 674 ,"0x0AA2 0x0ABC 0x0AC2"))); // 0x10247
        gGujaratiLigMap.insert(std::make_pair( 0xEC7D, dvngLig( 675 ,"0x0A95 0x0ABC 0x0ACD 0x0AB0 0x0AC2"))); // 0x10248
        gGujaratiLigMap.insert(std::make_pair( 0xEC7E, dvngLig( 675 ,"0x0A95 0x0ABC 0x0AB0 0x0ACD 0x0AC2"))); // 0x10248


    }
    return gGujaratiLigMap;
}


void SwitchGujaratiI(lString16* str)
{
/*
 * ( ? I )             -> ( I ? )
 * ( 0xEA3A 0x0AB8 I ) -> ( I 0xEA3A 0x0AB8 )
 * ( 0xEA27 0x0AA4 I ) -> ( I 0xEA27 0x0AA4 )
 */
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0ABF)
        {
            if(( i > 1) && (( str->at(i - 2) == 0xEA3A && str->at(i - 1) == 0x0AB8 ) ||
                            ( str->at(i - 2) == 0xEA27 && str->at(i - 1) == 0x0AA4 ) ))
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x0ABF;
            }
            else
            {
                //default
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x0ABF;
            }
        }
    }
}

void SwitchGujaratiI_reverse(lString16* str)
{
/*
 *  ( I ? )             -> ( ? I )
 *  ( I 0xEA3A 0x0AB8 ) -> ( 0xEA3A 0x0AB8 I )
 *  ( I 0xEA27 0x0AA4 ) -> ( 0xEA27 0x0AA4 I )

*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0ABF)
        {
            if (i < str->length() - 3)
            {
                if ( (str->at(i + 1) == 0xEA3A && str->at(i + 2) == 0x0AB8) ||
                     (str->at(i + 1) == 0xEA27 && str->at(i + 2) == 0x0AA4) )
                {
                    str->at(i + 0) = str->at(i + 1);
                    str->at(i + 1) = str->at(i + 2);
                    str->at(i + 2) = 0x0ABF;
                }
            }
            else
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x0ABF;
            }
        }
    }
}

void SwitchGujaratiRaVirama(lString16* str)
{
/*
 * RV = 0xEA24
 * ( RV EA42 ?)    -> ( EA42 ? RV )
 * ( RV 0AB5 0ABE) -> ( 0AB5 0ABE RV )
 * ( RV ? )        -> ( ? RV )
 */
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i-1) == 0xEA24)
        {
            if( str->at(i) == 0xEA42)
            {
                str->at(i - 1) = str->at(i);
                str->at(i - 0) = str->at(i + 1);
                str->at(i + 1) = 0xEA24;
                i+=2;
            }
            else if(str->at(i) == 0x0AB5 && str->at(i+1) == 0x0ABE )
            {
                str->at(i - 1) = str->at(i + 0);
                str->at(i - 0) = str->at(i + 1);
                str->at(i + 1) = 0x0EA24;
                i+=2;
            }
            else
            {
                //default
                str->at(i - 1) = str->at(i + 0);
                str->at(i + 0) = 0xEA24;
                i++;
            }
        }
    }
}

void SwitchGujaratiRaVirama_reverse(lString16* str)
{
/*
 * RV = 0xEA24
 * ( EA42 ? RV )    -> ( RV EA42 ?)
 * ( 0AB5 0ABE RV ) -> ( RV 0AB5 0ABE)
 * ( ? RV )         -> ( RV ? )
 */
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 0; i < str->length() - 1; i++)
    {
        if (str->at(i + 1) == 0xEA24)
        {
            if (str->at(i - 1) == 0xEA42)
            {
                str->at(i + 1) = str->at(i);
                str->at(i - 0) = 0xEA42;
                str->at(i - 1) = 0xEA24;
            }
            else if (str->at(i - 1) == 0x0AB5 && str->at(i) == 0x0ABE)
            {
                str->at(i - 1) = 0xEA24;
                str->at(i - 0) = 0x0AB5;
                str->at(i + 1) = 0x0ABE;
            }
            else
            {
                //default
                str->at(i + 1) = str->at(i + 0);
                str->at(i + 0) = 0xEA24;
            }
        }
    }
}

#if 0
void SwitchGujaratiEE(lString16 *str)
{
/*    ( ? 0xE271 E)   -> ( E ? 0xE271)
 *    ( ? 0xE272 E)   -> ( E ? 0xE272)
 *    ( ? 0xE273 E)   -> ( E ? 0xE273)
 *    ( ? EE )        -> ( EE ? )
 */
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0BC7)
        {
            //if( str->at(i - 1) == 0xE271 ||  //ya postform
            //    str->at(i - 1) == 0xE272 ||  //ya postform
            //    str->at(i - 1) == 0xE273 ||  //ya postform
            //    str->at(i - 1) == 0xE225 ||  //reph
            //    str->at(i - 1) == 0xE226   ) //reph
            //{
            //    str->at(i - 0) = str->at(i - 1);
            //    str->at(i - 1) = str->at(i - 2);
            //    str->at(i - 2) = 0x09C7;
            //}
            //else
            {
                //default
                str->at(i + 0) = str->at(i - 1);
                str->at(i - 1) = 0x0BC7;
            }
        }
    }
}

void SwitchGujaratiAI(lString16* str)
{
/*
 *  (? AI)   -> (AI ?)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0BC8)
        {
            str->at(i) = str->at(i - 1);
            str->at(i - 1) = 0x0BC8;
        }
    }
}


void SwitchGujaratiEE_reverse(lString16* str)
{
/*
 *  (ee ?)   -> (? ee)
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0BC7)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0x0BC7;
        }
    }
}

void SwitchGujaratiAI_reverse(lString16* str)
{
/*
 *  (ai ?)   -> (? ai)
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0BC8)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0x0BC8;
        }
    }
}

void SwitchGujaratiO(lString16 *str)
{
/* //0x0BCA
 * (? O)   -> (e ? aa)
 * (? ' O) -> (e ? ' aa) // ' == 0xE226 //ra virama
 * (? J O) -> (e ? J aa) // J == 0xE271 //ya postform
 * (? j O) -> (e ? j aa) // j == 0xE272 //ya postform
 * (? i O) -> (e ? i aa) // i == 0xE273 //ya postform
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0BCA)
        {
            /*if (i > 1 && (str->at(i - 1) == 0xE226 ||
                          str->at(i - 1) == 0xE271 ||
                          str->at(i - 1) == 0xE272 ||
                          str->at(i - 1) == 0xE273) )
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x09C7;
                str->insert(i + 1, 1, 0x09BE);
            }
            else*/
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x0BC6;
                str->insert(i + 1, 1, 0x0BBE);
            }
        }
    }
}

void SwitchGujaratiOO(lString16 *str)
{
/* //0x0D4B
 * (? O)   -> (e ? aa)
 * (? ' O) -> (e ? ' aa) // ' == 0xE226 //ra virama
 * (? J O) -> (e ? J aa) // J == 0xE271 //ya postform
 * (? j O) -> (e ? j aa) // j == 0xE272 //ya postform
 * (? i O) -> (e ? i aa) // i == 0xE273 //ya postform
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0BCB)
        {
            /*if (i > 1 && (str->at(i - 1) == 0xE226 ||
                          str->at(i - 1) == 0xE271 ||
                          str->at(i - 1) == 0xE272 ||
                          str->at(i - 1) == 0xE273) )
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x09C7;
                str->insert(i + 1, 1, 0x09BE);
            }
            else*/
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x0BC7;
                str->insert(i + 1, 1, 0x0BBE);
            }
        }
    }
}

void SwitchGujaratiAU(lString16 *str)
{
/* //0x0D4C
 * (? O)   -> (e ? aa)
 * (? ' O) -> (e ? ' aa) // ' == 0xE226 //ra virama
 * (? J O) -> (e ? J aa) // J == 0xE271 //ya postform
 * (? j O) -> (e ? j aa) // j == 0xE272 //ya postform
 * (? i O) -> (e ? i aa) // i == 0xE273 //ya postform
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0BCC)
        {
            /*if (i > 1 && (str->at(i - 1) == 0xE226 ||
                          str->at(i - 1) == 0xE271 ||
                          str->at(i - 1) == 0xE272 ||
                          str->at(i - 1) == 0xE273) )
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x09C7;
                str->insert(i + 1, 1, 0x09BE);
            }
            else*/
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x0BC6;
                str->insert(i + 1, 1, 0x0BD7);
            }
        }
    }
}

void SwitchGujaratiO_reverse(lString16* str)
{
/*
 *  (e ? aa)   -> (? O)
 *  (e ? ' aa) -> (? ' O)  // ' == 0xE226 //ra virama
 *  (e ? J aa) -> (? J O)  // J == 0xE271 //ya postform
 *  (e ? j aa) -> (? j O)  // j == 0xE272 //ya postform
 *  (e ? i aa) -> (? i O)  // i == 0xE273 //ya postform
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0BC6)
        {
            /*if ( i < str->length()-3    &&
                 str->at(i+3) == 0x09BE && (str->at(i + 2) == 0xE226 ||
                                            str->at(i + 2) == 0xE271 ||
                                            str->at(i + 2) == 0xE272 ||
                                            str->at(i + 2) == 0xE273 ))
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = 0x09CB;
                str->erase(i + 3);
                continue;
            }
             */
            if (str->at(i + 2) == 0x0BBE)
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x0BCA;
                str->erase(i + 2);
            }
        }
    }
}

void SwitchGujaratiOO_reverse(lString16* str)
{
/*
 *  (e ? aa)   -> (? O)
 *  (e ? ' aa) -> (? ' O)  // ' == 0xE226 //ra virama
 *  (e ? J aa) -> (? J O)  // J == 0xE271 //ya postform
 *  (e ? j aa) -> (? j O)  // j == 0xE272 //ya postform
 *  (e ? i aa) -> (? i O)  // i == 0xE273 //ya postform
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0BC7)
        {
            /*if ( i < str->length()-3    &&
                 str->at(i+3) == 0x09BE && (str->at(i + 2) == 0xE226 ||
                                            str->at(i + 2) == 0xE271 ||
                                            str->at(i + 2) == 0xE272 ||
                                            str->at(i + 2) == 0xE273 ))
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = 0x09CB;
                str->erase(i + 3);
                continue;
            }
             */
            if (str->at(i + 2) == 0x0BBE)
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x0BCB;
                str->erase(i + 2);
            }
        }
    }
}

void SwitchGujaratiAU_reverse(lString16* str)
{
/*
 *  (e ? aa)   -> (? O)
 *  (e ? ' aa) -> (? ' O)  // ' == 0xE226 //ra virama
 *  (e ? J aa) -> (? J O)  // J == 0xE271 //ya postform
 *  (e ? j aa) -> (? j O)  // j == 0xE272 //ya postform
 *  (e ? i aa) -> (? i O)  // i == 0xE273 //ya postform
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0BC6)
        {
            /*if ( i < str->length()-3    &&
                 str->at(i+3) == 0x09BE && (str->at(i + 2) == 0xE226 ||
                                            str->at(i + 2) == 0xE271 ||
                                            str->at(i + 2) == 0xE272 ||
                                            str->at(i + 2) == 0xE273 ))
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = 0x09CB;
                str->erase(i + 3);
                continue;
            }
             */
            if (str->at(i + 2) == 0x0BD7)
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x0BCC;
                str->erase(i + 2);
            }
        }
    }
}
#endif

lString16 processGujaratiLigatures(lString16 word)
{
    int j = (word.length() >= 7 ) ? 7 : word.length();
    for(;  j >= 2; j--)
    {
        for (int c = word.length() - j; c >= 0; c--)
        {
            lUInt32 fastkey = (word.at(c) << 16) + word.at(c + 1);
            if (gGujaratiFastLigMap.find(fastkey) == gGujaratiFastLigMap.end())
            {
                continue;
            }
            dvngLig lig(word.substr(c, j));
            lChar16 rep = findGujaratiLigRev(lig);

            if (rep != 0)
            {
                word.replace(c, j, lString16(&rep, 1));
                c -= j - 2;
            }
        }
    }
    return word;
}

lString16 lString16::processGujaratiText()
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
        word = processGujaratiLigatures(word);

        SwitchGujaratiRaVirama(&word);
        SwitchGujaratiI(&word);
        //SwitchGujaratiEE(&word);
        //SwitchGujaratiAI(&word);
        //SwitchGujaratiO(&word);
        //SwitchGujaratiOO(&word);
        //SwitchGujaratiAU(&word);

        res.append(word);
        res.append(L" ");
    }
    res.substr(0,res.size()-1);
    return res;
}

lString16 restoreGujaratiWord(lString16 in)
{
    if(GUJARATI_DISPLAY_ENABLE == 0 || gDocumentGujarati == 0)
    {
        return in;
    }

    //SwitchGujaratiAU_reverse(&in);
    //SwitchGujaratiOO_reverse(&in);
    //SwitchGujaratiO_reverse(&in);
    //SwitchGujaratiAI_reverse(&in);
    //SwitchGujaratiEE_reverse(&in);
    //SwitchGujaratiE_reverse(&in);
    SwitchGujaratiI_reverse(&in);
    SwitchGujaratiRaVirama_reverse(&in);

    for (int i = 0; i < in.length(); i++)
    {
        if (in[i] < GUJARATI_START || in[i] > GUJARATI_END)
        {
            continue;
        }

        dvngLig lig = findGujaratiLig(in[i]);
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
    return in;
}
