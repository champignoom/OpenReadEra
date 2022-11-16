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

#include <cstdlib>

#include "ore_log.h"
#include "StProtocol.h"

#include "EraDjvuBridge.h"

constexpr static bool LOG = false;
const char EMPTY_URL[1] = { 0x0 };

#define PAGE_LINK 1
#define URI_LINK 2

bool number_from_miniexp(miniexp_t sexp, int *number)
{
    if (miniexp_numberp(sexp))
    {
        *number = miniexp_to_int(sexp);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool string_from_miniexp(miniexp_t sexp, const char **str)
{
    if (miniexp_stringp(sexp))
    {
        *str = miniexp_to_str(sexp);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool getLinkArea(int pageNo, ddjvu_pageinfo_t *pi, miniexp_t sexp, float* data, int &type)
{
    miniexp_t iter = sexp;

    // LDD(LOG, "DjvuLinks: Hyperlink area %s", miniexp_to_name(miniexp_car(sexp)));

    if (miniexp_car(iter) == miniexp_symbol("rect"))
    {
        type = 1;
    }
    else if (miniexp_car(iter) == miniexp_symbol("oval"))
    {
        type = 2;
    }
    else if (miniexp_car(iter) == miniexp_symbol("poly"))
    {
        type = 3;
        LDD(LOG, "DjvuLinks: Page %d: Hyperlink area poly not supported", pageNo);
        return false;
    }
    else
    {
        LDD(LOG, "DjvuLinks: Page %d: Hyperlink area %s cannot be processed",
                pageNo, miniexp_to_name(miniexp_car(sexp)));
        return false;
    }

    int len = miniexp_length(iter);
    if (len < 4)
    {
        LDD(LOG, "DjvuLinks: Page %d: Bad Hyperlink area data length: %d %d", pageNo, type, len);
        return false;
    }

    int array[4];

    int i = 0;
    iter = miniexp_cdr(iter);
    while (iter != miniexp_nil && i < 4)
    {
        if (!number_from_miniexp(miniexp_car(iter), array + i))
        {
            LDD(LOG, "DjvuLinks: Page %d: Bad Hyperlink area data", pageNo);
            break;
        }
        iter = miniexp_cdr(iter);
        i++;
    }

    if (i != 4)
    {
        LDD(LOG, "DjvuLinks: Page %d: Bad Hyperlink area actual data length: %d", pageNo, len);
        return false;
    }

    float width = (float) (pi->width);
    float height = (float) (pi->height);

    data[0] = array[0] / width;
    data[1] = 1.0f - (array[1] + array[3]) / height;
    data[2] = (array[0] + array[2]) / width;
    data[3] = 1.0f - array[1] / height;

    return true;
}

void parseLink(ddjvu_document_t *doc, int pageNo, ddjvu_pageinfo_t *pi, miniexp_t sexp, CmdResponse& response)
{
    miniexp_t iter = sexp;
    if (miniexp_car(iter) != miniexp_symbol("maparea"))
    {
        LE("Page %d: Unknown hyperlink %s", pageNo, miniexp_to_name(miniexp_car(sexp)));
        return;
    }

    iter = miniexp_cdr(iter);

    const char *url, *url_target;

    if (miniexp_caar(iter) == miniexp_symbol("url"))
    {
        if (!string_from_miniexp(miniexp_cadr(miniexp_car(iter)), &url))
        {
            LE("Page %d: Unknown hyperlink %s", pageNo, miniexp_to_name(miniexp_car(sexp)));
            return;
        }
        if (!string_from_miniexp(miniexp_caddr(miniexp_car(iter)), &url_target))
        {
            LE("Page %d: Unknown hyperlink %s", pageNo, miniexp_to_name(miniexp_car(sexp)));
            return;
        }
    }
    else
    {
        if (!string_from_miniexp(miniexp_car(iter), &url))
        {
            LE("Page %d: Unknown hyperlink %s", pageNo, miniexp_to_name(miniexp_car(sexp)));
            return;
        }
        url_target = nullptr;
    }
    if (url == nullptr && url_target == nullptr)
    {
        LDD(LOG, "DjvuLinks: Page %d: No Hyperlink url or target", pageNo);
        return;
    }
    int targetPage = -1;

    if (url && url[0] == '#')
    {
        int number = ddjvu_document_search_pageno(doc, url + 1);
        targetPage = number >= 0 ? number : -1;
        LDD(LOG, "DjvuLinks: Page %d: target page: %d", pageNo, targetPage);
    }

    iter = miniexp_cdr(iter);
    /* FIXME: DjVu hyperlink comments are ignored */

    iter = miniexp_cdr(iter);

    int type = 0;
    float data[4];
    if (!getLinkArea(pageNo, pi, miniexp_car(iter), data, type))
    {
        return;
    }

    LDD(LOG, "DjvuLinks: Page %d: %d %s %s %f %f %f %f", pageNo, type, url, url_target, data[0], data[1], data[2], data[3]);

    if (targetPage != -1)
    {
        response.addWords(PAGE_LINK, targetPage);
        response.addFloat(data[0]);
        response.addFloat(data[1]);
        response.addFloat(data[2]);
        response.addFloat(data[3]);
        response.addFloat(.0f);
        response.addFloat(.0f);
    }
    else
    {
        response.addWords(URI_LINK, 0);
        response.addIpcString(url_target != nullptr ? url : EMPTY_URL, false);
        response.addFloat(data[0]);
        response.addFloat(data[1]);
        response.addFloat(data[2]);
        response.addFloat(data[3]);
    }
}

void DjvuBridge::processLinks(int pageNo, CmdResponse& response)
{
    miniexp_t page_annotations = miniexp_nil;
    miniexp_t *hyperlinks = nullptr, *iter = nullptr;

    page_annotations = ddjvu_document_get_pageanno(doc, pageNo);
    if (!page_annotations)
    {
        LDD(LOG, "DjvuLinks: No page annotation found");
        return;
    }

    hyperlinks = ddjvu_anno_get_hyperlinks(page_annotations);
    if (!hyperlinks)
    {
        LDD(LOG, "DjvuLinks: No links found");
        ddjvu_miniexp_release(doc, page_annotations);
        return;
    }

    for (iter = hyperlinks; *iter; ++iter)
    {
        parseLink(doc, pageNo, info[pageNo], *iter, response);
    }

    free(hyperlinks);
    ddjvu_miniexp_release(doc, page_annotations);
}

