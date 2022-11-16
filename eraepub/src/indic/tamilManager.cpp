//
// Created by Tarasus on 16.10.2020.
//

#include "include/indic/tamilManager.h"
#include "ore_log.h"
#include <vector>

LigMap     gTamilLigMap;
LigMapRev  gTamilLigMapRev;
FastLigMap gTamilFastLigMap;

bool CharIsTamil(int ch)
{
    return ((ch >= 0x0B80 && ch <= 0x0BFF));
}

bool lString16::CheckTamil()
{
    if(gDocumentTamil == 1)
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
        if (CharIsTamil(ch))
        {
            gDocumentTamil = 1;
            gDocumentINDIC = 1;

            if(gTamilLigMapRev.empty())
            {
                gTamilLigMapRev = TamilLigMapReversed();
            }
            return true;
        }
    }
    return false;
}

dvngLig findTamilLig(lChar16 ligature)
{
    if(gTamilLigMap.empty())
    {
        gTamilLigMap = GetTamilLigMap();
    }
    auto it = gTamilLigMap.find(ligature);
    if(it==gTamilLigMap.end())
    {
        return dvngLig();
    }
    return it->second;
}

lChar16 findTamilLigGlyphIndex(lChar16 ligature)
{
    auto it = gTamilLigMap.find(ligature);
    if(it==gTamilLigMap.end())
    {
        return 0;
    }
    return it->second.glyphindex;
}

lChar16 findTamilLigRev(dvngLig combo)
{
    if(gTamilLigMapRev.empty())
    {
        gTamilLigMapRev = TamilLigMapReversed();
    }
    if(combo.len < 2 || combo.len > 10 )
    {
        return 0;
    }
    auto it = gTamilLigMapRev.find(combo);
    if(it==gTamilLigMapRev.end())
    {
        return 0;
    }
    //LE("findTamilLigRev return %d", it->second);
    return it->second;
}

std::map <dvngLig, lChar16, Comparator> TamilLigMapReversed()
{
    if(!gTamilLigMapRev.empty())
    {
        return gTamilLigMapRev;
    }
    if(gTamilLigMap.empty())
    {
        gTamilLigMap = GetTamilLigMap();
    }
    gTamilLigMapRev = makeReverseLigMap(gTamilLigMap,&gTamilFastLigMap);
    return gTamilLigMapRev;
}

