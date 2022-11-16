//
// Created by Tarasus on 03.11.2020.
//

#ifndef _GUJARATI_MANAGER_H
#define _GUJARATI_MANAGER_H

#include <map>
#include <string>
#include <vector>
#include "../lvtinydom.h"
#include "../dvngLig.h"

extern LigMap     gGujaratiLigMap;
extern LigMapRev  gGujaratiLigMapRev;
extern FastLigMap gGujaratiFastLigMap;

LigMap GetGujaratiLigMap();

LigMapRev GujaratiLigMapReversed();

lChar16 findGujaratiLigRev(dvngLig combo);

dvngLig findGujaratiLig(lChar16 ligature);

lChar16 findGujaratiLigGlyphIndex(lChar16 ligature);

lString16 restoreGujaratiWord(lString16 in);

bool CharIsGujarati(int ch);

#endif //_GUJARATI_MANAGER_H
