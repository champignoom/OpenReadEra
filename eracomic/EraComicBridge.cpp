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

#include <math.h>
#include "include/EraComicBridge.h"
#include "../orecrop/include/smartcrop.h"
#include "include/EraCbrManager.h"
#include "include/EraCbzManager.h"
#include "include/EraComicRarUtils.h"
#include <unistd.h>

// Should go as last include to not trigger rebuild of other files on changes
#include "../orebridge/include/openreadera_version.h"
#include "../orebridge/include/StSocket.h"

EraComicBridge::EraComicBridge() : StBridge("EraComicBridge")
{
    format = 0;
    comicManager = new CbrManager(); // temporary holder
}

EraComicBridge::~EraComicBridge()
{
    free(comicManager);
}

void EraComicBridge::process(CmdRequest &request, CmdResponse &response)
{
    response.reset();
    request.print("EraPdfBridge");
    switch (request.cmd)
    {
        case CMD_REQ_VERSION:
            OreVerResporse(OPENREADERA_BASE_VERSION, response);
            break;
        case CMD_REQ_SET_CONFIG:
            processConfig(request, response);
            break;
        case CMD_REQ_OPEN:
            processOpen(request, response);
            break;
        case CMD_REQ_QUIT:
            processQuit(request, response);
            break;
        case CMD_REQ_PAGE:
            processPage(request, response);
            break;
        case CMD_REQ_PAGE_FREE:
            processPageFree(request, response);
            break;
        case CMD_REQ_PAGE_RENDER:
            processPageRender(request, response);
            break;
        case CMD_REQ_PAGE_INFO:
            processPageInfo(request, response);
            break;
        case CMD_REQ_SMART_CROP:
            processSmartCrop(request, response);
            break;
        case CMD_REQ_OUTLINE: //stub
            processOutline(request, response);
            break;
        case CMD_REQ_LINKS: //stub
            processPageLinks(request, response);
            break;
        case CMD_REQ_PAGE_TEXT: //stub
            processPageText(request, response);
            break;
        case CMD_REQ_SEARCH_COUNTER: //stub
            processSearchCounter(request, response);
            break;
        case CMD_REQ_SEARCH_PREVIEWS: //stub
            processTextSearchPreviews(request, response);
            break;
        case CMD_REQ_CRE_IMG_HITBOXES:
            processImageHitbox(request, response);
            break;
        case CMD_REQ_CRE_IMG_BLOB:
            processImageByXpath(request, response);
            break;
        case CMD_REQ_COMIC_RAR_INFO:
            processRarInfo(request,response);
            break;
        case CMD_REQ_COMIC_RAR_EXTRACT:
            processRarExtract(request,response);
            break;
        default:
            LE("Unknown request: %d", request.cmd);
            response.result = RES_UNKNOWN_CMD;
            break;
    }
    response.print("EraPdfBridge");
}

