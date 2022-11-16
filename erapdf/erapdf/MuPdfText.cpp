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
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <locale>
#include <regex>

#include "ore_log.h"
#include "StProtocol.h"

#include "EraPdfBridge.h"

constexpr static bool LOG = false;

static char utf8[32 * 1024];
static const char space[3] = {' ',0};
static const char paraend[3] = { '\n',0};
static fz_rect last_linechar;
static int length;

static int pagenum;
static int blocknum;
static int linenum;
static int charnum;

void toResult(std::vector<Hitbox> &result, fz_rect &bounds, fz_irect *rr, const char *str, int len)
{
    float width = bounds.x1 - bounds.x0;
    float height = bounds.y1 - bounds.y0;
    float left = (rr->x0 - bounds.x0) / width;
    float top = (rr->y0 - bounds.y0) / height;
    float right = (rr->x1 - bounds.x0) / width;
    float bottom = (rr->y1 - bounds.y0) / height;
    utf8[length] = 0;

    //if(strcmp(str,space) == 0 && left == right){return;}

    char xpath[100];
    sprintf(xpath, "/page[%d]/block[%d]/line[%d]/char[%d]", pagenum, blocknum, linenum, charnum);

    const std::string restext = std::string(str,len);
    std::wstring wrestext = stringToWstring(restext);
    Hitbox res = Hitbox(left, right, top, bottom, wrestext, xpath);
    result.push_back(res);
}

void toResultParaend(std::vector<Hitbox> &result, fz_rect &bounds, fz_irect *rr)
{
    float width = bounds.x1 - bounds.x0;
    float height = bounds.y1 - bounds.y0;
    float left = (rr->x0 - bounds.x0) / width;
    float top = (rr->y0 - bounds.y0) / height;
    float right = (rr->x1 - bounds.x0) / width;
    float bottom = (rr->y1 - bounds.y0) / height;
    if (right - left > 0.005f) {right = left + 0.005f;}

    std::string str(paraend);
    std::wstring wstr = stringToWstring(str);
    Hitbox res = Hitbox(left, right, top, bottom, wstr, std::string());
    result.push_back(res);
}

bool isStopper(int ch)
{
    return (ch == '.' ||
            ch == '!' ||
            ch == '?' ||
            ch == ':' ||
            ch == ';'   );
}

bool isQuote(int ch)
{
    return  (ch == 0x201D ||
             ch == 0x0022 ||
             ch == 0x2033 ||
             ch == 0x275E ||
             ch == 0x301E   );
}

