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
#include "StSocket.h"
#include <cstdlib>
// Should go as last include to not trigger rebuild of other files on changes
#include "openreadera_version.h"

static int CeilToEvenInt(int n)
{
    return (n + 1) & ~1;
}

static int FloorToEvenInt(int n)
{
    return n & ~1;
}

uint32_t CreBridge::ExportPagesCount(int columns, int pages)
{
	if (columns == 2) {
		return (uint32_t) (CeilToEvenInt(pages) / columns);
	}
	return (uint32_t) pages;
}

int CreBridge::ExportPage(int columns, int page)
{
    if (columns == 2) {
        return FloorToEvenInt(page) / columns;
    }
    return page;
}

int CreBridge::ImportPage(int page, int columns)
{
    return page * columns;
}

void CreBridge::responseAddLinkUnknown(CmdResponse& response, const lString16& href,
        float l, float t, float r, float b)
{
    response.addWords(LINK_TARGET_UNKNOWN, 0);
    responseAddString(response, href);
    response.addFloat(l);
    response.addFloat(t);
    response.addFloat(r);
    response.addFloat(b);
}

void CreBridge::responseAddString(CmdResponse& response, const lString16& str16)
{
    lString8 str8 = UnicodeToUtf8(str16);
    auto size = (uint32_t) str8.size();
    // We will place null-terminator at the string end
    size++;
    auto cmd_data = new CmdData();
    unsigned char* str_buffer = cmd_data->newByteArray(size);
    memcpy(str_buffer, str8.c_str(), (size - 1));
    str_buffer[size - 1] = 0;
    response.addData(cmd_data);
}

void CreBridge::convertBitmap(LVColorDrawBuf* bitmap)
{
    if (bitmap->GetBitsPerPixel() == 32) {
        // Convert Cre colors to Android
        int size = bitmap->GetWidth() * bitmap->GetHeight();
        for (lUInt8* p = bitmap->GetData(); --size >= 0; p+=4) {
            // Invert A
            p[3] ^= 0xFF;
            // Swap R and B
            lUInt8 t = p[0];
            p[0] = p[2];
            p[2] = t;
        }
    }
}

