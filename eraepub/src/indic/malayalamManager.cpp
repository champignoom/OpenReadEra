//
// Created by Tarasus on 08.10.2020.
//

#include "include/indic/malayalamManager.h"
#include "ore_log.h"

#include <vector>

LigMap     gMalayLigMap;
LigMapRev  gMalayLigMapRev;
FastLigMap gMalayFastLigMap;

bool CharIsMalay(int ch)
{
    return ((ch >= 0x0D00 && ch <= 0x0D7F));
}

bool lString16::CheckMalay()
{
    if(gDocumentMalay == 1)
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
        if (CharIsMalay(ch))
        {
            gDocumentMalay = 1;
            gDocumentINDIC = 1;

            if(gMalayLigMapRev.empty())
            {
                gMalayLigMapRev = MalayLigMapReversed();
            }
            return true;
        }
    }
    return false;
}

dvngLig findMalayLig(lChar16 ligature)
{
    if(gMalayLigMap.empty())
    {
        gMalayLigMap = GetMalayLigMap();
    }
    LigMap::iterator it = gMalayLigMap.find(ligature);
    if(it==gMalayLigMap.end())
    {
        return dvngLig();
    }
    return it->second;
}

lChar16 findMalayLigGlyphIndex(lChar16 ligature)
{
    auto it = gMalayLigMap.find(ligature);
    if(it==gMalayLigMap.end())
    {
        return 0;
    }
    return it->second.glyphindex;
}

lChar16 findMalayLigRev(dvngLig combo)
{
    if(gMalayLigMapRev.empty())
    {
        gMalayLigMapRev = MalayLigMapReversed();
    }
    if(combo.len < 2 || combo.len > 10 )
    {
        return 0;
    }
    auto it = gMalayLigMapRev.find(combo);
    if(it==gMalayLigMapRev.end())
    {
        return 0;
    }
    //LE("findMalayLigRev return %d", it->second);
    return it->second;
}

std::map <dvngLig, lChar16, Comparator> MalayLigMapReversed()
{
    if(!gMalayLigMapRev.empty())
    {
        return gMalayLigMapRev;
    }
    if(gMalayLigMap.empty())
    {
        gMalayLigMap = GetMalayLigMap();
    }
    gMalayLigMapRev = makeReverseLigMap(gMalayLigMap,&gMalayFastLigMap);
    return gMalayLigMapRev;
}

