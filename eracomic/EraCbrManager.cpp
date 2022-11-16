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
// Created by Tarasus on 22.12.2020.
//

#include "../orebridge/include/ore_log.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include "EraCbrManager.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

static const char *convert_comment(uint8_t *data, dmc_unrar_size_t size) {
    const dmc_unrar_unicode_encoding encoding = dmc_unrar_unicode_detect_encoding(data, size);
    if (encoding == DMC_UNRAR_UNICODE_ENCODING_UTF8) {
        data[size] = '\0';
        return (const char *)data;
    }

    if (encoding == DMC_UNRAR_UNICODE_ENCODING_UTF16LE) {
        char *utf8_data = NULL;

        dmc_unrar_size_t utf8_size = dmc_unrar_unicode_convert_utf16le_to_utf8(data, size, NULL, 0);
        if (utf8_size) {
            utf8_data = (char *)malloc(utf8_size);
            if (utf8_data) {
                utf8_size = dmc_unrar_unicode_convert_utf16le_to_utf8(data, size, utf8_data, utf8_size);
                if (utf8_size) {
                    free(data);
                    return utf8_data;
                }
            }
        }

        free(data);
        free(utf8_data);
        return NULL;
    }

    free(data);
    return NULL;
}

std::string get_archive_comment(dmc_unrar_archive *archive) {
    uint8_t *data = NULL;

    dmc_unrar_size_t size = dmc_unrar_get_archive_comment(archive, NULL, 0);

    if (!size)
        return std::string();

    data = (uint8_t *)malloc(size + 1);
    if (!data)
        return std::string();

    size = dmc_unrar_get_archive_comment(archive, data, size);
    if(size<=0)
    {
        return std::string();
    }

    static const char * str = convert_comment(data,size);
    if(!str)
    {
        return std::string();
    }
    LE("str= %s",str);
    return std::string(str);
}

std::string get_file_comment(dmc_unrar_archive *archive, dmc_unrar_size_t i)
{
    uint8_t *data = NULL;
    dmc_unrar_size_t size = dmc_unrar_get_file_comment(archive, i, NULL, 0);
    if (!size)
        return std::string();

    data = (uint8_t *)malloc(size + 1);
    if (!data)
        return std::string();

    size = dmc_unrar_get_file_comment(archive, i, data, size);
    if(size<=0)
    {
        return std::string();
    }
    static const char * str = convert_comment(data,size);
    return std::string(str,strlen(str));
}

std::string get_filename(dmc_unrar_archive *archive, dmc_unrar_size_t i)
{
    char *filename;
    dmc_unrar_size_t size = dmc_unrar_get_filename(archive, i, NULL, 0);
    if (!size)
        return std::string();

    filename = (char *)malloc(size);
    if (!filename)
        return std::string();

    size = dmc_unrar_get_filename(archive, i, filename, size);
    if (!size) {
        free(filename);
        return std::string();
    }

    dmc_unrar_unicode_make_valid_utf8(filename);
    if (filename[0] == '\0') {
        free(filename);
        return std::string();
    }

    return std::string(filename,strlen(filename));
}

std::string stripPath(std::string filename)
{
    if (filename.empty())
    {
        return filename;
    }
    int pos = filename.rfind('/');
    if (pos == std::string::npos)
    {
        return filename;
    }
    std::string result = filename.substr(pos+1, filename.length()-pos);
    return result;
}

void CbrManager::openDocument(std::string path, int fd)
{
    dmc_unrar_return err = dmc_unrar_archive_init(&arc);
    if (err != DMC_UNRAR_OK)
    {
        LE("CbrManager::openDocument Archive init failed: %s", dmc_unrar_strerror(err));
        return;
    }

    if(fd>0)
    {
        //LE("CbrManager openDocument read via fd (%d)",fd);
        struct stat stat;
        if (fstat(fd, &stat) == -1)
        {
            LE("CbrManager::openDocument failed fstat %d : %s",errno, strerror(errno));
            return;
        }

        size_t size = (int) stat.st_size;
        auto mem = (unsigned  char*)mmap( 0, size, PROT_READ, MAP_SHARED, fd, 0 );
        if ( mem == MAP_FAILED )
        {
            LE("CbrManager::openDocument mmap failed %d : %s",errno, strerror(errno));
            return;
        }

        err = dmc_unrar_archive_open_mem(&arc, mem, size);
    }
    else
    {
        err = dmc_unrar_archive_open_path(&arc, path.c_str());
    }
    if (err != DMC_UNRAR_OK)
    {
        LE("FAILED TO OPEN RAR [%s] : %s",path.c_str(),dmc_unrar_strerror(err));
        return;
    }
    arc_path = path;

    //arc_comment = get_archive_comment(&arc);
    //if(!arc_comment.empty())
    //{
    //    LE(" Archive comment: %s", arc_comment.c_str());
    //}

    int dmc_filecount = dmc_unrar_get_file_count(&arc);

    std::vector<std::pair<std::string,int>> filenames;
    pagecount = 0;
    for (int i = 0; i < dmc_filecount; i++)
    {
        files.push_back(NULL);
        std::string name = get_filename(&arc, i);
        if(validateName(name) != FORMAT_UNKNOWN)
        {
            filenames.push_back(std::make_pair(name,i));
            pagecount++;
        }
    }
    std::sort(filenames.begin(),filenames.end(),StrComparator);
    for (int i = 0; i < filenames.size(); i++)
    {
        //LE(" %d file is [%s] = %d",i,filenames.at(i).first.c_str(),filenames.at(i).second);
        indexmap.insert(std::make_pair(i, filenames.at(i).second));
        toc.emplace_back(i,filenames.at(i).first);
    }
    //LE("pagecount = %d", pagecount);
}

