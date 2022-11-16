/*
 * Copyright (C) 2013 The MuPDF CLI viewer interface Project
 * Copyright (C) 2013-2020 READERA LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Developers: ReadEra Team (2013-2020), Playful Curiosity (2013-2020),
 * Tarasus (2018-2020).
 */

#include <algorithm>
#include "smartcrop.h"
#include "../orebridge/include/ore_log.h"

constexpr int v_line_size = 5;
constexpr int h_line_size = 10;
int v_line_margin = 20;
int h_line_margin = 20;
constexpr float white_threshold = 0.01;
int pageLimit_h = 0;
int pageLimit_v = 0;

static bool IsRectWhite(const uint8_t *pixels, int width, int height, int sx, int sy, int sw, int sh)
{
    int darkPixels_count = 0;
    for (int y = sy; y < sy + sh; y++)
    {
        for (int x = sx; x < sx + sw; x++)
        {
            int i = (y * width * 4) + (x * 4);
            if(pixels[i + 0] == 0 || pixels[i + 1] == 0 || pixels[i + 2] == 0 )
            {
                darkPixels_count++;
            }
        }
    }
    int limit = floor(sw * sh * white_threshold);
    //LW("    IsRectWhite count = %d, limit = %d",darkPixels_count,limit);
    return (darkPixels_count < limit);
}

static float GetLeftCropBound(const uint8_t *pixels, int width, int height)
{
    int w = pageLimit_h;
    int white_count = 0;
    int x = 0;

    for (x = 0; x < w; x += v_line_size)
    {
        bool white = IsRectWhite(pixels, width, height, x, h_line_margin, v_line_size, height - (2 * h_line_margin));
        //LE("GetLeftCropBound x = %d white = %d cnt = %d",x,white,white_count);
        if (white)
        {
            white_count++;
        }
        else
        {
            if (white_count >= 1)
            {
                return (float) (std::max(0, x - v_line_size)) / (float) width;
            }
            if(x == 0)
            {
                //found black on first iteration == exit;
                return 0;
            }
            white_count = 0;
        }
    }
    return white_count > 0 ? (float) (std::max(0, x - v_line_size)) / (float) width : 0;
}

static float GetTopCropBound(const uint8_t *pixels, int width, int height, float ignoreZoneLeft)
{
    int h = pageLimit_v;
    int white_count = 0;
    int y = 0;

    int ignore = ceil(ignoreZoneLeft * width);

    for (y = 0; y < h; y += h_line_size)
    {
        bool white = IsRectWhite(pixels, width, height, ignore, y, width - (v_line_margin + ignore), h_line_size);
        //LE("GetTopCropBound y = %d white = %d cnt = %d",y,white,white_count);
        if (white)
        {
            white_count++;
        }
        else
        {
            if (white_count >= 1)
            {
                //LW("top ret 1");
                return (float) (std::max(0, y - h_line_size)) / (float) height;
            }
            if(y == 0)
            {
                //found black on first iteration == exit;
                return 0;
            }
            white_count = 0;
        }
    }
    //LW("top ret 2");
    return white_count > 0 ? (float) (std::max(0, y - h_line_size)) / (float) height : 0;
}

static float GetRightCropBound(const uint8_t *pixels, int width, int height, float ignoreZoneTop)
{
    int w = pageLimit_h;
    int white_count = 0;
    int x = 0;

    int ignore = ceil(ignoreZoneTop * height);

    for (x = width - v_line_size; x > width - w; x -= v_line_size)
    {
        bool white = IsRectWhite(pixels, width, height, x, ignore, v_line_size, height - (h_line_margin + ignore));
        //LE("GetRightCropBound x = %d white = %d cnt = %d",x,white,white_count);

        if (white)
        {
            white_count++;
        }
        else
        {
            if (white_count >= 1)
            {
                return (float) (std::min(width, x + (2 * v_line_size))) / (float) width;
            }
            if(x == width - v_line_size)
            {
                //found black on first iteration == exit;
                return 1;
            }
            white_count = 0;
        }
    }
    return white_count > 0 ? (float) (std::min(width, x + (2 * v_line_size))) / (float) width : 1;
}

