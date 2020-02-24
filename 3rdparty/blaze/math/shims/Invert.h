//=================================================================================================
/*!
//  \file blaze/math/shims/Invert.h
//  \brief Header file for the invert shim
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

#ifndef _BLAZE_MATH_SHIMS_INVERT_H_
#define _BLAZE_MATH_SHIMS_INVERT_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../../math/shims/Pow2.h"
#include "../../system/Inline.h"
#include "../../util/Assert.h"
#include "../../util/Complex.h"
#include "../../util/EnableIf.h"
#include "../../util/typetraits/IsIntegral.h"


namespace blaze {

//=================================================================================================
//
//  INV SHIMS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Inverting the given integral value.
// \ingroup math_shims
//
// \param a The integral value to be inverted.
// \return The inverse of the given value.
//
// The \a inv shim represents an abstract interface for inverting a value/object of any given
// data type. For integral values this results in \f$ \frac{1}{a} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
template< typename T >
BLAZE_ALWAYS_INLINE constexpr EnableIf_t< IsIntegral_v<T>, double > inv( T a ) noexcept
{
   BLAZE_USER_ASSERT( a != T(0), "Division by zero detected" );
   return ( 1.0 / a );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Inverting the given single precision value.
// \ingroup math_shims
//
// \param a The single precision value to be inverted.
// \return The inverse of the given value.
//
// The \a inv shim represents an abstract interface for inverting a value/object of any given
// data type. For single precision floating point values this results in \f$ \frac{1}{a} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
BLAZE_ALWAYS_INLINE constexpr float inv( float a ) noexcept
{
   BLAZE_USER_ASSERT( a != 0.0F, "Division by zero detected" );
   return ( 1.0F / a );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Inverting the given double precision value.
// \ingroup math_shims
//
// \param a The double precision value to be inverted.
// \return The inverse of the given value.
//
// The \a inv shim represents an abstract interface for inverting a value/object of any given
// data type. For double precision floating point values this results in \f$ \frac{1}{a} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
BLAZE_ALWAYS_INLINE constexpr double inv( double a ) noexcept
{
   BLAZE_USER_ASSERT( a != 0.0, "Division by zero detected" );
   return ( 1.0 / a );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Inverting the given extended precision value.
// \ingroup math_shims
//
// \param a The extended precision value to be inverted.
// \return The inverse of the given value.
//
// The \a inv shim represents an abstract interface for inverting a value/object of any given
// data type. For extended precision floating point values this results in \f$ \frac{1}{a} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
BLAZE_ALWAYS_INLINE constexpr long double inv( long double a ) noexcept
{
   BLAZE_USER_ASSERT( a != 0.0L, "Division by zero detected" );
   return ( 1.0L / a );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Inverting the given single precision complex number.
// \ingroup math_shims
//
// \param a The single precision complex number to be inverted.
// \return The inverse of the given value.
//
// The \a inv shim represents an abstract interface for inverting a value/object of any given
// data type. For a single precision floating point complex number \f$ z = x + yi \f$ this
// results in \f$ \frac{\overline{z}}{x^2+y^2} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
BLAZE_ALWAYS_INLINE constexpr complex<float> inv( const complex<float>& a ) noexcept
{
   const float abs( pow2( real(a) ) + pow2( imag(a) ) );
   BLAZE_USER_ASSERT( abs != 0.0F, "Division by zero detected" );

   const float iabs( 1.0F / abs );
   return complex<float>( iabs*real(a), -iabs*imag(a) );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Inverting the given double precision complex number.
// \ingroup math_shims
//
// \param a The double precision complex number to be inverted.
// \return The inverse of the given value.
//
// The \a inv shim represents an abstract interface for inverting a value/object of any given
// data type. For a double precision floating point complex number \f$ z = x + yi \f$ this
// results in \f$ \frac{\overline{z}}{x^2+y^2} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
BLAZE_ALWAYS_INLINE constexpr complex<double> inv( const complex<double>& a ) noexcept
{
   const double abs( pow2( real(a) ) + pow2( imag(a) ) );
   BLAZE_USER_ASSERT( abs != 0.0, "Division by zero detected" );

   const double iabs( 1.0 / abs );
   return complex<double>( iabs*real(a), -iabs*imag(a) );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Inverting the given extended precision complex number.
// \ingroup math_shims
//
// \param a The extended precision complex number to be inverted.
// \return The inverse of the given value.
//
// The \a inv shim represents an abstract interface for inverting a value/object of any given
// data type. For an extended precision floating point complex number \f$ z = x + yi \f$ this
// results in \f$ \frac{\overline{z}}{x^2+y^2} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
BLAZE_ALWAYS_INLINE constexpr complex<long double> inv( const complex<long double>& a ) noexcept
{
   const long double abs( pow2( real(a) ) + pow2( imag(a) ) );
   BLAZE_USER_ASSERT( abs != 0.0L, "Division by zero detected" );

   const long double iabs( 1.0L / abs );
   return complex<long double>( iabs*real(a), -iabs*imag(a) );
}
//*************************************************************************************************




//=================================================================================================
//
//  INVERT SHIMS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief In-place inversion of the given single precision value.
// \ingroup math_shims
//
// \param a The single precision value to be inverted.
// \return The inverse of the given value.
//
// The \a invert shim represents an abstract interface for inverting a value/object of any
// given data type in-place. For single precision floating point values this results in
// \f$ \frac{1}{a} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
BLAZE_ALWAYS_INLINE void invert( float& a ) noexcept
{
   a = inv( a );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief In-place inversion of the given double precision value.
// \ingroup math_shims
//
// \param a The double precision value to be inverted.
// \return The inverse of the given value.
//
// The \a invert shim represents an abstract interface for inverting a value/object of any
// given data type in-place. For double precision floating point values this results in
// \f$ \frac{1}{a} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
BLAZE_ALWAYS_INLINE void invert( double& a ) noexcept
{
   a = inv( a );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief In-place inversion of the given extended precision value.
// \ingroup math_shims
//
// \param a The extended precision value to be inverted.
// \return The inverse of the given value.
//
// The \a invert shim represents an abstract interface for inverting a value/object of any
// given data type in-place. For extended precision floating point values this results in
// \f$ \frac{1}{a} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
BLAZE_ALWAYS_INLINE void invert( long double& a ) noexcept
{
   a = inv( a );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief In-place inversion of the given single precision complex number.
// \ingroup math_shims
//
// \param a The single precision complex number to be inverted.
// \return The inverse of the given value.
//
// The \a invert shim represents an abstract interface for inverting a value/object of any given
// data type in-place. For a single precision floating point complex number \f$ z = x + yi \f$
// this results in \f$ \frac{\overline{z}}{x^2+y^2} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
BLAZE_ALWAYS_INLINE void invert( complex<float>& a ) noexcept
{
   a = inv( a );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief In-place inversion of the given double precision complex number.
// \ingroup math_shims
//
// \param a The double precision complex number to be inverted.
// \return The inverse of the given value.
//
// The \a invert shim represents an abstract interface for inverting a value/object of any given
// data type in-place. For a double precision floating point complex number \f$ z = x + yi \f$
// this results in \f$ \frac{\overline{z}}{x^2+y^2} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
BLAZE_ALWAYS_INLINE void invert( complex<double>& a ) noexcept
{
   a = inv( a );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief In-place inversion of the given extended precision complex number.
// \ingroup math_shims
//
// \param a The extended precision complex number to be inverted.
// \return The inverse of the given value.
//
// The \a invert shim represents an abstract interface for inverting a value/object of any given
// data type in-place. For an extended precision floating point complex number \f$ z = x + yi \f$
// this results in \f$ \frac{\overline{z}}{x^2+y^2} \f$.
//
// \note A division by zero is only checked by an user assert.
*/
BLAZE_ALWAYS_INLINE void invert( complex<long double>& a ) noexcept
{
   a = inv( a );
}
//*************************************************************************************************

} // namespace blaze

#endif
