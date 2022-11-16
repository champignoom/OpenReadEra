/*
 * Copyright (C) 2013 The MuPDF CLI viewer interface Project
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

#include <cstdlib>
#include <csignal>
#include <algorithm>
#include <sstream>

#include "ore_log.h"
#include "StSocket.h"
#include "EraPdfBridge.h"
#include "openreadera.h"
#include "debug_intentional_crash.h"
#include "smartcrop.h"
#include "StSearchUtils.h"
#include "EraPdfReflow.h"

// Should go as last include to not trigger rebuild of other files on changes
#include "openreadera_version.h"

MuPdfBridge::MuPdfBridge() : StBridge("EraPdfBridge")
{
    fd = -1;
    password = nullptr;
    ctx = nullptr;
    document = nullptr;
    outline = nullptr;
    pageCount = 0;
    pages = nullptr;
    pageLists = nullptr;
    storememory = 64 * 1024 * 1024;
    format = 0;
    layersmask = -1;
    resetFonts();
    reflowManager = nullptr;
}

MuPdfBridge::~MuPdfBridge()
{
    if (outline != nullptr) {
        fz_drop_outline(ctx, outline);
        outline = nullptr;
    }
    release();
    if (password) {
        free(password);
    }
}

void MuPdfBridge::process(CmdRequest& request, CmdResponse& response)
{
    response.reset();
    request.print("EraPdfBridge");
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
    case CMD_REQ_PAGE_TEXT:
        processPageText(request, response);
        break;
    case CMD_REQ_OUTLINE:
        processOutline(request, response);
        break;
    case CMD_REQ_QUIT:
        processQuit(request, response);
        break;
    case CMD_REQ_PDF_SYSTEM_FONT:
        processSystemFont(request, response);
        break;
    case CMD_REQ_PDF_GET_MISSED_FONTS:
        processGetMissedFonts(request, response);
        break;
    case CMD_REQ_PDF_GET_LAYERS_LIST:
        processGetLayersList(request, response);
        break;
    case CMD_REQ_PDF_STORAGE:
        processStorage(request, response);
        break;
    case CMD_REQ_PDF_SET_LAYERS_MASK:
        processSetLayersMask(request, response);
        break;
    case CMD_REQ_SET_FONT_CONFIG:
        processFontsConfig(request, response);
        break;
    case CMD_REQ_SET_CONFIG:
        processConfig(request, response);
        break;
    case CMD_REQ_SMART_CROP:
        processSmartCrop(request, response);
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
    case CMD_REQ_REFLOW_ANALYZE :
        processAnalyzeDocForReflow(request,response);
        break;
    case CMD_REQ_REFLOW_PROCESS:
        processDocToFB2(request,response);
        break;
    case CMD_REQ_PDF_XPATH_BY_COORDS:
        processXpathByCoords(request,response);
        break;
    case CMD_REQ_RTL_TEXT:
        processRtlText(request, response);
        break;
    default:
        LE("Unknown request: %d", request.cmd);
        response.result = RES_UNKNOWN_CMD;
        break;
    }
    response.print("EraPdfBridge");
}

void MuPdfBridge::processQuit(CmdRequest& request, CmdResponse& response)
{
    for (int i = 0; i < pagesCache.size(); i++)
    {
        pagesCache.at(i).reset();
    }
    pagesCache.clear();
    for (int i = 0; i < pageCount; i++)
    {
        free(ctx->darkmode_objs[i].obj);
    }
    free(ctx->darkmode_objs);
    response.cmd = CMD_RES_QUIT;
}

void MuPdfBridge::processStorage(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PDF_STORAGE;
    if (request.dataCount == 0)
    {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    uint32_t storageSize = 0;
    if (!CmdDataIterator(request.first).getInt(&storageSize).isValid())
    {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    this->storememory = storageSize * 1024 * 1024;

    LI("Storage size : %d MB", storageSize);
}

void MuPdfBridge::processSetLayersMask(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PDF_SET_LAYERS_MASK;
    if (request.dataCount == 0)
    {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    uint32_t layersMask = 0;
    if (!CmdDataIterator(request.first).getInt(&layersMask).isValid())
    {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    this->layersmask = layersMask;

    restart();
}

void MuPdfBridge::processOpen(CmdRequest& request, CmdResponse& response)
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

    if (doc_format == DOC_FORMAT_PDF) {
        config_format = FORMAT_PDF;
    } else if (doc_format == DOC_FORMAT_OXPS || doc_format == DOC_FORMAT_XPS) {
        config_format = FORMAT_XPS;
    } else {
        LE("Bad file type: %u", config_format);
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const bool send_fd_via_socket = ( strlen((const char*) socket_name) > 0 );

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

    if (this->password) {
        free(this->password);
    }
    this->password = password != nullptr ? strdup((const char*) password) : nullptr;
    this->format = config_format;

    if (ctx == nullptr) {
        LD("Creating context: storememory = %d", storememory);
        ctx = fz_new_context(nullptr, nullptr, storememory);
        if (!ctx)
        {
            LE("Out of Memory");
            response.result = RES_MUPDF_OOM;
            return;
        }
        ctx->erapdf_linearized_load = 0;
        ctx->erapdf_ignore_most_errors = 1;
        ctx->erapdf_has_password = (this->password && strlen(this->password));
    }
    eraConfig.applyToCtx(ctx);
    if (document == nullptr) {
        LI("Opening document: %d %d", format, fd);
        LI(ctx->erapdf_has_password ? "Password present" : "No password");
        fz_try(ctx) {
            if (format == FORMAT_XPS) {
                document = (fz_document*) xps_open_document_with_stream(ctx, fz_open_fd(ctx, dup(fd)));
            } else {
                document = (fz_document*) pdf_open_document_with_stream(ctx, fz_open_fd(ctx, dup(fd)));
            }
        } fz_catch(ctx) {
            const char* msg = fz_caught_message(ctx);
            LE("Opening document failed: %s", msg);
            response.result = RES_INTERNAL_ERROR;
            response.addIpcString(msg, true);
            return;
        }
        if (fz_needs_password(ctx, document)) {
            LD("Document required a password: %s", this->password);
            if (this->password && strlen(this->password)) {
                int ok = fz_authenticate_password(ctx, document, this->password);
                if (!ok) {
                    LE("Wrong password given");
                    response.result = RES_MUPDF_PWD_WRONG;
                    return;
                }
            } else {
                LE("Document needs a password!");
                response.result = RES_MUPDF_PWD_NEED;
                return;
            }
        }
        applyLayersMask();
        fz_try(ctx) {
            pageCount = fz_count_pages(ctx, document);
            LD("Document pages: %d", pageCount);
            pages = (fz_page**) calloc(pageCount, sizeof(fz_page*));
            pageLists = (fz_display_list**) calloc(pageCount, sizeof(fz_display_list*));
        } fz_catch(ctx) {
            const char* msg = fz_caught_message(ctx);
            LE("Counting pages failed: %s", msg );
            response.result = RES_INTERNAL_ERROR;
            response.addIpcString(msg, true);
            return;
        }
    }
    response.addInt(pageCount);
	pagesCache.reserve(5);
	ctx->darkmode_objs = static_cast<darkmode_obj_page *>(malloc(sizeof(darkmode_obj_page) * pageCount));
    for (int i = 0; i < pageCount; i++)
    {
        ctx->darkmode_objs[i].analyzed = 0;
    }
    ctx->erapdf_nightmode = config_invert_images;
    reflowManager = new ReflowManager(ctx, this);
}

void MuPdfBridge::processPageInfo(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE_INFO;
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t pageNo = 0;
    if (!CmdDataIterator(request.first).getInt(&pageNo).isValid()) {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (document == nullptr) {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }
#ifdef DEBUG_INTENTIONAL_CRASH
    if (pageNo == 1) {
        debug_generate_sigsegv_segv_maperr();
    }
#endif
    fz_page* page = getPage(pageNo, false);
    if (!page) {
        LE("No page %d found", pageNo);
        response.result = RES_INTERNAL_ERROR;
        return;
    }
    fz_try(ctx) {
        fz_rect bounds = fz_empty_rect;
        fz_bound_page(ctx, page, &bounds);
        float data[2];
        data[0] = fabs(bounds.x1 - bounds.x0);
        data[1] = fabs(bounds.y1 - bounds.y0);
        response.addFloatArray(2, data, true);
    } fz_catch(ctx) {
        const char* msg = fz_caught_message(ctx);
        LE("%s", msg);
        response.result = RES_INTERNAL_ERROR;
        return;
    }
}

void MuPdfBridge::analyzePageForDarkMode(fz_context *ctx, int index, int w, int h)
{
    if(ctx->darkmode_objs[index].analyzed != 0)
    {
        //LE("Already analyzed page %d",index)
        return;
    }
    ctx->darkmode_objs[index].analyzed = 1;

    fz_page* page = getPage(index, true);
    if (!page || !pageLists[index]) {
        return;
    }
    // Clear counter
    ctx->erapdf_setcolor_per_page = 0;
    fz_rect area;
    area.x0 = 0;
    area.y0 = 0;
    area.x1 = w;
    area.y1 = h;
    fz_matrix ctm = fz_identity;
    auto pixels = (uint8_t*) malloc(w * h * 4);
    fz_device* device = nullptr;
    fz_pixmap* pixmap = nullptr;
    fz_try(ctx) {
                pixmap = fz_new_pixmap_with_data(ctx, fz_device_rgb(ctx), w, h, pixels);
                fz_clear_pixmap_with_value(ctx, pixmap, 0xff);
                device = fz_new_draw_device(ctx, pixmap);
                fz_run_fake_display_list(ctx,pageLists[index], device, &ctm, &area, nullptr,index);
            } fz_catch(ctx) {
        const char* msg = fz_caught_message(ctx);
        LE("%s", msg);
    }
    if (pixels) {
        free(pixels);
    }
    fz_drop_device(ctx, device);
    fz_drop_pixmap(ctx, pixmap);
}

void MuPdfBridge::processPage(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE;
    if (document == nullptr) {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t index = 0;
    uint32_t preview = 0;
    CmdDataIterator iter(request.first);
    if (!iter.getInt(&index).getInt(&preview).isValid()) {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (index >= pageCount) {
        LE("Bad page index: %d", index);
        response.result = RES_BAD_REQ_DATA;
        return;
    }
#ifdef DEBUG_INTENTIONAL_CRASH
    if (pageNumber == 3) {
        debug_generate_sigsegv_segv_maperr();
    }
#endif
    ctx->previewmode = preview;
    ctx->preview_heavy_image = 0; // Reset flag
    fz_page* page = getPage(index, true);
    ctx->previewmode = 0;
    response.addInt(static_cast<uint32_t>(ctx->preview_heavy_image));
    if (page == nullptr) {
        response.result = RES_INTERNAL_ERROR;
        return;
    }
    eraConfig.applyToCtx(ctx);
    ctx->erapdf_nightmode = config_invert_images;
    if(ctx->erapdf_nightmode || ctx->erapdf_twilight_mode)
    {
        //LW("OPEN AnalyzePageforDarkMode, index = %d",index);
        fz_rect bounds = fz_empty_rect;
        fz_bound_page(ctx, page, &bounds);

        int w = fabs(bounds.x1 - bounds.x0);
        int h = fabs(bounds.y1 - bounds.y0);
        analyzePageForDarkMode(ctx,index,w,h);
    }
}

void MuPdfBridge::processPageFree(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE_FREE;
    if (document == nullptr) {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    uint32_t page_index = 0;
    CmdDataIterator iter(request.first);
    if (!iter.getInt(&page_index).isValid()) {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (page_index >= pageCount) {
        LE("Bad page index: %d", page_index);
        response.result = RES_BAD_REQ_DATA;
        return;
    }
#ifdef DEBUG_INTENTIONAL_CRASH
    if (page_index == 5) {
        debug_generate_sigsegv_segv_maperr();
    }
#endif
    ctx->previewmode = 0;
    if (pageLists[page_index]) {
        fz_try(ctx) {
            fz_drop_display_list(ctx, pageLists[page_index]);
        } fz_catch(ctx) {
            const char *msg = fz_caught_message(ctx);
            LE("%s", msg);
        }
        pageLists[page_index] = nullptr;
    }
    if (pages[page_index]) {
        fz_try(ctx) {
            fz_drop_page(ctx, pages[page_index]);
        } fz_catch(ctx) {
            const char *msg = fz_caught_message(ctx);
            LE("%s", msg);
        }
        pages[page_index] = nullptr;
    }
}

void MuPdfBridge::processOutline(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_OUTLINE;
    if (document == nullptr) {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }
    if (outline == nullptr) {
        outline = fz_load_outline(ctx, document);
    }
    response.addInt((uint32_t) 0);
    if (outline) {
    	processOutline(outline, 0, 0, response);
    } else {
        LE("No outline");
    }
}

void MuPdfBridge::processAnalyzeDocForReflow(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_REFLOW_ANALYZE;
    if(reflowManager->analyzed == 0)
    {
        reflowManager->analyzeDocument();
    }
    response.addInt(reflowManager->doctype);
}

void MuPdfBridge::processDocToFB2(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_REFLOW_PROCESS;

    uint8_t* path_out = nullptr;
    uint32_t page_index = 0;
    CmdDataIterator iter(request.first);
    iter.getByteArray(&path_out);
    iter.getInt(&page_index);

    if (!iter.isValid()) {
        LE("processDocToFB2 Bad request data: %s", path_out);
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if( path_out == nullptr)
    {
        LE("processDocToFB2 path_out == nullptr");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if(page_index < 0 || page_index > this->pageCount)
    {
        LE("processDocToFB2 invalid page requested");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    //if(reflowManager->analyzed == false)
    //{
    //    reflowManager->analyzeDocument();
    //}

    char* path = strdup((const char*) path_out);
    //char path[100] = "sdcard/download/pdf2fb2.fb2";
    fz_output *output = fz_new_output_to_filename(ctx, path);
    reflowManager->setOutput(output);
    bool result = reflowManager->reflowPageMain(page_index);
    response.addInt((int)result);
    fz_drop_output(ctx, reflowManager->getOutput());
}

void MuPdfBridge::processXpathByCoords(CmdRequest& request, CmdResponse& response)
{

    response.cmd = CMD_RES_PDF_XPATH_BY_COORDS;
    CmdDataIterator iter(request.first);
    uint32_t temp_page;
    float temp_x;
    float temp_y;
    uint32_t temp_reverse;
    iter.getInt(&temp_page);
    iter.getFloat(&temp_x);
    iter.getFloat(&temp_y);
    iter.getInt(&temp_reverse);
    if (!iter.isValid())
    {
        LE("processXpathByCoords bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    const int page = (int)temp_page;
    const float x = temp_x;
    const float y = temp_y;
    const bool reverse = (bool)temp_reverse;

    std::string xpath = GetXpathFromPageByCoords(page, x, y, true, reverse);
    response.addIpcString(xpath.c_str(), true);
}

void MuPdfBridge::processPageText(CmdRequest& request, CmdResponse& response)
{

    response.cmd = CMD_RES_PAGE_TEXT;
    if (document == nullptr)
    {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }

    uint32_t pageNo;
    uint8_t* pattern = nullptr;

    CmdDataIterator iter(request.first);
    if (!iter.getInt(&pageNo).isValid())
    {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    LD("Retrieve page text: %d", pageNo);

    std::vector<Hitbox> pagetext = processTextToArray( (int) pageNo);
    for (int i = 0; i < pagetext.size(); i++)
    {
        Hitbox curr = pagetext.at(i);
        std::string str = wstringToString(curr.text_);

        response.addFloat(curr.left_);
        response.addFloat(curr.top_);
        response.addFloat(curr.right_);
        response.addFloat(curr.bottom_);
        response.addIpcString(str.c_str(), true);
    }

/*
    fz_page *page = getPage(pageNo, false);
    if (page == NULL)
    {
        return;
    }

    fz_rect bounds = fz_empty_rect;
    fz_bound_page(ctx, page, &bounds);
    int full_w = fabs(bounds.x1 - bounds.x0);
    int full_h = fabs(bounds.y1 - bounds.y0);
    analyzePageForDarkMode(ctx,pageNo,full_w,full_h);

    for (int i = 0; i < ctx->darkmode_objs[pageNo].objcount; i++)
    {
        darkmode_obj obj =  ctx->darkmode_objs[pageNo].obj[i];
        float x0 = obj.rect[0] / full_w;
        float x1 = obj.rect[1] / full_w;
        float y0 = obj.rect[2] / full_h;
        float y1 = obj.rect[3] / full_h;


        std::string a = std::to_string(i) + ":" + std::to_string(obj.type) + "_" + std::to_string(obj.invert);
        response.addFloat(x0);
        response.addFloat(y0);
        response.addFloat(x1);
        response.addFloat(y1);
        response.addIpcString(a.c_str(), true);
    }
    */
    //processText((int) pageNo, response);
}

