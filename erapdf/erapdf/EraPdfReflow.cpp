/******************************************************************************
	Copyright (C) 2020 READERA LLC OpenReadEra - Ebook renderer

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
******************************************************************************/
//
// Created by Tarasus on 06.05.2020.
//

#include "EraPdfReflow.h"

void PageStats::analyzePage()
{
    float pageArea = (bounds.x1 - bounds.x0)*(bounds.y1 - bounds.y0);
    for (int block_n = 0; block_n < page->len; block_n++)
    {

        switch (page->blocks[block_n].type)
        {
            case FZ_PAGE_BLOCK_IMAGE:
            {
                fz_image_block *imgBlock = page->blocks[block_n].u.image;
                fz_rect rect = imgBlock->bbox;
                float blockArea = (rect.x1 - rect.x0)*(rect.y1 - rect.y0);
                if (blockArea >= pageArea * 0.7)
                    hasFullPageImage = true;

                break;
            }
            case FZ_PAGE_BLOCK_TEXT:
            {
                fz_text_block *block = page->blocks[block_n].u.text;
                for (int line_n = 0; line_n < block->len; line_n++)
                {
                    fz_text_line *temp_line = &block->lines[line_n];
                    for (fz_text_span *temp_span = temp_line->first_span; temp_span; temp_span = temp_span->next)
                    {
                        if (temp_span->bbox.y0 < top_span_y)
                        {
                            top_span_y = temp_span->bbox.y0;
                            topSpanBlockNum = block_n;
                        }
                        if (temp_span->bbox.y0 > bottom_span_y)
                        {
                            bottom_span_y = temp_span->bbox.y0;
                            bottomSpanBlockNum = block_n;
                        }

                        for (int ch_n = 0; ch_n < temp_span->len; ch_n++)
                        {
                            charnum++;
                            fz_text_char *temp_ch = &temp_span->text[ch_n];
                            //float size = temp_ch->style->size;// rounding?
                            //LE("char = [%lc], size = %f",temp_ch->c,size);
                            if (heightsMap.find(temp_ch->style->size) == heightsMap.end())
                            {
                                heightsMap[temp_ch->style->size] = 1;
                            }
                            else
                            {
                                heightsMap[temp_ch->style->size]++;
                            }
                        }
                    }
                }
            }
            default:
                break;
        }
    }
    banBlocks();
    if (charnum < 300)
    {
        return;
    }
    if(heightsMap.size() == 1)
    {
        heightsMap.clear();
        return;
    }

    float h1_ = -1;
    float h2_ = -1;
    for (auto & it : heightsMap)
    {
        h1_ = max(h1_, it.first);
    }
    auto it = heightsMap.find(h1_);

    if(it != heightsMap.begin())
    {
        if(it->second > charnum * 0.3)
        {
            return;
        }
        it--;                   //step backward
        if (it->second < charnum * 0.1)
        {
            h2_ = it->first;
        }
    }
    //LE("h1 = %f, h2 = %f",h1_,h2_)
    heightsMap.clear();
    h1 = h1_;
    h2 = h2_;
}

void PageStats::banBlocks()
{
    if(page->len < 3)
    {
        return;
    }
    if (topSpanBlockNum == -1 || bottomSpanBlockNum == -1)
    {
        return;
    }

    fz_text_block *block1 = page->blocks[topSpanBlockNum].u.text;
    fz_text_block *block2 = page->blocks[bottomSpanBlockNum].u.text;

    int charnum1 = 0;
    int charnum2 = 0;

    if(block1->len == 1)
    {
        fz_text_line *temp_line = &block1->lines[0];
        for (fz_text_span *temp_span = temp_line->first_span; temp_span; temp_span = temp_span->next)
        {
            charnum1+= temp_span->len;
        }
        if(charnum1 < 20)
        {
            bannedBlock1 = topSpanBlockNum;
            //LE("Banned top block #%d",bannedBlock1);
        }
    }

    if(block2->len == 1)
    {
        fz_text_line *temp_line = &block2->lines[0];
        for (fz_text_span *temp_span = temp_line->first_span; temp_span; temp_span = temp_span->next)
        {
            charnum2+= temp_span->len;
        }
        if(charnum2 < 10)
        {
            bannedBlock2 = bottomSpanBlockNum;
            //LE("Banned bottom block #%d",bannedBlock2);
        }
    }
}

ReflowManager::ReflowManager(fz_context *ctx, MuPdfBridge *muPdfBridge)
{
    this->muPdfBridge = muPdfBridge;
    this->pageCount = muPdfBridge->pageCount;
    this->ctx = ctx;
    this->out = nullptr;
    pageStatsArray = std::vector<PageStats>(this->pageCount);
}

void ReflowManager::freePage(int index)
{
    if (muPdfBridge->pageLists[index])
    {
        fz_try(ctx)
        {fz_drop_display_list(ctx, muPdfBridge->pageLists[index]);}
        fz_catch(ctx)
        {
            const char *msg = fz_caught_message(ctx);
            LE( "%s", msg);
        }
        muPdfBridge->pageLists[index] = nullptr;
    }
    if (muPdfBridge->pages[index])
    {
        fz_try(ctx)
        {fz_drop_page(ctx, muPdfBridge->pages[index]);}
        fz_catch(ctx)
        {
            const char *msg = fz_caught_message(ctx);
            LE( "%s", msg);
        }
        muPdfBridge->pages[index] = nullptr;
    }
}

