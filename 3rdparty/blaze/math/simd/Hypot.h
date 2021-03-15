//=================================================================================================
/*!
//  \file blaze/math/simd/Hypot.h
//  \brief Header file for the SIMD hypothenous functionality
//
//  Copyright (C) 2012-2020 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. You can redistribute it and/or modify it under
//  the terms of the New (Revised) BSD License. Redistribution and use in source and binary
//  forms, with or without modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//     of conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//  3. Neither the names of the Blaze development group nor the names of its contributors
//     may be used to endorse or promote products derived from this software without specific
//     prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
*/
//=================================================================================================

#ifndef _BLAZE_MATH_SIMD_HYPOT_H_
#define _BLAZE_MATH_SIMD_HYPOT_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../../math/simd/BasicTypes.h"
#include "../../system/Inline.h"
#include "../../system/Vectorization.h"


namespace blaze {

//=================================================================================================
//
//  32-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Computes the length of the hypotenous of a right triangle of a vector of single
//        precision floating point values (\f$ \sqrt( a^2 + b^2 ) \f$).
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This function computes the square root of the sum of the squares of \a a and \a b, which
// corresponds to the length of the hypotenous of a right triangle. This operation is only
// available via the SVML for SSE, AVX, MIC, and AVX-512.
*/
template< typename T >  // Type of the operand
BLAZE_ALWAYS_INLINE const SIMDfloat hypot( const SIMDf32<T>& a, const SIMDf32<T>& b ) noexcept
#if BLAZE_SVML_MODE && ( BLAZE_AVX512F_MODE || BLAZE_MIC_MODE )
{
   return _mm512_hypot_ps( (~a).eval().value, (~b).eval().value );
}
#elif BLAZE_SVML_MODE && BLAZE_AVX_MODE
{
   return _mm256_hypot_ps( (~a).eval().value, (~b).eval().value );
}
#elif BLAZE_SVML_MODE && BLAZE_SSE_MODE
{
   return _mm_hypot_ps( (~a).eval().value, (~b).eval().value );
}
#else
= delete;
#endif
//*************************************************************************************************




//=================================================================================================
//
//  64-BIT FLOATING POINT SIMD TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Computes the length of the hypotenous of a right triangle of a vector of double
//        precision floating point values (\f$ \sqrt( a^2 + b^2 ) \f$).
// \ingroup simd
//
// \param a The left-hand side SIMD operand.
// \param b The right-hand side SIMD operand.
// \return The resulting vector.
//
// This function computes the square root of the sum of the squares of \a a and \a b, which
// corresponds to the length of the hypotenous of a right triangle. This operation is only
// available via the SVML for SSE, AVX, MIC, and AVX-512.
*/
template< typename T >  // Type of the operand
BLAZE_ALWAYS_INLINE const SIMDdouble hypot( const SIMDf64<T>& a, const SIMDf64<T>& b ) noexcept
#if BLAZE_SVML_MODE && ( BLAZE_AVX512F_MODE || BLAZE_MIC_MODE )
{
   return _mm512_hypot_pd( (~a).eval().value, (~b).eval().value );
}
#elif BLAZE_SVML_MODE && BLAZE_AVX_MODE
{
   return _mm256_hypot_pd( (~a).eval().value, (~b).eval().value );
}
#elif BLAZE_SVML_MODE && BLAZE_SSE_MODE
{
   return _mm_hypot_pd( (~a).eval().value, (~b).eval().value );
}
#else
= delete;
#endif
//*************************************************************************************************

} // namespace blaze

#endif
