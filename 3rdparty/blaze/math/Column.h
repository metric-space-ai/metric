//=================================================================================================
/*!
//  \file blaze/math/Column.h
//  \brief Header file for the complete Column implementation
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

#ifndef _BLAZE_MATH_COLUMN_H_
#define _BLAZE_MATH_COLUMN_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../math/Aliases.h"
#include "../math/constraints/Column.h"
#include "../math/constraints/DenseVector.h"
#include "../math/constraints/SparseVector.h"
#include "../math/Exception.h"
#include "../math/smp/DenseVector.h"
#include "../math/smp/SparseVector.h"
#include "../math/views/Column.h"
#include "../math/views/Row.h"
#include "../util/Random.h"
#include "../util/typetraits/RemoveReference.h"


namespace blaze {

//=================================================================================================
//
//  RAND SPECIALIZATION FOR DENSE COLUMNS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of the Rand class template for dense columns.
// \ingroup random
//
// This specialization of the Rand class randomizes dense columns.
*/
template< typename MT       // Type of the matrix
        , bool SO           // Storage order
        , bool SF           // Symmetry flag
        , size_t... CCAs >  // Compile time column arguments
class Rand< Column<MT,SO,true,SF,CCAs...> >
{
 public:
   //**Randomize functions*************************************************************************
   /*!\name Randomize functions */
   //@{
   template< typename CT >
   inline void randomize( CT&& column ) const;

