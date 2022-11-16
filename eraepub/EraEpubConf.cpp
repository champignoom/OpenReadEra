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
 * Developers: ReadEra Team (2013-2020), Playful Curiosity (2013-2020),
 * Tarasus (2018-2020).
 */

#include "EraEpubBridge.h"
#include <dirent.h>

static const int ALLOWED_INTERLINE_SPACES[] =
{
        70, 75, 80, 85, 90, 95, 100, 105, 110, 115,
        120, 125, 130, 135, 140, 145, 150, 160, 180, 200
};

static const int ALLOWED_FONT_SIZES[] =
{
		12, 13, 14, 15, 16, 17, 18, 19, 20,
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
		31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		42, 44, 46, 48, 50,
		52, 54, 56, 58, 60,
		62, 64, 66, 68, 70,
        72, 74, 76, 78, 80,
        82, 84, 86, 88, 90,
        92, 94, 96, 98, 100,
        102, 104, 106, 108, 110,
        112, 114, 116, 118, 120,
        125, 130, 135, 140, 145, 150, 155, 160
};

static const int ALLOWED_PARAGRAPH_MARGINS[] =
{
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100,
        110, 120, 130, 140, 150, 160, 170, 180, 190, 200
};

static int GetClosestValueInArray(const int array[], int array_length, int value)
{
	int closest_value = -1;
	int smallest_delta = -1;
	for (int i = 0; i < array_length; i++) {
		int delta = array[i] - value;
		if (delta < 0) {
			delta = -delta;
		}
		if (smallest_delta == -1 || smallest_delta > delta) {
			smallest_delta = delta;
			closest_value = array[i];
		}
	}
	return closest_value;
}

static int parseInt(const char* s) {
    return atoi(s);
}

static lUInt32 parseColor(const char* s) {
    return (lUInt32) (atoi(s) & 0xFFFFFF);
}

