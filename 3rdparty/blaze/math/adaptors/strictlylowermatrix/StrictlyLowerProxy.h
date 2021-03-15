//=================================================================================================
/*!
//  \file blaze/math/adaptors/strictlylowermatrix/StrictlyLowerProxy.h
//  \brief Header file for the StrictlyLowerProxy class
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

#ifndef _BLAZE_MATH_ADAPTORS_LOWERMATRIX_STRICTLYLOWERPROXY_H_
#define _BLAZE_MATH_ADAPTORS_LOWERMATRIX_STRICTLYLOWERPROXY_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../../../math/Aliases.h"
#include "../../../math/constraints/Computation.h"
#include "../../../math/constraints/Hermitian.h"
#include "../../../math/constraints/Lower.h"
#include "../../../math/constraints/Matrix.h"
#include "../../../math/constraints/Symmetric.h"
#include "../../../math/constraints/Transformation.h"
#include "../../../math/constraints/Upper.h"
#include "../../../math/constraints/View.h"
#include "../../../math/Exception.h"
#include "../../../math/InitializerList.h"
#include "../../../math/proxy/Proxy.h"
#include "../../../math/RelaxationFlag.h"
#include "../../../math/shims/Clear.h"
#include "../../../math/shims/IsDefault.h"
#include "../../../math/shims/IsNaN.h"
#include "../../../math/shims/IsOne.h"
#include "../../../math/shims/IsReal.h"
#include "../../../math/shims/IsZero.h"
#include "../../../math/shims/Reset.h"
#include "../../../util/constraints/Const.h"
#include "../../../util/constraints/Pointer.h"
#include "../../../util/constraints/Reference.h"
#include "../../../util/constraints/Volatile.h"
#include "../../../util/typetraits/AddConst.h"
#include "../../../util/typetraits/AddReference.h"
#include "../../../util/Types.h"


namespace blaze {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Access proxy for strictly lower triangular matrices.
// \ingroup strictly_lower_matrix
//
// The StrictlyLowerProxy provides controlled access to the elements of a non-const strictly lower
// triangular matrix. It guarantees that the strictly lower matrix invariant is not violated, i.e.
// that elements on the diagonal and in the upper part of the matrix remain 0. The following
// example illustrates this by means of a \f$ 3 \times 3 \f$ dense strictly lower triangular
// matrix:

   \code
   // Creating a 3x3 strictly lower triangular dense matrix
   blaze::StrictlyLowerMatrix< blaze::DynamicMatrix<int> > A( 3UL );

   A(0,1) = -2;  //        (  0 0 0 )
   A(2,1) =  3;  // => A = ( -2 0 0 )
   A(2,2) =  5;  //        (  3 5 0 )

   A(1,1) =  4;  // Invalid assignment to diagonal matrix element; results in an exception!
   A(0,2) =  7;  // Invalid assignment to upper matrix element; results in an exception!
   \endcode
*/
template< typename MT >  // Type of the adapted matrix
class StrictlyLowerProxy
   : public Proxy< StrictlyLowerProxy<MT>, ElementType_t<MT> >
{
 private:
   //**Type definitions****************************************************************************
   //! Reference type of the underlying matrix type.
   using ReferenceType = AddConst_t< Reference_t<MT> >;
   //**********************************************************************************************

 public:
   //**Type definitions****************************************************************************
   using RepresentedType = ElementType_t<MT>;              //!< Type of the represented matrix element.
   using RawReference    = AddReference_t<ReferenceType>;  //!< Reference-to-non-const to the represented element.
   using ConstReference  = const RepresentedType&;         //!< Reference-to-const to the represented element.
   //**********************************************************************************************

   //**Constructors********************************************************************************
   /*!\name Constructors */
   //@{
   inline StrictlyLowerProxy( MT& matrix, size_t row, size_t column );
   inline StrictlyLowerProxy( const StrictlyLowerProxy& ulp );
   //@}
   //**********************************************************************************************

   //**Destructor**********************************************************************************
   /*!\name Destructor */
   //@{
   ~StrictlyLowerProxy() = default;
   //@}
   //**********************************************************************************************

   //**Assignment operators************************************************************************
   /*!\name Assignment operators */
   //@{
   inline const StrictlyLowerProxy& operator=( const StrictlyLowerProxy& ulp ) const;

   template< typename T >
   inline const StrictlyLowerProxy& operator=( initializer_list<T> list ) const;

   template< typename T >
   inline const StrictlyLowerProxy& operator=( initializer_list< initializer_list<T> > list ) const;

   template< typename T > inline const StrictlyLowerProxy& operator= ( const T& value ) const;
   template< typename T > inline const StrictlyLowerProxy& operator+=( const T& value ) const;
   template< typename T > inline const StrictlyLowerProxy& operator-=( const T& value ) const;
   template< typename T > inline const StrictlyLowerProxy& operator*=( const T& value ) const;
   template< typename T > inline const StrictlyLowerProxy& operator/=( const T& value ) const;
   template< typename T > inline const StrictlyLowerProxy& operator%=( const T& value ) const;
   //@}
   //**********************************************************************************************

   //**Access operators****************************************************************************
   /*!\name Access operators */
   //@{
   inline const StrictlyLowerProxy* operator->() const noexcept;
   //@}
   //**********************************************************************************************

   //**Utility functions***************************************************************************
   /*!\name Utility functions */
   //@{
   inline RawReference get()          const noexcept;
   inline bool         isRestricted() const noexcept;
   //@}
   //**********************************************************************************************

   //**Conversion operator*************************************************************************
   /*!\name Conversion operator */
   //@{
   inline operator ConstReference() const noexcept;
   //@}
   //**********************************************************************************************

 private:
   //**Member variables****************************************************************************
   /*!\name Member variables */
   //@{
   ReferenceType value_;    //!< Reference to the accessed matrix element.
   const bool restricted_;  //!< Access flag for the accessed matrix element.
                            /*!< The flag indicates if access to the matrix element is
                                 restricted. It is \a true in case the proxy represents an
                                 element on the diagonal or in the upper part of the matrix. */
   //@}
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CONSTRAINT_MUST_BE_MATRIX_TYPE              ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_REFERENCE_TYPE       ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_POINTER_TYPE         ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_CONST                ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_VOLATILE             ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_VIEW_TYPE            ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE     ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_TRANSFORMATION_TYPE  ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_HERMITIAN_MATRIX_TYPE( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_LOWER_MATRIX_TYPE    ( MT );
   BLAZE_CONSTRAINT_MUST_NOT_BE_UPPER_MATRIX_TYPE    ( MT );
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************




//=================================================================================================
//
//  CONSTRUCTORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Initialization constructor for an StrictlyLowerProxy.
//
// \param matrix Reference to the adapted matrix.
// \param row The row-index of the accessed matrix element.
// \param column The column-index of the accessed matrix element.
*/
template< typename MT >  // Type of the adapted matrix
inline StrictlyLowerProxy<MT>::StrictlyLowerProxy( MT& matrix, size_t row, size_t column )
   : value_     ( matrix( row, column ) )  // Reference to the accessed matrix element
   , restricted_( row <= column )          // Access flag for the accessed matrix element
{}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief The copy constructor for LowerProxy.
//
// \param slp Proxy to be copied.
*/
template< typename MT >  // Type of the adapted matrix
inline StrictlyLowerProxy<MT>::StrictlyLowerProxy( const StrictlyLowerProxy& slp )
   : value_     ( slp.value_      )  // Reference to the accessed matrix element
   , restricted_( slp.restricted_ )  // Access flag for the accessed matrix element
{}
//*************************************************************************************************




//=================================================================================================
//
//  OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Copy assignment operator for StrictlyLowerProxy.
//
// \param slp Proxy to be copied.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or upper matrix element.
//
// In case the proxy represents an element on the diagonal or in the upper part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted matrix
inline const StrictlyLowerProxy<MT>&
   StrictlyLowerProxy<MT>::operator=( const StrictlyLowerProxy& slp ) const
{
   if( restricted_ ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to diagonal or upper matrix element" );
   }

   value_ = slp.value_;

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Initializer list assignment to the accessed matrix element.
//
// \param list The list to be assigned to the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or upper matrix element.
//
// In case the proxy represents an element on the diagonal or in the upper part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted matrix
template< typename T >   // Type of the right-hand side value
inline const StrictlyLowerProxy<MT>&
   StrictlyLowerProxy<MT>::operator=( initializer_list<T> list ) const
{
   if( restricted_ ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to diagonal or upper matrix element" );
   }

   value_ = list;

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Initializer list assignment to the accessed matrix element.
//
// \param list The list to be assigned to the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or upper matrix element.
//
// In case the proxy represents an element on the diagonal or in the upper part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted matrix
template< typename T >   // Type of the right-hand side value
inline const StrictlyLowerProxy<MT>&
   StrictlyLowerProxy<MT>::operator=( initializer_list< initializer_list<T> > list ) const
{
   if( restricted_ ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to diagonal or upper matrix element" );
   }

   value_ = list;

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Assignment to the accessed matrix element.
//
// \param value The new value of the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or upper matrix element.
//
// In case the proxy represents an element on the diagonal or in the upper part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted matrix
template< typename T >   // Type of the right-hand side value
inline const StrictlyLowerProxy<MT>& StrictlyLowerProxy<MT>::operator=( const T& value ) const
{
   if( restricted_ ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to diagonal or upper matrix element" );
   }

   value_ = value;

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Addition assignment to the accessed matrix element.
//
// \param value The right-hand side value to be added to the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or upper matrix element.
//
// In case the proxy represents an element on the diagonal or in the upper part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted matrix
template< typename T >   // Type of the right-hand side value
inline const StrictlyLowerProxy<MT>& StrictlyLowerProxy<MT>::operator+=( const T& value ) const
{
   if( restricted_ ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to diagonal or upper matrix element" );
   }

   value_ += value;

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Subtraction assignment to the accessed matrix element.
//
// \param value The right-hand side value to be subtracted from the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or upper matrix element.
//
// In case the proxy represents an element on the diagonal or in the upper part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted matrix
template< typename T >   // Type of the right-hand side value
inline const StrictlyLowerProxy<MT>& StrictlyLowerProxy<MT>::operator-=( const T& value ) const
{
   if( restricted_ ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to diagonal or upper matrix element" );
   }

   value_ -= value;

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Multiplication assignment to the accessed matrix element.
//
// \param value The right-hand side value for the multiplication.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or upper matrix element.
//
// In case the proxy represents an element on the diagonal or in the upper part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted matrix
template< typename T >   // Type of the right-hand side value
inline const StrictlyLowerProxy<MT>& StrictlyLowerProxy<MT>::operator*=( const T& value ) const
{
   if( restricted_ ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to diagonal or upper matrix element" );
   }

   value_ *= value;

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Division assignment to the accessed matrix element.
//
// \param value The right-hand side value for the division.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or upper matrix element.
//
// In case the proxy represents an element on the diagonal or in the upper part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted matrix
template< typename T >   // Type of the right-hand side value
inline const StrictlyLowerProxy<MT>& StrictlyLowerProxy<MT>::operator/=( const T& value ) const
{
   if( restricted_ ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to diagonal or upper matrix element" );
   }

   value_ /= value;

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Modulo assignment to the accessed matrix element.
//
// \param value The right-hand side value for the modulo operation.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or upper matrix element.
//
// In case the proxy represents an element on the diagonal or in the upper part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template< typename MT >  // Type of the adapted matrix
template< typename T >   // Type of the right-hand side value
inline const StrictlyLowerProxy<MT>& StrictlyLowerProxy<MT>::operator%=( const T& value ) const
{
   if( restricted_ ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to diagonal or upper matrix element" );
   }

   value_ %= value;

   return *this;
}
//*************************************************************************************************




//=================================================================================================
//
//  ACCESS OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Direct access to the accessed matrix element.
//
// \return Pointer to the accessed matrix element.
*/
template< typename MT >  // Type of the adapted matrix
inline const StrictlyLowerProxy<MT>* StrictlyLowerProxy<MT>::operator->() const noexcept
{
   return this;
}
//*************************************************************************************************




//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returning the value of the accessed matrix element.
//
// \return Direct/raw reference to the accessed matrix element.
*/
template< typename MT >  // Type of the adapted matrix
inline typename StrictlyLowerProxy<MT>::RawReference StrictlyLowerProxy<MT>::get() const noexcept
{
   return value_;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the proxy represents a restricted matrix element..
//
// \return \a true in case access to the matrix element is restricted, \a false if not.
*/
template< typename MT >  // Type of the adapted matrix
inline bool StrictlyLowerProxy<MT>::isRestricted() const noexcept
{
   return restricted_;
}
//*************************************************************************************************




//=================================================================================================
//
//  CONVERSION OPERATOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Conversion to the accessed matrix element.
//
// \return Reference-to-const to the accessed matrix element.
*/
template< typename MT >  // Type of the adapted matrix
inline StrictlyLowerProxy<MT>::operator ConstReference() const noexcept
{
   return static_cast<ConstReference>( value_ );
}
//*************************************************************************************************




//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name StrictlyLowerProxy global functions */
//@{
template< typename MT >
void reset( const StrictlyLowerProxy<MT>& proxy );

template< typename MT >
void clear( const StrictlyLowerProxy<MT>& proxy );

template< RelaxationFlag RF, typename MT >
bool isDefault( const StrictlyLowerProxy<MT>& proxy );

template< RelaxationFlag RF, typename MT >
bool isReal( const StrictlyLowerProxy<MT>& proxy );

template< RelaxationFlag RF, typename MT >
bool isZero( const StrictlyLowerProxy<MT>& proxy );

template< RelaxationFlag RF, typename MT >
bool isOne( const StrictlyLowerProxy<MT>& proxy );

template< typename MT >
bool isnan( const StrictlyLowerProxy<MT>& proxy );
//@}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Resetting the represented element to the default initial values.
// \ingroup strictly_lower_matrix
//
// \param proxy The given access proxy.
// \return void
//
// This function resets the element represented by the access proxy to its default initial
// value.
*/
template< typename MT >
inline void reset( const StrictlyLowerProxy<MT>& proxy )
{
   using blaze::reset;

   reset( proxy.get() );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Clearing the represented element.
// \ingroup strictly_lower_matrix
//
// \param proxy The given access proxy.
// \return void
//
// This function clears the element represented by the access proxy to its default initial
// state.
*/
template< typename MT >
inline void clear( const StrictlyLowerProxy<MT>& proxy )
{
   using blaze::clear;

   clear( proxy.get() );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the represented element is in default state.
// \ingroup strictly_lower_matrix
//
// \param proxy The given access proxy
// \return \a true in case the represented element is in default state, \a false otherwise.
//
// This function checks whether the element represented by the access proxy is in default state.
// In case it is in default state, the function returns \a true, otherwise it returns \a false.
*/
template< RelaxationFlag RF, typename MT >
inline bool isDefault( const StrictlyLowerProxy<MT>& proxy )
{
   using blaze::isDefault;

   return isDefault<RF>( proxy.get() );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the matrix element represents a real number.
// \ingroup strictly_lower_matrix
//
// \param proxy The given access proxy.
// \return \a true in case the matrix element represents a real number, \a false otherwise.
//
// This function checks whether the element represented by the access proxy represents the a
// real number. In case the element is of built-in type, the function returns \a true. In case
// the element is of complex type, the function returns \a true if the imaginary part is equal
// to 0. Otherwise it returns \a false.
*/
template< RelaxationFlag RF, typename MT >
inline bool isReal( const StrictlyLowerProxy<MT>& proxy )
{
   using blaze::isReal;

   return isReal<RF>( proxy.get() );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the represented element is 0.
// \ingroup strictly_lower_matrix
//
// \param proxy The given access proxy.
// \return \a true in case the represented element is 0, \a false otherwise.
//
// This function checks whether the element represented by the access proxy represents the numeric
// value 0. In case it is 0, the function returns \a true, otherwise it returns \a false.
*/
template< RelaxationFlag RF, typename MT >
inline bool isZero( const StrictlyLowerProxy<MT>& proxy )
{
   using blaze::isZero;

   return isZero<RF>( proxy.get() );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the represented element is 1.
// \ingroup strictly_lower_matrix
//
// \param proxy The given access proxy.
// \return \a true in case the represented element is 1, \a false otherwise.
//
// This function checks whether the element represented by the access proxy represents the numeric
// value 1. In case it is 1, the function returns \a true, otherwise it returns \a false.
*/
template< RelaxationFlag RF, typename MT >
inline bool isOne( const StrictlyLowerProxy<MT>& proxy )
{
   using blaze::isOne;

   return isOne<RF>( proxy.get() );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the represented element is not a number.
// \ingroup strictly_lower_matrix
//
// \param proxy The given access proxy.
// \return \a true in case the represented element is in not a number, \a false otherwise.
//
// This function checks whether the element represented by the access proxy is not a number (NaN).
// In case it is not a number, the function returns \a true, otherwise it returns \a false.
*/
template< typename MT >
inline bool isnan( const StrictlyLowerProxy<MT>& proxy )
{
   using blaze::isnan;

   return isnan( proxy.get() );
}
//*************************************************************************************************

} // namespace blaze

#endif
