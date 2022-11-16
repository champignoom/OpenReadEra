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

#include <StSocket.h>
#include "EraMobiBridge.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
#include "src/mobi.h"
#include "tools/common.h"
#ifdef __cplusplus
}
#endif //__cplusplus

typedef struct {
    unsigned char *thumb;
    int thumb_width;
    int thumb_height;
    char *title;
    char *authors;
    char *series_name;
    int series_number;
    char *lang;
    int thumb_size;
    char *subject;
    char *description;
} eramobi_meta_pack;

static char* MallocEmptyString() {
    char * s = static_cast<char *>(malloc(sizeof(char) * 1));
    s[0] = '\0';
    return s;
}

static char* MetaStringNormalize(char *s) {
    if (s) {
        // META_STRING_MAX_LENGTH + 1, чтобы уместился терминатор
        size_t buff_size = sizeof(wchar_t) * (META_STRING_MAX_LENGTH + 1);
        auto *wstr = static_cast<wchar_t *>(malloc(buff_size));
        wstr[META_STRING_MAX_LENGTH] = L'\0';
        size_t len = std::mbstowcs(wstr, s, META_STRING_MAX_LENGTH);
        free(s);
        if (len == static_cast<std::size_t> (-1)) {
            free(wstr);
            return MallocEmptyString();
        }
        auto *mbstr = static_cast<char *>(malloc(buff_size));
        mbstr[buff_size / sizeof(char)] = '\0';
        len = std::wcstombs(mbstr, wstr, buff_size - 1);
        free(wstr);
        if (len == static_cast<std::size_t> (-1)) {
            free(mbstr);
            return MallocEmptyString();
        }
        return mbstr;
    } else {
        // Протокол передачи данных не принимает nullptr, нужна пустая строка.
        return MallocEmptyString();
    }
}

static int eramobi_cover_id(MOBIData *m)
{
    if(m == nullptr || m->eh == nullptr)
    {
        return -2;
    }
    MOBIExthHeader *hdr = mobi_get_exthrecord_by_tag(m, EXTH_COVEROFFSET);
    if(hdr == nullptr)
    {
        return -2;
    }
    uint32_t exthId = mobi_decode_exthvalue(static_cast<const unsigned char *>(hdr->data), hdr->size);
    return exthId;
}

static void eramobi_process_meta(const char *path, int src_fd, eramobi_meta_pack *pack) {
    MOBI_RET mobi_ret;
    /* Initialize main MOBIData structure */
    MOBIData *m = mobi_init();
    if (m == nullptr) {
        LE("Memory allocation failed");
        return;
    }
    mobi_parse_kf7(m);
    FILE *file;
    if (src_fd > 0)
    {
        file = fdopen(src_fd, "rb");
    }
    else
    {
        file = fopen(path, "rbe");
    }
    if (file == nullptr) {
        LE("Error opening file: %s", path);
        mobi_free(m);
        return;
    }
    mobi_ret = mobi_load_file(m, file);
    fclose(file);
    if (mobi_ret != MOBI_SUCCESS) {
        LE("mobi_ret != MOBI_SUCCESS");
        mobi_free(m);
        return;
    }
    pack->title = mobi_meta_get_title(m);
    pack->authors = mobi_meta_get_author(m);
    pack->lang = mobi_meta_get_language(m);
    pack->subject = mobi_meta_get_subject(m);
    pack->description = mobi_meta_get_description(m);

    MOBIRawml *rawml = mobi_init_rawml(m);
    if (rawml == nullptr) {
        LE("rawml == NULL");
        mobi_free(m);
        return;
    }
    mobi_ret = mobi_parse_rawml(rawml, m);
    if (mobi_ret != MOBI_SUCCESS) {
        LE("mobi_ret != MOBI_SUCCESS");
        mobi_free(m);
        mobi_free_rawml(rawml);
        return;
    }
    if (m->eh == nullptr) {
        mobi_free_rawml(rawml);
        mobi_free(m);
        return;
    }
    int cover_id = eramobi_cover_id(m);
    if (cover_id < 0) {
        mobi_free_rawml(rawml);
        mobi_free(m);
        return;
    }
    MOBIPart *result = nullptr;
    if (rawml->resources != nullptr) {
        MOBIPart *curr = rawml->resources;
        while (curr != nullptr) {
            MOBIFileMeta file_meta = mobi_get_filemeta_by_type(curr->type);
            if (curr->size > 0) {
                if (file_meta.type == T_GIF
                    || file_meta.type == T_JPG
                    || file_meta.type == T_BMP
                    || file_meta.type == T_PNG) {
                    if (curr->uid == cover_id) {
                        result = curr;
                    }
                }
            }
            curr = curr->next;
        }
    }
    if (!result) {
        mobi_free_rawml(rawml);
        mobi_free(m);
        return;
    }
    if(result->type == T_BMP || result->type == T_PNG || result->type == T_JPG || result->type == T_GIF )
    {
        pack->thumb = result->data;
        pack->thumb_size = result->size;
        return;
    }
    else
    {
        mobi_free_rawml(rawml);
        mobi_free(m);
        return;
    }
}

