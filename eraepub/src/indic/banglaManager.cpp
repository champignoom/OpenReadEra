//
// Created by Tarasus on 27.08.2020.
//
#include "include/indic/banglaManager.h"
#include "ore_log.h"

#include <vector>


LigMap     gBanglaLigMap;
LigMapRev  gBanglaLigMapRev;
FastLigMap gBanglaFastLigMap;

dvngLig findBanglaLig(lChar16 ligature)
{
    if(gBanglaLigMap.empty())
    {
        gBanglaLigMap = GetBanglaLigMap();
    }
    LigMap::iterator it = gBanglaLigMap.find(ligature);
    if(it==gBanglaLigMap.end())
    {
        return dvngLig();
    }
    return it->second;
}

lChar16 findBanglaLigGlyphIndex(lChar16 ligature)
{
    auto it = gBanglaLigMap.find(ligature);
    if(it==gBanglaLigMap.end())
    {
        return 0;
    }
    return it->second.glyphindex;
}

lChar16 findBanglaLigRev(dvngLig combo)
{
    if(gBanglaLigMapRev.empty())
    {
        gBanglaLigMapRev = BanglaLigMapReversed();
    }
    if(combo.len < 2 || combo.len > 10 )
    {
        return 0;
    }
    auto it = gBanglaLigMapRev.find(combo);
    if(it==gBanglaLigMapRev.end())
    {
        return 0;
    }
    return it->second;
}

std::map <dvngLig, lChar16, Comparator> BanglaLigMapReversed()
{
    if(!gBanglaLigMapRev.empty())
    {
        return gBanglaLigMapRev;
    }
    if(gBanglaLigMap.empty())
    {
        gBanglaLigMap = GetBanglaLigMap();
    }
    gBanglaLigMapRev = makeReverseLigMap(gBanglaLigMap,&gBanglaFastLigMap);
    return gBanglaLigMapRev;
}

