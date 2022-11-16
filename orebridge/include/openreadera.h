/*
 * Copyright (C) 2013-2020 READERA LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Developers: ReadEra Team (2013-2020), Playful Curiosity (2013-2020),
 * Tarasus (2018-2020).
 */

#ifndef _OPENREADERA_H_
#define _OPENREADERA_H_

#include <fcntl.h>
#include <string>
#include "StProtocol.h"
#include "ore_log.h"

bool OreBuildDebug();
std::string OreVersion(std::string base_version);
void OreStart(const char* name);
void OreVerResporse(const char* base_version, CmdResponse& response);

#define DOC_FORMAT_NULL 0
#define DOC_FORMAT_EPUB 1
#define DOC_FORMAT_FB2 2
#define DOC_FORMAT_PDF 3
#define DOC_FORMAT_MOBI 4
#define DOC_FORMAT_DJVU 5
#define DOC_FORMAT_DJV 6
#define DOC_FORMAT_DOC 7
#define DOC_FORMAT_RTF 8
#define DOC_FORMAT_TXT 9
#define DOC_FORMAT_XPS 10
#define DOC_FORMAT_OXPS 11
#define DOC_FORMAT_CHM 12
#define DOC_FORMAT_HTML 13
#define DOC_FORMAT_DOCX 14
#define DOC_FORMAT_ODT 15
#define DOC_FORMAT_AZW 16
#define DOC_FORMAT_AZW3 17
#define DOC_FORMAT_FB3 18
#define DOC_FORMAT_CBR 19
#define DOC_FORMAT_CBZ 20

#define CONFIG_CRE_FOOTNOTES 100
#define CONFIG_CRE_EMBEDDED_STYLES 101
#define CONFIG_CRE_EMBEDDED_FONTS 102
#define CONFIG_CRE_FONT_FACE_MAIN 103
#define CONFIG_CRE_FONT_FACE_FALLBACK 104
#define CONFIG_CRE_FONT_COLOR 105
#define CONFIG_CRE_FONT_SIZE 106
/**
 * Supported: 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.5, 1.9
 * See gammatbl.h
 */
#define CONFIG_CRE_FONT_GAMMA 107
/**
 * 0 - no antialias, 1 - antialias big fonts, 2 antialias all
 */
#define CONFIG_CRE_FONT_ANTIALIASING 108
/**
 * Supported: 80, 85, 90, 95, 100, 105, 110, 115, 120, 130, 140, 150, 160, 180, 200
 */
#define CONFIG_CRE_INTERLINE 110
#define CONFIG_CRE_BACKGROUND_COLOR 111
/**
 * 1 for one-column mode, 2 for two-column mode in landscape orientation
 */
#define CONFIG_CRE_PAGES_COLUMNS 112
#define CONFIG_CRE_MARGIN_TOP 113
#define CONFIG_CRE_MARGIN_BOTTOM 114
#define CONFIG_CRE_MARGIN_LEFT 115
#define CONFIG_CRE_MARGIN_RIGHT 116
#define CONFIG_CRE_PAGE_WIDTH 117
#define CONFIG_CRE_PAGE_HEIGHT 118
#define CONFIG_CRE_TEXT_ALIGN 119
#define CONFIG_CRE_HYPHENATION 120
#define CONFIG_CRE_FLOATING_PUNCTUATION 121
#define CONFIG_CRE_FIRSTPAGE_THUMB 122
#define CONFIG_CRE_FONT_CRE 123
#define CONFIG_MUPDF_FONT_PDF_SANS_R 124
#define CONFIG_MUPDF_FONT_PDF_SANS_I 125
#define CONFIG_MUPDF_FONT_PDF_SANS_B 126
#define CONFIG_MUPDF_FONT_PDF_SANS_BI 127
#define CONFIG_MUPDF_FONT_PDF_SERIF_R 128
#define CONFIG_MUPDF_FONT_PDF_SERIF_I 129
#define CONFIG_MUPDF_FONT_PDF_SERIF_B 130
#define CONFIG_MUPDF_FONT_PDF_SERIF_BI 131
#define CONFIG_MUPDF_FONT_PDF_MONO_R 132
#define CONFIG_MUPDF_FONT_PDF_MONO_I 133
#define CONFIG_MUPDF_FONT_PDF_MONO_B 134
#define CONFIG_MUPDF_FONT_PDF_MONO_BI 135
#define CONFIG_MUPDF_FONT_PDF_SYMBOL_R 136
#define CONFIG_MUPDF_FONT_PDF_DINGBAT_R 137
#define CONFIG_MUPDF_INVERT_IMAGES  200
#define CONFIG_ERA_EMBEDDED_STYLES  201
#define CONFIG_MUPDF_TWILIGHT_MODE  202
#define CONFIG_ERA_VERTICAL_MODE    203

#define CONFIG_ERA_TEXT_INDENT            204
#define CONFIG_ERA_PARAGRAPH_MARGIN       205
#define CONFIG_ERA_INDENT_MARGIN_OVERRIDE 206

#define HARDCONFIG_DJVU_RENDERING_MODE 0
#define HARDCONFIG_MUPDF_SLOW_CMYK 1 //if not ARM architecture it would convert cmyk slow but quality
#define TEXT_SEARCH_PREVIEW_WORD_NUM 7

// 32-bit unsigned integer max value.
#define DIRECT_ARCHIVE_SMART 4294967295

bool OreIsSmartDirectArchive(uint32_t direct_archive);
bool OreIsNormalDirectArchive(uint32_t direct_archive);

#endif /* _OPENREADERA_H_ */