fz_page* MuPdfBridge::getPage(uint32_t index, bool decode)
{
	if (index >= pageCount || index < 0) {
		LE("Invalid page number: %d from %d", index, pageCount);
		return nullptr;
	}
    if (pages[index] == nullptr) {
        fz_try(ctx) {
            pages[index] = fz_load_page(ctx, document, index);
        } fz_catch(ctx) {
            const char* msg = fz_caught_message(ctx);
            LE("%s", msg);
            if (ctx->erapdf_linearized_load) {
            	LE("Try to reopen in non-linearized mode");
            	ctx->erapdf_linearized_load = 0;
            	restart();
            	return getPage(index, decode);
            }
            return nullptr;
        }
    }
    if (decode && pageLists[index] == nullptr) {
        fz_device* dev = nullptr;
        fz_try(ctx) {
            pageLists[index] = fz_new_display_list(ctx);
            dev = fz_new_list_device(ctx, pageLists[index]);
            fz_matrix m = fz_identity;
            fz_run_page(ctx, pages[index], dev, &m, nullptr);
        }fz_always(ctx) {
            fz_drop_device(ctx, dev);
        } fz_catch(ctx) {
            const char* msg = fz_caught_message(ctx);
            LE("%s", msg);
            if (ctx->erapdf_linearized_load) {
            	LE("Try to reopen in non-linearized mode");
            	ctx->erapdf_linearized_load = 0;
            	restart();
            	return getPage(index, decode);
            } else {
            	fz_drop_display_list(ctx, pageLists[index]);
            	pageLists[index] = nullptr;
            }
        }
    }
    return pages[index];
}