void EraMobiBridge::processMeta(CmdRequest &request, CmdResponse &response) {
    response.cmd = CMD_RES_CRE_METADATA;
    CmdDataIterator iter(request.first);
    uint32_t format = 0;
    uint8_t *path_arg;
    uint8_t *socket_name;
    uint32_t direct_archive = 0;
    iter.getByteArray(&socket_name)
        .getInt(&format)
        .getByteArray(&path_arg)
        .getInt(&direct_archive);

    if (!iter.isValid() || !socket_name) {
        LE("processMeta: iterator invalid data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *path = reinterpret_cast<const char *>(path_arg);
    if (format != DOC_FORMAT_MOBI && format != DOC_FORMAT_AZW && format != DOC_FORMAT_AZW3) {
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (OreIsNormalDirectArchive(direct_archive)) {
        // Not supported
        response.result = RES_ARCHIVE_COLLISION;
        return;
    } else if (OreIsSmartDirectArchive(direct_archive)) {
        // Not supported
        response.result = RES_INTERNAL_ERROR;
        return;
    }

    const bool send_fd_via_socket = ( strlen((const char*) socket_name) > 0 );
    int fd;

    if (send_fd_via_socket)
    {
        StSocketConnection connection((const char *) socket_name);
        if (!connection.isValid()){
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        bool received = connection.receiveFileDescriptor(fd);
        if (!received)
        {
            response.result = RES_BAD_REQ_DATA;
            return;
        }
    }
    else
    {
        fd = -1;
    }

    eramobi_meta_pack meta = {nullptr, 0, 0, nullptr, nullptr, nullptr, 0, nullptr, 0, nullptr, nullptr};
    eramobi_process_meta(path, fd, &meta);
    auto doc_thumb = new CmdData();
    doc_thumb->type = TYPE_ARRAY_POINTER;
    if (meta.thumb) {
        if (meta.thumb_size <= META_THUMB_MAX_SIZE) {
            doc_thumb->external_array = meta.thumb;
            doc_thumb->value.value32 = meta.thumb_size;
            doc_thumb->owned_external = true;
        } else {
            LW("Ignoring large doc thumb");
            free(meta.thumb);
            meta.thumb = nullptr;
            meta.thumb_width = 0;
            meta.thumb_height = 0;
        }
    }
    response.addData(doc_thumb);
    response.addInt((uint32_t) meta.thumb_width);
    response.addInt((uint32_t) meta.thumb_height);
    response.addIpcString(MetaStringNormalize(meta.title), true);
    response.addIpcString(MetaStringNormalize(meta.authors), true);
    response.addIpcString(MetaStringNormalize(meta.series_name), true);
    response.addInt((uint32_t) meta.series_number);
    response.addIpcString(MetaStringNormalize(meta.lang), true);
    response.addIpcString(MetaStringNormalize(meta.subject), true);
    response.addIpcString(MetaStringNormalize(meta.description), true);
    response.addInt(0); //embedded fonts
    response.addInt(0); //japanese_vertical
}