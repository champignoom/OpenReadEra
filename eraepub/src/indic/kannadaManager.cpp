//
// Created by Tarasus on 16.10.2020.
//

#include "include/indic/kannadaManager.h"
#include "ore_log.h"
#include <vector>

LigMap     gKannadaLigMap;
LigMapRev  gKannadaLigMapRev;
FastLigMap gKannadaFastLigMap;

bool CharIsKannada(int ch)
{
    return ((ch >= 0x0C80 && ch <= 0x0CF2));
}

bool lString16::CheckKannada()
{
    if(gDocumentKannada == 1)
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
        if (CharIsKannada(ch))
        {
            gDocumentKannada = 1;
            gDocumentINDIC = 1;

            if(gKannadaLigMapRev.empty())
            {
                gKannadaLigMapRev = KannadaLigMapReversed();
            }
            return true;
        }
    }
    return false;
}

dvngLig findKannadaLig(lChar16 ligature)
{
    if(gKannadaLigMap.empty())
    {
        gKannadaLigMap = GetKannadaLigMap();
    }
    LigMap::iterator it = gKannadaLigMap.find(ligature);
    if(it==gKannadaLigMap.end())
    {
        return dvngLig();
    }
    return it->second;
}

lChar16 findKannadaLigGlyphIndex(lChar16 ligature)
{
    LigMap::iterator it = gKannadaLigMap.find(ligature);
    if(it==gKannadaLigMap.end())
    {
        return 0;
    }
    return it->second.glyphindex;
}

lChar16 findKannadaLigRev(dvngLig combo)
{
    if(gKannadaLigMapRev.empty())
    {
        gKannadaLigMapRev = KannadaLigMapReversed();
    }
    if(combo.len < 2 || combo.len > 10 )
    {
        return 0;
    }
    auto it = gKannadaLigMapRev.find(combo);
    if(it==gKannadaLigMapRev.end())
    {
        return 0;
    }
    //LE("findKannadaLigRev return %d", it->second);
    return it->second;
}

std::map <dvngLig, lChar16, Comparator> KannadaLigMapReversed()
{
    if(!gKannadaLigMapRev.empty())
    {
        return gKannadaLigMapRev;
    }
    if(gKannadaLigMap.empty())
    {
        gKannadaLigMap = GetKannadaLigMap();
    }
    gKannadaLigMapRev = makeReverseLigMap(gKannadaLigMap,&gKannadaFastLigMap);
    return gKannadaLigMapRev;
}

