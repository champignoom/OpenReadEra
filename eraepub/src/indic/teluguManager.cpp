//
// Created by Tarasus on 16.10.2020.
//

#include "include/indic/teluguManager.h"
#include "ore_log.h"
#include <vector>

LigMap     gTeluguLigMap;
LigMapRev  gTeluguLigMapRev;
FastLigMap gTeluguFastLigMap;

bool CharIsTelugu(int ch)
{
    return (ch >= 0x0C00 && ch <= 0x0C7F);
}

bool lString16::CheckTelugu()
{
    if(gDocumentTelugu == 1)
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
        if (CharIsTelugu(ch))
        {
            gDocumentTelugu = 1;
            gDocumentINDIC = 1;

            if(gTeluguLigMapRev.empty())
            {
                gTeluguLigMapRev = TeluguLigMapReversed();
            }
            return true;
        }
    }
    return false;
}

dvngLig findTeluguLig(lChar16 ligature)
{
    if(gTeluguLigMap.empty())
    {
        gTeluguLigMap = GetTeluguLigMap();
    }
    auto it = gTeluguLigMap.find(ligature);
    if(it==gTeluguLigMap.end())
    {
        return dvngLig();
    }
    return it->second;
}

lChar16 findTeluguLigGlyphIndex(lChar16 ligature)
{
    auto it = gTeluguLigMap.find(ligature);
    if(it==gTeluguLigMap.end())
    {
        return 0;
    }
    return it->second.glyphindex;
}

lChar16 findTeluguLigRev(dvngLig combo)
{
    if(gTeluguLigMapRev.empty())
    {
        gTeluguLigMapRev = TeluguLigMapReversed();
    }
    if(combo.len < 2 || combo.len > 10 )
    {
        return 0;
    }
    auto it = gTeluguLigMapRev.find(combo);
    if(it==gTeluguLigMapRev.end())
    {
        return 0;
    }
    //LE("findTeluguLigRev return %d", it->second);
    return it->second;
}

std::map <dvngLig, lChar16, Comparator> TeluguLigMapReversed()
{
    if(!gTeluguLigMapRev.empty())
    {
        return gTeluguLigMapRev;
    }
    if(gTeluguLigMap.empty())
    {
        gTeluguLigMap = GetTeluguLigMap();
    }
    gTeluguLigMapRev = makeReverseLigMap(gTeluguLigMap,&gTeluguFastLigMap);
    return gTeluguLigMapRev;
}

