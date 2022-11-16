//
// Created by Tarasus on 19.10.2020.
//

#ifndef _TAMILMANAGER_H
#define _TAMILMANAGER_H

#include <map>
#include <string>
#include <vector>
#include "../lvtinydom.h"
#include "../dvngLig.h"

extern LigMap     gTamilLigMap;
extern LigMapRev  gTamilLigMapRev;
extern FastLigMap gTamilFastLigMap;

LigMap GetTamilLigMap();

LigMapRev TamilLigMapReversed();

lChar16 findTamilLigRev(dvngLig combo);

dvngLig findTamilLig(lChar16 ligature);

lChar16 findTamilLigGlyphIndex(lChar16 ligature);

lString16 restoreTamilWord(lString16 in);

bool CharIsTamil(int ch);

#endif //_TAMILMANAGER_H