LigMap GetBanglaLigMap()
{
    if(!gBanglaLigMap.empty())
    {
        return gBanglaLigMap;
    }
    //BANGLA_START
    //from Noto Sans Bengali.ttf
    gBanglaLigMap.insert(std::make_pair( 0x09B0, dvngLig( 46  ,"0x09AC 0x09BC")));        // 0x09B0 <-> 0xE201
    gBanglaLigMap.insert(std::make_pair( 0x09CE, dvngLig( 66  ,"0x09A4 0x09CD 0x200D"))); // 0x09CE <-> 0xE202
    gBanglaLigMap.insert(std::make_pair( 0x09DC, dvngLig( 68  ,"0x09A1 0x09BC")));        // 0x09DC <-> 0xE203
    gBanglaLigMap.insert(std::make_pair( 0x09DD, dvngLig( 69  ,"0x09A2 0x09BC")));        // 0x09DD <-> 0xE204
    gBanglaLigMap.insert(std::make_pair( 0x09DF, dvngLig( 70  ,"0x09AF 0x09BC")));        // 0x09DF <-> 0xE205
    gBanglaLigMap.insert(std::make_pair( 0xE206, dvngLig( 97  ,"0x0995 0x09BC"))); // 0x10001
    gBanglaLigMap.insert(std::make_pair( 0xE207, dvngLig( 98  ,"0x0996 0x09BC"))); // 0x10002
    gBanglaLigMap.insert(std::make_pair( 0xE208, dvngLig( 99  ,"0x0997 0x09BC"))); // 0x10003
    gBanglaLigMap.insert(std::make_pair( 0xE209, dvngLig( 100 ,"0x0998 0x09BC"))); // 0x10004
    gBanglaLigMap.insert(std::make_pair( 0xE20A, dvngLig( 101 ,"0x0999 0x09BC"))); // 0x10005
    gBanglaLigMap.insert(std::make_pair( 0xE20B, dvngLig( 102 ,"0x099A 0x09BC"))); // 0x10006
    gBanglaLigMap.insert(std::make_pair( 0xE20C, dvngLig( 103 ,"0x099B 0x09BC"))); // 0x10007
    gBanglaLigMap.insert(std::make_pair( 0xE20D, dvngLig( 104 ,"0x099C 0x09BC"))); // 0x10008
    gBanglaLigMap.insert(std::make_pair( 0xE20E, dvngLig( 105 ,"0x099D 0x09BC"))); // 0x10009
    gBanglaLigMap.insert(std::make_pair( 0xE20F, dvngLig( 106 ,"0x099E 0x09BC"))); // 0x1000A
    gBanglaLigMap.insert(std::make_pair( 0xE210, dvngLig( 107 ,"0x099F 0x09BC"))); // 0x1000B
    gBanglaLigMap.insert(std::make_pair( 0xE211, dvngLig( 108 ,"0x09A0 0x09BC"))); // 0x1000C
    gBanglaLigMap.insert(std::make_pair( 0xE212, dvngLig( 109 ,"0x09A3 0x09BC"))); // 0x1000D
    gBanglaLigMap.insert(std::make_pair( 0xE213, dvngLig( 110 ,"0x09A4 0x09BC"))); // 0x1000E
    gBanglaLigMap.insert(std::make_pair( 0xE214, dvngLig( 111 ,"0x09A5 0x09BC"))); // 0x1000F
    gBanglaLigMap.insert(std::make_pair( 0xE215, dvngLig( 112 ,"0x09A6 0x09BC"))); // 0x10010
    gBanglaLigMap.insert(std::make_pair( 0xE216, dvngLig( 113 ,"0x09A7 0x09BC"))); // 0x10011
    gBanglaLigMap.insert(std::make_pair( 0xE217, dvngLig( 114 ,"0x09A8 0x09BC"))); // 0x10012
    gBanglaLigMap.insert(std::make_pair( 0xE218, dvngLig( 115 ,"0x09AA 0x09BC"))); // 0x10013
    gBanglaLigMap.insert(std::make_pair( 0xE219, dvngLig( 116 ,"0x09AB 0x09BC"))); // 0x10014
    gBanglaLigMap.insert(std::make_pair( 0xE21A, dvngLig( 117 ,"0x09AD 0x09BC"))); // 0x10015
    gBanglaLigMap.insert(std::make_pair( 0xE21B, dvngLig( 118 ,"0x09AE 0x09BC"))); // 0x10016
    gBanglaLigMap.insert(std::make_pair( 0xE21C, dvngLig( 119 ,"0x09B2 0x09BC"))); // 0x10017
    gBanglaLigMap.insert(std::make_pair( 0xE21D, dvngLig( 120 ,"0x09B6 0x09BC"))); // 0x10018
    gBanglaLigMap.insert(std::make_pair( 0xE21E, dvngLig( 121 ,"0x09B7 0x09BC"))); // 0x10019
    gBanglaLigMap.insert(std::make_pair( 0xE21F, dvngLig( 122 ,"0x09B8 0x09BC"))); // 0x1001A
    gBanglaLigMap.insert(std::make_pair( 0xE220, dvngLig( 123 ,"0x09B9 0x09BC"))); // 0x1001B
    gBanglaLigMap.insert(std::make_pair( 0xE221, dvngLig( 124 ,"0x09F0 0x09BC"))); // 0x1001C
    gBanglaLigMap.insert(std::make_pair( 0xE222, dvngLig( 125 ,"0x09F1 0x09BC"))); // 0x1001D
    gBanglaLigMap.insert(std::make_pair( 0xE223, dvngLig( 126 ,"0x0995 0x09CD 0x09B7"))); // 0x1001E
    gBanglaLigMap.insert(std::make_pair( 0xE224, dvngLig( 127 ,"0x099C 0x09CD 0x099E"))); // 0x1001F
    gBanglaLigMap.insert(std::make_pair( 0xE225, dvngLig( 128 ,"0x09F0 0x09CD"))); // 0x10020       //Ra1 virama (reph)
    gBanglaLigMap.insert(std::make_pair( 0xE226, dvngLig( 128 ,"0x09B0 0x09CD"))); // 0x10020       //Ra  virama (reph)
    //gBanglaLigMap.insert(std::make_pair( 0xE227, dvngLig( 129 ,"0x09CD 0x09B0"))); // 0x10021       //Ra  virama (vattu) 1
    //gBanglaLigMap.insert(std::make_pair( 0xE228, dvngLig( 129 ,"0x09CD 0x09F0"))); // 0x10021       //Ra1 virama (vattu) 1
    //gBanglaLigMap.insert(std::make_pair( 0xE229, dvngLig( 129 ,"0x09F0 0x09CD"))); // 0x10021       //Ra1 virama (vattu)
    //gBanglaLigMap.insert(std::make_pair( 0xE22A, dvngLig( 129 ,"0x09B0 0x09CD"))); // 0x10021       //Ra  virama (vattu)
    //gBanglaLigMap.insert(std::make_pair( 0xE22B, dvngLig( 130 ,"0x09CD 0x09AC"))); // 0x10022
    //gBanglaLigMap.insert(std::make_pair( 0xE22C, dvngLig( 130 ,"0x09AC 0x09CD"))); // 0x10022


    //replaced to regular "ka" glyph due to (presumed) bengali rules
    //gBanglaLigMap.insert(std::make_pair( 0xE22D, dvngLig( 131 ,"0x0995 0x09CD"))); // 0x10023
    gBanglaLigMap.insert(std::make_pair( 0xE22D, dvngLig( 20 ,"0x0995 0x09CD"))); // 0x0995

    gBanglaLigMap.insert(std::make_pair( 0xE22E, dvngLig( 132 ,"0x0996 0x09CD"))); // 0x10024
    //gBanglaLigMap.insert(std::make_pair( 0xE22F, dvngLig( 133 ,"0x0997 0x09CD"))); // 0x10025 //half form that shouldn't be used
    gBanglaLigMap.insert(std::make_pair( 0xE230, dvngLig( 134 ,"0x0998 0x09CD"))); // 0x10026
    gBanglaLigMap.insert(std::make_pair( 0xE231, dvngLig( 135 ,"0x0999 0x09CD"))); // 0x10027
    //gBanglaLigMap.insert(std::make_pair( 0xE232, dvngLig( 136 ,"0x099A 0x09CD"))); // 0x10028 //half form that shouldn't be used
    gBanglaLigMap.insert(std::make_pair( 0xE233, dvngLig( 137 ,"0x099B 0x09CD"))); // 0x10029
    gBanglaLigMap.insert(std::make_pair( 0xE234, dvngLig( 138 ,"0x099C 0x09CD"))); // 0x1002A
    gBanglaLigMap.insert(std::make_pair( 0xE235, dvngLig( 139 ,"0x099D 0x09CD"))); // 0x1002B
    gBanglaLigMap.insert(std::make_pair( 0xE236, dvngLig( 140 ,"0x099E 0x09CD"))); // 0x1002C
    //gBanglaLigMap.insert(std::make_pair( 0xE237, dvngLig( 141 ,"0x099F 0x09CD"))); // 0x1002D //??!
    gBanglaLigMap.insert(std::make_pair( 0xE238, dvngLig( 142 ,"0x09A0 0x09CD"))); // 0x1002E
    gBanglaLigMap.insert(std::make_pair( 0xE239, dvngLig( 143 ,"0x09A1 0x09CD"))); // 0x1002F
    gBanglaLigMap.insert(std::make_pair( 0xE23A, dvngLig( 144 ,"0x09A2 0x09CD"))); // 0x10030
    //gBanglaLigMap.insert(std::make_pair( 0xE23B, dvngLig( 145 ,"0x09A3 0x09CD"))); // 0x10031 //half form that shouldn't be used
    gBanglaLigMap.insert(std::make_pair( 0xE23C, dvngLig( 146 ,"0x09A4 0x09CD"))); // 0x10032
    gBanglaLigMap.insert(std::make_pair( 0xE23D, dvngLig( 147 ,"0x09A5 0x09CD"))); // 0x10033
    //gBanglaLigMap.insert(std::make_pair( 0xE23E, dvngLig( 148 ,"0x09A6 0x09CD"))); // 0x10034 //half form that shouldn't be used
    gBanglaLigMap.insert(std::make_pair( 0xE23F, dvngLig( 149 ,"0x09A7 0x09CD"))); // 0x10035
    //gBanglaLigMap.insert(std::make_pair( 0xE240, dvngLig( 150 ,"0x09A8 0x09CD"))); // 0x10036 //na half form that shouldn't be used
    //gBanglaLigMap.insert(std::make_pair( 0xE241, dvngLig( 151 ,"0x09AA 0x09CD"))); // 0x10037 //half form that shouldn't be used
    gBanglaLigMap.insert(std::make_pair( 0xE242, dvngLig( 152 ,"0x09AB 0x09CD"))); // 0x10038
    //gBanglaLigMap.insert(std::make_pair( 0xE243, dvngLig( 153 ,"0x09AC 0x09CD"))); // 0x10039 //half form that shouldn't be used
    gBanglaLigMap.insert(std::make_pair( 0xE244, dvngLig( 154 ,"0x09AD 0x09CD"))); // 0x1003A
    //gBanglaLigMap.insert(std::make_pair( 0xE245, dvngLig( 155 ,"0x09AE 0x09CD"))); // 0x1003B //half form that shouldn't be used
    gBanglaLigMap.insert(std::make_pair( 0xE246, dvngLig( 156 ,"0x09AF 0x09CD"))); // 0x1003C
    //gBanglaLigMap.insert(std::make_pair( 0xE247, dvngLig( 157 ,"0x09B2 0x09CD"))); // 0x1003D //half form that shouldn't be used
    //gBanglaLigMap.insert(std::make_pair( 0xE248, dvngLig( 158 ,"0x09B6 0x09CD"))); // 0x1003E //half form that shouldn't be used
    //gBanglaLigMap.insert(std::make_pair( 0xE249, dvngLig( 159 ,"0x09B7 0x09CD"))); // 0x1003F //half form that shouldn't be used

    ////replaced to regular "sa" glyph due to (presumed) bengali rules // NO NEED TO
    //gBanglaLigMap.insert(std::make_pair( 0xE24A, dvngLig( 160 ,"0x09B8 0x09CD"))); // 0x10040
    //gBanglaLigMap.insert(std::make_pair( 0xE24A, dvngLig( 49 ,"0x09B8 0x09CD"))); // 0x09B8

    gBanglaLigMap.insert(std::make_pair( 0xE24B, dvngLig( 161 ,"0x09B9 0x09CD"))); // 0x10041
    gBanglaLigMap.insert(std::make_pair( 0xE24C, dvngLig( 162 ,"0x09F0 0x09CD"))); // 0x10042       //Ra1 virama (full)
    gBanglaLigMap.insert(std::make_pair( 0xE24D, dvngLig( 163 ,"0x09F1 0x09CD"))); // 0x10043
    gBanglaLigMap.insert(std::make_pair( 0xE24E, dvngLig( 164 ,"0x0995 0x09CD 0x09B7 0x09CD"))); // 0x10044
    gBanglaLigMap.insert(std::make_pair( 0xE24F, dvngLig( 165 ,"0x099C 0x09CD 0x099E 0x09CD"))); // 0x10045
    gBanglaLigMap.insert(std::make_pair( 0xE250, dvngLig( 166 ,"0x0995 0x09BC 0x09CD"))); // 0x10046
    gBanglaLigMap.insert(std::make_pair( 0xE251, dvngLig( 167 ,"0x0996 0x09BC 0x09CD"))); // 0x10047
    gBanglaLigMap.insert(std::make_pair( 0xE252, dvngLig( 168 ,"0x0997 0x09BC 0x09CD"))); // 0x10048
    gBanglaLigMap.insert(std::make_pair( 0xE253, dvngLig( 169 ,"0x0998 0x09BC 0x09CD"))); // 0x10049
    gBanglaLigMap.insert(std::make_pair( 0xE254, dvngLig( 170 ,"0x0999 0x09BC 0x09CD"))); // 0x1004A
    gBanglaLigMap.insert(std::make_pair( 0xE255, dvngLig( 171 ,"0x099A 0x09BC 0x09CD"))); // 0x1004B
    gBanglaLigMap.insert(std::make_pair( 0xE256, dvngLig( 172 ,"0x099B 0x09BC 0x09CD"))); // 0x1004C
    gBanglaLigMap.insert(std::make_pair( 0xE257, dvngLig( 173 ,"0x099C 0x09BC 0x09CD"))); // 0x1004D
    gBanglaLigMap.insert(std::make_pair( 0xE258, dvngLig( 174 ,"0x099D 0x09BC 0x09CD"))); // 0x1004E
    gBanglaLigMap.insert(std::make_pair( 0xE259, dvngLig( 175 ,"0x099E 0x09BC 0x09CD"))); // 0x1004F
    gBanglaLigMap.insert(std::make_pair( 0xE25A, dvngLig( 176 ,"0x099F 0x09BC 0x09CD"))); // 0x10050
    gBanglaLigMap.insert(std::make_pair( 0xE25B, dvngLig( 177 ,"0x09A0 0x09BC 0x09CD"))); // 0x10051
    gBanglaLigMap.insert(std::make_pair( 0xE25C, dvngLig( 178 ,"0x09DC 0x09CD"))); // 0x10052
    gBanglaLigMap.insert(std::make_pair( 0xE25D, dvngLig( 179 ,"0x09DD 0x09CD"))); // 0x10053
    gBanglaLigMap.insert(std::make_pair( 0xE25E, dvngLig( 180 ,"0x09A3 0x09BC 0x09CD"))); // 0x10054
    gBanglaLigMap.insert(std::make_pair( 0xE25F, dvngLig( 181 ,"0x09A4 0x09BC 0x09CD"))); // 0x10055
    gBanglaLigMap.insert(std::make_pair( 0xE260, dvngLig( 182 ,"0x09A5 0x09BC 0x09CD"))); // 0x10056
    gBanglaLigMap.insert(std::make_pair( 0xE261, dvngLig( 183 ,"0x09A6 0x09BC 0x09CD"))); // 0x10057
    gBanglaLigMap.insert(std::make_pair( 0xE262, dvngLig( 184 ,"0x09A7 0x09BC 0x09CD"))); // 0x10058
    gBanglaLigMap.insert(std::make_pair( 0xE263, dvngLig( 185 ,"0x09A8 0x09BC 0x09CD"))); // 0x10059
    gBanglaLigMap.insert(std::make_pair( 0xE264, dvngLig( 186 ,"0x09AA 0x09BC 0x09CD"))); // 0x1005A
    gBanglaLigMap.insert(std::make_pair( 0xE265, dvngLig( 187 ,"0x09AB 0x09BC 0x09CD"))); // 0x1005B
    gBanglaLigMap.insert(std::make_pair( 0xE266, dvngLig( 188 ,"0x09B0 0x09CD"))); // 0x1005C           //Ra virama (full)
    gBanglaLigMap.insert(std::make_pair( 0xE267, dvngLig( 189 ,"0x09AD 0x09BC 0x09CD"))); // 0x1005D
    gBanglaLigMap.insert(std::make_pair( 0xE268, dvngLig( 190 ,"0x09AE 0x09BC 0x09CD"))); // 0x1005E
    gBanglaLigMap.insert(std::make_pair( 0xE269, dvngLig( 191 ,"0x09DF 0x09CD"))); // 0x1005F
    gBanglaLigMap.insert(std::make_pair( 0xE26A, dvngLig( 192 ,"0x09B2 0x09BC 0x09CD"))); // 0x10060
    gBanglaLigMap.insert(std::make_pair( 0xE26B, dvngLig( 193 ,"0x09B6 0x09BC 0x09CD"))); // 0x10061
    gBanglaLigMap.insert(std::make_pair( 0xE26C, dvngLig( 194 ,"0x09B7 0x09BC 0x09CD"))); // 0x10062
    gBanglaLigMap.insert(std::make_pair( 0xE26D, dvngLig( 195 ,"0x09B8 0x09BC 0x09CD"))); // 0x10063
    gBanglaLigMap.insert(std::make_pair( 0xE26E, dvngLig( 196 ,"0x09B9 0x09BC 0x09CD"))); // 0x10064
    gBanglaLigMap.insert(std::make_pair( 0xE26F, dvngLig( 197 ,"0x09F0 0x09BC 0x09CD"))); // 0x10065
    gBanglaLigMap.insert(std::make_pair( 0xE270, dvngLig( 198 ,"0x09F1 0x09BC 0x09CD"))); // 0x10066
    gBanglaLigMap.insert(std::make_pair( 0xE271, dvngLig( 199 ,"0x200D 0x09CD 0x09AF"))); // 0x10067    //YaPostForm //[a]
    gBanglaLigMap.insert(std::make_pair( 0xE272, dvngLig( 199 ,"0x09CD 0x09AF"))); // 0x10067           //YaPostForm  //??!
    gBanglaLigMap.insert(std::make_pair( 0xE273, dvngLig( 199 ,"0x09AF 0x09CD"))); // 0x10067           //YaPostForm
    gBanglaLigMap.insert(std::make_pair( 0xE274, dvngLig( 200 ,"0x0995 0x09CD 0x09B0"))); // 0x10068
    gBanglaLigMap.insert(std::make_pair( 0xE275, dvngLig( 200 ,"0x0995 0x09CD 0x09F0"))); // 0x10068
    gBanglaLigMap.insert(std::make_pair( 0xE276, dvngLig( 200 ,"0x0995 0x09F0 0x09CD"))); // 0x10068
    gBanglaLigMap.insert(std::make_pair( 0xE277, dvngLig( 200 ,"0x0995 0x09B0 0x09CD"))); // 0x10068
    gBanglaLigMap.insert(std::make_pair( 0xE278, dvngLig( 201 ,"0x0996 0x09CD 0x09B0"))); // 0x10069
    gBanglaLigMap.insert(std::make_pair( 0xE279, dvngLig( 201 ,"0x0996 0x09CD 0x09F0"))); // 0x10069
    gBanglaLigMap.insert(std::make_pair( 0xE27A, dvngLig( 201 ,"0x0996 0x09F0 0x09CD"))); // 0x10069
    gBanglaLigMap.insert(std::make_pair( 0xE27B, dvngLig( 201 ,"0x0996 0x09B0 0x09CD"))); // 0x10069
    gBanglaLigMap.insert(std::make_pair( 0xE27C, dvngLig( 202 ,"0x0997 0x09CD 0x09B0"))); // 0x1006A
    gBanglaLigMap.insert(std::make_pair( 0xE27D, dvngLig( 202 ,"0x0997 0x09CD 0x09F0"))); // 0x1006A
    gBanglaLigMap.insert(std::make_pair( 0xE27E, dvngLig( 202 ,"0x0997 0x09F0 0x09CD"))); // 0x1006A
    gBanglaLigMap.insert(std::make_pair( 0xE27F, dvngLig( 202 ,"0x0997 0x09B0 0x09CD"))); // 0x1006A
    gBanglaLigMap.insert(std::make_pair( 0xE280, dvngLig( 203 ,"0x0998 0x09CD 0x09B0"))); // 0x1006B
    gBanglaLigMap.insert(std::make_pair( 0xE281, dvngLig( 203 ,"0x0998 0x09CD 0x09F0"))); // 0x1006B
    gBanglaLigMap.insert(std::make_pair( 0xE282, dvngLig( 203 ,"0x0998 0x09F0 0x09CD"))); // 0x1006B
    gBanglaLigMap.insert(std::make_pair( 0xE283, dvngLig( 203 ,"0x0998 0x09B0 0x09CD"))); // 0x1006B
    gBanglaLigMap.insert(std::make_pair( 0xE284, dvngLig( 204 ,"0x0999 0x09CD 0x09B0"))); // 0x1006C
    gBanglaLigMap.insert(std::make_pair( 0xE285, dvngLig( 204 ,"0x0999 0x09CD 0x09F0"))); // 0x1006C
    gBanglaLigMap.insert(std::make_pair( 0xE286, dvngLig( 204 ,"0x0999 0x09F0 0x09CD"))); // 0x1006C
    gBanglaLigMap.insert(std::make_pair( 0xE287, dvngLig( 204 ,"0x0999 0x09B0 0x09CD"))); // 0x1006C
    gBanglaLigMap.insert(std::make_pair( 0xE288, dvngLig( 206 ,"0x099A 0x09CD 0x09B0"))); // 0x1006E
    gBanglaLigMap.insert(std::make_pair( 0xE289, dvngLig( 206 ,"0x099A 0x09CD 0x09F0"))); // 0x1006E
    gBanglaLigMap.insert(std::make_pair( 0xE28A, dvngLig( 206 ,"0x099A 0x09F0 0x09CD"))); // 0x1006E
    gBanglaLigMap.insert(std::make_pair( 0xE28B, dvngLig( 206 ,"0x099A 0x09B0 0x09CD"))); // 0x1006E
    gBanglaLigMap.insert(std::make_pair( 0xE28C, dvngLig( 207 ,"0x099B 0x09CD 0x09B0"))); // 0x1006F
    gBanglaLigMap.insert(std::make_pair( 0xE28D, dvngLig( 207 ,"0x099B 0x09CD 0x09F0"))); // 0x1006F
    gBanglaLigMap.insert(std::make_pair( 0xE28E, dvngLig( 207 ,"0x099B 0x09F0 0x09CD"))); // 0x1006F
    gBanglaLigMap.insert(std::make_pair( 0xE28F, dvngLig( 207 ,"0x099B 0x09B0 0x09CD"))); // 0x1006F
    gBanglaLigMap.insert(std::make_pair( 0xE290, dvngLig( 208 ,"0x099C 0x09CD 0x09B0"))); // 0x10070
    gBanglaLigMap.insert(std::make_pair( 0xE291, dvngLig( 208 ,"0x099C 0x09CD 0x09F0"))); // 0x10070
    gBanglaLigMap.insert(std::make_pair( 0xE292, dvngLig( 208 ,"0x099C 0x09F0 0x09CD"))); // 0x10070
    gBanglaLigMap.insert(std::make_pair( 0xE293, dvngLig( 208 ,"0x099C 0x09B0 0x09CD"))); // 0x10070
    gBanglaLigMap.insert(std::make_pair( 0xE294, dvngLig( 209 ,"0x099D 0x09CD 0x09B0"))); // 0x10071
    gBanglaLigMap.insert(std::make_pair( 0xE295, dvngLig( 209 ,"0x099D 0x09CD 0x09F0"))); // 0x10071
    gBanglaLigMap.insert(std::make_pair( 0xE296, dvngLig( 209 ,"0x099D 0x09F0 0x09CD"))); // 0x10071
    gBanglaLigMap.insert(std::make_pair( 0xE297, dvngLig( 209 ,"0x099D 0x09B0 0x09CD"))); // 0x10071
    gBanglaLigMap.insert(std::make_pair( 0xE298, dvngLig( 210 ,"0x099E 0x09CD 0x09B0"))); // 0x10072
    gBanglaLigMap.insert(std::make_pair( 0xE299, dvngLig( 210 ,"0x099E 0x09CD 0x09F0"))); // 0x10072
    gBanglaLigMap.insert(std::make_pair( 0xE29A, dvngLig( 210 ,"0x099E 0x09F0 0x09CD"))); // 0x10072
    gBanglaLigMap.insert(std::make_pair( 0xE29B, dvngLig( 210 ,"0x099E 0x09B0 0x09CD"))); // 0x10072
    gBanglaLigMap.insert(std::make_pair( 0xE29C, dvngLig( 211 ,"0x099F 0x09CD 0x09B0"))); // 0x10073
    gBanglaLigMap.insert(std::make_pair( 0xE29D, dvngLig( 211 ,"0x099F 0x09CD 0x09F0"))); // 0x10073
    gBanglaLigMap.insert(std::make_pair( 0xE29E, dvngLig( 211 ,"0x099F 0x09F0 0x09CD"))); // 0x10073
    gBanglaLigMap.insert(std::make_pair( 0xE29F, dvngLig( 211 ,"0x099F 0x09B0 0x09CD"))); // 0x10073
    gBanglaLigMap.insert(std::make_pair( 0xE2A0, dvngLig( 212 ,"0x09A0 0x09CD 0x09B0"))); // 0x10074
    gBanglaLigMap.insert(std::make_pair( 0xE2A1, dvngLig( 212 ,"0x09A0 0x09CD 0x09F0"))); // 0x10074
    gBanglaLigMap.insert(std::make_pair( 0xE2A2, dvngLig( 212 ,"0x09A0 0x09F0 0x09CD"))); // 0x10074
    gBanglaLigMap.insert(std::make_pair( 0xE2A3, dvngLig( 212 ,"0x09A0 0x09B0 0x09CD"))); // 0x10074
    gBanglaLigMap.insert(std::make_pair( 0xE2A4, dvngLig( 213 ,"0x09A1 0x09CD 0x09B0"))); // 0x10075
    gBanglaLigMap.insert(std::make_pair( 0xE2A5, dvngLig( 213 ,"0x09A1 0x09CD 0x09F0"))); // 0x10075
    gBanglaLigMap.insert(std::make_pair( 0xE2A6, dvngLig( 213 ,"0x09A1 0x09F0 0x09CD"))); // 0x10075
    gBanglaLigMap.insert(std::make_pair( 0xE2A7, dvngLig( 213 ,"0x09A1 0x09B0 0x09CD"))); // 0x10075
    gBanglaLigMap.insert(std::make_pair( 0xE2A8, dvngLig( 214 ,"0x09A2 0x09CD 0x09B0"))); // 0x10076
    gBanglaLigMap.insert(std::make_pair( 0xE2A9, dvngLig( 214 ,"0x09A2 0x09CD 0x09F0"))); // 0x10076
    gBanglaLigMap.insert(std::make_pair( 0xE2AA, dvngLig( 214 ,"0x09A2 0x09F0 0x09CD"))); // 0x10076
    gBanglaLigMap.insert(std::make_pair( 0xE2AB, dvngLig( 214 ,"0x09A2 0x09B0 0x09CD"))); // 0x10076
    gBanglaLigMap.insert(std::make_pair( 0xE2AC, dvngLig( 215 ,"0x09A3 0x09CD 0x09B0"))); // 0x10077
    gBanglaLigMap.insert(std::make_pair( 0xE2AD, dvngLig( 215 ,"0x09A3 0x09CD 0x09F0"))); // 0x10077
    gBanglaLigMap.insert(std::make_pair( 0xE2AE, dvngLig( 215 ,"0x09A3 0x09F0 0x09CD"))); // 0x10077
    gBanglaLigMap.insert(std::make_pair( 0xE2AF, dvngLig( 215 ,"0x09A3 0x09B0 0x09CD"))); // 0x10077
    gBanglaLigMap.insert(std::make_pair( 0xE2B0, dvngLig( 216 ,"0x09A4 0x09CD 0x09B0"))); // 0x10078  //??!
    gBanglaLigMap.insert(std::make_pair( 0xE2B1, dvngLig( 216 ,"0x09A4 0x09CD 0x09F0"))); // 0x10078  //??!
    gBanglaLigMap.insert(std::make_pair( 0xE2B2, dvngLig( 216 ,"0x09A4 0x09F0 0x09CD"))); // 0x10078
    gBanglaLigMap.insert(std::make_pair( 0xE2B3, dvngLig( 216 ,"0x09A4 0x09B0 0x09CD"))); // 0x10078
    gBanglaLigMap.insert(std::make_pair( 0xE2B4, dvngLig( 217 ,"0x09A5 0x09CD 0x09B0"))); // 0x10079
    gBanglaLigMap.insert(std::make_pair( 0xE2B5, dvngLig( 217 ,"0x09A5 0x09CD 0x09F0"))); // 0x10079
    gBanglaLigMap.insert(std::make_pair( 0xE2B6, dvngLig( 217 ,"0x09A5 0x09F0 0x09CD"))); // 0x10079
    gBanglaLigMap.insert(std::make_pair( 0xE2B7, dvngLig( 217 ,"0x09A5 0x09B0 0x09CD"))); // 0x10079
    gBanglaLigMap.insert(std::make_pair( 0xE2B8, dvngLig( 218 ,"0x09A6 0x09CD 0x09B0"))); // 0x1007A
    gBanglaLigMap.insert(std::make_pair( 0xE2B9, dvngLig( 218 ,"0x09A6 0x09CD 0x09F0"))); // 0x1007A
    gBanglaLigMap.insert(std::make_pair( 0xE2BA, dvngLig( 218 ,"0x09A6 0x09F0 0x09CD"))); // 0x1007A
    gBanglaLigMap.insert(std::make_pair( 0xE2BB, dvngLig( 218 ,"0x09A6 0x09B0 0x09CD"))); // 0x1007A
    gBanglaLigMap.insert(std::make_pair( 0xE2BC, dvngLig( 219 ,"0x09A7 0x09CD 0x09B0"))); // 0x1007B
    gBanglaLigMap.insert(std::make_pair( 0xE2BD, dvngLig( 219 ,"0x09A7 0x09CD 0x09F0"))); // 0x1007B
    gBanglaLigMap.insert(std::make_pair( 0xE2BE, dvngLig( 219 ,"0x09A7 0x09F0 0x09CD"))); // 0x1007B
    gBanglaLigMap.insert(std::make_pair( 0xE2BF, dvngLig( 219 ,"0x09A7 0x09B0 0x09CD"))); // 0x1007B
    gBanglaLigMap.insert(std::make_pair( 0xE2C0, dvngLig( 220 ,"0x09A8 0x09CD 0x09B0"))); // 0x1007C
    gBanglaLigMap.insert(std::make_pair( 0xE2C1, dvngLig( 220 ,"0x09A8 0x09CD 0x09F0"))); // 0x1007C
    gBanglaLigMap.insert(std::make_pair( 0xE2C2, dvngLig( 220 ,"0x09A8 0x09F0 0x09CD"))); // 0x1007C
    gBanglaLigMap.insert(std::make_pair( 0xE2C3, dvngLig( 220 ,"0x09A8 0x09B0 0x09CD"))); // 0x1007C
    gBanglaLigMap.insert(std::make_pair( 0xE2C4, dvngLig( 221 ,"0x09AA 0x09CD 0x09B0"))); // 0x1007D
    gBanglaLigMap.insert(std::make_pair( 0xE2C5, dvngLig( 221 ,"0x09AA 0x09CD 0x09F0"))); // 0x1007D
    gBanglaLigMap.insert(std::make_pair( 0xE2C6, dvngLig( 221 ,"0x09AA 0x09F0 0x09CD"))); // 0x1007D
    gBanglaLigMap.insert(std::make_pair( 0xE2C7, dvngLig( 221 ,"0x09AA 0x09B0 0x09CD"))); // 0x1007D
    gBanglaLigMap.insert(std::make_pair( 0xE2C8, dvngLig( 222 ,"0x09AB 0x09CD 0x09B0"))); // 0x1007E
    gBanglaLigMap.insert(std::make_pair( 0xE2C9, dvngLig( 222 ,"0x09AB 0x09CD 0x09F0"))); // 0x1007E
    gBanglaLigMap.insert(std::make_pair( 0xE2CA, dvngLig( 222 ,"0x09AB 0x09F0 0x09CD"))); // 0x1007E
    gBanglaLigMap.insert(std::make_pair( 0xE2CB, dvngLig( 222 ,"0x09AB 0x09B0 0x09CD"))); // 0x1007E
    gBanglaLigMap.insert(std::make_pair( 0xE2CC, dvngLig( 223 ,"0x09AC 0x09CD 0x09B0"))); // 0x1007F
    gBanglaLigMap.insert(std::make_pair( 0xE2CD, dvngLig( 223 ,"0x09AC 0x09CD 0x09F0"))); // 0x1007F
    gBanglaLigMap.insert(std::make_pair( 0xE2CE, dvngLig( 223 ,"0x09AC 0x09F0 0x09CD"))); // 0x1007F
    gBanglaLigMap.insert(std::make_pair( 0xE2CF, dvngLig( 223 ,"0x09AC 0x09B0 0x09CD"))); // 0x1007F
    gBanglaLigMap.insert(std::make_pair( 0xE2D0, dvngLig( 224 ,"0x09AD 0x09CD 0x09B0"))); // 0x10080
    gBanglaLigMap.insert(std::make_pair( 0xE2D1, dvngLig( 224 ,"0x09AD 0x09CD 0x09F0"))); // 0x10080
    gBanglaLigMap.insert(std::make_pair( 0xE2D2, dvngLig( 224 ,"0x09AD 0x09F0 0x09CD"))); // 0x10080
    gBanglaLigMap.insert(std::make_pair( 0xE2D3, dvngLig( 224 ,"0x09AD 0x09B0 0x09CD"))); // 0x10080 //??!
    gBanglaLigMap.insert(std::make_pair( 0xE2D4, dvngLig( 225 ,"0x09AE 0x09CD 0x09B0"))); // 0x10081
    gBanglaLigMap.insert(std::make_pair( 0xE2D5, dvngLig( 225 ,"0x09AE 0x09CD 0x09F0"))); // 0x10081
    gBanglaLigMap.insert(std::make_pair( 0xE2D6, dvngLig( 225 ,"0x09AE 0x09F0 0x09CD"))); // 0x10081  //??!
    gBanglaLigMap.insert(std::make_pair( 0xE2D7, dvngLig( 225 ,"0x09AE 0x09B0 0x09CD"))); // 0x10081  //??!
    gBanglaLigMap.insert(std::make_pair( 0xE2D8, dvngLig( 226 ,"0x09AF 0x09CD 0x09B0"))); // 0x10082
    gBanglaLigMap.insert(std::make_pair( 0xE2D9, dvngLig( 226 ,"0x09AF 0x09CD 0x09F0"))); // 0x10082
    gBanglaLigMap.insert(std::make_pair( 0xE2DA, dvngLig( 226 ,"0x09AF 0x09F0 0x09CD"))); // 0x10082
    gBanglaLigMap.insert(std::make_pair( 0xE2DB, dvngLig( 226 ,"0x09AF 0x09B0 0x09CD"))); // 0x10082
    gBanglaLigMap.insert(std::make_pair( 0xE2DC, dvngLig( 227 ,"0x09B0 0x09CD 0x09B0"))); // 0x10083
    gBanglaLigMap.insert(std::make_pair( 0xE2DD, dvngLig( 227 ,"0x09B0 0x09CD 0x09F0"))); // 0x10083
    gBanglaLigMap.insert(std::make_pair( 0xE2DE, dvngLig( 227 ,"0x09B0 0x09F0 0x09CD"))); // 0x10083
    gBanglaLigMap.insert(std::make_pair( 0xE2DF, dvngLig( 227 ,"0x09B0 0x09B0 0x09CD"))); // 0x10083
    gBanglaLigMap.insert(std::make_pair( 0xE2E0, dvngLig( 228 ,"0x09B2 0x09CD 0x09B0"))); // 0x10084
    gBanglaLigMap.insert(std::make_pair( 0xE2E1, dvngLig( 228 ,"0x09B2 0x09CD 0x09F0"))); // 0x10084
    gBanglaLigMap.insert(std::make_pair( 0xE2E2, dvngLig( 228 ,"0x09B2 0x09F0 0x09CD"))); // 0x10084
    gBanglaLigMap.insert(std::make_pair( 0xE2E3, dvngLig( 228 ,"0x09B2 0x09B0 0x09CD"))); // 0x10084
    gBanglaLigMap.insert(std::make_pair( 0xE2E4, dvngLig( 229 ,"0x09B6 0x09CD 0x09B0"))); // 0x10085
    gBanglaLigMap.insert(std::make_pair( 0xE2E5, dvngLig( 229 ,"0x09B6 0x09CD 0x09F0"))); // 0x10085
    gBanglaLigMap.insert(std::make_pair( 0xE2E6, dvngLig( 229 ,"0x09B6 0x09F0 0x09CD"))); // 0x10085
    gBanglaLigMap.insert(std::make_pair( 0xE2E7, dvngLig( 229 ,"0x09B6 0x09B0 0x09CD"))); // 0x10085
    gBanglaLigMap.insert(std::make_pair( 0xE2E8, dvngLig( 230 ,"0x09B7 0x09CD 0x09B0"))); // 0x10086
    gBanglaLigMap.insert(std::make_pair( 0xE2E9, dvngLig( 230 ,"0x09B7 0x09CD 0x09F0"))); // 0x10086
    gBanglaLigMap.insert(std::make_pair( 0xE2EA, dvngLig( 230 ,"0x09B7 0x09F0 0x09CD"))); // 0x10086
    gBanglaLigMap.insert(std::make_pair( 0xE2EB, dvngLig( 230 ,"0x09B7 0x09B0 0x09CD"))); // 0x10086
    gBanglaLigMap.insert(std::make_pair( 0xE2EC, dvngLig( 231 ,"0x09B8 0x09CD 0x09B0"))); // 0x10087  //??!
    gBanglaLigMap.insert(std::make_pair( 0xE2ED, dvngLig( 231 ,"0x09B8 0x09CD 0x09F0"))); // 0x10087  //??!
    gBanglaLigMap.insert(std::make_pair( 0xE2EE, dvngLig( 231 ,"0x09B8 0x09F0 0x09CD"))); // 0x10087  //??!
    gBanglaLigMap.insert(std::make_pair( 0xE2EF, dvngLig( 231 ,"0x09B8 0x09B0 0x09CD"))); // 0x10087  //??! ??!
    gBanglaLigMap.insert(std::make_pair( 0xE2F0, dvngLig( 232 ,"0x09B9 0x09CD 0x09B0"))); // 0x10088
    gBanglaLigMap.insert(std::make_pair( 0xE2F1, dvngLig( 232 ,"0x09B9 0x09CD 0x09F0"))); // 0x10088
    gBanglaLigMap.insert(std::make_pair( 0xE2F2, dvngLig( 232 ,"0x09B9 0x09F0 0x09CD"))); // 0x10088
    gBanglaLigMap.insert(std::make_pair( 0xE2F3, dvngLig( 232 ,"0x09B9 0x09B0 0x09CD"))); // 0x10088
    gBanglaLigMap.insert(std::make_pair( 0xE2F4, dvngLig( 233 ,"0x09F0 0x09CD 0x09B0"))); // 0x10089
    gBanglaLigMap.insert(std::make_pair( 0xE2F5, dvngLig( 233 ,"0x09F0 0x09CD 0x09F0"))); // 0x10089
    gBanglaLigMap.insert(std::make_pair( 0xE2F6, dvngLig( 233 ,"0x09F0 0x09F0 0x09CD"))); // 0x10089
    gBanglaLigMap.insert(std::make_pair( 0xE2F7, dvngLig( 233 ,"0x09F0 0x09B0 0x09CD"))); // 0x10089
    gBanglaLigMap.insert(std::make_pair( 0xE2F8, dvngLig( 234 ,"0x09F1 0x09CD 0x09B0"))); // 0x1008A
    gBanglaLigMap.insert(std::make_pair( 0xE2F9, dvngLig( 234 ,"0x09F1 0x09CD 0x09F0"))); // 0x1008A
    gBanglaLigMap.insert(std::make_pair( 0xE2FA, dvngLig( 234 ,"0x09F1 0x09F0 0x09CD"))); // 0x1008A
    gBanglaLigMap.insert(std::make_pair( 0xE2FB, dvngLig( 234 ,"0x09F1 0x09B0 0x09CD"))); // 0x1008A
    gBanglaLigMap.insert(std::make_pair( 0xE2FC, dvngLig( 235 ,"0x09DC 0x09CD 0x09B0"))); // 0x1008B
    gBanglaLigMap.insert(std::make_pair( 0xE2FD, dvngLig( 235 ,"0x09DC 0x09CD 0x09F0"))); // 0x1008B
    gBanglaLigMap.insert(std::make_pair( 0xE2FE, dvngLig( 235 ,"0x09DC 0x09F0 0x09CD"))); // 0x1008B
    gBanglaLigMap.insert(std::make_pair( 0xE2FF, dvngLig( 235 ,"0x09DC 0x09B0 0x09CD"))); // 0x1008B
    gBanglaLigMap.insert(std::make_pair( 0xE300, dvngLig( 237 ,"0x09DD 0x09CD 0x09B0"))); // 0x1008D
    gBanglaLigMap.insert(std::make_pair( 0xE301, dvngLig( 237 ,"0x09DD 0x09CD 0x09F0"))); // 0x1008D
    gBanglaLigMap.insert(std::make_pair( 0xE302, dvngLig( 237 ,"0x09DD 0x09F0 0x09CD"))); // 0x1008D
    gBanglaLigMap.insert(std::make_pair( 0xE303, dvngLig( 237 ,"0x09DD 0x09B0 0x09CD"))); // 0x1008D
    gBanglaLigMap.insert(std::make_pair( 0xE304, dvngLig( 239 ,"0x09DF 0x09CD 0x09B0"))); // 0x1008F
    gBanglaLigMap.insert(std::make_pair( 0xE305, dvngLig( 239 ,"0x09DF 0x09CD 0x09F0"))); // 0x1008F
    gBanglaLigMap.insert(std::make_pair( 0xE306, dvngLig( 239 ,"0x09DF 0x09F0 0x09CD"))); // 0x1008F
    gBanglaLigMap.insert(std::make_pair( 0xE307, dvngLig( 239 ,"0x09DF 0x09B0 0x09CD"))); // 0x1008F
    gBanglaLigMap.insert(std::make_pair( 0xE308, dvngLig( 240 ,"0x0995 0x09CD 0x09B7 0x09CD 0x09B0"))); // 0x10090
    gBanglaLigMap.insert(std::make_pair( 0xE309, dvngLig( 240 ,"0x0995 0x09CD 0x09B7 0x09CD 0x09F0"))); // 0x10090
    gBanglaLigMap.insert(std::make_pair( 0xE30A, dvngLig( 240 ,"0x0995 0x09CD 0x09B7 0x09F0 0x09CD"))); // 0x10090
    gBanglaLigMap.insert(std::make_pair( 0xE30B, dvngLig( 240 ,"0x0995 0x09CD 0x09B7 0x09B0 0x09CD"))); // 0x10090
    gBanglaLigMap.insert(std::make_pair( 0xE30C, dvngLig( 241 ,"0x099C 0x09CD 0x099E 0x09CD 0x09B0"))); // 0x10091
    gBanglaLigMap.insert(std::make_pair( 0xE30D, dvngLig( 241 ,"0x099C 0x09CD 0x099E 0x09CD 0x09F0"))); // 0x10091
    gBanglaLigMap.insert(std::make_pair( 0xE30E, dvngLig( 241 ,"0x099C 0x09CD 0x099E 0x09F0 0x09CD"))); // 0x10091
    gBanglaLigMap.insert(std::make_pair( 0xE30F, dvngLig( 241 ,"0x099C 0x09CD 0x099E 0x09B0 0x09CD"))); // 0x10091
    gBanglaLigMap.insert(std::make_pair( 0xE310, dvngLig( 242 ,"0x0995 0x09CD 0x09AC"))); // 0x10092
    gBanglaLigMap.insert(std::make_pair( 0xE311, dvngLig( 242 ,"0x0995 0x09AC 0x09CD"))); // 0x10092
    gBanglaLigMap.insert(std::make_pair( 0xE312, dvngLig( 243 ,"0x0996 0x09CD 0x09AC"))); // 0x10093
    gBanglaLigMap.insert(std::make_pair( 0xE313, dvngLig( 243 ,"0x0996 0x09AC 0x09CD"))); // 0x10093
    gBanglaLigMap.insert(std::make_pair( 0xE314, dvngLig( 244 ,"0x0997 0x09CD 0x09AC"))); // 0x10094
    gBanglaLigMap.insert(std::make_pair( 0xE315, dvngLig( 244 ,"0x0997 0x09AC 0x09CD"))); // 0x10094
    gBanglaLigMap.insert(std::make_pair( 0xE316, dvngLig( 245 ,"0x0998 0x09CD 0x09AC"))); // 0x10095
    gBanglaLigMap.insert(std::make_pair( 0xE317, dvngLig( 245 ,"0x0998 0x09AC 0x09CD"))); // 0x10095
    gBanglaLigMap.insert(std::make_pair( 0xE318, dvngLig( 246 ,"0x0999 0x09CD 0x09AC"))); // 0x10096
    gBanglaLigMap.insert(std::make_pair( 0xE319, dvngLig( 246 ,"0x0999 0x09AC 0x09CD"))); // 0x10096
    gBanglaLigMap.insert(std::make_pair( 0xE31A, dvngLig( 247 ,"0x099A 0x09CD 0x09AC"))); // 0x10097
    gBanglaLigMap.insert(std::make_pair( 0xE31B, dvngLig( 247 ,"0x099A 0x09AC 0x09CD"))); // 0x10097
    gBanglaLigMap.insert(std::make_pair( 0xE31C, dvngLig( 248 ,"0x099B 0x09CD 0x09AC"))); // 0x10098
    gBanglaLigMap.insert(std::make_pair( 0xE31D, dvngLig( 248 ,"0x099B 0x09AC 0x09CD"))); // 0x10098
    gBanglaLigMap.insert(std::make_pair( 0xE31E, dvngLig( 249 ,"0x099C 0x09CD 0x09AC"))); // 0x10099
    gBanglaLigMap.insert(std::make_pair( 0xE31F, dvngLig( 249 ,"0x099C 0x09AC 0x09CD"))); // 0x10099
    gBanglaLigMap.insert(std::make_pair( 0xE320, dvngLig( 250 ,"0x099D 0x09CD 0x09AC"))); // 0x1009A
    gBanglaLigMap.insert(std::make_pair( 0xE321, dvngLig( 250 ,"0x099D 0x09AC 0x09CD"))); // 0x1009A
    gBanglaLigMap.insert(std::make_pair( 0xE322, dvngLig( 251 ,"0x099E 0x09CD 0x09AC"))); // 0x1009B
    gBanglaLigMap.insert(std::make_pair( 0xE323, dvngLig( 251 ,"0x099E 0x09AC 0x09CD"))); // 0x1009B
    gBanglaLigMap.insert(std::make_pair( 0xE324, dvngLig( 252 ,"0x099F 0x09CD 0x09AC"))); // 0x1009C
    gBanglaLigMap.insert(std::make_pair( 0xE325, dvngLig( 252 ,"0x099F 0x09AC 0x09CD"))); // 0x1009C
    gBanglaLigMap.insert(std::make_pair( 0xE326, dvngLig( 253 ,"0x09A0 0x09CD 0x09AC"))); // 0x1009D
    gBanglaLigMap.insert(std::make_pair( 0xE327, dvngLig( 253 ,"0x09A0 0x09AC 0x09CD"))); // 0x1009D
    gBanglaLigMap.insert(std::make_pair( 0xE328, dvngLig( 254 ,"0x09A1 0x09CD 0x09AC"))); // 0x1009E
    gBanglaLigMap.insert(std::make_pair( 0xE329, dvngLig( 254 ,"0x09A1 0x09AC 0x09CD"))); // 0x1009E
    gBanglaLigMap.insert(std::make_pair( 0xE32A, dvngLig( 255 ,"0x09A2 0x09CD 0x09AC"))); // 0x1009F
    gBanglaLigMap.insert(std::make_pair( 0xE32B, dvngLig( 255 ,"0x09A2 0x09AC 0x09CD"))); // 0x1009F
    gBanglaLigMap.insert(std::make_pair( 0xE32C, dvngLig( 256 ,"0x09A3 0x09CD 0x09AC"))); // 0x100A0
    gBanglaLigMap.insert(std::make_pair( 0xE32D, dvngLig( 256 ,"0x09A3 0x09AC 0x09CD"))); // 0x100A0
    gBanglaLigMap.insert(std::make_pair( 0xE32E, dvngLig( 257 ,"0x09A4 0x09CD 0x09AC"))); // 0x100A1 !??
    gBanglaLigMap.insert(std::make_pair( 0xE32F, dvngLig( 257 ,"0x09A4 0x09AC 0x09CD"))); // 0x100A1 !??
    gBanglaLigMap.insert(std::make_pair( 0xE330, dvngLig( 258 ,"0x09A5 0x09CD 0x09AC"))); // 0x100A2
    gBanglaLigMap.insert(std::make_pair( 0xE331, dvngLig( 258 ,"0x09A5 0x09AC 0x09CD"))); // 0x100A2
    gBanglaLigMap.insert(std::make_pair( 0xE332, dvngLig( 259 ,"0x09A6 0x09CD 0x09AC"))); // 0x100A3
    gBanglaLigMap.insert(std::make_pair( 0xE333, dvngLig( 259 ,"0x09A6 0x09AC 0x09CD"))); // 0x100A3
    gBanglaLigMap.insert(std::make_pair( 0xE334, dvngLig( 260 ,"0x09A7 0x09CD 0x09AC"))); // 0x100A4
    gBanglaLigMap.insert(std::make_pair( 0xE335, dvngLig( 260 ,"0x09A7 0x09AC 0x09CD"))); // 0x100A4
    gBanglaLigMap.insert(std::make_pair( 0xE336, dvngLig( 261 ,"0x09A8 0x09CD 0x09AC"))); // 0x100A5
    gBanglaLigMap.insert(std::make_pair( 0xE337, dvngLig( 261 ,"0x09A8 0x09AC 0x09CD"))); // 0x100A5
    gBanglaLigMap.insert(std::make_pair( 0xE338, dvngLig( 262 ,"0x09AA 0x09CD 0x09AC"))); // 0x100A6
    gBanglaLigMap.insert(std::make_pair( 0xE339, dvngLig( 262 ,"0x09AA 0x09AC 0x09CD"))); // 0x100A6
    gBanglaLigMap.insert(std::make_pair( 0xE33A, dvngLig( 263 ,"0x09AB 0x09CD 0x09AC"))); // 0x100A7
    gBanglaLigMap.insert(std::make_pair( 0xE33B, dvngLig( 263 ,"0x09AB 0x09AC 0x09CD"))); // 0x100A7
    gBanglaLigMap.insert(std::make_pair( 0xE33C, dvngLig( 264 ,"0x09AC 0x09CD 0x09AC"))); // 0x100A8
    gBanglaLigMap.insert(std::make_pair( 0xE33D, dvngLig( 264 ,"0x09AC 0x09AC 0x09CD"))); // 0x100A8
    gBanglaLigMap.insert(std::make_pair( 0xE33E, dvngLig( 265 ,"0x09AD 0x09CD 0x09AC"))); // 0x100A9
    gBanglaLigMap.insert(std::make_pair( 0xE33F, dvngLig( 265 ,"0x09AD 0x09AC 0x09CD"))); // 0x100A9
    gBanglaLigMap.insert(std::make_pair( 0xE340, dvngLig( 266 ,"0x09AE 0x09CD 0x09AC"))); // 0x100AA
    gBanglaLigMap.insert(std::make_pair( 0xE341, dvngLig( 266 ,"0x09AE 0x09AC 0x09CD"))); // 0x100AA
    gBanglaLigMap.insert(std::make_pair( 0xE342, dvngLig( 267 ,"0x09AF 0x09CD 0x09AC"))); // 0x100AB
    gBanglaLigMap.insert(std::make_pair( 0xE343, dvngLig( 267 ,"0x09AF 0x09AC 0x09CD"))); // 0x100AB
    //gBanglaLigMap.insert(std::make_pair( 0xE344, dvngLig( 268 ,"0x09B0 0x09CD 0x09AC"))); // 0x100AC //??!
    //gBanglaLigMap.insert(std::make_pair( 0xE345, dvngLig( 268 ,"0x09B0 0x09AC 0x09CD"))); // 0x100AC //??!
    gBanglaLigMap.insert(std::make_pair( 0xE346, dvngLig( 269 ,"0x09B2 0x09CD 0x09AC"))); // 0x100AD
    gBanglaLigMap.insert(std::make_pair( 0xE347, dvngLig( 269 ,"0x09B2 0x09AC 0x09CD"))); // 0x100AD
    gBanglaLigMap.insert(std::make_pair( 0xE348, dvngLig( 270 ,"0x09B6 0x09CD 0x09AC"))); // 0x100AE
    gBanglaLigMap.insert(std::make_pair( 0xE349, dvngLig( 270 ,"0x09B6 0x09AC 0x09CD"))); // 0x100AE
    gBanglaLigMap.insert(std::make_pair( 0xE34A, dvngLig( 271 ,"0x09B7 0x09CD 0x09AC"))); // 0x100AF
    gBanglaLigMap.insert(std::make_pair( 0xE34B, dvngLig( 271 ,"0x09B7 0x09AC 0x09CD"))); // 0x100AF
    gBanglaLigMap.insert(std::make_pair( 0xE34C, dvngLig( 272 ,"0x09B8 0x09CD 0x09AC"))); // 0x100B0
    gBanglaLigMap.insert(std::make_pair( 0xE34D, dvngLig( 272 ,"0x09B8 0x09AC 0x09CD"))); // 0x100B0
    gBanglaLigMap.insert(std::make_pair( 0xE34E, dvngLig( 273 ,"0x09B9 0x09CD 0x09AC"))); // 0x100B1
    gBanglaLigMap.insert(std::make_pair( 0xE34F, dvngLig( 273 ,"0x09B9 0x09AC 0x09CD"))); // 0x100B1
    gBanglaLigMap.insert(std::make_pair( 0xE350, dvngLig( 274 ,"0x09F0 0x09CD 0x09AC"))); // 0x100B2
    gBanglaLigMap.insert(std::make_pair( 0xE351, dvngLig( 274 ,"0x09F0 0x09AC 0x09CD"))); // 0x100B2
    gBanglaLigMap.insert(std::make_pair( 0xE352, dvngLig( 275 ,"0x09F1 0x09CD 0x09AC"))); // 0x100B3
    gBanglaLigMap.insert(std::make_pair( 0xE353, dvngLig( 275 ,"0x09F1 0x09AC 0x09CD"))); // 0x100B3
    gBanglaLigMap.insert(std::make_pair( 0xE354, dvngLig( 276 ,"0x09DC 0x09CD 0x09AC"))); // 0x100B4
    gBanglaLigMap.insert(std::make_pair( 0xE355, dvngLig( 276 ,"0x09DC 0x09AC 0x09CD"))); // 0x100B4
    gBanglaLigMap.insert(std::make_pair( 0xE356, dvngLig( 277 ,"0x09DD 0x09CD 0x09AC"))); // 0x100B5
    gBanglaLigMap.insert(std::make_pair( 0xE357, dvngLig( 277 ,"0x09DD 0x09AC 0x09CD"))); // 0x100B5
    gBanglaLigMap.insert(std::make_pair( 0xE358, dvngLig( 278 ,"0x09DF 0x09CD 0x09AC"))); // 0x100B6
    gBanglaLigMap.insert(std::make_pair( 0xE359, dvngLig( 278 ,"0x09DF 0x09AC 0x09CD"))); // 0x100B6
    gBanglaLigMap.insert(std::make_pair( 0xE35A, dvngLig( 279 ,"0x0995 0x09CD 0x09B7 0x09CD 0x09AC"))); // 0x100B7
    gBanglaLigMap.insert(std::make_pair( 0xE35B, dvngLig( 279 ,"0x0995 0x09CD 0x09B7 0x09AC 0x09CD"))); // 0x100B7
    gBanglaLigMap.insert(std::make_pair( 0xE35C, dvngLig( 280 ,"0x099C 0x09CD 0x099E 0x09CD 0x09AC"))); // 0x100B8
    gBanglaLigMap.insert(std::make_pair( 0xE35D, dvngLig( 280 ,"0x099C 0x09CD 0x099E 0x09AC 0x09CD"))); // 0x100B8
    gBanglaLigMap.insert(std::make_pair( 0xE35E, dvngLig( 281 ,"0x0995 0x09CD 0x0995"))); // 0x100B9
    gBanglaLigMap.insert(std::make_pair( 0xE35F, dvngLig( 282 ,"0x0995 0x09CD 0x099F"))); // 0x100BA
    gBanglaLigMap.insert(std::make_pair( 0xE360, dvngLig( 283 ,"0x0995 0x09CD 0x099F 0x09CD 0x09B0"))); // 0x100BB
    gBanglaLigMap.insert(std::make_pair( 0xE361, dvngLig( 283 ,"0x0995 0x09CD 0x099F 0x09CD 0x09F0"))); // 0x100BB
    gBanglaLigMap.insert(std::make_pair( 0xE362, dvngLig( 283 ,"0x0995 0x09CD 0x099F 0x09F0 0x09CD"))); // 0x100BB
    gBanglaLigMap.insert(std::make_pair( 0xE363, dvngLig( 283 ,"0x0995 0x09CD 0x099F 0x09B0 0x09CD"))); // 0x100BB
    gBanglaLigMap.insert(std::make_pair( 0xE364, dvngLig( 284 ,"0x0995 0x09CD 0x09A4"))); // 0x100BC
    gBanglaLigMap.insert(std::make_pair( 0xE365, dvngLig( 285 ,"0x0995 0x09CD 0x09A4 0x09CD 0x09AC"))); // 0x100BD
    gBanglaLigMap.insert(std::make_pair( 0xE366, dvngLig( 285 ,"0x0995 0x09CD 0x09A4 0x09AC 0x09CD"))); // 0x100BD
    gBanglaLigMap.insert(std::make_pair( 0xE367, dvngLig( 286 ,"0x0995 0x09CD 0x09A4 0x09CD 0x09B0"))); // 0x100BE
    gBanglaLigMap.insert(std::make_pair( 0xE368, dvngLig( 286 ,"0x0995 0x09CD 0x09A4 0x09CD 0x09F0"))); // 0x100BE
    gBanglaLigMap.insert(std::make_pair( 0xE369, dvngLig( 286 ,"0x0995 0x09CD 0x09A4 0x09F0 0x09CD"))); // 0x100BE
    gBanglaLigMap.insert(std::make_pair( 0xE36A, dvngLig( 286 ,"0x0995 0x09CD 0x09A4 0x09B0 0x09CD"))); // 0x100BE
    gBanglaLigMap.insert(std::make_pair( 0xE36B, dvngLig( 287 ,"0x0995 0x09CD 0x09A8"))); // 0x100BF
    gBanglaLigMap.insert(std::make_pair( 0xE36C, dvngLig( 288 ,"0x0995 0x09CD 0x09AE"))); // 0x100C0
    gBanglaLigMap.insert(std::make_pair( 0xE36D, dvngLig( 289 ,"0x0995 0x09CD 0x09B2"))); // 0x100C1
    gBanglaLigMap.insert(std::make_pair( 0xE36E, dvngLig( 290 ,"0x0995 0x09CD 0x09B8"))); // 0x100C2
    gBanglaLigMap.insert(std::make_pair( 0xE36F, dvngLig( 291 ,"0x0995 0x09CD 0x09B7 0x09CD 0x09A3"))); // 0x100C3
    gBanglaLigMap.insert(std::make_pair( 0xE370, dvngLig( 292 ,"0x0995 0x09CD 0x09B7 0x09CD 0x09AE"))); // 0x100C4
    gBanglaLigMap.insert(std::make_pair( 0xE371, dvngLig( 293 ,"0x0997 0x09CD 0x0997"))); // 0x100C5
    gBanglaLigMap.insert(std::make_pair( 0xE372, dvngLig( 294 ,"0x0997 0x09CD 0x09A6"))); // 0x100C6
    gBanglaLigMap.insert(std::make_pair( 0xE373, dvngLig( 295 ,"0x0997 0x09CD 0x09A7"))); // 0x100C7
    gBanglaLigMap.insert(std::make_pair( 0xE374, dvngLig( 296 ,"0x0997 0x09CD 0x09A8"))); // 0x100C8
    gBanglaLigMap.insert(std::make_pair( 0xE375, dvngLig( 297 ,"0x0997 0x09CD 0x09AE"))); // 0x100C9
    gBanglaLigMap.insert(std::make_pair( 0xE376, dvngLig( 298 ,"0x0997 0x09CD 0x09B2"))); // 0x100CA
    gBanglaLigMap.insert(std::make_pair( 0xE377, dvngLig( 299 ,"0x0998 0x09CD 0x09A8"))); // 0x100CB
    gBanglaLigMap.insert(std::make_pair( 0xE378, dvngLig( 300 ,"0x0998 0x09CD 0x09B2"))); // 0x100CC
    gBanglaLigMap.insert(std::make_pair( 0xE379, dvngLig( 301 ,"0x0999 0x09CD 0x0995"))); // 0x100CD
    gBanglaLigMap.insert(std::make_pair( 0xE37A, dvngLig( 302 ,"0x0999 0x09CD 0x0995 0x09CD 0x09B0"))); // 0x100CE
    gBanglaLigMap.insert(std::make_pair( 0xE37B, dvngLig( 302 ,"0x0999 0x09CD 0x0995 0x09CD 0x09F0"))); // 0x100CE
    gBanglaLigMap.insert(std::make_pair( 0xE37C, dvngLig( 302 ,"0x0999 0x09CD 0x0995 0x09F0 0x09CD"))); // 0x100CE
    gBanglaLigMap.insert(std::make_pair( 0xE37D, dvngLig( 302 ,"0x0999 0x09CD 0x0995 0x09B0 0x09CD"))); // 0x100CE
    gBanglaLigMap.insert(std::make_pair( 0xE37E, dvngLig( 303 ,"0x0999 0x09CD 0x0996"))); // 0x100CF
    gBanglaLigMap.insert(std::make_pair( 0xE37F, dvngLig( 304 ,"0x0999 0x09CD 0x0997"))); // 0x100D0
    gBanglaLigMap.insert(std::make_pair( 0xE380, dvngLig( 305 ,"0x0999 0x09CD 0x0998"))); // 0x100D1
    gBanglaLigMap.insert(std::make_pair( 0xE381, dvngLig( 306 ,"0x0999 0x09CD 0x09AE"))); // 0x100D2
    gBanglaLigMap.insert(std::make_pair( 0xE382, dvngLig( 307 ,"0x0999 0x09CD 0x0995 0x09CD 0x09B7"))); // 0x100D3
    gBanglaLigMap.insert(std::make_pair( 0xE383, dvngLig( 308 ,"0x099A 0x09CD 0x099A"))); // 0x100D4
    gBanglaLigMap.insert(std::make_pair( 0xE384, dvngLig( 309 ,"0x099A 0x09CD 0x099B"))); // 0x100D5
    gBanglaLigMap.insert(std::make_pair( 0xE385, dvngLig( 310 ,"0x099A 0x09CD 0x099E"))); // 0x100D6
    gBanglaLigMap.insert(std::make_pair( 0xE386, dvngLig( 311 ,"0x099A 0x09CD 0x09A8"))); // 0x100D7
    gBanglaLigMap.insert(std::make_pair( 0xE387, dvngLig( 312 ,"0x099A 0x09CD 0x099B 0x09CD 0x09AC"))); // 0x100D8
    gBanglaLigMap.insert(std::make_pair( 0xE388, dvngLig( 312 ,"0x099A 0x09CD 0x099B 0x09AC 0x09CD"))); // 0x100D8
    gBanglaLigMap.insert(std::make_pair( 0xE389, dvngLig( 313 ,"0x099A 0x09CD 0x099B 0x09CD 0x09B0"))); // 0x100D9
    gBanglaLigMap.insert(std::make_pair( 0xE38A, dvngLig( 313 ,"0x099A 0x09CD 0x099B 0x09CD 0x09F0"))); // 0x100D9
    gBanglaLigMap.insert(std::make_pair( 0xE38B, dvngLig( 313 ,"0x099A 0x09CD 0x099B 0x09F0 0x09CD"))); // 0x100D9
    gBanglaLigMap.insert(std::make_pair( 0xE38C, dvngLig( 313 ,"0x099A 0x09CD 0x099B 0x09B0 0x09CD"))); // 0x100D9
    gBanglaLigMap.insert(std::make_pair( 0xE38D, dvngLig( 314 ,"0x099C 0x09CD 0x099C"))); // 0x100DA
    gBanglaLigMap.insert(std::make_pair( 0xE38E, dvngLig( 315 ,"0x099C 0x09CD 0x099D"))); // 0x100DB
    gBanglaLigMap.insert(std::make_pair( 0xE38F, dvngLig( 316 ,"0x099C 0x09CD 0x099C 0x09CD 0x09AC"))); // 0x100DC
    gBanglaLigMap.insert(std::make_pair( 0xE390, dvngLig( 316 ,"0x099C 0x09CD 0x099C 0x09AC 0x09CD"))); // 0x100DC
    gBanglaLigMap.insert(std::make_pair( 0xE391, dvngLig( 317 ,"0x099E 0x09CD 0x099A"))); // 0x100DD
    gBanglaLigMap.insert(std::make_pair( 0xE392, dvngLig( 318 ,"0x099E 0x09CD 0x099B"))); // 0x100DE
    gBanglaLigMap.insert(std::make_pair( 0xE393, dvngLig( 319 ,"0x099E 0x09CD 0x099C"))); // 0x100DF
    gBanglaLigMap.insert(std::make_pair( 0xE394, dvngLig( 320 ,"0x099E 0x09CD 0x099D"))); // 0x100E0
    gBanglaLigMap.insert(std::make_pair( 0xE395, dvngLig( 321 ,"0x099F 0x09CD 0x099F"))); // 0x100E1
    //gBanglaLigMap.insert(std::make_pair( 0xE396, dvngLig( 322 ,"0x099F 0x09CD 0x09AE"))); // 0x100E2 //??!
    gBanglaLigMap.insert(std::make_pair( 0xE397, dvngLig( 323 ,"0x09A1 0x09CD 0x0997"))); // 0x100E3
    gBanglaLigMap.insert(std::make_pair( 0xE398, dvngLig( 324 ,"0x09A1 0x09CD 0x09A1"))); // 0x100E4
    gBanglaLigMap.insert(std::make_pair( 0xE399, dvngLig( 325 ,"0x09A1 0x09CD 0x09AE"))); // 0x100E5
    gBanglaLigMap.insert(std::make_pair( 0xE39A, dvngLig( 326 ,"0x09DC 0x09CD 0x0997"))); // 0x100E6
    gBanglaLigMap.insert(std::make_pair( 0xE39B, dvngLig( 327 ,"0x09A3 0x09CD 0x099F"))); // 0x100E7
    gBanglaLigMap.insert(std::make_pair( 0xE39C, dvngLig( 328 ,"0x09A3 0x09CD 0x09A0"))); // 0x100E8
    gBanglaLigMap.insert(std::make_pair( 0xE39D, dvngLig( 329 ,"0x09A3 0x09CD 0x09A1"))); // 0x100E9
    gBanglaLigMap.insert(std::make_pair( 0xE39E, dvngLig( 330 ,"0x09A3 0x09CD 0x09A2"))); // 0x100EA
    gBanglaLigMap.insert(std::make_pair( 0xE39F, dvngLig( 331 ,"0x09A3 0x09CD 0x09A3"))); // 0x100EB
    gBanglaLigMap.insert(std::make_pair( 0xE3A0, dvngLig( 332 ,"0x09A3 0x09CD 0x09A8"))); // 0x100EC
    gBanglaLigMap.insert(std::make_pair( 0xE3A1, dvngLig( 333 ,"0x09A3 0x09CD 0x09AE"))); // 0x100ED
    gBanglaLigMap.insert(std::make_pair( 0xE3A2, dvngLig( 334 ,"0x09A3 0x09CD 0x099F 0x09CD 0x09B0"))); // 0x100EE
    gBanglaLigMap.insert(std::make_pair( 0xE3A3, dvngLig( 334 ,"0x09A3 0x09CD 0x099F 0x09CD 0x09F0"))); // 0x100EE
    gBanglaLigMap.insert(std::make_pair( 0xE3A4, dvngLig( 334 ,"0x09A3 0x09CD 0x099F 0x09F0 0x09CD"))); // 0x100EE
    gBanglaLigMap.insert(std::make_pair( 0xE3A5, dvngLig( 334 ,"0x09A3 0x09CD 0x099F 0x09B0 0x09CD"))); // 0x100EE
    gBanglaLigMap.insert(std::make_pair( 0xE3A6, dvngLig( 335 ,"0x09A3 0x09CD 0x09A1 0x09CD 0x09B0"))); // 0x100EF
    gBanglaLigMap.insert(std::make_pair( 0xE3A7, dvngLig( 335 ,"0x09A3 0x09CD 0x09A1 0x09CD 0x09F0"))); // 0x100EF
    gBanglaLigMap.insert(std::make_pair( 0xE3A8, dvngLig( 335 ,"0x09A3 0x09CD 0x09A1 0x09F0 0x09CD"))); // 0x100EF
    gBanglaLigMap.insert(std::make_pair( 0xE3A9, dvngLig( 335 ,"0x09A3 0x09CD 0x09A1 0x09B0 0x09CD"))); // 0x100EF
    gBanglaLigMap.insert(std::make_pair( 0xE3AA, dvngLig( 336 ,"0x09A4 0x09CD 0x09A4"))); // 0x100F0
    gBanglaLigMap.insert(std::make_pair( 0xE3AB, dvngLig( 337 ,"0x09A4 0x09CD 0x09A5"))); // 0x100F1
    gBanglaLigMap.insert(std::make_pair( 0xE3AC, dvngLig( 338 ,"0x09A4 0x09CD 0x09A8"))); // 0x100F2
    gBanglaLigMap.insert(std::make_pair( 0xE3AD, dvngLig( 339 ,"0x09A4 0x09CD 0x09AE"))); // 0x100F3
    gBanglaLigMap.insert(std::make_pair( 0xE3AE, dvngLig( 340 ,"0x09A4 0x09CD 0x09B2"))); // 0x100F4
    gBanglaLigMap.insert(std::make_pair( 0xE3AF, dvngLig( 341 ,"0x09A4 0x09CD 0x09A4 0x09CD 0x09AC"))); // 0x100F5
    gBanglaLigMap.insert(std::make_pair( 0xE3B0, dvngLig( 341 ,"0x09A4 0x09CD 0x09A4 0x09AC 0x09CD"))); // 0x100F5
    gBanglaLigMap.insert(std::make_pair( 0xE3B1, dvngLig( 342 ,"0x09A6 0x09CD 0x0997"))); // 0x100F6
    gBanglaLigMap.insert(std::make_pair( 0xE3B2, dvngLig( 343 ,"0x09A6 0x09CD 0x0998"))); // 0x100F7
    gBanglaLigMap.insert(std::make_pair( 0xE3B3, dvngLig( 344 ,"0x09A6 0x09CD 0x09A6"))); // 0x100F8
    gBanglaLigMap.insert(std::make_pair( 0xE3B4, dvngLig( 345 ,"0x09A6 0x09CD 0x09A6 0x09CD 0x09AC"))); // 0x100F9
    gBanglaLigMap.insert(std::make_pair( 0xE3B5, dvngLig( 345 ,"0x09A6 0x09CD 0x09A6 0x09AC 0x09CD"))); // 0x100F9
    gBanglaLigMap.insert(std::make_pair( 0xE3B6, dvngLig( 346 ,"0x09A6 0x09CD 0x09A8"))); // 0x100FA
    gBanglaLigMap.insert(std::make_pair( 0xE3B7, dvngLig( 347 ,"0x09A6 0x09CD 0x09A7"))); // 0x100FB
    gBanglaLigMap.insert(std::make_pair( 0xE3B8, dvngLig( 348 ,"0x09A6 0x09CD 0x09A7 0x09CD 0x09AC"))); // 0x100FC
    gBanglaLigMap.insert(std::make_pair( 0xE3B9, dvngLig( 348 ,"0x09A6 0x09CD 0x09A7 0x09AC 0x09CD"))); // 0x100FC
    gBanglaLigMap.insert(std::make_pair( 0xE3BA, dvngLig( 349 ,"0x09A6 0x09CD 0x09AD"))); // 0x100FD
    gBanglaLigMap.insert(std::make_pair( 0xE3BB, dvngLig( 350 ,"0x09A6 0x09CD 0x09AD 0x09CD 0x09B0"))); // 0x100FE
    gBanglaLigMap.insert(std::make_pair( 0xE3BC, dvngLig( 350 ,"0x09A6 0x09CD 0x09AD 0x09CD 0x09F0"))); // 0x100FE
    gBanglaLigMap.insert(std::make_pair( 0xE3BD, dvngLig( 350 ,"0x09A6 0x09CD 0x09AD 0x09F0 0x09CD"))); // 0x100FE
    gBanglaLigMap.insert(std::make_pair( 0xE3BE, dvngLig( 350 ,"0x09A6 0x09CD 0x09AD 0x09B0 0x09CD"))); // 0x100FE
    gBanglaLigMap.insert(std::make_pair( 0xE3BF, dvngLig( 351 ,"0x09A6 0x09CD 0x09AE"))); // 0x100FF
    gBanglaLigMap.insert(std::make_pair( 0xE3C0, dvngLig( 352 ,"0x09A7 0x09CD 0x09A8"))); // 0x10100
    gBanglaLigMap.insert(std::make_pair( 0xE3C1, dvngLig( 353 ,"0x09A7 0x09CD 0x09AE"))); // 0x10101
    gBanglaLigMap.insert(std::make_pair( 0xE3C2, dvngLig( 354 ,"0x09A8 0x09CD 0x09A4"))); // 0x10102
    gBanglaLigMap.insert(std::make_pair( 0xE3C3, dvngLig( 355 ,"0x09A8 0x09CD 0x09A5"))); // 0x10103
    gBanglaLigMap.insert(std::make_pair( 0xE3C4, dvngLig( 356 ,"0x09A8 0x09CD 0x09A5 0x09CD 0x09B0"))); // 0x10104
    gBanglaLigMap.insert(std::make_pair( 0xE3C5, dvngLig( 356 ,"0x09A8 0x09CD 0x09A5 0x09CD 0x09F0"))); // 0x10104
    gBanglaLigMap.insert(std::make_pair( 0xE3C6, dvngLig( 356 ,"0x09A8 0x09CD 0x09A5 0x09F0 0x09CD"))); // 0x10104
    gBanglaLigMap.insert(std::make_pair( 0xE3C7, dvngLig( 356 ,"0x09A8 0x09CD 0x09A5 0x09B0 0x09CD"))); // 0x10104
    gBanglaLigMap.insert(std::make_pair( 0xE3C8, dvngLig( 357 ,"0x09A8 0x09CD 0x09A0"))); // 0x10105
    gBanglaLigMap.insert(std::make_pair( 0xE3C9, dvngLig( 358 ,"0x09A8 0x09CD 0x09A1"))); // 0x10106
    gBanglaLigMap.insert(std::make_pair( 0xE3CA, dvngLig( 359 ,"0x09A8 0x09CD 0x09A1 0x09CD 0x09B0"))); // 0x10107
    gBanglaLigMap.insert(std::make_pair( 0xE3CB, dvngLig( 359 ,"0x09A8 0x09CD 0x09A1 0x09CD 0x09F0"))); // 0x10107
    gBanglaLigMap.insert(std::make_pair( 0xE3CC, dvngLig( 359 ,"0x09A8 0x09CD 0x09A1 0x09F0 0x09CD"))); // 0x10107
    gBanglaLigMap.insert(std::make_pair( 0xE3CD, dvngLig( 359 ,"0x09A8 0x09CD 0x09A1 0x09B0 0x09CD"))); // 0x10107
    gBanglaLigMap.insert(std::make_pair( 0xE3CE, dvngLig( 360 ,"0x09A8 0x09CD 0x09A6"))); // 0x10108
    gBanglaLigMap.insert(std::make_pair( 0xE3CF, dvngLig( 361 ,"0x09A8 0x09CD 0x09A7"))); // 0x10109
    gBanglaLigMap.insert(std::make_pair( 0xE3D0, dvngLig( 362 ,"0x09A8 0x09CD 0x09A8"))); // 0x1010A
    gBanglaLigMap.insert(std::make_pair( 0xE3D1, dvngLig( 363 ,"0x09A8 0x09CD 0x09AE"))); // 0x1010B
    gBanglaLigMap.insert(std::make_pair( 0xE3D2, dvngLig( 364 ,"0x09A8 0x09CD 0x09B8"))); // 0x1010C
    gBanglaLigMap.insert(std::make_pair( 0xE3D3, dvngLig( 365 ,"0x09A8 0x09CD 0x09A4 0x09CD 0x09AC"))); // 0x1010D
    gBanglaLigMap.insert(std::make_pair( 0xE3D4, dvngLig( 365 ,"0x09A8 0x09CD 0x09A4 0x09AC 0x09CD"))); // 0x1010D
    gBanglaLigMap.insert(std::make_pair( 0xE3D5, dvngLig( 366 ,"0x09A8 0x09CD 0x09A4 0x09CD 0x09B0"))); // 0x1010E
    gBanglaLigMap.insert(std::make_pair( 0xE3D6, dvngLig( 366 ,"0x09A8 0x09CD 0x09A4 0x09CD 0x09F0"))); // 0x1010E
    gBanglaLigMap.insert(std::make_pair( 0xE3D7, dvngLig( 366 ,"0x09A8 0x09CD 0x09A4 0x09F0 0x09CD"))); // 0x1010E
    gBanglaLigMap.insert(std::make_pair( 0xE3D8, dvngLig( 366 ,"0x09A8 0x09CD 0x09A4 0x09B0 0x09CD"))); // 0x1010E
    gBanglaLigMap.insert(std::make_pair( 0xE3D9, dvngLig( 367 ,"0x09A8 0x09CD 0x099F"))); // 0x1010F
    gBanglaLigMap.insert(std::make_pair( 0xE3DA, dvngLig( 368 ,"0x09A8 0x09CD 0x099F 0x09CD 0x09B0"))); // 0x10110
    gBanglaLigMap.insert(std::make_pair( 0xE3DB, dvngLig( 368 ,"0x09A8 0x09CD 0x099F 0x09CD 0x09F0"))); // 0x10110
    gBanglaLigMap.insert(std::make_pair( 0xE3DC, dvngLig( 368 ,"0x09A8 0x09CD 0x099F 0x09F0 0x09CD"))); // 0x10110
    gBanglaLigMap.insert(std::make_pair( 0xE3DD, dvngLig( 368 ,"0x09A8 0x09CD 0x099F 0x09B0 0x09CD"))); // 0x10110
    gBanglaLigMap.insert(std::make_pair( 0xE3DE, dvngLig( 369 ,"0x09A8 0x09CD 0x09A6 0x09CD 0x09B0"))); // 0x10111
    gBanglaLigMap.insert(std::make_pair( 0xE3DF, dvngLig( 369 ,"0x09A8 0x09CD 0x09A6 0x09CD 0x09F0"))); // 0x10111
    gBanglaLigMap.insert(std::make_pair( 0xE3E0, dvngLig( 369 ,"0x09A8 0x09CD 0x09A6 0x09F0 0x09CD"))); // 0x10111
    gBanglaLigMap.insert(std::make_pair( 0xE3E1, dvngLig( 369 ,"0x09A8 0x09CD 0x09A6 0x09B0 0x09CD"))); // 0x10111
    gBanglaLigMap.insert(std::make_pair( 0xE3E2, dvngLig( 370 ,"0x09A8 0x09CD 0x09A6 0x09CD 0x09AC"))); // 0x10112
    gBanglaLigMap.insert(std::make_pair( 0xE3E3, dvngLig( 370 ,"0x09A8 0x09CD 0x09A6 0x09AC 0x09CD"))); // 0x10112
    gBanglaLigMap.insert(std::make_pair( 0xE3E4, dvngLig( 371 ,"0x09A8 0x09CD 0x09A7 0x09CD 0x09B0"))); // 0x10113
    gBanglaLigMap.insert(std::make_pair( 0xE3E5, dvngLig( 371 ,"0x09A8 0x09CD 0x09A7 0x09CD 0x09F0"))); // 0x10113
    gBanglaLigMap.insert(std::make_pair( 0xE3E6, dvngLig( 371 ,"0x09A8 0x09CD 0x09A7 0x09F0 0x09CD"))); // 0x10113
    gBanglaLigMap.insert(std::make_pair( 0xE3E7, dvngLig( 371 ,"0x09A8 0x09CD 0x09A7 0x09B0 0x09CD"))); // 0x10113
    gBanglaLigMap.insert(std::make_pair( 0xE3E8, dvngLig( 372 ,"0x09AA 0x09CD 0x099F"))); // 0x10114
    gBanglaLigMap.insert(std::make_pair( 0xE3E9, dvngLig( 373 ,"0x09AA 0x09CD 0x09AA"))); // 0x10115
    gBanglaLigMap.insert(std::make_pair( 0xE3EA, dvngLig( 374 ,"0x09AA 0x09CD 0x09A8"))); // 0x10116
    gBanglaLigMap.insert(std::make_pair( 0xE3EB, dvngLig( 375 ,"0x09AA 0x09CD 0x09A4"))); // 0x10117
    gBanglaLigMap.insert(std::make_pair( 0xE3EC, dvngLig( 376 ,"0x09AA 0x09CD 0x09AE"))); // 0x10118
    gBanglaLigMap.insert(std::make_pair( 0xE3ED, dvngLig( 377 ,"0x09AA 0x09CD 0x09B2"))); // 0x10119
    gBanglaLigMap.insert(std::make_pair( 0xE3EE, dvngLig( 378 ,"0x09AA 0x09CD 0x09B8"))); // 0x1011A
    gBanglaLigMap.insert(std::make_pair( 0xE3EF, dvngLig( 379 ,"0x09AB 0x09CD 0x099F"))); // 0x1011B
    gBanglaLigMap.insert(std::make_pair( 0xE3F0, dvngLig( 380 ,"0x09AB 0x09CD 0x09B2"))); // 0x1011C
    gBanglaLigMap.insert(std::make_pair( 0xE3F1, dvngLig( 381 ,"0x09AC 0x09CD 0x099C"))); // 0x1011D
    gBanglaLigMap.insert(std::make_pair( 0xE3F2, dvngLig( 382 ,"0x09AC 0x09CD 0x09A6"))); // 0x1011E
    gBanglaLigMap.insert(std::make_pair( 0xE3F3, dvngLig( 383 ,"0x09AC 0x09CD 0x09A7"))); // 0x1011F
    gBanglaLigMap.insert(std::make_pair( 0xE3F4, dvngLig( 384 ,"0x09AC 0x09CD 0x09AD"))); // 0x10120
    gBanglaLigMap.insert(std::make_pair( 0xE3F5, dvngLig( 385 ,"0x09AC 0x09CD 0x09B2"))); // 0x10121
    gBanglaLigMap.insert(std::make_pair( 0xE3F6, dvngLig( 386 ,"0x09AC 0x09CD 0x09A6 0x09CD 0x09B0"))); // 0x10122
    gBanglaLigMap.insert(std::make_pair( 0xE3F7, dvngLig( 386 ,"0x09AC 0x09CD 0x09A6 0x09CD 0x09F0"))); // 0x10122
    gBanglaLigMap.insert(std::make_pair( 0xE3F8, dvngLig( 386 ,"0x09AC 0x09CD 0x09A6 0x09F0 0x09CD"))); // 0x10122
    gBanglaLigMap.insert(std::make_pair( 0xE3F9, dvngLig( 386 ,"0x09AC 0x09CD 0x09A6 0x09B0 0x09CD"))); // 0x10122
    gBanglaLigMap.insert(std::make_pair( 0xE3FA, dvngLig( 387 ,"0x09AD 0x09CD 0x09B2"))); // 0x10123
    //gBanglaLigMap.insert(std::make_pair( 0xE3FB, dvngLig( 388 ,"0x09AE 0x09CD 0x09A4"))); // 0x10124  //??!
    gBanglaLigMap.insert(std::make_pair( 0xE3FC, dvngLig( 389 ,"0x09AE 0x09CD 0x09A6"))); // 0x10125
    //gBanglaLigMap.insert(std::make_pair( 0xE3FD, dvngLig( 390 ,"0x09AE 0x09CD 0x09A8"))); // 0x10126  //??!
    gBanglaLigMap.insert(std::make_pair( 0xE3FE, dvngLig( 391 ,"0x09AE 0x09CD 0x09AA"))); // 0x10127
    gBanglaLigMap.insert(std::make_pair( 0xE3FF, dvngLig( 392 ,"0x09AE 0x09CD 0x09AB"))); // 0x10128
    gBanglaLigMap.insert(std::make_pair( 0xE400, dvngLig( 393 ,"0x09AE 0x09CD 0x09AD"))); // 0x10129
    gBanglaLigMap.insert(std::make_pair( 0xE401, dvngLig( 394 ,"0x09AE 0x09CD 0x09AE"))); // 0x1012A
    gBanglaLigMap.insert(std::make_pair( 0xE402, dvngLig( 395 ,"0x09AE 0x09CD 0x09B2"))); // 0x1012B
    gBanglaLigMap.insert(std::make_pair( 0xE403, dvngLig( 396 ,"0x09AE 0x09CD 0x09B8"))); // 0x1012C
    gBanglaLigMap.insert(std::make_pair( 0xE404, dvngLig( 397 ,"0x09AE 0x09CD 0x09AA 0x09CD 0x09B0"))); // 0x1012D
    gBanglaLigMap.insert(std::make_pair( 0xE405, dvngLig( 397 ,"0x09AE 0x09CD 0x09AA 0x09CD 0x09F0"))); // 0x1012D
    gBanglaLigMap.insert(std::make_pair( 0xE406, dvngLig( 397 ,"0x09AE 0x09CD 0x09AA 0x09F0 0x09CD"))); // 0x1012D
    gBanglaLigMap.insert(std::make_pair( 0xE407, dvngLig( 397 ,"0x09AE 0x09CD 0x09AA 0x09B0 0x09CD"))); // 0x1012D
    gBanglaLigMap.insert(std::make_pair( 0xE408, dvngLig( 398 ,"0x09AE 0x09CD 0x09AC 0x09CD 0x09B0"))); // 0x1012E
    gBanglaLigMap.insert(std::make_pair( 0xE409, dvngLig( 398 ,"0x09AE 0x09CD 0x09AC 0x09CD 0x09F0"))); // 0x1012E
    gBanglaLigMap.insert(std::make_pair( 0xE40A, dvngLig( 398 ,"0x09AE 0x09CD 0x09AC 0x09F0 0x09CD"))); // 0x1012E
    gBanglaLigMap.insert(std::make_pair( 0xE40B, dvngLig( 398 ,"0x09AE 0x09CD 0x09AC 0x09B0 0x09CD"))); // 0x1012E
    gBanglaLigMap.insert(std::make_pair( 0xE40C, dvngLig( 398 ,"0x09AE 0x09AC 0x09CD 0x09CD 0x09B0"))); // 0x1012E
    gBanglaLigMap.insert(std::make_pair( 0xE40D, dvngLig( 398 ,"0x09AE 0x09AC 0x09CD 0x09CD 0x09F0"))); // 0x1012E
    gBanglaLigMap.insert(std::make_pair( 0xE40E, dvngLig( 398 ,"0x09AE 0x09AC 0x09CD 0x09F0 0x09CD"))); // 0x1012E
    gBanglaLigMap.insert(std::make_pair( 0xE40F, dvngLig( 398 ,"0x09AE 0x09AC 0x09CD 0x09B0 0x09CD"))); // 0x1012E
    gBanglaLigMap.insert(std::make_pair( 0xE410, dvngLig( 399 ,"0x09AE 0x09CD 0x09AD 0x09CD 0x09B0"))); // 0x1012F
    gBanglaLigMap.insert(std::make_pair( 0xE411, dvngLig( 399 ,"0x09AE 0x09CD 0x09AD 0x09CD 0x09F0"))); // 0x1012F
    gBanglaLigMap.insert(std::make_pair( 0xE412, dvngLig( 399 ,"0x09AE 0x09CD 0x09AD 0x09F0 0x09CD"))); // 0x1012F
    gBanglaLigMap.insert(std::make_pair( 0xE413, dvngLig( 399 ,"0x09AE 0x09CD 0x09AD 0x09B0 0x09CD"))); // 0x1012F
    gBanglaLigMap.insert(std::make_pair( 0xE414, dvngLig( 400 ,"0x09B2 0x09CD 0x0995"))); // 0x10130
    gBanglaLigMap.insert(std::make_pair( 0xE415, dvngLig( 401 ,"0x09B2 0x09CD 0x0997"))); // 0x10131
    gBanglaLigMap.insert(std::make_pair( 0xE416, dvngLig( 402 ,"0x09B2 0x09CD 0x099F"))); // 0x10132
    gBanglaLigMap.insert(std::make_pair( 0xE417, dvngLig( 403 ,"0x09B2 0x09CD 0x09A1"))); // 0x10133
    gBanglaLigMap.insert(std::make_pair( 0xE418, dvngLig( 404 ,"0x09B2 0x09CD 0x09A4"))); // 0x10134
    gBanglaLigMap.insert(std::make_pair( 0xE419, dvngLig( 405 ,"0x09B2 0x09CD 0x09A6"))); // 0x10135
    gBanglaLigMap.insert(std::make_pair( 0xE41A, dvngLig( 406 ,"0x09B2 0x09CD 0x09A7"))); // 0x10136
    gBanglaLigMap.insert(std::make_pair( 0xE41B, dvngLig( 407 ,"0x09B2 0x09CD 0x09AA"))); // 0x10137
    gBanglaLigMap.insert(std::make_pair( 0xE41C, dvngLig( 408 ,"0x09B2 0x09CD 0x09AB"))); // 0x10138
    gBanglaLigMap.insert(std::make_pair( 0xE41D, dvngLig( 409 ,"0x09B2 0x09CD 0x09AE"))); // 0x10139
    gBanglaLigMap.insert(std::make_pair( 0xE41E, dvngLig( 410 ,"0x09B2 0x09CD 0x09B2"))); // 0x1013A
    gBanglaLigMap.insert(std::make_pair( 0xE41F, dvngLig( 411 ,"0x09B2 0x09CD 0x099F 0x09CD 0x09B0"))); // 0x1013B
    gBanglaLigMap.insert(std::make_pair( 0xE420, dvngLig( 411 ,"0x09B2 0x09CD 0x099F 0x09CD 0x09F0"))); // 0x1013B
    gBanglaLigMap.insert(std::make_pair( 0xE421, dvngLig( 411 ,"0x09B2 0x09CD 0x099F 0x09F0 0x09CD"))); // 0x1013B
    gBanglaLigMap.insert(std::make_pair( 0xE422, dvngLig( 411 ,"0x09B2 0x09CD 0x099F 0x09B0 0x09CD"))); // 0x1013B
    gBanglaLigMap.insert(std::make_pair( 0xE423, dvngLig( 412 ,"0x09B2 0x09CD 0x09A1 0x09CD 0x09B0"))); // 0x1013C
    gBanglaLigMap.insert(std::make_pair( 0xE424, dvngLig( 412 ,"0x09B2 0x09CD 0x09A1 0x09CD 0x09F0"))); // 0x1013C
    gBanglaLigMap.insert(std::make_pair( 0xE425, dvngLig( 412 ,"0x09B2 0x09CD 0x09A1 0x09F0 0x09CD"))); // 0x1013C
    gBanglaLigMap.insert(std::make_pair( 0xE426, dvngLig( 412 ,"0x09B2 0x09CD 0x09A1 0x09B0 0x09CD"))); // 0x1013C
    gBanglaLigMap.insert(std::make_pair( 0xE427, dvngLig( 413 ,"0x09B6 0x09CD 0x099A"))); // 0x1013D
    gBanglaLigMap.insert(std::make_pair( 0xE428, dvngLig( 414 ,"0x09B6 0x09CD 0x099B"))); // 0x1013E
    gBanglaLigMap.insert(std::make_pair( 0xE429, dvngLig( 415 ,"0x09B6 0x09CD 0x09A4"))); // 0x1013F
    gBanglaLigMap.insert(std::make_pair( 0xE42A, dvngLig( 416 ,"0x09B6 0x09CD 0x09A8"))); // 0x10140
    gBanglaLigMap.insert(std::make_pair( 0xE42B, dvngLig( 417 ,"0x09B6 0x09CD 0x09AE"))); // 0x10141
    gBanglaLigMap.insert(std::make_pair( 0xE42C, dvngLig( 418 ,"0x09B6 0x09CD 0x09B2"))); // 0x10142
    gBanglaLigMap.insert(std::make_pair( 0xE42D, dvngLig( 419 ,"0x09B7 0x09CD 0x0995"))); // 0x10143
    gBanglaLigMap.insert(std::make_pair( 0xE42E, dvngLig( 420 ,"0x09B7 0x09CD 0x099F"))); // 0x10144
    gBanglaLigMap.insert(std::make_pair( 0xE42F, dvngLig( 421 ,"0x09B7 0x09CD 0x09A0"))); // 0x10145
    gBanglaLigMap.insert(std::make_pair( 0xE430, dvngLig( 422 ,"0x09B7 0x09CD 0x09A3"))); // 0x10146
    gBanglaLigMap.insert(std::make_pair( 0xE431, dvngLig( 423 ,"0x09B7 0x09CD 0x09AA"))); // 0x10147
    gBanglaLigMap.insert(std::make_pair( 0xE432, dvngLig( 424 ,"0x09B7 0x09CD 0x09AB"))); // 0x10148
    gBanglaLigMap.insert(std::make_pair( 0xE433, dvngLig( 425 ,"0x09B7 0x09CD 0x09AE"))); // 0x10149
    gBanglaLigMap.insert(std::make_pair( 0xE434, dvngLig( 426 ,"0x09B7 0x09CD 0x0995 0x09CD 0x09B0"))); // 0x1014A
    gBanglaLigMap.insert(std::make_pair( 0xE435, dvngLig( 426 ,"0x09B7 0x09CD 0x0995 0x09CD 0x09F0"))); // 0x1014A
    gBanglaLigMap.insert(std::make_pair( 0xE436, dvngLig( 426 ,"0x09B7 0x09CD 0x0995 0x09F0 0x09CD"))); // 0x1014A
    gBanglaLigMap.insert(std::make_pair( 0xE437, dvngLig( 426 ,"0x09B7 0x09CD 0x0995 0x09B0 0x09CD"))); // 0x1014A
    gBanglaLigMap.insert(std::make_pair( 0xE438, dvngLig( 427 ,"0x09B7 0x09CD 0x099F 0x09CD 0x09B0"))); // 0x1014B
    gBanglaLigMap.insert(std::make_pair( 0xE439, dvngLig( 427 ,"0x09B7 0x09CD 0x099F 0x09CD 0x09F0"))); // 0x1014B
    gBanglaLigMap.insert(std::make_pair( 0xE43A, dvngLig( 427 ,"0x09B7 0x09CD 0x099F 0x09F0 0x09CD"))); // 0x1014B
    gBanglaLigMap.insert(std::make_pair( 0xE43B, dvngLig( 427 ,"0x09B7 0x09CD 0x099F 0x09B0 0x09CD"))); // 0x1014B
    gBanglaLigMap.insert(std::make_pair( 0xE43C, dvngLig( 428 ,"0x09B8 0x09CD 0x0995"))); // 0x1014C
    gBanglaLigMap.insert(std::make_pair( 0xE43D, dvngLig( 429 ,"0x09B8 0x09CD 0x0995 0x09CD 0x09B0"))); // 0x1014D
    gBanglaLigMap.insert(std::make_pair( 0xE43E, dvngLig( 429 ,"0x09B8 0x09CD 0x0995 0x09CD 0x09F0"))); // 0x1014D
    gBanglaLigMap.insert(std::make_pair( 0xE43F, dvngLig( 429 ,"0x09B8 0x09CD 0x0995 0x09F0 0x09CD"))); // 0x1014D
    gBanglaLigMap.insert(std::make_pair( 0xE440, dvngLig( 429 ,"0x09B8 0x09CD 0x0995 0x09B0 0x09CD"))); // 0x1014D
    gBanglaLigMap.insert(std::make_pair( 0xE441, dvngLig( 430 ,"0x09B8 0x09CD 0x0996"))); // 0x1014E
    gBanglaLigMap.insert(std::make_pair( 0xE442, dvngLig( 431 ,"0x09B8 0x09CD 0x099F"))); // 0x1014F
    gBanglaLigMap.insert(std::make_pair( 0xE443, dvngLig( 432 ,"0x09B8 0x09CD 0x09A4"))); // 0x10150
    gBanglaLigMap.insert(std::make_pair( 0xE444, dvngLig( 433 ,"0x09B8 0x09CD 0x09A4 0x09CD 0x09AC"))); // 0x10151
    gBanglaLigMap.insert(std::make_pair( 0xE445, dvngLig( 433 ,"0x09B8 0x09CD 0x09A4 0x09AC 0x09CD"))); // 0x10151
    gBanglaLigMap.insert(std::make_pair( 0xE446, dvngLig( 434 ,"0x09B8 0x09CD 0x09A5"))); // 0x10152
    gBanglaLigMap.insert(std::make_pair( 0xE447, dvngLig( 435 ,"0x09B8 0x09CD 0x09A8"))); // 0x10153
    gBanglaLigMap.insert(std::make_pair( 0xE448, dvngLig( 436 ,"0x09B8 0x09CD 0x09AA"))); // 0x10154
    gBanglaLigMap.insert(std::make_pair( 0xE449, dvngLig( 437 ,"0x09B8 0x09CD 0x09AB"))); // 0x10155
    gBanglaLigMap.insert(std::make_pair( 0xE44A, dvngLig( 438 ,"0x09B8 0x09CD 0x09AE"))); // 0x10156
    gBanglaLigMap.insert(std::make_pair( 0xE44B, dvngLig( 439 ,"0x09B8 0x09CD 0x09B2"))); // 0x10157
    gBanglaLigMap.insert(std::make_pair( 0xE44C, dvngLig( 440 ,"0x09B8 0x09CD 0x099F 0x09CD 0x09B0"))); // 0x10158
    gBanglaLigMap.insert(std::make_pair( 0xE44D, dvngLig( 440 ,"0x09B8 0x09CD 0x099F 0x09CD 0x09F0"))); // 0x10158
    gBanglaLigMap.insert(std::make_pair( 0xE44E, dvngLig( 440 ,"0x09B8 0x09CD 0x099F 0x09F0 0x09CD"))); // 0x10158
    gBanglaLigMap.insert(std::make_pair( 0xE44F, dvngLig( 440 ,"0x09B8 0x09CD 0x099F 0x09B0 0x09CD"))); // 0x10158
    gBanglaLigMap.insert(std::make_pair( 0xE450, dvngLig( 441 ,"0x09B8 0x09CD 0x09A4 0x09CD 0x09B0"))); // 0x10159
    gBanglaLigMap.insert(std::make_pair( 0xE451, dvngLig( 441 ,"0x09B8 0x09CD 0x09A4 0x09CD 0x09F0"))); // 0x10159
    gBanglaLigMap.insert(std::make_pair( 0xE452, dvngLig( 441 ,"0x09B8 0x09CD 0x09A4 0x09F0 0x09CD"))); // 0x10159
    gBanglaLigMap.insert(std::make_pair( 0xE453, dvngLig( 441 ,"0x09B8 0x09CD 0x09A4 0x09B0 0x09CD"))); // 0x10159
    gBanglaLigMap.insert(std::make_pair( 0xE454, dvngLig( 442 ,"0x09B8 0x09CD 0x09AA 0x09CD 0x09B0"))); // 0x1015A
    gBanglaLigMap.insert(std::make_pair( 0xE455, dvngLig( 442 ,"0x09B8 0x09CD 0x09AA 0x09CD 0x09F0"))); // 0x1015A
    gBanglaLigMap.insert(std::make_pair( 0xE456, dvngLig( 442 ,"0x09B8 0x09CD 0x09AA 0x09F0 0x09CD"))); // 0x1015A
    gBanglaLigMap.insert(std::make_pair( 0xE457, dvngLig( 442 ,"0x09B8 0x09CD 0x09AA 0x09B0 0x09CD"))); // 0x1015A
    gBanglaLigMap.insert(std::make_pair( 0xE458, dvngLig( 443 ,"0x09B9 0x09CD 0x09A3"))); // 0x1015B
    gBanglaLigMap.insert(std::make_pair( 0xE459, dvngLig( 444 ,"0x09B9 0x09CD 0x09A8"))); // 0x1015C
    gBanglaLigMap.insert(std::make_pair( 0xE45A, dvngLig( 445 ,"0x09B9 0x09CD 0x09AE"))); // 0x1015D
    gBanglaLigMap.insert(std::make_pair( 0xE45B, dvngLig( 446 ,"0x09B9 0x09CD 0x09B2"))); // 0x1015E
    gBanglaLigMap.insert(std::make_pair( 0xE45C, dvngLig( 449 ,"0x0997 0x09C1"))); // 0x10161
    gBanglaLigMap.insert(std::make_pair( 0xE45D, dvngLig( 449 ,"0x0997 0x200D 0x09C1"))); // 0x10161
    gBanglaLigMap.insert(std::make_pair( 0xE45E, dvngLig( 450 ,"0x09B2 0x09CD 0x0997 0x09C1"))); // 0x10162
    gBanglaLigMap.insert(std::make_pair( 0xE45F, dvngLig( 451 ,"0x09B0 0x09C1"))); // 0x10163
    gBanglaLigMap.insert(std::make_pair( 0xE460, dvngLig( 451 ,"0x09B0 0x200D 0x09C1"))); // 0x10163
    gBanglaLigMap.insert(std::make_pair( 0xE461, dvngLig( 452 ,"0x09F0 0x09C1"))); // 0x10164
    gBanglaLigMap.insert(std::make_pair( 0xE462, dvngLig( 452 ,"0x09F0 0x200D 0x09C1"))); // 0x10164
    gBanglaLigMap.insert(std::make_pair( 0xE463, dvngLig( 453 ,"0x09B6 0x09C1"))); // 0x10165
    gBanglaLigMap.insert(std::make_pair( 0xE464, dvngLig( 453 ,"0x09B6 0x200D 0x09C1"))); // 0x10165
    gBanglaLigMap.insert(std::make_pair( 0xE465, dvngLig( 454 ,"0x09B9 0x09C1"))); // 0x10166
    gBanglaLigMap.insert(std::make_pair( 0xE466, dvngLig( 454 ,"0x09B9 0x200D 0x09C1"))); // 0x10166
    gBanglaLigMap.insert(std::make_pair( 0xE467, dvngLig( 455 ,"0x09DC 0x09C1"))); // 0x10167
    gBanglaLigMap.insert(std::make_pair( 0xE468, dvngLig( 456 ,"0x09DC 0x09C2"))); // 0x10168
    gBanglaLigMap.insert(std::make_pair( 0xE469, dvngLig( 457 ,"0x09DC 0x09C3"))); // 0x10169
    gBanglaLigMap.insert(std::make_pair( 0xE46A, dvngLig( 458 ,"0x09DC 0x09CD"))); // 0x1016A
    gBanglaLigMap.insert(std::make_pair( 0xE46B, dvngLig( 459 ,"0x09DD 0x09C1"))); // 0x1016B
    gBanglaLigMap.insert(std::make_pair( 0xE46C, dvngLig( 460 ,"0x09DD 0x09C2"))); // 0x1016C
    gBanglaLigMap.insert(std::make_pair( 0xE46D, dvngLig( 461 ,"0x09DD 0x09C3"))); // 0x1016D
    gBanglaLigMap.insert(std::make_pair( 0xE46E, dvngLig( 462 ,"0x09DD 0x09CD"))); // 0x1016E
    gBanglaLigMap.insert(std::make_pair( 0xE46F, dvngLig( 463 ,"0x0997 0x09CD 0x09B0 0x09C1"))); // 0x1016F
    gBanglaLigMap.insert(std::make_pair( 0xE470, dvngLig( 463 ,"0x0997 0x09CD 0x09F0 0x09C1"))); // 0x1016F
    gBanglaLigMap.insert(std::make_pair( 0xE471, dvngLig( 463 ,"0x0997 0x09F0 0x09CD 0x09C1"))); // 0x1016F
    gBanglaLigMap.insert(std::make_pair( 0xE472, dvngLig( 463 ,"0x0997 0x09B0 0x09CD 0x09C1"))); // 0x1016F
    gBanglaLigMap.insert(std::make_pair( 0xE473, dvngLig( 463 ,"0x0997 0x09CD 0x09B0 0x200D 0x09C1"))); // 0x1016F
    gBanglaLigMap.insert(std::make_pair( 0xE474, dvngLig( 464 ,"0x09A4 0x09CD 0x09B0 0x09C1"))); // 0x10170
    gBanglaLigMap.insert(std::make_pair( 0xE475, dvngLig( 464 ,"0x09A4 0x09CD 0x09F0 0x09C1"))); // 0x10170
    gBanglaLigMap.insert(std::make_pair( 0xE476, dvngLig( 464 ,"0x09A4 0x09F0 0x09CD 0x09C1"))); // 0x10170
    gBanglaLigMap.insert(std::make_pair( 0xE477, dvngLig( 464 ,"0x09A4 0x09B0 0x09CD 0x09C1"))); // 0x10170
    gBanglaLigMap.insert(std::make_pair( 0xE478, dvngLig( 464 ,"0x09A4 0x09CD 0x09B0 0x200D 0x09C1"))); // 0x10170
    gBanglaLigMap.insert(std::make_pair( 0xE479, dvngLig( 465 ,"0x09A5 0x09CD 0x09B0 0x09C1"))); // 0x10171
    gBanglaLigMap.insert(std::make_pair( 0xE47A, dvngLig( 465 ,"0x09A5 0x09CD 0x09F0 0x09C1"))); // 0x10171
    gBanglaLigMap.insert(std::make_pair( 0xE47B, dvngLig( 465 ,"0x09A5 0x09F0 0x09CD 0x09C1"))); // 0x10171
    gBanglaLigMap.insert(std::make_pair( 0xE47C, dvngLig( 465 ,"0x09A5 0x09B0 0x09CD 0x09C1"))); // 0x10171
    gBanglaLigMap.insert(std::make_pair( 0xE47D, dvngLig( 465 ,"0x09A5 0x09CD 0x09B0 0x200D 0x09C1"))); // 0x10171
    gBanglaLigMap.insert(std::make_pair( 0xE47E, dvngLig( 466 ,"0x09A6 0x09CD 0x09B0 0x09C1"))); // 0x10172
    gBanglaLigMap.insert(std::make_pair( 0xE47F, dvngLig( 466 ,"0x09A6 0x09CD 0x09F0 0x09C1"))); // 0x10172
    gBanglaLigMap.insert(std::make_pair( 0xE480, dvngLig( 466 ,"0x09A6 0x09F0 0x09CD 0x09C1"))); // 0x10172
    gBanglaLigMap.insert(std::make_pair( 0xE481, dvngLig( 466 ,"0x09A6 0x09B0 0x09CD 0x09C1"))); // 0x10172
    gBanglaLigMap.insert(std::make_pair( 0xE482, dvngLig( 466 ,"0x09A6 0x09CD 0x09B0 0x200D 0x09C1"))); // 0x10172
    gBanglaLigMap.insert(std::make_pair( 0xE483, dvngLig( 467 ,"0x09A7 0x09CD 0x09B0 0x09C1"))); // 0x10173
    gBanglaLigMap.insert(std::make_pair( 0xE484, dvngLig( 467 ,"0x09A7 0x09CD 0x09F0 0x09C1"))); // 0x10173
    gBanglaLigMap.insert(std::make_pair( 0xE485, dvngLig( 467 ,"0x09A7 0x09F0 0x09CD 0x09C1"))); // 0x10173
    gBanglaLigMap.insert(std::make_pair( 0xE486, dvngLig( 467 ,"0x09A7 0x09B0 0x09CD 0x09C1"))); // 0x10173
    gBanglaLigMap.insert(std::make_pair( 0xE487, dvngLig( 467 ,"0x09A7 0x09CD 0x09B0 0x200D 0x09C1"))); // 0x10173
    gBanglaLigMap.insert(std::make_pair( 0xE488, dvngLig( 468 ,"0x09A8 0x09CD 0x09A4 0x09C1"))); // 0x10174
    gBanglaLigMap.insert(std::make_pair( 0xE489, dvngLig( 468 ,"0x09A8 0x09CD 0x09A4 0x200D 0x09C1"))); // 0x10174
    gBanglaLigMap.insert(std::make_pair( 0xE48A, dvngLig( 469 ,"0x09AC 0x09CD 0x09B0 0x09C1"))); // 0x10175
    gBanglaLigMap.insert(std::make_pair( 0xE48B, dvngLig( 469 ,"0x09AC 0x09CD 0x09F0 0x09C1"))); // 0x10175
    gBanglaLigMap.insert(std::make_pair( 0xE48C, dvngLig( 469 ,"0x09AC 0x09F0 0x09CD 0x09C1"))); // 0x10175
    gBanglaLigMap.insert(std::make_pair( 0xE48D, dvngLig( 469 ,"0x09AC 0x09B0 0x09CD 0x09C1"))); // 0x10175
    gBanglaLigMap.insert(std::make_pair( 0xE48E, dvngLig( 469 ,"0x09AC 0x09CD 0x09B0 0x200D 0x09C1"))); // 0x10175
    gBanglaLigMap.insert(std::make_pair( 0xE48F, dvngLig( 470 ,"0x09AD 0x09CD 0x09B0 0x09C1"))); // 0x10176
    gBanglaLigMap.insert(std::make_pair( 0xE490, dvngLig( 470 ,"0x09AD 0x09CD 0x09F0 0x09C1"))); // 0x10176
    gBanglaLigMap.insert(std::make_pair( 0xE491, dvngLig( 470 ,"0x09AD 0x09F0 0x09CD 0x09C1"))); // 0x10176
    gBanglaLigMap.insert(std::make_pair( 0xE492, dvngLig( 470 ,"0x09AD 0x09B0 0x09CD 0x09C1"))); // 0x10176
    gBanglaLigMap.insert(std::make_pair( 0xE493, dvngLig( 470 ,"0x09AD 0x09CD 0x09B0 0x200D 0x09C1"))); // 0x10176
    gBanglaLigMap.insert(std::make_pair( 0xE494, dvngLig( 471 ,"0x09B6 0x09CD 0x09B0 0x09C1"))); // 0x10177
    gBanglaLigMap.insert(std::make_pair( 0xE495, dvngLig( 471 ,"0x09B6 0x09CD 0x09F0 0x09C1"))); // 0x10177
    gBanglaLigMap.insert(std::make_pair( 0xE496, dvngLig( 471 ,"0x09B6 0x09F0 0x09CD 0x09C1"))); // 0x10177
    gBanglaLigMap.insert(std::make_pair( 0xE497, dvngLig( 471 ,"0x09B6 0x09B0 0x09CD 0x09C1"))); // 0x10177
    gBanglaLigMap.insert(std::make_pair( 0xE498, dvngLig( 471 ,"0x09B6 0x09CD 0x09B0 0x200D 0x09C1"))); // 0x10177
    gBanglaLigMap.insert(std::make_pair( 0xE499, dvngLig( 472 ,"0x09B8 0x09CD 0x09A4 0x09C1"))); // 0x10178
    gBanglaLigMap.insert(std::make_pair( 0xE49A, dvngLig( 472 ,"0x09B8 0x09CD 0x09A4 0x200D 0x09C1"))); // 0x10178
    gBanglaLigMap.insert(std::make_pair( 0xE49B, dvngLig( 473 ,"0x09B8 0x09CD 0x09B0 0x09C1"))); // 0x10179
    gBanglaLigMap.insert(std::make_pair( 0xE49C, dvngLig( 473 ,"0x09B8 0x09CD 0x09F0 0x09C1"))); // 0x10179
    gBanglaLigMap.insert(std::make_pair( 0xE49D, dvngLig( 473 ,"0x09B8 0x09F0 0x09CD 0x09C1"))); // 0x10179
    gBanglaLigMap.insert(std::make_pair( 0xE49E, dvngLig( 473 ,"0x09B8 0x09B0 0x09CD 0x09C1"))); // 0x10179
    gBanglaLigMap.insert(std::make_pair( 0xE49F, dvngLig( 473 ,"0x09B8 0x09CD 0x09B0 0x200D 0x09C1"))); // 0x10179
    gBanglaLigMap.insert(std::make_pair( 0xE4A0, dvngLig( 474 ,"0x09B8 0x09CD 0x09B2 0x09C1"))); // 0x1017A
    gBanglaLigMap.insert(std::make_pair( 0xE4A1, dvngLig( 475 ,"0x09B0 0x09C2"))); // 0x1017B
    gBanglaLigMap.insert(std::make_pair( 0xE4A2, dvngLig( 475 ,"0x09B0 0x200D 0x09C2"))); // 0x1017B
    gBanglaLigMap.insert(std::make_pair( 0xE4A3, dvngLig( 476 ,"0x09F0 0x09C2"))); // 0x1017C
    gBanglaLigMap.insert(std::make_pair( 0xE4A4, dvngLig( 476 ,"0x09F0 0x200D 0x09C2"))); // 0x1017C
    gBanglaLigMap.insert(std::make_pair( 0xE4A5, dvngLig( 477 ,"0x0997 0x09CD 0x09B0 0x09C2"))); // 0x1017D
    gBanglaLigMap.insert(std::make_pair( 0xE4A6, dvngLig( 477 ,"0x0997 0x09CD 0x09F0 0x09C2"))); // 0x1017D
    gBanglaLigMap.insert(std::make_pair( 0xE4A7, dvngLig( 477 ,"0x0997 0x09F0 0x09CD 0x09C2"))); // 0x1017D
    gBanglaLigMap.insert(std::make_pair( 0xE4A8, dvngLig( 477 ,"0x0997 0x09B0 0x09CD 0x09C2"))); // 0x1017D
    gBanglaLigMap.insert(std::make_pair( 0xE4A9, dvngLig( 477 ,"0x0997 0x09CD 0x09B0 0x200D 0x09C2"))); // 0x1017D
    gBanglaLigMap.insert(std::make_pair( 0xE4AA, dvngLig( 478 ,"0x09A5 0x09CD 0x09B0 0x09C2"))); // 0x1017E
    gBanglaLigMap.insert(std::make_pair( 0xE4AB, dvngLig( 478 ,"0x09A5 0x09CD 0x09F0 0x09C2"))); // 0x1017E
    gBanglaLigMap.insert(std::make_pair( 0xE4AC, dvngLig( 478 ,"0x09A5 0x09F0 0x09CD 0x09C2"))); // 0x1017E
    gBanglaLigMap.insert(std::make_pair( 0xE4AD, dvngLig( 478 ,"0x09A5 0x09B0 0x09CD 0x09C2"))); // 0x1017E
    gBanglaLigMap.insert(std::make_pair( 0xE4AE, dvngLig( 478 ,"0x09A5 0x09CD 0x09B0 0x200D 0x09C2"))); // 0x1017E
    gBanglaLigMap.insert(std::make_pair( 0xE4AF, dvngLig( 479 ,"0x09A6 0x09CD 0x09B0 0x09C2"))); // 0x1017F
    gBanglaLigMap.insert(std::make_pair( 0xE4B0, dvngLig( 479 ,"0x09A6 0x09CD 0x09F0 0x09C2"))); // 0x1017F
    gBanglaLigMap.insert(std::make_pair( 0xE4B1, dvngLig( 479 ,"0x09A6 0x09F0 0x09CD 0x09C2"))); // 0x1017F
    gBanglaLigMap.insert(std::make_pair( 0xE4B2, dvngLig( 479 ,"0x09A6 0x09B0 0x09CD 0x09C2"))); // 0x1017F
    gBanglaLigMap.insert(std::make_pair( 0xE4B3, dvngLig( 479 ,"0x09A6 0x09CD 0x09B0 0x200D 0x09C2"))); // 0x1017F
    gBanglaLigMap.insert(std::make_pair( 0xE4B4, dvngLig( 480 ,"0x09A7 0x09CD 0x09B0 0x09C2"))); // 0x10180
    gBanglaLigMap.insert(std::make_pair( 0xE4B5, dvngLig( 480 ,"0x09A7 0x09CD 0x09F0 0x09C2"))); // 0x10180
    gBanglaLigMap.insert(std::make_pair( 0xE4B6, dvngLig( 480 ,"0x09A7 0x09F0 0x09CD 0x09C2"))); // 0x10180
    gBanglaLigMap.insert(std::make_pair( 0xE4B7, dvngLig( 480 ,"0x09A7 0x09B0 0x09CD 0x09C2"))); // 0x10180
    gBanglaLigMap.insert(std::make_pair( 0xE4B8, dvngLig( 480 ,"0x09A7 0x09CD 0x09B0 0x200D 0x09C2"))); // 0x10180
    gBanglaLigMap.insert(std::make_pair( 0xE4B9, dvngLig( 481 ,"0x09AD 0x09CD 0x09B0 0x09C2"))); // 0x10181
    gBanglaLigMap.insert(std::make_pair( 0xE4BA, dvngLig( 481 ,"0x09AD 0x09CD 0x09F0 0x09C2"))); // 0x10181
    gBanglaLigMap.insert(std::make_pair( 0xE4BB, dvngLig( 481 ,"0x09AD 0x09F0 0x09CD 0x09C2"))); // 0x10181
    gBanglaLigMap.insert(std::make_pair( 0xE4BC, dvngLig( 481 ,"0x09AD 0x09B0 0x09CD 0x09C2"))); // 0x10181
    gBanglaLigMap.insert(std::make_pair( 0xE4BD, dvngLig( 481 ,"0x09AD 0x09CD 0x09B0 0x200D 0x09C2"))); // 0x10181
    gBanglaLigMap.insert(std::make_pair( 0xE4BE, dvngLig( 482 ,"0x09B6 0x09CD 0x09B0 0x09C2"))); // 0x10182
    gBanglaLigMap.insert(std::make_pair( 0xE4BF, dvngLig( 482 ,"0x09B6 0x09CD 0x09F0 0x09C2"))); // 0x10182
    gBanglaLigMap.insert(std::make_pair( 0xE4C0, dvngLig( 482 ,"0x09B6 0x09F0 0x09CD 0x09C2"))); // 0x10182
    gBanglaLigMap.insert(std::make_pair( 0xE4C1, dvngLig( 482 ,"0x09B6 0x09B0 0x09CD 0x09C2"))); // 0x10182
    gBanglaLigMap.insert(std::make_pair( 0xE4C2, dvngLig( 482 ,"0x09B6 0x09CD 0x09B0 0x200D 0x09C2"))); // 0x10182
    gBanglaLigMap.insert(std::make_pair( 0xE4C3, dvngLig( 483 ,"0x09B8 0x09CD 0x09B0 0x09C2"))); // 0x10183
    gBanglaLigMap.insert(std::make_pair( 0xE4C4, dvngLig( 483 ,"0x09B8 0x09CD 0x09F0 0x09C2"))); // 0x10183
    gBanglaLigMap.insert(std::make_pair( 0xE4C5, dvngLig( 483 ,"0x09B8 0x09F0 0x09CD 0x09C2"))); // 0x10183
    gBanglaLigMap.insert(std::make_pair( 0xE4C6, dvngLig( 483 ,"0x09B8 0x09B0 0x09CD 0x09C2"))); // 0x10183
    gBanglaLigMap.insert(std::make_pair( 0xE4C7, dvngLig( 483 ,"0x09B8 0x09CD 0x09B0 0x200D 0x09C2"))); // 0x10183
    gBanglaLigMap.insert(std::make_pair( 0xE4C8, dvngLig( 484 ,"0x09B9 0x09C3"))); // 0x10184
    gBanglaLigMap.insert(std::make_pair( 0xE4C9, dvngLig( 484 ,"0x09B9 0x200D 0x09C3"))); // 0x10184
    gBanglaLigMap.insert(std::make_pair( 0xE4CA, dvngLig( 486 ,"0x09F0 0x09CD 0x0981"))); // 0x10186
    gBanglaLigMap.insert(std::make_pair( 0xE4CB, dvngLig( 486 ,"0x09B0 0x09CD 0x0981"))); // 0x10186
    gBanglaLigMap.insert(std::make_pair( 0xE4CC, dvngLig( 486 ,"0x0981 0x09F0 0x09CD"))); // 0x10186
    gBanglaLigMap.insert(std::make_pair( 0xE4CD, dvngLig( 486 ,"0x0981 0x09B0 0x09CD"))); // 0x10186
    gBanglaLigMap.insert(std::make_pair( 0xE4CE, dvngLig( 487 ,"0x09C0 0x0981"))); // 0x10187
    gBanglaLigMap.insert(std::make_pair( 0xE4CF, dvngLig( 487 ,"0x0981 0x09C0"))); // 0x10187
    gBanglaLigMap.insert(std::make_pair( 0xE4D0, dvngLig( 488 ,"0x09F0 0x09CD 0x09C0"))); // 0x10188
    gBanglaLigMap.insert(std::make_pair( 0xE4D1, dvngLig( 488 ,"0x09B0 0x09CD 0x09C0"))); // 0x10188
    gBanglaLigMap.insert(std::make_pair( 0xE4D2, dvngLig( 489 ,"0x09F0 0x09CD 0x09C0 0x0981"))); // 0x10189
    gBanglaLigMap.insert(std::make_pair( 0xE4D3, dvngLig( 489 ,"0x09B0 0x09CD 0x09C0 0x0981"))); // 0x10189
    gBanglaLigMap.insert(std::make_pair( 0xE4D4, dvngLig( 489 ,"0x09F0 0x09CD 0x0981 0x09C0"))); // 0x10189
    gBanglaLigMap.insert(std::make_pair( 0xE4D5, dvngLig( 489 ,"0x09B0 0x09CD 0x0981 0x09C0"))); // 0x10189
    gBanglaLigMap.insert(std::make_pair( 0xE4D6, dvngLig( 500 ,"0x0981 0x09D7"))); // 0x10194
    gBanglaLigMap.insert(std::make_pair( 0xE4D7, dvngLig( 501 ,"0x09F0 0x09CD 0x0981 0x09D7"))); // 0x10195
    gBanglaLigMap.insert(std::make_pair( 0xE4D8, dvngLig( 501 ,"0x09B0 0x09CD 0x0981 0x09D7"))); // 0x10195
    gBanglaLigMap.insert(std::make_pair( 0xE4D9, dvngLig( 501 ,"0x09F0 0x09CD 0x09D7 0x0981"))); // 0x10195
    gBanglaLigMap.insert(std::make_pair( 0xE4DA, dvngLig( 501 ,"0x09B0 0x09CD 0x09D7 0x0981"))); // 0x10195
    gBanglaLigMap.insert(std::make_pair( 0xE4DB, dvngLig( 501 ,"0x0981 0x09F0 0x09CD 0x09D7"))); // 0x10195
    gBanglaLigMap.insert(std::make_pair( 0xE4DC, dvngLig( 501 ,"0x0981 0x09B0 0x09CD 0x09D7"))); // 0x10195
    gBanglaLigMap.insert(std::make_pair( 0xE4DD, dvngLig( 502 ,"0x099F 0x09C0"))); // 0x10196
    gBanglaLigMap.insert(std::make_pair( 0xE4DE, dvngLig( 503 ,"0x09B7 0x09CD 0x099F 0x09C0"))); // 0x10197
    gBanglaLigMap.insert(std::make_pair( 0xE4DF, dvngLig( 504 ,"0x09B8 0x09CD 0x099F 0x09C0"))); // 0x10198
    gBanglaLigMap.insert(std::make_pair( 0xE4E0, dvngLig( 505 ,"0x09B8 0x09CD 0x099F 0x09CD 0x09B0 0x09C0"))); // 0x10199
    gBanglaLigMap.insert(std::make_pair( 0xE4E1, dvngLig( 505 ,"0x09B8 0x09CD 0x099F 0x09CD 0x09F0 0x09C0"))); // 0x10199
    gBanglaLigMap.insert(std::make_pair( 0xE4E2, dvngLig( 505 ,"0x09B8 0x09CD 0x099F 0x09F0 0x09CD 0x09C0"))); // 0x10199
    gBanglaLigMap.insert(std::make_pair( 0xE4E3, dvngLig( 505 ,"0x09B8 0x09CD 0x099F 0x09B0 0x09CD 0x09C0"))); // 0x10199
    //gBanglaLigMap.insert(std::make_pair( 0xE4E4, dvngLig( 506 ,"0x09BF 0x099F"))); // 0x1019A //??!
    gBanglaLigMap.insert(std::make_pair( 0xE4E5, dvngLig( 507 ,"0x09BF 0x09A0"))); // 0x1019B
    gBanglaLigMap.insert(std::make_pair( 0xE4E6, dvngLig( 508 ,"0x09BF 0x099F 0x09CD 0x09B0"))); // 0x1019C
    gBanglaLigMap.insert(std::make_pair( 0xE4E7, dvngLig( 508 ,"0x09BF 0x099F 0x09CD 0x09F0"))); // 0x1019C
    gBanglaLigMap.insert(std::make_pair( 0xE4E8, dvngLig( 508 ,"0x09BF 0x099F 0x09F0 0x09CD"))); // 0x1019C
    gBanglaLigMap.insert(std::make_pair( 0xE4E9, dvngLig( 508 ,"0x09BF 0x099F 0x09B0 0x09CD"))); // 0x1019C
    //gBanglaLigMap.insert(std::make_pair( 0xE4EA, dvngLig( 509 ,"0x09BF 0x09B7 0x09CD 0x099F"))); // 0x1019D //??!
    gBanglaLigMap.insert(std::make_pair( 0xE4EB, dvngLig( 510 ,"0x09F0 0x09CD 0x0981 0x200D 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4EC, dvngLig( 510 ,"0x09F0 0x09CD 0x0981 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4ED, dvngLig( 510 ,"0x09F0 0x09CD 0x0981 0x09AF 0x09CD 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4EE, dvngLig( 510 ,"0x09B0 0x09CD 0x0981 0x200D 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4EF, dvngLig( 510 ,"0x09B0 0x09CD 0x0981 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4F0, dvngLig( 510 ,"0x09B0 0x09CD 0x0981 0x09AF 0x09CD 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4F1, dvngLig( 510 ,"0x0981 0x09F0 0x09CD 0x200D 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4F2, dvngLig( 510 ,"0x0981 0x09F0 0x09CD 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4F3, dvngLig( 510 ,"0x0981 0x09F0 0x09CD 0x09AF 0x09CD 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4F4, dvngLig( 510 ,"0x0981 0x09B0 0x09CD 0x200D 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4F5, dvngLig( 510 ,"0x0981 0x09B0 0x09CD 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4F6, dvngLig( 510 ,"0x0981 0x09B0 0x09CD 0x09AF 0x09CD 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4F7, dvngLig( 510 ,"0x09F0 0x09CD 0x200D 0x09CD 0x09AF 0x09C0 0x0981"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4F8, dvngLig( 510 ,"0x09F0 0x09CD 0x200D 0x09CD 0x09AF 0x0981 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4F9, dvngLig( 510 ,"0x09F0 0x09CD 0x09CD 0x09AF 0x09C0 0x0981"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4FA, dvngLig( 510 ,"0x09F0 0x09CD 0x09CD 0x09AF 0x0981 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4FB, dvngLig( 510 ,"0x09F0 0x09CD 0x09AF 0x09CD 0x09C0 0x0981"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4FC, dvngLig( 510 ,"0x09F0 0x09CD 0x09AF 0x09CD 0x0981 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4FD, dvngLig( 510 ,"0x09B0 0x09CD 0x200D 0x09CD 0x09AF 0x09C0 0x0981"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4FE, dvngLig( 510 ,"0x09B0 0x09CD 0x200D 0x09CD 0x09AF 0x0981 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE4FF, dvngLig( 510 ,"0x09B0 0x09CD 0x09CD 0x09AF 0x09C0 0x0981"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE500, dvngLig( 510 ,"0x09B0 0x09CD 0x09CD 0x09AF 0x0981 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE501, dvngLig( 510 ,"0x09B0 0x09CD 0x09AF 0x09CD 0x09C0 0x0981"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE502, dvngLig( 510 ,"0x09B0 0x09CD 0x09AF 0x09CD 0x0981 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE503, dvngLig( 510 ,"0x09F0 0x09CD 0x200D 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE504, dvngLig( 510 ,"0x09F0 0x09CD 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE505, dvngLig( 510 ,"0x09F0 0x09CD 0x09AF 0x09CD 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE506, dvngLig( 510 ,"0x09B0 0x09CD 0x200D 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE507, dvngLig( 510 ,"0x09B0 0x09CD 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE508, dvngLig( 510 ,"0x09B0 0x09CD 0x09AF 0x09CD 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE509, dvngLig( 510 ,"0x0981 0x200D 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE50A, dvngLig( 510 ,"0x0981 0x09CD 0x09AF 0x09C0"))); // 0x1019E
    gBanglaLigMap.insert(std::make_pair( 0xE50B, dvngLig( 510 ,"0x0981 0x09AF 0x09CD 0x09C0"))); // 0x1019E


    //BANGLA_END
    //debug printout
    //LE("gBanglaLigMap debug printout");
    //for (BanglaLigMap::iterator it = gBanglaLigMap.begin(); it != gBanglaLigMap.end() ; it++)
    //{
    //    dvngLig lig = it->second;
    //    LE("Lig 0x%X index = %d (len %d), 0x%X%s 0x%X%s 0x%X%s 0x%X%s 0x%X%s 0x%x%s ",it->first, it->second.glyphindex, it->second.len,
    //       it->second.a,it->second.a_mod.c_str(),
    //       it->second.b,it->second.b_mod.c_str(),
    //       it->second.c,it->second.c_mod.c_str(),
    //       it->second.d,it->second.d_mod.c_str(),
    //       it->second.e,it->second.e_mod.c_str(),
    //       it->second.f,it->second.f_mod.c_str());
    //}

    return gBanglaLigMap;
}

bool isViramaComboBangla(lChar16 ch)
{
    return (ch == 0xE225 ||     //Ra1 virama (reph)
            ch == 0xE226 ||     //Ra  virama (reph)
            ch == 0xE24C ||     //Ra1 virama (full)
            ch == 0xE266);      //Ra  virama (full)

}

bool isBangla_consonant(lChar16 ch)
{
    if(ch >= 0x0995 && ch <= 0x09B9) return true;
    if(ch >= 0xE204 && ch <= 0xE206) return true;
    return false;
}

void SwitchBanglaI(lString16* str)
{
/*
* (E ? i) -> (i E ?)  // E = 0x09C7
* (? J i) -> (i ? J)  // J = 0xE225
* (? j i) -> (i ? j)  // j = 0xE226
* (? x i) -> (i ? x)  // x = 0xE4EA #509 "0x09BF 0x09B7 0x09CD 0x099F" //0x1019D
* (? Y i) -> (i ? Y)  // Y = 0xE271 ya postform
* (? y i) -> (i ? y)  // y = 0xE272 ya postform
* (? U i) -> (i ? U)  // U = 0xE273 ya postform
* (? i)   -> (i ?)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x09BF)
        {
            if (i >= 2 && str->at(i - 2) == 0x09C7) //fix for bangla E vovel
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x09C7;
                str->at(i - 2) = 0x09BF;
            }
            else if (i >= 2 && (str->at(i - 1) == 0xE225 ||
                                str->at(i - 1) == 0xE226 ||
                                str->at(i - 1) == 0xE4EA ||
                                str->at(i - 1) == 0xE271 ||
                                str->at(i - 1) == 0xE272 ||
                                str->at(i - 1) == 0xE273)  )
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x9Bf;
            }
            else
            {
                //LE("i 0x9BF regular");
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x9BF;
            }
        }
    }
}

void SwitchBanglaReph(lString16* str)
{
/*
 *
 *  (? E)   -> (E ?)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 0; i < str->length()-1; i++)
    {
        if (str->at(i) == 0xE225)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0xE225;
            i++;
        }
        if (str->at(i) == 0xE226)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0xE226;
            i++;
        }
    }
}

void SwitchBanglaE(lString16* str)
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
        if (str->at(i) == 0x09C7)
        {
            if( str->at(i - 1) == 0xE271 ||  //ya postform
                str->at(i - 1) == 0xE272 ||  //ya postform
                str->at(i - 1) == 0xE273 ||  //ya postform
                str->at(i - 1) == 0xE225 ||  //reph
                str->at(i - 1) == 0xE226   ) //reph
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x09C7;
            }
            else
            {
                //default
                str->at(i + 0) = str->at(i - 1);
                str->at(i - 1) = 0x09C7;
            }
        }
    }
}

void SwitchBanglaAI(lString16* str)
{
/*
 *  (? E)   -> (E ?)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x09C8)
        {
            str->at(i) = str->at(i - 1);
            str->at(i - 1) = 0x09C8;
        }
    }
}

void SwitchBanglaO(lString16 *str)
{
/*
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
        if (str->at(i) == 0x09CB)
        {
            if (i > 1 && (str->at(i - 1) == 0xE226 ||
                          str->at(i - 1) == 0xE271 ||
                          str->at(i - 1) == 0xE272 ||
                          str->at(i - 1) == 0xE273) )
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x09C7;
                str->insert(i + 1, 1, 0x09BE);
            }
            else
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x09C7;
                str->insert(i + 1, 1, 0x09BE);
            }
        }
    }
}

void SwitchBanglaOU(lString16 *str)
{
/*
 * (? OU)   -> (e ? ou)
 * (? ' OU) -> (e ? ' ou) // ' == 0xE226 //ra virama
 * (? J OU) -> (e ? J ou) // J == 0xE271 //ya postform
 * (? j OU) -> (e ? j ou) // j == 0xE272 //ya postform
 * (? i OU) -> (e ? i ou) // i == 0xE273 //ya postform
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = 1; i < str->length(); i++)
    {
        if (str->at(i) == 0x09CC)
        {
            if (i > 1 && (str->at(i - 1) == 0xE226 ||
                          str->at(i - 1) == 0xE271 ||
                          str->at(i - 1) == 0xE272 ||
                          str->at(i - 1) == 0xE273) )
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = 0x09C7;
                str->insert(i + 1, 1, 0x09D7);
            }
            else
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = 0x09C7;
                str->insert(i + 1, 1, 0x09D7);
            }
        }
    }
}

void SwitchBanglaRaITa(lString16 *str)
{
    //0x09B0 0x09bf 0x09a4 -> 0x09BF 0x09A4 0xE226
    if (str->length() < 3)
    {
        return;
    }
    for (int i = 0; i < str->length()-2; i++)
    {
        if (str->at(i + 0) == 0x09B0 && str->at(i + 1) == 0x09BF && str->at(i + 2) == 0x09A4)
        {
            str->at(i + 0) = 0x09BF;
            str->at(i + 1) = 0x09A4;
            str->at(i + 2) = 0xE226;
        }
    }
}

void SwitchBanglaI_reverse(lString16* str)
{

/*
 * (i E ?) -> (E ? i)
 * (i ? J) -> (? J i) // J = 0xE225
 * (i ? j) -> (? j i) // j = 0xE226
 * (i ? x) -> (? j x) // x = 0xE4EA
 * (i ? Y) -> (? Y i) // Y = 0xE271 ya postform
 * (i ? y) -> (? y i) // y = 0xE272 ya postform
 * (i ? U) -> (? U i) // U = 0xE273 ya postform
 * (i ?)   -> (? i)
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if (str->at(i) == 0x09BF)
        {
            if (str->at(i + 1) == 0x09C7) //fix for bangla E vovel
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = 0x09BF;
            }
            else if (str->at(i+2) == 0xE225 ||
                     str->at(i+2) == 0xE226 ||
                     str->at(i+2) == 0xE4EA ||
                     str->at(i+2) == 0xE271 ||
                     str->at(i+2) == 0xE272 ||
                     str->at(i+2) == 0xE273   )
            {
                str->at(i + 0) = str->at(i+1);
                str->at(i + 1) = str->at(i+2);
                str->at(i + 2) = 0x09BF;
            }
            else
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x09Bf;
            }
        }
    }
}

void SwitchBanglaE_reverse(lString16* str)
{
/*  ( E ? 0xE271) -> ( ? 0xE271 E )
 *  ( E ? 0xE272) -> ( ? 0xE272 E )
 *  ( E ? 0xE273) -> ( ? 0xE273 E )
 *  ( E ? )       -> ( ? E )
 */
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2 ; i >= 0 ; i--)
    {
        if (str->at(i) == 0x09C7)
        {
            if( i <= str->length()-3 &&
                ( str->at(i + 2) == 0xE271 ||  //ya postform
                  str->at(i + 2) == 0xE272 ||  //ya postform
                  str->at(i + 2) == 0xE273 ||  //ya postform
                  str->at(i + 2) == 0xE225 ||  //reph
                  str->at(i + 2) == 0xE226   ) //reph
                    )
            {
                str->at(i + 0) = str->at(i+1);
                str->at(i + 1) = str->at(i+2);
                str->at(i + 2) = 0x09C7;
                i+=2;
            }
            else
            {
                //default
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = 0x09C7;
                i++;
            }
        }
    }
}

void SwitchBanglaAI_reverse(lString16* str)
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
        if(str->at(i) == 0x09C8)
        {
            str->at(i) = str->at(i + 1);
            str->at(i + 1) = 0x09C8;
        }
    }
}

void SwitchBanglaO_reverse(lString16* str)
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
        if(str->at(i) == 0x09C7)
        {
            if ( i < str->length()-3    &&
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
            if (str->at(i + 2) == 0x09BE)
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x09CB;
                str->erase(i + 2);
            }
        }
    }
}

void SwitchBanglaOU_reverse(lString16* str)
{
/*
 *  (e ? ' ou) -> (? ' OU)  // ' == 0xE226 //ra virama
 *  (e ? J ou) -> (? J OU)  // J == 0xE271 //ya postform
 *  (e ? j ou) -> (? j OU)  // j == 0xE272 //ya postform
 *  (e ? i ou) -> (? i OU)  // i == 0xE273 //ya postform
 *  (e ? ou)   -> (? OU)
*/
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x09C7)
        {
            if ( i < str->length()-3    &&
                 str->at(i+3) == 0x09D7 && (str->at(i + 2) == 0xE226 ||
                                            str->at(i + 2) == 0xE271 ||
                                            str->at(i + 2) == 0xE272 ||
                                            str->at(i + 2) == 0xE273 ))
            {
                str->at(i + 0) = str->at(i + 1);
                str->at(i + 1) = str->at(i + 2);
                str->at(i + 2) = 0x09CC;
                str->erase(i + 3);
                continue;
            }
            if (str->at(i + 2) == 0x09D7)
            {
                str->at(i) = str->at(i + 1);
                str->at(i + 1) = 0x09CC;
                str->erase(i + 2);
            }
        }
    }
}

void SwitchBanglaRaITa_reverse(lString16* str)
{
    // 0x09BF 0x09A4 0xE226 -> 0x09B0 0x09BF 0x09A4
    if (str->length() < 3)
    {
        return;
    }
    for (int i = 0; i < str->length()-2; i++)
    {
        if (str->at(i + 0) == 0x09BF && str->at(i + 1) == 0x09A4 && str->at(i + 2) == 0xE226)
        {
            str->at(i + 0) = 0x09B0;
            str->at(i + 1) = 0x09BF;
            str->at(i + 2) = 0x09A4;
        }
    }
}

void SwitchBanglaReph_reverse(lString16* str)
{
/*
 *  (? e `) -> (` ? e) // e = 0x09C7
 *  (? i `) -> (` ? i) // i = 0x09bf
 *  (? `)   -> (` ?)
*/
    if (str->length() < 2)
    {
        return;
    }
    for (int i = str->length() - 1; i >=1 ; i--)
    {
        int ch = str->at(i);
        if (ch == 0xE266 || ch == 0xE24C || ch ==  0xE226 || ch ==  0xE225)
        {
            if( i >= 2 && (str->at(i - 1) == 0x09C7 || str->at(i - 1) == 0x09BF) )
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = str->at(i - 2);
                str->at(i - 2) = ch;
                i-=2;
            }
            else
            {
                str->at(i - 0) = str->at(i - 1);
                str->at(i - 1) = ch;
                i--;
            }
        }
    }
}

bool CharIsBangla(int ch)
{
    return (ch >= 0x0980 && ch <= 0x09FF);
}

bool lString16::CheckBangla()
{
    if(gDocumentBangla == 1)
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
        if (CharIsBangla(ch))
        {
            gDocumentBangla = 1;
            gDocumentINDIC = 1;

            if(gBanglaLigMapRev.empty())
            {
                gBanglaLigMapRev = BanglaLigMapReversed();
            }
            return true;
        }
    }
    return false;
}

lString16 processBanglaLigatures(lString16 word)
{
    // bangla lig max length = 7
    int j = (word.length() >= 7 ) ? 7 : word.length();
    for(;  j >= 2; j--)
    {
        for (int c = word.length() - j; c >= 0; c--)
        {
            lUInt32 fastkey = (word.at(c) << 16) + word.at(c + 1);
            if (gBanglaFastLigMap.find(fastkey) == gBanglaFastLigMap.end())
            {
                continue;
            }
            dvngLig lig(word.substr(c, j));
            lChar16 rep = findBanglaLigRev(lig);

            if(isViramaComboBangla(rep))
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
            if (rep != 0)
            {
                if(lig.banglaRa && c+j < word.length() && isBangla_consonant(word.at(c+j)) )
                {
                    //LE("rep = 0x%04X, char+1 = 0x%04X",rep,word.at(c+j));
                    continue;
                }
                //LE("found!!, char = 0x%X",rep);
                word.replace(c, j, lString16(&rep, 1));
                c -= j - 2;
            }
        }
    }
    return word;
}

void StripZWNJ(lString16 *word)
{
    for (int i = 0; i < word->size(); ++i)
    {
        //if (word->at(i) == 0x200C) word->erase(i);
        if (word->at(i) == 0x200C) word->at(i) = 0x200B;
    }
}

void StripZWNJ_reverse(lString16 *word)
{
    for (int i = 0; i < word->size(); ++i)
    {
        if (word->at(i) == 0x200B) word->at(i) = 0x200C;
    }
}

lString16 lString16::processBanglaText()
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
        word = processBanglaLigatures(word);

        SwitchBanglaReph(&word);
        SwitchBanglaE(&word);
        SwitchBanglaI(&word);
        SwitchBanglaAI(&word);
        SwitchBanglaO(&word);
        SwitchBanglaOU(&word);
        SwitchBanglaRaITa(&word);

        StripZWNJ(&word);

        res.append(word);
        res.append(L" ");
    }
    res.substr(0,res.size()-1);
    return res;
}

lString16 restoreBanglaWord(lString16 in)
{
    if(BANGLA_DISPLAY_ENABLE == 0 || gDocumentBangla == 0)
    {
        return in;
    }
    StripZWNJ_reverse(&in);
    SwitchBanglaRaITa_reverse(&in);
    SwitchBanglaOU_reverse(&in);
    SwitchBanglaO_reverse(&in);
    SwitchBanglaAI_reverse(&in);
    SwitchBanglaE_reverse(&in);
    SwitchBanglaI_reverse(&in);
    SwitchBanglaReph_reverse(&in);

    for (int i = 0; i < in.length(); i++)
    {
        if (in[i] < BANGLA_START || in[i] > BANGLA_END)
        {
            continue;
        }

        dvngLig lig = findBanglaLig(in[i]);
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
