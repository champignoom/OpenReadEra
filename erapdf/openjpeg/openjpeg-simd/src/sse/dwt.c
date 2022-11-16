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

void v4dwt_decode_step1_simd(float* w, OPJ_INT32 count, float coeff)
{
    __m128* restrict vw = (__m128*) w;
    const __m128 c = _mm_set1_ps(coeff);
    OPJ_INT32 i;
    /* 4x unrolled loop */
    for(i = 0; i < count >> 2; ++i)
    {
        *vw = _mm_mul_ps(*vw, c);
        vw += 2;
        *vw = _mm_mul_ps(*vw, c);
        vw += 2;
        *vw = _mm_mul_ps(*vw, c);
        vw += 2;
        *vw = _mm_mul_ps(*vw, c);
        vw += 2;
    }
    count &= 3;
    for(i = 0; i < count; ++i)
    {
        *vw = _mm_mul_ps(*vw, c);
        vw += 2;
    }
}

void v4dwt_decode_step2_simd(float* l, float* w, OPJ_INT32 k, OPJ_INT32 m, float coeff)
{
    __m128* restrict vl = (__m128*) l;
    __m128* restrict vw = (__m128*) w;
    __m128 c = _mm_set1_ps(coeff);

    OPJ_INT32 i;
    __m128 tmp1, tmp2, tmp3;
    tmp1 = vl[0];
    for(i = 0; i < m; ++i)
    {
        tmp2 = vw[-1];
        tmp3 = vw[ 0];
        vw[-1] = _mm_add_ps(tmp2, _mm_mul_ps(_mm_add_ps(tmp1, tmp3), c));
        tmp1 = tmp3;
        vw += 2;
    }
    vl = vw - 2;
    if(m >= k)
    {
        return;
    }
    c = _mm_add_ps(c, c);
    c = _mm_mul_ps(c, vl[0]);
    for(; m < k; ++m)
    {
        __m128 tmp = vw[-1];
        vw[-1] = _mm_add_ps(tmp, c);
        vw += 2;
    }
}