void ReflowManager::reflowPage(fz_text_page *page, int pagenum, fz_display_list_s *displayList)
{
    if(pageStatsArray.at(pagenum).isEmpty())
    {
        pageStatsArray.at(pagenum) = PageStats(page,pagenum);
    }
    reflowPageText(page,pagenum,displayList);
}

void ReflowManager::analyzeDocument()
{
    //LE("ANALYZE DOC START");
    ctx->previewmode = 2;
    for (unsigned int i = 0; i < pageCount; i++)
    {
        if (!pageStatsArray.at(i).isEmpty())
        {
            continue;
        }
        fz_page *page = muPdfBridge->getPage(i, true);
        if (page == NULL)
        {
            continue;
        }

        fz_text_sheet *sheet = NULL;
        fz_text_page *pagetext = NULL;
        fz_device *dev = NULL;
        fz_try(ctx)
                {
                    sheet = fz_new_text_sheet(ctx);
                    pagetext = fz_new_text_page(ctx);
                    dev = fz_new_text_device(ctx, sheet, pagetext);
                    dev->hints = 0;
                    fz_run_page(ctx, page, dev, &fz_identity, NULL);
                    pageStatsArray.at(i) = PageStats(pagetext,i);
                    //pageStatsArray.at(i).printStats();
                }
        fz_always(ctx)
                {
                    if (dev)      fz_drop_device(ctx, dev);
                    if (pagetext) fz_drop_text_page(ctx, pagetext);
                    if (sheet)    fz_drop_text_sheet(ctx, sheet);

                    freePage(i);
                }
        fz_catch(ctx)
        {
            const char *msg = fz_caught_message(ctx);
            LE( "Page %d analyze failed : %s",i , msg);
        }
    }
    ctx->previewmode = 0;


    int fullpage_images_count = 0;
    int textCount = 0;
    int pagesHasText = 0;
    for (int i = 0; i < pageStatsArray.size(); i++)
    {
        PageStats curr = pageStatsArray.at(i);

        textCount += curr.charnum;
        if(curr.charnum>0) pagesHasText++;
        if(curr.hasFullPageImage) fullpage_images_count ++;
    }

    LE("| DOC  STATS:"                                       );
    LE("| textCount              = %d", textCount            );
    LE("| pagesHasText           = %d", pagesHasText         );
    LE("| fullpage_images_count  = %d", fullpage_images_count);

    analyzed = true;
    if(textCount <= 0)
    {
        this->doctype = REFLOW_UNSUPPORTED;
        return;
    }
    this->doctype = REFLOW_ERAEPUB;
}

bool ReflowManager::reflowPageMain(int page_index)
{
    fz_printf(ctx, out, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    fz_printf(ctx, out, "<FictionBook xmlns=\"http://www.gribuser.ru/xml/fictionbook/2.0\" xmlns:l=\"http://www.w3.org/1999/xlink\">\n");
    fz_printf(ctx, out, "<body>\n");

    ctx->erapdf_twilight_mode = 0;
    ctx->erapdf_nightmode = 0;

    //LE("Reflowing page %d", page_index);
    fz_page *page = muPdfBridge->getPage(page_index, true);
    if (page == NULL)
    {
        LE("Page %d == NULL", page_index);
        fz_printf(ctx, out, "<br><p><b>PAGE %d REFLOW GEN FAILED</b></p><br>", page_index);
        return false;
    }

    fz_text_sheet *sheet = NULL;
    fz_text_page *pagetext = NULL;
    fz_device *dev = NULL;

    fz_try(ctx)
            {
                fz_rect bounds = fz_empty_rect;
                fz_bound_page(ctx, page, &bounds);

                sheet = fz_new_text_sheet(ctx);
                pagetext = fz_new_text_page(ctx);
                dev = fz_new_text_device(ctx, sheet, pagetext);
                dev->hints = 0;
                fz_run_page(ctx, page, dev, &fz_identity, NULL);
                if (muPdfBridge->pageLists[page_index] != NULL)
                {
                    fz_printf(ctx, out, "<section data-page=\"%d\">\n", page_index);
                    reflowPage(pagetext, page_index, muPdfBridge->pageLists[page_index]);
                    fz_printf(ctx, out, "</section>\n");
                    //LE("Wrote page %d", page_index);
                }
                else
                {
                    LE("Page %d FAILED", page_index);
                    fz_printf(ctx, out, "<br><p><b>PAGE %d REFLOW FAILED</b></p><br>", page_index);
                }
            }
    fz_always(ctx)
            {
                if (dev)
                {
                    fz_drop_device(ctx, dev);
                }
                if (pagetext)
                {
                    fz_drop_text_page(ctx, pagetext);
                }
                if (sheet)
                {
                    fz_drop_text_sheet(ctx, sheet);
                }
            }
    fz_catch(ctx)
    {
        const char *msg = fz_caught_message(ctx);
        LE("%s", msg);
    }
    fz_printf(ctx, out, "</body>\n");
    fz_printf(ctx, out, "</FictionBook>\n");
    LE("Conversion finished ");
    return true;
}