void EraComicBridge::processOpen(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_OPEN;
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t doc_format = 0;
    uint8_t* socket_name = nullptr;
    uint8_t* file_name = nullptr;
    uint32_t direct_archive = 0;
    uint8_t* password = nullptr;
    CmdDataIterator iter(request.first);
    iter.getByteArray(&socket_name)
         .getInt(&doc_format)
         .getByteArray(&file_name)
         .getInt(&direct_archive)
         .optionalByteArray(&password, nullptr);
    if (!iter.isValid()) {
        LE("Bad request data: %u %u %u %p %p",
           request.cmd, request.dataCount, iter.getCount(), socket_name, password);
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    bool invert_images = comicManager->config_invert_images; // remember old state
    delete (CbrManager*)comicManager; //deleting temporary comicManager

    const bool send_fd_via_socket = ( strlen((const char*) socket_name) > 0 );



    int doc_format_check;
    std::string path = std::string(reinterpret_cast<const char*>(file_name));
    int fd = -1;

    if (send_fd_via_socket)
    {
        StSocketConnection connection((const char *) socket_name);
        if (!connection.isValid())
        {
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        bool received = connection.receiveFileDescriptor(fd);
        if (!received)
        {
            response.result = RES_BAD_REQ_DATA;
            return;
        }

        doc_format_check = checkArchiveFormat("", fd);
    }
    else
    {
        doc_format_check = checkArchiveFormat(path, -1);
    }

    if(doc_format_check == DOC_FORMAT_NULL)
    {
        //LE("UNSUPPORTED ARCHIVE [%s]",path.c_str());
        //return;
        LE("doc_format_check == DOC_FORMAT_NULL [%s]",path.c_str());
        //signature check failed, but we still can try opening it as doc_format first suggested
        doc_format_check = doc_format;
    }

    if(doc_format != doc_format_check)
    {
        LE("DOC FORMAT IS WRONG: given %d, but file is %d",doc_format,doc_format_check);
        doc_format = doc_format_check;
    }

    if(doc_format == DOC_FORMAT_CBR)
    {
        comicManager = new CbrManager();
    }
    else if (doc_format == DOC_FORMAT_CBZ)
    {
        comicManager = new CbzManager();
    }
    else
    {
        LE("Unknown file format %d",doc_format);
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    comicManager->config_invert_images = invert_images;
    comicManager->archive_format = doc_format;
    if (send_fd_via_socket)
    {
        //std::string newpath = "/proc/self/fd/";
        //newpath.append(std::to_string(fd));
        LE("fd = %d", fd);
        comicManager->openDocument("", fd);
    }
    else
    {
        comicManager->openDocument(path, -1);
    }

    response.addInt(comicManager->pagecount);
}

void EraComicBridge::processQuit(CmdRequest &request, CmdResponse &response)
{
    LE("process close");
    comicManager->closeDocument();
    response.cmd = CMD_RES_QUIT;
}

void EraComicBridge::processPage(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE;
    if (comicManager->pagecount == 0) {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t raw_page_index = 0;
    uint32_t preview = 0;
    CmdDataIterator iter(request.first);
    if (!iter.getInt(&raw_page_index).getInt(&preview).isValid()) {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    auto it = comicManager->indexmap.find(raw_page_index);
    if (it == comicManager->indexmap.end()) {
        LE("No page %d found", raw_page_index);
        response.result = RES_INTERNAL_ERROR;
        return;
    }
    int page_index = it->second;

    ComicFile *p = comicManager->loadFile(page_index);
    if (p == NULL) {
        response.result = RES_INTERNAL_ERROR;
        return;
    }
    response.addInt(0);
}

void EraComicBridge::processPageFree(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE_FREE;
    if (comicManager->pagecount == 0) {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t raw_page_index = 0;
    CmdDataIterator iter(request.first);
    if (!iter.getInt(&raw_page_index).isValid()) {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (raw_page_index >= comicManager->pagecount) {
        LE("Bad page index: %d", raw_page_index);
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    auto it = comicManager->indexmap.find(raw_page_index);
    if (it == comicManager->indexmap.end()) {
        LE("No page %d found", raw_page_index);
        response.result = RES_INTERNAL_ERROR;
        return;
    }
    comicManager->freeFile(it->second);
    LI("page %d (raw %d) released",it->second,raw_page_index);
}

void EraComicBridge::processPageInfo(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_PAGE_INFO;
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t raw_page_index = 0;
    if (!CmdDataIterator(request.first).getInt(&raw_page_index).isValid()) {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    int w = 0;
    int h = 0;

    auto it = comicManager->indexmap.find(raw_page_index);
    if (it == comicManager->indexmap.end()) {
        LE("No page %d found", raw_page_index);
        response.result = RES_INTERNAL_ERROR;
        return;
    }
    int page_index = it->second;

    if(!comicManager->getPageInfo(page_index, &w, &h))
    {
        LE("failed to get page info for page %d (raw %d)",page_index,raw_page_index);
        response.result = RES_INTERNAL_ERROR;
        return;
    }
    //LE("processPageInfo w = %d, h = %d",w,h);
    float data[2];

    data[0] = fabs(w);
    data[1] = fabs(h);
    response.addFloatArray(2, data, true);
}

void EraComicBridge::processConfig(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_SET_CONFIG;
    CmdDataIterator iter(request.first);
    bool invert_old = comicManager->config_invert_images;
    while (iter.hasNext()) {
        uint32_t key;
        uint8_t* temp_val;
        iter.getInt(&key).getByteArray(&temp_val);
        if (!iter.isValid()) {
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        const char* val = reinterpret_cast<const char*>(temp_val);
        if (key == CONFIG_MUPDF_INVERT_IMAGES) {
            comicManager->config_invert_images = (atoi(val) == 3);
        } else {
            LE("processConfig unknown key: key=%d, val=%s", key, val);
        }
    }
    response.addInt(comicManager->pagecount);
    if(invert_old != comicManager->config_invert_images)
    {
        if (!comicManager->freeAllPages())
        {
            LE("processConfig FAILED TO FREE ALL FILES!");
        }
    }
}

void EraComicBridge::processSmartCrop(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_SMART_CROP;
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (comicManager->pagecount == 0) {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }

    uint32_t page_index;
    float origin_w;
    float origin_h;
    float slice_l;
    float slice_t;
    float slice_r;
    float slice_b;
    CmdDataIterator iter(request.first);
    iter.getInt(&page_index)
    .getFloat(&origin_w)
    .getFloat(&origin_h)
    .getFloat(&slice_l)
    .getFloat(&slice_t)
    .getFloat(&slice_r)
    .getFloat(&slice_b);
    if (!iter.isValid()) {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    matrix ctm = default_matrix;

    ctm.a = slice_l;
    ctm.b = slice_t;
    ctm.c = slice_r;
    ctm.d = slice_b;

    float slice_w = slice_r - slice_l;
    float slice_h = slice_b - slice_t;

    float lambda = origin_h / origin_w;
    int smart_S = smart_crop_w * smart_crop_h;

    int new_w = sqrt(smart_S / lambda);
    int new_h = new_w * lambda;

    int targetRect_w = new_w;
    int targetRect_h = new_h;

    int size = targetRect_w * targetRect_h * 4;
    auto pixels = (unsigned char*) malloc(size);
    bool result = renderPage(page_index, targetRect_w, targetRect_h, pixels, ctm, false);
    if (!result)
    {
        response.result = RES_INTERNAL_ERROR;
        return;
    }

    float smart_crop[4] = {0,0,1,1};
    CalcBitmapSmartCrop(smart_crop, (uint8_t *) pixels, targetRect_w, targetRect_h, slice_l, slice_t, slice_r, slice_b);
    response.addFloatArray(4, smart_crop, true);
    if (pixels) { free(pixels); }
}

void EraComicBridge::processPageRender(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_PAGE_RENDER;
    //if (document == nullptr || pages == nullptr) {
    //    LE("Document not yet opened");
    //    response.result = RES_ILLEGAL_STATE;
    //    return;
    //}
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t page_index;
    uint32_t w;
    uint32_t h;
    float* matrix;
    uint32_t preview;
    CmdDataIterator iter(request.first);
    iter.getInt(&page_index).getInt(&w).getInt(&h);
    iter.getFloatArray(&matrix, RENDER_MATRIX_SIZE).getInt(&preview);
    if (!iter.isValid()) {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    matrix_s transform_matrix = default_matrix;
    transform_matrix.a = matrix[0];
    transform_matrix.b = matrix[1];
    transform_matrix.c = matrix[2];
    transform_matrix.d = matrix[3];
    transform_matrix.e = matrix[4];
    transform_matrix.f = matrix[5];

    //LE("ctm a = %f", transform_matrix.a);
    //LE("ctm b = %f", transform_matrix.b);
    //LE("ctm c = %f", transform_matrix.c);
    //LE("ctm d = %f", transform_matrix.d);

    auto pixelsHolder = new CmdData();
    uint8_t *pixels = pixelsHolder->newByteArray((w) * (h) * 4);

    LI("renderPage %d [%d x %d] preview = %d",page_index,w,h,preview);
    int size = 0;
    if (renderPage(page_index, w, h, pixels, transform_matrix, preview))
    {
        response.addData(pixelsHolder);
    }
    else
    {
        response.result = RES_INTERNAL_ERROR;
        delete pixelsHolder;
    }
}

void EraComicBridge::processOutline(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_OUTLINE;
    response.addInt((uint32_t) 0);

    const char EMPTY_TITLE[1] = { 0x0 };

    for (int i = 0; i < comicManager->toc.size(); i++)
    {
        auto item = comicManager->toc.at(i);
        std::string title = item.second;
        //LE("title = %d %s",i,title.c_str());
        response.addWords((uint16_t) 1, (uint16_t) item.first);
        response.addInt((uint32_t) 0);
        response.addIpcString(title.empty() ? EMPTY_TITLE : title.c_str(), true);
        response.addFloat(.0f).addFloat(.0f);
    }
}

void EraComicBridge::processPageLinks(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_LINKS;
}

void EraComicBridge::processPageText(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_PAGE_TEXT;
}

void EraComicBridge::processTextSearchPreviews(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_SEARCH_PREVIEWS;
}

void EraComicBridge::processSearchCounter(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_SEARCH_COUNTER;
    response.addInt(0);
}

bool getFirstBytes(std::string path, int bytesToRead, char* buffer)
{
    FILE *archive = fopen(path.c_str(), "rb");
    if (archive == NULL)
    {
        fclose(archive);
        return false;
    }

    size_t read = fread(buffer, 1, bytesToRead, archive);

    fclose(archive);
    return (read == bytesToRead);
}

bool getFirstBytes(int fd, int bytesToRead, char* buffer)
{
    FILE *archive = fdopen(fd, "rb");
    if (archive == NULL)
    {
        //fclose(archive);
        return false;
    }

    //size_t read = fread(buffer, 1, bytesToRead, archive);
    size_t readnum = read(fd, buffer, bytesToRead);

    //fclose(archive);
    return (readnum == bytesToRead);
}

int EraComicBridge::checkArchiveFormat(std::string path, int fd)
{
    const int bytesToRead = 8;
    auto buf = (char *) malloc(bytesToRead);
    if (buf == NULL)
    {
        return false;
    }

    if(fd>0)
    {
        if(!getFirstBytes(fd,bytesToRead,buf))
        {
            return DOC_FORMAT_NULL;
        }
    }
    else
    {
        if(!getFirstBytes(path,bytesToRead,buf))
        {
            return DOC_FORMAT_NULL;
        }
    }

    const unsigned char rar1[8] = {0x52,0x61,0x72,0x21,0x1A,0x07,0x01,0x00};
    const unsigned char rar2[7] = {0x52,0x61,0x72,0x21,0x1A,0x07,0x00};
    const unsigned char zip1[4] = {0x50,0x4B,0x03,0x04};
    const unsigned char zip2[4] = {0x50,0x4B,0x05,0x06};
    const unsigned char zip3[4] = {0x50,0x4B,0x07,0x08};

    if(strncmp(buf,(char*)rar1,8) == 0) return DOC_FORMAT_CBR;
    if(strncmp(buf,(char*)rar2,7) == 0) return DOC_FORMAT_CBR;
    if(strncmp(buf,(char*)zip1,4) == 0) return DOC_FORMAT_CBZ;
    if(strncmp(buf,(char*)zip2,4) == 0) return DOC_FORMAT_CBZ;
    if(strncmp(buf,(char*)zip3,4) == 0) return DOC_FORMAT_CBZ;

    char log[100];
    sprintf(log,"0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
    LE("unknown archive signature: [%s]",log);

    return DOC_FORMAT_NULL;
}

void EraComicBridge::processImageHitbox(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_CRE_IMG_HITBOXES;
    /*
    CmdDataIterator iter(request.first);
    uint32_t external_page = 0;
    iter.getInt(&external_page);
    if (!iter.isValid())
    {
        LE("processImageHitbox bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    std::string pagestr = std::to_string(external_page);
    response.addFloat(0);
    response.addFloat(0);
    response.addFloat(1);
    response.addFloat(1);
    response.addIpcString(pagestr.c_str(),true);
    */
}

void EraComicBridge::processImageByXpath(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_CRE_IMG_BLOB;
    /*
    CmdDataIterator iter(request.first);
    uint8_t *page_str;

    iter.getByteArray(&page_str);
    if (!iter.isValid())
    {
        LE("processImageByXpath invalid iterator");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    std::string page_string(reinterpret_cast<const char *>(page_str));

    int page_index = std::strtol(page_string.c_str(),NULL,10);
    //LE("processImageByXpath raw page index = %d",page_index);

    if (page_index < 0 || page_index > comicManager->pagecount)
    {
        LE("processImageByXpath bad page");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    ComicFile *file = comicManager->getPage(page_index);

    if(file->orig_width < 0 || file->orig_height < 0)
    {
        LE("w or h < 0");
        response.result = RES_INTERNAL_ERROR;
        return;
    }

    auto imgData = new CmdData();
    imgData->type = TYPE_ARRAY_POINTER;

    const int size = file->orig_width * file->orig_height * 4;
    unsigned char *pixels = imgData->newByteArray(size);
    if(pixels == NULL)
    {
        LE("Failed allocating %d bytes for image",size);
        response.result = RES_INTERNAL_ERROR;
        return;
    }

    memcpy(pixels,file->page_buf, size);

    const int w = file->orig_width;
    const int h = file->orig_height;
    const int RGBA = 4;

    int r = 0;
    int g = 0;
    int b = 0;
    int a = 0;

    for (int y = 0; y < h; y++)
    {
        bool row_white = true;
        for (int x = 0; x < w; x++)
        {
            if(!row_white)
            {
                break;
            }
            r = pixels[(w * y * RGBA) + (x * RGBA) + 0 ];
            g = pixels[(w * y * RGBA) + (x * RGBA) + 1 ];
            b = pixels[(w * y * RGBA) + (x * RGBA) + 2 ];
            //a = pixels[(w * y * RGBA) + (x * RGBA) + 3 ] = 0x0; //alpha channel, ignore

            if(r + g + b < 0xFF+0xFF)
            {
                row_white = false;
            }

            //for (int k = 0; k < RGBA; k++)
            //{
            //}
        }
        if(row_white)
        {
            for (int x = 0; x < w; x++)
            {
                pixels[(w * y * RGBA) + (x * RGBA) + 0] = 0x0;
                pixels[(w * y * RGBA) + (x * RGBA) + 1] = 0xFF;
                pixels[(w * y * RGBA) + (x * RGBA) + 2] = 0x0;
            }
        }
    }


    //if(comicManager->config_invert_images)
    //{
    //    //invert image back
    //    for (int i = 0; i < size; i++)
    //    {
    //        pixels[i] = 0xFF - pixels[i];
    //    }
    //}

    response.addData(imgData);
    response.addInt((uint32_t) file->orig_width);
    response.addInt((uint32_t) file->orig_height);
    */
}

void EraComicBridge::processRarInfo(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_COMIC_RAR_INFO;
    if (request.dataCount == 0) {
        LE("EraComicBridge::processRarInfo: No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint8_t* file_name = nullptr;
    uint8_t* socket_name = nullptr;
    CmdDataIterator iter(request.first);
    iter.getByteArray(&socket_name)
        .getByteArray(&file_name);
    if (!iter.isValid() || !socket_name) {
        LE("EraComicBridge::processRarInfo: Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    //std::string path = "/sdcard/download/archive.rar";
    std::string arc_path = std::string(reinterpret_cast<const char*>(file_name));
    //LE("socket name = [%s]",reinterpret_cast<const char*>(socket_name));
    //LE("arcpath     = [%s]",arc_path.c_str());

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

    if(checkArchiveFormat(arc_path,fd) != DOC_FORMAT_CBR)
    {
        LE("EraComicBridge::processRarInfo: UNSUPPORTED ARCHIVE format [%s]",arc_path.c_str());
        //return;
        //signature check failed, but we still can try opening it
    }

    std::vector<std::string> entries = getRarEntries(arc_path, fd);
    for (int i = 0; i < entries.size(); i++)
    {
        //LE("entry %d = [%s]",i,entries.at(i).c_str());
        response.addIpcString(entries.at(i).c_str(),true);
    }
}

void EraComicBridge::processRarExtract(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_COMIC_RAR_EXTRACT;
    if (request.dataCount == 0)
    {
        LE("EraComicBridge::processRarExtract No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint8_t *socket_name = nullptr;
    uint8_t *path_in = nullptr;
    uint8_t *path_out = nullptr;
    uint32_t entry_num = 0;

    CmdDataIterator iter(request.first);
    iter.getByteArray(&socket_name)
        .getByteArray(&path_in)
        .getInt(&entry_num)
        .getByteArray(&path_out);
    if (!iter.isValid())
    {
        LE("EraComicBridge::processRarExtract: Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    //std::string arc_path = "/sdcard/download/archive.rar";
    std::string arc_path  = std::string(reinterpret_cast<const char *>(path_in));
    std::string file_path = std::string(reinterpret_cast<const char *>(path_out));

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

    if(checkArchiveFormat(arc_path, fd) != DOC_FORMAT_CBR)
    {
        LE("EraComicBridge::processRarExtract: UNSUPPORTED ARCHIVE format [%s]",arc_path.c_str());
        //return;
        //signature check failed, but we still can try opening it
    }
    //bool res = extractRarEntry(arc_path, 1, "/sdcard/download/a.jpg");
    bool res = extractRarEntry(arc_path, fd, entry_num, file_path);
    //if(res) LI("EraComicBridge::processRarExtract: Extracted entry [%d] to [%s] from [%s]", entry_num, file_path.c_str(), arc_path.c_str());

    response.addInt((int)res); // 1 on success
}