bool MuPdfBridge::restart()
{
    release();
    LD("Creating context: storememory = %d", storememory);
    ctx = fz_new_context(nullptr, nullptr, storememory);
    if (!ctx) {
        return false;
    }
    ctx->erapdf_ignore_most_errors = 1;
    fz_try(ctx) {
        if (format == FORMAT_XPS) {
            document = (fz_document*) xps_open_document_with_stream(ctx, fz_open_fd(ctx, dup(fd)));
        } else {
            document = (fz_document*) pdf_open_document_with_stream(ctx, fz_open_fd(ctx, dup(fd)));
        }
        LD("Document pages: %d", pageCount);
        pages = (fz_page**) calloc(pageCount, sizeof(fz_page*));
        pageLists = (fz_display_list**) calloc(pageCount, sizeof(fz_display_list*));
    } fz_catch(ctx) {
        const char* msg = fz_caught_message(ctx);
        LE("%s", msg);
        return false;
    }
    if (fz_needs_password(ctx, document)) {
        LD("Document required a password: %s", password);
        if (password && strlen(password)) {
            int ok = fz_authenticate_password(ctx, document, password);
            if (!ok) {
                LE("Wrong password given");
                return false;
            }
        } else {
            LE("Document needs a password!");
            return false;
        }
    }
    applyLayersMask();
    return true;
}

