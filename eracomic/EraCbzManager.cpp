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
// Created by Tarasus on 5/1/2021.
//

#include <sys/stat.h>
#include <sys/mman.h>
#include "include/EraCbzManager.h"
#include "../orebridge/include/ore_log.h"
#include <errno.h>

void CbzManager::selectFileByIndex(int index)
{
    if(index > entryCount)
    {
        LE("FAILED TO SELECT %d ENTRY: (%d > %d)", index, index, entryCount);
        return;
    }
    int err = unzGoToFirstFile(arc);
    if(err!= UNZ_OK)
    {
        LE("FAILED TO GO TO FIRST FILE");
        return;
    }
    entryIndex = 0;

    for (int i = 0; i < index; i++)
    {
        err = unzGoToNextFile(arc);
        if (err != UNZ_OK)
        {
            LE("selectFileByIndex: error %d with zipfile in unzGoToNextFile", err);
            break;
        }
        entryIndex++;
    }
}

bool CbzManager::getPageInfo(uint32_t index, int *w, int *h)
{
    //LE("CBZ: getPageInfo START");
    selectFileByIndex(index);

    int err;
    unz_file_info file_info;

    char filename[256];

    err = unzGetCurrentFileInfo(arc,&file_info,filename,256,NULL,0,NULL,0);
    if (err!=UNZ_OK)
    {
        LE("getPageInfo: error %d with zipfile in unzGetCurrentFileInfo",err);
        return false;
    }
    //LE("file [%s]",filename);

    unsigned char *fileBuf;
    bool result = false;
    const int max = 5;
    const unsigned long long maxSize = file_info.uncompressed_size;

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
        err = unzOpenCurrentFile(arc);
        if (err != UNZ_OK)
        {
            LE("getPageInfo: error %d with zipfile in unzOpenCurrentFile",err);
            free(fileBuf);
            return false;
        }
        err = unzReadCurrentFile(arc, fileBuf, fileSize);
        if (err<0)
        {
            LE("getPageInfo: error %d with zipfile in unzReadCurrentFile 1",err);
            free(fileBuf);
            return false;
        }
        err = unzCloseCurrentFile(arc);
        if (err != UNZ_OK)
        {
            LE("getPageInfo: error %d with zipfile in unzCloseCurrentFile",err);
            free(fileBuf);
            return false;
        }
        result = getImageInfo(filename, fileBuf, fileSize, w, h);

        free(fileBuf);
        if(result || fileSize == maxSize)
        {
            //LI("Header extracted at %llu size for file [%s]",fileSize,filename);
            break;
        }
    }
    //LE("CBZ: getPageInfo END");
    return result;
}


