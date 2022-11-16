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


#include <xmmintrin.h>

#include "opj_simd.h"

void opj_mct_decode_simd(OPJ_INT32* restrict c0, OPJ_INT32* restrict c1, OPJ_INT32* restrict c2, OPJ_UINT32 n)
{
    OPJ_UINT32 i;
    for (i = 0; i < n; ++i)
    {
        OPJ_INT32 y = c0[i];
        OPJ_INT32 u = c1[i];
        OPJ_INT32 v = c2[i];
        OPJ_INT32 g = y - ((u + v) >> 2);
        OPJ_INT32 r = v + g;
        OPJ_INT32 b = u + g;
        c0[i] = r;
        c1[i] = g;
        c2[i] = b;
    }
}

void opj_mct_decode_real_simd(OPJ_FLOAT32* restrict c0, OPJ_FLOAT32* restrict c1, OPJ_FLOAT32* restrict c2,
                              OPJ_UINT32 n)
{
    OPJ_UINT32 i;
    __m128 vrv, vgu, vgv, vbu;
    vrv = _mm_set1_ps(1.402f);
    vgu = _mm_set1_ps(0.34413f);
    vgv = _mm_set1_ps(0.71414f);
    vbu = _mm_set1_ps(1.772f);
    for (i = 0; i < (n >> 3); ++i)
    {
        __m128 vy, vu, vv;
        __m128 vr, vg, vb;

        vy = _mm_load_ps(c0);
        vu = _mm_load_ps(c1);
        vv = _mm_load_ps(c2);
        vr = _mm_add_ps(vy, _mm_mul_ps(vv, vrv));
        vg = _mm_sub_ps(_mm_sub_ps(vy, _mm_mul_ps(vu, vgu)), _mm_mul_ps(vv, vgv));
        vb = _mm_add_ps(vy, _mm_mul_ps(vu, vbu));
        _mm_store_ps(c0, vr);
        _mm_store_ps(c1, vg);
        _mm_store_ps(c2, vb);
        c0 += 4;
        c1 += 4;
        c2 += 4;

        vy = _mm_load_ps(c0);
        vu = _mm_load_ps(c1);
        vv = _mm_load_ps(c2);
        vr = _mm_add_ps(vy, _mm_mul_ps(vv, vrv));
        vg = _mm_sub_ps(_mm_sub_ps(vy, _mm_mul_ps(vu, vgu)), _mm_mul_ps(vv, vgv));
        vb = _mm_add_ps(vy, _mm_mul_ps(vu, vbu));
        _mm_store_ps(c0, vr);
        _mm_store_ps(c1, vg);
        _mm_store_ps(c2, vb);
        c0 += 4;
        c1 += 4;
        c2 += 4;
    }
    n &= 7;
    for (i = 0; i < n; ++i)
    {
        OPJ_FLOAT32 y = c0[i];
        OPJ_FLOAT32 u = c1[i];
        OPJ_FLOAT32 v = c2[i];
        OPJ_FLOAT32 r = y + (v * 1.402f);
        OPJ_FLOAT32 g = y - (u * 0.34413f) - (v * (0.71414f));
        OPJ_FLOAT32 b = y + (u * 1.772f);
        c0[i] = r;
        c1[i] = g;
        c2[i] = b;
    }
}