void CreBridge::processOpen(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_OPEN;
    CmdDataIterator iter(request.first);
    uint32_t doc_format = 0;
    uint8_t* socket_name = nullptr;
    uint8_t* absolute_path_arg = nullptr;
    uint32_t direct_archive = 0;
    iter.getByteArray(&socket_name)
        .getInt(&doc_format)
        .getByteArray(&absolute_path_arg)
        .getInt(&direct_archive);
    if (!iter.isValid() || !socket_name || !absolute_path_arg) {
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    const bool send_fd_via_socket = ( strlen((const char*) socket_name) > 0 );
    bool result;

    if (send_fd_via_socket)
    {
        StSocketConnection connection((const char *) socket_name);
        if (!connection.isValid()){
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        int fd;
        bool received = connection.receiveFileDescriptor(fd);
        if (!received){
            response.result = RES_BAD_REQ_DATA;
            return;
        }
        LE("CreBridge::processOpen via fd = [%d]", fd);
        result = doc_view_->LoadDoc(doc_format, "" , fd, direct_archive);
    }
    else
    {
        const char* absolute_path = reinterpret_cast<const char*>(absolute_path_arg);
        LE("CreBridge::processOpen via path = [%s]", absolute_path);
        result = doc_view_->LoadDoc(doc_format, absolute_path, -1, direct_archive);
    }

    if (result)
    {
        doc_view_->RenderIfDirty();
        response.addInt(ExportPagesCount(doc_view_->GetColumns(), doc_view_->GetPagesCount()));
    }
    else if (OreIsNormalDirectArchive(direct_archive))
        {response.result = RES_ARCHIVE_COLLISION;}
    else
        {response.result = RES_INTERNAL_ERROR;}
}

void CreBridge::processPageRender(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE_RENDER;
    CmdDataIterator iter(request.first);
    uint32_t page;
    uint32_t width;
    uint32_t height;
    iter.getInt(&page).getInt(&width).getInt(&height);
    if (!iter.isValid()) {
        CRLog::error("processPageRender bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (doc_view_ == nullptr) {
        CRLog::error("processPageRender doc not opened");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    doc_view_->GoToPage(ImportPage(page, doc_view_->GetColumns()));
    auto resp = new CmdData();
    unsigned char* pixels = resp->newByteArray(width * height * 4);
    if(gJapaneseVerticalMode)
    {
        //reversed height and width
        auto buf = new LVColorDrawBuf(height, width, pixels, 32);
        doc_view_->Draw(*buf);
        convertBitmap(buf);
        delete buf;

        auto data = (lUInt8 *) malloc(width * height * 4*4);
        if(!data)
        {
            LE("failed to allocate %d bytes for page",width*height*4*4);
            return;
        }
        int ptr = 0;
        //LE("start w = %d , h = %d",width,height);
        for (int x = 0 ; x < height; x++)
        {
            //LE("column x = %d",x);
            for (int y = (int)width- 1; y >= 0; y--)
            {
                for (int i = 0; i < 4; i++)
                {
                    data[ptr] = pixels[(y*height*4)+(x*4)+ i];
                    ptr++;
                }
            }
        }
        memcpy(pixels,data,width*height*4);
        free(data);

        response.addData(resp);
    }
    else
    {
        auto buf = new LVColorDrawBuf(width, height, pixels, 32);
        doc_view_->Draw(*buf);
        convertBitmap(buf);
        delete buf;
        response.addData(resp);
    }
}

void CreBridge::processQuit(CmdRequest& request, CmdResponse& response)
{
    doc_view_->Clear();
    response.cmd = CMD_RES_QUIT;
}

CreBridge::CreBridge() : StBridge(ORE_LOG_TAG)
{
    doc_view_ = nullptr;
#ifdef OREDEBUG
    CRLog::setLevel(CRLog::TRACE);
#else
    CRLog::setLevel(CRLog::FATAL);
#endif
    InitFontManager(lString8::empty_str);
    // 0 - disabled, 1 - bytecode, 2 - auto
    fontMan->SetHintingMode(HINTING_MODE_BYTECODE_INTERPRETOR);
    fontMan->setKerning(true);
    HyphMan::init();
}

CreBridge::~CreBridge()
{
    delete doc_view_;
    HyphMan::uninit();
    ShutdownFontManager();
}

void CreBridge::process(CmdRequest& request, CmdResponse& response)
{
    response.reset();
    request.print("EraEpubBridge");
    switch (request.cmd)
    {
        case CMD_REQ_SET_FONT_CONFIG:
            processFontsConfig(request, response);
            break;
        case CMD_REQ_SET_CONFIG:
            processConfig(request, response);
            break;
        case CMD_REQ_OPEN:
            processOpen(request, response);
            break;
        case CMD_REQ_PAGE_RENDER:
            processPageRender(request, response);
            break;
        case CMD_REQ_LINKS:
            processPageLinks(request, response);
            break;
        case CMD_REQ_PAGE_TEXT:
            processPageText(request, response);
            break;
        case CMD_REQ_XPATH_BY_HITBOX:
            processXPathByHitbox(request, response);
            break;
        case CMD_REQ_XPATH_BY_RECT_ID:
            processXPathByRectId(request, response);
            break;
        case CMD_REQ_RANGE_HITBOX:
            processPageRangeText(request, response);
            break;
        case CMD_REQ_OUTLINE:
            processOutline(request, response);
            break;
        case CMD_REQ_CRE_PAGE_BY_XPATH:
            processPageByXPath(request, response);
            break;
        case CMD_REQ_CRE_PAGE_BY_XPATH_MULT:
            processPageByXPathMultiple(request, response);
            break;
        case CMD_REQ_CRE_PAGE_XPATH:
            processPageXPath(request, response);
            break;
        case CMD_REQ_CRE_METADATA:
            processMeta(request, response);
            break;
        case CMD_REQ_QUIT:
            processQuit(request, response);
            break;
        case CMD_REQ_VERSION:
            OreVerResporse(OPENREADERA_BASE_VERSION, response);
            break;
        case CMD_REQ_SEARCH_PREVIEWS:
            processTextSearchPreviews(request, response);
            break;
        case CMD_REQ_SEARCH_HITBOXES:
            processTextSearchHitboxes(request, response);
            break;
        case CMD_REQ_INDEXER:
            processTextSearchGetSuggestionsIndex(request, response);
            break;
        case CMD_REQ_CRE_IMG_XPATHS:
            processImagesXpaths(request, response);
            break;
        case CMD_REQ_CRE_IMG_BLOB:
            processImageByXpath(request, response);
            break;
        case CMD_REQ_CRE_IMG_HITBOXES:
            processImageHitbox(request, response);
            break;
        case CMD_REQ_RTL_TEXT:
            processRtlText(request, response);
            break;
        case CMD_REQ_FONT_NAMES:
            processFontNames(request, response);
            break;
        case CMD_REQ_FONT_LIGMAP:
            processFontLigmap(request, response);
            break;
        default:
            CRLog::error("Unknown request: %d", request.cmd);
            response.result = RES_UNKNOWN_CMD;
            break;
    }
    response.print("EraEpubBridge");
}