void CbzManager::openDocument(std::string path, int fd)
{

    if(fd>0)
    {
        //LE("CbzManager openDocument read via fd (%d)",fd);
        struct stat stat;
        if (fstat(fd, &stat) == -1)
        {
            LE("CbzManager::openDocument failed fstat %d : %s",errno, strerror(errno));
            return;
        }

        size_t size = (int) stat.st_size;
        auto mem = (char*)mmap( 0, size, PROT_READ, MAP_SHARED, fd, 0 );
        if ( mem == MAP_FAILED )
        {
            LE("CbzManager::openDocument mmap failed %d : %s",errno, strerror(errno));
            return;
        }
        arc = unzOpenBuffer(mem,size);
        if(arc == NULL)
        {
            LE("FAILED TO OPEN ZIP VIA FD [%d] [0x%X]",fd,arc);
            return;
        }
    }
    else
    {
        arc = unzOpen(path.c_str());
        if (arc == NULL)
        {
            std::string new_path = path + ".zip";
            arc = unzOpen(new_path.c_str());
        }
    }


    if (arc == NULL)
    {
        LE("FAILED TO OPEN ZIP [%s]",path.c_str());
        return;
    }
    arc_path = path;

    unz_global_info gi;
    int err;

    err = unzGetGlobalInfo (arc,&gi);
    if (err!=UNZ_OK)
    {
        LE("error %d with zipfile in unzGetGlobalInfo", err);
    }

    char * arccomment_buf = (char*) malloc(gi.size_comment);
    int bytecount = unzGetGlobalComment(arc, arccomment_buf, gi.size_comment);
    if (bytecount < 0)
    {
        LE("error retrieving global comment");
    }
    arc_comment = arccomment_buf;
    free(arccomment_buf);
    if(!arc_comment.empty())
    {
        LE(" Archive comment: %s", arc_comment.c_str());
    }

    std::vector<std::pair<std::string,int>> filenames;
    pagecount = 0;
    entryCount = gi.number_entry;
    for (int i = 0; i < gi.number_entry; i++)
    {
        files.push_back(NULL);
        unz_file_info file_info;

        err = unzGetCurrentFileInfo(arc,&file_info,NULL,0,NULL,0,NULL,0);
        if (err!=UNZ_OK)
        {
            LE("error %d with zipfile in unzGetCurrentFileInfo",err);
            continue;
        }
        auto filename_inzip = (char*) malloc(file_info.size_filename);
        err = unzGetCurrentFileInfo(arc,&file_info,filename_inzip,file_info.size_filename,NULL,0,NULL,0);
        if (err!=UNZ_OK)
        {
            LE("error %d with zipfile in unzGetCurrentFileInfo",err);
            continue;
        }

        std::string name = std::string(filename_inzip,file_info.size_filename);
        free(filename_inzip);

        if(validateName(name) != FORMAT_UNKNOWN)
        {
            filenames.push_back(std::make_pair(name,i));
            pagecount++;
        }
        //LE("validate name [%s] = %d",name.c_str(),validateName(name));

        if ((i + 1) < gi.number_entry)
        {
            err = unzGoToNextFile(arc);
            if (err != UNZ_OK)
            {
                LE("error %d with zipfile in unzGoToNextFile", err);
                break;
            }
        }
    }

    std::sort(filenames.begin(),filenames.end(),StrComparator);
    for (int i = 0; i < filenames.size(); i++)
    {
        //LE(" %d file is entry %d [%s]",i,filenames.at(i).second,filenames.at(i).first.c_str());
        indexmap.insert(std::make_pair(i, filenames.at(i).second));
        toc.emplace_back(i ,filenames.at(i).first);
    }
    //LE("pagecount = %d", pagecount);
}

void CbzManager::closeDocument()
{
    unzClose(arc);
    freeAllPages();
}

ComicFile* CbzManager::loadFile(uint32_t index)
{
    selectFileByIndex(index);

    int err;
    unz_file_info file_info;

    char filename[256];
    char file_comment[10] = "";
    err = unzGetCurrentFileInfo(arc,&file_info,filename,256,NULL,0,NULL,0);
    if (err!=UNZ_OK)
    {
        LE("loadFile: error %d with zipfile in unzGetCurrentFileInfo",err);
        return NULL;
    }
    //LE("file [%s]",filename);

    unsigned long fileSize = file_info.uncompressed_size;
    auto fileBuf = (unsigned char* )malloc(fileSize);
    if(fileBuf == NULL)
    {
        LE("loadFile: Failed to allocate %lu bytes", fileSize);
        return NULL;
    }

    err = unzOpenCurrentFile(arc);
    if (err != UNZ_OK)
    {
        LE("loadFile: error %d with zipfile in unzOpenCurrentFile",err);
        return NULL;
    }

    err = unzReadCurrentFile(arc, fileBuf, fileSize);
    if (err<0)
    {
        LE("loadFile: error %d with zipfile in unzReadCurrentFile",err);
        return NULL;
    }
    //LE("read %d bytes",err);

    err = unzCloseCurrentFile(arc);
    if (err != UNZ_OK)
    {
        LE("loadFile: error %d with zipfile in unzCloseCurrentFile",err);
        return NULL;
    }

    auto *cbzFile = new ComicFile(index, filename, file_comment);
    imgFormat bin_format  = ComicManager::checkImageFormat(fileBuf);

#ifdef OREDEBUG
    imgFormat name_format = ComicManager::validateName(filename);
    if(name_format!=bin_format)
    {
        LE("CbzManager name_format is %d but file signature tells %d",name_format,bin_format);
    }
#endif
    if(bin_format == FORMAT_UNKNOWN)
    {
        LE("Failed extracting file format signature [%s]",filename);
        return NULL;
    }
    cbzFile->format = bin_format;

    if(!decodeImageBuf(fileBuf,fileSize,cbzFile))
    {
        LE("Failed to decode file %d [%s]",index,filename);
        return NULL;
    }
    //LE("CBZ: Success decoding file %d [%s]",index,filename);
    files[index] = cbzFile;
    return cbzFile;
}