LigMap GetTeluguLigMap()
{
    if(!gTeluguLigMap.empty())
    {
        return gTeluguLigMap;
    }
    //Telugu_START
    {
        //from Noto Sans Telugu.ttf
        gTeluguLigMap.insert(std::make_pair( 0xE790, dvngLig( 101 ,"0x0C15 0x0C4D 0x0C37"))); // 0x10001
        gTeluguLigMap.insert(std::make_pair( 0xE791, dvngLig( 101 ,"0x0C15 0x0C37 0x0C4D"))); // 0x10001
        gTeluguLigMap.insert(std::make_pair( 0xE792, dvngLig( 102 ,"0x0C15 0x0C4D"))); // 0x10002
        gTeluguLigMap.insert(std::make_pair( 0xE793, dvngLig( 102 ,"0x0C15 0x0C4D 0x200D"))); // 0x10002
        gTeluguLigMap.insert(std::make_pair( 0xE794, dvngLig( 103 ,"0x0C16 0x0C4D"))); // 0x10003
        gTeluguLigMap.insert(std::make_pair( 0xE795, dvngLig( 103 ,"0x0C16 0x0C4D 0x200D"))); // 0x10003
        //gTeluguLigMap.insert(std::make_pair( 0xE796, dvngLig( 104 ,"0x0C17 0x0C4D"))); // 0x10004
        gTeluguLigMap.insert(std::make_pair( 0xE797, dvngLig( 104 ,"0x0C17 0x0C4D 0x200D"))); // 0x10004
        gTeluguLigMap.insert(std::make_pair( 0xE798, dvngLig( 105 ,"0x0C18 0x0C4D"))); // 0x10005
        gTeluguLigMap.insert(std::make_pair( 0xE799, dvngLig( 105 ,"0x0C18 0x0C4D 0x200D"))); // 0x10005
        gTeluguLigMap.insert(std::make_pair( 0xE79A, dvngLig( 106 ,"0x0C19 0x0C4D"))); // 0x10006
        gTeluguLigMap.insert(std::make_pair( 0xE79B, dvngLig( 106 ,"0x0C19 0x0C4D 0x200D"))); // 0x10006
        gTeluguLigMap.insert(std::make_pair( 0xE79C, dvngLig( 107 ,"0x0C1A 0x0C4D"))); // 0x10007
        gTeluguLigMap.insert(std::make_pair( 0xE79D, dvngLig( 107 ,"0x0C1A 0x0C4D 0x200D"))); // 0x10007
        gTeluguLigMap.insert(std::make_pair( 0xE79E, dvngLig( 108 ,"0x0C1B 0x0C4D"))); // 0x10008
        gTeluguLigMap.insert(std::make_pair( 0xE79F, dvngLig( 108 ,"0x0C1B 0x0C4D 0x200D"))); // 0x10008
        gTeluguLigMap.insert(std::make_pair( 0xE7A0, dvngLig( 109 ,"0x0C1C 0x0C4D"))); // 0x10009
        gTeluguLigMap.insert(std::make_pair( 0xE7A1, dvngLig( 109 ,"0x0C1C 0x0C4D 0x200D"))); // 0x10009
        gTeluguLigMap.insert(std::make_pair( 0xE7A2, dvngLig( 110 ,"0x0C1D 0x0C4D"))); // 0x1000A
        gTeluguLigMap.insert(std::make_pair( 0xE7A3, dvngLig( 110 ,"0x0C1D 0x0C4D 0x200D"))); // 0x1000A
        gTeluguLigMap.insert(std::make_pair( 0xE7A4, dvngLig( 111 ,"0x0C1E 0x0C4D"))); // 0x1000B
        gTeluguLigMap.insert(std::make_pair( 0xE7A5, dvngLig( 111 ,"0x0C1E 0x0C4D 0x200D"))); // 0x1000B
        gTeluguLigMap.insert(std::make_pair( 0xE7A6, dvngLig( 112 ,"0x0C1F 0x0C4D"))); // 0x1000C
        gTeluguLigMap.insert(std::make_pair( 0xE7A7, dvngLig( 112 ,"0x0C1F 0x0C4D 0x200D"))); // 0x1000C
        gTeluguLigMap.insert(std::make_pair( 0xE7A8, dvngLig( 113 ,"0x0C20 0x0C4D"))); // 0x1000D
        gTeluguLigMap.insert(std::make_pair( 0xE7A9, dvngLig( 113 ,"0x0C20 0x0C4D 0x200D"))); // 0x1000D
        gTeluguLigMap.insert(std::make_pair( 0xE7AA, dvngLig( 114 ,"0x0C21 0x0C4D"))); // 0x1000E
        gTeluguLigMap.insert(std::make_pair( 0xE7AB, dvngLig( 114 ,"0x0C21 0x0C4D 0x200D"))); // 0x1000E
        gTeluguLigMap.insert(std::make_pair( 0xE7AC, dvngLig( 115 ,"0x0C22 0x0C4D"))); // 0x1000F
        gTeluguLigMap.insert(std::make_pair( 0xE7AD, dvngLig( 115 ,"0x0C22 0x0C4D 0x200D"))); // 0x1000F
        gTeluguLigMap.insert(std::make_pair( 0xE7AE, dvngLig( 116 ,"0x0C23 0x0C4D"))); // 0x10010
        gTeluguLigMap.insert(std::make_pair( 0xE7AF, dvngLig( 116 ,"0x0C23 0x0C4D 0x200D"))); // 0x10010
        gTeluguLigMap.insert(std::make_pair( 0xE7B0, dvngLig( 117 ,"0x0C24 0x0C4D"))); // 0x10011
        gTeluguLigMap.insert(std::make_pair( 0xE7B1, dvngLig( 117 ,"0x0C24 0x0C4D 0x200D"))); // 0x10011
        gTeluguLigMap.insert(std::make_pair( 0xE7B2, dvngLig( 118 ,"0x0C25 0x0C4D"))); // 0x10012
        gTeluguLigMap.insert(std::make_pair( 0xE7B3, dvngLig( 118 ,"0x0C25 0x0C4D 0x200D"))); // 0x10012
        gTeluguLigMap.insert(std::make_pair( 0xE7B4, dvngLig( 119 ,"0x0C26 0x0C4D"))); // 0x10013
        gTeluguLigMap.insert(std::make_pair( 0xE7B5, dvngLig( 119 ,"0x0C26 0x0C4D 0x200D"))); // 0x10013
        gTeluguLigMap.insert(std::make_pair( 0xE7B6, dvngLig( 120 ,"0x0C27 0x0C4D"))); // 0x10014
        gTeluguLigMap.insert(std::make_pair( 0xE7B7, dvngLig( 120 ,"0x0C27 0x0C4D 0x200D"))); // 0x10014
        gTeluguLigMap.insert(std::make_pair( 0xE7B8, dvngLig( 121 ,"0x0C28 0x0C4D"))); // 0x10015
        gTeluguLigMap.insert(std::make_pair( 0xE7B9, dvngLig( 121 ,"0x0C28 0x0C4D 0x200D"))); // 0x10015
        gTeluguLigMap.insert(std::make_pair( 0xE7BA, dvngLig( 122 ,"0x0C2A 0x0C4D"))); // 0x10016
        gTeluguLigMap.insert(std::make_pair( 0xE7BB, dvngLig( 122 ,"0x0C2A 0x0C4D 0x200D"))); // 0x10016
        gTeluguLigMap.insert(std::make_pair( 0xE7BC, dvngLig( 123 ,"0x0C2B 0x0C4D"))); // 0x10017
        gTeluguLigMap.insert(std::make_pair( 0xE7BD, dvngLig( 123 ,"0x0C2B 0x0C4D 0x200D"))); // 0x10017
        //gTeluguLigMap.insert(std::make_pair( 0xE7BE, dvngLig( 124 ,"0x0C2C 0x0C4D"))); // 0x10018
        gTeluguLigMap.insert(std::make_pair( 0xE7BF, dvngLig( 124 ,"0x0C2C 0x0C4D 0x200D"))); // 0x10018
        gTeluguLigMap.insert(std::make_pair( 0xE7C0, dvngLig( 125 ,"0x0C2D 0x0C4D"))); // 0x10019
        gTeluguLigMap.insert(std::make_pair( 0xE7C1, dvngLig( 125 ,"0x0C2D 0x0C4D 0x200D"))); // 0x10019
        gTeluguLigMap.insert(std::make_pair( 0xE7C2, dvngLig( 126 ,"0x0C2E 0x0C4D"))); // 0x1001A
        gTeluguLigMap.insert(std::make_pair( 0xE7C3, dvngLig( 126 ,"0x0C2E 0x0C4D 0x200D"))); // 0x1001A
        gTeluguLigMap.insert(std::make_pair( 0xE7C4, dvngLig( 127 ,"0x0C2F 0x0C4D"))); // 0x1001B
        gTeluguLigMap.insert(std::make_pair( 0xE7C5, dvngLig( 127 ,"0x0C2F 0x0C4D 0x200D"))); // 0x1001B
        gTeluguLigMap.insert(std::make_pair( 0xE7C6, dvngLig( 128 ,"0x0C30 0x0C4D"))); // 0x1001C
        gTeluguLigMap.insert(std::make_pair( 0xE7C7, dvngLig( 128 ,"0x0C30 0x0C4D 0x200D"))); // 0x1001C
        gTeluguLigMap.insert(std::make_pair( 0xE7C8, dvngLig( 129 ,"0x0C31 0x0C4D"))); // 0x1001D
        gTeluguLigMap.insert(std::make_pair( 0xE7C9, dvngLig( 129 ,"0x0C31 0x0C4D 0x200D"))); // 0x1001D
        gTeluguLigMap.insert(std::make_pair( 0xE7CA, dvngLig( 130 ,"0x0C32 0x0C4D"))); // 0x1001E
        gTeluguLigMap.insert(std::make_pair( 0xE7CB, dvngLig( 130 ,"0x0C32 0x0C4D 0x200D"))); // 0x1001E
        gTeluguLigMap.insert(std::make_pair( 0xE7CC, dvngLig( 131 ,"0x0C33 0x0C4D"))); // 0x1001F
        gTeluguLigMap.insert(std::make_pair( 0xE7CD, dvngLig( 131 ,"0x0C33 0x0C4D 0x200D"))); // 0x1001F
        gTeluguLigMap.insert(std::make_pair( 0xE7CE, dvngLig( 132 ,"0x0C35 0x0C4D"))); // 0x10020
        gTeluguLigMap.insert(std::make_pair( 0xE7CF, dvngLig( 132 ,"0x0C35 0x0C4D 0x200D"))); // 0x10020
        //gTeluguLigMap.insert(std::make_pair( 0xE7D0, dvngLig( 133 ,"0x0C36 0x0C4D"))); // 0x10021
        gTeluguLigMap.insert(std::make_pair( 0xE7D1, dvngLig( 133 ,"0x0C36 0x0C4D 0x200D"))); // 0x10021
        gTeluguLigMap.insert(std::make_pair( 0xE7D2, dvngLig( 134 ,"0x0C37 0x0C4D"))); // 0x10022
        gTeluguLigMap.insert(std::make_pair( 0xE7D3, dvngLig( 134 ,"0x0C37 0x0C4D 0x200D"))); // 0x10022
        gTeluguLigMap.insert(std::make_pair( 0xE7D4, dvngLig( 135 ,"0x0C38 0x0C4D"))); // 0x10023
        gTeluguLigMap.insert(std::make_pair( 0xE7D5, dvngLig( 135 ,"0x0C38 0x0C4D 0x200D"))); // 0x10023
        gTeluguLigMap.insert(std::make_pair( 0xE7D6, dvngLig( 136 ,"0x0C39 0x0C4D"))); // 0x10024
        gTeluguLigMap.insert(std::make_pair( 0xE7D7, dvngLig( 136 ,"0x0C39 0x0C4D 0x200D"))); // 0x10024
        gTeluguLigMap.insert(std::make_pair( 0xE7D8, dvngLig( 137 ,"0x0C15 0x0C4D 0x0C37 0x0C4D"))); // 0x10025
        gTeluguLigMap.insert(std::make_pair( 0xE7D9, dvngLig( 137 ,"0x0C15 0x0C37 0x0C4D 0x0C4D"))); // 0x10025
        gTeluguLigMap.insert(std::make_pair( 0xE7DA, dvngLig( 137 ,"0x0C15 0x0C4D 0x0C37 0x0C4D 0x200D"))); // 0x10025
        gTeluguLigMap.insert(std::make_pair( 0xE7DB, dvngLig( 137 ,"0x0C15 0x0C37 0x0C4D 0x0C4D 0x200D"))); // 0x10025
        gTeluguLigMap.insert(std::make_pair( 0xE7DC, dvngLig( 138 ,"0x0C15 0x0C3E"))); // 0x10026
        gTeluguLigMap.insert(std::make_pair( 0xE7DD, dvngLig( 139 ,"0x0C16 0x0C3E"))); // 0x10027
        gTeluguLigMap.insert(std::make_pair( 0xE7DE, dvngLig( 140 ,"0x0C17 0x0C3E"))); // 0x10028
        gTeluguLigMap.insert(std::make_pair( 0xE7DF, dvngLig( 141 ,"0x0C18 0x0C3E"))); // 0x10029
        gTeluguLigMap.insert(std::make_pair( 0xE7E0, dvngLig( 142 ,"0x0C19 0x0C3E"))); // 0x1002A
        gTeluguLigMap.insert(std::make_pair( 0xE7E1, dvngLig( 143 ,"0x0C1A 0x0C3E"))); // 0x1002B
        gTeluguLigMap.insert(std::make_pair( 0xE7E2, dvngLig( 144 ,"0x0C1B 0x0C3E"))); // 0x1002C
        gTeluguLigMap.insert(std::make_pair( 0xE7E3, dvngLig( 145 ,"0x0C1C 0x0C3E"))); // 0x1002D
        gTeluguLigMap.insert(std::make_pair( 0xE7E4, dvngLig( 146 ,"0x0C1D 0x0C3E"))); // 0x1002E
        gTeluguLigMap.insert(std::make_pair( 0xE7E5, dvngLig( 147 ,"0x0C1E 0x0C3E"))); // 0x1002F
        gTeluguLigMap.insert(std::make_pair( 0xE7E6, dvngLig( 148 ,"0x0C1F 0x0C3E"))); // 0x10030
        gTeluguLigMap.insert(std::make_pair( 0xE7E7, dvngLig( 149 ,"0x0C20 0x0C3E"))); // 0x10031
        gTeluguLigMap.insert(std::make_pair( 0xE7E8, dvngLig( 150 ,"0x0C21 0x0C3E"))); // 0x10032
        gTeluguLigMap.insert(std::make_pair( 0xE7E9, dvngLig( 151 ,"0x0C22 0x0C3E"))); // 0x10033
        gTeluguLigMap.insert(std::make_pair( 0xE7EA, dvngLig( 152 ,"0x0C23 0x0C3E"))); // 0x10034
        gTeluguLigMap.insert(std::make_pair( 0xE7EB, dvngLig( 153 ,"0x0C24 0x0C3E"))); // 0x10035
        gTeluguLigMap.insert(std::make_pair( 0xE7EC, dvngLig( 154 ,"0x0C25 0x0C3E"))); // 0x10036
        gTeluguLigMap.insert(std::make_pair( 0xE7ED, dvngLig( 155 ,"0x0C26 0x0C3E"))); // 0x10037
        gTeluguLigMap.insert(std::make_pair( 0xE7EE, dvngLig( 156 ,"0x0C27 0x0C3E"))); // 0x10038
        gTeluguLigMap.insert(std::make_pair( 0xE7EF, dvngLig( 157 ,"0x0C28 0x0C3E"))); // 0x10039
        gTeluguLigMap.insert(std::make_pair( 0xE7F0, dvngLig( 158 ,"0x0C2A 0x0C3E"))); // 0x1003A
        gTeluguLigMap.insert(std::make_pair( 0xE7F1, dvngLig( 159 ,"0x0C2B 0x0C3E"))); // 0x1003B
        gTeluguLigMap.insert(std::make_pair( 0xE7F2, dvngLig( 160 ,"0x0C2C 0x0C3E"))); // 0x1003C
        gTeluguLigMap.insert(std::make_pair( 0xE7F3, dvngLig( 161 ,"0x0C2D 0x0C3E"))); // 0x1003D
        gTeluguLigMap.insert(std::make_pair( 0xE7F4, dvngLig( 162 ,"0x0C2E 0x0C3E"))); // 0x1003E
        gTeluguLigMap.insert(std::make_pair( 0xE7F5, dvngLig( 163 ,"0x0C2F 0x0C3E"))); // 0x1003F
        gTeluguLigMap.insert(std::make_pair( 0xE7F6, dvngLig( 164 ,"0x0C30 0x0C3E"))); // 0x10040
        gTeluguLigMap.insert(std::make_pair( 0xE7F7, dvngLig( 165 ,"0x0C31 0x0C3E"))); // 0x10041
        gTeluguLigMap.insert(std::make_pair( 0xE7F8, dvngLig( 166 ,"0x0C32 0x0C3E"))); // 0x10042
        gTeluguLigMap.insert(std::make_pair( 0xE7F9, dvngLig( 167 ,"0x0C33 0x0C3E"))); // 0x10043
        gTeluguLigMap.insert(std::make_pair( 0xE7FA, dvngLig( 168 ,"0x0C35 0x0C3E"))); // 0x10044
        gTeluguLigMap.insert(std::make_pair( 0xE7FB, dvngLig( 169 ,"0x0C36 0x0C3E"))); // 0x10045
        gTeluguLigMap.insert(std::make_pair( 0xE7FC, dvngLig( 170 ,"0x0C37 0x0C3E"))); // 0x10046
        gTeluguLigMap.insert(std::make_pair( 0xE7FD, dvngLig( 171 ,"0x0C38 0x0C3E"))); // 0x10047
        gTeluguLigMap.insert(std::make_pair( 0xE7FE, dvngLig( 172 ,"0x0C39 0x0C3E"))); // 0x10048
        gTeluguLigMap.insert(std::make_pair( 0xE7FF, dvngLig( 173 ,"0x0C15 0x0C4D 0x0C37 0x0C3E"))); // 0x10049
        gTeluguLigMap.insert(std::make_pair( 0xE800, dvngLig( 173 ,"0x0C15 0x0C37 0x0C4D 0x0C3E"))); // 0x10049
        gTeluguLigMap.insert(std::make_pair( 0xE801, dvngLig( 174 ,"0x0C15 0x0C3F"))); // 0x1004A
        gTeluguLigMap.insert(std::make_pair( 0xE802, dvngLig( 175 ,"0x0C16 0x0C3F"))); // 0x1004B
        gTeluguLigMap.insert(std::make_pair( 0xE803, dvngLig( 176 ,"0x0C17 0x0C3F"))); // 0x1004C
        gTeluguLigMap.insert(std::make_pair( 0xE804, dvngLig( 177 ,"0x0C18 0x0C3F"))); // 0x1004D
        gTeluguLigMap.insert(std::make_pair( 0xE805, dvngLig( 178 ,"0x0C19 0x0C3F"))); // 0x1004E
        gTeluguLigMap.insert(std::make_pair( 0xE806, dvngLig( 179 ,"0x0C1A 0x0C3F"))); // 0x1004F
        gTeluguLigMap.insert(std::make_pair( 0xE807, dvngLig( 180 ,"0x0C1B 0x0C3F"))); // 0x10050
        gTeluguLigMap.insert(std::make_pair( 0xE808, dvngLig( 181 ,"0x0C1C 0x0C3F"))); // 0x10051
        gTeluguLigMap.insert(std::make_pair( 0xE809, dvngLig( 182 ,"0x0C1D 0x0C3F"))); // 0x10052
        gTeluguLigMap.insert(std::make_pair( 0xE80A, dvngLig( 183 ,"0x0C1E 0x0C3F"))); // 0x10053
        gTeluguLigMap.insert(std::make_pair( 0xE80B, dvngLig( 184 ,"0x0C1F 0x0C3F"))); // 0x10054
        gTeluguLigMap.insert(std::make_pair( 0xE80C, dvngLig( 185 ,"0x0C20 0x0C3F"))); // 0x10055
        //gTeluguLigMap.insert(std::make_pair( 0xE80D, dvngLig( 186 ,"0x0C21 0x0C3F"))); // 0x10056
        gTeluguLigMap.insert(std::make_pair( 0xE80E, dvngLig( 187 ,"0x0C22 0x0C3F"))); // 0x10057
        gTeluguLigMap.insert(std::make_pair( 0xE80F, dvngLig( 188 ,"0x0C23 0x0C3F"))); // 0x10058
        gTeluguLigMap.insert(std::make_pair( 0xE810, dvngLig( 189 ,"0x0C24 0x0C3F"))); // 0x10059
        gTeluguLigMap.insert(std::make_pair( 0xE811, dvngLig( 190 ,"0x0C25 0x0C3F"))); // 0x1005A
        gTeluguLigMap.insert(std::make_pair( 0xE812, dvngLig( 191 ,"0x0C26 0x0C3F"))); // 0x1005B
        gTeluguLigMap.insert(std::make_pair( 0xE813, dvngLig( 192 ,"0x0C27 0x0C3F"))); // 0x1005C
        //gTeluguLigMap.insert(std::make_pair( 0xE814, dvngLig( 193 ,"0x0C28 0x0C3F"))); // 0x1005D
        gTeluguLigMap.insert(std::make_pair( 0xE815, dvngLig( 194 ,"0x0C2A 0x0C3F"))); // 0x1005E
        gTeluguLigMap.insert(std::make_pair( 0xE816, dvngLig( 195 ,"0x0C2B 0x0C3F"))); // 0x1005F
        //gTeluguLigMap.insert(std::make_pair( 0xE817, dvngLig( 196 ,"0x0C2C 0x0C3F"))); // 0x10060
        gTeluguLigMap.insert(std::make_pair( 0xE818, dvngLig( 197 ,"0x0C2D 0x0C3F"))); // 0x10061
        gTeluguLigMap.insert(std::make_pair( 0xE819, dvngLig( 198 ,"0x0C2E 0x0C3F"))); // 0x10062
        gTeluguLigMap.insert(std::make_pair( 0xE81A, dvngLig( 199 ,"0x0C2F 0x0C3F"))); // 0x10063
        //gTeluguLigMap.insert(std::make_pair( 0xE81B, dvngLig( 200 ,"0x0C30 0x0C3F"))); // 0x10064
        gTeluguLigMap.insert(std::make_pair( 0xE81C, dvngLig( 201 ,"0x0C31 0x0C3F"))); // 0x10065
        //gTeluguLigMap.insert(std::make_pair( 0xE81D, dvngLig( 202 ,"0x0C32 0x0C3F"))); // 0x10066
        gTeluguLigMap.insert(std::make_pair( 0xE81E, dvngLig( 203 ,"0x0C33 0x0C3F"))); // 0x10067
        gTeluguLigMap.insert(std::make_pair( 0xE81F, dvngLig( 204 ,"0x0C35 0x0C3F"))); // 0x10068
        gTeluguLigMap.insert(std::make_pair( 0xE820, dvngLig( 205 ,"0x0C36 0x0C3F"))); // 0x10069
        gTeluguLigMap.insert(std::make_pair( 0xE821, dvngLig( 206 ,"0x0C37 0x0C3F"))); // 0x1006A
        gTeluguLigMap.insert(std::make_pair( 0xE822, dvngLig( 207 ,"0x0C38 0x0C3F"))); // 0x1006B
        gTeluguLigMap.insert(std::make_pair( 0xE823, dvngLig( 208 ,"0x0C39 0x0C3F"))); // 0x1006C
        gTeluguLigMap.insert(std::make_pair( 0xE824, dvngLig( 209 ,"0x0C15 0x0C4D 0x0C37 0x0C3F"))); // 0x1006D
        gTeluguLigMap.insert(std::make_pair( 0xE825, dvngLig( 209 ,"0x0C15 0x0C37 0x0C4D 0x0C3F"))); // 0x1006D
        gTeluguLigMap.insert(std::make_pair( 0xE826, dvngLig( 210 ,"0x0C15 0x0C40"))); // 0x1006E
        gTeluguLigMap.insert(std::make_pair( 0xE827, dvngLig( 211 ,"0x0C16 0x0C40"))); // 0x1006F
        gTeluguLigMap.insert(std::make_pair( 0xE828, dvngLig( 212 ,"0x0C17 0x0C40"))); // 0x10070
        gTeluguLigMap.insert(std::make_pair( 0xE829, dvngLig( 213 ,"0x0C18 0x0C40"))); // 0x10071
        gTeluguLigMap.insert(std::make_pair( 0xE82A, dvngLig( 214 ,"0x0C19 0x0C40"))); // 0x10072
        gTeluguLigMap.insert(std::make_pair( 0xE82B, dvngLig( 215 ,"0x0C1A 0x0C40"))); // 0x10073
        gTeluguLigMap.insert(std::make_pair( 0xE82C, dvngLig( 216 ,"0x0C1B 0x0C40"))); // 0x10074
        gTeluguLigMap.insert(std::make_pair( 0xE82D, dvngLig( 217 ,"0x0C1C 0x0C40"))); // 0x10075
        gTeluguLigMap.insert(std::make_pair( 0xE82E, dvngLig( 218 ,"0x0C1D 0x0C40"))); // 0x10076
        gTeluguLigMap.insert(std::make_pair( 0xE82F, dvngLig( 219 ,"0x0C1E 0x0C40"))); // 0x10077
        gTeluguLigMap.insert(std::make_pair( 0xE830, dvngLig( 220 ,"0x0C1F 0x0C40"))); // 0x10078
        gTeluguLigMap.insert(std::make_pair( 0xE831, dvngLig( 221 ,"0x0C20 0x0C40"))); // 0x10079
        gTeluguLigMap.insert(std::make_pair( 0xE832, dvngLig( 222 ,"0x0C21 0x0C40"))); // 0x1007A
        gTeluguLigMap.insert(std::make_pair( 0xE833, dvngLig( 223 ,"0x0C22 0x0C40"))); // 0x1007B
        gTeluguLigMap.insert(std::make_pair( 0xE834, dvngLig( 224 ,"0x0C23 0x0C40"))); // 0x1007C
        gTeluguLigMap.insert(std::make_pair( 0xE835, dvngLig( 225 ,"0x0C24 0x0C40"))); // 0x1007D
        gTeluguLigMap.insert(std::make_pair( 0xE836, dvngLig( 226 ,"0x0C25 0x0C40"))); // 0x1007E
        gTeluguLigMap.insert(std::make_pair( 0xE837, dvngLig( 227 ,"0x0C26 0x0C40"))); // 0x1007F
        gTeluguLigMap.insert(std::make_pair( 0xE838, dvngLig( 228 ,"0x0C27 0x0C40"))); // 0x10080
        gTeluguLigMap.insert(std::make_pair( 0xE839, dvngLig( 229 ,"0x0C28 0x0C40"))); // 0x10081
        gTeluguLigMap.insert(std::make_pair( 0xE83A, dvngLig( 230 ,"0x0C2A 0x0C40"))); // 0x10082
        gTeluguLigMap.insert(std::make_pair( 0xE83B, dvngLig( 231 ,"0x0C2B 0x0C40"))); // 0x10083
        gTeluguLigMap.insert(std::make_pair( 0xE83C, dvngLig( 232 ,"0x0C2C 0x0C40"))); // 0x10084
        gTeluguLigMap.insert(std::make_pair( 0xE83D, dvngLig( 233 ,"0x0C2D 0x0C40"))); // 0x10085
        //gTeluguLigMap.insert(std::make_pair( 0xE83E, dvngLig( 234 ,"0x0C2E 0x0C40"))); // 0x10086
        gTeluguLigMap.insert(std::make_pair( 0xE83F, dvngLig( 235 ,"0x0C2F 0x0C40"))); // 0x10087
        //gTeluguLigMap.insert(std::make_pair( 0xE840, dvngLig( 236 ,"0x0C30 0x0C40"))); // 0x10088
        gTeluguLigMap.insert(std::make_pair( 0xE841, dvngLig( 237 ,"0x0C31 0x0C40"))); // 0x10089
        gTeluguLigMap.insert(std::make_pair( 0xE842, dvngLig( 238 ,"0x0C32 0x0C40"))); // 0x1008A
        gTeluguLigMap.insert(std::make_pair( 0xE843, dvngLig( 239 ,"0x0C33 0x0C40"))); // 0x1008B
        gTeluguLigMap.insert(std::make_pair( 0xE844, dvngLig( 240 ,"0x0C35 0x0C40"))); // 0x1008C
        gTeluguLigMap.insert(std::make_pair( 0xE845, dvngLig( 241 ,"0x0C36 0x0C40"))); // 0x1008D
        gTeluguLigMap.insert(std::make_pair( 0xE846, dvngLig( 242 ,"0x0C37 0x0C40"))); // 0x1008E
        gTeluguLigMap.insert(std::make_pair( 0xE847, dvngLig( 243 ,"0x0C38 0x0C40"))); // 0x1008F
        gTeluguLigMap.insert(std::make_pair( 0xE848, dvngLig( 244 ,"0x0C39 0x0C40"))); // 0x10090
        gTeluguLigMap.insert(std::make_pair( 0xE849, dvngLig( 245 ,"0x0C15 0x0C4D 0x0C37 0x0C40"))); // 0x10091
        gTeluguLigMap.insert(std::make_pair( 0xE84A, dvngLig( 245 ,"0x0C15 0x0C37 0x0C4D 0x0C40"))); // 0x10091
        gTeluguLigMap.insert(std::make_pair( 0xE84B, dvngLig( 246 ,"0x0C1C 0x0C41"))); // 0x10092
        gTeluguLigMap.insert(std::make_pair( 0xE84C, dvngLig( 247 ,"0x0C2A 0x0C41"))); // 0x10093
        gTeluguLigMap.insert(std::make_pair( 0xE84D, dvngLig( 248 ,"0x0C2B 0x0C41"))); // 0x10094
        gTeluguLigMap.insert(std::make_pair( 0xE84E, dvngLig( 249 ,"0x0C35 0x0C41"))); // 0x10095
        gTeluguLigMap.insert(std::make_pair( 0xE84F, dvngLig( 250 ,"0x0C1C 0x0C42"))); // 0x10096
        gTeluguLigMap.insert(std::make_pair( 0xE850, dvngLig( 251 ,"0x0C2A 0x0C42"))); // 0x10097
        gTeluguLigMap.insert(std::make_pair( 0xE851, dvngLig( 252 ,"0x0C2B 0x0C42"))); // 0x10098
        gTeluguLigMap.insert(std::make_pair( 0xE852, dvngLig( 253 ,"0x0C35 0x0C42"))); // 0x10099
        gTeluguLigMap.insert(std::make_pair( 0xE853, dvngLig( 254 ,"0x0C15 0x0C46"))); // 0x1009A
        gTeluguLigMap.insert(std::make_pair( 0xE854, dvngLig( 255 ,"0x0C16 0x0C46"))); // 0x1009B
        gTeluguLigMap.insert(std::make_pair( 0xE855, dvngLig( 256 ,"0x0C17 0x0C46"))); // 0x1009C
        gTeluguLigMap.insert(std::make_pair( 0xE856, dvngLig( 257 ,"0x0C18 0x0C46"))); // 0x1009D
        gTeluguLigMap.insert(std::make_pair( 0xE857, dvngLig( 258 ,"0x0C19 0x0C46"))); // 0x1009E
        gTeluguLigMap.insert(std::make_pair( 0xE858, dvngLig( 259 ,"0x0C1A 0x0C46"))); // 0x1009F
        gTeluguLigMap.insert(std::make_pair( 0xE859, dvngLig( 260 ,"0x0C1B 0x0C46"))); // 0x100A0
        gTeluguLigMap.insert(std::make_pair( 0xE85A, dvngLig( 261 ,"0x0C1C 0x0C46"))); // 0x100A1
        gTeluguLigMap.insert(std::make_pair( 0xE85B, dvngLig( 262 ,"0x0C1D 0x0C46"))); // 0x100A2
        gTeluguLigMap.insert(std::make_pair( 0xE85C, dvngLig( 263 ,"0x0C1E 0x0C46"))); // 0x100A3
        gTeluguLigMap.insert(std::make_pair( 0xE85D, dvngLig( 264 ,"0x0C1F 0x0C46"))); // 0x100A4
        gTeluguLigMap.insert(std::make_pair( 0xE85E, dvngLig( 265 ,"0x0C20 0x0C46"))); // 0x100A5
        gTeluguLigMap.insert(std::make_pair( 0xE85F, dvngLig( 266 ,"0x0C21 0x0C46"))); // 0x100A6
        gTeluguLigMap.insert(std::make_pair( 0xE860, dvngLig( 267 ,"0x0C22 0x0C46"))); // 0x100A7
        gTeluguLigMap.insert(std::make_pair( 0xE861, dvngLig( 268 ,"0x0C23 0x0C46"))); // 0x100A8
        gTeluguLigMap.insert(std::make_pair( 0xE862, dvngLig( 269 ,"0x0C24 0x0C46"))); // 0x100A9
        gTeluguLigMap.insert(std::make_pair( 0xE863, dvngLig( 270 ,"0x0C25 0x0C46"))); // 0x100AA
        gTeluguLigMap.insert(std::make_pair( 0xE864, dvngLig( 271 ,"0x0C26 0x0C46"))); // 0x100AB
        gTeluguLigMap.insert(std::make_pair( 0xE865, dvngLig( 272 ,"0x0C27 0x0C46"))); // 0x100AC
        gTeluguLigMap.insert(std::make_pair( 0xE866, dvngLig( 273 ,"0x0C28 0x0C46"))); // 0x100AD
        gTeluguLigMap.insert(std::make_pair( 0xE867, dvngLig( 274 ,"0x0C2A 0x0C46"))); // 0x100AE
        gTeluguLigMap.insert(std::make_pair( 0xE868, dvngLig( 275 ,"0x0C2B 0x0C46"))); // 0x100AF
        gTeluguLigMap.insert(std::make_pair( 0xE869, dvngLig( 276 ,"0x0C2C 0x0C46"))); // 0x100B0
        gTeluguLigMap.insert(std::make_pair( 0xE86A, dvngLig( 277 ,"0x0C2D 0x0C46"))); // 0x100B1
        gTeluguLigMap.insert(std::make_pair( 0xE86B, dvngLig( 278 ,"0x0C2E 0x0C46"))); // 0x100B2
        gTeluguLigMap.insert(std::make_pair( 0xE86C, dvngLig( 279 ,"0x0C2F 0x0C46"))); // 0x100B3
        gTeluguLigMap.insert(std::make_pair( 0xE86D, dvngLig( 280 ,"0x0C30 0x0C46"))); // 0x100B4
        gTeluguLigMap.insert(std::make_pair( 0xE86E, dvngLig( 281 ,"0x0C31 0x0C46"))); // 0x100B5
        gTeluguLigMap.insert(std::make_pair( 0xE86F, dvngLig( 282 ,"0x0C32 0x0C46"))); // 0x100B6
        gTeluguLigMap.insert(std::make_pair( 0xE870, dvngLig( 283 ,"0x0C33 0x0C46"))); // 0x100B7
        gTeluguLigMap.insert(std::make_pair( 0xE871, dvngLig( 284 ,"0x0C35 0x0C46"))); // 0x100B8
        gTeluguLigMap.insert(std::make_pair( 0xE872, dvngLig( 285 ,"0x0C36 0x0C46"))); // 0x100B9
        gTeluguLigMap.insert(std::make_pair( 0xE873, dvngLig( 286 ,"0x0C37 0x0C46"))); // 0x100BA
        gTeluguLigMap.insert(std::make_pair( 0xE874, dvngLig( 287 ,"0x0C38 0x0C46"))); // 0x100BB
        gTeluguLigMap.insert(std::make_pair( 0xE875, dvngLig( 288 ,"0x0C39 0x0C46"))); // 0x100BC
        gTeluguLigMap.insert(std::make_pair( 0xE876, dvngLig( 289 ,"0x0C15 0x0C4D 0x0C37 0x0C46"))); // 0x100BD
        gTeluguLigMap.insert(std::make_pair( 0xE877, dvngLig( 289 ,"0x0C15 0x0C37 0x0C4D 0x0C46"))); // 0x100BD
        gTeluguLigMap.insert(std::make_pair( 0xE878, dvngLig( 290 ,"0x0C15 0x0C47"))); // 0x100BE
        gTeluguLigMap.insert(std::make_pair( 0xE879, dvngLig( 291 ,"0x0C16 0x0C47"))); // 0x100BF
        gTeluguLigMap.insert(std::make_pair( 0xE87A, dvngLig( 292 ,"0x0C17 0x0C47"))); // 0x100C0
        gTeluguLigMap.insert(std::make_pair( 0xE87B, dvngLig( 293 ,"0x0C18 0x0C47"))); // 0x100C1
        gTeluguLigMap.insert(std::make_pair( 0xE87C, dvngLig( 294 ,"0x0C19 0x0C47"))); // 0x100C2
        gTeluguLigMap.insert(std::make_pair( 0xE87D, dvngLig( 295 ,"0x0C1A 0x0C47"))); // 0x100C3
        gTeluguLigMap.insert(std::make_pair( 0xE87E, dvngLig( 296 ,"0x0C1B 0x0C47"))); // 0x100C4
        gTeluguLigMap.insert(std::make_pair( 0xE87F, dvngLig( 297 ,"0x0C1C 0x0C47"))); // 0x100C5
        gTeluguLigMap.insert(std::make_pair( 0xE880, dvngLig( 298 ,"0x0C1D 0x0C47"))); // 0x100C6
        gTeluguLigMap.insert(std::make_pair( 0xE881, dvngLig( 299 ,"0x0C1E 0x0C47"))); // 0x100C7
        gTeluguLigMap.insert(std::make_pair( 0xE882, dvngLig( 300 ,"0x0C1F 0x0C47"))); // 0x100C8
        gTeluguLigMap.insert(std::make_pair( 0xE883, dvngLig( 301 ,"0x0C20 0x0C47"))); // 0x100C9
        gTeluguLigMap.insert(std::make_pair( 0xE884, dvngLig( 302 ,"0x0C21 0x0C47"))); // 0x100CA
        gTeluguLigMap.insert(std::make_pair( 0xE885, dvngLig( 303 ,"0x0C22 0x0C47"))); // 0x100CB
        gTeluguLigMap.insert(std::make_pair( 0xE886, dvngLig( 304 ,"0x0C23 0x0C47"))); // 0x100CC
        gTeluguLigMap.insert(std::make_pair( 0xE887, dvngLig( 305 ,"0x0C24 0x0C47"))); // 0x100CD
        gTeluguLigMap.insert(std::make_pair( 0xE888, dvngLig( 306 ,"0x0C25 0x0C47"))); // 0x100CE
        gTeluguLigMap.insert(std::make_pair( 0xE889, dvngLig( 307 ,"0x0C26 0x0C47"))); // 0x100CF
        gTeluguLigMap.insert(std::make_pair( 0xE88A, dvngLig( 308 ,"0x0C27 0x0C47"))); // 0x100D0
        gTeluguLigMap.insert(std::make_pair( 0xE88B, dvngLig( 309 ,"0x0C28 0x0C47"))); // 0x100D1
        gTeluguLigMap.insert(std::make_pair( 0xE88C, dvngLig( 310 ,"0x0C2A 0x0C47"))); // 0x100D2
        gTeluguLigMap.insert(std::make_pair( 0xE88D, dvngLig( 311 ,"0x0C2B 0x0C47"))); // 0x100D3
        gTeluguLigMap.insert(std::make_pair( 0xE88E, dvngLig( 312 ,"0x0C2C 0x0C47"))); // 0x100D4
        gTeluguLigMap.insert(std::make_pair( 0xE88F, dvngLig( 313 ,"0x0C2D 0x0C47"))); // 0x100D5
        gTeluguLigMap.insert(std::make_pair( 0xE890, dvngLig( 314 ,"0x0C2E 0x0C47"))); // 0x100D6
        gTeluguLigMap.insert(std::make_pair( 0xE891, dvngLig( 315 ,"0x0C2F 0x0C47"))); // 0x100D7
        gTeluguLigMap.insert(std::make_pair( 0xE892, dvngLig( 316 ,"0x0C30 0x0C47"))); // 0x100D8
        gTeluguLigMap.insert(std::make_pair( 0xE893, dvngLig( 317 ,"0x0C31 0x0C47"))); // 0x100D9
        gTeluguLigMap.insert(std::make_pair( 0xE894, dvngLig( 318 ,"0x0C32 0x0C47"))); // 0x100DA
        gTeluguLigMap.insert(std::make_pair( 0xE895, dvngLig( 319 ,"0x0C33 0x0C47"))); // 0x100DB
        gTeluguLigMap.insert(std::make_pair( 0xE896, dvngLig( 320 ,"0x0C35 0x0C47"))); // 0x100DC
        gTeluguLigMap.insert(std::make_pair( 0xE897, dvngLig( 321 ,"0x0C36 0x0C47"))); // 0x100DD
        gTeluguLigMap.insert(std::make_pair( 0xE898, dvngLig( 322 ,"0x0C37 0x0C47"))); // 0x100DE
        gTeluguLigMap.insert(std::make_pair( 0xE899, dvngLig( 323 ,"0x0C38 0x0C47"))); // 0x100DF
        gTeluguLigMap.insert(std::make_pair( 0xE89A, dvngLig( 324 ,"0x0C39 0x0C47"))); // 0x100E0
        gTeluguLigMap.insert(std::make_pair( 0xE89B, dvngLig( 325 ,"0x0C15 0x0C4D 0x0C37 0x0C47"))); // 0x100E1
        gTeluguLigMap.insert(std::make_pair( 0xE89C, dvngLig( 325 ,"0x0C15 0x0C37 0x0C4D 0x0C47"))); // 0x100E1
        gTeluguLigMap.insert(std::make_pair( 0xE89D, dvngLig( 326 ,"0x0C15 0x0C46 0x0C56"))); // 0x100E2
        gTeluguLigMap.insert(std::make_pair( 0xE89E, dvngLig( 327 ,"0x0C16 0x0C46 0x0C56"))); // 0x100E3
        gTeluguLigMap.insert(std::make_pair( 0xE89F, dvngLig( 328 ,"0x0C17 0x0C46 0x0C56"))); // 0x100E4
        gTeluguLigMap.insert(std::make_pair( 0xE8A0, dvngLig( 329 ,"0x0C18 0x0C46 0x0C56"))); // 0x100E5
        gTeluguLigMap.insert(std::make_pair( 0xE8A1, dvngLig( 330 ,"0x0C19 0x0C46 0x0C56"))); // 0x100E6
        gTeluguLigMap.insert(std::make_pair( 0xE8A2, dvngLig( 331 ,"0x0C1A 0x0C46 0x0C56"))); // 0x100E7
        gTeluguLigMap.insert(std::make_pair( 0xE8A3, dvngLig( 332 ,"0x0C1B 0x0C46 0x0C56"))); // 0x100E8
        gTeluguLigMap.insert(std::make_pair( 0xE8A4, dvngLig( 333 ,"0x0C1C 0x0C46 0x0C56"))); // 0x100E9
        gTeluguLigMap.insert(std::make_pair( 0xE8A5, dvngLig( 334 ,"0x0C1D 0x0C46 0x0C56"))); // 0x100EA
        gTeluguLigMap.insert(std::make_pair( 0xE8A6, dvngLig( 335 ,"0x0C1E 0x0C46 0x0C56"))); // 0x100EB
        gTeluguLigMap.insert(std::make_pair( 0xE8A7, dvngLig( 336 ,"0x0C1F 0x0C46 0x0C56"))); // 0x100EC
        gTeluguLigMap.insert(std::make_pair( 0xE8A8, dvngLig( 337 ,"0x0C20 0x0C46 0x0C56"))); // 0x100ED
        gTeluguLigMap.insert(std::make_pair( 0xE8A9, dvngLig( 338 ,"0x0C21 0x0C46 0x0C56"))); // 0x100EE
        gTeluguLigMap.insert(std::make_pair( 0xE8AA, dvngLig( 339 ,"0x0C22 0x0C46 0x0C56"))); // 0x100EF
        gTeluguLigMap.insert(std::make_pair( 0xE8AB, dvngLig( 340 ,"0x0C23 0x0C46 0x0C56"))); // 0x100F0
        gTeluguLigMap.insert(std::make_pair( 0xE8AC, dvngLig( 341 ,"0x0C24 0x0C46 0x0C56"))); // 0x100F1
        gTeluguLigMap.insert(std::make_pair( 0xE8AD, dvngLig( 342 ,"0x0C25 0x0C46 0x0C56"))); // 0x100F2
        gTeluguLigMap.insert(std::make_pair( 0xE8AE, dvngLig( 343 ,"0x0C26 0x0C46 0x0C56"))); // 0x100F3
        gTeluguLigMap.insert(std::make_pair( 0xE8AF, dvngLig( 344 ,"0x0C27 0x0C46 0x0C56"))); // 0x100F4
        gTeluguLigMap.insert(std::make_pair( 0xE8B0, dvngLig( 345 ,"0x0C28 0x0C46 0x0C56"))); // 0x100F5
        gTeluguLigMap.insert(std::make_pair( 0xE8B1, dvngLig( 346 ,"0x0C2A 0x0C46 0x0C56"))); // 0x100F6
        gTeluguLigMap.insert(std::make_pair( 0xE8B2, dvngLig( 347 ,"0x0C2B 0x0C46 0x0C56"))); // 0x100F7
        gTeluguLigMap.insert(std::make_pair( 0xE8B3, dvngLig( 348 ,"0x0C2C 0x0C46 0x0C56"))); // 0x100F8
        gTeluguLigMap.insert(std::make_pair( 0xE8B4, dvngLig( 349 ,"0x0C2D 0x0C46 0x0C56"))); // 0x100F9
        gTeluguLigMap.insert(std::make_pair( 0xE8B5, dvngLig( 350 ,"0x0C2E 0x0C46 0x0C56"))); // 0x100FA
        gTeluguLigMap.insert(std::make_pair( 0xE8B6, dvngLig( 351 ,"0x0C2F 0x0C46 0x0C56"))); // 0x100FB
        gTeluguLigMap.insert(std::make_pair( 0xE8B7, dvngLig( 352 ,"0x0C30 0x0C46 0x0C56"))); // 0x100FC
        gTeluguLigMap.insert(std::make_pair( 0xE8B8, dvngLig( 353 ,"0x0C31 0x0C46 0x0C56"))); // 0x100FD
        gTeluguLigMap.insert(std::make_pair( 0xE8B9, dvngLig( 354 ,"0x0C32 0x0C46 0x0C56"))); // 0x100FE
        gTeluguLigMap.insert(std::make_pair( 0xE8BA, dvngLig( 355 ,"0x0C33 0x0C46 0x0C56"))); // 0x100FF
        gTeluguLigMap.insert(std::make_pair( 0xE8BB, dvngLig( 356 ,"0x0C35 0x0C46 0x0C56"))); // 0x10100
        gTeluguLigMap.insert(std::make_pair( 0xE8BC, dvngLig( 357 ,"0x0C36 0x0C46 0x0C56"))); // 0x10101
        gTeluguLigMap.insert(std::make_pair( 0xE8BD, dvngLig( 358 ,"0x0C37 0x0C46 0x0C56"))); // 0x10102
        gTeluguLigMap.insert(std::make_pair( 0xE8BE, dvngLig( 359 ,"0x0C38 0x0C46 0x0C56"))); // 0x10103
        gTeluguLigMap.insert(std::make_pair( 0xE8BF, dvngLig( 360 ,"0x0C39 0x0C46 0x0C56"))); // 0x10104
        gTeluguLigMap.insert(std::make_pair( 0xE8C0, dvngLig( 361 ,"0x0C15 0x0C4D 0x0C37 0x0C46 0x0C56"))); // 0x10105
        gTeluguLigMap.insert(std::make_pair( 0xE8C1, dvngLig( 361 ,"0x0C15 0x0C37 0x0C4D 0x0C46 0x0C56"))); // 0x10105
        gTeluguLigMap.insert(std::make_pair( 0xE8C2, dvngLig( 362 ,"0x0C15 0x0C4A"))); // 0x10106
        gTeluguLigMap.insert(std::make_pair( 0xE8C3, dvngLig( 363 ,"0x0C16 0x0C4A"))); // 0x10107
        gTeluguLigMap.insert(std::make_pair( 0xE8C4, dvngLig( 364 ,"0x0C17 0x0C4A"))); // 0x10108
        gTeluguLigMap.insert(std::make_pair( 0xE8C5, dvngLig( 365 ,"0x0C18 0x0C4A"))); // 0x10109
        gTeluguLigMap.insert(std::make_pair( 0xE8C6, dvngLig( 366 ,"0x0C19 0x0C4A"))); // 0x1010A
        gTeluguLigMap.insert(std::make_pair( 0xE8C7, dvngLig( 367 ,"0x0C1A 0x0C4A"))); // 0x1010B
        gTeluguLigMap.insert(std::make_pair( 0xE8C8, dvngLig( 368 ,"0x0C1B 0x0C4A"))); // 0x1010C
        gTeluguLigMap.insert(std::make_pair( 0xE8C9, dvngLig( 369 ,"0x0C1C 0x0C4A"))); // 0x1010D
        gTeluguLigMap.insert(std::make_pair( 0xE8CA, dvngLig( 370 ,"0x0C1D 0x0C4A"))); // 0x1010E
        gTeluguLigMap.insert(std::make_pair( 0xE8CB, dvngLig( 371 ,"0x0C1E 0x0C4A"))); // 0x1010F
        gTeluguLigMap.insert(std::make_pair( 0xE8CC, dvngLig( 372 ,"0x0C1F 0x0C4A"))); // 0x10110
        gTeluguLigMap.insert(std::make_pair( 0xE8CD, dvngLig( 373 ,"0x0C20 0x0C4A"))); // 0x10111
        gTeluguLigMap.insert(std::make_pair( 0xE8CE, dvngLig( 374 ,"0x0C21 0x0C4A"))); // 0x10112
        gTeluguLigMap.insert(std::make_pair( 0xE8CF, dvngLig( 375 ,"0x0C22 0x0C4A"))); // 0x10113
        gTeluguLigMap.insert(std::make_pair( 0xE8D0, dvngLig( 376 ,"0x0C23 0x0C4A"))); // 0x10114
        gTeluguLigMap.insert(std::make_pair( 0xE8D1, dvngLig( 377 ,"0x0C24 0x0C4A"))); // 0x10115
        gTeluguLigMap.insert(std::make_pair( 0xE8D2, dvngLig( 378 ,"0x0C25 0x0C4A"))); // 0x10116
        gTeluguLigMap.insert(std::make_pair( 0xE8D3, dvngLig( 379 ,"0x0C26 0x0C4A"))); // 0x10117
        gTeluguLigMap.insert(std::make_pair( 0xE8D4, dvngLig( 380 ,"0x0C27 0x0C4A"))); // 0x10118
        gTeluguLigMap.insert(std::make_pair( 0xE8D5, dvngLig( 381 ,"0x0C28 0x0C4A"))); // 0x10119
        gTeluguLigMap.insert(std::make_pair( 0xE8D6, dvngLig( 382 ,"0x0C2A 0x0C4A"))); // 0x1011A
        gTeluguLigMap.insert(std::make_pair( 0xE8D7, dvngLig( 383 ,"0x0C2B 0x0C4A"))); // 0x1011B
        gTeluguLigMap.insert(std::make_pair( 0xE8D8, dvngLig( 384 ,"0x0C2C 0x0C4A"))); // 0x1011C
        gTeluguLigMap.insert(std::make_pair( 0xE8D9, dvngLig( 385 ,"0x0C2D 0x0C4A"))); // 0x1011D
        gTeluguLigMap.insert(std::make_pair( 0xE8DA, dvngLig( 386 ,"0x0C2E 0x0C4A"))); // 0x1011E
        gTeluguLigMap.insert(std::make_pair( 0xE8DB, dvngLig( 387 ,"0x0C2F 0x0C4A"))); // 0x1011F
        gTeluguLigMap.insert(std::make_pair( 0xE8DC, dvngLig( 388 ,"0x0C30 0x0C4A"))); // 0x10120
        gTeluguLigMap.insert(std::make_pair( 0xE8DD, dvngLig( 389 ,"0x0C31 0x0C4A"))); // 0x10121
        gTeluguLigMap.insert(std::make_pair( 0xE8DE, dvngLig( 390 ,"0x0C32 0x0C4A"))); // 0x10122
        gTeluguLigMap.insert(std::make_pair( 0xE8DF, dvngLig( 391 ,"0x0C33 0x0C4A"))); // 0x10123
        gTeluguLigMap.insert(std::make_pair( 0xE8E0, dvngLig( 392 ,"0x0C35 0x0C4A"))); // 0x10124
        gTeluguLigMap.insert(std::make_pair( 0xE8E1, dvngLig( 393 ,"0x0C36 0x0C4A"))); // 0x10125
        gTeluguLigMap.insert(std::make_pair( 0xE8E2, dvngLig( 394 ,"0x0C37 0x0C4A"))); // 0x10126
        gTeluguLigMap.insert(std::make_pair( 0xE8E3, dvngLig( 395 ,"0x0C38 0x0C4A"))); // 0x10127
        gTeluguLigMap.insert(std::make_pair( 0xE8E4, dvngLig( 396 ,"0x0C39 0x0C4A"))); // 0x10128
        gTeluguLigMap.insert(std::make_pair( 0xE8E5, dvngLig( 397 ,"0x0C15 0x0C4D 0x0C37 0x0C4A"))); // 0x10129
        gTeluguLigMap.insert(std::make_pair( 0xE8E6, dvngLig( 397 ,"0x0C15 0x0C37 0x0C4D 0x0C4A"))); // 0x10129
        gTeluguLigMap.insert(std::make_pair( 0xE8E7, dvngLig( 398 ,"0x0C15 0x0C4B"))); // 0x1012A
        gTeluguLigMap.insert(std::make_pair( 0xE8E8, dvngLig( 399 ,"0x0C16 0x0C4B"))); // 0x1012B
        gTeluguLigMap.insert(std::make_pair( 0xE8E9, dvngLig( 400 ,"0x0C17 0x0C4B"))); // 0x1012C
        gTeluguLigMap.insert(std::make_pair( 0xE8EA, dvngLig( 401 ,"0x0C18 0x0C4B"))); // 0x1012D
        gTeluguLigMap.insert(std::make_pair( 0xE8EB, dvngLig( 402 ,"0x0C19 0x0C4B"))); // 0x1012E
        gTeluguLigMap.insert(std::make_pair( 0xE8EC, dvngLig( 403 ,"0x0C1A 0x0C4B"))); // 0x1012F
        gTeluguLigMap.insert(std::make_pair( 0xE8ED, dvngLig( 404 ,"0x0C1B 0x0C4B"))); // 0x10130
        gTeluguLigMap.insert(std::make_pair( 0xE8EE, dvngLig( 405 ,"0x0C1C 0x0C4B"))); // 0x10131
        gTeluguLigMap.insert(std::make_pair( 0xE8EF, dvngLig( 406 ,"0x0C1D 0x0C4B"))); // 0x10132
        gTeluguLigMap.insert(std::make_pair( 0xE8F0, dvngLig( 407 ,"0x0C1E 0x0C4B"))); // 0x10133
        gTeluguLigMap.insert(std::make_pair( 0xE8F1, dvngLig( 408 ,"0x0C1F 0x0C4B"))); // 0x10134
        gTeluguLigMap.insert(std::make_pair( 0xE8F2, dvngLig( 409 ,"0x0C20 0x0C4B"))); // 0x10135
        gTeluguLigMap.insert(std::make_pair( 0xE8F3, dvngLig( 410 ,"0x0C21 0x0C4B"))); // 0x10136
        gTeluguLigMap.insert(std::make_pair( 0xE8F4, dvngLig( 411 ,"0x0C22 0x0C4B"))); // 0x10137
        gTeluguLigMap.insert(std::make_pair( 0xE8F5, dvngLig( 412 ,"0x0C23 0x0C4B"))); // 0x10138
        gTeluguLigMap.insert(std::make_pair( 0xE8F6, dvngLig( 413 ,"0x0C24 0x0C4B"))); // 0x10139
        gTeluguLigMap.insert(std::make_pair( 0xE8F7, dvngLig( 414 ,"0x0C25 0x0C4B"))); // 0x1013A
        gTeluguLigMap.insert(std::make_pair( 0xE8F8, dvngLig( 415 ,"0x0C26 0x0C4B"))); // 0x1013B
        gTeluguLigMap.insert(std::make_pair( 0xE8F9, dvngLig( 416 ,"0x0C27 0x0C4B"))); // 0x1013C
        gTeluguLigMap.insert(std::make_pair( 0xE8FA, dvngLig( 417 ,"0x0C28 0x0C4B"))); // 0x1013D
        gTeluguLigMap.insert(std::make_pair( 0xE8FB, dvngLig( 418 ,"0x0C2A 0x0C4B"))); // 0x1013E
        gTeluguLigMap.insert(std::make_pair( 0xE8FC, dvngLig( 419 ,"0x0C2B 0x0C4B"))); // 0x1013F
        gTeluguLigMap.insert(std::make_pair( 0xE8FD, dvngLig( 420 ,"0x0C2C 0x0C4B"))); // 0x10140
        gTeluguLigMap.insert(std::make_pair( 0xE8FE, dvngLig( 421 ,"0x0C2D 0x0C4B"))); // 0x10141
        gTeluguLigMap.insert(std::make_pair( 0xE8FF, dvngLig( 422 ,"0x0C2E 0x0C4B"))); // 0x10142
        gTeluguLigMap.insert(std::make_pair( 0xE900, dvngLig( 423 ,"0x0C2F 0x0C4B"))); // 0x10143
        gTeluguLigMap.insert(std::make_pair( 0xE901, dvngLig( 424 ,"0x0C30 0x0C4B"))); // 0x10144
        gTeluguLigMap.insert(std::make_pair( 0xE902, dvngLig( 425 ,"0x0C31 0x0C4B"))); // 0x10145
        gTeluguLigMap.insert(std::make_pair( 0xE903, dvngLig( 426 ,"0x0C32 0x0C4B"))); // 0x10146
        gTeluguLigMap.insert(std::make_pair( 0xE904, dvngLig( 427 ,"0x0C33 0x0C4B"))); // 0x10147
        gTeluguLigMap.insert(std::make_pair( 0xE905, dvngLig( 428 ,"0x0C35 0x0C4B"))); // 0x10148
        gTeluguLigMap.insert(std::make_pair( 0xE906, dvngLig( 429 ,"0x0C36 0x0C4B"))); // 0x10149
        gTeluguLigMap.insert(std::make_pair( 0xE907, dvngLig( 430 ,"0x0C37 0x0C4B"))); // 0x1014A
        gTeluguLigMap.insert(std::make_pair( 0xE908, dvngLig( 431 ,"0x0C38 0x0C4B"))); // 0x1014B
        gTeluguLigMap.insert(std::make_pair( 0xE909, dvngLig( 432 ,"0x0C39 0x0C4B"))); // 0x1014C
        gTeluguLigMap.insert(std::make_pair( 0xE90A, dvngLig( 433 ,"0x0C15 0x0C4D 0x0C37 0x0C4B"))); // 0x1014D
        gTeluguLigMap.insert(std::make_pair( 0xE90B, dvngLig( 433 ,"0x0C15 0x0C37 0x0C4D 0x0C4B"))); // 0x1014D
        gTeluguLigMap.insert(std::make_pair( 0xE90C, dvngLig( 434 ,"0x0C15 0x0C4C"))); // 0x1014E
        gTeluguLigMap.insert(std::make_pair( 0xE90D, dvngLig( 435 ,"0x0C16 0x0C4C"))); // 0x1014F
        gTeluguLigMap.insert(std::make_pair( 0xE90E, dvngLig( 436 ,"0x0C17 0x0C4C"))); // 0x10150
        gTeluguLigMap.insert(std::make_pair( 0xE90F, dvngLig( 437 ,"0x0C18 0x0C4C"))); // 0x10151
        gTeluguLigMap.insert(std::make_pair( 0xE910, dvngLig( 438 ,"0x0C19 0x0C4C"))); // 0x10152
        gTeluguLigMap.insert(std::make_pair( 0xE911, dvngLig( 439 ,"0x0C1A 0x0C4C"))); // 0x10153
        gTeluguLigMap.insert(std::make_pair( 0xE912, dvngLig( 440 ,"0x0C1B 0x0C4C"))); // 0x10154
        gTeluguLigMap.insert(std::make_pair( 0xE913, dvngLig( 441 ,"0x0C1C 0x0C4C"))); // 0x10155
        gTeluguLigMap.insert(std::make_pair( 0xE914, dvngLig( 442 ,"0x0C1D 0x0C4C"))); // 0x10156
        gTeluguLigMap.insert(std::make_pair( 0xE915, dvngLig( 443 ,"0x0C1E 0x0C4C"))); // 0x10157
        gTeluguLigMap.insert(std::make_pair( 0xE916, dvngLig( 444 ,"0x0C1F 0x0C4C"))); // 0x10158
        gTeluguLigMap.insert(std::make_pair( 0xE917, dvngLig( 445 ,"0x0C20 0x0C4C"))); // 0x10159
        gTeluguLigMap.insert(std::make_pair( 0xE918, dvngLig( 446 ,"0x0C21 0x0C4C"))); // 0x1015A
        gTeluguLigMap.insert(std::make_pair( 0xE919, dvngLig( 447 ,"0x0C22 0x0C4C"))); // 0x1015B
        gTeluguLigMap.insert(std::make_pair( 0xE91A, dvngLig( 448 ,"0x0C23 0x0C4C"))); // 0x1015C
        gTeluguLigMap.insert(std::make_pair( 0xE91B, dvngLig( 449 ,"0x0C24 0x0C4C"))); // 0x1015D
        gTeluguLigMap.insert(std::make_pair( 0xE91C, dvngLig( 450 ,"0x0C25 0x0C4C"))); // 0x1015E
        gTeluguLigMap.insert(std::make_pair( 0xE91D, dvngLig( 451 ,"0x0C26 0x0C4C"))); // 0x1015F
        gTeluguLigMap.insert(std::make_pair( 0xE91E, dvngLig( 452 ,"0x0C27 0x0C4C"))); // 0x10160
        gTeluguLigMap.insert(std::make_pair( 0xE91F, dvngLig( 453 ,"0x0C28 0x0C4C"))); // 0x10161
        gTeluguLigMap.insert(std::make_pair( 0xE920, dvngLig( 454 ,"0x0C2A 0x0C4C"))); // 0x10162
        gTeluguLigMap.insert(std::make_pair( 0xE921, dvngLig( 455 ,"0x0C2B 0x0C4C"))); // 0x10163
        gTeluguLigMap.insert(std::make_pair( 0xE922, dvngLig( 456 ,"0x0C2C 0x0C4C"))); // 0x10164
        gTeluguLigMap.insert(std::make_pair( 0xE923, dvngLig( 457 ,"0x0C2D 0x0C4C"))); // 0x10165
        gTeluguLigMap.insert(std::make_pair( 0xE924, dvngLig( 458 ,"0x0C2E 0x0C4C"))); // 0x10166
        gTeluguLigMap.insert(std::make_pair( 0xE925, dvngLig( 459 ,"0x0C2F 0x0C4C"))); // 0x10167
        gTeluguLigMap.insert(std::make_pair( 0xE926, dvngLig( 460 ,"0x0C30 0x0C4C"))); // 0x10168
        gTeluguLigMap.insert(std::make_pair( 0xE927, dvngLig( 461 ,"0x0C31 0x0C4C"))); // 0x10169
        gTeluguLigMap.insert(std::make_pair( 0xE928, dvngLig( 462 ,"0x0C32 0x0C4C"))); // 0x1016A
        gTeluguLigMap.insert(std::make_pair( 0xE929, dvngLig( 463 ,"0x0C33 0x0C4C"))); // 0x1016B
        gTeluguLigMap.insert(std::make_pair( 0xE92A, dvngLig( 464 ,"0x0C35 0x0C4C"))); // 0x1016C
        gTeluguLigMap.insert(std::make_pair( 0xE92B, dvngLig( 465 ,"0x0C36 0x0C4C"))); // 0x1016D
        gTeluguLigMap.insert(std::make_pair( 0xE92C, dvngLig( 466 ,"0x0C37 0x0C4C"))); // 0x1016E
        gTeluguLigMap.insert(std::make_pair( 0xE92D, dvngLig( 467 ,"0x0C38 0x0C4C"))); // 0x1016F
        gTeluguLigMap.insert(std::make_pair( 0xE92E, dvngLig( 468 ,"0x0C39 0x0C4C"))); // 0x10170
        gTeluguLigMap.insert(std::make_pair( 0xE92F, dvngLig( 469 ,"0x0C15 0x0C4D 0x0C37 0x0C4C"))); // 0x10171
        gTeluguLigMap.insert(std::make_pair( 0xE930, dvngLig( 469 ,"0x0C15 0x0C37 0x0C4D 0x0C4C"))); // 0x10171
        gTeluguLigMap.insert(std::make_pair( 0xE931, dvngLig( 470 ,"0x0C4D 0x0C15"))); // 0x10172
        gTeluguLigMap.insert(std::make_pair( 0xE932, dvngLig( 470 ,"0x0C15 0x0C4D"))); // 0x10172
        gTeluguLigMap.insert(std::make_pair( 0xE933, dvngLig( 471 ,"0x0C4D 0x0C16"))); // 0x10173
        gTeluguLigMap.insert(std::make_pair( 0xE934, dvngLig( 471 ,"0x0C16 0x0C4D"))); // 0x10173
        gTeluguLigMap.insert(std::make_pair( 0xE935, dvngLig( 472 ,"0x0C4D 0x0C17"))); // 0x10174
        //gTeluguLigMap.insert(std::make_pair( 0xE936, dvngLig( 472 ,"0x0C17 0x0C4D"))); // 0x10174
        gTeluguLigMap.insert(std::make_pair( 0xE937, dvngLig( 473 ,"0x0C4D 0x0C18"))); // 0x10175
        gTeluguLigMap.insert(std::make_pair( 0xE938, dvngLig( 473 ,"0x0C18 0x0C4D"))); // 0x10175
        gTeluguLigMap.insert(std::make_pair( 0xE939, dvngLig( 474 ,"0x0C4D 0x0C19"))); // 0x10176
        gTeluguLigMap.insert(std::make_pair( 0xE93A, dvngLig( 474 ,"0x0C19 0x0C4D"))); // 0x10176
        gTeluguLigMap.insert(std::make_pair( 0xE93B, dvngLig( 475 ,"0x0C4D 0x0C1A"))); // 0x10177
        gTeluguLigMap.insert(std::make_pair( 0xE93C, dvngLig( 475 ,"0x0C1A 0x0C4D"))); // 0x10177
        gTeluguLigMap.insert(std::make_pair( 0xE93D, dvngLig( 476 ,"0x0C4D 0x0C1B"))); // 0x10178
        gTeluguLigMap.insert(std::make_pair( 0xE93E, dvngLig( 476 ,"0x0C1B 0x0C4D"))); // 0x10178
        gTeluguLigMap.insert(std::make_pair( 0xE93F, dvngLig( 477 ,"0x0C4D 0x0C1C"))); // 0x10179
        gTeluguLigMap.insert(std::make_pair( 0xE940, dvngLig( 477 ,"0x0C1C 0x0C4D"))); // 0x10179
        gTeluguLigMap.insert(std::make_pair( 0xE941, dvngLig( 478 ,"0x0C4D 0x0C1D"))); // 0x1017A
        gTeluguLigMap.insert(std::make_pair( 0xE942, dvngLig( 478 ,"0x0C1D 0x0C4D"))); // 0x1017A
        gTeluguLigMap.insert(std::make_pair( 0xE943, dvngLig( 479 ,"0x0C4D 0x0C1E"))); // 0x1017B
        gTeluguLigMap.insert(std::make_pair( 0xE944, dvngLig( 479 ,"0x0C1E 0x0C4D"))); // 0x1017B
        gTeluguLigMap.insert(std::make_pair( 0xE945, dvngLig( 480 ,"0x0C4D 0x0C1F"))); // 0x1017C
        gTeluguLigMap.insert(std::make_pair( 0xE946, dvngLig( 480 ,"0x0C1F 0x0C4D"))); // 0x1017C
        gTeluguLigMap.insert(std::make_pair( 0xE947, dvngLig( 481 ,"0x0C4D 0x0C20"))); // 0x1017D
        gTeluguLigMap.insert(std::make_pair( 0xE948, dvngLig( 481 ,"0x0C20 0x0C4D"))); // 0x1017D
        gTeluguLigMap.insert(std::make_pair( 0xE949, dvngLig( 482 ,"0x0C4D 0x0C21"))); // 0x1017E
        gTeluguLigMap.insert(std::make_pair( 0xE94A, dvngLig( 482 ,"0x0C21 0x0C4D"))); // 0x1017E
        gTeluguLigMap.insert(std::make_pair( 0xE94B, dvngLig( 483 ,"0x0C4D 0x0C22"))); // 0x1017F
        gTeluguLigMap.insert(std::make_pair( 0xE94C, dvngLig( 483 ,"0x0C22 0x0C4D"))); // 0x1017F
        gTeluguLigMap.insert(std::make_pair( 0xE94D, dvngLig( 484 ,"0x0C4D 0x0C23"))); // 0x10180
        gTeluguLigMap.insert(std::make_pair( 0xE94E, dvngLig( 484 ,"0x0C23 0x0C4D"))); // 0x10180
        gTeluguLigMap.insert(std::make_pair( 0xE94F, dvngLig( 485 ,"0x0C4D 0x0C24"))); // 0x10181
        gTeluguLigMap.insert(std::make_pair( 0xE950, dvngLig( 485 ,"0x0C24 0x0C4D"))); // 0x10181
        gTeluguLigMap.insert(std::make_pair( 0xE951, dvngLig( 486 ,"0x0C4D 0x0C25"))); // 0x10182
        gTeluguLigMap.insert(std::make_pair( 0xE952, dvngLig( 486 ,"0x0C25 0x0C4D"))); // 0x10182
        gTeluguLigMap.insert(std::make_pair( 0xE953, dvngLig( 487 ,"0x0C4D 0x0C26"))); // 0x10183
        gTeluguLigMap.insert(std::make_pair( 0xE954, dvngLig( 487 ,"0x0C26 0x0C4D"))); // 0x10183
        gTeluguLigMap.insert(std::make_pair( 0xE955, dvngLig( 488 ,"0x0C4D 0x0C27"))); // 0x10184
        gTeluguLigMap.insert(std::make_pair( 0xE956, dvngLig( 488 ,"0x0C27 0x0C4D"))); // 0x10184
        gTeluguLigMap.insert(std::make_pair( 0xE957, dvngLig( 489 ,"0x0C4D 0x0C28"))); // 0x10185
        gTeluguLigMap.insert(std::make_pair( 0xE958, dvngLig( 489 ,"0x0C28 0x0C4D"))); // 0x10185
        gTeluguLigMap.insert(std::make_pair( 0xE959, dvngLig( 490 ,"0x0C4D 0x0C2A"))); // 0x10186
        //gTeluguLigMap.insert(std::make_pair( 0xE95A, dvngLig( 490 ,"0x0C2A 0x0C4D"))); // 0x10186
        gTeluguLigMap.insert(std::make_pair( 0xE95B, dvngLig( 491 ,"0x0C4D 0x0C2B"))); // 0x10187
        gTeluguLigMap.insert(std::make_pair( 0xE95C, dvngLig( 491 ,"0x0C2B 0x0C4D"))); // 0x10187
        gTeluguLigMap.insert(std::make_pair( 0xE95D, dvngLig( 492 ,"0x0C4D 0x0C2C"))); // 0x10188
        //gTeluguLigMap.insert(std::make_pair( 0xE95E, dvngLig( 492 ,"0x0C2C 0x0C4D"))); // 0x10188
        gTeluguLigMap.insert(std::make_pair( 0xE95F, dvngLig( 493 ,"0x0C4D 0x0C2D"))); // 0x10189
        gTeluguLigMap.insert(std::make_pair( 0xE960, dvngLig( 493 ,"0x0C2D 0x0C4D"))); // 0x10189
        gTeluguLigMap.insert(std::make_pair( 0xE961, dvngLig( 494 ,"0x0C4D 0x0C2E"))); // 0x1018A
        gTeluguLigMap.insert(std::make_pair( 0xE962, dvngLig( 494 ,"0x0C2E 0x0C4D"))); // 0x1018A
        gTeluguLigMap.insert(std::make_pair( 0xE963, dvngLig( 495 ,"0x0C4D 0x0C2F"))); // 0x1018B
        gTeluguLigMap.insert(std::make_pair( 0xE964, dvngLig( 495 ,"0x0C2F 0x0C4D"))); // 0x1018B
        //gTeluguLigMap.insert(std::make_pair( 0xE965, dvngLig( 496 ,"0x0C4D 0x0C30"))); // 0x1018C
        //gTeluguLigMap.insert(std::make_pair( 0xE966, dvngLig( 496 ,"0x0C30 0x0C4D"))); // 0x1018C
        gTeluguLigMap.insert(std::make_pair( 0xE967, dvngLig( 497 ,"0x0C4D 0x0C31"))); // 0x1018D
        gTeluguLigMap.insert(std::make_pair( 0xE968, dvngLig( 497 ,"0x0C31 0x0C4D"))); // 0x1018D
        gTeluguLigMap.insert(std::make_pair( 0xE969, dvngLig( 498 ,"0x0C4D 0x0C32"))); // 0x1018E
        //gTeluguLigMap.insert(std::make_pair( 0xE96A, dvngLig( 498 ,"0x0C32 0x0C4D"))); // 0x1018E
        gTeluguLigMap.insert(std::make_pair( 0xE96B, dvngLig( 499 ,"0x0C4D 0x0C33"))); // 0x1018F
        gTeluguLigMap.insert(std::make_pair( 0xE96C, dvngLig( 499 ,"0x0C33 0x0C4D"))); // 0x1018F
        gTeluguLigMap.insert(std::make_pair( 0xE96D, dvngLig( 500 ,"0x0C4D 0x0C35"))); // 0x10190
        gTeluguLigMap.insert(std::make_pair( 0xE96E, dvngLig( 500 ,"0x0C35 0x0C4D"))); // 0x10190
        gTeluguLigMap.insert(std::make_pair( 0xE96F, dvngLig( 501 ,"0x0C4D 0x0C36"))); // 0x10191
        //gTeluguLigMap.insert(std::make_pair( 0xE970, dvngLig( 501 ,"0x0C36 0x0C4D"))); // 0x10191
        gTeluguLigMap.insert(std::make_pair( 0xE971, dvngLig( 502 ,"0x0C4D 0x0C37"))); // 0x10192
        gTeluguLigMap.insert(std::make_pair( 0xE972, dvngLig( 502 ,"0x0C37 0x0C4D"))); // 0x10192
        gTeluguLigMap.insert(std::make_pair( 0xE973, dvngLig( 503 ,"0x0C4D 0x0C38"))); // 0x10193
        //gTeluguLigMap.insert(std::make_pair( 0xE974, dvngLig( 503 ,"0x0C38 0x0C4D"))); // 0x10193
        gTeluguLigMap.insert(std::make_pair( 0xE975, dvngLig( 504 ,"0x0C4D 0x0C39"))); // 0x10194
        gTeluguLigMap.insert(std::make_pair( 0xE976, dvngLig( 504 ,"0x0C39 0x0C4D"))); // 0x10194
        gTeluguLigMap.insert(std::make_pair( 0xE977, dvngLig( 588 ,"0x0C56 0x0C4D 0x0C24"))); // 0x101E8
        gTeluguLigMap.insert(std::make_pair( 0xE978, dvngLig( 588 ,"0x0C56 0x0C24 0x0C4D"))); // 0x101E8
        gTeluguLigMap.insert(std::make_pair( 0xE979, dvngLig( 589 ,"0x0C56 0x0C4D 0x0C24"))); // 0x101E9
        gTeluguLigMap.insert(std::make_pair( 0xE97A, dvngLig( 589 ,"0x0C56 0x0C24 0x0C4D"))); // 0x101E9
        gTeluguLigMap.insert(std::make_pair( 0xE97B, dvngLig( 591 ,"0x0C56 0x0C4D 0x0C16"))); // 0x101EB
        gTeluguLigMap.insert(std::make_pair( 0xE97C, dvngLig( 591 ,"0x0C56 0x0C16 0x0C4D"))); // 0x101EB
        gTeluguLigMap.insert(std::make_pair( 0xE97D, dvngLig( 592 ,"0x0C56 0x0C4D 0x0C17"))); // 0x101EC
        gTeluguLigMap.insert(std::make_pair( 0xE97E, dvngLig( 592 ,"0x0C56 0x0C17 0x0C4D"))); // 0x101EC
        gTeluguLigMap.insert(std::make_pair( 0xE97F, dvngLig( 593 ,"0x0C56 0x0C4D 0x0C18"))); // 0x101ED
        gTeluguLigMap.insert(std::make_pair( 0xE980, dvngLig( 593 ,"0x0C56 0x0C18 0x0C4D"))); // 0x101ED
        gTeluguLigMap.insert(std::make_pair( 0xE981, dvngLig( 594 ,"0x0C56 0x0C4D 0x0C1C"))); // 0x101EE
        gTeluguLigMap.insert(std::make_pair( 0xE982, dvngLig( 594 ,"0x0C56 0x0C1C 0x0C4D"))); // 0x101EE
        gTeluguLigMap.insert(std::make_pair( 0xE983, dvngLig( 595 ,"0x0C56 0x0C4D 0x0C1F"))); // 0x101EF
        gTeluguLigMap.insert(std::make_pair( 0xE984, dvngLig( 595 ,"0x0C56 0x0C1F 0x0C4D"))); // 0x101EF
        gTeluguLigMap.insert(std::make_pair( 0xE985, dvngLig( 596 ,"0x0C56 0x0C4D 0x0C21"))); // 0x101F0
        gTeluguLigMap.insert(std::make_pair( 0xE986, dvngLig( 596 ,"0x0C56 0x0C21 0x0C4D"))); // 0x101F0
        gTeluguLigMap.insert(std::make_pair( 0xE987, dvngLig( 597 ,"0x0C56 0x0C4D 0x0C23"))); // 0x101F1
        gTeluguLigMap.insert(std::make_pair( 0xE988, dvngLig( 597 ,"0x0C56 0x0C23 0x0C4D"))); // 0x101F1
        gTeluguLigMap.insert(std::make_pair( 0xE989, dvngLig( 598 ,"0x0C56 0x0C4D 0x0C25"))); // 0x101F2
        gTeluguLigMap.insert(std::make_pair( 0xE98A, dvngLig( 598 ,"0x0C56 0x0C25 0x0C4D"))); // 0x101F2
        gTeluguLigMap.insert(std::make_pair( 0xE98B, dvngLig( 599 ,"0x0C56 0x0C4D 0x0C26"))); // 0x101F3
        gTeluguLigMap.insert(std::make_pair( 0xE98C, dvngLig( 599 ,"0x0C56 0x0C26 0x0C4D"))); // 0x101F3
        gTeluguLigMap.insert(std::make_pair( 0xE98D, dvngLig( 600 ,"0x0C56 0x0C4D 0x0C27"))); // 0x101F4
        gTeluguLigMap.insert(std::make_pair( 0xE98E, dvngLig( 600 ,"0x0C56 0x0C27 0x0C4D"))); // 0x101F4
        gTeluguLigMap.insert(std::make_pair( 0xE98F, dvngLig( 601 ,"0x0C56 0x0C4D 0x0C32"))); // 0x101F5
        gTeluguLigMap.insert(std::make_pair( 0xE990, dvngLig( 601 ,"0x0C56 0x0C32 0x0C4D"))); // 0x101F5
        gTeluguLigMap.insert(std::make_pair( 0xE991, dvngLig( 602 ,"0x0C56 0x0C4D 0x0C37"))); // 0x101F6
        gTeluguLigMap.insert(std::make_pair( 0xE992, dvngLig( 602 ,"0x0C56 0x0C37 0x0C4D"))); // 0x101F6
        gTeluguLigMap.insert(std::make_pair( 0xE993, dvngLig( 603 ,"0x0C56 0x0C4D 0x0C39"))); // 0x101F7
        gTeluguLigMap.insert(std::make_pair( 0xE994, dvngLig( 603 ,"0x0C56 0x0C39 0x0C4D"))); // 0x101F7
        gTeluguLigMap.insert(std::make_pair( 0xE995, dvngLig( 604 ,"0x0C4D 0x0C24 0x0C4D 0x0C30"))); // 0x101F8
        gTeluguLigMap.insert(std::make_pair( 0xE996, dvngLig( 604 ,"0x0C4D 0x0C24 0x0C30 0x0C4D"))); // 0x101F8
        gTeluguLigMap.insert(std::make_pair( 0xE997, dvngLig( 604 ,"0x0C24 0x0C4D 0x0C4D 0x0C30"))); // 0x101F8
        gTeluguLigMap.insert(std::make_pair( 0xE998, dvngLig( 604 ,"0x0C24 0x0C4D 0x0C30 0x0C4D"))); // 0x101F8
        gTeluguLigMap.insert(std::make_pair( 0xE999, dvngLig( 605 ,"0x0C4D 0x0C37 0x0C4D 0x0C23"))); // 0x101F9
        gTeluguLigMap.insert(std::make_pair( 0xE99A, dvngLig( 605 ,"0x0C4D 0x0C37 0x0C23 0x0C4D"))); // 0x101F9
        gTeluguLigMap.insert(std::make_pair( 0xE99B, dvngLig( 605 ,"0x0C37 0x0C4D 0x0C4D 0x0C23"))); // 0x101F9
        gTeluguLigMap.insert(std::make_pair( 0xE99C, dvngLig( 605 ,"0x0C37 0x0C4D 0x0C23 0x0C4D"))); // 0x101F9

    }
    return gTeluguLigMap;
}

