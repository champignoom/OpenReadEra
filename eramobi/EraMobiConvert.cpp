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
/* miniz file is needed for EPUB creation */
#define MINIZ_HEADER_FILE_ONLY
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include "src/miniz.c"
#ifdef __cplusplus
}
#endif //__cplusplus

#define EPUB_CONTAINER "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n\
  <rootfiles>\n\
    <rootfile full-path=\"OEBPS/content.opf\" media-type=\"application/oebps-package+xml\"/>\n\
  </rootfiles>\n\
</container>"
#define EPUB_MIMETYPE "application/epub+zip"

/**
 @brief Bundle recreated source files into EPUB container

 This function is a simple example.
 In real world implementation one should validate and correct all input
 markup to check if it conforms to OPF and HTML specifications and
 correct all the issues.

 @param[in] rawml MOBIRawml structure holding parsed records
 @param[in] fullpath File path will be parsed to build basenames of dumped records
 */
static bool eramobi_epub_create(const MOBIRawml *rawml, const char *fullpath) {
    if (rawml == nullptr) {
        printf("Rawml structure not initialized\n");
        return false;
    }
    char dirname[FILENAME_MAX];
    char basename[FILENAME_MAX];
    split_fullpath(fullpath, dirname, basename);
    char zipfile[FILENAME_MAX];
    snprintf(zipfile, sizeof(zipfile), "%s%s", dirname, basename);

    LI("Saving EPUB to %s\n", zipfile);
    /* create zip (epub) archive */
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(mz_zip_archive));
    mz_bool mz_ret = mz_zip_writer_init_file(&zip, zipfile, 0);
    if (!mz_ret) {
        LI("Could not initialize zip archive\n");
        return false;
    }
    /* start adding files to archive */
    mz_ret = mz_zip_writer_add_mem(&zip, "mimetype", EPUB_MIMETYPE, sizeof(EPUB_MIMETYPE) - 1,
                                   MZ_NO_COMPRESSION);
    if (!mz_ret) {
        LI("Could not add mimetype\n");
        mz_zip_writer_end(&zip);
        return false;
    }
    mz_ret = mz_zip_writer_add_mem(&zip, "META-INF/container.xml", EPUB_CONTAINER,
                                   sizeof(EPUB_CONTAINER) - 1, (mz_uint) MZ_DEFAULT_COMPRESSION);
    if (!mz_ret) {
        LI("Could not add container.xml\n");
        mz_zip_writer_end(&zip);
        return false;
    }
    char partname[FILENAME_MAX];
    if (rawml->markup != nullptr) {
        /* Linked list of MOBIPart structures in rawml->markup holds main text files */
        MOBIPart *curr = rawml->markup;
        while (curr != nullptr) {
            MOBIFileMeta file_meta = mobi_get_filemeta_by_type(curr->type);
            snprintf(partname, sizeof(partname), "OEBPS/part%05zu.%s", curr->uid,
                     file_meta.extension);
            mz_ret = mz_zip_writer_add_mem(&zip, partname, curr->data, curr->size,
                                           (mz_uint) MZ_DEFAULT_COMPRESSION);
            if (!mz_ret) {
                LI("Could not add file to archive: %s\n", partname);
                mz_zip_writer_end(&zip);
                return false;
            }
            curr = curr->next;
        }
    }
    if (rawml->flow != nullptr) {
        /* Linked list of MOBIPart structures in rawml->flow holds supplementary text files */
        MOBIPart *curr = rawml->flow;
        /* skip raw html file */
        curr = curr->next;
        while (curr != nullptr) {
            MOBIFileMeta file_meta = mobi_get_filemeta_by_type(curr->type);
            snprintf(partname, sizeof(partname), "OEBPS/flow%05zu.%s", curr->uid,
                     file_meta.extension);
            mz_ret = mz_zip_writer_add_mem(&zip, partname, curr->data, curr->size,
                                           (mz_uint) MZ_DEFAULT_COMPRESSION);
            if (!mz_ret) {
                printf("Could not add file to archive: %s\n", partname);
                mz_zip_writer_end(&zip);
                return false;
            }
            curr = curr->next;
        }
    }
    if (rawml->resources != nullptr) {
        /* Linked list of MOBIPart structures in rawml->resources holds binary files, also opf files */
        MOBIPart *curr = rawml->resources;
        /* jpg, gif, png, bmp, font, audio, video, also opf, ncx */
        while (curr != nullptr) {
            MOBIFileMeta file_meta = mobi_get_filemeta_by_type(curr->type);
            if (curr->size > 0) {
                if (file_meta.type == T_OPF) {
                    snprintf(partname, sizeof(partname), "OEBPS/content.opf");
                } else {
                    snprintf(partname, sizeof(partname), "OEBPS/resource%05zu.%s", curr->uid,
                             file_meta.extension);
                }
                mz_ret = mz_zip_writer_add_mem(&zip, partname, curr->data, curr->size,
                                               (mz_uint) MZ_DEFAULT_COMPRESSION);
                if (!mz_ret) {
                    LI("Could not add file to archive: %s\n", partname);
                    mz_zip_writer_end(&zip);
                    return false;
                }
            }
            curr = curr->next;
        }
    }
    /* Finalize epub archive */
    mz_ret = mz_zip_writer_finalize_archive(&zip);
    if (!mz_ret) {
        LI("Could not finalize zip archive\n");
        mz_zip_writer_end(&zip);
        return false;
    }
    mz_ret = mz_zip_writer_end(&zip);
    if (!mz_ret) {
        LI("Could not finalize zip writer\n");
        return false;
    }
    LI("Create_epub: done!");
    return true;
}