void processLineToResult_v2(std::vector<Hitbox> &result, fz_context *ctx, fz_rect &bounds, fz_text_line &line)
{
    if(line.first_span == nullptr)
    {
        return;
    }
    charnum = 0;
    int spanIndex;
    fz_text_span *span;
    float lastright = -1.0f;
    float nextleft  = -1.0f;
    bool space = false;
    bool hyph = false;
    bool punct = false;
    bool isChar = false;
    bool isLast = false;
    int last_char0 = -1;
    int last_char1 = -1;

    bool needs_t_b_alignment = false;
    std::vector<Hitbox> subbuf;
    for (span = line.first_span; span != nullptr; span = span->next)
    {
        bool prevspace = false;
        LDD(LOG, "PdfText: processLineToResult: span processing: %d", spanIndex);
        if (span->text && span->len > 0)
        {
            for (int textIndex = 0; textIndex < span->len;)
            {
                fz_rect nextbox;

                if(textIndex+1 < span->len)
                {
                    fz_text_char_bbox(ctx, &nextbox, span, textIndex+1);
                    nextleft  = nextbox.x0;
                }
                else if (span->next!=nullptr)
                {
                    fz_text_char_bbox(ctx, &nextbox, span->next, 0);
                    nextleft  = nextbox.x0;
                }
                else
                {
                    isLast = true;
                    nextleft = bounds.x1;
                }

                fz_text_char& text = span->text[textIndex];
                if (lastright == -1.0)
                {
                    fz_rect bbox;
                    fz_text_char_bbox(ctx, &bbox, span, textIndex);
                    if      (bbox.x1 > bounds.x0 && bbox.x1 < bounds.x1) lastright = bbox.x1;
                }
                space = (text.c == 0x0D) || (text.c == 0x0A) || (text.c == 0x09) || (text.c == 0x20);
                hyph = (text.c == 0x002D);
                punct = char_isPunct(text.c);
                isChar = !punct && !space;
                last_char0 = last_char1;
                last_char1 = text.c;

                if ((prevspace && space) || text.c == '\r')
                {
                    // Do nothing with spaces or '\r'
                    textIndex++;
                    charnum++;
                }
                else
                {
                    fz_rect bbox;
                    fz_text_char_bbox(ctx, &bbox, span, textIndex);
                    float charheight = abs(bbox.y0 - bbox.y1);

                    if(bbox.x0 < 0)
                    {
                        bbox.x0 = lastright;
                    }
                    last_linechar = bbox;
                    lastright = bbox.x1;
                    length = fz_runetochar(utf8, text.c);
                    LDD(LOG, "PdfText: processText: char processing: %d %lc %f %f %f %f",
                            charnum, text.c, bbox.x0, bbox.y0, bbox.x1, bbox.y1);
                    //toResponse(response, bounds, fz_round_rect(&box, &bbox), utf8, 2);
                    // for removing of coordinates rounding if needed

                    // broken right side of bounding box  (too narrow or outside the range)
                    if (bbox.x1 < bbox.x0 + (charheight * 0.1f) || bbox.x1 > bounds.x1)
                    {
                        if(space && isLast)
                        {
                            bbox.x1 = bbox.x0 + (charheight * 0.2f);
                        }
                        else {
                            bbox.x1 = bbox.x0 + (charheight * 0.7f);
                        }
                        if (bbox.x1 > nextleft && nextleft < bounds.x1 && nextleft > bounds.x0)
                        {
                            bbox.x1 = nextleft;
                        }
                    }
                    bool needSpace = false;

                    float charwidth = bbox.x1 - bbox.x0;

                    if (bbox.x1 < nextleft && nextleft < bounds.x1 && nextleft > bounds.x0)
                    {

                        float dist = nextleft - bbox.x1;

                        if (space)
                        {
                            if (dist + charwidth > charheight * 0.7f)
                            {
                                bbox.x1 = nextleft;
                            }
                        }
                        else
                        {
                            if (dist > charheight * 0.7f)
                            {
                                needSpace = true;
                            }
                        }
                    }

                    if(charheight <= 0.1f && charwidth > 0)
                    {
                        needs_t_b_alignment = true;
                        bbox.y0 -= charwidth/2;
                        bbox.y1 += charwidth/2;
                        //update charheight
                        charheight = abs(bbox.y0 - bbox.y1);
                    }


                    fz_irect curr;
                    curr.x0 = bbox.x0;
                    curr.x1 = bbox.x1;
                    curr.y0 = bbox.y0;
                    curr.y1 = bbox.y1 + charheight / 8;
                    last_linechar.x0 = curr.x0;
                    last_linechar.x1 = curr.x1;
                    last_linechar.y0 = curr.y0;
                    last_linechar.y1 = curr.y1;

                    charnum++;
                    toResult(subbuf, bounds, &curr, utf8, length);

                    if(needSpace)
                    {
                        fz_irect space_irect;
                        space_irect.x0 = curr.x1;
                        space_irect.x1 = nextleft;
                        space_irect.y0 = curr.y0;
                        space_irect.y1 = curr.y1;
                        text.c = 0x0020;
                        length = fz_runetochar(utf8, text.c);
                        charnum++;
                        //LE("ADDING SPACE");
                        toResult(subbuf, bounds, &space_irect,utf8, length);
                    }
                    textIndex++;
                }
                prevspace = space;
            }
        }
    }

    if(last_char1 == '\n' || (last_char0 == '\r' && last_char1 == '\n'))
    {
        return;
    }

    bool stopper = isStopper(last_char1);

    if (!stopper && (last_char1 == ' ' || isQuote(last_char1)))
    {
        stopper = isStopper(last_char0);
    }

    fz_irect irect = fz_empty_irect;
    fz_rect rect = fz_empty_rect;
    rect.y0 = last_linechar.y0;
    rect.y1 = last_linechar.y1;
    float height = abs(rect.y1 - rect.y0);
    rect.x0 = last_linechar.x1;
    rect.x1 = rect.x0 + (height * 0.2f);

    if(stopper)
    {
        charnum++;
        toResultParaend(subbuf, bounds, fz_round_rect(&irect,&rect));
    }
    else if (!space && !hyph)
    {
        fz_text_char &text = line.first_span->text[0];
        text.c = 0x0020;
        length = fz_runetochar(utf8, text.c);
        charnum++;
        toResult(subbuf, bounds, fz_round_rect(&irect,&rect),utf8,length);
    }

    if(needs_t_b_alignment)
    {
        float min_t = 10000;
        float max_b = -10000;
        for (auto &hb : subbuf)
        {
            float h = abs(hb.bottom_ - hb.top_);
            min_t = (hb.top_ < min_t) ? hb.top_ : min_t;
            max_b = (hb.bottom_ > max_b) ? hb.bottom_ : max_b;
        }
        //LE("min_t = %f, max_b = %f",min_t, max_b);
        for (auto &hb : subbuf)
        {
            hb.top_ = min_t;
            hb.bottom_ = max_b;
        }
    }
    result.insert(result.end(),subbuf.begin(),subbuf.end());
}

