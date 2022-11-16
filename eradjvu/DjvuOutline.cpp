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
#include <cstring>

#include "ore_log.h"
#include "StProtocol.h"

#include "DjvuOutline.h"

constexpr static bool LOG = false;
const char EMPTY_TITLE[1] = { 0x0 };

#define PAGE_LINK 1
#define URI_LINK 2

DjvuOutlineItem::DjvuOutlineItem(ddjvu_document_t* doc, int level, int index, miniexp_t& expr)
{
    this->level = level;
    this->index = index;
    this->pageNo = -1;
    this->title = nullptr;

    this->firstChild = nullptr;
    this->nextItem = nullptr;

    LDD(LOG, "DjvuOutline: [%d:%d] Outline item found", this->level, this->index);

    miniexp_t item = miniexp_car(expr);

    miniexp_t head = miniexp_car(item);
    if (miniexp_stringp(head))
    {
        const char* buf = miniexp_to_str(head);
        this->title = buf == nullptr ? nullptr : strdup(buf);
        LDD(LOG, "DjvuOutline: [%d:%d] Title: %s", this->level, this->index, this->title);
    }
    else
    {
        LDD(LOG, "DjvuOutline: [%d:%d] No title", this->level, this->index);
    }

    miniexp_t tail = miniexp_cdr(item);
    if (miniexp_consp(tail))
    {
        miniexp_t tailHead = miniexp_car(tail);
        if (miniexp_stringp(tailHead))
        {
            const char *link = miniexp_to_str(tailHead);
            if (link && link[0] == '#')
            {
                int number = ddjvu_document_search_pageno(doc, link + 1);
                this->pageNo = number >= 0 ? number : -1;
                LDD(LOG, "DjvuOutline: [%d:%d] PageNo: %d", this->level, this->index, this->pageNo);
            }
            else
            {
                LDD(LOG, "DjvuOutline: [%d:%d] Unknown link: %s", this->level, this->index, link);
            }
        }
        else
        {
            LDD(LOG, "DjvuOutline: [%d:%d] No pageNo", this->level, this->index);
        }

        miniexp_t tailTail = miniexp_cdr(tail);
        if (miniexp_consp(tailTail))
        {
            this->firstChild = new DjvuOutlineItem(doc, level + 1, 0, tailTail);
        }
    }
    else
    {
        LDD(LOG, "DjvuOutline: [%d:%d] No pageNo and children", this->level, this->index);
    }

    miniexp_t next = miniexp_cdr(expr);
    if (miniexp_consp(next))
    {
        this->nextItem = new DjvuOutlineItem(doc, level, index + 1, next);
    }
}

DjvuOutlineItem::~DjvuOutlineItem()
{
    if (title)
    {
        free(title);
        title = nullptr;
    }

    if (firstChild != nullptr)
    {
        delete firstChild;
        firstChild = nullptr;
    }

    if (nextItem == nullptr)
    {
        delete nextItem;
        nextItem = nullptr;
    }
}

void DjvuOutlineItem::toResponse(CmdResponse& response)
{
    response.addWords((uint16_t) PAGE_LINK, (uint16_t) this->pageNo);
	response.addInt((uint32_t) this->level);
	response.addIpcString(this->title != nullptr ? this->title : EMPTY_TITLE, false);
    response.addFloat(.0f).addFloat(.0f);

    LDD(LOG, "DjvuOutline: [%d:%d] %d", this->level, this->index, response.dataCount);

    if (firstChild != nullptr)
    {
        firstChild->toResponse(response);
    }
    if (nextItem != nullptr)
    {
        nextItem->toResponse(response);
    }
}

DjvuOutline::DjvuOutline(ddjvu_document_t* doc)
{
    this->firstPageIndex = 0;
    this->firstItem = nullptr;

    miniexp_t outline = ddjvu_document_get_outline(doc);
    if (outline && outline != miniexp_dummy )
    {
        if (!miniexp_consp(outline) || miniexp_car(outline) != miniexp_symbol("bookmarks"))
        {
            LE("Outline data is corrupted");
            return;
        }
        else
        {
            LDD(LOG, "DjvuOutline: Outline found");
            this->firstItem = new DjvuOutlineItem(doc, 0, 0, outline);
        }
    }
    else
    {
        LDD(LOG, "DjvuOutline: Outline not found");
    }
}

DjvuOutline::~DjvuOutline()
{
    if (firstItem != nullptr)
    {
        delete firstItem;
        firstItem = nullptr;
    }
}

void DjvuOutline::toResponse(CmdResponse& response)
{
    if (firstItem != nullptr)
    {
        firstItem->toResponse(response);
    }
}