LigMap GetMalayLigMap()
{
    if(!gMalayLigMap.empty())
    {
        return gMalayLigMap;
    }
    //MALAY_START
    {
    //from Noto Sans Malayalam.ttf
    gMalayLigMap.insert(std::make_pair( 0xE520, dvngLig( 96 ,"0x0D23 0x0D4D 0x200D"))); // 0x0D7A
    gMalayLigMap.insert(std::make_pair( 0xE521, dvngLig( 97 ,"0x0D28 0x0D4D 0x200D"))); // 0x0D7B
    gMalayLigMap.insert(std::make_pair( 0xE522, dvngLig( 98 ,"0x0D30 0x0D4D 0x200D"))); // 0x0D7C
    gMalayLigMap.insert(std::make_pair( 0xE523, dvngLig( 99 ,"0x0D32 0x0D4D 0x200D"))); // 0x0D7D
    gMalayLigMap.insert(std::make_pair( 0xE524, dvngLig( 100 ,"0x0D33 0x0D4D 0x200D"))); // 0x0D7E
    gMalayLigMap.insert(std::make_pair( 0xE525, dvngLig( 101 ,"0x0D15 0x0D4D 0x200D"))); // 0x0D7F
    gMalayLigMap.insert(std::make_pair( 0xE526, dvngLig( 106 ,"0x0D15 0x0D4D"))); // 0x10001
    gMalayLigMap.insert(std::make_pair( 0xE527, dvngLig( 107 ,"0x0D16 0x0D4D"))); // 0x10002
    gMalayLigMap.insert(std::make_pair( 0xE528, dvngLig( 108 ,"0x0D17 0x0D4D"))); // 0x10003
    gMalayLigMap.insert(std::make_pair( 0xE529, dvngLig( 109 ,"0x0D18 0x0D4D"))); // 0x10004
    gMalayLigMap.insert(std::make_pair( 0xE52A, dvngLig( 110 ,"0x0D19 0x0D4D"))); // 0x10005
    gMalayLigMap.insert(std::make_pair( 0xE52B, dvngLig( 111 ,"0x0D1A 0x0D4D"))); // 0x10006
    gMalayLigMap.insert(std::make_pair( 0xE52C, dvngLig( 112 ,"0x0D1B 0x0D4D"))); // 0x10007
    gMalayLigMap.insert(std::make_pair( 0xE52D, dvngLig( 113 ,"0x0D1C 0x0D4D"))); // 0x10008
    gMalayLigMap.insert(std::make_pair( 0xE52E, dvngLig( 114 ,"0x0D1D 0x0D4D"))); // 0x10009
    gMalayLigMap.insert(std::make_pair( 0xE52F, dvngLig( 115 ,"0x0D1E 0x0D4D"))); // 0x1000A
    gMalayLigMap.insert(std::make_pair( 0xE530, dvngLig( 116 ,"0x0D1F 0x0D4D"))); // 0x1000B
    gMalayLigMap.insert(std::make_pair( 0xE531, dvngLig( 117 ,"0x0D20 0x0D4D"))); // 0x1000C
    gMalayLigMap.insert(std::make_pair( 0xE532, dvngLig( 118 ,"0x0D21 0x0D4D"))); // 0x1000D
    gMalayLigMap.insert(std::make_pair( 0xE533, dvngLig( 119 ,"0x0D22 0x0D4D"))); // 0x1000E
    gMalayLigMap.insert(std::make_pair( 0xE534, dvngLig( 120 ,"0x0D23 0x0D4D"))); // 0x1000F
    gMalayLigMap.insert(std::make_pair( 0xE535, dvngLig( 121 ,"0x0D24 0x0D4D"))); // 0x10010
    gMalayLigMap.insert(std::make_pair( 0xE536, dvngLig( 122 ,"0x0D25 0x0D4D"))); // 0x10011
    gMalayLigMap.insert(std::make_pair( 0xE537, dvngLig( 123 ,"0x0D26 0x0D4D"))); // 0x10012
    gMalayLigMap.insert(std::make_pair( 0xE538, dvngLig( 124 ,"0x0D27 0x0D4D"))); // 0x10013
    gMalayLigMap.insert(std::make_pair( 0xE539, dvngLig( 125 ,"0x0D28 0x0D4D"))); // 0x10014
    gMalayLigMap.insert(std::make_pair( 0xE53A, dvngLig( 126 ,"0x0D2A 0x0D4D"))); // 0x10015
    gMalayLigMap.insert(std::make_pair( 0xE53B, dvngLig( 127 ,"0x0D2B 0x0D4D"))); // 0x10016
    gMalayLigMap.insert(std::make_pair( 0xE53C, dvngLig( 128 ,"0x0D2C 0x0D4D"))); // 0x10017
    gMalayLigMap.insert(std::make_pair( 0xE53D, dvngLig( 129 ,"0x0D2D 0x0D4D"))); // 0x10018
    gMalayLigMap.insert(std::make_pair( 0xE53E, dvngLig( 130 ,"0x0D2E 0x0D4D"))); // 0x10019
    gMalayLigMap.insert(std::make_pair( 0xE53F, dvngLig( 131 ,"0x0D2F 0x0D4D"))); // 0x1001A
    gMalayLigMap.insert(std::make_pair( 0xE540, dvngLig( 132 ,"0x0D30 0x0D4D"))); // 0x1001B
    gMalayLigMap.insert(std::make_pair( 0xE541, dvngLig( 133 ,"0x0D31 0x0D4D"))); // 0x1001C
    gMalayLigMap.insert(std::make_pair( 0xE542, dvngLig( 134 ,"0x0D32 0x0D4D"))); // 0x1001D
    gMalayLigMap.insert(std::make_pair( 0xE543, dvngLig( 135 ,"0x0D33 0x0D4D"))); // 0x1001E
    gMalayLigMap.insert(std::make_pair( 0xE544, dvngLig( 136 ,"0x0D34 0x0D4D"))); // 0x1001F
    gMalayLigMap.insert(std::make_pair( 0xE545, dvngLig( 137 ,"0x0D35 0x0D4D"))); // 0x10020
    gMalayLigMap.insert(std::make_pair( 0xE546, dvngLig( 138 ,"0x0D36 0x0D4D"))); // 0x10021
    gMalayLigMap.insert(std::make_pair( 0xE547, dvngLig( 139 ,"0x0D37 0x0D4D"))); // 0x10022
    gMalayLigMap.insert(std::make_pair( 0xE548, dvngLig( 140 ,"0x0D38 0x0D4D"))); // 0x10023
    gMalayLigMap.insert(std::make_pair( 0xE549, dvngLig( 141 ,"0x0D39 0x0D4D"))); // 0x10024
    gMalayLigMap.insert(std::make_pair( 0xE54A, dvngLig( 142 ,"0x0D15 0x0D4D 0x0D37 0x0D4D"))); // 0x10025
    gMalayLigMap.insert(std::make_pair( 0xE54B, dvngLig( 143 ,"0x0D32 0x0D4D"))); // 0x10026
    gMalayLigMap.insert(std::make_pair( 0xE54C, dvngLig( 144 ,"0x0D2F 0x0D4D"))); // 0x10027
    gMalayLigMap.insert(std::make_pair( 0xE54D, dvngLig( 144 ,"0x0D4D 0x0D2F"))); // 0x10027
    //gMalayLigMap.insert(std::make_pair( 0xE54E, dvngLig( 145 ,"0x0D35 0x0D4D"))); // 0x10028
    gMalayLigMap.insert(std::make_pair( 0xE54F, dvngLig( 145 ,"0x0D4D 0x0D35"))); // 0x10028
    //gMalayLigMap.insert(std::make_pair( 0xE550, dvngLig( 146 ,"0x0D30 0x0D4D"))); // 0x10029
    gMalayLigMap.insert(std::make_pair( 0xE551, dvngLig( 146 ,"0x0D4D 0x0D30"))); // 0x10029        //virama Ra
    gMalayLigMap.insert(std::make_pair( 0xE552, dvngLig( 147 ,"0x0D15 0x0D4D 0x0D15"))); // 0x1002A
    gMalayLigMap.insert(std::make_pair( 0xE553, dvngLig( 148 ,"0x0D15 0x0D4D 0x0D24"))); // 0x1002B
    gMalayLigMap.insert(std::make_pair( 0xE554, dvngLig( 149 ,"0x0D15 0x200D 0x0D4D 0x0D1F"))); // 0x1002C
    gMalayLigMap.insert(std::make_pair( 0xE555, dvngLig( 149 ,"0x0D15 0x0D4D 0x0D1F"))); // 0x1002C
    gMalayLigMap.insert(std::make_pair( 0xE556, dvngLig( 150 ,"0x0D15 0x0D32 0x0D4D"))); // 0x1002D
    gMalayLigMap.insert(std::make_pair( 0xE557, dvngLig( 150 ,"0x0D15 0x0D4D 0x0D32"))); // 0x1002D
    gMalayLigMap.insert(std::make_pair( 0xE558, dvngLig( 151 ,"0x0D15 0x0D4D 0x0D37"))); // 0x1002E
    gMalayLigMap.insert(std::make_pair( 0xE559, dvngLig( 152 ,"0x0D17 0x0D4D 0x0D17"))); // 0x1002F
    gMalayLigMap.insert(std::make_pair( 0xE55A, dvngLig( 153 ,"0x0D17 0x200D 0x0D4D 0x0D26"))); // 0x10030
    gMalayLigMap.insert(std::make_pair( 0xE55B, dvngLig( 153 ,"0x0D17 0x0D4D 0x0D26"))); // 0x10030
    gMalayLigMap.insert(std::make_pair( 0xE55C, dvngLig( 154 ,"0x0D17 0x200D 0x0D4D 0x0D2E"))); // 0x10031
    gMalayLigMap.insert(std::make_pair( 0xE55D, dvngLig( 154 ,"0x0D17 0x0D4D 0x0D2E"))); // 0x10031
    gMalayLigMap.insert(std::make_pair( 0xE55E, dvngLig( 155 ,"0x0D17 0x200D 0x0D4D 0x0D28"))); // 0x10032
    gMalayLigMap.insert(std::make_pair( 0xE55F, dvngLig( 155 ,"0x0D17 0x0D4D 0x0D28"))); // 0x10032
    gMalayLigMap.insert(std::make_pair( 0xE560, dvngLig( 156 ,"0x0D17 0x0D32 0x0D4D"))); // 0x10033
    gMalayLigMap.insert(std::make_pair( 0xE561, dvngLig( 156 ,"0x0D17 0x0D4D 0x0D32"))); // 0x10033
    gMalayLigMap.insert(std::make_pair( 0xE562, dvngLig( 157 ,"0x0D19 0x0D4D 0x0D15"))); // 0x10034
    gMalayLigMap.insert(std::make_pair( 0xE563, dvngLig( 158 ,"0x0D19 0x0D4D 0x0D19"))); // 0x10035
    gMalayLigMap.insert(std::make_pair( 0xE564, dvngLig( 159 ,"0x0D1A 0x0D4D 0x0D1A"))); // 0x10036
    gMalayLigMap.insert(std::make_pair( 0xE565, dvngLig( 160 ,"0x0D1A 0x0D4D 0x0D1B"))); // 0x10037
    gMalayLigMap.insert(std::make_pair( 0xE566, dvngLig( 161 ,"0x0D1C 0x0D4D 0x0D1C"))); // 0x10038
    gMalayLigMap.insert(std::make_pair( 0xE567, dvngLig( 162 ,"0x0D1C 0x0D4D 0x0D1E"))); // 0x10039
    gMalayLigMap.insert(std::make_pair( 0xE568, dvngLig( 163 ,"0x0D1E 0x0D4D 0x0D1A"))); // 0x1003A
    gMalayLigMap.insert(std::make_pair( 0xE569, dvngLig( 164 ,"0x0D1E 0x200D 0x0D4D 0x0D1B"))); // 0x1003B
    gMalayLigMap.insert(std::make_pair( 0xE56A, dvngLig( 165 ,"0x0D1E 0x0D4D 0x0D1C"))); // 0x1003C
    gMalayLigMap.insert(std::make_pair( 0xE56B, dvngLig( 166 ,"0x0D1E 0x0D4D 0x0D1E"))); // 0x1003D
    gMalayLigMap.insert(std::make_pair( 0xE56C, dvngLig( 167 ,"0x0D1F 0x0D4D 0x0D1F"))); // 0x1003E
    gMalayLigMap.insert(std::make_pair( 0xE56D, dvngLig( 168 ,"0x0D21 0x0D4D 0x0D21"))); // 0x1003F
    gMalayLigMap.insert(std::make_pair( 0xE56E, dvngLig( 169 ,"0x0D21 0x0D4D 0x0D22"))); // 0x10040
    gMalayLigMap.insert(std::make_pair( 0xE56F, dvngLig( 170 ,"0x0D23 0x0D4D 0x0D21"))); // 0x10041
    gMalayLigMap.insert(std::make_pair( 0xE570, dvngLig( 171 ,"0x0D23 0x200D 0x0D4D 0x0D22"))); // 0x10042
    gMalayLigMap.insert(std::make_pair( 0xE571, dvngLig( 172 ,"0x0D23 0x0D4D 0x0D2E"))); // 0x10043
    gMalayLigMap.insert(std::make_pair( 0xE572, dvngLig( 173 ,"0x0D23 0x0D4D 0x0D23"))); // 0x10044
    gMalayLigMap.insert(std::make_pair( 0xE573, dvngLig( 174 ,"0x0D23 0x0D4D 0x0D1F"))); // 0x10045
    gMalayLigMap.insert(std::make_pair( 0xE574, dvngLig( 175 ,"0x0D24 0x0D4D 0x0D28"))); // 0x10046
    gMalayLigMap.insert(std::make_pair( 0xE575, dvngLig( 176 ,"0x0D24 0x200D 0x0D4D 0x0D2D"))); // 0x10047
    gMalayLigMap.insert(std::make_pair( 0xE576, dvngLig( 176 ,"0x0D24 0x0D4D 0x0D2D"))); // 0x10047
    gMalayLigMap.insert(std::make_pair( 0xE577, dvngLig( 177 ,"0x0D24 0x0D4D 0x0D2E"))); // 0x10048
    gMalayLigMap.insert(std::make_pair( 0xE578, dvngLig( 178 ,"0x0D24 0x0D4D 0x0D38"))); // 0x10049
    gMalayLigMap.insert(std::make_pair( 0xE579, dvngLig( 179 ,"0x0D24 0x0D4D 0x0D24"))); // 0x1004A
    gMalayLigMap.insert(std::make_pair( 0xE57A, dvngLig( 180 ,"0x0D24 0x0D4D 0x0D25"))); // 0x1004B
    gMalayLigMap.insert(std::make_pair( 0xE57B, dvngLig( 181 ,"0x0D24 0x0D32 0x0D4D"))); // 0x1004C
    gMalayLigMap.insert(std::make_pair( 0xE57C, dvngLig( 181 ,"0x0D24 0x0D4D 0x0D32"))); // 0x1004C
    gMalayLigMap.insert(std::make_pair( 0xE57D, dvngLig( 182 ,"0x0D26 0x0D4D 0x0D26"))); // 0x1004D
    gMalayLigMap.insert(std::make_pair( 0xE57E, dvngLig( 183 ,"0x0D26 0x0D4D 0x0D27"))); // 0x1004E
    gMalayLigMap.insert(std::make_pair( 0xE57F, dvngLig( 184 ,"0x0D28 0x0D4D 0x0D26"))); // 0x1004F
    gMalayLigMap.insert(std::make_pair( 0xE580, dvngLig( 185 ,"0x0D28 0x0D4D 0x0D27"))); // 0x10050
    gMalayLigMap.insert(std::make_pair( 0xE581, dvngLig( 186 ,"0x0D28 0x0D4D 0x0D28"))); // 0x10051
    gMalayLigMap.insert(std::make_pair( 0xE582, dvngLig( 187 ,"0x0D28 0x0D4D 0x0D2E"))); // 0x10052
    gMalayLigMap.insert(std::make_pair( 0xE583, dvngLig( 188 ,"0x0D7B 0x0D4D 0x0D31"))); // 0x10053
    gMalayLigMap.insert(std::make_pair( 0xE584, dvngLig( 188 ,"0x0D28 0x0D4D 0x200D 0x0D31"))); // 0x10053
    gMalayLigMap.insert(std::make_pair( 0xE585, dvngLig( 188 ,"0x0D28 0x0D4D 0x0D31"))); // 0x10053
    gMalayLigMap.insert(std::make_pair( 0xE586, dvngLig( 189 ,"0x0D28 0x0D4D 0x0D24"))); // 0x10054
    gMalayLigMap.insert(std::make_pair( 0xE587, dvngLig( 190 ,"0x0D28 0x200D 0x0D4D 0x0D25"))); // 0x10055
    gMalayLigMap.insert(std::make_pair( 0xE588, dvngLig( 190 ,"0x0D28 0x0D4D 0x0D25"))); // 0x10055
    gMalayLigMap.insert(std::make_pair( 0xE589, dvngLig( 191 ,"0x0D2A 0x0D4D 0x0D2A"))); // 0x10056
    gMalayLigMap.insert(std::make_pair( 0xE58A, dvngLig( 192 ,"0x0D2A 0x0D32 0x0D4D"))); // 0x10057
    gMalayLigMap.insert(std::make_pair( 0xE58B, dvngLig( 192 ,"0x0D2A 0x0D4D 0x0D32"))); // 0x10057
    gMalayLigMap.insert(std::make_pair( 0xE58C, dvngLig( 193 ,"0x0D2B 0x0D32 0x0D4D"))); // 0x10058
    gMalayLigMap.insert(std::make_pair( 0xE58D, dvngLig( 193 ,"0x0D2B 0x0D4D 0x0D32"))); // 0x10058
    gMalayLigMap.insert(std::make_pair( 0xE58E, dvngLig( 194 ,"0x0D2C 0x0D4D 0x0D2C"))); // 0x10059
    gMalayLigMap.insert(std::make_pair( 0xE58F, dvngLig( 195 ,"0x0D2C 0x200D 0x0D4D 0x0D26"))); // 0x1005A
    gMalayLigMap.insert(std::make_pair( 0xE590, dvngLig( 196 ,"0x0D2C 0x200D 0x0D4D 0x0D27"))); // 0x1005B
    gMalayLigMap.insert(std::make_pair( 0xE591, dvngLig( 197 ,"0x0D2C 0x0D32 0x0D4D"))); // 0x1005C
    gMalayLigMap.insert(std::make_pair( 0xE592, dvngLig( 197 ,"0x0D2C 0x0D4D 0x0D32"))); // 0x1005C
    gMalayLigMap.insert(std::make_pair( 0xE593, dvngLig( 198 ,"0x0D2E 0x0D4D 0x0D2E"))); // 0x1005D
    gMalayLigMap.insert(std::make_pair( 0xE594, dvngLig( 199 ,"0x0D2E 0x0D4D 0x0D2A"))); // 0x1005E
    gMalayLigMap.insert(std::make_pair( 0xE595, dvngLig( 200 ,"0x0D2E 0x0D4D 0x0D2A 0x0D32 0x0D4D"))); // 0x1005F
    gMalayLigMap.insert(std::make_pair( 0xE596, dvngLig( 200 ,"0x0D2E 0x0D4D 0x0D2A 0x0D4D 0x0D32"))); // 0x1005F
    gMalayLigMap.insert(std::make_pair( 0xE597, dvngLig( 201 ,"0x0D2E 0x0D32 0x0D4D"))); // 0x10060
    gMalayLigMap.insert(std::make_pair( 0xE598, dvngLig( 201 ,"0x0D2E 0x0D4D 0x0D32"))); // 0x10060
    gMalayLigMap.insert(std::make_pair( 0xE599, dvngLig( 202 ,"0x0D2F 0x0D2F 0x0D4D"))); // 0x10061
    gMalayLigMap.insert(std::make_pair( 0xE59A, dvngLig( 202 ,"0x0D2F 0x0D4D 0x0D2F"))); // 0x10061
    gMalayLigMap.insert(std::make_pair( 0xE59B, dvngLig( 203 ,"0x0D31 0x0D4D 0x0D31"))); // 0x10062
    gMalayLigMap.insert(std::make_pair( 0xE59C, dvngLig( 204 ,"0x0D32 0x200D 0x0D4D 0x0D2A"))); // 0x10063
    gMalayLigMap.insert(std::make_pair( 0xE59D, dvngLig( 204 ,"0x0D32 0x0D4D 0x0D2A"))); // 0x10063
    gMalayLigMap.insert(std::make_pair( 0xE59E, dvngLig( 205 ,"0x0D32 0x0D4D 0x0D32"))); // 0x10064
    gMalayLigMap.insert(std::make_pair( 0xE59F, dvngLig( 205 ,"0x0D32 0x0D32 0x0D4D"))); // 0x10064
    gMalayLigMap.insert(std::make_pair( 0xE5A0, dvngLig( 206 ,"0x0D33 0x0D4D 0x0D33"))); // 0x10065
    gMalayLigMap.insert(std::make_pair( 0xE5A1, dvngLig( 207 ,"0x0D35 0x0D32 0x0D4D"))); // 0x10066
    gMalayLigMap.insert(std::make_pair( 0xE5A2, dvngLig( 207 ,"0x0D35 0x0D4D 0x0D32"))); // 0x10066
    gMalayLigMap.insert(std::make_pair( 0xE5A3, dvngLig( 208 ,"0x0D35 0x0D35 0x0D4D"))); // 0x10067
    gMalayLigMap.insert(std::make_pair( 0xE5A4, dvngLig( 208 ,"0x0D35 0x0D4D 0x0D35"))); // 0x10067
    gMalayLigMap.insert(std::make_pair( 0xE5A5, dvngLig( 209 ,"0x0D36 0x0D4D 0x0D1A"))); // 0x10068
    gMalayLigMap.insert(std::make_pair( 0xE5A6, dvngLig( 210 ,"0x0D36 0x200D 0x0D4D 0x0D1B"))); // 0x10069
    gMalayLigMap.insert(std::make_pair( 0xE5A7, dvngLig( 211 ,"0x0D36 0x0D4D 0x0D36"))); // 0x1006A
    gMalayLigMap.insert(std::make_pair( 0xE5A8, dvngLig( 212 ,"0x0D36 0x0D32 0x0D4D"))); // 0x1006B
    gMalayLigMap.insert(std::make_pair( 0xE5A9, dvngLig( 212 ,"0x0D36 0x0D4D 0x0D32"))); // 0x1006B
    gMalayLigMap.insert(std::make_pair( 0xE5AA, dvngLig( 213 ,"0x0D37 0x200D 0x0D4D 0x0D1F"))); // 0x1006C
    gMalayLigMap.insert(std::make_pair( 0xE5AB, dvngLig( 214 ,"0x0D38 0x0D4D 0x0D31 0x0D4D 0x0D31"))); // 0x1006D
    gMalayLigMap.insert(std::make_pair( 0xE5AC, dvngLig( 215 ,"0x0D38 0x0D4D 0x0D38"))); // 0x1006E
    gMalayLigMap.insert(std::make_pair( 0xE5AD, dvngLig( 216 ,"0x0D38 0x200D 0x0D4D 0x0D25"))); // 0x1006F
    gMalayLigMap.insert(std::make_pair( 0xE5AE, dvngLig( 216 ,"0x0D38 0x0D4D 0x0D25"))); // 0x1006F
    gMalayLigMap.insert(std::make_pair( 0xE5AF, dvngLig( 217 ,"0x0D38 0x0D32 0x0D4D"))); // 0x10070
    gMalayLigMap.insert(std::make_pair( 0xE5B0, dvngLig( 217 ,"0x0D38 0x0D4D 0x0D32"))); // 0x10070
    gMalayLigMap.insert(std::make_pair( 0xE5B1, dvngLig( 218 ,"0x0D39 0x200D 0x0D4D 0x0D2E"))); // 0x10071
    gMalayLigMap.insert(std::make_pair( 0xE5B2, dvngLig( 218 ,"0x0D39 0x0D4D 0x0D2E"))); // 0x10071
    gMalayLigMap.insert(std::make_pair( 0xE5B3, dvngLig( 219 ,"0x0D39 0x200D 0x0D4D 0x0D28"))); // 0x10072
    gMalayLigMap.insert(std::make_pair( 0xE5B4, dvngLig( 219 ,"0x0D39 0x0D4D 0x0D28"))); // 0x10072
    gMalayLigMap.insert(std::make_pair( 0xE5B5, dvngLig( 220 ,"0x0D39 0x0D32 0x0D4D"))); // 0x10073
    gMalayLigMap.insert(std::make_pair( 0xE5B6, dvngLig( 220 ,"0x0D39 0x0D4D 0x0D32"))); // 0x10073
    gMalayLigMap.insert(std::make_pair( 0xE5B7, dvngLig( 234 ,"0x0D1F 0x0D62"))); // 0x10081
    gMalayLigMap.insert(std::make_pair( 0xE5B8, dvngLig( 235 ,"0x0D20 0x0D62"))); // 0x10082
    gMalayLigMap.insert(std::make_pair( 0xE5B9, dvngLig( 236 ,"0x0D26 0x0D62"))); // 0x10083
    gMalayLigMap.insert(std::make_pair( 0xE5BA, dvngLig( 237 ,"0x0D2D 0x0D62"))); // 0x10084
    gMalayLigMap.insert(std::make_pair( 0xE5BB, dvngLig( 238 ,"0x0D2E 0x0D62"))); // 0x10085
    gMalayLigMap.insert(std::make_pair( 0xE5BC, dvngLig( 239 ,"0x0D30 0x0D62"))); // 0x10086
    gMalayLigMap.insert(std::make_pair( 0xE5BD, dvngLig( 240 ,"0x0D31 0x0D62"))); // 0x10087
    gMalayLigMap.insert(std::make_pair( 0xE5BE, dvngLig( 241 ,"0x0D34 0x0D62"))); // 0x10088
    gMalayLigMap.insert(std::make_pair( 0xE5BF, dvngLig( 242 ,"0x0D1F 0x0D63"))); // 0x10089
    gMalayLigMap.insert(std::make_pair( 0xE5C0, dvngLig( 243 ,"0x0D20 0x0D63"))); // 0x1008A
    gMalayLigMap.insert(std::make_pair( 0xE5C1, dvngLig( 244 ,"0x0D26 0x0D63"))); // 0x1008B
    gMalayLigMap.insert(std::make_pair( 0xE5C2, dvngLig( 245 ,"0x0D2D 0x0D63"))); // 0x1008C
    gMalayLigMap.insert(std::make_pair( 0xE5C3, dvngLig( 246 ,"0x0D2E 0x0D63"))); // 0x1008D
    gMalayLigMap.insert(std::make_pair( 0xE5C4, dvngLig( 247 ,"0x0D30 0x0D63"))); // 0x1008E
    gMalayLigMap.insert(std::make_pair( 0xE5C5, dvngLig( 248 ,"0x0D31 0x0D63"))); // 0x1008F
    gMalayLigMap.insert(std::make_pair( 0xE5C6, dvngLig( 249 ,"0x0D34 0x0D63"))); // 0x10090
    gMalayLigMap.insert(std::make_pair( 0xE5C7, dvngLig( 304 ,"0x0D17 0x0307"))); // 0x10091
    gMalayLigMap.insert(std::make_pair( 0xE5C8, dvngLig( 305 ,"0x0D17 0x0323"))); // 0x10092
    gMalayLigMap.insert(std::make_pair( 0xE5C9, dvngLig( 306 ,"0x0D27 0x0307"))); // 0x10093
    gMalayLigMap.insert(std::make_pair( 0xE5CA, dvngLig( 307 ,"0x0D27 0x0323"))); // 0x10094
    gMalayLigMap.insert(std::make_pair( 0xE5CB, dvngLig( 308 ,"0x0D28 0x0307 0x0D3F"))); // 0x10095
    gMalayLigMap.insert(std::make_pair( 0xE5CC, dvngLig( 308 ,"0x0D28 0x0D3F 0x0307"))); // 0x10095
    gMalayLigMap.insert(std::make_pair( 0xE5CD, dvngLig( 309 ,"0x0D28 0x0323 0x0D3F"))); // 0x10096
    gMalayLigMap.insert(std::make_pair( 0xE5CE, dvngLig( 309 ,"0x0D28 0x0D3F 0x0323"))); // 0x10096
    gMalayLigMap.insert(std::make_pair( 0xE5CF, dvngLig( 310 ,"0x0D2A 0x0307"))); // 0x10097
    gMalayLigMap.insert(std::make_pair( 0xE5D0, dvngLig( 311 ,"0x0D2A 0x0323"))); // 0x10098
    gMalayLigMap.insert(std::make_pair( 0xE5D1, dvngLig( 312 ,"0x0D2E 0x0307"))); // 0x10099
    gMalayLigMap.insert(std::make_pair( 0xE5D2, dvngLig( 313 ,"0x0D2E 0x0323"))); // 0x1009A
    gMalayLigMap.insert(std::make_pair( 0xE5D3, dvngLig( 314 ,"0x0D30 0x0307 0x0D3F"))); // 0x1009B
    gMalayLigMap.insert(std::make_pair( 0xE5D4, dvngLig( 314 ,"0x0D30 0x0D3F 0x0307"))); // 0x1009B
    gMalayLigMap.insert(std::make_pair( 0xE5D5, dvngLig( 315 ,"0x0D30 0x0323 0x0D3F"))); // 0x1009C
    gMalayLigMap.insert(std::make_pair( 0xE5D6, dvngLig( 315 ,"0x0D30 0x0D3F 0x0323"))); // 0x1009C
    gMalayLigMap.insert(std::make_pair( 0xE5D7, dvngLig( 316 ,"0x0D38 0x0307"))); // 0x1009D
    gMalayLigMap.insert(std::make_pair( 0xE5D8, dvngLig( 317 ,"0x0D38 0x0323"))); // 0x1009E
    }
    return gMalayLigMap;
}