LigMap GetKannadaLigMap()
{
    if(!gKannadaLigMap.empty())
    {
        return gKannadaLigMap;
    }
    //Kannada_START
    {
        //from Noto Sans kannada.ttf
        gKannadaLigMap.insert(std::make_pair( 0xE600, dvngLig( 66 ,"0x0CC6 0x0CD6"))); // 0x0CC8
        gKannadaLigMap.insert(std::make_pair( 0xE601, dvngLig( 67 ,"0x0CC6 0x0CC2"))); // 0x0CCA
        gKannadaLigMap.insert(std::make_pair( 0xE602, dvngLig( 94 ,"0x0CB0 0x0CCD"))); // 0x10001
        gKannadaLigMap.insert(std::make_pair( 0xE603, dvngLig( 95 ,"0x0C9C 0x0CBC"))); // 0x10002
        gKannadaLigMap.insert(std::make_pair( 0xE604, dvngLig( 96 ,"0x0CAB 0x0CBC"))); // 0x10003
        gKannadaLigMap.insert(std::make_pair( 0xE605, dvngLig( 97 ,"0x0CCD 0x0C95"))); // 0x10004
        gKannadaLigMap.insert(std::make_pair( 0xE606, dvngLig( 97 ,"0x0C95 0x0CCD"))); // 0x10004
        gKannadaLigMap.insert(std::make_pair( 0xE607, dvngLig( 98 ,"0x0CCD 0x0C96"))); // 0x10005
        gKannadaLigMap.insert(std::make_pair( 0xE608, dvngLig( 98 ,"0x0C96 0x0CCD"))); // 0x10005
        gKannadaLigMap.insert(std::make_pair( 0xE609, dvngLig( 99 ,"0x0CCD 0x0C97"))); // 0x10006
        gKannadaLigMap.insert(std::make_pair( 0xE60A, dvngLig( 99 ,"0x0C97 0x0CCD"))); // 0x10006
        gKannadaLigMap.insert(std::make_pair( 0xE60B, dvngLig( 100 ,"0x0CCD 0x0C98"))); // 0x10007
        gKannadaLigMap.insert(std::make_pair( 0xE60C, dvngLig( 100 ,"0x0C98 0x0CCD"))); // 0x10007
        gKannadaLigMap.insert(std::make_pair( 0xE60D, dvngLig( 101 ,"0x0CCD 0x0C99"))); // 0x10008
        gKannadaLigMap.insert(std::make_pair( 0xE60E, dvngLig( 101 ,"0x0C99 0x0CCD"))); // 0x10008
        gKannadaLigMap.insert(std::make_pair( 0xE60F, dvngLig( 102 ,"0x0CCD 0x0C9A"))); // 0x10009
        gKannadaLigMap.insert(std::make_pair( 0xE610, dvngLig( 102 ,"0x0C9A 0x0CCD"))); // 0x10009
        gKannadaLigMap.insert(std::make_pair( 0xE611, dvngLig( 103 ,"0x0CCD 0x0C9B"))); // 0x1000A
        gKannadaLigMap.insert(std::make_pair( 0xE612, dvngLig( 103 ,"0x0C9B 0x0CCD"))); // 0x1000A
        gKannadaLigMap.insert(std::make_pair( 0xE613, dvngLig( 104 ,"0x0CCD 0x0C9C"))); // 0x1000B
        gKannadaLigMap.insert(std::make_pair( 0xE614, dvngLig( 104 ,"0x0C9C 0x0CCD"))); // 0x1000B
        gKannadaLigMap.insert(std::make_pair( 0xE615, dvngLig( 105 ,"0x0CCD 0x0C9D"))); // 0x1000C
        gKannadaLigMap.insert(std::make_pair( 0xE616, dvngLig( 105 ,"0x0C9D 0x0CCD"))); // 0x1000C
        gKannadaLigMap.insert(std::make_pair( 0xE617, dvngLig( 106 ,"0x0CCD 0x0C9E"))); // 0x1000D
        gKannadaLigMap.insert(std::make_pair( 0xE618, dvngLig( 106 ,"0x0C9E 0x0CCD"))); // 0x1000D
        gKannadaLigMap.insert(std::make_pair( 0xE619, dvngLig( 107 ,"0x0CCD 0x0C9F"))); // 0x1000E
        gKannadaLigMap.insert(std::make_pair( 0xE61A, dvngLig( 107 ,"0x0C9F 0x0CCD"))); // 0x1000E
        gKannadaLigMap.insert(std::make_pair( 0xE61B, dvngLig( 108 ,"0x0CCD 0x0CA0"))); // 0x1000F
        gKannadaLigMap.insert(std::make_pair( 0xE61C, dvngLig( 108 ,"0x0CA0 0x0CCD"))); // 0x1000F
        gKannadaLigMap.insert(std::make_pair( 0xE61D, dvngLig( 109 ,"0x0CCD 0x0CA1"))); // 0x10010
        gKannadaLigMap.insert(std::make_pair( 0xE61E, dvngLig( 109 ,"0x0CA1 0x0CCD"))); // 0x10010
        gKannadaLigMap.insert(std::make_pair( 0xE61F, dvngLig( 110 ,"0x0CCD 0x0CA2"))); // 0x10011
        gKannadaLigMap.insert(std::make_pair( 0xE620, dvngLig( 110 ,"0x0CA2 0x0CCD"))); // 0x10011
        gKannadaLigMap.insert(std::make_pair( 0xE621, dvngLig( 111 ,"0x0CCD 0x0CA3"))); // 0x10012
        gKannadaLigMap.insert(std::make_pair( 0xE622, dvngLig( 111 ,"0x0CA3 0x0CCD"))); // 0x10012
        gKannadaLigMap.insert(std::make_pair( 0xE623, dvngLig( 112 ,"0x0CCD 0x0CA4"))); // 0x10013
        gKannadaLigMap.insert(std::make_pair( 0xE624, dvngLig( 112 ,"0x0CA4 0x0CCD"))); // 0x10013
        gKannadaLigMap.insert(std::make_pair( 0xE625, dvngLig( 113 ,"0x0CCD 0x0CA5"))); // 0x10014
        gKannadaLigMap.insert(std::make_pair( 0xE626, dvngLig( 113 ,"0x0CA5 0x0CCD"))); // 0x10014
        gKannadaLigMap.insert(std::make_pair( 0xE627, dvngLig( 114 ,"0x0CCD 0x0CA6"))); // 0x10015
        gKannadaLigMap.insert(std::make_pair( 0xE628, dvngLig( 114 ,"0x0CA6 0x0CCD"))); // 0x10015
        gKannadaLigMap.insert(std::make_pair( 0xE629, dvngLig( 115 ,"0x0CCD 0x0CA7"))); // 0x10016
        gKannadaLigMap.insert(std::make_pair( 0xE62A, dvngLig( 115 ,"0x0CA7 0x0CCD"))); // 0x10016
        gKannadaLigMap.insert(std::make_pair( 0xE62B, dvngLig( 116 ,"0x0CCD 0x0CA8"))); // 0x10017
        gKannadaLigMap.insert(std::make_pair( 0xE62C, dvngLig( 116 ,"0x0CA8 0x0CCD"))); // 0x10017
        gKannadaLigMap.insert(std::make_pair( 0xE62D, dvngLig( 117 ,"0x0CCD 0x0CAA"))); // 0x10018
        gKannadaLigMap.insert(std::make_pair( 0xE62E, dvngLig( 117 ,"0x0CAA 0x0CCD"))); // 0x10018
        gKannadaLigMap.insert(std::make_pair( 0xE62F, dvngLig( 118 ,"0x0CCD 0x0CAB"))); // 0x10019
        gKannadaLigMap.insert(std::make_pair( 0xE630, dvngLig( 118 ,"0x0CAB 0x0CCD"))); // 0x10019
        gKannadaLigMap.insert(std::make_pair( 0xE631, dvngLig( 119 ,"0x0CCD 0x0CAC"))); // 0x1001A
        gKannadaLigMap.insert(std::make_pair( 0xE632, dvngLig( 119 ,"0x0CAC 0x0CCD"))); // 0x1001A
        gKannadaLigMap.insert(std::make_pair( 0xE633, dvngLig( 120 ,"0x0CCD 0x0CAD"))); // 0x1001B
        gKannadaLigMap.insert(std::make_pair( 0xE634, dvngLig( 120 ,"0x0CAD 0x0CCD"))); // 0x1001B
        gKannadaLigMap.insert(std::make_pair( 0xE635, dvngLig( 121 ,"0x0CCD 0x0CAE"))); // 0x1001C
        gKannadaLigMap.insert(std::make_pair( 0xE636, dvngLig( 121 ,"0x0CAE 0x0CCD"))); // 0x1001C
        gKannadaLigMap.insert(std::make_pair( 0xE637, dvngLig( 122 ,"0x0CCD 0x0CAF"))); // 0x1001D
        gKannadaLigMap.insert(std::make_pair( 0xE638, dvngLig( 122 ,"0x0CAF 0x0CCD"))); // 0x1001D
        gKannadaLigMap.insert(std::make_pair( 0xE639, dvngLig( 123 ,"0x0CCD 0x0CB0"))); // 0x1001E
        gKannadaLigMap.insert(std::make_pair( 0xE63A, dvngLig( 123 ,"0x0CB0 0x0CCD"))); // 0x1001E
        gKannadaLigMap.insert(std::make_pair( 0xE63B, dvngLig( 124 ,"0x0CCD 0x0CB1"))); // 0x1001F
        gKannadaLigMap.insert(std::make_pair( 0xE63C, dvngLig( 124 ,"0x0CB1 0x0CCD"))); // 0x1001F
        gKannadaLigMap.insert(std::make_pair( 0xE63D, dvngLig( 125 ,"0x0CCD 0x0CB2"))); // 0x10020
        gKannadaLigMap.insert(std::make_pair( 0xE63E, dvngLig( 125 ,"0x0CB2 0x0CCD"))); // 0x10020
        gKannadaLigMap.insert(std::make_pair( 0xE63F, dvngLig( 126 ,"0x0CCD 0x0CB3"))); // 0x10021
        gKannadaLigMap.insert(std::make_pair( 0xE640, dvngLig( 126 ,"0x0CB3 0x0CCD"))); // 0x10021
        gKannadaLigMap.insert(std::make_pair( 0xE641, dvngLig( 127 ,"0x0CCD 0x0CB5"))); // 0x10022
        gKannadaLigMap.insert(std::make_pair( 0xE642, dvngLig( 127 ,"0x0CB5 0x0CCD"))); // 0x10022
        gKannadaLigMap.insert(std::make_pair( 0xE643, dvngLig( 128 ,"0x0CCD 0x0CB6"))); // 0x10023
        gKannadaLigMap.insert(std::make_pair( 0xE644, dvngLig( 128 ,"0x0CB6 0x0CCD"))); // 0x10023
        gKannadaLigMap.insert(std::make_pair( 0xE645, dvngLig( 129 ,"0x0CCD 0x0CB7"))); // 0x10024
        gKannadaLigMap.insert(std::make_pair( 0xE646, dvngLig( 129 ,"0x0CB7 0x0CCD"))); // 0x10024
        gKannadaLigMap.insert(std::make_pair( 0xE647, dvngLig( 130 ,"0x0CCD 0x0CB8"))); // 0x10025
        gKannadaLigMap.insert(std::make_pair( 0xE648, dvngLig( 130 ,"0x0CB8 0x0CCD"))); // 0x10025
        gKannadaLigMap.insert(std::make_pair( 0xE649, dvngLig( 131 ,"0x0CCD 0x0CB9"))); // 0x10026
        gKannadaLigMap.insert(std::make_pair( 0xE64A, dvngLig( 131 ,"0x0CB9 0x0CCD"))); // 0x10026
        gKannadaLigMap.insert(std::make_pair( 0xE64B, dvngLig( 132 ,"0x0CCD 0x0CDE"))); // 0x10027
        gKannadaLigMap.insert(std::make_pair( 0xE64C, dvngLig( 132 ,"0x0CDE 0x0CCD"))); // 0x10027
        gKannadaLigMap.insert(std::make_pair( 0xE64D, dvngLig( 167 ,"0x0C95 0x0CCD"))); // 0x1004A
        gKannadaLigMap.insert(std::make_pair( 0xE64E, dvngLig( 167 ,"0x0C95 0x0CCD 0x200D"))); // 0x1004A
        gKannadaLigMap.insert(std::make_pair( 0xE64F, dvngLig( 168 ,"0x0C96 0x0CCD"))); // 0x1004B
        gKannadaLigMap.insert(std::make_pair( 0xE650, dvngLig( 168 ,"0x0C96 0x0CCD 0x200D"))); // 0x1004B
        gKannadaLigMap.insert(std::make_pair( 0xE651, dvngLig( 169 ,"0x0C97 0x0CCD"))); // 0x1004C
        gKannadaLigMap.insert(std::make_pair( 0xE652, dvngLig( 169 ,"0x0C97 0x0CCD 0x200D"))); // 0x1004C
        gKannadaLigMap.insert(std::make_pair( 0xE653, dvngLig( 170 ,"0x0C98 0x0CCD"))); // 0x1004D
        gKannadaLigMap.insert(std::make_pair( 0xE654, dvngLig( 170 ,"0x0C98 0x0CCD 0x200D"))); // 0x1004D
        gKannadaLigMap.insert(std::make_pair( 0xE655, dvngLig( 171 ,"0x0C99 0x0CCD"))); // 0x1004E
        gKannadaLigMap.insert(std::make_pair( 0xE656, dvngLig( 171 ,"0x0C99 0x0CCD 0x200D"))); // 0x1004E
        gKannadaLigMap.insert(std::make_pair( 0xE657, dvngLig( 172 ,"0x0C9A 0x0CCD"))); // 0x1004F
        gKannadaLigMap.insert(std::make_pair( 0xE658, dvngLig( 172 ,"0x0C9A 0x0CCD 0x200D"))); // 0x1004F
        gKannadaLigMap.insert(std::make_pair( 0xE659, dvngLig( 173 ,"0x0C9B 0x0CCD"))); // 0x10050
        gKannadaLigMap.insert(std::make_pair( 0xE65A, dvngLig( 173 ,"0x0C9B 0x0CCD 0x200D"))); // 0x10050
        gKannadaLigMap.insert(std::make_pair( 0xE65B, dvngLig( 174 ,"0x0C9C 0x0CCD"))); // 0x10051
        gKannadaLigMap.insert(std::make_pair( 0xE65C, dvngLig( 174 ,"0x0C9C 0x0CCD 0x200D"))); // 0x10051
        gKannadaLigMap.insert(std::make_pair( 0xE65D, dvngLig( 175 ,"0x0C9D 0x0CCD"))); // 0x10052
        gKannadaLigMap.insert(std::make_pair( 0xE65E, dvngLig( 175 ,"0x0C9D 0x0CCD 0x200D"))); // 0x10052
        gKannadaLigMap.insert(std::make_pair( 0xE65F, dvngLig( 176 ,"0x0C9E 0x0CCD"))); // 0x10053
        gKannadaLigMap.insert(std::make_pair( 0xE660, dvngLig( 176 ,"0x0C9E 0x0CCD 0x200D"))); // 0x10053
        gKannadaLigMap.insert(std::make_pair( 0xE661, dvngLig( 177 ,"0x0C9F 0x0CCD"))); // 0x10054
        gKannadaLigMap.insert(std::make_pair( 0xE662, dvngLig( 177 ,"0x0C9F 0x0CCD 0x200D"))); // 0x10054
        gKannadaLigMap.insert(std::make_pair( 0xE663, dvngLig( 178 ,"0x0CA0 0x0CCD"))); // 0x10055
        gKannadaLigMap.insert(std::make_pair( 0xE664, dvngLig( 178 ,"0x0CA0 0x0CCD 0x200D"))); // 0x10055
        gKannadaLigMap.insert(std::make_pair( 0xE665, dvngLig( 179 ,"0x0CA1 0x0CCD"))); // 0x10056
        gKannadaLigMap.insert(std::make_pair( 0xE666, dvngLig( 179 ,"0x0CA1 0x0CCD 0x200D"))); // 0x10056
        gKannadaLigMap.insert(std::make_pair( 0xE667, dvngLig( 180 ,"0x0CA2 0x0CCD"))); // 0x10057
        gKannadaLigMap.insert(std::make_pair( 0xE668, dvngLig( 180 ,"0x0CA2 0x0CCD 0x200D"))); // 0x10057
        gKannadaLigMap.insert(std::make_pair( 0xE669, dvngLig( 181 ,"0x0CA3 0x0CCD"))); // 0x10058
        gKannadaLigMap.insert(std::make_pair( 0xE66A, dvngLig( 181 ,"0x0CA3 0x0CCD 0x200D"))); // 0x10058
        gKannadaLigMap.insert(std::make_pair( 0xE66B, dvngLig( 182 ,"0x0CA4 0x0CCD"))); // 0x10059
        gKannadaLigMap.insert(std::make_pair( 0xE66C, dvngLig( 182 ,"0x0CA4 0x0CCD 0x200D"))); // 0x10059
        gKannadaLigMap.insert(std::make_pair( 0xE66D, dvngLig( 183 ,"0x0CA5 0x0CCD"))); // 0x1005A
        gKannadaLigMap.insert(std::make_pair( 0xE66E, dvngLig( 183 ,"0x0CA5 0x0CCD 0x200D"))); // 0x1005A
        gKannadaLigMap.insert(std::make_pair( 0xE66F, dvngLig( 184 ,"0x0CA6 0x0CCD"))); // 0x1005B
        gKannadaLigMap.insert(std::make_pair( 0xE670, dvngLig( 184 ,"0x0CA6 0x0CCD 0x200D"))); // 0x1005B
        gKannadaLigMap.insert(std::make_pair( 0xE671, dvngLig( 185 ,"0x0CA7 0x0CCD"))); // 0x1005C
        gKannadaLigMap.insert(std::make_pair( 0xE672, dvngLig( 185 ,"0x0CA7 0x0CCD 0x200D"))); // 0x1005C
        gKannadaLigMap.insert(std::make_pair( 0xE673, dvngLig( 186 ,"0x0CA8 0x0CCD"))); // 0x1005D
        gKannadaLigMap.insert(std::make_pair( 0xE674, dvngLig( 186 ,"0x0CA8 0x0CCD 0x200D"))); // 0x1005D
        gKannadaLigMap.insert(std::make_pair( 0xE675, dvngLig( 187 ,"0x0CAA 0x0CCD"))); // 0x1005E
        gKannadaLigMap.insert(std::make_pair( 0xE676, dvngLig( 187 ,"0x0CAA 0x0CCD 0x200D"))); // 0x1005E
        gKannadaLigMap.insert(std::make_pair( 0xE677, dvngLig( 188 ,"0x0CAB 0x0CCD"))); // 0x1005F
        gKannadaLigMap.insert(std::make_pair( 0xE678, dvngLig( 188 ,"0x0CAB 0x0CCD 0x200D"))); // 0x1005F
        gKannadaLigMap.insert(std::make_pair( 0xE679, dvngLig( 189 ,"0x0CAC 0x0CCD"))); // 0x10060
        gKannadaLigMap.insert(std::make_pair( 0xE67A, dvngLig( 189 ,"0x0CAC 0x0CCD 0x200D"))); // 0x10060
        gKannadaLigMap.insert(std::make_pair( 0xE67B, dvngLig( 190 ,"0x0CAD 0x0CCD"))); // 0x10061
        gKannadaLigMap.insert(std::make_pair( 0xE67C, dvngLig( 190 ,"0x0CAD 0x0CCD 0x200D"))); // 0x10061
        gKannadaLigMap.insert(std::make_pair( 0xE67D, dvngLig( 191 ,"0x0CAE 0x0CCD"))); // 0x10062
        gKannadaLigMap.insert(std::make_pair( 0xE67E, dvngLig( 191 ,"0x0CAE 0x0CCD 0x200D"))); // 0x10062
        gKannadaLigMap.insert(std::make_pair( 0xE67F, dvngLig( 192 ,"0x0CAF 0x0CCD"))); // 0x10063
        gKannadaLigMap.insert(std::make_pair( 0xE680, dvngLig( 192 ,"0x0CAF 0x0CCD 0x200D"))); // 0x10063
        gKannadaLigMap.insert(std::make_pair( 0xE681, dvngLig( 193 ,"0x0CB0 0x0CCD"))); // 0x10064
        gKannadaLigMap.insert(std::make_pair( 0xE682, dvngLig( 193 ,"0x0CB0 0x0CCD 0x200D"))); // 0x10064
        gKannadaLigMap.insert(std::make_pair( 0xE683, dvngLig( 194 ,"0x0CB1 0x0CCD"))); // 0x10065
        gKannadaLigMap.insert(std::make_pair( 0xE684, dvngLig( 194 ,"0x0CB1 0x0CCD 0x200D"))); // 0x10065
        gKannadaLigMap.insert(std::make_pair( 0xE685, dvngLig( 195 ,"0x0CB2 0x0CCD"))); // 0x10066
        gKannadaLigMap.insert(std::make_pair( 0xE686, dvngLig( 195 ,"0x0CB2 0x0CCD 0x200D"))); // 0x10066
        gKannadaLigMap.insert(std::make_pair( 0xE687, dvngLig( 196 ,"0x0CB3 0x0CCD"))); // 0x10067
        gKannadaLigMap.insert(std::make_pair( 0xE688, dvngLig( 196 ,"0x0CB3 0x0CCD 0x200D"))); // 0x10067
        gKannadaLigMap.insert(std::make_pair( 0xE689, dvngLig( 197 ,"0x0CB5 0x0CCD"))); // 0x10068
        gKannadaLigMap.insert(std::make_pair( 0xE68A, dvngLig( 197 ,"0x0CB5 0x0CCD 0x200D"))); // 0x10068
        gKannadaLigMap.insert(std::make_pair( 0xE68B, dvngLig( 198 ,"0x0CB6 0x0CCD"))); // 0x10069
        gKannadaLigMap.insert(std::make_pair( 0xE68C, dvngLig( 198 ,"0x0CB6 0x0CCD 0x200D"))); // 0x10069
        gKannadaLigMap.insert(std::make_pair( 0xE68D, dvngLig( 199 ,"0x0CB7 0x0CCD"))); // 0x1006A
        gKannadaLigMap.insert(std::make_pair( 0xE68E, dvngLig( 199 ,"0x0CB7 0x0CCD 0x200D"))); // 0x1006A
        gKannadaLigMap.insert(std::make_pair( 0xE68F, dvngLig( 200 ,"0x0CB8 0x0CCD"))); // 0x1006B
        gKannadaLigMap.insert(std::make_pair( 0xE690, dvngLig( 200 ,"0x0CB8 0x0CCD 0x200D"))); // 0x1006B
        gKannadaLigMap.insert(std::make_pair( 0xE691, dvngLig( 201 ,"0x0CB9 0x0CCD"))); // 0x1006C
        gKannadaLigMap.insert(std::make_pair( 0xE692, dvngLig( 201 ,"0x0CB9 0x0CCD 0x200D"))); // 0x1006C
        gKannadaLigMap.insert(std::make_pair( 0xE693, dvngLig( 202 ,"0x0CDE 0x0CCD"))); // 0x1006D
        gKannadaLigMap.insert(std::make_pair( 0xE694, dvngLig( 202 ,"0x0CDE 0x0CCD 0x200D"))); // 0x1006D
        gKannadaLigMap.insert(std::make_pair( 0xE695, dvngLig( 203 ,"0x0C9C 0x0CBC 0x0CCD"))); // 0x1006E
        gKannadaLigMap.insert(std::make_pair( 0xE696, dvngLig( 203 ,"0x0C9C 0x0CBC 0x0CCD 0x200D"))); // 0x1006E
        gKannadaLigMap.insert(std::make_pair( 0xE697, dvngLig( 204 ,"0x0CAB 0x0CBC 0x0CCD"))); // 0x1006F
        gKannadaLigMap.insert(std::make_pair( 0xE698, dvngLig( 204 ,"0x0CAB 0x0CBC 0x0CCD 0x200D"))); // 0x1006F
        gKannadaLigMap.insert(std::make_pair( 0xE699, dvngLig( 205 ,"0x0C95 0x0CBF"))); // 0x10070
        gKannadaLigMap.insert(std::make_pair( 0xE69A, dvngLig( 206 ,"0x0C96 0x0CBF"))); // 0x10071
        gKannadaLigMap.insert(std::make_pair( 0xE69B, dvngLig( 207 ,"0x0C97 0x0CBF"))); // 0x10072
        gKannadaLigMap.insert(std::make_pair( 0xE69C, dvngLig( 208 ,"0x0C98 0x0CBF"))); // 0x10073
        gKannadaLigMap.insert(std::make_pair( 0xE69D, dvngLig( 209 ,"0x0C9A 0x0CBF"))); // 0x10074
        gKannadaLigMap.insert(std::make_pair( 0xE69E, dvngLig( 210 ,"0x0C9B 0x0CBF"))); // 0x10075
        gKannadaLigMap.insert(std::make_pair( 0xE69F, dvngLig( 211 ,"0x0C9C 0x0CBF"))); // 0x10076
        gKannadaLigMap.insert(std::make_pair( 0xE6A0, dvngLig( 212 ,"0x0C9D 0x0CBF"))); // 0x10077
        gKannadaLigMap.insert(std::make_pair( 0xE6A1, dvngLig( 213 ,"0x0C9F 0x0CBF"))); // 0x10078
        gKannadaLigMap.insert(std::make_pair( 0xE6A2, dvngLig( 214 ,"0x0CA0 0x0CBF"))); // 0x10079
        gKannadaLigMap.insert(std::make_pair( 0xE6A3, dvngLig( 215 ,"0x0CA1 0x0CBF"))); // 0x1007A
        gKannadaLigMap.insert(std::make_pair( 0xE6A4, dvngLig( 216 ,"0x0CA2 0x0CBF"))); // 0x1007B
        gKannadaLigMap.insert(std::make_pair( 0xE6A5, dvngLig( 217 ,"0x0CA3 0x0CBF"))); // 0x1007C
        gKannadaLigMap.insert(std::make_pair( 0xE6A6, dvngLig( 218 ,"0x0CA4 0x0CBF"))); // 0x1007D
        gKannadaLigMap.insert(std::make_pair( 0xE6A7, dvngLig( 219 ,"0x0CA5 0x0CBF"))); // 0x1007E
        gKannadaLigMap.insert(std::make_pair( 0xE6A8, dvngLig( 220 ,"0x0CA6 0x0CBF"))); // 0x1007F
        gKannadaLigMap.insert(std::make_pair( 0xE6A9, dvngLig( 221 ,"0x0CA7 0x0CBF"))); // 0x10080
        gKannadaLigMap.insert(std::make_pair( 0xE6AA, dvngLig( 222 ,"0x0CA8 0x0CBF"))); // 0x10081
        gKannadaLigMap.insert(std::make_pair( 0xE6AB, dvngLig( 223 ,"0x0CAA 0x0CBF"))); // 0x10082
        gKannadaLigMap.insert(std::make_pair( 0xE6AC, dvngLig( 224 ,"0x0CAB 0x0CBF"))); // 0x10083
        gKannadaLigMap.insert(std::make_pair( 0xE6AD, dvngLig( 225 ,"0x0CAC 0x0CBF"))); // 0x10084
        gKannadaLigMap.insert(std::make_pair( 0xE6AE, dvngLig( 226 ,"0x0CAD 0x0CBF"))); // 0x10085
        gKannadaLigMap.insert(std::make_pair( 0xE6AF, dvngLig( 227 ,"0x0CAE 0x0CBF"))); // 0x10086
        gKannadaLigMap.insert(std::make_pair( 0xE6B0, dvngLig( 228 ,"0x0CAF 0x0CBF"))); // 0x10087
        gKannadaLigMap.insert(std::make_pair( 0xE6B1, dvngLig( 229 ,"0x0CB0 0x0CBF"))); // 0x10088
        gKannadaLigMap.insert(std::make_pair( 0xE6B2, dvngLig( 230 ,"0x0CB2 0x0CBF"))); // 0x10089
        gKannadaLigMap.insert(std::make_pair( 0xE6B3, dvngLig( 231 ,"0x0CB3 0x0CBF"))); // 0x1008A
        gKannadaLigMap.insert(std::make_pair( 0xE6B4, dvngLig( 232 ,"0x0CB5 0x0CBF"))); // 0x1008B
        gKannadaLigMap.insert(std::make_pair( 0xE6B5, dvngLig( 233 ,"0x0CB6 0x0CBF"))); // 0x1008C
        gKannadaLigMap.insert(std::make_pair( 0xE6B6, dvngLig( 234 ,"0x0CB7 0x0CBF"))); // 0x1008D
        gKannadaLigMap.insert(std::make_pair( 0xE6B7, dvngLig( 235 ,"0x0CB8 0x0CBF"))); // 0x1008E
        gKannadaLigMap.insert(std::make_pair( 0xE6B8, dvngLig( 236 ,"0x0CB9 0x0CBF"))); // 0x1008F
        gKannadaLigMap.insert(std::make_pair( 0xE6B9, dvngLig( 237 ,"0x0C9C 0x0CBC 0x0CBF"))); // 0x10090
        gKannadaLigMap.insert(std::make_pair( 0xE6BA, dvngLig( 238 ,"0x0CAB 0x0CBC 0x0CBF"))); // 0x10091
        gKannadaLigMap.insert(std::make_pair( 0xE6BB, dvngLig( 239 ,"0x0C95 0x0CC6"))); // 0x10092
        gKannadaLigMap.insert(std::make_pair( 0xE6BC, dvngLig( 240 ,"0x0C96 0x0CC6"))); // 0x10093
        gKannadaLigMap.insert(std::make_pair( 0xE6BD, dvngLig( 241 ,"0x0C97 0x0CC6"))); // 0x10094
        gKannadaLigMap.insert(std::make_pair( 0xE6BE, dvngLig( 242 ,"0x0C98 0x0CC6"))); // 0x10095
        gKannadaLigMap.insert(std::make_pair( 0xE6BF, dvngLig( 243 ,"0x0C9A 0x0CC6"))); // 0x10096
        gKannadaLigMap.insert(std::make_pair( 0xE6C0, dvngLig( 244 ,"0x0C9B 0x0CC6"))); // 0x10097
        gKannadaLigMap.insert(std::make_pair( 0xE6C1, dvngLig( 245 ,"0x0C9C 0x0CC6"))); // 0x10098
        gKannadaLigMap.insert(std::make_pair( 0xE6C2, dvngLig( 246 ,"0x0C9D 0x0CC6"))); // 0x10099
        gKannadaLigMap.insert(std::make_pair( 0xE6C3, dvngLig( 247 ,"0x0C9F 0x0CC6"))); // 0x1009A
        gKannadaLigMap.insert(std::make_pair( 0xE6C4, dvngLig( 248 ,"0x0CA0 0x0CC6"))); // 0x1009B
        gKannadaLigMap.insert(std::make_pair( 0xE6C5, dvngLig( 249 ,"0x0CA1 0x0CC6"))); // 0x1009C
        gKannadaLigMap.insert(std::make_pair( 0xE6C6, dvngLig( 250 ,"0x0CA2 0x0CC6"))); // 0x1009D
        gKannadaLigMap.insert(std::make_pair( 0xE6C7, dvngLig( 251 ,"0x0CA3 0x0CC6"))); // 0x1009E
        gKannadaLigMap.insert(std::make_pair( 0xE6C8, dvngLig( 252 ,"0x0CA4 0x0CC6"))); // 0x1009F
        gKannadaLigMap.insert(std::make_pair( 0xE6C9, dvngLig( 253 ,"0x0CA5 0x0CC6"))); // 0x100A0
        gKannadaLigMap.insert(std::make_pair( 0xE6CA, dvngLig( 254 ,"0x0CA6 0x0CC6"))); // 0x100A1
        gKannadaLigMap.insert(std::make_pair( 0xE6CB, dvngLig( 255 ,"0x0CA7 0x0CC6"))); // 0x100A2
        gKannadaLigMap.insert(std::make_pair( 0xE6CC, dvngLig( 256 ,"0x0CA8 0x0CC6"))); // 0x100A3
        gKannadaLigMap.insert(std::make_pair( 0xE6CD, dvngLig( 257 ,"0x0CAA 0x0CC6"))); // 0x100A4
        gKannadaLigMap.insert(std::make_pair( 0xE6CE, dvngLig( 258 ,"0x0CAB 0x0CC6"))); // 0x100A5
        gKannadaLigMap.insert(std::make_pair( 0xE6CF, dvngLig( 259 ,"0x0CAC 0x0CC6"))); // 0x100A6
        gKannadaLigMap.insert(std::make_pair( 0xE6D0, dvngLig( 260 ,"0x0CAD 0x0CC6"))); // 0x100A7
        gKannadaLigMap.insert(std::make_pair( 0xE6D1, dvngLig( 261 ,"0x0CAE 0x0CC6"))); // 0x100A8
        gKannadaLigMap.insert(std::make_pair( 0xE6D2, dvngLig( 262 ,"0x0CAF 0x0CC6"))); // 0x100A9
        gKannadaLigMap.insert(std::make_pair( 0xE6D3, dvngLig( 263 ,"0x0CB0 0x0CC6"))); // 0x100AA
        gKannadaLigMap.insert(std::make_pair( 0xE6D4, dvngLig( 264 ,"0x0CB2 0x0CC6"))); // 0x100AB
        gKannadaLigMap.insert(std::make_pair( 0xE6D5, dvngLig( 265 ,"0x0CB3 0x0CC6"))); // 0x100AC
        gKannadaLigMap.insert(std::make_pair( 0xE6D6, dvngLig( 266 ,"0x0CB5 0x0CC6"))); // 0x100AD
        gKannadaLigMap.insert(std::make_pair( 0xE6D7, dvngLig( 267 ,"0x0CB6 0x0CC6"))); // 0x100AE
        gKannadaLigMap.insert(std::make_pair( 0xE6D8, dvngLig( 268 ,"0x0CB7 0x0CC6"))); // 0x100AF
        gKannadaLigMap.insert(std::make_pair( 0xE6D9, dvngLig( 269 ,"0x0CB8 0x0CC6"))); // 0x100B0
        gKannadaLigMap.insert(std::make_pair( 0xE6DA, dvngLig( 270 ,"0x0CB9 0x0CC6"))); // 0x100B1
        gKannadaLigMap.insert(std::make_pair( 0xE6DB, dvngLig( 271 ,"0x0C9C 0x0CBC 0x0CC6"))); // 0x100B2
        gKannadaLigMap.insert(std::make_pair( 0xE6DC, dvngLig( 272 ,"0x0CAB 0x0CBC 0x0CC6"))); // 0x100B3
        gKannadaLigMap.insert(std::make_pair( 0xE6DD, dvngLig( 280 ,"0x0CAE 0x0CC6 0x0CC2"))); // 0x100BB
        gKannadaLigMap.insert(std::make_pair( 0xE6DE, dvngLig( 281 ,"0x0CAF 0x0CC6 0x0CC2"))); // 0x100BC
        gKannadaLigMap.insert(std::make_pair( 0xE6DF, dvngLig( 282 ,"0x0C95 0x0CCD 0x0CB7"))); // 0x100BD
        gKannadaLigMap.insert(std::make_pair( 0xE6E0, dvngLig( 282 ,"0x0C95 0x0CB7 0x0CCD"))); // 0x100BD
        gKannadaLigMap.insert(std::make_pair( 0xE6E1, dvngLig( 284 ,"0x0C95 0x0CCD 0x0CB7 0x0CCD"))); // 0x100BF
        gKannadaLigMap.insert(std::make_pair( 0xE6E2, dvngLig( 284 ,"0x0C95 0x0CB7 0x0CCD 0x0CCD"))); // 0x100BF
        gKannadaLigMap.insert(std::make_pair( 0xE6E3, dvngLig( 284 ,"0x0C95 0x0CCD 0x0CB7 0x0CCD 0x200D"))); // 0x100BF
        gKannadaLigMap.insert(std::make_pair( 0xE6E4, dvngLig( 284 ,"0x0C95 0x0CB7 0x0CCD 0x0CCD 0x200D"))); // 0x100BF
        gKannadaLigMap.insert(std::make_pair( 0xE6E5, dvngLig( 285 ,"0x0C95 0x0CCD 0x0CB7 0x0CBF"))); // 0x100C0
        gKannadaLigMap.insert(std::make_pair( 0xE6E6, dvngLig( 285 ,"0x0C95 0x0CB7 0x0CCD 0x0CBF"))); // 0x100C0
        gKannadaLigMap.insert(std::make_pair( 0xE6E7, dvngLig( 286 ,"0x0C95 0x0CCD 0x0CB7 0x0CC6"))); // 0x100C1
        gKannadaLigMap.insert(std::make_pair( 0xE6E8, dvngLig( 286 ,"0x0C95 0x0CB7 0x0CCD 0x0CC6"))); // 0x100C1
        gKannadaLigMap.insert(std::make_pair( 0xE6E9, dvngLig( 287 ,"0x0C9C 0x0CCD 0x0C9E"))); // 0x100C2
        gKannadaLigMap.insert(std::make_pair( 0xE6EA, dvngLig( 287 ,"0x0C9C 0x0C9E 0x0CCD"))); // 0x100C2
        gKannadaLigMap.insert(std::make_pair( 0xE6EB, dvngLig( 289 ,"0x0C9C 0x0CCD 0x0C9E 0x0CCD"))); // 0x100C4
        gKannadaLigMap.insert(std::make_pair( 0xE6EC, dvngLig( 289 ,"0x0C9C 0x0C9E 0x0CCD 0x0CCD"))); // 0x100C4
        gKannadaLigMap.insert(std::make_pair( 0xE6ED, dvngLig( 289 ,"0x0C9C 0x0CCD 0x0C9E 0x0CCD 0x200D"))); // 0x100C4
        gKannadaLigMap.insert(std::make_pair( 0xE6EE, dvngLig( 289 ,"0x0C9C 0x0C9E 0x0CCD 0x0CCD 0x200D"))); // 0x100C4
        gKannadaLigMap.insert(std::make_pair( 0xE6EF, dvngLig( 290 ,"0x0C9C 0x0CCD 0x0C9E 0x0CBF"))); // 0x100C5
        gKannadaLigMap.insert(std::make_pair( 0xE6F0, dvngLig( 290 ,"0x0C9C 0x0C9E 0x0CCD 0x0CBF"))); // 0x100C5
        gKannadaLigMap.insert(std::make_pair( 0xE6F1, dvngLig( 291 ,"0x0C9C 0x0CCD 0x0C9E 0x0CC6"))); // 0x100C6
        gKannadaLigMap.insert(std::make_pair( 0xE6F2, dvngLig( 291 ,"0x0C9C 0x0C9E 0x0CCD 0x0CC6"))); // 0x100C6
        gKannadaLigMap.insert(std::make_pair( 0xE6F3, dvngLig( 292 ,"0x0CCD 0x0C95 0x0CCD 0x0CB0"))); // 0x100C7
        gKannadaLigMap.insert(std::make_pair( 0xE6F4, dvngLig( 292 ,"0x0CCD 0x0C95 0x0CB0 0x0CCD"))); // 0x100C7
        gKannadaLigMap.insert(std::make_pair( 0xE6F5, dvngLig( 292 ,"0x0C95 0x0CCD 0x0CCD 0x0CB0"))); // 0x100C7
        gKannadaLigMap.insert(std::make_pair( 0xE6F6, dvngLig( 292 ,"0x0C95 0x0CCD 0x0CB0 0x0CCD"))); // 0x100C7
        gKannadaLigMap.insert(std::make_pair( 0xE6F7, dvngLig( 293 ,"0x0CCD 0x0C97 0x0CCD 0x0CB0"))); // 0x100C8
        gKannadaLigMap.insert(std::make_pair( 0xE6F8, dvngLig( 293 ,"0x0CCD 0x0C97 0x0CB0 0x0CCD"))); // 0x100C8
        gKannadaLigMap.insert(std::make_pair( 0xE6F9, dvngLig( 293 ,"0x0C97 0x0CCD 0x0CCD 0x0CB0"))); // 0x100C8
        gKannadaLigMap.insert(std::make_pair( 0xE6FA, dvngLig( 293 ,"0x0C97 0x0CCD 0x0CB0 0x0CCD"))); // 0x100C8
        gKannadaLigMap.insert(std::make_pair( 0xE6FB, dvngLig( 294 ,"0x0CCD 0x0C9F 0x0CCD 0x0CB0"))); // 0x100C9
        gKannadaLigMap.insert(std::make_pair( 0xE6FC, dvngLig( 294 ,"0x0CCD 0x0C9F 0x0CB0 0x0CCD"))); // 0x100C9
        gKannadaLigMap.insert(std::make_pair( 0xE6FD, dvngLig( 294 ,"0x0C9F 0x0CCD 0x0CCD 0x0CB0"))); // 0x100C9
        gKannadaLigMap.insert(std::make_pair( 0xE6FE, dvngLig( 294 ,"0x0C9F 0x0CCD 0x0CB0 0x0CCD"))); // 0x100C9
        gKannadaLigMap.insert(std::make_pair( 0xE6FF, dvngLig( 295 ,"0x0CCD 0x0CA1 0x0CCD 0x0CB0"))); // 0x100CA
        gKannadaLigMap.insert(std::make_pair( 0xE700, dvngLig( 295 ,"0x0CCD 0x0CA1 0x0CB0 0x0CCD"))); // 0x100CA
        gKannadaLigMap.insert(std::make_pair( 0xE701, dvngLig( 295 ,"0x0CA1 0x0CCD 0x0CCD 0x0CB0"))); // 0x100CA
        gKannadaLigMap.insert(std::make_pair( 0xE702, dvngLig( 295 ,"0x0CA1 0x0CCD 0x0CB0 0x0CCD"))); // 0x100CA
        gKannadaLigMap.insert(std::make_pair( 0xE703, dvngLig( 296 ,"0x0CCD 0x0CA4 0x0CCD 0x0CB0"))); // 0x100CB
        gKannadaLigMap.insert(std::make_pair( 0xE704, dvngLig( 296 ,"0x0CCD 0x0CA4 0x0CB0 0x0CCD"))); // 0x100CB
        gKannadaLigMap.insert(std::make_pair( 0xE705, dvngLig( 296 ,"0x0CA4 0x0CCD 0x0CCD 0x0CB0"))); // 0x100CB
        gKannadaLigMap.insert(std::make_pair( 0xE706, dvngLig( 296 ,"0x0CA4 0x0CCD 0x0CB0 0x0CCD"))); // 0x100CB
        gKannadaLigMap.insert(std::make_pair( 0xE707, dvngLig( 297 ,"0x0CCD 0x0CA6 0x0CCD 0x0CB0"))); // 0x100CC
        gKannadaLigMap.insert(std::make_pair( 0xE708, dvngLig( 297 ,"0x0CCD 0x0CA6 0x0CB0 0x0CCD"))); // 0x100CC
        gKannadaLigMap.insert(std::make_pair( 0xE709, dvngLig( 297 ,"0x0CA6 0x0CCD 0x0CCD 0x0CB0"))); // 0x100CC
        gKannadaLigMap.insert(std::make_pair( 0xE70A, dvngLig( 297 ,"0x0CA6 0x0CCD 0x0CB0 0x0CCD"))); // 0x100CC
        gKannadaLigMap.insert(std::make_pair( 0xE70B, dvngLig( 298 ,"0x0CCD 0x0CAA 0x0CCD 0x0CB0"))); // 0x100CD
        gKannadaLigMap.insert(std::make_pair( 0xE70C, dvngLig( 298 ,"0x0CCD 0x0CAA 0x0CB0 0x0CCD"))); // 0x100CD
        gKannadaLigMap.insert(std::make_pair( 0xE70D, dvngLig( 298 ,"0x0CAA 0x0CCD 0x0CCD 0x0CB0"))); // 0x100CD
        gKannadaLigMap.insert(std::make_pair( 0xE70E, dvngLig( 298 ,"0x0CAA 0x0CCD 0x0CB0 0x0CCD"))); // 0x100CD
        gKannadaLigMap.insert(std::make_pair( 0xE70F, dvngLig( 299 ,"0x0CCD 0x0CAD 0x0CCD 0x0CB0"))); // 0x100CE
        gKannadaLigMap.insert(std::make_pair( 0xE710, dvngLig( 299 ,"0x0CCD 0x0CAD 0x0CB0 0x0CCD"))); // 0x100CE
        gKannadaLigMap.insert(std::make_pair( 0xE711, dvngLig( 299 ,"0x0CAD 0x0CCD 0x0CCD 0x0CB0"))); // 0x100CE
        gKannadaLigMap.insert(std::make_pair( 0xE712, dvngLig( 299 ,"0x0CAD 0x0CCD 0x0CB0 0x0CCD"))); // 0x100CE
        gKannadaLigMap.insert(std::make_pair( 0xE713, dvngLig( 300 ,"0x0CCD 0x0CA4 0x0CCD 0x0CAF"))); // 0x100CF
        gKannadaLigMap.insert(std::make_pair( 0xE714, dvngLig( 300 ,"0x0CCD 0x0CA4 0x0CAF 0x0CCD"))); // 0x100CF
        gKannadaLigMap.insert(std::make_pair( 0xE715, dvngLig( 300 ,"0x0CA4 0x0CCD 0x0CCD 0x0CAF"))); // 0x100CF
        gKannadaLigMap.insert(std::make_pair( 0xE716, dvngLig( 300 ,"0x0CA4 0x0CCD 0x0CAF 0x0CCD"))); // 0x100CF
        gKannadaLigMap.insert(std::make_pair( 0xE717, dvngLig( 301 ,"0x0CCD 0x0CAE 0x0CCD 0x0CAF"))); // 0x100D0
        gKannadaLigMap.insert(std::make_pair( 0xE718, dvngLig( 301 ,"0x0CCD 0x0CAE 0x0CAF 0x0CCD"))); // 0x100D0
        gKannadaLigMap.insert(std::make_pair( 0xE719, dvngLig( 301 ,"0x0CAE 0x0CCD 0x0CCD 0x0CAF"))); // 0x100D0
        gKannadaLigMap.insert(std::make_pair( 0xE71A, dvngLig( 301 ,"0x0CAE 0x0CCD 0x0CAF 0x0CCD"))); // 0x100D0
        gKannadaLigMap.insert(std::make_pair( 0xE71B, dvngLig( 302 ,"0x0CCD 0x0CA4 0x0CC3"))); // 0x100D1
        gKannadaLigMap.insert(std::make_pair( 0xE71C, dvngLig( 302 ,"0x0CA4 0x0CCD 0x0CC3"))); // 0x100D1
        gKannadaLigMap.insert(std::make_pair( 0xE71D, dvngLig( 303 ,"0x0CCD 0x0CAE 0x0CC3"))); // 0x100D2
        gKannadaLigMap.insert(std::make_pair( 0xE71E, dvngLig( 303 ,"0x0CAE 0x0CCD 0x0CC3"))); // 0x100D2
        gKannadaLigMap.insert(std::make_pair( 0xE71F, dvngLig( 304 ,"0x0CCD 0x0CA4 0x0CD6"))); // 0x100D3
        gKannadaLigMap.insert(std::make_pair( 0xE720, dvngLig( 304 ,"0x0CA4 0x0CCD 0x0CD6"))); // 0x100D3
        gKannadaLigMap.insert(std::make_pair( 0xE721, dvngLig( 305 ,"0x0CCD 0x0CAE 0x0CD6"))); // 0x100D4
        gKannadaLigMap.insert(std::make_pair( 0xE722, dvngLig( 305 ,"0x0CAE 0x0CCD 0x0CD6"))); // 0x100D4
        gKannadaLigMap.insert(std::make_pair( 0xE723, dvngLig( 306 ,"0x0CD6 0x0CB0 0x0CCD"))); // 0x100D5
        gKannadaLigMap.insert(std::make_pair( 0xE724, dvngLig( 306 ,"0x0CCD 0x0CB0 0x0CD6"))); // 0x100D5
        gKannadaLigMap.insert(std::make_pair( 0xE725, dvngLig( 306 ,"0x0CB0 0x0CCD 0x0CD6"))); // 0x100D5
    }
    return gKannadaLigMap;
}