LigMap GetTamilLigMap()
{
    if(!gTamilLigMap.empty())
    {
        return gTamilLigMap;
    }
    //Tamil_START
    {
        //from Noto Sans Tamil.ttf
        gTamilLigMap.insert(std::make_pair( 0xE730, dvngLig( 76 ,"0x0B95 0x0BCD 0x0BB7"))); // 0x10001
        gTamilLigMap.insert(std::make_pair( 0xE731, dvngLig( 77 ,"0x0B95 0x0BCD"))); // 0x10002
        gTamilLigMap.insert(std::make_pair( 0xE732, dvngLig( 78 ,"0x0B99 0x0BCD"))); // 0x10003
        gTamilLigMap.insert(std::make_pair( 0xE733, dvngLig( 79 ,"0x0B9A 0x0BCD"))); // 0x10004
        gTamilLigMap.insert(std::make_pair( 0xE734, dvngLig( 80 ,"0x0B9C 0x0BCD"))); // 0x10005
        gTamilLigMap.insert(std::make_pair( 0xE735, dvngLig( 81 ,"0x0B9E 0x0BCD"))); // 0x10006
        gTamilLigMap.insert(std::make_pair( 0xE736, dvngLig( 82 ,"0x0B9F 0x0BCD"))); // 0x10007
        gTamilLigMap.insert(std::make_pair( 0xE737, dvngLig( 83 ,"0x0BA3 0x0BCD"))); // 0x10008
        gTamilLigMap.insert(std::make_pair( 0xE738, dvngLig( 84 ,"0x0BA4 0x0BCD"))); // 0x10009
        gTamilLigMap.insert(std::make_pair( 0xE739, dvngLig( 85 ,"0x0BA8 0x0BCD"))); // 0x1000A
        gTamilLigMap.insert(std::make_pair( 0xE73A, dvngLig( 86 ,"0x0BA9 0x0BCD"))); // 0x1000B
        gTamilLigMap.insert(std::make_pair( 0xE73B, dvngLig( 87 ,"0x0BAA 0x0BCD"))); // 0x1000C
        gTamilLigMap.insert(std::make_pair( 0xE73C, dvngLig( 88 ,"0x0BAE 0x0BCD"))); // 0x1000D
        gTamilLigMap.insert(std::make_pair( 0xE73D, dvngLig( 89 ,"0x0BAF 0x0BCD"))); // 0x1000E
        gTamilLigMap.insert(std::make_pair( 0xE73E, dvngLig( 90 ,"0x0BB0 0x0BCD"))); // 0x1000F
        gTamilLigMap.insert(std::make_pair( 0xE73F, dvngLig( 91 ,"0x0BB1 0x0BCD"))); // 0x10010
        gTamilLigMap.insert(std::make_pair( 0xE740, dvngLig( 92 ,"0x0BB2 0x0BCD"))); // 0x10011
        gTamilLigMap.insert(std::make_pair( 0xE741, dvngLig( 93 ,"0x0BB3 0x0BCD"))); // 0x10012
        gTamilLigMap.insert(std::make_pair( 0xE742, dvngLig( 94 ,"0x0BB4 0x0BCD"))); // 0x10013
        gTamilLigMap.insert(std::make_pair( 0xE743, dvngLig( 95 ,"0x0BB5 0x0BCD"))); // 0x10014
        gTamilLigMap.insert(std::make_pair( 0xE744, dvngLig( 96 ,"0x0BB6 0x0BCD"))); // 0x10015
        gTamilLigMap.insert(std::make_pair( 0xE745, dvngLig( 97 ,"0x0BB7 0x0BCD"))); // 0x10016
        gTamilLigMap.insert(std::make_pair( 0xE746, dvngLig( 98 ,"0x0BB8 0x0BCD"))); // 0x10017
        gTamilLigMap.insert(std::make_pair( 0xE747, dvngLig( 99 ,"0x0BB9 0x0BCD"))); // 0x10018
        gTamilLigMap.insert(std::make_pair( 0xE748, dvngLig( 100 ,"0x0B95 0x0BCD 0x0BB7 0x0BCD"))); // 0x10019
        gTamilLigMap.insert(std::make_pair( 0xE749, dvngLig( 101 ,"0x0B95 0x0BC1"))); // 0x1001A
        gTamilLigMap.insert(std::make_pair( 0xE74A, dvngLig( 102 ,"0x0B95 0x0BC2"))); // 0x1001B
        gTamilLigMap.insert(std::make_pair( 0xE74B, dvngLig( 103 ,"0x0B99 0x0BC0"))); // 0x1001C
        gTamilLigMap.insert(std::make_pair( 0xE74C, dvngLig( 104 ,"0x0B99 0x0BC1"))); // 0x1001D
        gTamilLigMap.insert(std::make_pair( 0xE74D, dvngLig( 105 ,"0x0B99 0x0BC2"))); // 0x1001E
        gTamilLigMap.insert(std::make_pair( 0xE74E, dvngLig( 106 ,"0x0B9A 0x0BC1"))); // 0x1001F
        gTamilLigMap.insert(std::make_pair( 0xE74F, dvngLig( 107 ,"0x0B9A 0x0BC2"))); // 0x10020
        gTamilLigMap.insert(std::make_pair( 0xE750, dvngLig( 108 ,"0x0B9E 0x0BC1"))); // 0x10021
        gTamilLigMap.insert(std::make_pair( 0xE751, dvngLig( 109 ,"0x0B9E 0x0BC2"))); // 0x10022
        gTamilLigMap.insert(std::make_pair( 0xE752, dvngLig( 110 ,"0x0B9F 0x0BBF"))); // 0x10023
        gTamilLigMap.insert(std::make_pair( 0xE753, dvngLig( 111 ,"0x0B9F 0x0BC0"))); // 0x10024
        gTamilLigMap.insert(std::make_pair( 0xE754, dvngLig( 112 ,"0x0B9F 0x0BC1"))); // 0x10025
        gTamilLigMap.insert(std::make_pair( 0xE755, dvngLig( 113 ,"0x0B9F 0x0BC2"))); // 0x10026
        gTamilLigMap.insert(std::make_pair( 0xE756, dvngLig( 114 ,"0x0BA3 0x0BC1"))); // 0x10027
        gTamilLigMap.insert(std::make_pair( 0xE757, dvngLig( 115 ,"0x0BA3 0x0BC2"))); // 0x10028
        gTamilLigMap.insert(std::make_pair( 0xE758, dvngLig( 116 ,"0x0BA4 0x0BC1"))); // 0x10029
        gTamilLigMap.insert(std::make_pair( 0xE759, dvngLig( 117 ,"0x0BA4 0x0BC2"))); // 0x1002A
        gTamilLigMap.insert(std::make_pair( 0xE75A, dvngLig( 118 ,"0x0BA8 0x0BC1"))); // 0x1002B
        gTamilLigMap.insert(std::make_pair( 0xE75B, dvngLig( 119 ,"0x0BA8 0x0BC2"))); // 0x1002C
        gTamilLigMap.insert(std::make_pair( 0xE75C, dvngLig( 120 ,"0x0BA9 0x0BC1"))); // 0x1002D
        gTamilLigMap.insert(std::make_pair( 0xE75D, dvngLig( 121 ,"0x0BA9 0x0BC2"))); // 0x1002E
        gTamilLigMap.insert(std::make_pair( 0xE75E, dvngLig( 122 ,"0x0BAA 0x0BC0"))); // 0x1002F
        gTamilLigMap.insert(std::make_pair( 0xE75F, dvngLig( 123 ,"0x0BAA 0x0BC1"))); // 0x10030
        gTamilLigMap.insert(std::make_pair( 0xE760, dvngLig( 124 ,"0x0BAA 0x0BC2"))); // 0x10031
        gTamilLigMap.insert(std::make_pair( 0xE761, dvngLig( 125 ,"0x0BAE 0x0BC1"))); // 0x10032
        gTamilLigMap.insert(std::make_pair( 0xE762, dvngLig( 126 ,"0x0BAE 0x0BC2"))); // 0x10033
        gTamilLigMap.insert(std::make_pair( 0xE763, dvngLig( 127 ,"0x0BAF 0x0BC0"))); // 0x10034
        gTamilLigMap.insert(std::make_pair( 0xE764, dvngLig( 128 ,"0x0BAF 0x0BC1"))); // 0x10035
        gTamilLigMap.insert(std::make_pair( 0xE765, dvngLig( 129 ,"0x0BAF 0x0BC2"))); // 0x10036
        gTamilLigMap.insert(std::make_pair( 0xE766, dvngLig( 130 ,"0x0BB0 0x0BC1"))); // 0x10037
        gTamilLigMap.insert(std::make_pair( 0xE767, dvngLig( 131 ,"0x0BB0 0x0BC2"))); // 0x10038
        gTamilLigMap.insert(std::make_pair( 0xE768, dvngLig( 132 ,"0x0BB1 0x0BC1"))); // 0x10039
        gTamilLigMap.insert(std::make_pair( 0xE769, dvngLig( 133 ,"0x0BB1 0x0BC2"))); // 0x1003A
        gTamilLigMap.insert(std::make_pair( 0xE76A, dvngLig( 134 ,"0x0BB2 0x0BBF"))); // 0x1003B
        gTamilLigMap.insert(std::make_pair( 0xE76B, dvngLig( 135 ,"0x0BB2 0x0BC0"))); // 0x1003C
        gTamilLigMap.insert(std::make_pair( 0xE76C, dvngLig( 136 ,"0x0BB2 0x0BC1"))); // 0x1003D
        gTamilLigMap.insert(std::make_pair( 0xE76D, dvngLig( 137 ,"0x0BB2 0x0BC2"))); // 0x1003E
        gTamilLigMap.insert(std::make_pair( 0xE76E, dvngLig( 138 ,"0x0BB3 0x0BC1"))); // 0x1003F
        gTamilLigMap.insert(std::make_pair( 0xE76F, dvngLig( 139 ,"0x0BB3 0x0BC2"))); // 0x10040
        gTamilLigMap.insert(std::make_pair( 0xE770, dvngLig( 140 ,"0x0BB4 0x0BC1"))); // 0x10041
        gTamilLigMap.insert(std::make_pair( 0xE771, dvngLig( 141 ,"0x0BB4 0x0BC2"))); // 0x10042
        gTamilLigMap.insert(std::make_pair( 0xE772, dvngLig( 142 ,"0x0BB5 0x0BC0"))); // 0x10043
        gTamilLigMap.insert(std::make_pair( 0xE773, dvngLig( 143 ,"0x0BB5 0x0BC1"))); // 0x10044
        gTamilLigMap.insert(std::make_pair( 0xE774, dvngLig( 144 ,"0x0BB5 0x0BC2"))); // 0x10045
        gTamilLigMap.insert(std::make_pair( 0xE775, dvngLig( 145 ,"0x0BB8 0x0BBF"))); // 0x10046
        gTamilLigMap.insert(std::make_pair( 0xE776, dvngLig( 146 ,"0x0BB8 0x0BC0"))); // 0x10047
        gTamilLigMap.insert(std::make_pair( 0xE777, dvngLig( 147 ,"0x0BB8 0x0BCD 0x0BB0 0x0BC0"))); // 0x10048
        gTamilLigMap.insert(std::make_pair( 0xE778, dvngLig( 147 ,"0x0BB6 0x0BCD 0x0BB0 0x0BC0"))); // 0x10048
    }
    return gTamilLigMap;
}

