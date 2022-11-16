/*
 * Copyright (C) 2013-2021 READERA LLC
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
 * Developers: ReadEra Team (2013-2021), Playful Curiosity (2013-2021),
 * Tarasus (2018-2021).
 */
//
// Created by Tarasus on 4/1/2021.
//

#ifndef CODE_READERA_TARASUS_ERACOMICMANAGER_H
#define CODE_READERA_TARASUS_ERACOMICMANAGER_H

#include <map>
#include <vector>
#include <string>
#include "../../orebridge/include/ore_log.h"

enum imgFormat
{
    FORMAT_UNKNOWN = 0,
    FORMAT_JPG,
    FORMAT_PNG,
    FORMAT_BMP,
    FORMAT_WEBP
};

class ComicFile;

class ComicManager
{
public:
    int archive_format = 0;

    bool config_invert_images = false;

    std::string arc_path;
    std::string arc_comment;

    std::vector<ComicFile*> files;
    int pagecount;
    std::map<int, int> indexmap;
    std::vector<std::pair<int,std::string>> toc;

    virtual void openDocument(std::string path, int fd) = 0;
    virtual void closeDocument() = 0;
    virtual ComicFile * loadFile(uint32_t index) = 0;
    virtual bool getPageInfo(uint32_t index, int *w, int *h) = 0;

    ComicFile* getPage(uint32_t raw_index);
    bool freeFile(uint32_t index);
    static imgFormat validateName(std::string name);
    bool freeAllPages();

    bool getImageInfo(std::string filename, unsigned char *inBuf, unsigned long inSize, int *w, int *h);
    bool getJpgInfo(unsigned char *inBuf, unsigned long inSize, int *w, int *h);
    bool getPngInfo(unsigned char *inBuf, unsigned long inSize, int *w, int *h);
    bool getBmpInfo(unsigned char *inBuf, unsigned long inSize, int *w, int *h);
    bool getWebpInfo(unsigned char *inBuf, unsigned long inSize, int *w, int *h);

    bool decodeImageBuf(unsigned char *inBuf, unsigned long inSize, ComicFile *comicFile);
    bool decodeJpgBuf(unsigned char *inBuf, unsigned long inSize, ComicFile *comicFile);
    bool decodePngBuf(unsigned char *inBuf, unsigned long inSize, ComicFile *comicFile);
    bool decodeBmpBuf(unsigned char *inBuf, unsigned long inSize, ComicFile *comicFile);
    bool decodeWebpBuf(unsigned char *inBuf, unsigned long inSize, ComicFile *comicFile);

    imgFormat checkImageFormat(unsigned char *buf);
};

class ComicFile
{
private:

public:
    ComicFile(int pos, std::string name, std::string comment) :
            pos(pos),
            name(name),
            comment(comment)
    {}

    int pos;
    std::string name;
    std::string comment;

    unsigned char* page_buf;
    int page_buf_size = 0;
    int orig_width = 0;
    int orig_height = 0;

    imgFormat format = FORMAT_UNKNOWN;


    bool alloc_page_buf(int size) {
        page_buf = (unsigned char*)malloc(size);
        if(page_buf == NULL)
        {
            return false;
        }
        page_buf_size = size;
        return true;
    };

    void free_page_buf()
    {
        if(page_buf && page_buf_size >= 0)
        {
            page_buf_size = 0;
            free(page_buf);
        }
    }
};

bool StrComparator(std::pair<std::string,int> &x, std::pair<std::string,int> &y);

#endif //CODE_READERA_TARASUS_ERACOMICMANAGER_H
