//
// Created by Tarasus on 03.11.2020.
//

#ifndef _TELUGU_MANAGER_H
#define _TELUGU_MANAGER_H

#include <map>
#include <string>
#include <vector>
#include "../lvtinydom.h"
#include "../dvngLig.h"

extern LigMap     gTeluguLigMap;
extern LigMapRev  gTeluguLigMapRev;
extern FastLigMap gTeluguFastLigMap;

LigMap GetTeluguLigMap();

LigMapRev TeluguLigMapReversed();

lChar16 findTeluguLigRev(dvngLig combo);

dvngLig findTeluguLig(lChar16 ligature);

lChar16 findTeluguLigGlyphIndex(lChar16 ligature);

lString16 restoreTeluguWord(lString16 in);

bool CharIsTelugu(int ch);

#endif //_TELUGU_MANAGER_H