void SwitchMalayE(lString16* str)
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
        if (str->at(i) == 0x0D46)
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
                str->at(i - 1) = 0x0D46;
            }
        }
    }
}

void SwitchMalayEE(lString16 *str)
{
//0x0D47
/*    ( ? 0xE54D EE)   -> ( EE ? 0xE54D)
 *    ( ? 0xE54F EE)   -> ( EE ? 0xE54F)
 *    ( ? 0xE551 EE)   -> ( EE ? 0xE551)
 *    ( ? EE )         -> ( EE ? )
 */
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0D47)
        {
            if(str->at(i - 1) == 0xE54D ||
               str->at(i - 1) == 0xE54F ||
               str->at(i - 1) == 0xE551)
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x0D47;
            }
            else
            {
                //default
                str->at(i + 0) = str->at(i - 1);
                str->at(i - 1) = 0x0D47;
            }
        }
    }
}

void SwitchMalayAI(lString16* str)
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
        if (str->at(i) == 0x0D48)
        {
            str->at(i) = str->at(i - 1);
            str->at(i - 1) = 0x0D48;
        }
    }
}

void SwitchMalayO(lString16 *str)
{
/* //0x0D4A
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
        if (str->at(i) == 0x0D4A)
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
                str->at(i - 1) = 0x0D46;
                str->insert(i + 1, 1, 0x0D3E);
            }
        }
    }
}

void SwitchMalayOO(lString16 *str)
{
/* //0x0D4B
 * (? O)   -> (e ? aa)
 * (? ' O) -> (e ? ' aa) // ' == 0xE54D
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x0D4B)
        {
            if (i > 1 && str->at(i - 1) == 0xE54D)
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x0D47;
                str->insert(i + 1, 1, 0x0D3E);
            }
            else
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x0D47;
                str->insert(i + 1, 1, 0x0D3E);
            }
        }
    }
}

void SwitchMalayAU(lString16 *str)
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
        if (str->at(i) == 0x0D4C)
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
                str->at(i - 1) = 0x0D46;
                str->insert(i + 1, 1, 0x0D57);
            }
        }
    }
}

void SwitchMalayViramaRa(lString16 *str)
{
/* //0xE551
 * (? Vr)   -> (Vr ?)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0xE551)
        {
            str->at(i) = str->at(i - 1);
            str->at(i - 1) = 0xE551;
        }
    }
}

void SwitchMalayO_reverse(lString16* str)
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
        if(str->at(i) == 0x0D46)
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
            if (str->at(i + 2) == 0x0D3E)
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x0D4A;
                str->erase(i + 2);
            }
        }
    }
}

void SwitchMalayOO_reverse(lString16* str)
{
/*  //0x0D4B
 *  (e ? aa)   -> (? OO)
 *  (e ? ' aa) -> (? ' OO)  // ' == 0xE54D
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0D47)
        {
            if(str->at(i + 3) == 0x0D3E && str->at(i + 2) == 0xE54D)
            {
                str->at(i) = str->at(i+1);
                str->at(i + 1) = str->at(i+2);
                str->at(i + 2) = 0x0D4B;
                str->erase(i + 3);
            }
            else if (str->at(i + 2) == 0x0D3E)
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x0D4B;
                str->erase(i + 2);
            }
        }
    }
}

void SwitchMalayAU_reverse(lString16* str)
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
        if(str->at(i) == 0x0D46)
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
            if (str->at(i + 2) == 0x0D57)
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x0D4C;
                str->erase(i + 2);
            }
        }
    }
}

void SwitchMalayE_reverse(lString16* str)
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
        if(str->at(i) == 0x0D46)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0x0D46;
        }
    }
}

void SwitchMalayEE_reverse(lString16* str)
{
/* //0x0D47
 * ( EE ? 0xE54D) -> ( ? 0xE54D EE)
 * ( EE ? 0xE54F) -> ( ? 0xE54F EE)
 * ( EE ? 0xE551) -> ( ? 0xE551 EE)
 * ( EE ? )       -> ( ? EE )
 */
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0D47)
        {
            if(str->at(i+2) == 0xE54D ||
               str->at(i+2) == 0xE54F ||
               str->at(i+2) == 0xE551 )
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = 0x0D47;
            }
            else
            {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0x0D47;
            }
        }
    }
}

