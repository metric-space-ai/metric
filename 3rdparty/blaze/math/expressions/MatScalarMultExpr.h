//=================================================================================================
/*!
//  \file blaze/math/expressions/MatScalarMultExpr.h
//  \brief Header file for the MatScalarMultExpr base class
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

#ifndef _BLAZE_MATH_EXPRESSIONS_MATSCALARMULTEXPR_H_
#define _BLAZE_MATH_EXPRESSIONS_MATSCALARMULTEXPR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../../math/Exception.h"
#include "../../math/expressions/MultExpr.h"
#include "../../util/FunctionTrace.h"


namespace blaze {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all matrix/scalar multiplication expression templates.
// \ingroup math
//
// The MatScalarMultExpr class serves as a tag for all expression templates that implement a
// matrix/scalar multiplication. All classes, that represent a matrix/scalar multiplication
// and that are used within the expression template environment of the Blaze library have
// to derive publicly from this class in order to qualify as matrix/scalar multiplication
// expression template. Only in case a class is derived publicly from the MatScalarMultExpr
// base class, the IsMatScalarMultExpr type trait recognizes the class as valid matrix/scalar
// multiplication expression template.
*/
template< typename MT >  // Matrix base type of the expression
struct MatScalarMultExpr
   : public MultExpr<MT>
{};
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Calculation of the transpose of the given matrix-scalar multiplication.
// \ingroup math
//
// \param matrix The matrix-scalar multiplication expression to be transposed.
// \return The transpose of the expression.
//
// This operator implements the performance optimized treatment of the transpose of a
// matrix-scalar multiplication. It restructures the expression \f$ A=trans(B*s1) \f$ to
// the expression \f$ A=trans(B)*s1 \f$.
*/
template< typename MT >  // Matrix base type of the expression
inline decltype(auto) trans( const MatScalarMultExpr<MT>& matrix )
{
   BLAZE_FUNCTION_TRACE;

   return trans( (~matrix).leftOperand() ) * (~matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Calculation of the complex conjugate of the given matrix-scalar multiplication.
// \ingroup math
//
// \param matrix The matrix-scalar multiplication expression to be conjugated.
// \return The complex conjugate of the expression.
//
// This operator implements the performance optimized treatment of the complex conjugate of a
// matrix-scalar multiplication. It restructures the expression \f$ a=conj(b*s1) \f$ to the
// expression \f$ a=conj(b)*s1 \f$.
*/
template< typename MT >  // Matrix base type of the expression
inline decltype(auto) conj( const MatScalarMultExpr<MT>& matrix )
{
   BLAZE_FUNCTION_TRACE;

   return conj( (~matrix).leftOperand() ) * (~matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-symmetric matrix-scalar multiplication expression as symmetric.
// \ingroup math
//
// \param matrix The input matrix-scalar multiplication expression.
// \return The redeclared expression.
// \exception std::invalid_argument Invalid symmetric matrix specification.
//
// This function implements the application of the \a declsym operation on a matrix-scalar
// multiplication. It restructures the expression \f$ A=declsym(B*s1) \f$ to the expression
// \f$ A=declsym(B)*s1 \f$. In case the given matrix is not a square matrix, a
// \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Matrix base type of the expression
inline decltype(auto) declsym( const MatScalarMultExpr<MT>& matrix )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( ~matrix ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid symmetric matrix specification" );
   }

   return declsym( (~matrix).leftOperand() ) * (~matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-Hermitian matrix-scalar multiplication expression as Hermitian.
// \ingroup math
//
// \param matrix The input matrix-scalar multiplication expression.
// \return The redeclared expression.
// \exception std::invalid_argument Invalid Hermitian matrix specification.
//
// This function implements the application of the declherm() operation on a matrix-scalar
// multiplication. It restructures the expression \f$ A=declherm(B*s1) \f$ to the expression
// \f$ A=declherm(B)*s1 \f$. In case the given matrix is not a square matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Matrix base type of the expression
inline decltype(auto) declherm( const MatScalarMultExpr<MT>& matrix )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( ~matrix ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid Hermitian matrix specification" );
   }

   return declherm( (~matrix).leftOperand() ) * (~matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-lower matrix-scalar multiplication expression as lower.
// \ingroup math
//
// \param matrix The input matrix-scalar multiplication expression.
// \return The redeclared expression.
// \exception std::invalid_argument Invalid lower matrix specification.
//
// This function implements the application of the decllow() operation on a matrix-scalar
// multiplication. It restructures the expression \f$ A=decllow(B*s1) \f$ to the expression
// \f$ A=decllow(B)*s1 \f$. In case the given matrix is not a square matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Matrix base type of the expression
inline decltype(auto) decllow( const MatScalarMultExpr<MT>& matrix )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( ~matrix ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid lower matrix specification" );
   }

   return decllow( (~matrix).leftOperand() ) * (~matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-unilower matrix-scalar multiplication expression as unilower.
// \ingroup math
//
// \param matrix The input matrix-scalar multiplication expression.
// \return The redeclared expression.
// \exception std::invalid_argument Invalid unilower matrix specification.
//
// This function implements the application of the declunilow() operation on a matrix-scalar
// multiplication. It restructures the expression \f$ A=declunilow(B*s1) \f$ to the expression
// \f$ A=declunilow(B)*s1 \f$. In case the given matrix is not a square matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Matrix base type of the expression
inline decltype(auto) declunilow( const MatScalarMultExpr<MT>& matrix )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( ~matrix ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid unilower matrix specification" );
   }

   return declunilow( (~matrix).leftOperand() ) * (~matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-strictly-lower matrix-scalar multiplication expression as
//        strictly lower.
// \ingroup math
//
// \param matrix The input matrix-scalar multiplication expression.
// \return The redeclared expression.
// \exception std::invalid_argument Invalid strictly lower matrix specification.
//
// This function implements the application of the declstrlow() operation on a matrix-scalar
// multiplication. It restructures the expression \f$ A=declstrlow(B*s1) \f$ to the expression
// \f$ A=declstrlow(B)*s1 \f$. In case the given matrix is not a square matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Matrix base type of the expression
inline decltype(auto) declstrlow( const MatScalarMultExpr<MT>& matrix )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( ~matrix ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid strictly lower matrix specification" );
   }

   return declstrlow( (~matrix).leftOperand() ) * (~matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-upper matrix-scalar multiplication expression as upper.
// \ingroup math
//
// \param matrix The input matrix-scalar multiplication expression.
// \return The redeclared expression.
// \exception std::invalid_argument Invalid upper matrix specification.
//
// This function implements the application of the declupp() operation on a matrix-scalar
// multiplication. It restructures the expression \f$ A=declupp(B*s1) \f$ to the expression
// \f$ A=declupp(B)*s1 \f$. In case the given matrix is not a square matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Matrix base type of the expression
inline decltype(auto) declupp( const MatScalarMultExpr<MT>& matrix )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( ~matrix ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid upper matrix specification" );
   }

   return declupp( (~matrix).leftOperand() ) * (~matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-uniupper matrix-scalar multiplication expression as uniupper.
// \ingroup math
//
// \param matrix The input matrix-scalar multiplication expression.
// \return The redeclared expression.
// \exception std::invalid_argument Invalid uniupper matrix specification.
//
// This function implements the application of the decluniupp() operation on a matrix-scalar
// multiplication. It restructures the expression \f$ A=decluniupp(B*s1) \f$ to the expression
// \f$ A=decluniupp(B)*s1 \f$. In case the given matrix is not a square matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Matrix base type of the expression
inline decltype(auto) decluniupp( const MatScalarMultExpr<MT>& matrix )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( ~matrix ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid uniupper matrix specification" );
   }

   return decluniupp( (~matrix).leftOperand() ) * (~matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-strictly-upper matrix-scalar multiplication expression as
//        strictly upper.
// \ingroup math
//
// \param matrix The input matrix-scalar multiplication expression.
// \return The redeclared expression.
// \exception std::invalid_argument Invalid strictly upper matrix specification.
//
// This function implements the application of the declstrupp() operation on a matrix-scalar
// multiplication. It restructures the expression \f$ A=declstrupp(B*s1) \f$ to the expression
// \f$ A=declstrupp(B)*s1 \f$. In case the given matrix is not a square matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Matrix base type of the expression
inline decltype(auto) declstrupp( const MatScalarMultExpr<MT>& matrix )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( ~matrix ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid strictly upper matrix specification" );
   }

   return declstrupp( (~matrix).leftOperand() ) * (~matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Declares the given non-diagonal matrix-scalar multiplication expression as diagonal.
// \ingroup math
//
// \param matrix The input matrix-scalar multiplication expression.
// \return The redeclared expression.
// \exception std::invalid_argument Invalid diagonal matrix specification.
//
// This function implements the application of the decldiag() operation on a matrix-scalar
// multiplication. It restructures the expression \f$ A=decldiag(B*s1) \f$ to the expression
// \f$ A=decldiag(B)*s1 \f$. In case the given matrix is not a square matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Matrix base type of the expression
inline decltype(auto) decldiag( const MatScalarMultExpr<MT>& matrix )
{
   BLAZE_FUNCTION_TRACE;

   if( !isSquare( ~matrix ) ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid diagonal matrix specification" );
   }

   return decldiag( (~matrix).leftOperand() ) * (~matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif
