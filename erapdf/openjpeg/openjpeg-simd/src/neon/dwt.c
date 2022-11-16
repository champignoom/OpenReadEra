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

void v4dwt_decode_step1_simd(OPJ_FLOAT32* restrict fw, int count, const OPJ_FLOAT32 c)
{
    float32_t cc = c;
    int i;
    for (i = 0; i < count; ++i)
    {
        float32x4_t vTmp = vld1q_f32(fw + i * 8);
        float32x4_t vOp = vmulq_n_f32(vTmp, c);
        vst1q_f32(fw + i * 8, vOp);
    }
}

void v4dwt_decode_step2_simd(OPJ_FLOAT32* restrict fl, OPJ_FLOAT32* restrict fw, int k, int m, OPJ_FLOAT32 c)
{
    float32x4_t vC = vdupq_n_f32(c);

    OPJ_INT32 i;
    for (i = 0; i < m; ++i)
    {
        float32x4_t vTmp1 = vld1q_f32(fl);
        float32x4_t vTmp2 = vld1q_f32(fw - 4);
        float32x4_t vTmp3 = vld1q_f32(fw);

        float32x4_t vOp1 = vaddq_f32(vTmp1, vTmp3);
        float32x4_t vOp2 = vmulq_f32(vOp1, vC);
        float32x4_t vOp3 = vaddq_f32(vTmp2, vOp2);

        vst1q_f32(fw - 4, vOp3);

        fl = fw;
        fw += 8;
    }

    if (m < k)
    {
        float32x4_t vC2 = vaddq_f32(vC, vC);
        float32x4_t vFl = vld1q_f32(fl);
        float32x4_t vFlC = vmulq_f32(vFl, vC2);
        for (; m < k; ++m)
        {
            float32x4_t vTmp = vld1q_f32(fw - 4);
            float32x4_t vOp = vaddq_f32(vTmp, vFlC);
            vst1q_f32(fw - 4, vOp);

            fw += 8;
        }
    }
}

