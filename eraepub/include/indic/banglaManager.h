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
 //
 // Created by Tarasus on 27.08.2020.
 //

#ifndef _BANGLA_MANAGER_H_
#define _BANGLA_MANAGER_H_

#include <map>
#include <string>
#include <vector>
#include "../lvtinydom.h"
#include "../dvngLig.h"

extern LigMap     gBanglaLigMap;
extern LigMapRev  gBanglaLigMapRev;
extern FastLigMap gBanglaFastLigMap;

bool CharIsBangla(int ch);

LigMap GetBanglaLigMap();

LigMapRev BanglaLigMapReversed();

lChar16 findBanglaLigRev(dvngLig combo);

dvngLig findBanglaLig(lChar16 ligature);

lChar16 findBanglaLigGlyphIndex(lChar16 ligature);

lString16 restoreBanglaWord(lString16 in);


#endif //_BANGLA_MANAGER_H_