static bool eramobi_process_convert(const char *src_path, int src_fd, const char *dst_path) {
    // Initialize main MOBIData structure , must be deallocated with mobi_free() when not needed
    MOBIData *m = mobi_init();
    if (m == nullptr) {
        LE("m == NULL");
        return false;
    }

    FILE *file;
    if(src_fd > 0)
    {
        file = fdopen(src_fd, "rb");
    }
    else
    {
        file = fopen(src_path, "rbe");
    }
    if (file == nullptr) {
        mobi_free(m);
        LE("file == NULL, fd: %d src_path: %s", src_fd, src_path);
        return false;
    }
    /* Load file into MOBIData structure */
    /* This structure will hold raw data/metadata from mobi document */
    MOBI_RET mobi_ret = mobi_load_file(m, file);
    fclose(file);
    if (mobi_ret != MOBI_SUCCESS) {
        LE("mobi_ret != MOBI_SUCCESS");
        mobi_free(m);
        return false;
    }
    /* Initialize MOBIRawml structure */
    /* Must be deallocated with mobi_free_rawml() when not needed */
    /* In the next step this structure will be filled with parsed data */
    MOBIRawml *rawml = mobi_init_rawml(m);
    if (rawml == nullptr) {
        LE("rawml == NULL");
        mobi_free(m);
        return false;
    }
    /* Raw data from MOBIData will be converted to html, css, fonts, media resources */
    /* Parsed data will be available in MOBIRawml structure */
    mobi_ret = mobi_parse_rawml(rawml, m);
    if (mobi_ret != MOBI_SUCCESS) {
        LE("mobi_ret != MOBI_SUCCESS");
        mobi_free(m);
        mobi_free_rawml(rawml);
        return false;
    }
#if 0
    FILE *filedump = fopen("data/data/org.readera/files/mobidump.xml", "we");
    if (filedump == nullptr) {
        mobi_free(m);
        LE("file == NULL, src_path = %s", src_path);
        return false;
    }
    mobi_dump_rawml(m, filedump);
    fclose(filedump);
#endif
    LV("EPUB creation GO");
    if (eramobi_epub_create(rawml, dst_path)) {
        LV("EPUB creation OK");
    } else {
        LE("EPUB creation ER");
    }
    mobi_free_rawml(rawml);
    mobi_free(m);
    return true;
}

void EraMobiBridge::processConvert(CmdRequest &request, CmdResponse &response) {
    response.cmd = CMD_RES_CONVERT;
    CmdDataIterator iter(request.first);
    uint32_t src_format = 0;
    uint8_t *src_path_arg = nullptr;
    uint32_t dst_format = 0;
    uint8_t *dst_path_arg = nullptr;
    uint8_t* socket_name = nullptr;

    iter.getByteArray(&socket_name)
        .getInt(&src_format)
        .getByteArray(&src_path_arg)
        .getInt(&dst_format)
        .getByteArray(&dst_path_arg);

    if (!iter.isValid() || !src_path_arg || !dst_path_arg || !socket_name) {
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (src_format != DOC_FORMAT_MOBI || dst_format != DOC_FORMAT_EPUB) {
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *src_path = reinterpret_cast<const char *>(src_path_arg);
    const char *dst_path = reinterpret_cast<const char *>(dst_path_arg);

    const bool send_fd_via_socket = ( strlen((const char*) socket_name) > 0 );

    int fd = -1;
    if (send_fd_via_socket)
    {
        StSocketConnection connection((const char*) socket_name);
        if (!connection.isValid()) {
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        bool received = connection.receiveFileDescriptor(fd);
        if (!received) {
            response.result = RES_BAD_REQ_DATA;
            return;
        }
    }

    if (!eramobi_process_convert(src_path, fd, dst_path))
    {
        response.result = RES_INTERNAL_ERROR;
    }
}