void processLineToResult_v1(std::vector<Hitbox> &result, fz_context *ctx, fz_rect &bounds, fz_text_line &line)
{
    fz_rect rr = fz_empty_rect;
    fz_irect box = fz_empty_irect;
    charnum = 0;
    int spanIndex;
    fz_text_span *span;
    float lastright = -1.0f;
    float nextleft  = -1.0f;
    bool space = false;
    bool hyph = false;
    for (span = line.first_span; span != nullptr; span = span->next)
    {
        bool prevspace = false;
        LDD(LOG, "PdfText: processLineToResult: span processing: %d", spanIndex);
        if (span->text && span->len > 0)
        {
            for (int textIndex = 0; textIndex < span->len;)
            {
                fz_rect nextbox;

                if(textIndex+1 < span->len)
                {
                    fz_text_char_bbox(ctx, &nextbox, span, textIndex+1);
                    nextleft  = nextbox.x0;
                }
                else if (span->next!=nullptr)
                {
                    fz_text_char_bbox(ctx, &nextbox, span->next, 0);
                    nextleft  = nextbox.x0;
                }
                else
                {
                    nextleft = bounds.x1;
                }

                fz_text_char& text = span->text[textIndex];
                if (lastright == -1.0)
                {
                    fz_rect bbox;
                    fz_text_char_bbox(ctx, &bbox, span, textIndex);
                    if      (bbox.x1 > bounds.x0 && bbox.x1 < bounds.x1) lastright = bbox.x1;
                }
                space = (text.c == 0x0D) || (text.c == 0x0A) || (text.c == 0x09) || (text.c == 0x20) || (text.c == 0x002e);
                hyph = (text.c == 0x002D);
                if (prevspace && space)
                {
                    // Do nothing with spaces
                    textIndex++;
                    charnum++;
                }
                else if (!prevspace)// &&) !space)
                {
                    fz_rect bbox;
                    fz_text_char_bbox(ctx, &bbox, span, textIndex);
                    float height = abs(bbox.y0-bbox.y1);

                    if(bbox.x0 < 0)
                    {
                        bbox.x0 = lastright;
                    }
                    //fz_union_rect(&rr, &bbox);
                    last_linechar = bbox;
                    lastright = bbox.x1;
                    length = fz_runetochar(utf8, text.c);
                    LDD(LOG, "PdfText: processText: char processing: %d %lc %f %f %f %f",
                            charnum, text.c, bbox.x0, bbox.y0, bbox.x1, bbox.y1);
                    //toResponse(response, bounds, fz_round_rect(&box, &bbox), utf8, 2);
                    // for removing of coordinates rounding if needed

                    // broken right side of bounding box  (too narrow or outside the range)
                    if(bbox.x1 < bbox.x0 + (height / 1.5f) || bbox.x1 > bounds.x1)
                    {
                        bbox.x1 = bbox.x0 + (height / 1.5f);
                        if(bbox.x1 > nextleft && nextleft < bounds.x1 && nextleft > bounds.x0 )
                        {
                            bbox.x1 = nextleft;
                        }
                    }
                    bool needSpace = false;
                    if (bbox.x1 < nextleft)
                    {
                        float charheight = bbox.y1 - bbox.y0;
                        float dist = nextleft - bbox.x1;
                        if(dist > charheight * 0.7f )
                        {
                            if (nextleft < bounds.x1 && nextleft > bounds.x0)
                            {
                                if (space)
                                {
                                    bbox.x1 = nextleft;
                                }
                                else
                                {
                                    needSpace = true;
                                }
                            }
                        }
                    }

                    fz_irect curr;
                    curr.x0 = bbox.x0;
                    curr.x1 = bbox.x1;
                    curr.y0 = bbox.y0;
                    curr.y1 = bbox.y1 + height / 8;
                    last_linechar.x0 = curr.x0;
                    last_linechar.x1 = curr.x1;
                    last_linechar.y0 = curr.y0;
                    last_linechar.y1 = curr.y1;

                    charnum++;
                    toResult(result, bounds, &curr, utf8, length);

                    if(needSpace)
                    {
                        fz_irect space_irect;
                        space_irect.x0 = curr.x1;
                        space_irect.x1 = nextleft;
                        space_irect.y0 = curr.y0;
                        space_irect.y1 = curr.y1;
                        text.c = 0x0020;
                        length = fz_runetochar(utf8, text.c);
                        charnum++;
                        //LE("ADDING SPACE");
                        toResult(result, bounds, &space_irect,utf8, length);
                    }
                    charnum++;
                    textIndex++;
                }
                else
                {
                    rr = fz_empty_rect;
                    box = fz_empty_irect;
                }
                prevspace = space;
            }
        }
    }
    if (line.first_span != nullptr && !space && !hyph)
    {
        fz_irect last_space_irect;
        last_space_irect.y0 = last_linechar.y0;
        last_space_irect.y1 = last_linechar.y1;
        float width = abs(last_linechar.x1 - last_linechar.x0);

        last_space_irect.x0 = last_linechar.x1;
        last_space_irect.x1 = last_space_irect.x0 + (width / 4);

        fz_text_char &text = line.first_span->text[0];
        text.c = 0x0020;
        length = fz_runetochar(utf8, text.c);
        charnum++;
        toResult(result, bounds, &last_space_irect,utf8,length);
    }
}

