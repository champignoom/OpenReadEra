/*
 * Copyright (C) 2013-2021 READERA LLC
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
 * Developers: ReadEra Team (2013-2021), Playful Curiosity (2013-2021),
 * Tarasus (2018-2021).
 */
//
// Created by Tarasus on 23.12.2020.
//


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cmath>
#include "EraComicBridge.h"
#include "EraCbrManager.h"
#include "EraCbzManager.h"
#include "EraComicManager.h"
#include "../jpeg-turbo/turbojpeg.h"

int pixelFormat = TJPF_RGBA;

void crop_top(unsigned char *buf, unsigned char *target, int n, int *width, int *height, int top)
{
    int offset = n* *width * top;
    int res_size = (*width* *height * n ) - offset;

    memcpy(target,buf+offset,res_size);
    *height -= top;
}

void crop_bottom(int *height, int bottom)
{
    *height -= bottom;
}

void crop_right(unsigned char *buf, unsigned char *target, int n, int *width, int *height, int right)
{
    auto ptr = buf;
    for (int i = 0; i < *height; i++)
    {
        memcpy(target,ptr,(*width-right)*n); //copy (width - right) pixels
        ptr += *width*n; //next row
        target+=(*width-right)*n; //next row for target
    }
    *width -= right;
}

void crop_left(unsigned char *buf, unsigned char *target, int n, int *width, int *height, int left)
{
    auto ptr = buf;
    for (int i = 0; i < *height; i++)
    {
        memcpy(target, ptr + (left * n), (*width-left)*n ); //copy (width - left) pixels, skipping first ones
        ptr += *width*n; //next row
        target+=(*width-left)*n;  //next row for target
    }
    *width -= left;
}

bool crop(unsigned char *buf, int n, int *orig_w, int *orig_h, int left, int top, int width, int height)
{
    //LE("crop %d x %d with %d %d %d %d",*orig_w,*orig_h,left,top,width,height);
    auto newbuf = (unsigned char *) malloc(*orig_w * *orig_h * n);
    if(newbuf == NULL)
    {
        LE("Failed to allocate %d bytes for image cropping",*orig_w * *orig_h * n);
        return false;
    }
    if(top>0)
    {
        crop_top(buf, newbuf, n, orig_w, orig_h, top);
        memcpy(buf, newbuf, *orig_w * *orig_h * n);
    }

    int bottom_cutoff = * orig_h - height;
    if(bottom_cutoff>0)
    {
        crop_bottom(orig_h, bottom_cutoff);
    }

    if(left>0)
    {
        crop_left(buf, newbuf, n, orig_w, orig_h, left);
        memcpy(buf, newbuf, *orig_w * *orig_h * n);
    }
    int right_cutoff = * orig_w - width;

    if(right_cutoff>0)
    {
        crop_right(buf, newbuf, n, orig_w, orig_h, right_cutoff);
        memcpy(buf, newbuf, *orig_w * *orig_h * n);
    }
    free(newbuf);

    //LE("crop result %d x %d ",*orig_w,*orig_h);
    return true;
}

void scale_bilinear(unsigned char* __restrict__ inBuf, unsigned int old_w, unsigned int old_h, unsigned char* __restrict__ outBuf, unsigned int new_w, unsigned int new_h) {
    unsigned int x, y, index ;
    float x_ratio = ((float)(old_w-1))/new_w ;
    float y_ratio = ((float)(old_h-1))/new_h ;
    float x_diff, y_diff;

    const int RGBA = 4;

    const unsigned int row = old_w * RGBA;

    unsigned int offset = 0;
    for (unsigned int i = 0; i < new_h; i++)
    {
        y = (unsigned int) (y_ratio * i);
        y_diff =  (y_ratio * i) - y;

        for (unsigned int j = 0; j < new_w; j++)
        {
            x = (unsigned int) (x_ratio * j);
            x_diff =  (x_ratio * j) - x;

            index = ((y * row) + (x*RGBA));

            const char Ar = inBuf[index];
            const char Ag = inBuf[index + 1];
            const char Ab = inBuf[index + 2];

            const char Br = inBuf[index + RGBA];
            const char Bg = inBuf[index + RGBA + 1];
            const char Bb = inBuf[index + RGBA + 2];

            const char Cr = inBuf[index + row];
            const char Cg = inBuf[index + row + 1];
            const char Cb = inBuf[index + row + 2];

            const char Dr = inBuf[index + row + RGBA];
            const char Dg = inBuf[index + row + RGBA + 1];
            const char Db = inBuf[index + row + RGBA + 2];

            const float _1mx_diff = (1-x_diff);
            const float _1my_diff = (1-y_diff);
            const float Acoeff = _1mx_diff * _1my_diff ;
            const float Bcoeff = (x_diff)  * _1my_diff;
            const float Ccoeff = (y_diff)  * _1mx_diff;
            const float Dcoeff = x_diff*y_diff;
            const float x_diff_x_y_diff = x_diff*y_diff;


            // red element
            // Yr = Ar(1-old_w)(1-old_h) + Br(old_w)(1-old_h) + Cr(old_h)(1-old_w) + Dr(wh)
            const char red = Ar * Acoeff + Br * Bcoeff + Cr * Ccoeff + Dr * Dcoeff;

            // green element
            // Yg = Ag(1-old_w)(1-old_h) + Bg(old_w)(1-old_h) + Cg(old_h)(1-old_w) + Dg(wh)
            const char green = Ag * Acoeff + Bg * Bcoeff + Cg * Ccoeff + Dg * Dcoeff;

            // blue element
            // Yb = Ab(1-old_w)(1-old_h) + Bb(old_w)(1-old_h) + Cb(old_h)(1-old_w) + Db(wh)
            const char blue = Ab * Acoeff + Bb * Bcoeff + Cb * Ccoeff + Db * Dcoeff;

            outBuf[offset++] = red;
            outBuf[offset++] = green;
            outBuf[offset++] = blue;
            outBuf[offset++] = 0xff;
        }
    }
}

