//
// Created by Tarasus on 16.10.2020.
//

#ifndef _KANNADAMANAGER_H
#define _KANNADAMANAGER_H

#include <map>
#include <string>
#include <vector>
#include "../lvtinydom.h"
#include "../dvngLig.h"

extern LigMap     gKannadaLigMap;
extern LigMapRev  gKannadaLigMapRev;
extern FastLigMap gKannadaFastLigMap;

bool CharIsKannada(int ch);

LigMap GetKannadaLigMap();

LigMapRev KannadaLigMapReversed();

lChar16 findKannadaLigRev(dvngLig combo);

dvngLig findKannadaLig(lChar16 ligature);

lChar16 findKannadaLigGlyphIndex(lChar16 ligature);

lString16 restoreKannadaWord(lString16 in);



#endif //_KANNADAMANAGER_H