void processLineToResult_v0(std::vector<Hitbox> &result, fz_context *ctx, fz_rect &bounds, fz_text_line &line)
{
    int index = 0;
    fz_rect rr = fz_empty_rect;
    fz_irect box = fz_empty_irect;
    charnum = 0;
    int spanIndex;
    fz_text_span *span;
    for (span = line.first_span; span != nullptr; span = span->next)
    {
        bool prevspace = false;
        LDD(LOG, "PdfText: processText: span processing: %d", spanIndex);
        if (span->text && span->len > 0)
        {
            float lastright;
            int textIndex;
            for (textIndex = 0; textIndex < span->len;)
            {
                fz_text_char &text = span->text[textIndex];
                bool space = (text.c == 0x0D) || (text.c == 0x0A) || (text.c == 0x09) || (text.c == 0x20) || (text.c == 0x002e);
                if (prevspace && space)
                {
                    // Do nothing with spaces
                    textIndex++;
                }
                else if (!prevspace)// &&) !space)
                {
                    fz_rect bbox;
                    fz_text_char_bbox(ctx, &bbox, span, textIndex);
                    if (textIndex == 0)
                    {
                        lastright = bbox.x1;
                    }
                    else
                    {
                        bbox.x0 = lastright;
                    }
                    //fz_union_rect(&rr, &bbox);
                    last_linechar = bbox;
                    lastright = bbox.x1;
                    length = fz_runetochar(utf8, text.c);
                    LDD(LOG, "PdfText: processText: char processing: %d %lc %f %f %f %f",
                            index, text.c, bbox.x0, bbox.y0, bbox.x1, bbox.y1);
                    //toResponse(response, bounds, fz_round_rect(&box, &bbox), utf8, 2);
                    // for removing of coordinates rounding if needed
                    fz_irect curr;
                    float height = abs(bbox.y0 - bbox.y1);
                    curr.x0 = bbox.x0;
                    curr.x1 = bbox.x1;
                    curr.y0 = bbox.y0;
                    curr.y1 = bbox.y1 + height / 8;
                    last_linechar.x0 = curr.x0;
                    last_linechar.x1 = curr.x1;
                    last_linechar.y0 = curr.y0;
                    last_linechar.y1 = curr.y1;
                    toResult(result, bounds, &curr, utf8, length);

                    charnum = textIndex;
                    textIndex++;
                }
                else
                {
                    if (index > 0)
                    {
                        toResult(result, bounds, fz_round_rect(&box, &rr), utf8, length);
                        index = 0;
                    }
                    rr = fz_empty_rect;
                    box = fz_empty_irect;
                }
                prevspace = space;
            }
        }
    }

    if (index > 0)
    {
        LDD(LOG, "PdfText: processText: tail processing: %d", index);
        toResult(result, bounds, fz_round_rect(&box, &rr), utf8, length);
    }
}