void CreBridge::processFontsConfig(CmdRequest& request, CmdResponse& response)
{
    LD("processFontsConfig");
    response.cmd = CMD_RES_SET_FONT_CONFIG;
    CmdDataIterator iter(request.first);
    fontMan->clear();
    while (iter.hasNext()) {
        uint32_t key;
        uint8_t* temp_val;
        iter.getInt(&key).getByteArray(&temp_val);
        if (!iter.isValid()) {
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        const char* val = reinterpret_cast<const char*>(temp_val);
        int len = strlen(val);
        if (len == 0) {
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        lString8 font = lString8(val,len);
        fontMan->RegisterFont(font, false);
    }
    if(doc_view_ && doc_view_->GetCrDom())
    {
        doc_view_->GetCrDom()->registerEmbeddedFonts();
    }
    // For the future return of pages count if fonts may be added during work
    // and they may change layout of doc.
    response.addInt(0);
}

void CreBridge::processConfig(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_SET_CONFIG;
    if (!doc_view_) {
        doc_view_ = new LVDocView();
        if  (FALLBACK_FACE_DEFAULT != lString8("NONE")) {
            fontMan->InitFallbackFontDefault();
        }
    }
    doc_view_->clearStylesheetToBase();
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
        if (key == CONFIG_CRE_PAGE_WIDTH) {
            int int_val = parseInt(val);
            doc_view_->Resize(int_val, doc_view_->height_);
        } else if (key == CONFIG_CRE_PAGE_HEIGHT) {
            int int_val = parseInt(val);
            doc_view_->Resize(doc_view_->width_, int_val);
        } else if (key == CONFIG_CRE_FONT_ANTIALIASING) {
            int int_val = parseInt(val);
            if (int_val < 0 || int_val > 2) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            fontMan->SetAntialiasMode(int_val);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_FONT_GAMMA) {
            double gamma = 1.0;
            if (sscanf(val, "%lf", &gamma) == 1) {
                fontMan->SetGamma(gamma);
            }
        } else if (key == CONFIG_CRE_PAGES_COLUMNS) {
            int int_val = parseInt(val);
            if (int_val < 1 || int_val > 2) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            if (doc_view_->page_columns_ != int_val) {
                doc_view_->page_columns_ = int_val;
                doc_view_->UpdateLayout();
                doc_view_->RequestRender();
                doc_view_->position_is_set_ = false;
            }
        } else if (key == CONFIG_CRE_FONT_COLOR) {
            doc_view_->text_color_ = parseColor(val);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_BACKGROUND_COLOR) {
            doc_view_->background_color_ = parseColor(val);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_MARGIN_LEFT) {
            int margin = parseInt(val);
            doc_view_->cfg_margins_.left = margin;
            doc_view_->UpdatePageMargins();
        } else if (key == CONFIG_CRE_MARGIN_TOP) {
            int margin = parseInt(val);
            doc_view_->cfg_margins_.top = margin;
            doc_view_->UpdatePageMargins();
        } else if (key == CONFIG_CRE_MARGIN_RIGHT) {
            int margin = parseInt(val);
            doc_view_->cfg_margins_.right = margin;
            doc_view_->UpdatePageMargins();
        } else if (key == CONFIG_CRE_MARGIN_BOTTOM) {
            int margin = parseInt(val);
            doc_view_->cfg_margins_.bottom = margin;
            doc_view_->UpdatePageMargins();
        } else if (key == CONFIG_CRE_FONT_FACE_MAIN) {
            doc_view_->cfg_font_face_ = UnicodeToUtf8(lString16(val));
            gligMapManager.initMap(lString16(val));
            doc_view_->UpdatePageMargins();
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_FONT_FACE_FALLBACK) {
            //
        } else if (key == CONFIG_CRE_FONT_SIZE) {
            int int_val = parseInt(val);
            int array_length = sizeof(ALLOWED_FONT_SIZES) / sizeof(int);
            int_val = GetClosestValueInArray(ALLOWED_FONT_SIZES, array_length, int_val);
            if (doc_view_->cfg_font_size_ != int_val) {
                doc_view_->cfg_font_size_ = int_val;
                fontMan->font_size_ = int_val;
                doc_view_->UpdatePageMargins();
                doc_view_->RequestRender();
            }
        } else if (key == CONFIG_CRE_INTERLINE) {
            int int_val = parseInt(val);
            int array_length = sizeof(ALLOWED_INTERLINE_SPACES) / sizeof(int);
            int_val = GetClosestValueInArray(ALLOWED_INTERLINE_SPACES, array_length, int_val);
            if (doc_view_->cfg_interline_space_ != int_val) {
                    doc_view_->cfg_interline_space_ = int_val;
                    doc_view_->RequestRender();
                    doc_view_->position_is_set_ = false;
            }
        } else if (key == CONFIG_CRE_EMBEDDED_STYLES) {
            int int_val = parseInt(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            bool bool_val = (bool) int_val;
            doc_view_->cfg_embeded_styles_ = bool_val;
            doc_view_->GetCrDom()->setDocFlag(DOC_FLAG_EMBEDDED_STYLES, bool_val);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_EMBEDDED_FONTS) {
            int int_val = parseInt(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            bool bool_val = (bool) int_val;
            doc_view_->cfg_embeded_fonts_ = bool_val;
            doc_view_->GetCrDom()->setDocFlag(DOC_FLAG_EMBEDDED_FONTS, bool_val);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_FOOTNOTES) {
            int int_val = parseInt(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            bool bool_val = (bool) int_val;
            doc_view_->cfg_enable_footnotes_ = bool_val;
            //doc_view_->GetCrDom()->setDocFlag(DOC_FLAG_ENABLE_FOOTNOTES, bool_val);
            doc_view_->GetCrDom()->setDocFlag(DOC_FLAG_ENABLE_FOOTNOTES, true);
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_TEXT_ALIGN) {
            int int_val = parseInt(val);
            if (int_val < 0 || int_val > 3) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            doc_view_->SetTextAlign(int_val);
            doc_view_->UpdatePageMargins();
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_HYPHENATION) {
            int int_val = parseInt(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            if (int_val == 0) {
                HyphMan::activateDictionary(lString16(HYPH_DICT_ID_NONE));
            } else {
                HyphMan::activateDictionary(lString16(HYPH_DICT_ID_ALGORITHM));
            }
            doc_view_->UpdatePageMargins();
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_FLOATING_PUNCTUATION) {
            int int_val = parseInt(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            bool bool_val = (bool) int_val;
            if (bool_val != gFlgFloatingPunctuationEnabled) {
                gFlgFloatingPunctuationEnabled = bool_val;
            }
            doc_view_->UpdatePageMargins();
            doc_view_->RequestRender();
        } else if (key == CONFIG_CRE_FIRSTPAGE_THUMB) {
            int int_val = parseInt(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            bool bool_val = (bool) int_val;
            doc_view_->cfg_firstpage_thumb_ = bool_val;
        } else if (key == CONFIG_ERA_EMBEDDED_STYLES) {
            int int_val = parseInt(val);
            if (int_val < 0 || int_val > 5) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            gEmbeddedStylesLVL = int_val;
        } else if (key == CONFIG_ERA_VERTICAL_MODE){
            int int_val = parseInt(val);
            gJapaneseVerticalMode = int_val;
        } else if (key == CONFIG_ERA_TEXT_INDENT){
            int int_val = parseInt(val);
            if (int_val < 0 || int_val > 2) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            doc_view_->GetCrDom()->cfg_txt_indent = int_val;
            doc_view_->SetTextIndent(int_val);
            doc_view_->UpdatePageMargins();
            doc_view_->RequestRender();
        } else if (key == CONFIG_ERA_PARAGRAPH_MARGIN){
            int int_val = parseInt(val);
            int array_length = sizeof(ALLOWED_PARAGRAPH_MARGINS) / sizeof(int);
            int_val = GetClosestValueInArray(ALLOWED_PARAGRAPH_MARGINS, array_length, int_val);
            doc_view_->GetCrDom()->cfg_txt_margin = int_val;
            doc_view_->SetParagraphMargin(int_val);
            doc_view_->UpdatePageMargins();
            doc_view_->RequestRender();
        }else if (key == CONFIG_ERA_INDENT_MARGIN_OVERRIDE){
            int int_val = parseInt(val);
            if (int_val < 0 || int_val > 1) {
                response.result = RES_BAD_REQ_DATA;
                return;
            }
            bool old_val = doc_view_->GetCrDom()->cfg_txt_indent_margin_override;
            if( old_val != (bool)int_val)
            {
                doc_view_->GetCrDom()->force_render = true;
            }
            doc_view_->GetCrDom()->cfg_txt_indent_margin_override = (bool)int_val;
            doc_view_->RequestRender();
        } else {
            CRLog::warn("processConfig unknown key: key=%d, val=%s", key, val);
        }
    }

    if(doc_view_->GetCrDom()->cfg_txt_indent_margin_override)
    {
        //LE("cfg_txt_indent_margin_override in  indent = [%d]  margin = [%d] ", doc_view_->GetCrDom()->cfg_txt_indent, doc_view_->GetCrDom()->cfg_txt_margin);
        int fsize = fontMan->font_size_;
        switch (doc_view_->GetCrDom()->cfg_txt_indent)
        {
            case  0: doc_view_->GetCrDom()->cfg_txt_indent = 0            ; break; // CR_CSS_INDENT_NONE
            case  1: doc_view_->GetCrDom()->cfg_txt_indent = fsize * 1.2f ; break; // CR_CSS_INDENT_NORMAL
            case  2: doc_view_->GetCrDom()->cfg_txt_indent = fsize * 2    ; break; // CR_CSS_INDENT_DOUBLE
            default: doc_view_->GetCrDom()->cfg_txt_indent = fsize * 1.2f ; break; // CR_CSS_INDENT_NORMAL
        }
        doc_view_->GetCrDom()->cfg_txt_margin = fsize * doc_view_->GetCrDom()->cfg_txt_margin / 100;
        //LE("cfg_txt_indent_margin_override out indent = [%d]  margin = [%d] ", doc_view_->GetCrDom()->cfg_txt_indent, doc_view_->GetCrDom()->cfg_txt_margin);
    }
    doc_view_->hitboxesCash.reset();
    doc_view_->RenderIfDirty();
    response.addInt(ExportPagesCount(doc_view_->GetColumns(), doc_view_->GetPagesCount()));
}

void CreBridge::processFontNames(CmdRequest &request, CmdResponse &response)
{
    CRLog::trace("processFontNames START");
    response.cmd = CMD_RES_FONT_NAMES;
    CmdDataIterator iter(request.first);
    uint8_t *dir_str;

    iter.getByteArray(&dir_str);
    if (!iter.isValid())
    {
        //CRLog::error("processFontNames invalid iterator BUT THAT's DEBUG!!11");
        CRLog::error("processFontNames invalid iterator");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    lString16 dir_path(reinterpret_cast<const char *>(dir_str));
    //lString16 dir_path("/sdcard/fonts");
    //lString16 dir_path("/system/fonts");

    //std::string path(LCSTR(dir_path));

    LVArray<lString16> filelist;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(LCSTR(dir_path))) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            lString16 filename(ent->d_name);
            lString16 filename_check = filename;
            filename_check.lowercase();
            if (filename_check.startsWith(".") || !filename_check.endsWith(".ttf") && !filename_check.endsWith(".otf") && !filename_check.endsWith(".ttc"))
            {
                if (filename_check != "." && filename_check != "..") {
                    // "." и ".." это специальные файлы указывающие на текущую и родительскую директорию
                    CRLog::warn("processFontNames invalid file format at [%s/%s]",
                            LCSTR(dir_path), LCSTR(filename));
                }
                continue;
            }
            filename = dir_path + "/" + filename;
            //CRLog::error("path = %s", LCSTR(filename));
            lString8Collection coll;
            fontMan->UnregisterDocumentFonts(-1);
            coll.addAll(fontMan->RegisterFont(UnicodeToUtf8(filename), true));
            if (coll.length() == 0)
            {
                CRLog::warn("processFontNames file [%s] contains no faces or corrupted", LCSTR(filename));
                continue;
            }
            for (int i = 0; i < coll.length(); i++)
            {
                lString16 resp = filename + ";" + Utf8ToUnicode(coll.at(i));
                //CRLog::error("response = %s", LCSTR(resp));
                responseAddString(response, resp);
            }
        }
        closedir(dir);
    }
    else
    {
        CRLog::warn("processFontNames couldn't open dir [%s]", LCSTR(dir_path));
    }
    CRLog::trace("processFontNames END");
}

void CreBridge::processFontLigmap(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_FONT_LIGMAP;
    CmdDataIterator iter(request.first);
    while (iter.hasNext())
    {
        uint32_t lig_type;
        if (!iter.getInt(&lig_type).isValid()) {
            LE("No lig_type found");
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        uint8_t* face_str;
        uint8_t* path_str;
        iter.getByteArray(&face_str).getByteArray(&path_str);

        lString16 face(reinterpret_cast<const char *>(face_str));
        lString16 path(reinterpret_cast<const char *>(path_str));
        //const char* path ="sdcard/download/font.txt";

        //LE("processFontLigmap addmap [%d] %s -> %s", lig_type,LCSTR(face), LCSTR(path));
        gligMapManager.addMap(face,lig_type,path);

        if (!iter.isValid()) {
            LE("processFontLigmap no fonts");
            response.result = RES_BAD_REQ_DATA;
            return;
        }
    }
}