lString16 processKannadaLigatures(lString16 word)
{
    int j = (word.length() >= 7 ) ? 7 : word.length();
    for(;  j >= 2; j--)
    {
        for (int c = word.length() - j; c >= 0; c--)
        {
            lUInt32 fastkey = (word.at(c) << 16) + word.at(c + 1);
            if (gKannadaFastLigMap.find(fastkey) == gKannadaFastLigMap.end())
            {
                continue;
            }
            dvngLig lig(word.substr(c, j));
            lChar16 rep = findKannadaLigRev(lig);
/*
            if(isViramaComboKannada(rep))
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

lString16 lString16::processKannadaText()
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
        word = processKannadaLigatures(word);

        res.append(word);
        res.append(L" ");
    }
    res.substr(0,res.size()-1);
    return res;
}

lString16 restoreKannadaWord(lString16 in)
{
    if(KANNADA_DISPLAY_ENABLE == 0 || gDocumentKannada == 0)
    {
        return in;
    }
    //lString16::StripZWNJ_reverse(&in);
    //lString16::SwitchBanglaRaITa_reverse(&in);
    //lString16::SwitchBanglaOU_reverse(&in);
    //lString16::SwitchBanglaO_reverse(&in)
    //SwitchKannadaAU_reverse(&in);
    //SwitchKannadaOO_reverse(&in);
    //SwitchKannadaO_reverse(&in);
    //lString16::SwitchBanglaAI_reverse(&in);
    //lString16::SwitchBanglaE_reverse(&in);
    //lString16::SwitchBanglaI_reverse(&in);
    //lString16::SwitchBanglaReph_reverse(&in);

    for (int i = 0; i < in.length(); i++)
    {
        if (in[i] < KANNADA_START || in[i] > KANNADA_END)
        {
            continue;
        }

        dvngLig lig = findKannadaLig(in[i]);
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
