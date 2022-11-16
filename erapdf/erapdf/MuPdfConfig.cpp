/*
 * Copyright (C) 2013 The MuPDF CLI viewer interface Project
 * Copyright (C) 2013-2020 READERA LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstdlib>
#include <set>
#include "ore_log.h"
#include "StProtocol.h"
#include "EraPdfBridge.h"

static unsigned int parseColor(const char* s) {
    return (unsigned int) (atoi(s) & 0xFFFFFF);
}
#define FLOAT_COEFF 0.00390625f // == 1/256

void MuPdfBridge::processConfig(CmdRequest& request, CmdResponse& response)
{
    LD("processConfig");
    response.cmd = CMD_RES_SET_CONFIG;
    CmdDataIterator iter(request.first);
    while (iter.hasNext()) {
        uint32_t key;
        uint8_t* temp_val;
        iter.getInt(&key).getByteArray(&temp_val);
        if (!iter.isValid()) {
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        const char *val = reinterpret_cast<const char *>(temp_val);
        if (key == CONFIG_MUPDF_INVERT_IMAGES)
        {
            config_invert_images = atoi(val);
        }
        else if (key == CONFIG_MUPDF_TWILIGHT_MODE)
        {
            eraConfig.erapdf_twilight_mode = atoi(val);
        }
        else if (key == CONFIG_CRE_FONT_COLOR)
        {
            unsigned int c = parseColor(val);
            eraConfig.f_font_color[0] = ((c >> 16) & 0xFF) * FLOAT_COEFF ; //r
            eraConfig.f_font_color[1] = ((c >> 8 ) & 0xFF) * FLOAT_COEFF ; //g
            eraConfig.f_font_color[2] = ( c        & 0xFF) * FLOAT_COEFF ; //b
        }
        else if (key == CONFIG_CRE_BACKGROUND_COLOR)
        {
            unsigned int c = parseColor(val);
            eraConfig.f_bg_color[0] = ((c >> 16) & 0xFF) * FLOAT_COEFF ; //r
            eraConfig.f_bg_color[1] = ((c >> 8 ) & 0xFF) * FLOAT_COEFF ; //g
            eraConfig.f_bg_color[2] = ( c        & 0xFF) * FLOAT_COEFF ; //b
        }
        else
        {
            LE("processConfig unknown key: key=%d, val=%s", key, val);
        }
    }
    response.addInt(pageCount);
}

extern "C"
{
extern char ext_font_Courier[1024];
extern char ext_font_CourierBold[1024];
extern char ext_font_CourierOblique[1024];
extern char ext_font_CourierBoldOblique[1024];
extern char ext_font_Helvetica[1024];
extern char ext_font_HelveticaBold[1024];
extern char ext_font_HelveticaOblique[1024];
extern char ext_font_HelveticaBoldOblique[1024];
extern char ext_font_TimesRoman[1024];
extern char ext_font_TimesBold[1024];
extern char ext_font_TimesItalic[1024];
extern char ext_font_TimesBoldItalic[1024];
extern char ext_font_Symbol[1024];
extern char ext_font_ZapfDingbats[1024];

extern char ext_system_fonts[MAX_SYS_FONT_TABLE_SIZE][2][512];
extern unsigned int ext_system_fonts_idx[MAX_SYS_FONT_TABLE_SIZE];
extern int ext_system_fonts_count;
}

void MuPdfBridge::resetFonts()
{
    ext_font_Courier[0] = 0;
    ext_font_CourierBold[0] = 0;
    ext_font_CourierOblique[0] = 0;
    ext_font_CourierBoldOblique[0] = 0;
    ext_font_Helvetica[0] = 0;
    ext_font_HelveticaBold[0] = 0;
    ext_font_HelveticaOblique[0] = 0;
    ext_font_HelveticaBoldOblique[0] = 0;
    ext_font_TimesRoman[0] = 0;
    ext_font_TimesBold[0] = 0;
    ext_font_TimesItalic[0] = 0;
    ext_font_TimesBoldItalic[0] = 0;
    ext_font_Symbol[0] = 0;
    ext_font_ZapfDingbats[0] = 0;
    for (int i = 0; i < MAX_SYS_FONT_TABLE_SIZE; i++) {
        ext_system_fonts[i][0][0] = 0;
        ext_system_fonts[i][1][0] = 0;
        ext_system_fonts_idx[i] = 0;
    }
    ext_system_fonts_count = 0;
}

static void setFontFileName(char* ext_Font, const char* fontFileName)
{
    if (fontFileName && fontFileName[0]) {
        strcpy(ext_Font, fontFileName);
    } else {
        ext_Font[0] = 0;
    }
}

void MuPdfBridge::processFontsConfig(CmdRequest& request, CmdResponse& response)
{
    LD("processFontsConfig");
    response.cmd = CMD_RES_SET_FONT_CONFIG;
    CmdDataIterator iter(request.first);
    while (iter.hasNext()) {
        uint32_t key;
        uint8_t* temp_val;
        iter.getInt(&key).getByteArray(&temp_val);
        if (!iter.isValid()) {
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        const char* val = reinterpret_cast<const char*>(temp_val);
        if (key == CONFIG_MUPDF_FONT_PDF_SANS_R) {
            setFontFileName(ext_font_Helvetica, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_SANS_I) {
            setFontFileName(ext_font_HelveticaOblique, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_SANS_B) {
            setFontFileName(ext_font_HelveticaBold, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_SANS_BI) {
            setFontFileName(ext_font_HelveticaBoldOblique, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_SERIF_R) {
            setFontFileName(ext_font_TimesRoman, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_SERIF_I) {
            setFontFileName(ext_font_TimesItalic, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_SERIF_B) {
            setFontFileName(ext_font_TimesBold, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_SERIF_BI) {
            setFontFileName(ext_font_TimesBoldItalic, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_MONO_R) {
            setFontFileName(ext_font_Courier, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_MONO_I) {
            setFontFileName(ext_font_CourierOblique, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_MONO_B) {
            setFontFileName(ext_font_CourierBold, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_MONO_BI) {
            setFontFileName(ext_font_CourierBoldOblique, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_SYMBOL_R) {
            setFontFileName(ext_font_Symbol, val);
        } else if (key == CONFIG_MUPDF_FONT_PDF_DINGBAT_R) {
            setFontFileName(ext_font_ZapfDingbats, val);
        } else {
            LE("processFontsConfig unknown key: key=%d, val=%s", key, val);
        }
    }
    if (document != nullptr) {
        restart();
    }
    response.addInt(pageCount);
}

void MuPdfBridge::processSystemFont(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PDF_SYSTEM_FONT;
    CmdDataIterator iter(request.first);
    uint8_t* fonts[2];
    uint32_t index;
    iter.getByteArray(&fonts[0]).getByteArray(&fonts[1]).getInt(&index);
    if (!iter.isValid()) {
        LE("No mapping found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    LI("%s: %s[%d]", fonts[0], fonts[1], index);
    if (ext_system_fonts_count < MAX_SYS_FONT_TABLE_SIZE) {
        setFontFileName(ext_system_fonts[ext_system_fonts_count][0],
                reinterpret_cast<const char *>(fonts[0]));
        setFontFileName(ext_system_fonts[ext_system_fonts_count][1],
                reinterpret_cast<const char *>(fonts[1]));
        ext_system_fonts_idx[ext_system_fonts_count] = index;
        ext_system_fonts_count++;
    } else {
        LE("No more fonts allowed");
        return;
    }
}

static void gatherfonts(fz_context *ctx, pdf_document* doc, pdf_obj* rsrc, pdf_obj *dict,
        std::set<std::string>& external, std::set<std::string>& all)
{
    int n = pdf_dict_len(ctx, dict);
    for (int i = 0; i < n; i++) {
        pdf_obj *fontdict = nullptr;
        pdf_obj *subtype = nullptr;
        pdf_obj *basefont = nullptr;
        pdf_obj *name = nullptr;
        int k;
        fontdict = pdf_dict_get_val(ctx, dict, i);
        if (!pdf_is_dict(ctx, fontdict)) {
            continue;
        }
        basefont = pdf_dict_gets(ctx, fontdict, "BaseFont");
        if (!basefont || pdf_is_null(ctx, basefont)) {
            continue;
        }
        std::string basefontname = std::string(pdf_to_name(ctx, basefont));
        if (all.find(basefontname) != all.end()) {
            continue;
        }
        all.insert(basefontname);
        pdf_font_desc* font = pdf_load_font(ctx, doc, rsrc, fontdict, 0);
        if (font) {
            if (font->is_embedded) {
                LI("Embedded Document font: basefont=%s", basefontname.c_str());
            } else if (font->external_origin == PDF_FD0_BUILTIN) {
                LI("Built-in Document font: basefont=%s file=%s",
                        basefontname.c_str(), font->font->ft_filepath);
            } else if (font->external_origin == PDF_FD0_SYSTEM) {
                LI("System   Document font: basefont=%s file=%s",
                        basefontname.c_str(), font->font->ft_filepath);
            } else {
                LI("External Document font: basefont=%s file=%s",
                        basefontname.c_str(), font->font->ft_filepath);
                external.insert(basefontname);
            }
        } else {
               LE("Unknown  Document font: basefont=%s", basefontname.c_str());
        }
    }
}

static void gatherresourceinfo(fz_context *ctx, pdf_document* doc, pdf_obj *rsrc,
        std::set<std::string>& external, std::set<std::string>& all)
{
    pdf_obj *font;
    pdf_obj *xobj;
    pdf_obj *shade;
    pdf_obj *pattern;
    pdf_obj *subrsrc;
    font = pdf_dict_gets(ctx, rsrc, "Font");
    gatherfonts(ctx, doc, rsrc, font, external, all);
    int n = pdf_dict_len(ctx, font);
    for (int i = 0; i < n; i++) {
        pdf_obj *obj = pdf_dict_get_val(ctx, font, i);
        subrsrc = pdf_dict_gets(ctx, obj, "Resources");
        if (subrsrc && pdf_objcmp(ctx, rsrc, subrsrc)) {
            gatherresourceinfo(ctx, doc, subrsrc, external, all);
        }
    }
}

void MuPdfBridge::processGetMissedFonts(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PDF_GET_MISSED_FONTS;
    if (format != FORMAT_PDF) {
        response.result = RES_ILLEGAL_STATE;
        return;
    }
    restart();
    auto doc = (pdf_document*) document;
    std::set<std::string> all;
    fonts.clear();
    for (int i = 0; i < pageCount; i++) {
        pdf_obj *pageobj;
        pdf_obj *pageref;
        pdf_obj *rsrc;
        pageref = pdf_lookup_page_obj(ctx, doc, i);
        pageobj = pdf_resolve_indirect(ctx, pageref);
        if (pageobj) {
            rsrc = pdf_dict_gets(ctx, pageobj, "Resources");
            gatherresourceinfo(ctx, doc, rsrc, fonts, all);
        }
    }
    response.addInt(fonts.size());
    for (const auto & font : fonts) {
    	response.addIpcString(font.c_str(), false);
    }
}

