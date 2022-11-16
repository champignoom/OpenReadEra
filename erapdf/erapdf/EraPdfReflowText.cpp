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
// Created by Tarasus on 23/10/2019.
//

#include "EraPdfReflow.h"

class SpanStyle
{
public:
    fz_text_style *original;
    bool  bold;
    bool  italic;
    int   script;
    float size;
    SpanStyle() :original(nullptr), bold(false), italic(false), script(0), size(-1.0F) {};
    ~SpanStyle(){};

    SpanStyle(fz_text_style* style)
    {
        original = style;
        size     = style->size;
        script   = style->script;
        bold     = (bool)font_is_bold(style->font);
        italic   = (bool)font_is_italic(style->font);
    }

    int font_is_bold(fz_font *font)
    {
        FT_Face face = (FT_Face)font->ft_face;
        if (face && (face->style_flags & FT_STYLE_FLAG_BOLD))
            return 1;
        if (strstr(font->name, "Bold"))
            return 1;
        return 0;
    }

    int font_is_italic(fz_font *font)
    {
        FT_Face face = (FT_Face)font->ft_face;
        if (face && (face->style_flags & FT_STYLE_FLAG_ITALIC))
            return 1;
        if (strstr(font->name, "Italic") || strstr(font->name, "Oblique"))
            return 1;
        return 0;
    }

    bool compare(SpanStyle* comp)
    {
        if( comp == nullptr) return false;
        return compare_imp(*comp);
    }

    bool operator == (SpanStyle *comp)
    {
        if( comp == nullptr) return false;
        return compare_imp(*comp);
    }

    bool operator == (SpanStyle comp)
    {
        return compare_imp(comp);
    }

    bool printAttribs()
    {
        return bold || italic || script > 0;
    }
private:
    bool compare_imp(SpanStyle comp)
    {
        if ( this          == nullptr) return false;
        if ( original      == nullptr) return false;
        if ( comp.original == nullptr) return false;
        if ( size     != comp.size)    return false;
        if ( bold     != comp.bold)    return false;
        if ( italic   != comp.italic)  return false;
        if ( script   != comp.script)  return false;
        return true;
    }
};

void ReflowManager::send_img_base64(fz_buffer *buffer)
{
    int i, len;
    static const char set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    len = buffer->len / 3;
    for (i = 0; i < len; i++)
    {
        int c = buffer->data[3 * i];
        int d = buffer->data[3 * i + 1];
        int e = buffer->data[3 * i + 2];
        if ((i & 15) == 0)
        {
            fz_printf(ctx, out, "\n");
        }
        fz_printf(ctx, out, "%c%c%c%c", set[c >> 2], set[((c & 3) << 4) | (d >> 4)], set[((d & 15) << 2) | (e >> 6)], set[e & 63]);
    }
    i *= 3;
    switch (buffer->len - i)
    {
        case 2:
        {
            int c = buffer->data[i];
            int d = buffer->data[i + 1];
            fz_printf(ctx, out, "%c%c%c=", set[c >> 2], set[((c & 3) << 4) | (d >> 4)], set[((d & 15) << 2)]);
            break;
        }
        case 1:
        {
            int c = buffer->data[i];
            fz_printf(ctx, out, "%c%c==", set[c >> 2], set[(c & 3) << 4]);
            break;
        }
        default:
        case 0:
            break;
    }
}

static void print_style_begin(PageBlock *Pblock, SpanStyle *style, PageStats stats, int spanLength, int line_n, float span_y, int spanStart)
{
    if(!style) return;
    float h1Height = stats.h1;
    float h2Height = stats.h2;
    if(!style->printAttribs() || (spanLength < 3) || style->size < h2Height )
    {
        Pblock->printInside(L"<span data-line=\"%d\" data-y=\"%f\" data-start=\"%d\">\n", line_n, span_y, spanStart); // open line
        return;
    }

    int script = style->script;
    //char *s = strchr(style->font->name, '+');
    //s = s ? s + 1 : style->font->name;
    //Pblock->printInside(L"\"font-family:\"%s\";font-size:%gpt;", s, style->size);
    //Pblock->printInside(L"\"font-size:%gpt;", style->size);
    //Pblock->printInside(L"\" ");

    bool b = style->bold;
    bool i = style->italic;

    if (spanLength > 3 && h1Height > 0 && style->size >= h1Height)
    {
        Pblock->printInside(L"\n<h1 data-line = \"%d\" data-y=\"%f\" data-start=\"%d\"", line_n, span_y, spanStart);
    }
    else if (spanLength > 3 && h2Height > 0 && style->size >= h2Height)
    {
        Pblock->printInside(L"\n<h2 data-line = \"%d\" data-y=\"%f\" data-start=\"%d\"", line_n, span_y, spanStart);
    }
    else
    {
        Pblock->printInside(L"\n<span data-line = \"%d\" data-y=\"%f\" data-start=\"%d\"", line_n, span_y, spanStart);
    }

    if (b || i) Pblock->printInside(L" style=\"");
    if (b)      Pblock->printInside(L"font-weight:bold;");
    if (i)      Pblock->printInside(L"font-style:italic;");
    if (b || i) Pblock->printInside(L"\"");

    Pblock->printInside(L">\n");

    while (script-- > 0)
        Pblock->printInside(L"<sup>");
    while (++script < 0)
        Pblock->printInside(L"<sub>");
    //Pblock->printInside(L"{[");
}