void CbrManager::closeDocument()
{
    dmc_unrar_archive_close(&arc);

    freeAllPages();
}

bool CbrManager::getPageInfo(uint32_t index, int *w, int *h)
{
    //LE("CBR: getPageInfo START");
    std::string filename = get_filename(&arc, index);

    const dmc_unrar_file *file = dmc_unrar_get_file_stat(&arc, index);

    if (!file)
    {
        LE("COULD NOT OPEN FILE [%d] [%s]", index, filename.c_str());
        return false;
    }


    if (dmc_unrar_file_is_directory(&arc, index))
    {
        LE("FAIL dmc_unrar_file_is_directory = true");
        return false;
    }

    dmc_unrar_return supported = dmc_unrar_file_is_supported(&arc, index);
    if (supported != DMC_UNRAR_OK)
    {
        LE("Not supported: %s", dmc_unrar_strerror(supported));
        return false;
    }


    unsigned char *fileBuf;
    bool result = false;
    const int max = 5;
    const unsigned long long maxSize = file->uncompressed_size;

    unsigned long long sizes[max] = {1024,32768,65536,131072,maxSize};
    int iterator = 0;
    for (int i = 0; i < max; i++ )
    {
        unsigned long long fileSize = (sizes[i] > maxSize)? maxSize: sizes[i];
        fileBuf = (unsigned char *) malloc(fileSize);
        if (fileBuf == NULL)
        {
            LE("unable to malloc [%llu] bytes ", fileSize);
            return false;
        }
        dmc_unrar_return extracted = dmc_unrar_extract_file_to_mem(&arc, index, fileBuf, fileSize, NULL, false);
        if (extracted != DMC_UNRAR_OK)
        {
            LE("Error: %d %s [%s]", extracted, dmc_unrar_strerror(extracted), filename.c_str());
            free(fileBuf);
            return false;
        }
        result = getImageInfo(filename, fileBuf, fileSize, w, h);

        free(fileBuf);
        if(result || fileSize == maxSize)
        {
            //LI("Header extracted at %llu size for file [%s]",fileSize,filename.c_str());
            break;
        }
    }
    //LE("CBR: getPageInfo END");
    return result;
}

ComicFile* CbrManager::loadFile(uint32_t index)
{
    std::string filename = get_filename(&arc, index);
    std::string file_comment = get_file_comment(&arc, index);

    const dmc_unrar_file *file = dmc_unrar_get_file_stat(&arc, index);

    if (!file)
    {
        LE("COULD NOT OPEN FILE [%d] [%s]", index, filename.c_str());
        return NULL;
    }

    std::string stripped = stripPath(filename);

    if (dmc_unrar_file_is_directory(&arc, index))
    {
        LE("FAIL dmc_unrar_file_is_directory = true");
        return NULL;
    }

    dmc_unrar_return supported = dmc_unrar_file_is_supported(&arc, index);
    if (supported != DMC_UNRAR_OK)
    {
        LE("Not supported: %s", dmc_unrar_strerror(supported));
        return NULL;
    }

    auto *fileBuf = (unsigned char*)malloc(file->uncompressed_size);
    if (fileBuf == NULL)
    {
        LE("unable to malloc [%llu] bytes ", file->uncompressed_size);
        return NULL;
    }
    unsigned long fileSize = file->uncompressed_size;

    dmc_unrar_return extracted = dmc_unrar_extract_file_to_mem(&arc, index, fileBuf, file->uncompressed_size, NULL, false);
    if (extracted != DMC_UNRAR_OK)
    {
        LE("Error: %d %s [%s]", extracted, dmc_unrar_strerror(extracted), filename.c_str());
        free(fileBuf);
        return NULL;
    }

    auto *cbrFile = new ComicFile(index, filename, file_comment);

    imgFormat bin_format  = ComicManager::checkImageFormat(fileBuf);

#ifdef OREDEBUG
    imgFormat name_format = ComicManager::validateName(filename);
    if(name_format!=bin_format)
    {
        LE("CbrManager name_format is %d but file signature tells %d",name_format,bin_format);
    }
#endif
    if(bin_format == FORMAT_UNKNOWN)
    {
        LE("Failed extracting file format signature [%s]",filename.c_str());
        return NULL;
    }
    cbrFile->format = bin_format;

    if(!decodeImageBuf(fileBuf,fileSize,cbrFile))
    {
        LE("Failed decoding file [%s]",filename.c_str());
        return NULL;
    }
    //LE("CBR: Success decoding file %d [%s]",index,filename.c_str());
    files[index] = cbrFile;
    return cbrFile;
}