void processLineToResult(std::vector<Hitbox> &result, fz_context *ctx, fz_rect &bounds, fz_text_line &line, int version)
{
    switch (version)
    {
        case 0:
            processLineToResult_v0(result, ctx, bounds, line);
            break;
        case 1:
            processLineToResult_v1(result, ctx, bounds, line);
            break;
        case CURRENT_MAX_VERSION:
        default:
            processLineToResult_v2(result, ctx, bounds, line);
            break;
    }
}

std::string getLineText(fz_context *ctx, fz_rect& bounds, fz_text_line& line)
{
    std::string result;

    std::vector<Hitbox> hitboxes;
    processLineToResult(hitboxes, ctx, bounds, line,-1);
    for (int i = 0; i < hitboxes.size(); i++)
    {
        Hitbox curr = hitboxes.at(i);
        result.append(wstringToString(curr.text_));
    }
    return result;
    /*
    int spanIndex;
    bool hastext = false;
    for (fz_text_span* span = line.first_span; span != nullptr; span = span->next)
    {
        bool prevspace = false;
        if (span->text && span->len > 0)
        {
            int textIndex;
            for (textIndex = 0; textIndex < span->len;)
            {
                hastext = true;
                char utf8[32];
                fz_text_char &text = span->text[textIndex];
                bool space = (text.c == 0x0D) || (text.c == 0x0A) || (text.c == 0x09) || (text.c == 0x20) || (text.c == 0x002e);
                if (prevspace && space)
                {
                    // Do nothing with spaces
                    textIndex++;
                }
                else if (!prevspace)// &&) !space)
                {
                    int length = fz_runetochar(utf8, text.c);
                    std::string a(utf8, length);
                    result.append(a);
                    textIndex++;
                }
                prevspace = space;
            }
        }
    }
    if (hastext)
    {
    //    result.append(std::string(" "));
    }
    return result;
*/
}

std::string MuPdfBridge::getPageText(int pageNo)
{
    pagenum = pageNo;
    std::string result;
    fz_page *page = getPage(pageNo, false);
    if (page == nullptr)
    {
        LDD(LOG, "PdfText: processText: no page %d", pageNo);
        return result;
    }

    fz_text_sheet *sheet = nullptr;
    fz_text_page *pagetext = nullptr;
    fz_device *dev = nullptr;

    fz_try(ctx)
            {
                fz_rect bounds = fz_empty_rect;
                fz_bound_page(ctx, page, &bounds);

                LDD(LOG, "PdfText: processText: page bounds: %f %f %f %f", bounds.x0, bounds.y0, bounds.x1, bounds.y1);

                sheet = fz_new_text_sheet(ctx);
                pagetext = fz_new_text_page(ctx);
                dev = fz_new_text_device(ctx, sheet, pagetext);

                fz_run_page(ctx, page, dev, &fz_identity, nullptr);

                // !!! Last line added to page only on device release
                fz_drop_device(ctx, dev);
                dev = nullptr;

                if (pagetext->blocks && pagetext->len > 0)
                {
                    LDD(LOG, "PdfText: processText: text found on page %d: %d/%d blocks", pageNo, pagetext->len, pagetext->cap);

                    int blockIndex;
                    for (blockIndex = 0; blockIndex < pagetext->len; blockIndex++)
                    {
                        fz_page_block& block = pagetext->blocks[blockIndex];
                        if (block.type != FZ_PAGE_BLOCK_TEXT)
                        {
                            continue;
                        }
                        if (block.u.text->lines && block.u.text->len > 0)
                        {
                            blocknum = blockIndex;
                            LDD(LOG, "PdfText: processText: block processing: %d, %d/%d lines",
                                    blockIndex, block.u.text->len, block.u.text->cap);
                            int lineIndex;
                            for (lineIndex = 0; lineIndex < block.u.text->len; lineIndex++)
                            {
                                linenum = lineIndex;
                                fz_text_line& line = block.u.text->lines[lineIndex];
                                if (line.first_span)
                                {
                                    LDD(LOG, "PdfText: processText: line processing: %d", lineIndex);
                                    result.append(getLineText(ctx, bounds, line));
                                }
                            }
                        }
                    }
                    LDD(LOG, "PdfText: processText: page processed");
                }
                else
                {
                    LDD(LOG, "PdfText: processText: no text found on page %d", pageNo);
                }
            }
    fz_always(ctx)
            {
                if (dev)
                {
                    LDD(LOG, "PdfText: processText: cleanup dev");
                    fz_drop_device(ctx, dev);
                }
                if (pagetext)
                {
                    LDD(LOG, "PdfText: processText: cleanup pagetext");
                    fz_drop_text_page(ctx, pagetext);
                }
                if (sheet)
                {
                    LDD(LOG, "PdfText: processText: cleanup sheet");
                    fz_drop_text_sheet(ctx, sheet);
                }
            }fz_catch(ctx)
    {
        const char* msg = fz_caught_message(ctx);
        LE("%s", msg);
    }
    LDD(LOG, "PdfText: processText: end");
    return result;
}

