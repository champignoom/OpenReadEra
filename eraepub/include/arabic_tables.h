/*
 * Copyright (C) 2013-2020 READERA LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Developers: ReadEra Team (2013-2020), Tarasus (2018-2020).
 */

#ifndef _OPENREADERA_ARABIC_TABLES_H_
#define _OPENREADERA_ARABIC_TABLES_H_

#include "lvtinydom.h"
#include <array>
typedef std::array<lChar16,4> CharArr;
typedef std::map<lChar16 , CharArr> LetterMap;
typedef std::map<lChar16 , lChar16 > ReverseLetterMap;
enum charType {arabic_end = 0, arabic_mid = 1,arabic_start = 2, arabic_isolated = 3};


ReverseLetterMap ArabicReverseLetterMap()
{
    ReverseLetterMap reverseLetterMap;

    reverseLetterMap.insert(std::make_pair(0xFE8E,0x0627));
    reverseLetterMap.insert(std::make_pair(0xFE8D,0x0627));
    reverseLetterMap.insert(std::make_pair(0xFE90,0x0628));
    reverseLetterMap.insert(std::make_pair(0xFE92,0x0628));
    reverseLetterMap.insert(std::make_pair(0xFE91,0x0628));
    reverseLetterMap.insert(std::make_pair(0xFE8F,0x0628));
    reverseLetterMap.insert(std::make_pair(0xFE96,0x062A));
    reverseLetterMap.insert(std::make_pair(0xFE98,0x062A));
    reverseLetterMap.insert(std::make_pair(0xFE97,0x062A));
    reverseLetterMap.insert(std::make_pair(0xFE95,0x062A));
    reverseLetterMap.insert(std::make_pair(0xFE9A,0x062B));
    reverseLetterMap.insert(std::make_pair(0xFE9C,0x062B));
    reverseLetterMap.insert(std::make_pair(0xFE9B,0x062B));
    reverseLetterMap.insert(std::make_pair(0xFE99,0x062B));
    reverseLetterMap.insert(std::make_pair(0xFE9E,0x062C));
    reverseLetterMap.insert(std::make_pair(0xFEA0,0x062C));
    reverseLetterMap.insert(std::make_pair(0xFE9F,0x062C));
    reverseLetterMap.insert(std::make_pair(0xFE9D,0x062C));
    reverseLetterMap.insert(std::make_pair(0xFEA2,0x062D));
    reverseLetterMap.insert(std::make_pair(0xFEA4,0x062D));
    reverseLetterMap.insert(std::make_pair(0xFEA3,0x062D));
    reverseLetterMap.insert(std::make_pair(0xFEA1,0x062D));
    reverseLetterMap.insert(std::make_pair(0xFEA6,0x062E));
    reverseLetterMap.insert(std::make_pair(0xFEA8,0x062E));
    reverseLetterMap.insert(std::make_pair(0xFEA7,0x062E));
    reverseLetterMap.insert(std::make_pair(0xFEA5,0x062E));
    reverseLetterMap.insert(std::make_pair(0xFEAA,0x062F));
    reverseLetterMap.insert(std::make_pair(0xFEA9,0x062F));
    reverseLetterMap.insert(std::make_pair(0xFEAC,0x0630));
    reverseLetterMap.insert(std::make_pair(0xFEAB,0x0630));
    reverseLetterMap.insert(std::make_pair(0xFEAE,0x0631));
    reverseLetterMap.insert(std::make_pair(0xFEAD,0x0631));
    reverseLetterMap.insert(std::make_pair(0xFEB0,0x0632));
    reverseLetterMap.insert(std::make_pair(0xFEAF,0x0632));
    reverseLetterMap.insert(std::make_pair(0xFEB2,0x0633));
    reverseLetterMap.insert(std::make_pair(0xFEB4,0x0633));
    reverseLetterMap.insert(std::make_pair(0xFEB3,0x0633));
    reverseLetterMap.insert(std::make_pair(0xFEB1,0x0633));
    reverseLetterMap.insert(std::make_pair(0xFEB6,0x0634));
    reverseLetterMap.insert(std::make_pair(0xFEB8,0x0634));
    reverseLetterMap.insert(std::make_pair(0xFEB7,0x0634));
    reverseLetterMap.insert(std::make_pair(0xFEB5,0x0634));
    reverseLetterMap.insert(std::make_pair(0xFEBA,0x0635));
    reverseLetterMap.insert(std::make_pair(0xFEBC,0x0635));
    reverseLetterMap.insert(std::make_pair(0xFEBB,0x0635));
    reverseLetterMap.insert(std::make_pair(0xFEB9,0x0635));
    reverseLetterMap.insert(std::make_pair(0xFEBE,0x0636));
    reverseLetterMap.insert(std::make_pair(0xFEC0,0x0636));
    reverseLetterMap.insert(std::make_pair(0xFEBF,0x0636));
    reverseLetterMap.insert(std::make_pair(0xFEBD,0x0636));
    reverseLetterMap.insert(std::make_pair(0xFEC2,0x0637));
    reverseLetterMap.insert(std::make_pair(0xFEC4,0x0637));
    reverseLetterMap.insert(std::make_pair(0xFEC3,0x0637));
    reverseLetterMap.insert(std::make_pair(0xFEC1,0x0637));
    reverseLetterMap.insert(std::make_pair(0xFEC6,0x0638));
    reverseLetterMap.insert(std::make_pair(0xFEC8,0x0638));
    reverseLetterMap.insert(std::make_pair(0xFEC7,0x0638));
    reverseLetterMap.insert(std::make_pair(0xFEC5,0x0638));
    reverseLetterMap.insert(std::make_pair(0xFECA,0x0639));
    reverseLetterMap.insert(std::make_pair(0xFECC,0x0639));
    reverseLetterMap.insert(std::make_pair(0xFECB,0x0639));
    reverseLetterMap.insert(std::make_pair(0xFEC9,0x0639));
    reverseLetterMap.insert(std::make_pair(0xFECE,0x063A));
    reverseLetterMap.insert(std::make_pair(0xFED0,0x063A));
    reverseLetterMap.insert(std::make_pair(0xFECF,0x063A));
    reverseLetterMap.insert(std::make_pair(0xFECD,0x063A));
    reverseLetterMap.insert(std::make_pair(0xFED2,0x0641));
    reverseLetterMap.insert(std::make_pair(0xFED4,0x0641));
    reverseLetterMap.insert(std::make_pair(0xFED3,0x0641));
    reverseLetterMap.insert(std::make_pair(0xFED1,0x0641));
    reverseLetterMap.insert(std::make_pair(0xFED6,0x0642));
    reverseLetterMap.insert(std::make_pair(0xFED8,0x0642));
    reverseLetterMap.insert(std::make_pair(0xFED7,0x0642));
    reverseLetterMap.insert(std::make_pair(0xFED5,0x0642));
    reverseLetterMap.insert(std::make_pair(0xFEDA,0x0643));
    reverseLetterMap.insert(std::make_pair(0xFEDC,0x0643));
    reverseLetterMap.insert(std::make_pair(0xFEDB,0x0643));
    reverseLetterMap.insert(std::make_pair(0xFED9,0x0643));
    reverseLetterMap.insert(std::make_pair(0xFEDE,0x0644));
    reverseLetterMap.insert(std::make_pair(0xFEE0,0x0644));
    reverseLetterMap.insert(std::make_pair(0xFEDF,0x0644));
    reverseLetterMap.insert(std::make_pair(0xFEDD,0x0644));
    reverseLetterMap.insert(std::make_pair(0xFEE2,0x0645));
    reverseLetterMap.insert(std::make_pair(0xFEE4,0x0645));
    reverseLetterMap.insert(std::make_pair(0xFEE3,0x0645));
    reverseLetterMap.insert(std::make_pair(0xFEE1,0x0645));
    reverseLetterMap.insert(std::make_pair(0xFEE6,0x0646));
    reverseLetterMap.insert(std::make_pair(0xFEE8,0x0646));
    reverseLetterMap.insert(std::make_pair(0xFEE7,0x0646));
    reverseLetterMap.insert(std::make_pair(0xFEE5,0x0646));
    reverseLetterMap.insert(std::make_pair(0xFEEA,0x0647));
    reverseLetterMap.insert(std::make_pair(0xFEEC,0x0647));
    reverseLetterMap.insert(std::make_pair(0xFEEB,0x0647));
    reverseLetterMap.insert(std::make_pair(0xFEE9,0x0647));
    reverseLetterMap.insert(std::make_pair(0xFEEE,0x0648));
    reverseLetterMap.insert(std::make_pair(0xFEED,0x0648));
    reverseLetterMap.insert(std::make_pair(0xFEF2,0x064A));
    reverseLetterMap.insert(std::make_pair(0xFEF4,0x064A));
    reverseLetterMap.insert(std::make_pair(0xFEF3,0x064A));
    reverseLetterMap.insert(std::make_pair(0xFEF1,0x064A));
    reverseLetterMap.insert(std::make_pair(0xFE82,0x0622));
    reverseLetterMap.insert(std::make_pair(0xFE81,0x0622));
    reverseLetterMap.insert(std::make_pair(0xFE84,0x0623));
    reverseLetterMap.insert(std::make_pair(0xFE83,0x0623));
    reverseLetterMap.insert(std::make_pair(0xFE88,0x0625));
    reverseLetterMap.insert(std::make_pair(0xFE87,0x0625));
    reverseLetterMap.insert(std::make_pair(0xFE86,0x0624));
    reverseLetterMap.insert(std::make_pair(0xFE85,0x0624));
    reverseLetterMap.insert(std::make_pair(0xFE8A,0x0626));
    reverseLetterMap.insert(std::make_pair(0xFE8C,0x0626));
    reverseLetterMap.insert(std::make_pair(0xFE8B,0x0626));
    reverseLetterMap.insert(std::make_pair(0xFE89,0x0626));
    reverseLetterMap.insert(std::make_pair(0xFE94,0x0629));
    reverseLetterMap.insert(std::make_pair(0xFE93,0x0629));
    reverseLetterMap.insert(std::make_pair(0xFEF0,0x0649));
    reverseLetterMap.insert(std::make_pair(0xFEEF,0x0649));
    reverseLetterMap.insert(std::make_pair(0xFEFB,0x0627));     //lam-alef ligature
    reverseLetterMap.insert(std::make_pair(0xFEFC,0x0627));     //lam-alef ligature
    reverseLetterMap.insert(std::make_pair(0xFEF5,0x0622));     //lam-alef + madda above
    reverseLetterMap.insert(std::make_pair(0xFEF6,0x0622));     //lam-alef + madda above
    reverseLetterMap.insert(std::make_pair(0xFEF7,0x0623));     //lam-alef + hamza above
    reverseLetterMap.insert(std::make_pair(0xFEF8,0x0623));     //lam-alef + hamza above
    reverseLetterMap.insert(std::make_pair(0xFEF9,0x0625));     //lam-alef + hamza below
    reverseLetterMap.insert(std::make_pair(0xFEFA,0x0625));     //lam-alef + hamza below

    //urdu
    reverseLetterMap.insert(std::make_pair(0xFB57,0x067E));
    reverseLetterMap.insert(std::make_pair(0xFB59,0x067E));
    reverseLetterMap.insert(std::make_pair(0xFB58,0x067E));
    reverseLetterMap.insert(std::make_pair(0xFB56,0x067E));
    reverseLetterMap.insert(std::make_pair(0xFB67,0x0679));
    reverseLetterMap.insert(std::make_pair(0xFB69,0x0679));
    reverseLetterMap.insert(std::make_pair(0xFB68,0x0679));
    reverseLetterMap.insert(std::make_pair(0xFB66,0x0679));
    reverseLetterMap.insert(std::make_pair(0xFB7B,0x0686));
    reverseLetterMap.insert(std::make_pair(0xFB7D,0x0686));
    reverseLetterMap.insert(std::make_pair(0xFB7C,0x0686));
    reverseLetterMap.insert(std::make_pair(0xFB7A,0x0686));
    reverseLetterMap.insert(std::make_pair(0xFB89,0x0688));
    reverseLetterMap.insert(std::make_pair(0xFB89,0x0688));
    reverseLetterMap.insert(std::make_pair(0xFB88,0x0688));
    reverseLetterMap.insert(std::make_pair(0xFB88,0x0688));
    reverseLetterMap.insert(std::make_pair(0xFB8D,0x0691));
    reverseLetterMap.insert(std::make_pair(0xFB8D,0x0691));
    reverseLetterMap.insert(std::make_pair(0xFB8C,0x0691));
    reverseLetterMap.insert(std::make_pair(0xFB8C,0x0691));
    reverseLetterMap.insert(std::make_pair(0xFB8B,0x0698));
    reverseLetterMap.insert(std::make_pair(0xFB8B,0x0698));
    reverseLetterMap.insert(std::make_pair(0xFB8A,0x0698));
    reverseLetterMap.insert(std::make_pair(0xFB8A,0x0698));
    reverseLetterMap.insert(std::make_pair(0xFB8F,0x06A9));
    reverseLetterMap.insert(std::make_pair(0xFB91,0x06A9));
    reverseLetterMap.insert(std::make_pair(0xFB90,0x06A9));
    reverseLetterMap.insert(std::make_pair(0xFB8E,0x06A9));
    reverseLetterMap.insert(std::make_pair(0xFB93,0x06AF));
    reverseLetterMap.insert(std::make_pair(0xFB95,0x06AF));
    reverseLetterMap.insert(std::make_pair(0xFB94,0x06AF));
    reverseLetterMap.insert(std::make_pair(0xFB92,0x06AF));
    reverseLetterMap.insert(std::make_pair(0xFBA7,0x06C1));
    reverseLetterMap.insert(std::make_pair(0xFBA9,0x06C1));
    reverseLetterMap.insert(std::make_pair(0xFBA8,0x06C1));
    reverseLetterMap.insert(std::make_pair(0xFBA6,0x06C1));
    reverseLetterMap.insert(std::make_pair(0xFE80,0x0621));
    reverseLetterMap.insert(std::make_pair(0xFE80,0x0621));
    reverseLetterMap.insert(std::make_pair(0xFE80,0x0621));
    reverseLetterMap.insert(std::make_pair(0xFE80,0x0621));
    reverseLetterMap.insert(std::make_pair(0xFBFD,0x06CC));
    reverseLetterMap.insert(std::make_pair(0xFBFF,0x06CC));
    reverseLetterMap.insert(std::make_pair(0xFBFE,0x06CC));
    reverseLetterMap.insert(std::make_pair(0xFBFC,0x06CC));
    reverseLetterMap.insert(std::make_pair(0xFBAF,0x06D2));
    reverseLetterMap.insert(std::make_pair(0xFBFF,0x06D2));
    reverseLetterMap.insert(std::make_pair(0xFBFE,0x06D2));
    reverseLetterMap.insert(std::make_pair(0xFBAE,0x06D2));
    return reverseLetterMap;
}

