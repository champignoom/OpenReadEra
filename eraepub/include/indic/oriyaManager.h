//
// Created by Tarasus on 03.11.2020.
//

#ifndef _ORIYA_MANAGER_H
#define _ORIYA_MANAGER_H

#include <map>
#include <string>
#include <vector>
#include "../lvtinydom.h"
#include "../dvngLig.h"

extern LigMap     gOriyaLigMap;
extern LigMapRev  gOriyaLigMapRev;
extern FastLigMap gOriyaFastLigMap;

LigMap GetOriyaLigMap();

LigMapRev OriyaLigMapReversed();

lChar16 findOriyaLigRev(dvngLig combo);

dvngLig findOriyaLig(lChar16 ligature);

lChar16 findOriyaLigGlyphIndex(lChar16 ligature);

lString16 restoreOriyaWord(lString16 in);

bool CharIsOriya(int ch);

#endif //_ORIYA_MANAGER_H