void MuPdfBridge::release()
{
    if (pageLists != nullptr) {
        for (int i = 0; i < pageCount; i++) {
            if (pageLists[i] != nullptr) {
                fz_drop_display_list(ctx, pageLists[i]);
            }
        }
        free(pageLists);
        pageLists = nullptr;
    }
    if (pages != nullptr) {
        for (int i = 0; i < pageCount; i++) {
            if (pages[i] != nullptr) {
                fz_drop_page(ctx, pages[i]);
            }
        }
        free(pages);
        pages = nullptr;
    }
    if (document) {
        fz_drop_document(ctx, document);
        document = nullptr;
    }
    if (ctx) {
        fz_flush_warnings(ctx);
        fz_drop_context(ctx);
        ctx = nullptr;
    }
}

void MuPdfBridge::applyLayersMask()
{
    if (document && format == FORMAT_PDF) {
        pdf_ocg_descriptor* ocg;
        ocg = ((pdf_document*) document)->ocg;
        if (ocg) {
            for (int i = 0; i < std::min(31, ocg->len); i++) {
                ocg->ocgs[i].state = ((layersmask & (1 << i)) != 0) ? 1 : 0;
            }
        }
    }
}

void MuPdfBridge::processGetLayersList(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PDF_GET_LAYERS_LIST;

    if (format != FORMAT_PDF || !document)
    {
        response.result = RES_ILLEGAL_STATE;
        return;
    }

    pdf_document* doc = (pdf_document*) document;

	pdf_ocg_descriptor* ocg;
	ocg = ((pdf_document*) document)->ocg;
	if (ocg)
	{
	    response.addInt(ocg->len);
		for (int i = 0; i < ocg->len; i++)
		{
			pdf_obj* obj = pdf_load_object(ctx, (pdf_document*)document, ocg->ocgs[i].num, ocg->ocgs[i].gen);

			char *name;
			name = pdf_to_utf8(ctx, (pdf_document*)document, pdf_dict_get(ctx, obj, PDF_NAME_Name));

	    	response.addIpcString(name, false);

			pdf_drop_obj(ctx, obj);
		}
	}
	else
	{
	    response.addInt(0);
	}
}