void SwitchTamilE(lString16* str)
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

void SwitchTamilEE(lString16 *str)
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

void SwitchTamilAI(lString16* str)
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

void SwitchTamilE_reverse(lString16* str)
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

void SwitchTamilEE_reverse(lString16* str)
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

void SwitchTamilAI_reverse(lString16* str)
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

void SwitchTamilO(lString16 *str)
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

void SwitchTamilOO(lString16 *str)
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

void SwitchTamilAU(lString16 *str)
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

void SwitchTamilO_reverse(lString16* str)
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

void SwitchTamilOO_reverse(lString16* str)
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

void SwitchTamilAU_reverse(lString16* str)
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

lString16 processTamilLigatures(lString16 word)
{
    int j = (word.length() >= 7 ) ? 7 : word.length();
    for(;  j >= 2; j--)
    {
        for (int c = word.length() - j; c >= 0; c--)
        {
            lUInt32 fastkey = (word.at(c) << 16) + word.at(c + 1);
            if (gTamilFastLigMap.find(fastkey) == gTamilFastLigMap.end())
            {
                continue;
            }
            dvngLig lig(word.substr(c, j));
            lChar16 rep = findTamilLigRev(lig);
/*
            if(isViramaComboTamil(rep))
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

lString16 lString16::processTamilText()
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
        word = processTamilLigatures(word);

        //SwitchBanglaReph(&word);
        SwitchTamilE(&word);
        SwitchTamilEE(&word);
        SwitchTamilAI(&word);
        SwitchTamilO(&word);
        SwitchTamilOO(&word);
        SwitchTamilAU(&word);
        //SwitchBanglaRaITa(&word);
//
        //StripZWNJ(&word);

        res.append(word);
        res.append(L" ");
    }
    res.substr(0,res.size()-1);
    return res;
}

lString16 restoreTamilWord(lString16 in)
{
    if(TAMIL_DISPLAY_ENABLE == 0 || gDocumentTamil == 0)
    {
        return in;
    }
    //lString16::StripZWNJ_reverse(&in);
    //lString16::SwitchBanglaRaITa_reverse(&in);
    //lString16::SwitchBanglaOU_reverse(&in);
    SwitchTamilAU_reverse(&in);
    SwitchTamilOO_reverse(&in);
    SwitchTamilO_reverse(&in);
    SwitchTamilAI_reverse(&in);
    SwitchTamilEE_reverse(&in);
    SwitchTamilE_reverse(&in);
    //lString16::SwitchBanglaReph_reverse(&in);

    for (int i = 0; i < in.length(); i++)
    {
        if (in[i] < TAMIL_START || in[i] > TAMIL_END)
        {
            continue;
        }

        dvngLig lig = findTamilLig(in[i]);
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