float getNextBlockFirstCharYCenter(fz_context *ctx, fz_text_page *pagetext, int currblockIndex)
{
    if (currblockIndex + 1 >= pagetext->len)
    {return -1;}

    fz_page_block &next_block = pagetext->blocks[currblockIndex + 1];

    if (next_block.type != FZ_PAGE_BLOCK_TEXT)
    {return -1;}


    if (!next_block.u.text->lines || next_block.u.text->len == 0)
    {return -1;}


    fz_text_line &line = next_block.u.text->lines[0];
    if (!line.first_span)
    {return -1;}


    fz_text_span *span = line.first_span;
    if (!span->text || span->len == 0)
    {return -1;}


    fz_rect bbox;
    fz_text_char_bbox(ctx, &bbox, span, 0);
    float height = abs(bbox.y1 - bbox.y0);
    return  (bbox.y0 + height/2) + height / 8;
}


float getNextLineFirstCharHeight(fz_context *ctx, fz_rect bounds , fz_page_block &currblock, int currlineIndex)
{
    if(currlineIndex + 1 >= currblock.u.text->len)
    {return -1;}

    fz_text_line &nextline = currblock.u.text->lines[currlineIndex+1];

    if (!nextline.first_span)
    {return -1;}

    fz_text_span *span = nextline.first_span;
    if (!span->text || span->len == 0)
    {return -1;}

    fz_rect bbox;
    fz_text_char_bbox(ctx, &bbox, span, 0);
    float bheight = abs(bbox.y0-bbox.y1);

    fz_irect curr = fz_empty_irect;
    curr.x0 = bbox.x0;
    curr.x1 = bbox.x1;
    curr.y0 = bbox.y0;
    curr.y1 = bbox.y1 + bheight / 8;

    std::vector<Hitbox> res;
    const char* a = " ";
    toResult(res,bounds,&curr,a,1);
    if(res.size()==0)
    {return -1;}

    return res.at(0).bottom_ - res.at(0).top_;
}


void postProcessLine(std::vector<Hitbox> result, fz_context *ctx, fz_rect bounds, fz_page_block &block,int lineIndex, int version)
{
    if(result.size()==0 || version == 0 || version == 1)
    {return;}
    Hitbox curr = result.at(result.size() - 1);
    if ( curr.text_.compare(L" ") == 0 )
    {return;}
    float nextlineHeight = getNextLineFirstCharHeight(ctx, bounds, block, lineIndex);
    if (nextlineHeight == -1)
    {return;}

    float currheight = curr.bottom_ - curr.top_;
    int nh = nextlineHeight * 100000;
    int ch = currheight     * 100000;

    if (ch > (nh + (nh * 0.2)))
    {
        fz_irect irect = fz_empty_irect;
        fz_rect rect = fz_empty_rect;
        rect.y0 = last_linechar.y0;
        rect.y1 = last_linechar.y1;
        float height = abs(rect.y1 - rect.y0);
        rect.x0 = last_linechar.x1;
        rect.x1 = rect.x0 + (height * 0.2f);

        toResultParaend(result, bounds, fz_round_rect(&irect, &rect));
    }
}

