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

#ifndef _OPENREADERA_FB3FMT_H_
#define _OPENREADERA_FB3FMT_H_

#include "lvtinydom.h"

bool ImportFb3Document(LVStreamRef stream, CrDom *m_doc, bool firstpage_thumb);

void GetFb3Metadata(CrDom *dom, lString16 *res_title, lString16 *res_authors, lString16 *res_lang, lString16 *res_series, int *res_series_number, lString16 *res_genre, lString16 *res_annotation);

LVStreamRef GetFb3CoverImage(LVContainerRef container);

std::map<lUInt32,lString16> BuildFb3RelsMap(LVContainerRef container);

#endif //_OPENREADERA_FB3FMT_H_