void SwitchMalayAI_reverse(lString16* str)
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
        if(str->at(i) == 0x0D48)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0x0D48;
        }
    }
}

void SwitchMalayViramaRa_reverse(lString16* str)
{
/* //0xE551
 * (Vr ?)   -> (? Vr)
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x0D48)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0x0D48;
        }
    }
}

lString16 processMalayLigatures(lString16 word)
{
    int j = (word.length() >= 7 ) ? 7 : word.length();
    for(;  j >= 2; j--)
    {
        for (int c = word.length() - j; c >= 0; c--)
        {
            lUInt32 fastkey = (word.at(c) << 16) + word.at(c + 1);
            if (gMalayFastLigMap.find(fastkey) == gMalayFastLigMap.end())
            {
                continue;
            }
            dvngLig lig(word.substr(c, j));
            lChar16 rep = findMalayLigRev(lig);
/*
            if(isViramaComboMalay(rep))
            {
                if (c == word.length() - j)
                {
                    if (rep == 0xE226 ) rep = 0xE266;
                    if (rep == 0xE225 ) rep = 0xE24C;
                }
                else
                {
                    if (rep == 0xE266 ) rep = 0xE226;
                    if (rep == 0xE24C ) rep = 0xE225;
                }
            }
            //avoiding ra+virama and ya_postform conflict
            if (rep == 0xE272 && c>0 && (word.at(c-1) == 0x09B0 || word.at(c-1) == 0x09F0))
            {
                rep = 0;
            }
            */
            if (rep != 0)
            {
                //if(lig.banglaRa && c+j < word.length() && isBangla_consonant(word.at(c+j)) )
                //{
                //    //LE("rep = 0x%04X, char+1 = 0x%04X",rep,word.at(c+j));
                //    continue;
                //}
                //LE("found!!, char = 0x%X",rep);
                word.replace(c, j, lString16(&rep, 1));
                c -= j - 2;
            }
        }
    }
    return word;
}

