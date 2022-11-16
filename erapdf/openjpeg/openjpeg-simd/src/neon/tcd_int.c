/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2014, Andrei Komarovskikh, Alex Kasatkin
 * Copyright (c) 2002-2014, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2014, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux
 * Copyright (c) 2003-2014, Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * Copyright (c) 2007, Jonathan Ballard <dzonatas@dzonux.net>
 * Copyright (c) 2007, Callum Lerwick <seg@haxxed.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arm_neon.h>

#include "opj_simd.h"

#define clamp(a, min, max) ((a) < (min) ? (min) : (a > max) ? (max) : (a))

void opj_tcd_dc_lsc_simd(OPJ_INT32* restrict current_ptr, OPJ_UINT32 height, OPJ_UINT32 width, OPJ_UINT32 stride,
                         OPJ_INT32 dc_level_shift, OPJ_INT32 min, OPJ_INT32 max)
{
    int32x4_t shift = vdupq_n_s32(dc_level_shift);
    int32x4_t lower = vdupq_n_s32(min);
    int32x4_t higher = vdupq_n_s32(max);

    OPJ_INT32 opt_width = width - 4;
    OPJ_INT32 line_width = width + stride;
    OPJ_INT32* end = current_ptr + width;
    OPJ_INT32* image_end = current_ptr + (line_width * height);

    if (opt_width < 0)
    {
        while (current_ptr < image_end)
        {
            while (current_ptr < end)
            {
                OPJ_INT32 l_value = *current_ptr;
                *current_ptr = clamp(l_value + dc_level_shift, min, max);

                ++current_ptr;
            }
            current_ptr += stride;
            end += line_width;
        }
    }
    else
    {
        OPJ_INT32* optend = current_ptr + opt_width;
        while (current_ptr < image_end)
        {
            while (current_ptr < optend)
            {
                int32x4_t iv = vld1q_s32(current_ptr);
                int32x4_t siv = vaddq_s32(iv, shift);
                int32x4_t civ = vmaxq_s32(vminq_s32(siv, higher), lower);
                vst1q_s32(current_ptr, civ);

                current_ptr += 4;
            }
            while (current_ptr < end)
            {
                OPJ_INT32 l_value = *current_ptr;
                *current_ptr = clamp(l_value + dc_level_shift, min, max);

                ++current_ptr;
            }
            current_ptr += stride;
            end += line_width;
            optend += line_width;
        }
    }
}