void postProcessBlock(std::vector<Hitbox> result, fz_context *ctx, fz_text_page *pagetext, int blockIndex, fz_rect bounds)
{
    if(result.size()==0) return;
    Hitbox curr = result.at(result.size() - 1);
    if ( curr.text_.compare(L"\n") == 0 )
    {return;}

    fz_irect irect = fz_empty_irect;
    fz_rect rect = fz_empty_rect;
    rect.y0 = last_linechar.y0;
    rect.y1 = last_linechar.y1;
    float height = abs(rect.y1 - rect.y0);
    rect.x0 = last_linechar.x1;
    rect.x1 = rect.x0 + (height * 0.2f);

    float next_char_y = getNextBlockFirstCharYCenter(ctx, pagetext, blockIndex);

    if(next_char_y < rect.y0 || next_char_y > rect.y1)
    {
        toResultParaend(result, bounds, fz_round_rect(&irect, &rect));
    }
}

std::vector<Hitbox> MuPdfBridge::processTextToArray(int pageNo, int version)
{
    if (!pagesCache.empty() && ( version == -1 || version == CURRENT_MAX_VERSION) )
    {
        for (int i = pagesCache.size()-1; i >= 0; i--)
        {
            if (pagesCache.at(i).getPage() == pageNo)
            {
                return pagesCache.at(i).getArray();
            }
        }
        if (pagesCache.size() >= 5)
        {
            pagesCache.erase(pagesCache.begin());
        }
    }

    std::vector<Hitbox> boxes = processTextToArray_imp(pageNo, version);
    if(version == -1 || version == CURRENT_MAX_VERSION )
    {
        pagesCache.push_back(PageHitboxesCash(pageNo, boxes));
    }
    return boxes;
}

std::vector<Hitbox> MuPdfBridge::processTextToArray_imp(int pageNo, int version)
{
    pagenum = pageNo;
    std::vector<Hitbox> result;
    fz_page *page = getPage(pageNo, false);
    if (page == nullptr)
    {
        LDD(LOG, "PdfText: processText: no page %d", pageNo);
        return result;
    }

    fz_text_sheet *sheet = nullptr;
    fz_text_page *pagetext = nullptr;
    fz_device *dev = nullptr;

    fz_try(ctx)
            {
                fz_rect bounds = fz_empty_rect;
                fz_bound_page(ctx, page, &bounds);

                LDD(LOG, "PdfText: processText: page bounds: %f %f %f %f",
                        bounds.x0, bounds.y0, bounds.x1, bounds.y1);

                sheet = fz_new_text_sheet(ctx);
                pagetext = fz_new_text_page(ctx);
                dev = fz_new_text_device(ctx, sheet, pagetext);

                fz_run_page(ctx, page, dev, &fz_identity, nullptr);

                // !!! Last line added to page only on device release
                fz_drop_device(ctx, dev);
                dev = nullptr;

                if (pagetext->blocks && pagetext->len > 0)
                {
                    LDD(LOG, "PdfText: processText: text found on page %d: %d/%d blocks",
                            pageNo, pagetext->len, pagetext->cap);

                    for (int blockIndex = 0; blockIndex < pagetext->len; blockIndex++)
                    {
                        fz_page_block &block = pagetext->blocks[blockIndex];
                        if (block.type != FZ_PAGE_BLOCK_TEXT)
                        {
                            continue;
                        }
                        if (block.u.text->lines && block.u.text->len > 0)
                        {
                            blocknum = blockIndex;
                            LDD(LOG, "PdfText: processText: block processing: %d, %d/%d lines",
                                    blockIndex, block.u.text->len, block.u.text->cap);
                            int lineIndex;
                            for (lineIndex = 0; lineIndex < block.u.text->len; lineIndex++)
                            {
                                linenum = lineIndex;
                                fz_text_line &line = block.u.text->lines[lineIndex];
                                if (line.first_span)
                                {
                                    LDD(LOG, "PdfText: processText: line processing: %d", lineIndex);
                                    processLineToResult(result, ctx, bounds, line, version);
                                    postProcessLine(result, ctx, bounds, block, lineIndex, version);
                                }
                            }
                            postProcessBlock(result,ctx,pagetext,blockIndex,bounds);
                        }
                    }
                    LDD(LOG, "PdfText: processText: page processed");
                }
                else
                {
                    LDD(LOG, "PdfText: processText: no text found on page %d", pageNo);
                }
            }
    fz_always(ctx)
            {
                if (dev)
                {
                    LDD(LOG, "PdfText: processText: cleanup dev");
                    fz_drop_device(ctx, dev);
                }
                if (pagetext)
                {
                    LDD(LOG, "PdfText: processText: cleanup pagetext");
                    fz_drop_text_page(ctx, pagetext);
                }
                if (sheet)
                {
                    LDD(LOG, "PdfText: processText: cleanup sheet");
                    fz_drop_text_sheet(ctx, sheet);
                }
            }
    fz_catch(ctx)
    {
        const char *msg = fz_caught_message(ctx);
        LE("%s", msg);
    }
    LDD(LOG, "PdfText: processText: end");
    return ReplaceUnusualSpaces(result);
}