void scale(unsigned char* __restrict__ inBuf, unsigned int old_w, unsigned int old_h, unsigned char* __restrict__ outBuf, unsigned int new_w, unsigned int new_h)
{
    if(old_w == 0 || old_h == 0)
    {
        LE("INVALID SOURCE DIMENSIONS!");
        return;
    }
    //LE("scale %u x %u to %u x %u",old_w,old_h,new_w,new_h);
    const unsigned int x_ratio = (old_w << 16) / new_w;
    const unsigned int y_ratio = (old_h << 16) / new_h;
    const int colors = 4;

    for (unsigned int y = 0; y < new_h; y++)
    {
        unsigned int y2_xsource = ((y * y_ratio) >> 16) * old_w;
        unsigned int i_xdest = y * new_w;
        for (unsigned int x = 0; x < new_w; x++)
        {
            unsigned int x2 = (x * x_ratio) >> 16 ;
            unsigned int y2_x2_colors = (y2_xsource + x2) * colors;
            unsigned int i_x_colors = (i_xdest + x) * colors;

            outBuf[i_x_colors]     = inBuf[y2_x2_colors];
            outBuf[i_x_colors + 1] = inBuf[y2_x2_colors + 1];
            outBuf[i_x_colors + 2] = inBuf[y2_x2_colors + 2];
            outBuf[i_x_colors + 3] = 0xFF;  //hardcoded full alpha

            //tile debugging green borders
            //if(y==0 || y== new_h - 1|| x == 0 || x == new_w -1)
            //{
            //    outBuf[i_x_colors]     = 0;
            //    outBuf[i_x_colors + 1] = 0xFF;
            //    outBuf[i_x_colors + 2] = 0;
            //    outBuf[i_x_colors + 3] = 0;
            //}
        }
    }
}

bool getCroppedPage(ComicFile *file, unsigned char *targetBuf, int target_w,
                    int target_h, matrix_s ctm, bool preview)
{
    int orig_width = file->orig_width;
    int orig_height = file->orig_height;
    int orig_S = orig_width * orig_height;
    int target_S = target_w * target_h;

    if(preview)
    {
        //no crop needed + fit image
        LE("Create preview [%dx%d] -> [%dx%d]", orig_width, orig_height, target_w, target_h);
        scale(file->page_buf, orig_width, orig_height, targetBuf, target_w, target_h);
        return true;
    }

    if(ctm.c == 1 && ctm.d == 1)
    {
        //no crop needed
        if( orig_S < target_S)
        {
            //stretch image
            scale_bilinear(file->page_buf, orig_width, orig_height, targetBuf, target_w, target_h);
        }
        else
        {
            //fit image
            scale(file->page_buf, orig_width, orig_height, targetBuf, target_w, target_h);
        }
        return true;
    }

    int crop_l = floor((float) orig_width  * ctm.a);
    int crop_t = floor((float) orig_height * ctm.b);
    int crop_w = round((float) orig_width  * ctm.c);
    int crop_h = round((float) orig_height * ctm.d);

    auto temp_buf = (unsigned char*)malloc(file->page_buf_size);
    if (temp_buf == NULL)
    {
        LE("Failed to allocate %d bytes",file->page_buf_size);
        return false;
    }
    memcpy(temp_buf,file->page_buf,file->page_buf_size);

    bool crop_res = crop(temp_buf, tjPixelSize[pixelFormat], &orig_width, &orig_height, crop_l, crop_t, crop_w, crop_h);
    if (!crop_res)
    {
        LE("CROP FAILED");
        free(temp_buf);
        return false;
    }

    if((ctm.c < 1 && ctm.c > 0.5f) && (ctm.d < 1 && ctm.d > 0.5f))
    {
        if (orig_S < target_S)
        {
            //bilinear stretch image
            scale_bilinear(temp_buf, orig_width, orig_height, targetBuf, target_w, target_h);
        }
        else
        {
            //nearest neighbor fit image
            scale(temp_buf, orig_width, orig_height, targetBuf, target_w, target_h);
        }
    }
    else
    {
        //nearest neighbor fit image
        scale(temp_buf, orig_width, orig_height, targetBuf, target_w, target_h);
    }

    free(temp_buf);
    //LE("Output Image :  %d x %d pixels", target_w, target_h);
    return true;
}

bool EraComicBridge::renderPage(uint32_t page_index, uint32_t w, uint32_t h, uint8_t *pixels, matrix_s transform_matrix, bool preview)
{
    ComicFile* file = comicManager->getPage(page_index);
    if (file == NULL)
    {
        LE("Failed to load file for page %d", page_index);
        return false;
    }
    return getCroppedPage(file, pixels, w, h, transform_matrix, preview);
}
