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

#include "ore_log.h"
#include "StProtocol.h"

#include "EraPdfBridge.h"

constexpr static bool LOG = false;

#define ERAPDF_PAGE_LINK 1
#define ERAPDF_URI_LINK 2
#define ERAPDF_OUTER_PAGE_LINK 3
#define ERAPDF_LAUNCH_LINK 4

const char EMPTY_URL[1] = { 0x0 };

void MuPdfBridge::processLinks(int pageNo, CmdResponse& response)
{
    fz_page* page = getPage(pageNo, false);
    if (!page)
    {
        return;
    }

    fz_link* link = fz_load_links(ctx, page);
    if (!link)
    {
        LDD(LOG, "PdfLinks: No links found on page %d", pageNo);
        return;
    }

    do
    {
        if (link->dest.kind == FZ_LINK_URI)
        {
            fz_rect sourceBounds = fz_empty_rect;
            fz_bound_page(ctx, page, &sourceBounds);

            float sourceWidth = sourceBounds.x1 - sourceBounds.x0;
            float sourceHeight = sourceBounds.y1 - sourceBounds.y0;
            float left = (link->rect.x0 - sourceBounds.x0) / sourceWidth;
            float top = (link->rect.y0 - sourceBounds.y0) / sourceHeight;
            float right = (link->rect.x1 - sourceBounds.x0) / sourceWidth;
            float bottom = (link->rect.y1 - sourceBounds.y0) / sourceHeight;

            LDD(LOG, "PdfLinks: load: Source rect on page %d: %f %f %f %f / %f %f %f %f",
                    pageNo, link->rect.x0, link->rect.y0, link->rect.x1, link->rect.y1,
                    sourceBounds.x0, sourceBounds.y0, sourceBounds.x1, sourceBounds.y1);
            LDD(LOG, "PdfLinks: Uri link found on page %d: %s", pageNo, link->dest.ld.uri.uri);

            char* uri = link->dest.ld.uri.uri;
            response.addWords((uint16_t) ERAPDF_URI_LINK, 0);
            response.addIpcString(uri != NULL ? uri : EMPTY_URL, false);
            response.addFloat(left);
            response.addFloat(top);
            response.addFloat(right);
            response.addFloat(bottom);
        }
        else if (link->dest.kind == FZ_LINK_LAUNCH)
        {
            fz_rect sourceBounds = fz_empty_rect;
            fz_bound_page(ctx, page, &sourceBounds);

            float sourceWidth = sourceBounds.x1 - sourceBounds.x0;
            float sourceHeight = sourceBounds.y1 - sourceBounds.y0;
            float left = (link->rect.x0 - sourceBounds.x0) / sourceWidth;
            float top = (link->rect.y0 - sourceBounds.y0) / sourceHeight;
            float right = (link->rect.x1 - sourceBounds.x0) / sourceWidth;
            float bottom = (link->rect.y1 - sourceBounds.y0) / sourceHeight;

            LDD(LOG, "PdfLinks: load: Source rect on page %d: %f %f %f %f / %f %f %f %f",
                    pageNo, link->rect.x0, link->rect.y0, link->rect.x1, link->rect.y1,
                    sourceBounds.x0, sourceBounds.y0, sourceBounds.x1, sourceBounds.y1);
            LDD(LOG, "PdfLinks: Launch link found on page %d: %s", pageNo, link->dest.ld.launch.file_spec);

            char* uri = link->dest.ld.launch.file_spec;
            response.addWords((uint16_t) ERAPDF_LAUNCH_LINK, 0);
            response.addIpcString(uri != NULL ? uri : EMPTY_URL, false);
            response.addFloat(left);
            response.addFloat(top);
            response.addFloat(right);
            response.addFloat(bottom);
        }else if (link->dest.kind == FZ_LINK_GOTO)
        {
            int targetNo = link->dest.ld.gotor.page;
            fz_page* target = getPage(targetNo, false);

            if (target)
            {
                fz_rect sourceBounds = fz_empty_rect;
                fz_rect targetBounds = fz_empty_rect;
                fz_bound_page(ctx, page, &sourceBounds);
                fz_bound_page(ctx, target, &targetBounds);

                float sourceWidth = sourceBounds.x1 - sourceBounds.x0;
                float sourceHeight = sourceBounds.y1 - sourceBounds.y0;
                float left = (link->rect.x0 - sourceBounds.x0) / sourceWidth;
                float top = (link->rect.y0 - sourceBounds.y0) / sourceHeight;
                float right = (link->rect.x1 - sourceBounds.x0) / sourceWidth;
                float bottom = (link->rect.y1 - sourceBounds.y0) / sourceHeight;

                LDD(LOG, "PdfLinks: load: Source rect on page %d: %f %f %f %f / %f %f %f %f",
                        pageNo, link->rect.x0, link->rect.y0, link->rect.x1, link->rect.y1,
                        sourceBounds.x0, sourceBounds.y0, sourceBounds.x1, sourceBounds.y1);

                float targetWidth = targetBounds.x1 - targetBounds.x0;
                float targetHeight = targetBounds.y1 - targetBounds.y0;
                float x = 0;
                float y = 0;
                if ((link->dest.ld.gotor.flags & 0x03) != 0)
                {
                    x = link->dest.ld.gotor.lt.x / targetWidth;
                    y = 1.0f - link->dest.ld.gotor.lt.y / targetHeight;
                }

                LDD(LOG, "PdfLinks: load: Target rect on page %d: %x %f %f %f %f / %f %f %f %f",
                        targetNo, link->dest.ld.gotor.flags,
                        link->dest.ld.gotor.lt.x, link->dest.ld.gotor.lt.y,
                        link->dest.ld.gotor.rb.x, link->dest.ld.gotor.rb.y,
                        targetBounds.x0, targetBounds.y0, targetBounds.x1, targetBounds.y1);

                response.addWords((uint16_t) ERAPDF_PAGE_LINK, (uint16_t) link->dest.ld.gotor.page);
                response.addFloat(left);
                response.addFloat(top);
                response.addFloat(right);
                response.addFloat(bottom);
                response.addFloat(x);
                response.addFloat(y);

                LDD(LOG, "PdfLinks: load: Page link found on page %d: %f %f %f %f -> %d %f %f",
                        pageNo, left, top, right, bottom, targetNo, x, y);
            }
            else
            {
                LDD(LOG, "PdfLinks: No target link page %d found", targetNo);
            }
        }
        else if (link->dest.kind == FZ_LINK_GOTOR)
        {
            int targetNo = link->dest.ld.gotor.page;
            fz_page* target = getPage(targetNo, false);

            if (target)
            {
                const char* targetFile = link->dest.ld.gotor.file_spec;

                fz_rect sourceBounds = fz_empty_rect;
                fz_bound_page(ctx, page, &sourceBounds);

                float sourceWidth = sourceBounds.x1 - sourceBounds.x0;
                float sourceHeight = sourceBounds.y1 - sourceBounds.y0;
                float left = (link->rect.x0 - sourceBounds.x0) / sourceWidth;
                float top = (link->rect.y0 - sourceBounds.y0) / sourceHeight;
                float right = (link->rect.x1 - sourceBounds.x0) / sourceWidth;
                float bottom = (link->rect.y1 - sourceBounds.y0) / sourceHeight;

                LDD(LOG, "PdfLinks: load: Source rect on page %d: %f %f %f %f / %f %f %f %f",
                    pageNo, link->rect.x0, link->rect.y0, link->rect.x1, link->rect.y1,
                    sourceBounds.x0, sourceBounds.y0, sourceBounds.x1, sourceBounds.y1);

                float x = 0;
                float y = 0;
                if ((link->dest.ld.gotor.flags & 0x03) != 0)
                {
                    x = link->dest.ld.gotor.lt.x;
                    y = -link->dest.ld.gotor.lt.y;
                }

                LDD(LOG, "PdfLinks: load: Target rect on page %s:%d: %x %f %f %f %f",
                        targetFile, targetNo, link->dest.ld.gotor.flags,
                        link->dest.ld.gotor.lt.x, link->dest.ld.gotor.lt.y,
                        link->dest.ld.gotor.rb.x, link->dest.ld.gotor.rb.y);

                response.addWords((uint16_t) ERAPDF_OUTER_PAGE_LINK, (uint16_t) link->dest.ld.gotor.page);
                response.addFloat(left);
                response.addFloat(top);
                response.addFloat(right);
                response.addFloat(bottom);
                response.addFloat(x);
                response.addFloat(y);
                response.addIpcString(targetFile != NULL ? targetFile : EMPTY_URL, false);

                LDD(LOG, "PdfLinks: load: Remote page link found on page %d: %f %f %f %f -> %s:%d %f %f",
                        pageNo, left, top, right, bottom, targetFile, targetNo, x, y);
            }
            else
            {
                LDD(LOG, "PdfLinks: No target link page %d found", targetNo);
            }
        }
        else
        {
            LDD(LOG, "PdfLinks: Unknown link kind %d found on page %d", link->dest.kind, pageNo);
        }
        link = link->next;
    } while (link);
}

