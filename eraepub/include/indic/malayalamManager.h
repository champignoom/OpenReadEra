//
// Created by Tarasus on 08.10.2020.
//

#ifndef _MALAYALAMMANAGER_H
#define _MALAYALAMMANAGER_H

#include <map>
#include <string>
#include <vector>
#include "../lvtinydom.h"
#include "../dvngLig.h"

extern LigMap     gMalayLigMap;
extern LigMapRev  gMalayLigMapRev;
extern FastLigMap gMalayFastLigMap;

bool CharIsMalay(int ch);

LigMap GetMalayLigMap();

LigMapRev MalayLigMapReversed();

lChar16 findMalayLigRev(dvngLig combo);

dvngLig findMalayLig(lChar16 ligature);

lChar16 findMalayLigGlyphIndex(lChar16 ligature);

lString16 restoreMalayWord(lString16 in);

#endif //_MALAYALAMMANAGER_H