LetterMap ArabicLetterMap()
{
    LVArray<lChar16> list;
    lString16 temp;
    LetterMap LetterMap;
            //key   //end    //mid   //start //isolated
    CharArr x0627 = {0xFE8E, 0xFE8E, 0xFE8D, 0xFE8D};
    CharArr x0628 = {0xFE90, 0xFE92, 0xFE91, 0xFE8F};
    CharArr x062A = {0xFE96, 0xFE98, 0xFE97, 0xFE95};
    CharArr x062B = {0xFE9A, 0xFE9C, 0xFE9B, 0xFE99};
    CharArr x062C = {0xFE9E, 0xFEA0, 0xFE9F, 0xFE9D};
    CharArr x062D = {0xFEA2, 0xFEA4, 0xFEA3, 0xFEA1};
    CharArr x062E = {0xFEA6, 0xFEA8, 0xFEA7, 0xFEA5};
    CharArr x062F = {0xFEAA, 0xFEAA, 0xFEA9, 0xFEA9};
    CharArr x0630 = {0xFEAC, 0xFEAC, 0xFEAB, 0xFEAB};
    CharArr x0631 = {0xFEAE, 0xFEAE, 0xFEAD, 0xFEAD};
    CharArr x0632 = {0xFEB0, 0xFEB0, 0xFEAF, 0xFEAF};
    CharArr x0633 = {0xFEB2, 0xFEB4, 0xFEB3, 0xFEB1};
    CharArr x0634 = {0xFEB6, 0xFEB8, 0xFEB7, 0xFEB5};
    CharArr x0635 = {0xFEBA, 0xFEBC, 0xFEBB, 0xFEB9};
    CharArr x0636 = {0xFEBE, 0xFEC0, 0xFEBF, 0xFEBD};
    CharArr x0637 = {0xFEC2, 0xFEC4, 0xFEC3, 0xFEC1};
    CharArr x0638 = {0xFEC6, 0xFEC8, 0xFEC7, 0xFEC5};
    CharArr x0639 = {0xFECA, 0xFECC, 0xFECB, 0xFEC9};
    CharArr x063A = {0xFECE, 0xFED0, 0xFECF, 0xFECD};
    CharArr x0641 = {0xFED2, 0xFED4, 0xFED3, 0xFED1};
    CharArr x0642 = {0xFED6, 0xFED8, 0xFED7, 0xFED5};
    CharArr x0643 = {0xFEDA, 0xFEDC, 0xFEDB, 0xFED9};
    CharArr x0644 = {0xFEDE, 0xFEE0, 0xFEDF, 0xFEDD};
    CharArr x0645 = {0xFEE2, 0xFEE4, 0xFEE3, 0xFEE1};
    CharArr x0646 = {0xFEE6, 0xFEE8, 0xFEE7, 0xFEE5};
    CharArr x0647 = {0xFEEA, 0xFEEC, 0xFEEB, 0xFEE9};
    CharArr x0648 = {0xFEEE, 0xFEEE, 0xFEED, 0xFEED};
    CharArr x064A = {0xFEF2, 0xFEF4, 0xFEF3, 0xFEF1};
    CharArr x0622 = {0xFE82, 0xFE82, 0xFE81, 0xFE81};
    CharArr xFEFB = {0xFEFC, 0xFEFC, 0xFEFB, 0xFEFB}; // LAM-ALEF LIGATURE //fefb comes from LigatureCheck() function
    CharArr xFEF5 = {0xFEF6, 0xFEF6, 0xFEF5, 0xFEF5}; // LAM-ALEF WITH MADDA ABOVE 1
    CharArr xFEF6 = {0xFEF6, 0xFEF6, 0xFEF5, 0xFEF5}; // LAM-ALEF WITH MADDA ABOVE 2
    CharArr xFEF7 = {0xFEF8, 0xFEF8, 0xFEF7, 0xFEF7}; // LAM-ALEF WITH HAMZA ABOVE 1
    CharArr xFEF8 = {0xFEF8, 0xFEF8, 0xFEF7, 0xFEF7}; // LAM-ALEF WITH HAMZA ABOVE 2
    CharArr xFEF9 = {0xFEFA, 0xFEFA, 0xFEF9, 0xFEF9}; // LAM-ALEF WITH HAMZA BELOW 1
    CharArr xFEFA = {0xFEFA, 0xFEFA, 0xFEF9, 0xFEF9}; // LAM-ALEF WITH HAMZA BELOW 2
    CharArr xFE81 = {0xFE82, 0xFE82, 0xFE81, 0xFE81}; // ALEF     WITH MADDA ABOVE 1
    CharArr xFE82 = {0xFE82, 0xFE82, 0xFE81, 0xFE81}; // ALEF     WITH MADDA ABOVE 2
    CharArr x0671 = {0xFB51, 0xFB51, 0xFB50, 0xFB50}; // ALEF     WITH WASLA ABOVE
    CharArr x0623 = {0xFE84, 0xFE84, 0xFE83, 0xFE83}; // ALEF     WITH HAMZA ABOVE
    CharArr x0625 = {0xFE88, 0xFE88, 0xFE87, 0xFE87}; // ALEF     WITH HAMZA BELOW
    CharArr x0624 = {0xFE86, 0xFE86, 0xFE85, 0xFE85}; // WAW      WITH HAMZA ABOVE
    CharArr x0626 = {0xFE8A, 0xFE8C, 0xFE8B, 0xFE89}; // YEH      WITH HAMZA ABOVE
    CharArr x0629 = {0xFE94, 0xFE94, 0xFE93, 0xFE93}; // TEH      MARBUTA
    CharArr x0649 = {0xFEF0, 0xFEF0, 0xFEEF, 0xFEEF}; // ALEF     MAKSURA

    //Urdu addition
    CharArr x0621 = {0xFE80, 0xFE80, 0xFE80, 0xFE80};
    CharArr x0679 = {0xFB67, 0xFB69, 0xFB68, 0xFB66};
    CharArr x067E = {0xFB57, 0xFB59, 0xFB58, 0xFB56};
    CharArr x0686 = {0xFB7B, 0xFB7D, 0xFB7C, 0xFB7A};
    CharArr x0688 = {0xFB89, 0xFB89, 0xFB88, 0xFB88};
    CharArr x0691 = {0xFB8D, 0xFB8D, 0xFB8C, 0xFB8C};
    CharArr x0698 = {0xFB8B, 0xFB8B, 0xFB8A, 0xFB8A};
    CharArr x06A9 = {0xFB8F, 0xFB91, 0xFB90, 0xFB8E};
    CharArr x06AF = {0xFB93, 0xFB95, 0xFB94, 0xFB92};
    CharArr x06C1 = {0xFBA7, 0xFBA9, 0xFBA8, 0xFBA6};
    CharArr x06CC = {0xFBFD, 0xFBFF, 0xFBFE, 0xFBFC};
    CharArr x06D2 = {0xFBAF, 0xFBFF, 0xFBFE, 0xFBAE};

    LetterMap.insert(std::make_pair(0x0627,x0627));
    LetterMap.insert(std::make_pair(0x0628,x0628));
    LetterMap.insert(std::make_pair(0x062A,x062A));
    LetterMap.insert(std::make_pair(0x062B,x062B));
    LetterMap.insert(std::make_pair(0x062C,x062C));
    LetterMap.insert(std::make_pair(0x062D,x062D));
    LetterMap.insert(std::make_pair(0x062E,x062E));
    LetterMap.insert(std::make_pair(0x062F,x062F));
    LetterMap.insert(std::make_pair(0x0630,x0630));
    LetterMap.insert(std::make_pair(0x0631,x0631));
    LetterMap.insert(std::make_pair(0x0632,x0632));
    LetterMap.insert(std::make_pair(0x0633,x0633));
    LetterMap.insert(std::make_pair(0x0634,x0634));
    LetterMap.insert(std::make_pair(0x0635,x0635));
    LetterMap.insert(std::make_pair(0x0636,x0636));
    LetterMap.insert(std::make_pair(0x0637,x0637));
    LetterMap.insert(std::make_pair(0x0638,x0638));
    LetterMap.insert(std::make_pair(0x0639,x0639));
    LetterMap.insert(std::make_pair(0x063A,x063A));
    LetterMap.insert(std::make_pair(0x0641,x0641));
    LetterMap.insert(std::make_pair(0x0642,x0642));
    LetterMap.insert(std::make_pair(0x0643,x0643));
    LetterMap.insert(std::make_pair(0x0644,x0644));
    LetterMap.insert(std::make_pair(0x0645,x0645));
    LetterMap.insert(std::make_pair(0x0646,x0646));
    LetterMap.insert(std::make_pair(0x0647,x0647));
    LetterMap.insert(std::make_pair(0x0648,x0648));
    LetterMap.insert(std::make_pair(0x064A,x064A));
    LetterMap.insert(std::make_pair(0x0622,x0622));
    LetterMap.insert(std::make_pair(0xFEFB,xFEFB));
    LetterMap.insert(std::make_pair(0xFEF5,xFEF5));
    LetterMap.insert(std::make_pair(0xFEF6,xFEF6));
    LetterMap.insert(std::make_pair(0xFEF7,xFEF7));
    LetterMap.insert(std::make_pair(0xFEF8,xFEF8));
    LetterMap.insert(std::make_pair(0xFEF9,xFEF9));
    LetterMap.insert(std::make_pair(0xFEFA,xFEFA));
    LetterMap.insert(std::make_pair(0xFE81,xFE81));
    LetterMap.insert(std::make_pair(0xFE82,xFE82));
    LetterMap.insert(std::make_pair(0x0671,x0671));
    LetterMap.insert(std::make_pair(0x0623,x0623));
    LetterMap.insert(std::make_pair(0x0625,x0625));
    LetterMap.insert(std::make_pair(0x0624,x0624));
    LetterMap.insert(std::make_pair(0x0626,x0626));
    LetterMap.insert(std::make_pair(0x0629,x0629));
    LetterMap.insert(std::make_pair(0x0649,x0649));

    //urdu
    LetterMap.insert(std::make_pair(0x067E,x067E));
    LetterMap.insert(std::make_pair(0x0679,x0679));
    LetterMap.insert(std::make_pair(0x0686,x0686));
    LetterMap.insert(std::make_pair(0x0688,x0688));
    LetterMap.insert(std::make_pair(0x0691,x0691));
    LetterMap.insert(std::make_pair(0x0698,x0698));
    LetterMap.insert(std::make_pair(0x06A9,x06A9));
    LetterMap.insert(std::make_pair(0x06AF,x06AF));
    LetterMap.insert(std::make_pair(0x06C1,x06C1));
    LetterMap.insert(std::make_pair(0x0621,x0621));
    LetterMap.insert(std::make_pair(0x06CC,x06CC));
    LetterMap.insert(std::make_pair(0x06D2,x06D2));

 return LetterMap;
}

// INCREMENT THIS NUMBER WHILE ADDING COMBOS!!!1
#define LAM_ALEF_COMBOS_LENGTH 10
lChar16 lamAlefCombos[LAM_ALEF_COMBOS_LENGTH][3] =
        {       //lam    //alef  //ligature
                {0x0644, 0x0627, 0xFEFB}, // regular lam + regular alef

                {0x0644, 0x0622, 0xFEF5}, // + madda above
                {0x0644, 0xFE81, 0xFEF5},
                {0x0644, 0xFE82, 0xFEF6},

                {0x0644, 0x0623 ,0xFEF7}, //+ hamza above
                {0x0644, 0xFE83, 0xFEF7},
                {0x0644, 0xFE84, 0xFEF8},

                {0x0644, 0x0625, 0xFEF9}, //+ hamza below
                {0x0644, 0xFE87, 0xFEF9},
                {0x0644, 0xFE88, 0xFEFA}
        };
#endif //_OPENREADERA_ARABIC_TABLES_H_