   template< typename CT, typename Arg >
   inline void randomize( CT&& column, const Arg& min, const Arg& max ) const;
   //@}
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Randomization of a dense column.
//
// \param column The column to be randomized.
// \return void
*/
template< typename MT       // Type of the matrix
        , bool SO           // Storage order
        , bool SF           // Symmetry flag
        , size_t... CCAs >  // Compile time column arguments
template< typename CT >     // Type of the column
inline void Rand< Column<MT,SO,true,SF,CCAs...> >::randomize( CT&& column ) const
{
   using blaze::randomize;

   using ColumnType = RemoveReference_t<CT>;

   BLAZE_CONSTRAINT_MUST_BE_COLUMN_TYPE( ColumnType );
   BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE( ColumnType );

   for( size_t i=0UL; i<column.size(); ++i ) {
      randomize( column[i] );
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Randomization of a dense column.
//
// \param column The column to be randomized.
// \param min The smallest possible value for a column element.
// \param max The largest possible value for a column element.
// \return void
*/
template< typename MT       // Type of the matrix
        , bool SO           // Storage order
        , bool SF           // Symmetry flag
        , size_t... CCAs >  // Compile time column arguments
template< typename CT       // Type of the column
        , typename Arg >    // Min/max argument type
inline void Rand< Column<MT,SO,true,SF,CCAs...> >::randomize( CT&& column, const Arg& min, const Arg& max ) const
{
   using blaze::randomize;

   using ColumnType = RemoveReference_t<CT>;

   BLAZE_CONSTRAINT_MUST_BE_COLUMN_TYPE( ColumnType );
   BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE( ColumnType );

   for( size_t i=0UL; i<column.size(); ++i ) {
      randomize( column[i], min, max );
   }
}
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  RAND SPECIALIZATION FOR SPARSE COLUMNS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Specialization of the Rand class template for sparse columns.
// \ingroup random
//
// This specialization of the Rand class randomizes sparse columns.
*/
template< typename MT       // Type of the matrix
        , bool SO           // Storage order
        , bool SF           // Symmetry flag
        , size_t... CCAs >  // Compile time column arguments
class Rand< Column<MT,SO,false,SF,CCAs...> >
{
 public:
   //**Randomize functions*************************************************************************
   /*!\name Randomize functions */
   //@{
   template< typename CT >
   inline void randomize( CT&& column ) const;

   template< typename CT >
   inline void randomize( CT&& column, size_t nonzeros ) const;

   template< typename CT, typename Arg >
   inline void randomize( CT&& column, const Arg& min, const Arg& max ) const;

   template< typename CT, typename Arg >
   inline void randomize( CT&& column, size_t nonzeros, const Arg& min, const Arg& max ) const;
   //@}
   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Randomization of a sparse column.
//
// \param column The column to be randomized.
// \return void
*/
template< typename MT       // Type of the matrix
        , bool SO           // Storage order
        , bool SF           // Symmetry flag
        , size_t... CCAs >  // Compile time column arguments
template< typename CT >     // Type of the column
inline void Rand< Column<MT,SO,false,SF,CCAs...> >::randomize( CT&& column ) const
{
   using ColumnType  = RemoveReference_t<CT>;
   using ElementType = ElementType_t<ColumnType>;

   BLAZE_CONSTRAINT_MUST_BE_COLUMN_TYPE( ColumnType );
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE( ColumnType );

   const size_t size( column.size() );

   if( size == 0UL ) return;

   const size_t nonzeros( rand<size_t>( 1UL, std::ceil( 0.5*size ) ) );

   column.reset();
   column.reserve( nonzeros );

   while( column.nonZeros() < nonzeros ) {
      column[ rand<size_t>( 0UL, size-1UL ) ] = rand<ElementType>();
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Randomization of a sparse column.
//
// \param column The column to be randomized.
// \param nonzeros The number of non-zero elements of the random column.
// \return void
// \exception std::invalid_argument Invalid number of non-zero elements.
*/
template< typename MT       // Type of the matrix
        , bool SO           // Storage order
        , bool SF           // Symmetry flag
        , size_t... CCAs >  // Compile time column arguments
template< typename CT >     // Type of the column
inline void Rand< Column<MT,SO,false,SF,CCAs...> >::randomize( CT&& column, size_t nonzeros ) const
{
   using ColumnType  = RemoveReference_t<CT>;
   using ElementType = ElementType_t<ColumnType>;

   BLAZE_CONSTRAINT_MUST_BE_COLUMN_TYPE( ColumnType );
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE( ColumnType );

   const size_t size( column.size() );

   if( nonzeros > size ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid number of non-zero elements" );
   }

   if( size == 0UL ) return;

   column.reset();
   column.reserve( nonzeros );

   while( column.nonZeros() < nonzeros ) {
      column[ rand<size_t>( 0UL, size-1UL ) ] = rand<ElementType>();
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Randomization of a sparse column.
//
// \param column The column to be randomized.
// \param min The smallest possible value for a column element.
// \param max The largest possible value for a column element.
// \return void
*/
template< typename MT       // Type of the matrix
        , bool SO           // Storage order
        , bool SF           // Symmetry flag
        , size_t... CCAs >  // Compile time column arguments
template< typename CT       // Type of the column
        , typename Arg >    // Min/max argument type
inline void Rand< Column<MT,SO,false,SF,CCAs...> >::randomize( CT&& column, const Arg& min, const Arg& max ) const
{
   using ColumnType  = RemoveReference_t<CT>;
   using ElementType = ElementType_t<ColumnType>;

   BLAZE_CONSTRAINT_MUST_BE_COLUMN_TYPE( ColumnType );
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE( ColumnType );

   const size_t size( column.size() );

   if( size == 0UL ) return;

   const size_t nonzeros( rand<size_t>( 1UL, std::ceil( 0.5*size ) ) );

   column.reset();
   column.reserve( nonzeros );

   while( column.nonZeros() < nonzeros ) {
      column[ rand<size_t>( 0UL, size-1UL ) ] = rand<ElementType>( min, max );
   }
}
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
/*!\brief Randomization of a sparse column.
//
// \param column The column to be randomized.
// \param nonzeros The number of non-zero elements of the random column.
// \param min The smallest possible value for a column element.
// \param max The largest possible value for a column element.
// \return void
// \exception std::invalid_argument Invalid number of non-zero elements.
*/
template< typename MT       // Type of the matrix
        , bool SO           // Storage order
        , bool SF           // Symmetry flag
        , size_t... CCAs >  // Compile time column arguments
template< typename CT       // Type of the column
        , typename Arg >    // Min/max argument type
inline void Rand< Column<MT,SO,false,SF,CCAs...> >::randomize( CT&& column, size_t nonzeros,
                                                               const Arg& min, const Arg& max ) const
{
   using ColumnType  = RemoveReference_t<CT>;
   using ElementType = ElementType_t<ColumnType>;

   BLAZE_CONSTRAINT_MUST_BE_COLUMN_TYPE( ColumnType );
   BLAZE_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE( ColumnType );

   const size_t size( column.size() );

   if( nonzeros > size ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid number of non-zero elements" );
   }

   if( size == 0UL ) return;

   column.reset();
   column.reserve( nonzeros );

   while( column.nonZeros() < nonzeros ) {
      column[ rand<size_t>( 0UL, size-1UL ) ] = rand<ElementType>( min, max );
   }
}
/*! \endcond */
//*************************************************************************************************

} // namespace blaze

#endif