static void print_style_end(PageBlock *Pblock, SpanStyle *style, PageStats stats, int spanLength, int line_n)
{
    float h1Height = stats.h1;
    float h2Height = stats.h2;

    if(!style->printAttribs() || (spanLength < 3) || style->size < h2Height)
    {
        Pblock->printInside(L" </span>\n"); // do NOT remove space in (L" </span>")
        return;
    }

    int script = style->script;
    //Pblock->printInside(L"]}");
    while (script-- > 0)
        Pblock->printInside(L"</sup>");
    while (++script < 0)
        Pblock->printInside(L"</sub>");

    if (spanLength > 3 && h1Height > 0 && style->size >= h1Height)
    {
        Pblock->printInside(L"\n</h1>\n", line_n);
    }
    else if (spanLength > 3 && h2Height > 0 && style->size >= h2Height)
    {
        Pblock->printInside(L"\n</h2>\n", line_n);
    }
    else
    {
        Pblock->printInside(L"\n </span>\n", line_n); // do NOT remove space in (L" </span>")
    }
}

void ReflowManager::reflowPageText(fz_text_page *page, int pagenum, fz_display_list_s *displayList)
{
    SpanStyle *currStyle = nullptr;
    fz_text_line *line;
    fz_text_span *span;
    std::vector<PageBlock> blocks;

    PageStats pageStats = this->pageStatsArray.at(pagenum);

    for (int block_n = 0; block_n < page->len; block_n++)
    {
        if (block_n == pageStats.bannedBlock1 && pageStats.bannedBlock1 != -1)
        {
            continue;
        }
        if (block_n == pageStats.bannedBlock2 && pageStats.bannedBlock2 != -1)
        {
            continue;
        }

        PageBlock Pblock;
        Pblock.index = block_n;

        switch (page->blocks[block_n].type)
        {
            case FZ_PAGE_BLOCK_TEXT:
            {

                fz_text_block *block = page->blocks[block_n].u.text;
                if(!rect_includes_rect(pageStats.bounds,block->bbox))
                {
                    Pblock.draw = false;
                    break;
                }

                Pblock.type = FZ_PAGE_BLOCK_TEXT;
                Pblock.rect = block->bbox;

                for (int line_n = 0; line_n < block->len; line_n++)
                {
                    int lastcol = -1;
                    line = &block->lines[line_n];
                    currStyle = nullptr;

                    #ifndef DEBUG_INTERNALS
                    //fz_printf(ctx, out, ">");
                    #else
                    if (line->region)
                    fz_printf(ctx, out, " region=\"%x\"", line->region);
                    fz_printf(ctx, out, ">");
                    #endif

                    for (span = line->first_span; span; span = span->next)
                    {
                        float size = fz_matrix_expansion(&span->transform);
                        float base_offset = span->base_offset / size;
                        int spanstart = 0;
                        /*
                        if (lastcol != span->column)
                        {
                            if (lastcol >= 0)
                            {
                                fz_printf(ctx, out, "\n</div>");
                            }
                            // If we skipped any columns then output some spacer spans
                            while (lastcol < span->column - 1)
                            {
                                fz_printf(ctx, out, "\n<div class=\"cell\">\n</div>");
                                lastcol++;
                            }
                            lastcol++;
                            // Now output the span to contain this entire column
                            fz_printf(ctx, out, "\n<div class=\"cell\" style=\"");
                            {
                                fz_text_span *sn;
                                for (sn = span->next; sn; sn = sn->next)
                                {
                                    if (sn->column != lastcol)
                                    {
                                        break;
                                    }
                                }
                                fz_printf(ctx, out, "width:%g%%;align:%s", span->column_width, (span->align == 0 ? "left" : (span->align == 1 ? "center" : "right")));
                            }
                            if (span->indent > 1)
                            {
                                fz_printf(ctx, out, ";padding-left:1em;text-indent:-1em");
                            }
                            if (span->indent < -1)
                            {
                                fz_printf(ctx, out, ";text-indent:1em");
                            }
                            fz_printf(ctx, out, "\">");
                        }

                        if (span->spacing >= 1)
                        {
                            fz_printf(ctx, out, " ");
                        }
                         */
                        if (base_offset > SUBSCRIPT_OFFSET)
                        {
                            Pblock.printInside(L"<sub>");
                        }
                        else if (base_offset < SUPERSCRIPT_OFFSET)
                        {
                            Pblock.printInside(L"<sup>");
                        }
                        for (int ch_n = 0; ch_n < span->len; ch_n++)
                        {
                            fz_text_char *ch = &span->text[ch_n];
                            if (!currStyle->compare(new SpanStyle(ch->style)))
                            {
                                if (currStyle)
                                {
                                    print_style_end(&Pblock, currStyle,pageStats, span->len, line_n);
                                }
                                currStyle = new SpanStyle(ch->style);

                                print_style_begin(&Pblock, currStyle,pageStats, span->len, line_n,span->bbox.y0,spanstart);
                            }
                            spanstart ++;

                            if(isQuote(ch->c))
                            {
                                ch->c = '"';
                            }
                            if(char_isSpace(ch->c))
                            {
                                ch->c = ' ';
                            }

                            if (ch->c == '<')
                            {
                                Pblock.printInside(L"&lt;");
                            }
                            else if (ch->c == '>')
                            {
                                Pblock.printInside(L"&gt;");
                            }
                            else if (ch->c == '&')
                            {
                                Pblock.printInside(L"&amp;");
                            }
                            else if (ch->c >= 32 && ch->c <= 127)
                            {
                                Pblock.printInside(L"%c", ch->c);
                            }
                            else
                            {
                                //Pblock.printInside(L"l&#x%x;", ch->c);
                                Pblock.printInside(L"%lc", ch->c);
                            }
                        }
                        if (currStyle)
                        {
                            print_style_end(&Pblock, currStyle, pageStats, span->len, line_n);
                            currStyle = nullptr;
                        }

                        if (base_offset > SUBSCRIPT_OFFSET)
                        {
                            Pblock.printInside(L"</sub>");
                        }
                        else if (base_offset < SUPERSCRIPT_OFFSET)
                        {
                            Pblock.printInside(L"</sup>");
                        }

                        //Pblock.printInside( L"</span>\n");
                    }
                }

                break;
            }
            case FZ_PAGE_BLOCK_IMAGE:
            {
                fz_image_block *imgBlock = page->blocks[block_n].u.image;
                Pblock.type = FZ_PAGE_BLOCK_IMAGE;
                Pblock.rect = imgBlock->bbox;
                Pblock.imgBlock = imgBlock;
                break;
            }
            default:
                break;
        }
        blocks.push_back(Pblock);
    }

    for (int i = 0; i < blocks.size(); i++)
    {

        PageBlock currblock = blocks.at(i);
        if (!currblock.draw)
        {
            continue;
        }

        fz_printf(ctx, out, "<p data-block=\"%d\">\n",i);

        switch (currblock.type)
        {
            case FZ_PAGE_BLOCK_TEXT:
            {
                fz_printf(ctx, out, "%s", wstringToString(currblock.text).c_str());
                break;
            }
            case FZ_PAGE_BLOCK_IMAGE:
            {
                for (int j = 0; j < blocks.size(); j++)
                {
                    PageBlock intersect = blocks.at(j);
                    fz_rect a = currblock.rect;
                    fz_rect b = intersect.rect;
                    if (intersect.type == FZ_PAGE_BLOCK_IMAGE)
                    {
                        if (rects_intersect(a, b) && intersect.draw)
                        {
                            currblock.rect = union_rect(a, b);
                            blocks.at(i).rect = union_rect(a, b);
                            blocks.at(j).draw = false;
                        }
                    }
                    if (intersect.type == FZ_PAGE_BLOCK_TEXT)
                    {
                        if (!rect_includes_rect(a, b))
                        {
                            ctx->ignore_rects[ctx->ignore_rects_num] = intersect.rect;
                            ctx->ignore_rects_num++;
                        }
                    }
                }

                fz_image_block *imgBlock = currblock.imgBlock;

                float iwidth = currblock.rect.x1 - currblock.rect.x0;
                float iheight = currblock.rect.y1 - currblock.rect.y0;

                //fz_rect orig_rect = currblock.rect;
                //if (!(orig_rect.x0 > 0 && orig_rect.x1 > 0 && orig_rect.y0 > 0 && orig_rect.y1 > 0))
                //{
                //    break;
                //}

                if (currblock.rect.x1 > pageStats.bounds.x1)
                {
                    currblock.rect.x1 = pageStats.bounds.x1;
                    currblock.rect.x0 = pageStats.bounds.x1 - iwidth;
                }
                if (currblock.rect.x0 < pageStats.bounds.x0)
                {
                    currblock.rect.x0 = pageStats.bounds.x0;
                    currblock.rect.x1 = pageStats.bounds.x0 + iwidth;
                }

                if (currblock.rect.y1 > pageStats.bounds.y1)
                {
                    currblock.rect.y1 = pageStats.bounds.y1;
                    currblock.rect.y0 = pageStats.bounds.y1 - iheight;
                }
                if (currblock.rect.y0 < pageStats.bounds.y0)
                {
                    currblock.rect.y0 = pageStats.bounds.y0;
                    currblock.rect.y1 = pageStats.bounds.y0 + iheight;
                }

                fz_matrix ctm = fz_identity;
                fz_translate(&ctm, -currblock.rect.x0, -currblock.rect.y0);

                auto size = static_cast<size_t>((iwidth) * (iheight) * 4);

                auto pixels = (uint8_t *) malloc(size);

                fz_device *dev = nullptr;
                fz_pixmap *pixmap = nullptr;
                fz_try(ctx)
                        {
                            pixmap = fz_new_pixmap_with_data(ctx, fz_device_rgb(ctx), static_cast<int>(iwidth), static_cast<int>(iheight), pixels);

                            //fz_clear_pixmap_with_value(ctx, pixmap, 0xff);
                            fz_clear_pixmap_with_value(ctx, pixmap, 0xDD);

                            dev = fz_new_draw_device(ctx, pixmap);

                            ctx->flag_interpolate_images = 1;
                            //fz_rect rects[64];
                            //int rectnum = 0;
                            //LE("ANALYZE START");
                            //fz_analyze_display_list(ctx, displayList, dev, &ctm, nullptr, nullptr,rects,&rectnum);
                            //LE("ANALYZE END: Tile rects = %d",rectnum);

                            //LE("Drawing = [%f:%f][%f:%f]",orig_rect.x0,orig_rect.x1,orig_rect.y0,orig_rect.y1);
                            //bool draw = true;
                            //for (int i = 0; i < rectnum; i++)
                            //{
                            //    LE("rect = [%f:%f][%f:%f]",rects[i].x0,rects[i].x1,rects[i].y0,rects[i].y1);
                            //}

                            fz_run_display_list(ctx, displayList, dev, &ctm, nullptr, nullptr,pagenum);
                            ctx->ignore_rects_num = 0;

                            imgBlock->image = fz_new_image_from_pixmap(ctx, pixmap, nullptr);

                            //ctx->ignore_rects = nullptr;
                        }
                fz_always(ctx)
                        {
                            //fz_drop_device(ctx, dev);
                            //fz_drop_pixmap(ctx, pixmap);
                        }
                fz_catch(ctx)
                {
                    const char *msg = fz_caught_message(ctx);
                    LE( "%s", msg);
                }


                switch (imgBlock->image->buffer == nullptr ? FZ_IMAGE_JPX : imgBlock->image->buffer->params.type)
                {
                    case FZ_IMAGE_JPEG:
                        fz_printf(ctx, out, "<image href=\"#page%d_img%d.jpg\"/>\n", pagenum, i);
                        fz_printf(ctx, out, "<binary id=\"page%d_img%d.jpg\">", pagenum, i);
                        send_img_base64( imgBlock->image->buffer->buffer);

                        break;
                    case FZ_IMAGE_PNG:
                        fz_printf(ctx, out, "<image href=\"#page%d_img%d.png\"/>\n", pagenum, i);
                        fz_printf(ctx, out, "<binary id=\"page%d_img%d.png\">", pagenum, i);
                        send_img_base64( imgBlock->image->buffer->buffer);
                        break;
                    default:
                    {
                        fz_buffer *buf = fz_new_png_from_image(ctx, imgBlock->image, imgBlock->image->w, imgBlock->image->h);
                        fz_printf(ctx, out, "<image href=\"#page%d_img%d.png\"/>\n", pagenum, i);
                        fz_printf(ctx, out, "<binary id=\"page%d_img%d.png\">", pagenum, i);
                        send_img_base64( buf);
                        fz_drop_buffer(ctx, buf);
                        break;
                    }
                }
                fz_printf(ctx, out, "\n</binary>\n");
                break;
            }
            default:
                break;
        }
        fz_printf(ctx, out, "</p>\n");//  class="block" Close the block
    }
}