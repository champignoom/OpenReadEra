/*
 * Copyright (C) 2013 The DjVU CLI viewer interface Project
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

#include <stdlib.h>

#include "ore_log.h"
#include "StProtocol.h"
#include "StSocket.h"
#include "EraDjvuBridge.h"
#include "openreadera.h"
#include "smartcrop.h"
// Should go as last include to not trigger rebuild of other files on changes
#include "openreadera_version.h"

#define RES_DJVU_FAIL       255

DjvuBridge::DjvuBridge() : StBridge("EraDjvuBridge")
{
    context = NULL;
    doc = NULL;
    pageCount = 0;
    pages = NULL;
    info = NULL;
    outline = NULL;
}

DjvuBridge::~DjvuBridge()
{
    if (outline != NULL)
    {
        delete outline;
        outline = NULL;
    }

    if (pages != NULL)
    {
        int i;
        for (i = 0; i < pageCount; i++)
        {
            if (pages[i] != NULL)
            {
                ddjvu_page_release(pages[i]);
            }
        }
        free(pages);
        pages = NULL;
    }
    if (info != NULL)
    {
        int i;
        for (i = 0; i < pageCount; i++)
        {
            if (info[i] != NULL)
            {
                delete info[i];
            }
        }
        free(info);
        info = NULL;
    }

    if (doc)
    {
        ddjvu_document_release(doc);
        doc = NULL;
    }
    if (context)
    {
        ddjvu_context_release(context);
        context = NULL;
    }
}

void DjvuBridge::process(CmdRequest &request, CmdResponse &response)
{
    response.reset();

    request.print("EraDjvuBridge");

    switch (request.cmd)
    {
        case CMD_REQ_OPEN:
            processOpen(request, response);
            break;
        case CMD_REQ_PAGE_INFO:
            processPageInfo(request, response);
            break;
        case CMD_REQ_PAGE:
            processPage(request, response);
            break;
        case CMD_REQ_LINKS:
            processPageLinks(request, response);
            break;
        case CMD_REQ_PAGE_RENDER:
            processPageRender(request, response);
            break;
        case CMD_REQ_PAGE_FREE:
            processPageFree(request, response);
            break;
        case CMD_REQ_SMART_CROP:
            processSmartCrop(request, response);
            break;
        case CMD_REQ_PAGE_TEXT:
            processPageText(request, response);
            break;
        case CMD_REQ_OUTLINE:
            processOutline(request, response);
            break;
        case CMD_REQ_QUIT:
            processQuit(request, response);
            break;
        case CMD_REQ_VERSION:
            OreVerResporse(OPENREADERA_BASE_VERSION, response);
            break;
        case CMD_REQ_XPATH_BY_RECT_ID:
            processXPathByRectId(request, response);
            break;
        case CMD_REQ_SEARCH_PREVIEWS:
            processTextSearchPreviews(request, response);
            break;
        case CMD_REQ_SEARCH_HITBOXES:
            processTextSearchHitboxes(request, response);
            break;
        case CMD_REQ_SEARCH_COUNTER:
            processSearchCounter(request, response);
            break;
        case CMD_REQ_RANGE_HITBOX:
            processPageRangeText(request, response);
            break;
        case CMD_REQ_RTL_TEXT:
            processRtlText(request, response);
            break;
        default:
            LE("Unknown request: %d", request.cmd);
            response.result = RES_UNKNOWN_CMD;
            break;
    }

    response.print("EraDjvuBridge");
}

void DjvuBridge::processQuit(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_QUIT;
}

void DjvuBridge::processOpen(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_OPEN;
    if (request.dataCount == 0)
    {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    uint32_t doc_format = 0;
    uint8_t *socket_name = NULL;
    uint8_t *file_name = NULL;

    CmdDataIterator iter(request.first);
    iter.getByteArray(&socket_name);
    iter.getInt(&doc_format);
    iter.getByteArray(&file_name);

    if ((!iter.isValid()) || (!socket_name))
    {
        LE("Bad request data: %u %u %u %p", request.cmd, request.dataCount, iter.getCount(), socket_name);
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    const bool send_fd_via_socket = (strlen((const char *) socket_name) > 0);
    int fd;

    if (send_fd_via_socket)
    {
        LD("Socket name: %s", socket_name);
        StSocketConnection connection((const char *) socket_name);
        if (!connection.isValid())
        {
            response.result = RES_BAD_REQ_DATA;
            return;
        }

        LD("client: connected!");
        bool received = connection.receiveFileDescriptor(fd);

        LD("File descriptor:  %d", fd);
        if (!received)
        {
            response.result = RES_BAD_REQ_DATA;
            return;
        }
    }
    else
    {
        fd = open(reinterpret_cast<const char *>(file_name), O_RDONLY);
    }

    if (doc == NULL)
    {
        context = ddjvu_context_create("EraDjvuBridge");
        char url[1024];
        sprintf(url, "fd:%d", fd);
        LD("Opening url: %s", url);
        doc = ddjvu_document_create_by_filename(context, url, FALSE);
        if (!doc)
        {
            LE("DJVU file not found or corrupted.");
            response.result = RES_BAD_REQ_DATA;
            return;
        }

        ddjvu_fileinfo_t finfo;
        ddjvu_status_t status;
        while ((status = ddjvu_document_get_fileinfo(doc, 0, &finfo)) < DDJVU_JOB_OK)
        {
            waitAndHandleMessages();
        }

        if (status != DDJVU_JOB_OK)
        {
            LE("DJVU file corrupted: %d", status);
            response.result = RES_DJVU_FAIL;
            return;
        }

        pageCount = ddjvu_document_get_pagenum(doc);

        info = (ddjvu_pageinfo_t**) calloc(pageCount, sizeof(ddjvu_pageinfo_t*));
        pages = (ddjvu_page_t**) calloc(pageCount, sizeof(ddjvu_page_t*));

        outline = new DjvuOutline(doc);
    }
    response.addInt(pageCount);
}

void DjvuBridge::processPageInfo(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE_INFO;
    if (request.dataCount == 0)
    {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    uint32_t pageNo = 0;
    if (!CmdDataIterator(request.first).getInt(&pageNo).isValid())
    {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (doc == nullptr)
    {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }

    ddjvu_pageinfo_t* i = getPageInfo(pageNo);
    if (i == nullptr)
    {
        response.result = RES_DJVU_FAIL;
        return;
    }

    float data[2];
    data[0] = i->width;
    data[1] = i->height;

    response.addFloatArray(2, data, true);
}

void DjvuBridge::processPage(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE;
    if (request.dataCount == 0)
    {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    uint32_t pageNumber = 0;
    CmdDataIterator iter(request.first);
    if (!iter.getInt(&pageNumber).isValid())
    {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (doc == nullptr)
    {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }

    if (pageNumber >= pageCount)
    {
        LE("Bad page index: %d", pageNumber);
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    ddjvu_pageinfo_t* i = getPageInfo(pageNumber);
#ifdef OREDEBUG
    LI("EraDjvuBridge::processPage getPage %d GO", pageNumber);
#endif
    ddjvu_page_t* p = getPage(pageNumber, false);
#ifdef OREDEBUG
    LI("EraDjvuBridge::processPage getPage %d OK", pageNumber);
#endif
    if (i == nullptr || p == nullptr)
    {
        response.result = RES_DJVU_FAIL;
        return;
    }
}

void DjvuBridge::processPageFree(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE_FREE;
    if (request.dataCount == 0)
    {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    uint32_t pageNumber = 0;
    CmdDataIterator iter(request.first);
    if (!iter.getInt(&pageNumber).isValid())
    {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (doc == NULL)
    {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }

    if (pageNumber >= pageCount)
    {
        LE("Bad page index: %d", pageNumber);
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (pages[pageNumber])
    {
        ddjvu_page_release(pages[pageNumber]);
        pages[pageNumber] = NULL;
    }
}

void DjvuBridge::processPageRender(CmdRequest& request, CmdResponse& response)
{
#ifdef OREDEBUG
    //LI("EraDjvuBridge::processPageRender GO");
#endif
    response.cmd = CMD_RES_PAGE_RENDER;
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t pageNumber, targetWidth, targetHeight;
    float *temp_config;
    CmdDataIterator iter(request.first);
    iter.getInt(&pageNumber)
            .getInt(&targetWidth)
            .getInt(&targetHeight)
            .getFloatArray(&temp_config, 6);

    if (!iter.isValid()) {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (doc == nullptr || pages == nullptr) {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }
    float pageSliceX = temp_config[0];
    float pageSliceY = temp_config[1];
    float pageSliceWidth = temp_config[2];
    float pageSliceHeight = temp_config[3];
#ifdef OREDEBUG
    LI("EraDjvuBridge::processPageRender getPage %d GO", pageNumber);
#endif
    getPage(pageNumber, true);
#ifdef OREDEBUG
    LI("EraDjvuBridge::processPageRender getPage %d OK", pageNumber);
#endif
    ddjvu_rect_t pageRect;
    pageRect.x = 0;
    pageRect.y = 0;
    pageRect.w = targetWidth / pageSliceWidth;
    pageRect.h = targetHeight / pageSliceHeight;
    ddjvu_rect_t targetRect;
    targetRect.x = pageSliceX * targetWidth / pageSliceWidth;
    targetRect.y = pageSliceY * targetHeight / pageSliceHeight;
    targetRect.w = targetWidth;
    targetRect.h = targetHeight;
    unsigned int masks[] = { 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 };
    ddjvu_format_t* pixelFormat = ddjvu_format_create(DDJVU_FORMAT_RGBMASK32, 4, masks);
    ddjvu_format_set_row_order(pixelFormat, TRUE);
    ddjvu_format_set_y_direction(pixelFormat, TRUE);
    int size = targetRect.w * targetRect.h * 4;
    auto resp = new CmdData();
    char* pixels = (char*)resp->newByteArray(size);
    int result = ddjvu_page_render(
            pages[pageNumber],
            (ddjvu_render_mode_t) HARDCONFIG_DJVU_RENDERING_MODE,
            &pageRect,
            &targetRect,
            pixelFormat, targetWidth * 4, pixels);
    ddjvu_format_release(pixelFormat);
    if (!result) {
        response.result = RES_DJVU_FAIL;
        delete resp;
    } else {
        response.addData(resp);
    }
#ifdef OREDEBUG
    //LI("EraDjvuBridge::processPageRender OK");
#endif
}

void DjvuBridge::processOutline(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_OUTLINE;

    if (doc == NULL)
    {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }

    if (outline == NULL)
    {
        outline = new DjvuOutline(doc);
    }

    response.addInt(0);
    outline->toResponse(response);
}

void DjvuBridge::processPageText(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE_TEXT;

    if (doc == NULL)
    {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }

    uint32_t pageNo;
    uint8_t* pattern = NULL;

    CmdDataIterator iter(request.first);
    iter.getInt(&pageNo);

    if (!iter.isValid())
    {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    processText((int) pageNo, (const char*) pattern, response);
}

ddjvu_pageinfo_t *DjvuBridge::getPageInfo(uint32_t pageNo) {
    if (info[pageNo] == nullptr) {
        auto i = new ddjvu_pageinfo_t();
        ddjvu_status_t r;
        while ((r = ddjvu_document_get_pageinfo(doc, pageNo, i)) < DDJVU_JOB_OK) {
            waitAndHandleMessages();
        }
        if (r == DDJVU_JOB_OK) {
            info[pageNo] = i;
        } else {
            LE("Cannot retrieve page info: %d %d", pageNo, r);
            delete i;
        }
    }
    return info[pageNo];
}

ddjvu_page_t* DjvuBridge::getPage(uint32_t pageNo, bool decode)
{
#ifdef OREDEBUG
    //LD("EraDjvuBridge::getPage page %d, GO", pageNo);
#endif
    if (pages[pageNo] == nullptr) {
        pages[pageNo] = ddjvu_page_create(doc, pageNo);
    }
    if (decode) {
        int step = 0;
        ddjvu_status_t r;
        while ((r = ddjvu_page_decoding_status(pages[pageNo])) < DDJVU_JOB_OK) {
#ifdef OREDEBUG
            LD("EraDjvuBridge::getPage waitAndHandleMessages step %d GO", step);
#endif
            waitAndHandleMessages();
#ifdef OREDEBUG
            LD("EraDjvuBridge::getPage waitAndHandleMessages step %d OK", step);
            step++;
#endif
        }
        if (r != DDJVU_JOB_OK) {
            LE("Cannot decode page %d, error %d", pageNo, r);
        }
    }
#ifdef OREDEBUG
    //LD("EraDjvuBridge::getPage %d OK", pageNo);
#endif
    return pages[pageNo];
}

void DjvuBridge::waitAndHandleMessages()
{
#ifdef OREDEBUG
    //LD("EraDjvuBridge::waitAndHandleMessages GO");
#endif
    // Wait for first message
    ddjvu_message_wait(context);
    // Process available messages
    const ddjvu_message_t* msg;
    while ((msg = ddjvu_message_peek(context))) {
        switch (msg->m_any.tag) {
            case DDJVU_ERROR:
                LE("decoding error: %s %s %d",
                        msg->m_error.filename, msg->m_error.function, msg->m_error.lineno);
                break;
            case DDJVU_INFO:
                break;
            case DDJVU_DOCINFO:
                break;
            default:
                break;
        }
        ddjvu_message_pop(context);
    }
#ifdef OREDEBUG
    //LD("EraDjvuBridge::waitAndHandleMessages OK");
#endif
}

void DjvuBridge::processPageLinks(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_LINKS;
    if (request.dataCount == 0)
    {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t pageNumber = 0;
    CmdDataIterator iter(request.first);
    if (!iter.getInt(&pageNumber).isValid())
    {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (doc == NULL)
    {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }
    if (pageNumber >= pageCount)
    {
        LE("Bad page index: %d", pageNumber);
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    processLinks(pageNumber, response);
}

void DjvuBridge::processSmartCrop(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_SMART_CROP;
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (doc == NULL || pages == NULL) {
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
    iter.getInt(&page_index).getFloat(&origin_w).getFloat(&origin_h)
            .getFloat(&slice_l).getFloat(&slice_t).getFloat(&slice_r).getFloat(&slice_b);
    if (!iter.isValid()) {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    float slice_w = slice_r - slice_l;
    float slice_h = slice_b - slice_t;

#ifdef OREDEBUG
    LI("EraDjvuBridge::processSmartCrop getPage %d GO", page_index);
#endif
    getPage(page_index, true);
#ifdef OREDEBUG
    LI("EraDjvuBridge::processSmartCrop getPage %d OK", page_index);
#endif

    float lambda = origin_h / origin_w;
    int smart_S = smart_crop_w * smart_crop_h;

    int new_w = sqrt(smart_S / lambda);
    int new_h = new_w * lambda;

    ddjvu_rect_t pageRect;
    pageRect.x = 0;
    pageRect.y = 0;
    pageRect.w = new_w / slice_w;
    pageRect.h = new_h / slice_h;
    ddjvu_rect_t targetRect;
    targetRect.x = slice_l * new_w / slice_w;
    targetRect.y = slice_t * new_h / slice_h;
    targetRect.w = new_w;
    targetRect.h = new_h;

    unsigned int masks[] = { 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 };
    ddjvu_format_t* pixelFormat = ddjvu_format_create(DDJVU_FORMAT_RGBMASK32, 4, masks);

    ddjvu_format_set_row_order(pixelFormat, TRUE);
    ddjvu_format_set_y_direction(pixelFormat, TRUE);
    int size = (targetRect.w * targetRect.h * 4 );
    char* pixels = (char*) malloc(size);

    //TODO DDJVU_RENDER_BLACK?
    int result = ddjvu_page_render(
            pages[page_index],
            DDJVU_RENDER_COLOR,
            &pageRect,
            &targetRect,
            pixelFormat,
            targetRect.w * 4,
            pixels);

    ddjvu_format_release(pixelFormat);

    if (!result)
    {
        response.result = RES_DJVU_FAIL;
        return;
    }
    float smart_crop[4] = {0, 0, 1, 1};

    CalcBitmapSmartCrop(smart_crop, (uint8_t *) pixels, new_w, new_h, slice_l, slice_t, slice_r, slice_b);
    response.addFloatArray(4, smart_crop, true);

    if(pixels) { free(pixels); }
}

void DjvuBridge::processXPathByRectId(CmdRequest& request, CmdResponse& response)
{
    if (doc == NULL)
    {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }

    response.cmd = CMD_RES_XPATH_BY_RECT_ID;
    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        //CRLog::error("processXPathByHitbox bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);

    //std::stringstream input_str("1:22:0"); //page:id:end

    std::stringstream input_str(val);
    std::string key;
    std::vector<std::string> keys;

    while(std::getline(input_str, key, ':'))
    {
        keys.push_back(key);
    }

    int page = atoi(keys.at(0).c_str());
    int id = atoi(keys.at(1).c_str());

    std::string xpath = GetXpathFromPageById(page, id, true);
    response.addIpcString(xpath.c_str(),true);
}

void DjvuBridge::responseAddString(CmdResponse& response, std::wstring str16)
{
    std::string str8 = djvu_wstringToString(str16);
    uint32_t size = (uint32_t) str8.size();
    // We will place null-terminator at the string end
    size++;
    CmdData* cmd_data = new CmdData();
    unsigned char* str_buffer = cmd_data->newByteArray(size);
    memcpy(str_buffer, str8.c_str(), (size - 1));
    str_buffer[size - 1] = 0;
    response.addData(cmd_data);
}

void DjvuBridge::processSearchCounter(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_SEARCH_COUNTER;
    uint32_t flag;

    CmdDataIterator iter(request.first);
    iter.getInt(&flag);

    if (!iter.isValid())
    {
        LE("processSearchCounter Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    if (flag == 1)     // 1 == on // 0 = off
    {
        response.addInt(searchPackCounter);
    }
    else if(flag == 0)
    {
        response.addInt(searchPackCounter);
        searchPackCounter = 0;
    }
}

std::vector<Hitbox>
DjvuBridge::GetHitboxesBetweenXpaths(uint32_t page, std::string xpStart, std::string xpEnd, int startPage)
{
    std::vector<Hitbox> result;
    std::vector<Hitbox> hitboxes = processTextToArray(page);

    if(hitboxes.empty())
    {
        return result;
    }
    bool collect = false;

    if(startPage < page)
    {
        collect = true;
    }

    for (int i = 0; i < hitboxes.size(); i++)
    {
        Hitbox curr = hitboxes.at(i);
        std::string xp = curr.xpointer_;

        if(xp == xpStart)
        {
            collect = true;
        }
        if(xp == xpEnd)
        {
            collect = false;
            result.push_back(curr);
            break;
        }
        if(collect)
        {
            result.push_back(curr);
        }
    }

    return result;
}

void DjvuBridge::processPageRangeText(CmdRequest &request, CmdResponse &response) {

    response.cmd = CMD_RES_RANGE_HITBOX;

    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        LE("processPageRangeText bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    std::string inputstr = std::string(val);

    //"page;start;end"
    //lString16 key_input = lString16("0;"
    //                                "/body/body[1]/p[11]/h6/text().0;"
    //                                "/body/body[1]/p[44]/a/text().1");


    std::stringstream in_stream = std::stringstream(inputstr);

    std::string key;
    std::vector<std::string> in_arr;

    while (std::getline(in_stream, key, ';'))
    {
        in_arr.push_back(key);
    }
    if (in_arr.at(0).empty() || in_arr.at(1).empty() || in_arr.at(2).empty() )
    {
        LE("processPageRangeText bad request data : invalid key string [%s]",inputstr.c_str());
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    uint32_t page = std::atoi(in_arr.at(0).c_str());
    std::string startstr = in_arr.at(1);
    std::string endstr = in_arr.at(2);

    int pos1 = startstr.find("[") +1;
    int pos2 = startstr.find("]");
    std::string startpagestr = startstr.substr(pos1,pos2-pos1);
    int startPage = std::atoi(startpagestr.c_str());

    //LE(" xp = %s",startstr.c_str());
    //LE(" page = %d , startpage = %d",page,startPage);

    std::vector<Hitbox> BookmarkHitboxes = GetHitboxesBetweenXpaths(page, startstr, endstr, startPage);
    BookmarkHitboxes = unionRects(BookmarkHitboxes);

    std::string resp_key;
    resp_key.append(startstr);
    resp_key.append(":");
    resp_key.append(endstr);

    for (int i = 0; i < BookmarkHitboxes.size(); i++)
    {
        Hitbox currHitbox = BookmarkHitboxes.at(i);
        response.addFloat(currHitbox.left_);
        response.addFloat(currHitbox.top_);
        response.addFloat(currHitbox.right_);
        response.addFloat(currHitbox.bottom_);
        response.addIpcString(resp_key.c_str(), true);
    }
}