void MuPdfBridge::processPageLinks(CmdRequest& request, CmdResponse& response)
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
    if (document == nullptr)
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

std::vector<Hitbox> MuPdfBridge::GetHitboxesBetweenXpaths(int page,
        std::string xpStart, std::string xpEnd, int startPage, int version)
{
    std::vector<Hitbox> result;
    std::vector<Hitbox> hitboxes = processTextToArray(page, version);
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

void MuPdfBridge::processPageRangeText(CmdRequest &request, CmdResponse &response) {

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
        LE("processPageRangeText bad request data : invalid key string [%s]",
                inputstr.c_str());
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    uint32_t page = std::atoi(in_arr.at(0).c_str());
    std::string startstr = in_arr.at(1);
    std::string endstr = in_arr.at(2);
    std::string versionstr = in_arr.at(3);

    int pos1 = startstr.find("[") +1;
    int pos2 = startstr.find("]");
    std::string startpagestr = startstr.substr(pos1,pos2-pos1);
    int startPage = std::atoi(startpagestr.c_str());
    int version = CURRENT_MAX_VERSION;
    if(!versionstr.empty())
    {
        version = std::atoi(versionstr.c_str());
    }
    //LE(" xp = %s",startstr.c_str());
    //LE(" page = %d , startpage = %d",page,startPage);
    //LE(" version = %d",version);
    std::vector<Hitbox> BookmarkHitboxes = GetHitboxesBetweenXpaths(page, startstr, endstr,
            startPage, version);
    BookmarkHitboxes = unionRects(BookmarkHitboxes,false);

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

bool MuPdfBridge::renderPage(uint32_t index, int w, int h, unsigned char* pixels,
        const fz_matrix_s* ctm)
{
    fz_page* page = getPage(index, true);
    if (!page || !pageLists[index]) {
        return false;
    }
    if(ctx->erapdf_nightmode || ctx->erapdf_twilight_mode)
    {
        //LW("RENDER AnalyzePageforDarkMode, index = %d",index);

        //we need to get fresh page bounds because this method can be called from pagecrop
        fz_rect bounds = fz_empty_rect;
        fz_bound_page(ctx, page, &bounds);

        int full_w = fabs(bounds.x1 - bounds.x0);
        int full_h = fabs(bounds.y1 - bounds.y0);
        analyzePageForDarkMode(ctx,index,full_w,full_h);
    }
    // Clear counter
    ctx->erapdf_setcolor_per_page = 0;
    bool res = false;
    fz_rect area;
    area.x0 = 0;
    area.y0 = 0;
    area.x1 = w;
    area.y1 = h;
    fz_device* device = nullptr;
    fz_pixmap* pixmap = nullptr;
    fz_try(ctx) {
        pixmap = fz_new_pixmap_with_data(ctx, fz_device_rgb(ctx), w, h, pixels);
        int value = (ctx->erapdf_twilight_mode)? ((int)(ctx->f_bg_color[0] * 256)) : 0xFF;
        fz_clear_pixmap_with_value(ctx, pixmap, value);
        device = fz_new_draw_device(ctx, pixmap);
        fz_run_display_list(ctx, pageLists[index], device, ctm, &area, nullptr,index);
        res = true;
    } fz_catch(ctx) {
        const char* msg = fz_caught_message(ctx);
        LE("%s", msg);
        res = false;
    }
    fz_drop_device(ctx, device);
    fz_drop_pixmap(ctx, pixmap);
    return res;
}

void MuPdfBridge::processSmartCrop(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_SMART_CROP;
    if (document == nullptr || pages == nullptr) {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
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
#ifdef DEBUG_INTENTIONAL_CRASH
    if (page_index == 9) {
        debug_generate_sigsegv_segv_maperr();
    }
#endif
    float slice_w = slice_r - slice_l;
    float slice_h = slice_b - slice_t;
    LI("origin_w=%f origin_h=%f slices[%f %f]",	origin_w, origin_h, slice_l, slice_r);

    float lambda = origin_h / origin_w;
    int smart_S = smart_crop_w * smart_crop_h;

    int new_w = sqrt(smart_S / lambda);
    int new_h = new_w * lambda;

    fz_matrix ctm = fz_identity;
    fz_pre_scale(&ctm, (float) new_w / origin_w, (float) new_h / origin_h);
    // Manual translating (fz_pre_translate and fz_translate works not as expected)
    ctm.e = -slice_l * (float) new_w; // Translate left
    ctm.f = -slice_t * (float) new_h; // Translate top
    fz_pre_scale(&ctm, 1 / (slice_w), 1 / (slice_h));
    // Manual applying scale to previous translate (fz_pre_scale and fz_scale ignores translation)
    ctm.e *= 1 / (slice_w); // Scale translate left
    ctm.f *= 1 / (slice_h); // Scale translate top
    //LI("fz_matrix[a=%f b=%f c=%f d=%f e=%f f=%f]", ctm.a, ctm.b, ctm.c, ctm.d, ctm.e, ctm.f);
    ctx->erapdf_twilight_mode = 0;
    ctx->erapdf_nightmode = 0;
    ctx->erapdf_slowcmyk = 0;
    ctx->flag_interpolate_images = 0;
    int size = new_w * new_h * 4;
    auto pixels = (uint8_t *) malloc(size);

    bool result = renderPage(page_index, new_w, new_h, pixels, &ctm);
    if (!result)
    {
        response.result = RES_INTERNAL_ERROR;
        return;
    }
    float smart_crop[4] = {0, 0, 1, 1};

    CalcBitmapSmartCrop(smart_crop, pixels, new_w, new_h, slice_l, slice_t, slice_r, slice_b);
    response.addFloatArray(4, smart_crop, true);

    if (pixels) { free(pixels); }
}

void MuPdfBridge::processPageRender(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_PAGE_RENDER;
    if (document == nullptr || pages == nullptr) {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }
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
#ifdef DEBUG_INTENTIONAL_CRASH
    if (page_index == 7) {
        debug_generate_sigsegv_segv_maperr();
    }
#endif
    // fz_matrix is a a row-major 3x3 matrix used for representing transformations of coordinates
    // throughout MuPDF. Since all points reside in a two-dimensional space, one vector is always
    // a constant unit vector; hence only some elements may	vary in a matrix. Below is how the
    // elements map between	different representations.
    //
    //  a - scale width
    //  b - ?
    //  c - ?
    //  d - scale height
    //  e - translate left
    //  f - translate top
    //
    //  / a b 0 \
    //  | c d 0 | normally represented as [ a b c d e f ].
    //  \ e f 1 /
    //
    //  typedef struct fz_matrix_s fz_matrix;
    //  struct fz_matrix_s { float a, b, c, d, e, f; };
    fz_matrix ctm = fz_identity;
    ctm.a = matrix[0];
    ctm.b = matrix[1];
    ctm.c = matrix[2];
    ctm.d = matrix[3];
    ctm.e = matrix[4];
    ctm.f = matrix[5];
    if (preview) {
        ctx->erapdf_slowcmyk = 0;
        ctx->flag_interpolate_images = 0;
    } else {
        ctx->erapdf_slowcmyk = HARDCONFIG_MUPDF_SLOW_CMYK;
        ctx->flag_interpolate_images = 1;
    }
    //LW("RENDER page %d , preview = %d, [%d x %d]",page_index,preview,w,h);
    ctx->erapdf_nightmode = config_invert_images;
    eraConfig.applyToCtx(ctx);
    auto pixelsHolder = new CmdData();
    int size = (w) * (h) * 4;
    auto pixels = pixelsHolder->newByteArray(size);
    if (renderPage(page_index, w, h, pixels, &ctm)) {
        response.addData(pixelsHolder);
    } else {
        response.result = RES_INTERNAL_ERROR;
        delete pixelsHolder;
    }
}