#if 0
void SwitchTeluguE(lString16* str)
{
/*    ( ? 0xE271 E)   -> ( E ? 0xE271)
 *    ( ? 0xE272 E)   -> ( E ? 0xE272)
 *    ( ? 0xE273 E)   -> ( E ? 0xE273)
 *    ( ? E )         -> ( E ? )
 */
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0BC6)
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
                str->at(i - 1) = 0x0BC6;
            }
        }
    }
}

void SwitchTeluguEE(lString16 *str)
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

void SwitchTeluguAI(lString16* str)
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

void SwitchTeluguE_reverse(lString16* str)
{
/*
 *  (e ?)   -> (? e)
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0BC6)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0x0BC6;
        }
    }
}

void SwitchTeluguEE_reverse(lString16* str)
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

void SwitchTeluguAI_reverse(lString16* str)
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

void SwitchTeluguO(lString16 *str)
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

void SwitchTeluguOO(lString16 *str)
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

void SwitchTeluguAU(lString16 *str)
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

void SwitchTeluguO_reverse(lString16* str)
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

void SwitchTeluguOO_reverse(lString16* str)
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

void SwitchTeluguAU_reverse(lString16* str)
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

void SwitchTeluguUnicode(lString16* str)
{
/*
 * ( ? ' ) -> ( ' ? )
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 0; i < str->length(); i++)
    {
        int ch1 = str->at(i+0);
        int ch2 = str->at(i+1);
        if (ch1 == 0x0C12 && ch2 == 0x0C55 )
        {
            str->at(i) = 0x0C13;
            str->erase(i+1);
        }
        if (ch1 == 0x0C12 && ch2 == 0x0C4C)
        {
            str->at(i) = 0x0C14;
            str->erase(i + 1);
        }
        if (ch1 == 0x0C3F && ch2 == 0x0C55)
        {
            str->at(i) = 0x0C40;
            str->erase(i + 1);
        }
        if (ch1 == 0x0C46 && ch2 == 0x0C55)
        {
            str->at(i) = 0x0C47;
            str->erase(i + 1);
        }
        if (ch1 == 0x0C4A && ch2 == 0x0C55)
        {
            str->at(i) = 0x0C4B;
            str->erase(i + 1);
        }
    }
}

void SwitchTeluguViarama(lString16* str)
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
        if (str->at(i) == 0x0C4D)
        {
            str->at(i + 0) = str->at(i - 1);
            str->at(i - 1) = 0x0C4D;
        }
    }
}

void SwitchTeluguViarama_reverse(lString16* str)
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
        if(str->at(i) == 0x0C4D)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0x0C4D;
        }
    }
}

lString16 processTeluguLigatures(lString16 word)
{
    int j = (word.length() >= 7 ) ? 7 : word.length();
    for(;  j >= 2; j--)
    {
        for (int c = word.length() - j; c >= 0; c--)
        {
            lUInt32 fastkey = (word.at(c) << 16) + word.at(c + 1);
            if (gTeluguFastLigMap.find(fastkey) == gTeluguFastLigMap.end())
            {
                continue;
            }
            dvngLig lig(word.substr(c, j));
            lChar16 rep = findTeluguLigRev(lig);

            if (rep != 0)
            {
                word.replace(c, j, lString16(&rep, 1));
                c -= j - 2;
            }
        }
    }
    return word;
}

lString16 lString16::processTeluguText()
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
        SwitchTeluguUnicode(&word);
        word = processTeluguLigatures(word);
        SwitchTeluguViarama(&word);
        //SwitchTeluguE(&word);
        //SwitchTeluguEE(&word);
        //SwitchTeluguAI(&word);
        //SwitchTeluguO(&word);
        //SwitchTeluguOO(&word);
        //SwitchTeluguAU(&word);

        res.append(word);
        res.append(L" ");
    }
    res.substr(0,res.size()-1);
    return res;
}

lString16 restoreTeluguWord(lString16 in)
{
    if(TELUGU_DISPLAY_ENABLE == 0 || gDocumentTelugu == 0)
    {
        return in;
    }

    //SwitchTeluguAU_reverse(&in);
    //SwitchTeluguOO_reverse(&in);
    //SwitchTeluguO_reverse(&in);
    //SwitchTeluguAI_reverse(&in);
    //SwitchTeluguEE_reverse(&in);
    //SwitchTeluguE_reverse(&in);
    SwitchTeluguViarama_reverse(&in);
    for (int i = 0; i < in.length(); i++)
    {
        if (in[i] < TELUGU_START || in[i] > TELUGU_END)
        {
            continue;
        }

        dvngLig lig = findTeluguLig(in[i]);
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
