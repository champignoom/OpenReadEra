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

#include "opj_simd.h"


int opj_init_simd()
{
    return 0;
}

void v4dwt_decode_step1_simd(float* fw, int count, const float c)
{

}

void v4dwt_decode_step2_simd(float* fl, float* fw, int k, int m, float c)
{

}

void opj_mct_decode_simd(OPJ_INT32* restrict c0, OPJ_INT32* c1, OPJ_INT32* restrict c2, OPJ_UINT32 n)
{

}

void opj_mct_decode_real_simd(OPJ_FLOAT32* restrict c0, OPJ_FLOAT32* restrict c1, OPJ_FLOAT32* restrict c2,
                              OPJ_UINT32 n)
{

}

void opj_tcd_dc_lsc_simd(OPJ_INT32* current_ptr, OPJ_UINT32 height, OPJ_UINT32 width, OPJ_UINT32 stride,
                         OPJ_INT32 dc_level_shift, OPJ_INT32 min, OPJ_INT32 max)
{

}

void opj_tcd_dc_lsc_real_simd(OPJ_INT32* current_ptr, OPJ_UINT32 height, OPJ_UINT32 width, OPJ_UINT32 stride,
                              OPJ_INT32 dc_level_shift, OPJ_INT32 min, OPJ_INT32 max)
{

}
