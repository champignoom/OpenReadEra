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

#ifndef INLINE
    #if defined(_MSC_VER)
        #define INLINE __forceinline
    #elif defined(__GNUC__)
        #define INLINE __inline__
    #elif defined(__MWERKS__)
        #define INLINE inline
    #else
        /* add other compilers here ... */
        #define INLINE
    #endif /* defined(<Compiler>) */
#endif /* INLINE */

static INLINE OPJ_INT32 clamp(OPJ_INT32 a, OPJ_INT32 min, OPJ_INT32 max) {
    if (a < min)
        return min;
    if (a > max)
        return max;
    return a;
}

void opj_tcd_dc_lsc_simd(OPJ_INT32* l_current_ptr, OPJ_UINT32 l_height, OPJ_UINT32 l_width, OPJ_UINT32 l_stride,
                         OPJ_INT32 m_dc_level_shift, OPJ_INT32 l_min, OPJ_INT32 l_max)
{
    OPJ_UINT32 i, j;
    for (j = 0; j < l_height; ++j)
    {
        for (i = 0; i < l_width; ++i)
        {
            OPJ_INT32 l_value = *l_current_ptr;
            *l_current_ptr = clamp(l_value + m_dc_level_shift, l_min, l_max);
            ++l_current_ptr;
        }
        l_current_ptr += l_stride;
    }
}

void opj_tcd_dc_lsc_real_simd(OPJ_INT32* l_current_ptr, OPJ_UINT32 l_height, OPJ_UINT32 l_width, OPJ_UINT32 l_stride,
                              OPJ_INT32 m_dc_level_shift, OPJ_INT32 l_min, OPJ_INT32 l_max)
{
    OPJ_UINT32 i, j, n;

    for (j = 0; j < l_height; ++j)
    {
        i = 0;
        while (i < l_width)
        {
            OPJ_INT32 l_value = (OPJ_INT32) (0.5f + *((OPJ_FLOAT32 *) l_current_ptr));
            *l_current_ptr = clamp(l_value + m_dc_level_shift, l_min, l_max);
            ++l_current_ptr;
            ++i;
        }
        l_current_ptr += l_stride;
    }
}
