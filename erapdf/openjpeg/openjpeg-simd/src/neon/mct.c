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

void opj_mct_decode_simd(OPJ_INT32* restrict c0, OPJ_INT32* c1, OPJ_INT32* restrict c2, OPJ_UINT32 n)
{
    OPJ_INT32 opt_width = n - 8;
    OPJ_INT32* end = c0 + n;

    if (opt_width >= 0)
    {
        OPJ_INT32* opt_end = c0 + opt_width;
        while (c0 <= opt_end)
        {
            int32x4_t vY = vld1q_s32(c0);
            int32x4_t vU = vld1q_s32(c1);
            int32x4_t vV = vld1q_s32(c2);

            int32x4_t vG = vsubq_s32(vY, vshrq_n_s32(vaddq_s32(vU, vV), 2));
            int32x4_t vR = vaddq_s32(vV, vG);
            int32x4_t vB = vaddq_s32(vU, vG);

            vst1q_s32(c0, vR);
            vst1q_s32(c1, vG);
            vst1q_s32(c2, vB);

            c0 += 4;
            c1 += 4;
            c2 += 4;

            vY = vld1q_s32(c0);
            vU = vld1q_s32(c1);
            vV = vld1q_s32(c2);

            vG = vsubq_s32(vY, vshrq_n_s32(vaddq_s32(vU, vV), 2));
            vR = vaddq_s32(vV, vG);
            vB = vaddq_s32(vU, vG);

            vst1q_s32(c0, vR);
            vst1q_s32(c1, vG);
            vst1q_s32(c2, vB);

            c0 += 4;
            c1 += 4;
            c2 += 4;
        }
    }

    while (c0 < end)
    {
        OPJ_INT32 y = *c0;
        OPJ_INT32 u = *c1;
        OPJ_INT32 v = *c2;
        OPJ_INT32 g = y - ((u + v) >> 2);
        OPJ_INT32 r = v + g;
        OPJ_INT32 b = u + g;
        *c0 = r;
        *c1 = g;
        *c2 = b;
        c0++;
        c1++;
        c2++;
    }
}

void opj_mct_decode_real_simd(OPJ_FLOAT32* restrict c0, OPJ_FLOAT32* restrict c1, OPJ_FLOAT32* restrict c2,
                              OPJ_UINT32 n)
{
    OPJ_UINT32 i;
    float32x4_t v_1_402 = vdupq_n_f32(1.402f);
    float32x4_t v_0_344 = vdupq_n_f32(0.34413f);
    float32x4_t v_0_714 = vdupq_n_f32(0.71414f);
    float32x4_t v_1_772 = vdupq_n_f32(1.772f);

    OPJ_INT32 opt_width = n - 8;
    OPJ_FLOAT32* end = c0 + n;

    if (opt_width >= 0)
    {
        OPJ_FLOAT32* opt_end = c0 + opt_width;
        while (c0 <= opt_end)
        {
            float32x4_t vY = vld1q_f32(c0);
            float32x4_t vU = vld1q_f32(c1);
            float32x4_t vV = vld1q_f32(c2);

            float32x4_t vR = vaddq_f32(vY, vmulq_f32(vV, v_1_402));
            float32x4_t vG = vsubq_f32(vsubq_f32(vY, vmulq_f32(vU, v_0_344)), vmulq_f32(vV, v_0_714));
            float32x4_t vB = vaddq_f32(vY, vmulq_f32(vU, v_1_772));

            vst1q_f32(c0, vR);
            vst1q_f32(c1, vG);
            vst1q_f32(c2, vB);

            c0 += 4;
            c1 += 4;
            c2 += 4;

            vY = vld1q_f32(c0);
            vU = vld1q_f32(c1);
            vV = vld1q_f32(c2);

            vR = vaddq_f32(vY, vmulq_f32(vV, v_1_402));
            vG = vsubq_f32(vsubq_f32(vY, vmulq_f32(vU, v_0_344)), vmulq_f32(vV, v_0_714));
            vB = vaddq_f32(vY, vmulq_f32(vU, v_1_772));

            vst1q_f32(c0, vR);
            vst1q_f32(c1, vG);
            vst1q_f32(c2, vB);

            c0 += 4;
            c1 += 4;
            c2 += 4;
        }
    }

    while (c0 < end)
    {
        OPJ_FLOAT32 y = *c0;
        OPJ_FLOAT32 u = *c1;
        OPJ_FLOAT32 v = *c2;
        OPJ_FLOAT32 r = y + (v * 1.402f);
        OPJ_FLOAT32 g = y - (u * 0.34413f) - (v * (0.71414f));
        OPJ_FLOAT32 b = y + (u * 1.772f);
        *c0 = r;
        *c1 = g;
        *c2 = b;
        c0++;
        c1++;
        c2++;
    }
}