float inline pointsDistance(float x1, float y1, float x2, float y2)
{
    float Dx = x2 - x1;
    float Dy = y2 - y1;
    return sqrt((Dx * Dx) + (Dy * Dy));
}

std::string MuPdfBridge::GetXpathFromPageByCoords(int page, float x, float y, bool addcoords, bool reverse)
{
    std::vector<Hitbox> hitboxes = processTextToArray(page);
    float mindistance = 65536.0F;
    int mindistance_id = -1;
    if (reverse)
    {
        for (int i = hitboxes.size() - 1; i >= 0 ; i--)
        {
            Hitbox curr = hitboxes.at(i);
            float dist = pointsDistance(x, y, curr.left_, curr.top_);
            if (dist < mindistance)
            {
                mindistance = dist;
                mindistance_id = i;
            }
        }
    }
    else
    {
        for (int i = 0; i < hitboxes.size(); i++)
        {
            Hitbox curr = hitboxes.at(i);
            float dist = pointsDistance(x, y, curr.left_, curr.top_);
            if (dist < mindistance)
            {
                mindistance = dist;
                mindistance_id = i;
            }
        }
    }
    if(mindistance_id <0)
    {
        return std::string();
    }
    return GetXpathFromPageById(hitboxes, mindistance_id, addcoords, reverse);
}

void SwitchDvngI_reverse(std::wstring* str)
{
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x093F)
        {
            str->at(i) = str->at(i+1);
            str->at(i+1) = 0x093f;
        }
    }
}

void SwitchBanglaLetters_reverse(std::wstring* str)
{
    // reverse
    if(str->length() < 2)
    {
        return;
    }
    for (int i = str->length()-2; i >=0 ; i--)
    {
        if(str->at(i) == 0x09BF || str->at(i) == 0x09C7 || str->at(i) == 0x09C8 )
        {
            str->at(i) = str->at(i+1);
            str->at(i+1) = 0x09BF;
        }
    }
}

void SwitchIndicChars(std::wstring* str)
{
    std::wistringstream in_stream = std::wistringstream(*str);

    std::wstring key;
    std::wstring res;
    while (std::getline(in_stream, key, L' '))
    {
        std::wstring *curr = &key;
        SwitchDvngI_reverse(curr);
        SwitchBanglaLetters_reverse(curr);
        res.append(*curr);
        res.append(L" ");
    }
    res.substr(0,res.length()-1);
    *str = res;
}

void MuPdfBridge::processRtlText(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_RTL_TEXT;

    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        LE("processRtlText bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    std::string inputstr = std::string(val);

    //page:startindex:endindex
    //"1:0:40"

    std::stringstream in_stream = std::stringstream(inputstr);

    std::string key;
    std::vector<std::string> in_arr;

    while (std::getline(in_stream, key, ':'))
    {
        in_arr.push_back(key);
    }

    if (in_arr.at(0).empty() || in_arr.at(1).empty() || in_arr.at(2).empty() )
    {
        LE("processRtlText bad request data : invalid key string [%s]",inputstr.c_str());
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    uint32_t page = std::atoi(in_arr.at(0).c_str());
    uint32_t startindex = std::atoi(in_arr.at(1).c_str());
    uint32_t endindex = std::atoi(in_arr.at(2).c_str());

    std::vector<Hitbox> selection = processTextToArray(page);

    std::wstring respstr;
    for (int i = startindex; i <= endindex && i < selection.size(); i++)
    {
        Hitbox currHitbox = selection.at(i);
        respstr += currHitbox.text_;
    }

    SwitchIndicChars(&respstr);

    response.addIpcString(wstringToString(respstr).c_str(), true);
}