static float GetBottomCropBound(const uint8_t *pixels, int width, int height, float ignoreZoneLeft, float ignoreZoneRight )
{
    int h = pageLimit_v;
    int white_count = 0;
    int y = 0;

    int ignoreLeft  = ceil(ignoreZoneLeft  * width);
    int ignoreRight = ceil( (1 - ignoreZoneRight) * width);
    for (y = height - h_line_size; y > height - h; y -= h_line_size)
    {
        bool white = IsRectWhite(pixels, width, height, ignoreLeft, y, width - (ignoreLeft + ignoreRight), h_line_size);
        //LE("GetBottomCropBound y = %d white = %d cnt = %d",y,white,white_count);
        if (white)
        {
            white_count++;
        }
        else
        {
            if (white_count >= 1)
            {
                return (float) (std::min(height, y + (h_line_size * 2))) / (float) height;
            }
            if(y == height - h_line_size)
            {
                //found black on first iteration == exit;
                return 1;
            }
            white_count = 0;
        }
    }
    return white_count > 0 ? (float) (std::min(height, y + (h_line_size * 2))) / (float) height : 1;
}

void CalcBitmapSmartCrop(float *crop, uint8_t *pixels, int w, int h, float slice_l, float slice_t, float slice_r, float slice_b)
{
    //LE("CalcBitmapSmartCrop w = %d, h = %d ,[%f %f %f %f]",w,h,slice_l,slice_t,slice_r,slice_b);
    pageLimit_h = ceil(w * 0.9);
    pageLimit_v = ceil(h * 0.9);
    v_line_margin = ceil(w * 0.05);
    h_line_margin = ceil(h * 0.05);

    int size = w * h * 4;
    auto edge_pixels = (unsigned char *) malloc(size);
    if (edge_pixels == NULL)
    {
        LE("CalcBitmapSmartCrop: failed to allocate %d bytes for inverted pixels", size);
        return;
    }

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            unsigned int i = (y * w * 4) + (x * 4);
            if ( (x <= w * 0.01) ||  (x >= w * 0.99) || (y <= h/100) || y >= h *0.99)
            {
                edge_pixels[i+0] = 255;
                edge_pixels[i+1] = 255;
                edge_pixels[i+2] = 255;
                edge_pixels[i+3] = 255;
                continue;
            }

            int currlum = ceil((pixels[i + 0] + pixels[i + 1] + pixels[i + 2]) / 3);
            int prevlum = ceil((pixels[i - 4] + pixels[i - 3] + pixels[i - 2]) / 3);

            int val = (abs(currlum - prevlum) > 8) ? 0 : 255;
            edge_pixels[i+0] = val;
            edge_pixels[i+1] = val;
            edge_pixels[i+2] = val;
            edge_pixels[i+3] = 255;
        }
    }

    crop[0] = GetLeftCropBound  (edge_pixels, w, h);
    crop[1] = GetTopCropBound   (edge_pixels, w, h, crop[0]);
    crop[2] = GetRightCropBound (edge_pixels, w, h, crop[1]);
    crop[3] = GetBottomCropBound(edge_pixels, w, h, crop[0], crop[2]);

    if(crop[0] >= crop[2])
    {
        crop[0] = 0;
        crop[2] = 1;
    }

    if(crop[1] >= crop[3])
    {
        crop[1] = 0;
        crop[3] = 1;
    }

    crop[0] = crop[0] * (slice_r - slice_l) + slice_l;
    crop[1] = crop[1] * (slice_b - slice_t) + slice_t;
    crop[2] = crop[2] * (slice_r - slice_l) + slice_l;
    crop[3] = crop[3] * (slice_b - slice_t) + slice_t;
    free(edge_pixels);

    //LE("slices = [%f %f %f %f]",slice_l,slice_t,slice_r,slice_b);
    //LE("crop   = [%f %f %f %f]",crop[0],crop[1],crop[2],crop[3]);
}