lString16 lString16::processMalayText()
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
        word = processMalayLigatures(word);

        SwitchMalayViramaRa(&word);

        SwitchMalayE(&word);
        SwitchMalayEE(&word);
        SwitchMalayAI(&word);
        SwitchMalayO(&word);
        SwitchMalayOO(&word);
        SwitchMalayAU(&word);
        //SwitchBanglaRaITa(&word);
//
        //StripZWNJ(&word);

        res.append(word);
        res.append(L" ");
    }
    res.substr(0,res.size()-1);
    return res;
}

lString16 restoreMalayWord(lString16 in)
{
    if(MALAY_DISPLAY_ENABLE == 0 || gDocumentMalay == 0)
    {
        return in;
    }
   //lString16::StripZWNJ_reverse(&in);
   //lString16::SwitchBanglaRaITa_reverse(&in);
   //lString16::SwitchBanglaOU_reverse(&in);
   //lString16::SwitchBanglaO_reverse(&in)
    SwitchMalayAU_reverse(&in);
    SwitchMalayOO_reverse(&in);
    SwitchMalayO_reverse(&in);
    SwitchMalayAI_reverse(&in);
    SwitchMalayEE_reverse(&in);
    SwitchMalayE_reverse(&in);

    SwitchMalayViramaRa_reverse(&in);
    for (int i = 0; i < in.length(); i++)
    {
        if (in[i] < MALAY_START || in[i] > MALAY_END)
        {
            continue;
        }

        dvngLig lig = findMalayLig(in[i]);
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
