/*
 * Copyright (C) 2016 Bartek Fabiszewski: http://www.fabiszewski.net
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

#include "EraMobiBridge.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
#include "src/mobi.h"
#include "tools/common.h"
#ifdef __cplusplus
}
#endif //__cplusplus

static bool eramobi_format_probe(const char *absolute_path) {
    MOBI_RET mobi_ret;
    MOBIData *m = mobi_init();
    if (m == nullptr) {
        LE("Memory allocation failed");
        return false;
    }
    mobi_parse_kf7(m);
    FILE *file = fopen(absolute_path, "rbe");
    if (file == nullptr) {
        LE("Error opening file: %s", absolute_path);
        mobi_free(m);
        return false;
    }
    mobi_ret = mobi_load_file(m, file);
    fclose(file);
    if (mobi_ret != MOBI_SUCCESS) {
        LE("mobi_ret != MOBI_SUCCESS");
        mobi_free(m);
        return false;
    }
    mobi_free(m);
    return true;
}

/**
 @brief Print all loaded headers meta information
 @param[in] m MOBIData structure
 */
static void eramobi_meta_test(const MOBIData *m) {
    /* Full name stored at offset given in MOBI header */
    if (m->mh && m->mh->full_name) {
        char full_name[FULLNAME_MAX + 1];
        if (mobi_get_fullname(m, full_name, FULLNAME_MAX) == MOBI_SUCCESS) {
            LV("\nFull name: %s\n", full_name);
        }
    }
    /* Palm database header */
    if (m->ph) {
        LV("\nPalm doc header:\n");
        LV("name: %s\n", m->ph->name);
        LV("attributes: %hu\n", m->ph->attributes);
        LV("version: %hu\n", m->ph->version);
        struct tm *timeinfo = mobi_pdbtime_to_time(static_cast<const long>(m->ph->ctime));
        LV("ctime: %s", asctime(timeinfo));
        timeinfo = mobi_pdbtime_to_time(static_cast<const long>(m->ph->mtime));
        LV("mtime: %s", asctime(timeinfo));
        timeinfo = mobi_pdbtime_to_time(static_cast<const long>(m->ph->btime));
        LV("btime: %s", asctime(timeinfo));
        LV("mod_num: %u\n", m->ph->mod_num);
        LV("appinfo_offset: %u\n", m->ph->appinfo_offset);
        LV("sortinfo_offset: %u\n", m->ph->sortinfo_offset);
        LV("type: %s\n", m->ph->type);
        LV("creator: %s\n", m->ph->creator);
        LV("uid: %u\n", m->ph->uid);
        LV("next_rec: %u\n", m->ph->next_rec);
        LV("rec_count: %u\n", m->ph->rec_count);
    }
    /* Record 0 header */
    if (m->rh) {
        LV("\nRecord 0 header:\n");
        LV("compresion type: %u\n", m->rh->compression_type);
        LV("text length: %u\n", m->rh->text_length);
        LV("text record count: %u\n", m->rh->text_record_count);
        LV("text record size: %u\n", m->rh->text_record_size);
        LV("encryption type: %u\n", m->rh->encryption_type);
        LV("unknown: %u\n", m->rh->unknown1);
    }
    /* Mobi header */
    if (m->mh) {
        LV("MOBI header:");
        LV("identifier: %s", m->mh->mobi_magic);
        if (m->mh->header_length) { LV("header length: %u", *m->mh->header_length); }
        if (m->mh->mobi_type) { LV("mobi type: %u", *m->mh->mobi_type); }
        if (m->mh->text_encoding) { LV("text encoding: %u", *m->mh->text_encoding); }
        if (m->mh->uid) { LV("unique id: %u", *m->mh->uid); }
        if (m->mh->version) { LV("file version: %u", *m->mh->version); }
        if (m->mh->orth_index) { LV("orth index: %u", *m->mh->orth_index); }
        if (m->mh->infl_index) { LV("infl index: %u", *m->mh->infl_index); }
        if (m->mh->names_index) { LV("names index: %u", *m->mh->names_index); }
        if (m->mh->keys_index) { LV("keys index: %u", *m->mh->keys_index); }
        if (m->mh->extra0_index) { LV("extra0 index: %u", *m->mh->extra0_index); }
        if (m->mh->extra1_index) { LV("extra1 index: %u", *m->mh->extra1_index); }
        if (m->mh->extra2_index) { LV("extra2 index: %u", *m->mh->extra2_index); }
        if (m->mh->extra3_index) { LV("extra3 index: %u", *m->mh->extra3_index); }
        if (m->mh->extra4_index) { LV("extra4 index: %u", *m->mh->extra4_index); }
        if (m->mh->extra5_index) { LV("extra5 index: %u", *m->mh->extra5_index); }
        if (m->mh->non_text_index) { LV("non text index: %u", *m->mh->non_text_index); }
        if (m->mh->full_name_offset) { LV("full name offset: %u", *m->mh->full_name_offset); }
        if (m->mh->full_name_length) { LV("full name length: %u", *m->mh->full_name_length); }
        if (m->mh->locale) {
            const char *locale_string = mobi_get_locale_string(*m->mh->locale);
            if (locale_string) {
                LV("locale: %s (%u)", locale_string, *m->mh->locale);
            } else {
                LV("locale: unknown (%u)", *m->mh->locale);
            }
        }
        if (m->mh->dict_input_lang) {
            const char *locale_string = mobi_get_locale_string(*m->mh->dict_input_lang);
            if (locale_string) {
                LV("dict input lang: %s (%u)\n", locale_string, *m->mh->dict_input_lang);
            } else {
                LV("dict input lang: unknown (%u)\n", *m->mh->dict_input_lang);
            }
        }
        if (m->mh->dict_output_lang) {
            const char *locale_string = mobi_get_locale_string(*m->mh->dict_output_lang);
            if (locale_string) {
                LV("dict output lang: %s (%u)\n", locale_string, *m->mh->dict_output_lang);
            } else {
                LV("dict output lang: unknown (%u)\n", *m->mh->dict_output_lang);
            }
        }
        if (m->mh->min_version) { LV("minimal version: %u", *m->mh->min_version); }
        if (m->mh->image_index) { LV("first image index: %u", *m->mh->image_index); }
        if (m->mh->huff_rec_index) { LV("huffman record offset: %u", *m->mh->huff_rec_index); }
        if (m->mh->huff_rec_count) { LV("huffman records count: %u", *m->mh->huff_rec_count); }
        if (m->mh->datp_rec_index) { LV("DATP record offset: %u", *m->mh->datp_rec_index); }
        if (m->mh->datp_rec_count) { LV("DATP records count: %u", *m->mh->datp_rec_count); }
        if (m->mh->exth_flags) { LV("EXTH flags: %u", *m->mh->exth_flags); }
        if (m->mh->unknown6) { LV("unknown: %u", *m->mh->unknown6); }
        if (m->mh->drm_offset) { LV("drm offset: %u", *m->mh->drm_offset); }
        if (m->mh->drm_count) { LV("drm count: %u", *m->mh->drm_count); }
        if (m->mh->drm_size) { LV("drm size: %u", *m->mh->drm_size); }
        if (m->mh->drm_flags) { LV("drm flags: %u", *m->mh->drm_flags); }
        if (m->mh->first_text_index) { LV("first text index: %u", *m->mh->first_text_index); }
        if (m->mh->last_text_index) { LV("last text index: %u", *m->mh->last_text_index); }
        if (m->mh->fdst_index) { LV("FDST offset: %u", *m->mh->fdst_index); }
        if (m->mh->fdst_section_count) { LV("FDST count: %u", *m->mh->fdst_section_count); }
        if (m->mh->fcis_index) { LV("FCIS index: %u", *m->mh->fcis_index); }
        if (m->mh->fcis_count) { LV("FCIS count: %u", *m->mh->fcis_count); }
        if (m->mh->flis_index) { LV("FLIS index: %u", *m->mh->flis_index); }
        if (m->mh->flis_count) { LV("FLIS count: %u", *m->mh->flis_count); }
        if (m->mh->unknown10) { LV("unknown: %u", *m->mh->unknown10); }
        if (m->mh->unknown11) { LV("unknown: %u", *m->mh->unknown11); }
        if (m->mh->srcs_index) { LV("SRCS index: %u", *m->mh->srcs_index); }
        if (m->mh->srcs_count) { LV("SRCS count: %u", *m->mh->srcs_count); }
        if (m->mh->unknown12) { LV("unknown: %u", *m->mh->unknown12); }
        if (m->mh->unknown13) { LV("unknown: %u", *m->mh->unknown13); }
        if (m->mh->extra_flags) { LV("extra record flags: %u\n", *m->mh->extra_flags); }
        if (m->mh->ncx_index) { LV("NCX offset: %u", *m->mh->ncx_index); }
        if (m->mh->unknown14) { LV("unknown: %u", *m->mh->unknown14); }
        if (m->mh->unknown15) { LV("unknown: %u", *m->mh->unknown15); }
        if (m->mh->fragment_index) { LV("fragment index: %u", *m->mh->fragment_index); }
        if (m->mh->skeleton_index) { LV("skeleton index: %u", *m->mh->skeleton_index); }
        if (m->mh->datp_index) { LV("DATP index: %u", *m->mh->datp_index); }
        if (m->mh->unknown16) { LV("unknown: %u", *m->mh->unknown16); }
        if (m->mh->guide_index) { LV("guide index: %u", *m->mh->guide_index); }
        if (m->mh->unknown17) { LV("unknown: %u", *m->mh->unknown17); }
        if (m->mh->unknown18) { LV("unknown: %u", *m->mh->unknown18); }
        if (m->mh->unknown19) { LV("unknown: %u", *m->mh->unknown19); }
        if (m->mh->unknown20) { LV("unknown: %u", *m->mh->unknown20); }
    }
}
