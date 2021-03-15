//=================================================================================================
/*!
//  \file blaze/math/dense/StaticVector.h
//  \brief Header file for the implementation of a fixed-size vector
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

#ifndef _BLAZE_MATH_DENSE_STATICVECTOR_H_
#define _BLAZE_MATH_DENSE_STATICVECTOR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <array>
#include <tuple>
#include <utility>
#include "../../math/Aliases.h"
#include "../../math/AlignmentFlag.h"
#include "../../math/constraints/DenseVector.h"
#include "../../math/constraints/RequiresEvaluation.h"
#include "../../math/constraints/TransposeFlag.h"
#include "../../math/dense/DenseIterator.h"
#include "../../math/dense/Forward.h"
#include "../../math/Exception.h"
#include "../../math/expressions/DenseVector.h"
#include "../../math/expressions/SparseVector.h"
#include "../../math/Forward.h"
#include "../../math/InitializerList.h"
#include "../../math/PaddingFlag.h"
#include "../../math/ReductionFlag.h"
#include "../../math/RelaxationFlag.h"
#include "../../math/shims/Clear.h"
#include "../../math/shims/IsDefault.h"
#include "../../math/shims/NextMultiple.h"
#include "../../math/shims/PrevMultiple.h"
#include "../../math/shims/Serial.h"
#include "../../math/SIMD.h"
#include "../../math/traits/AddTrait.h"
#include "../../math/traits/BandTrait.h"
#include "../../math/traits/ColumnTrait.h"
#include "../../math/traits/CrossTrait.h"
#include "../../math/traits/DivTrait.h"
#include "../../math/traits/ElementsTrait.h"
#include "../../math/traits/KronTrait.h"
#include "../../math/traits/MapTrait.h"
#include "../../math/traits/MultTrait.h"
#include "../../math/traits/ReduceTrait.h"
#include "../../math/traits/RowTrait.h"
#include "../../math/traits/SolveTrait.h"
#include "../../math/traits/SubTrait.h"
#include "../../math/traits/SubvectorTrait.h"
#include "../../math/typetraits/HasConstDataAccess.h"
#include "../../math/typetraits/HasMutableDataAccess.h"
#include "../../math/typetraits/HasSIMDAdd.h"
#include "../../math/typetraits/HasSIMDDiv.h"
#include "../../math/typetraits/HasSIMDMult.h"
#include "../../math/typetraits/HasSIMDSub.h"
#include "../../math/typetraits/HighType.h"
#include "../../math/typetraits/IsAligned.h"
#include "../../math/typetraits/IsColumnVector.h"
#include "../../math/typetraits/IsContiguous.h"
#include "../../math/typetraits/IsDenseMatrix.h"
#include "../../math/typetraits/IsDenseVector.h"
#include "../../math/typetraits/IsMatrix.h"
#include "../../math/typetraits/IsPadded.h"
#include "../../math/typetraits/IsRowVector.h"
#include "../../math/typetraits/IsSIMDCombinable.h"
#include "../../math/typetraits/IsSparseVector.h"
#include "../../math/typetraits/IsSquare.h"
#include "../../math/typetraits/IsStatic.h"
#include "../../math/typetraits/IsVector.h"
#include "../../math/typetraits/LowType.h"
#include "../../math/typetraits/MaxSize.h"
#include "../../math/typetraits/Size.h"
#include "../../math/typetraits/TransposeFlag.h"
#include "../../system/Inline.h"
#include "../../system/Optimizations.h"
#include "../../system/Standard.h"
#include "../../system/TransposeFlag.h"
#include "../../util/algorithms/Max.h"
#include "../../util/algorithms/Min.h"
#include "../../util/AlignedArray.h"
#include "../../util/AlignmentCheck.h"
#include "../../util/Assert.h"
#include "../../util/constraints/Const.h"
#include "../../util/constraints/Pointer.h"
#include "../../util/constraints/Reference.h"
#include "../../util/constraints/Vectorizable.h"
#include "../../util/constraints/Volatile.h"
#include "../../util/EnableIf.h"
#include "../../util/IntegralConstant.h"
#include "../../util/MaybeUnused.h"
#include "../../util/Memory.h"
#include "../../util/StaticAssert.h"
#include "../../util/Types.h"
#include "../../util/typetraits/AlignmentOf.h"
#include "../../util/typetraits/IsNumeric.h"
#include "../../util/typetraits/IsVectorizable.h"
#include "../../util/typetraits/RemoveConst.h"
#include "../../util/typetraits/RemoveCV.h"


namespace blaze {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\defgroup static_vector StaticVector
// \ingroup dense_vector
*/
/*!\brief Efficient implementation of a fixed-sized vector.
// \ingroup static_vector
//
// The StaticVector class template is the representation of a fixed-size vector with statically
// allocated elements of arbitrary type. The type of the elements, the number of elements, the
// transpose flag, the alignment, and the padding of the vector can be specified via the five
// template parameters:

   \code
   template< typename Type, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
   class StaticVector;
   \endcode

//  - Type: specifies the type of the vector elements. StaticVector can be used with any
//          non-cv-qualified, non-reference, non-pointer element type.
//  - N   : specifies the total number of vector elements. It is expected that StaticVector is
//          only used for tiny and small vectors.
//  - TF  : specifies whether the vector is a row vector (\a blaze::rowVector) or a column
//          vector (\a blaze::columnVector). The default value is \a blaze::columnVector.
//  - AF  : specifies whether the first element of the vector is properly aligned with respect to
//          the available instruction set (SSE, AVX, ...). Possible values are \a blaze::aligned
//          and \a blaze::unaligned. The default value is \a blaze::aligned.
//  - PF  : specifies whether the vector should be padded to maximize the efficiency of vectorized
//          operations. Possible values are \a blaze::padded and \a blaze::unpadded. The default
//          value is \a blaze::padded.
//
// These contiguously stored elements can be directly accessed with the subscript operator. The
// numbering of the vector elements is

                             \f[\left(\begin{array}{*{4}{c}}
                             0 & 1 & \cdots & N-1 \\
                             \end{array}\right)\f]

// The use of StaticVector is very natural and intuitive. All operations (addition, subtraction,
// multiplication, scaling, ...) can be performed on all possible combinations of dense and sparse
// vectors with fitting element types. The following example gives an impression of the use of a
// 2-dimensional StaticVector:

   \code
   using blaze::StaticVector;
   using blaze::CompressedVector;
   using blaze::StaticMatrix;

   StaticVector<double,2UL> a;  // Default initialized 2D vector
   a[0] = 1.0;                  // Initialization of the first element
   a[1] = 2.0;                  // Initialization of the second element

   StaticVector<double,2UL> b( 3.0, 2.0 );  // Directly initialized 2D vector
   CompressedVector<float>  c( 2 );         // Empty sparse single precision vector
   StaticVector<double,2UL> d;              // Default constructed static vector
   StaticMatrix<double,2UL,2UL> A;          // Default constructed static row-major matrix

   d = a + b;  // Vector addition between vectors of equal element type
   d = a - c;  // Vector subtraction between a dense and sparse vector with different element types
   d = a * b;  // Component-wise vector multiplication

   a *= 2.0;      // In-place scaling of vector
   d  = a * 2.0;  // Scaling of vector a
   d  = 2.0 * a;  // Scaling of vector a

   d += a - b;  // Addition assignment
   d -= a + c;  // Subtraction assignment
   d *= a * b;  // Multiplication assignment

   double scalar = trans( a ) * b;  // Scalar/dot/inner product between two vectors

   A = a * trans( b );  // Outer product between two vectors
   \endcode
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
class StaticVector
   : public DenseVector< StaticVector<Type,N,TF,AF,PF>, TF >
{
 public:
   //**Type definitions****************************************************************************
   using This          = StaticVector<Type,N,TF,AF,PF>;   //!< Type of this StaticVector instance.
   using BaseType      = DenseVector<This,TF>;            //!< Base type of this StaticVector instance.
   using ResultType    = This;                            //!< Result type for expression template evaluations.
   using TransposeType = StaticVector<Type,N,!TF,AF,PF>;  //!< Transpose type for expression template evaluations.
   using ElementType   = Type;                            //!< Type of the vector elements.
   using SIMDType      = SIMDTrait_t<ElementType>;        //!< SIMD type of the vector elements.
   using ReturnType    = const Type&;                     //!< Return type for expression template evaluations.
   using CompositeType = const StaticVector&;             //!< Data type for composite expression templates.

   using Reference      = Type&;        //!< Reference to a non-constant vector value.
   using ConstReference = const Type&;  //!< Reference to a constant vector value.
   using Pointer        = Type*;        //!< Pointer to a non-constant vector value.
   using ConstPointer   = const Type*;  //!< Pointer to a constant vector value.

   using Iterator      = DenseIterator<Type,AF>;        //!< Iterator over non-constant elements.
   using ConstIterator = DenseIterator<const Type,AF>;  //!< Iterator over constant elements.
   //**********************************************************************************************

   //**Rebind struct definition********************************************************************
   /*!\brief Rebind mechanism to obtain a StaticVector with different data/element type.
   */
   template< typename NewType >  // Data type of the other vector
   struct Rebind {
      using Other = StaticVector<NewType,N,TF,AF,PF>;  //!< The type of the other StaticVector.
   };
   //**********************************************************************************************

   //**Resize struct definition********************************************************************
   /*!\brief Resize mechanism to obtain a StaticVector with a different fixed number of elements.
   */
   template< size_t NewN >  // Number of elements of the other vector
   struct Resize {
      using Other = StaticVector<Type,NewN,TF,AF,PF>;  //!< The type of the other StaticVector.
   };
   //**********************************************************************************************

   //**Compilation flags***************************************************************************
   //! Compilation flag for SIMD optimization.
   /*! The \a simdEnabled compilation flag indicates whether expressions the vector is involved
       in can be optimized via SIMD operations. In case the element type of the vector is a
       vectorizable data type, the \a simdEnabled compilation flag is set to \a true, otherwise
       it is set to \a false. */
   static constexpr bool simdEnabled = IsVectorizable_v<Type>;

   //! Compilation flag for SMP assignments.
   /*! The \a smpAssignable compilation flag indicates whether the vector can be used in SMP
       (shared memory parallel) assignments (both on the left-hand and right-hand side of the
       assignment). */
   static constexpr bool smpAssignable = false;
   //**********************************************************************************************

   //**Constructors********************************************************************************
   /*!\name Constructors */
   //@{
            inline    StaticVector();
   explicit inline    StaticVector( const Type& init );
            constexpr StaticVector( initializer_list<Type> list );

   template< typename Other >
   inline StaticVector( size_t n, const Other* array );

   template< typename Other, size_t Dim >
   constexpr StaticVector( const Other (&array)[Dim] );

   template< typename Other, size_t Dim >
   constexpr StaticVector( const std::array<Other,Dim>& array );

   constexpr StaticVector( const StaticVector& v );

   template< typename Other, AlignmentFlag AF2, PaddingFlag PF2 >
   inline StaticVector( const StaticVector<Other,N,TF,AF2,PF2>& v );

   template< typename VT >
   inline StaticVector( const Vector<VT,TF>& v );
   //@}
   //**********************************************************************************************

   //**Destructor**********************************************************************************
   /*!\name Destructor */
   //@{
   ~StaticVector() = default;
   //@}
   //**********************************************************************************************

   //**Data access functions***********************************************************************
   /*!\name Data access functions */
   //@{
   constexpr Reference      operator[]( size_t index ) noexcept;
   constexpr ConstReference operator[]( size_t index ) const noexcept;
   inline    Reference      at( size_t index );
   inline    ConstReference at( size_t index ) const;
   constexpr Pointer        data  () noexcept;
   constexpr ConstPointer   data  () const noexcept;
   constexpr Iterator       begin () noexcept;
   constexpr ConstIterator  begin () const noexcept;
   constexpr ConstIterator  cbegin() const noexcept;
   constexpr Iterator       end   () noexcept;
   constexpr ConstIterator  end   () const noexcept;
   constexpr ConstIterator  cend  () const noexcept;
   //@}
   //**********************************************************************************************

   //**Assignment operators************************************************************************
   /*!\name Assignment operators */
   //@{
   constexpr StaticVector& operator=( const Type& rhs );
   constexpr StaticVector& operator=( initializer_list<Type> list );

   template< typename Other, size_t Dim >
   constexpr StaticVector& operator=( const Other (&array)[Dim] );

   template< typename Other, size_t Dim >
   constexpr StaticVector& operator=( const std::array<Other,Dim>& array );

   constexpr StaticVector& operator=( const StaticVector& rhs );

   template< typename Other, AlignmentFlag AF2, PaddingFlag PF2 >
   inline StaticVector& operator=( const StaticVector<Other,N,TF,AF2,PF2>& rhs );

   template< typename VT > inline StaticVector& operator= ( const Vector<VT,TF>& rhs );
   template< typename VT > inline StaticVector& operator+=( const Vector<VT,TF>& rhs );
   template< typename VT > inline StaticVector& operator-=( const Vector<VT,TF>& rhs );
   template< typename VT > inline StaticVector& operator*=( const Vector<VT,TF>& rhs );
   template< typename VT > inline StaticVector& operator/=( const DenseVector<VT,TF>& rhs );
   template< typename VT > inline StaticVector& operator%=( const Vector<VT,TF>& rhs );
   //@}
   //**********************************************************************************************

   //**Utility functions***************************************************************************
   /*!\name Utility functions */
   //@{
   static constexpr size_t size() noexcept;
   static constexpr size_t spacing() noexcept;
   static constexpr size_t capacity() noexcept;
          inline    size_t nonZeros() const;
          constexpr void   reset();
          inline    void   swap( StaticVector& v ) noexcept;
   //@}
   //**********************************************************************************************

   //**Numeric functions***************************************************************************
   /*!\name Numeric functions */
   //@{
   template< typename Other > inline StaticVector& scale( const Other& scalar );
   //@}
   //**********************************************************************************************

   //**Memory functions****************************************************************************
   /*!\name Memory functions */
   //@{
   static inline void* operator new  ( std::size_t size );
   static inline void* operator new[]( std::size_t size );
   static inline void* operator new  ( std::size_t size, const std::nothrow_t& );
   static inline void* operator new[]( std::size_t size, const std::nothrow_t& );

   static inline void operator delete  ( void* ptr );
   static inline void operator delete[]( void* ptr );
   static inline void operator delete  ( void* ptr, const std::nothrow_t& );
   static inline void operator delete[]( void* ptr, const std::nothrow_t& );
   //@}
   //**********************************************************************************************

 private:
   //**********************************************************************************************
   //! The number of elements packed within a single SIMD vector.
   static constexpr size_t SIMDSIZE = SIMDTrait<Type>::size;

   //! Alignment adjustment.
   static constexpr size_t NN = ( PF == padded ? nextMultiple( N, SIMDSIZE ) : N );
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper variable template for the explicit application of the SFINAE principle.
   template< typename VT >
   static constexpr bool VectorizedAssign_v =
      ( useOptimizedKernels &&
        NN >= SIMDSIZE &&
        simdEnabled && VT::simdEnabled &&
        IsSIMDCombinable_v< Type, ElementType_t<VT> > );
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper variable template for the explicit application of the SFINAE principle.
   template< typename VT >
   static constexpr bool VectorizedAddAssign_v =
      ( VectorizedAssign_v<VT> &&
        HasSIMDAdd_v< Type, ElementType_t<VT> > );
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper variable template for the explicit application of the SFINAE principle.
   template< typename VT >
   static constexpr bool VectorizedSubAssign_v =
      ( VectorizedAssign_v<VT> &&
        HasSIMDSub_v< Type, ElementType_t<VT> > );
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper variable template for the explicit application of the SFINAE principle.
   template< typename VT >
   static constexpr bool VectorizedMultAssign_v =
      ( VectorizedAssign_v<VT> &&
        HasSIMDMult_v< Type, ElementType_t<VT> > );
   /*! \endcond */
   //**********************************************************************************************

   //**********************************************************************************************
   /*! \cond BLAZE_INTERNAL */
   //! Helper variable template for the explicit application of the SFINAE principle.
   template< typename VT >
   static constexpr bool VectorizedDivAssign_v =
      ( VectorizedAssign_v<VT> &&
        HasSIMDDiv_v< Type, ElementType_t<VT> > );
   /*! \endcond */
   //**********************************************************************************************

 public:
   //**Debugging functions*************************************************************************
   /*!\name Debugging functions */
   //@{
   constexpr bool isIntact() const noexcept;
   //@}
   //**********************************************************************************************

   //**Expression template evaluation functions****************************************************
   /*!\name Expression template evaluation functions */
   //@{
   template< typename Other > inline bool canAlias ( const Other* alias ) const noexcept;
   template< typename Other > inline bool isAliased( const Other* alias ) const noexcept;

   static constexpr bool isAligned() noexcept;

   BLAZE_ALWAYS_INLINE SIMDType load ( size_t index ) const noexcept;
   BLAZE_ALWAYS_INLINE SIMDType loada( size_t index ) const noexcept;
   BLAZE_ALWAYS_INLINE SIMDType loadu( size_t index ) const noexcept;

   BLAZE_ALWAYS_INLINE void store ( size_t index, const SIMDType& value ) noexcept;
   BLAZE_ALWAYS_INLINE void storea( size_t index, const SIMDType& value ) noexcept;
   BLAZE_ALWAYS_INLINE void storeu( size_t index, const SIMDType& value ) noexcept;
   BLAZE_ALWAYS_INLINE void stream( size_t index, const SIMDType& value ) noexcept;

   template< typename VT >
   inline auto assign( const DenseVector<VT,TF>& rhs ) -> DisableIf_t< VectorizedAssign_v<VT> >;

   template< typename VT >
   inline auto assign( const DenseVector<VT,TF>& rhs ) -> EnableIf_t< VectorizedAssign_v<VT> >;

   template< typename VT > inline void assign( const SparseVector<VT,TF>& rhs );

   template< typename VT >
   inline auto addAssign( const DenseVector<VT,TF>& rhs ) -> DisableIf_t< VectorizedAddAssign_v<VT> >;

   template< typename VT >
   inline auto addAssign( const DenseVector<VT,TF>& rhs ) -> EnableIf_t< VectorizedAddAssign_v<VT> >;

   template< typename VT > inline void addAssign( const SparseVector<VT,TF>& rhs );

   template< typename VT >
   inline auto subAssign( const DenseVector<VT,TF>& rhs ) -> DisableIf_t< VectorizedSubAssign_v<VT> >;

   template< typename VT >
   inline auto subAssign( const DenseVector<VT,TF>& rhs ) -> EnableIf_t< VectorizedSubAssign_v<VT> >;

   template< typename VT > inline void subAssign( const SparseVector<VT,TF>& rhs );

   template< typename VT >
   inline auto multAssign( const DenseVector<VT,TF>& rhs ) -> DisableIf_t< VectorizedMultAssign_v<VT> >;

   template< typename VT >
   inline auto multAssign( const DenseVector<VT,TF>& rhs ) -> EnableIf_t< VectorizedMultAssign_v<VT> >;

   template< typename VT > inline void multAssign( const SparseVector<VT,TF>& rhs );

   template< typename VT >
   inline auto divAssign( const DenseVector<VT,TF>& rhs ) -> DisableIf_t< VectorizedDivAssign_v<VT> >;

   template< typename VT >
   inline auto divAssign( const DenseVector<VT,TF>& rhs ) -> EnableIf_t< VectorizedDivAssign_v<VT> >;
   //@}
   //**********************************************************************************************

 private:
   //**********************************************************************************************
   //! Alignment of the data elements.
   static constexpr size_t Alignment =
      ( AF == aligned ? AlignmentOf_v<Type> : std::alignment_of<Type>::value );

   //! Type of the aligned storage.
   using AlignedStorage = AlignedArray<Type,NN,Alignment>;
   //**********************************************************************************************

   //**Member variables****************************************************************************
   /*!\name Member variables */
   //@{
   AlignedStorage v_;  //!< The statically allocated vector elements.
                       /*!< Access to the vector values is gained via the subscript operator.
                            The order of the elements is
                            \f[\left(\begin{array}{*{4}{c}}
                            0 & 1 & \cdots & N-1 \\
                            \end{array}\right)\f] */
   //@}
   //**********************************************************************************************

   //**Compile time checks*************************************************************************
   /*! \cond BLAZE_INTERNAL */
   BLAZE_CONSTRAINT_MUST_NOT_BE_POINTER_TYPE  ( Type );
   BLAZE_CONSTRAINT_MUST_NOT_BE_REFERENCE_TYPE( Type );
   BLAZE_CONSTRAINT_MUST_NOT_BE_CONST         ( Type );
   BLAZE_CONSTRAINT_MUST_NOT_BE_VOLATILE      ( Type );
   BLAZE_STATIC_ASSERT( PF == unpadded || ( NN % SIMDSIZE == 0UL ) );
   BLAZE_STATIC_ASSERT( NN >= N );
   BLAZE_STATIC_ASSERT( IsVectorizable_v<Type> || NN == N );
   /*! \endcond */
   //**********************************************************************************************
};
//*************************************************************************************************




//=================================================================================================
//
//  DEDUCTION GUIDES
//
//=================================================================================================

//*************************************************************************************************
#if BLAZE_CPP17_MODE

template< typename Type, typename... Ts >
StaticVector( Type, Ts... ) -> StaticVector<Type,1+sizeof...(Ts)>;

template< typename Type, size_t N >
StaticVector( Type (&)[N] ) -> StaticVector< RemoveCV_t<Type>, N >;

template< typename Type, size_t N >
StaticVector( std::array<Type,N> ) -> StaticVector<Type,N>;

#endif
//*************************************************************************************************




//=================================================================================================
//
//  CONSTRUCTORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief The default constructor for StaticVector.
//
// All vector elements are initialized to the default value (i.e. 0 for integral data types).
//
// Note that it is possible to skip the default initialization by means of the
// \a BLAZE_USE_DEFAULT_INITIALIZATION configuration switch. In case the switch is set to 1
// all elements are initialized to their respective default. In case the switch is set to 0 the
// default initialization is skipped and the elements are not initialized. Please note that this
// switch is only effective in case the elements are of fundamental type (i.e. integral or
// floating point). In case the elements are of class type, this switch has no effect. See
// the <tt>"../../config/Optimizations.h"</tt> configuration file for more details.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline StaticVector<Type,N,TF,AF,PF>::StaticVector()
#if BLAZE_USE_DEFAULT_INITIALIZATION
   : v_()  // The statically allocated vector elements
#endif
{
#if !BLAZE_USE_DEFAULT_INITIALIZATION
   if( IsNumeric_v<Type> && PF == unpadded ) {
      for( size_t i=N; i<NN; ++i )
         v_[i] = Type();
   }
#endif

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Constructor for a homogenous initialization of all elements.
//
// \param init Initial value for all vector elements.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline StaticVector<Type,N,TF,AF,PF>::StaticVector( const Type& init )
   // v_ is intentionally left uninitialized
{
   for( size_t i=0UL; i<N; ++i )
      v_[i] = init;

   for( size_t i=N; i<NN; ++i )
      v_[i] = Type();

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief List initialization of all vector elements.
//
// \param list The initializer list.
// \exception std::invalid_argument Invalid setup of static vector.
//
// This constructor provides the option to explicitly initialize the elements of the vector by
// means of an initializer list:

   \code
   blaze::StaticVector<double,3UL> v1{ 4.2, 6.3, -1.2 };
   \endcode

// The vector elements are (copy) assigned the values of the given initializer list. Missing values
// are initialized as default. Note that in case the size of the initializer list exceeds the size
// of the vector, a \a std::invalid_argument exception is thrown.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr StaticVector<Type,N,TF,AF,PF>::StaticVector( initializer_list<Type> list )
   : v_()  // The statically allocated vector elements
{
   if( list.size() > N ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid setup of static vector" );
   }

   size_t i( 0UL );

   for( const auto& element : list ) {
      v_[i] = element;
      ++i;
   }

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Array initialization of all vector elements.
//
// \param n The size of the vector.
// \param array Dynamic array for the initialization.
//
// This constructor offers the option to directly initialize the elements of the vector with a
// dynamic array:

   \code
   const double array* = new double[2];
   // ... Initialization of the array
   blaze::StaticVector<double,2> v( 2UL, array );
   delete[] array;
   \endcode

// The vector is initialized with the values from the given array. Missing values are initialized
// with default values. In case the size of the given vector exceeds the maximum size of the
// static vector (i.e. is larger than N), a \a std::invalid_argument exception is thrown.\n
// Note that it is expected that the given \a array has at least \a n elements. Providing an
// array with less elements results in undefined behavior!
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename Other >  // Data type of the initialization array
inline StaticVector<Type,N,TF,AF,PF>::StaticVector( size_t n, const Other* array )
   // v_ is intentionally left uninitialized
{
   if( n > N ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid setup of static vector" );
   }

   for( size_t i=0UL; i<n; ++i )
      v_[i] = array[i];

   if( IsNumeric_v<Type> ) {
      for( size_t i=n; i<NN; ++i )
         v_[i] = Type();
   }

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Array initialization of all vector elements.
//
// \param array Static array for the initialization.
//
// This constructor operator offers the option to directly initialize the elements of the vector
// with a static array:

   \code
   const double init[3] = { 1.0, 2.0 };
   blaze::StaticVector<double,3> v( init );
   \endcode

// The vector is initialized with the values from the given static array. Whereas the dimensions
// of the vector and the static array must match, it is allowed to provide fewer initializers for
// the static array. Missing values are initialized with default values (as e.g. the third value
// in the example).
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename Other    // Data type of the static array
        , size_t Dim >      // Dimension of the static array
constexpr StaticVector<Type,N,TF,AF,PF>::StaticVector( const Other (&array)[Dim] )
   : v_( array )  // The statically allocated vector elements
{
   BLAZE_STATIC_ASSERT( Dim == N );

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Initialization of all vector elements from the given std::array.
//
// \param array The given std::array for the initialization.
//
// This constructor operator offers the option to directly initialize the elements of the vector
// with a std::array:

   \code
   std::array<double,3> init{ 1.0, 2.0 };
   blaze::StaticVector<double,3> v( init );
   \endcode

// The vector is initialized with the values from the given std::array. Whereas the dimensions
// of the vector and the std::array must match, it is allowed to provide fewer initializers for
// the std::array. Missing values are initialized with default values (as e.g. the third value
// in the example).
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename Other    // Data type of the std::array
        , size_t Dim >      // Dimension of the std::array
constexpr StaticVector<Type,N,TF,AF,PF>::StaticVector( const std::array<Other,Dim>& array )
   : v_( array )  // The statically allocated vector elements
{
   BLAZE_STATIC_ASSERT( Dim == N );

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief The copy constructor for StaticVector.
//
// \param v Vector to be copied.
//
// The copy constructor is explicitly defined in order to enable/facilitate NRV optimization.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr StaticVector<Type,N,TF,AF,PF>::StaticVector( const StaticVector& v )
   : BaseType()  // Initialization of the base class
   , v_( v.v_ )  // The statically allocated vector elements
{
   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Conversion constructor from different StaticVector instances.
//
// \param v Vector to be copied.
*/
template< typename Type      // Data type of the vector
        , size_t N           // Number of elements
        , bool TF            // Transpose flag
        , AlignmentFlag AF   // Alignment flag
        , PaddingFlag PF >   // Padding flag
template< typename Other     // Data type of the foreign vector
        , AlignmentFlag AF2  // Alignment flag of the foreign vector
        , PaddingFlag PF2 >  // Padding flag of the foreign vector
inline StaticVector<Type,N,TF,AF,PF>::StaticVector( const StaticVector<Other,N,TF,AF2,PF2>& v )
   // v_ is intentionally left uninitialized
{
   for( size_t i=0UL; i<N; ++i )
      v_[i] = v[i];

   for( size_t i=N; i<NN; ++i )
      v_[i] = Type();

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Conversion constructor from different vectors.
//
// \param v Vector to be copied.
// \exception std::invalid_argument Invalid setup of static vector.
//
// This constructor initializes the static vector from the given vector. In case the size
// of the given vector does not match the size of the static vector (i.e. is not N), a
// \a std::invalid_argument exception is thrown.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the foreign vector
inline StaticVector<Type,N,TF,AF,PF>::StaticVector( const Vector<VT,TF>& v )
   // v_ is intentionally left uninitialized
{
   using blaze::assign;

   if( (~v).size() != N ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid setup of static vector" );
   }

   for( size_t i=( IsSparseVector_v<VT> ? 0UL : N ); i<NN; ++i ) {
      v_[i] = Type();
   }

   assign( *this, ~v );

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );
}
//*************************************************************************************************




//=================================================================================================
//
//  DATA ACCESS FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Subscript operator for the direct access to the vector elements.
//
// \param index Access index. The index has to be in the range \f$[0..N-1]\f$.
// \return Reference to the accessed value.
//
// This function only performs an index check in case BLAZE_USER_ASSERT() is active. In contrast,
// the at() function is guaranteed to perform a check of the given access index.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr typename StaticVector<Type,N,TF,AF,PF>::Reference
   StaticVector<Type,N,TF,AF,PF>::operator[]( size_t index ) noexcept
{
   BLAZE_USER_ASSERT( index < N, "Invalid vector access index" );
   return v_[index];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Subscript operator for the direct access to the vector elements.
//
// \param index Access index. The index has to be in the range \f$[0..N-1]\f$.
// \return Reference-to-const to the accessed value.
//
// This function only performs an index check in case BLAZE_USER_ASSERT() is active. In contrast,
// the at() function is guaranteed to perform a check of the given access index.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr typename StaticVector<Type,N,TF,AF,PF>::ConstReference
   StaticVector<Type,N,TF,AF,PF>::operator[]( size_t index ) const noexcept
{
   BLAZE_USER_ASSERT( index < N, "Invalid vector access index" );
   return v_[index];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Checked access to the vector elements.
//
// \param index Access index. The index has to be in the range \f$[0..N-1]\f$.
// \return Reference to the accessed value.
// \exception std::out_of_range Invalid vector access index.
//
// In contrast to the subscript operator this function always performs a check of the given
// access index.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline typename StaticVector<Type,N,TF,AF,PF>::Reference
   StaticVector<Type,N,TF,AF,PF>::at( size_t index )
{
   if( index >= N ) {
      BLAZE_THROW_OUT_OF_RANGE( "Invalid vector access index" );
   }
   return (*this)[index];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Checked access to the vector elements.
//
// \param index Access index. The index has to be in the range \f$[0..N-1]\f$.
// \return Reference to the accessed value.
// \exception std::out_of_range Invalid vector access index.
//
// In contrast to the subscript operator this function always performs a check of the given
// access index.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline typename StaticVector<Type,N,TF,AF,PF>::ConstReference
   StaticVector<Type,N,TF,AF,PF>::at( size_t index ) const
{
   if( index >= N ) {
      BLAZE_THROW_OUT_OF_RANGE( "Invalid vector access index" );
   }
   return (*this)[index];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Low-level data access to the vector elements.
//
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage of the static vector.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr typename StaticVector<Type,N,TF,AF,PF>::Pointer
   StaticVector<Type,N,TF,AF,PF>::data() noexcept
{
   return v_;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Low-level data access to the vector elements.
//
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage of the static vector.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr typename StaticVector<Type,N,TF,AF,PF>::ConstPointer
   StaticVector<Type,N,TF,AF,PF>::data() const noexcept
{
   return v_;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first element of the static vector.
//
// \return Iterator to the first element of the static vector.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr typename StaticVector<Type,N,TF,AF,PF>::Iterator
   StaticVector<Type,N,TF,AF,PF>::begin() noexcept
{
   return Iterator( v_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first element of the static vector.
//
// \return Iterator to the first element of the static vector.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr typename StaticVector<Type,N,TF,AF,PF>::ConstIterator
   StaticVector<Type,N,TF,AF,PF>::begin() const noexcept
{
   return ConstIterator( v_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator to the first element of the static vector.
//
// \return Iterator to the first element of the static vector.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr typename StaticVector<Type,N,TF,AF,PF>::ConstIterator
   StaticVector<Type,N,TF,AF,PF>::cbegin() const noexcept
{
   return ConstIterator( v_ );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of the static vector.
//
// \return Iterator just past the last element of the static vector.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr typename StaticVector<Type,N,TF,AF,PF>::Iterator
   StaticVector<Type,N,TF,AF,PF>::end() noexcept
{
   return Iterator( v_ + N );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of the static vector.
//
// \return Iterator just past the last element of the static vector.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr typename StaticVector<Type,N,TF,AF,PF>::ConstIterator
   StaticVector<Type,N,TF,AF,PF>::end() const noexcept
{
   return ConstIterator( v_ + N );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of the static vector.
//
// \return Iterator just past the last element of the static vector.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr typename StaticVector<Type,N,TF,AF,PF>::ConstIterator
   StaticVector<Type,N,TF,AF,PF>::cend() const noexcept
{
   return ConstIterator( v_ + N );
}
//*************************************************************************************************




//=================================================================================================
//
//  ASSIGNMENT OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Homogenous assignment to all vector elements.
//
// \param rhs Scalar value to be assigned to all vector elements.
// \return Reference to the assigned vector.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::operator=( const Type& rhs )
{
   for( size_t i=0UL; i<N; ++i )
      v_[i] = rhs;
   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief List assignment to all vector elements.
//
// \param list The initializer list.
// \exception std::invalid_argument Invalid assignment to static vector.
//
// This assignment operator offers the option to directly assign to all elements of the vector
// by means of an initializer list:

   \code
   blaze::StaticVector<double,3UL> v;
   v = { 4.2, 6.3, -1.2 };
   \endcode

// The vector elements are (copy) assigned the values from the given initializer list. Missing
// values are reset to their default state. Note that in case the size of the initializer list
// exceeds the size of the vector, a \a std::invalid_argument exception is thrown.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::operator=( initializer_list<Type> list )
{
   if( list.size() > N ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to static vector" );
   }

   size_t i( 0UL );

   for( const auto& element : list ) {
      v_[i] = element;
      ++i;
   }

   for( ; i<N; ++i ) {
      v_[i] = Type();
   }

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Array assignment to all vector elements.
//
// \param array Static array for the assignment.
// \return Reference to the assigned vector.
//
// This assignment operator offers the option to directly set all elements of the vector:

   \code
   const double init[3] = { 1.0, 2.0 };
   blaze::StaticVector<double,3> v;
   v = init;
   \endcode

// The vector is assigned the values from the given static array. Whereas the dimensions of the
// vector and the static array must match, it is allowed to provide fewer initializers for the
// static array. Missing values are initialized with default values (as e.g. the third value in
// the example).
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename Other    // Data type of the static array
        , size_t Dim >      // Dimension of the static array
constexpr StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::operator=( const Other (&array)[Dim] )
{
   BLAZE_STATIC_ASSERT( Dim == N );

   for( size_t i=0UL; i<N; ++i )
      v_[i] = array[i];

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Array assignment to all vector elements.
//
// \param array The given std::array for the assignment.
// \return Reference to the assigned vector.
//
// This assignment operator offers the option to directly set all elements of the vector:

   \code
   const std::array<double,3> init{ 1.0, 2.0 };
   blaze::StaticVector<double,3> v;
   v = init;
   \endcode

// The vector is assigned the values from the given std::array. Whereas the dimensions of the
// vector and the std::array must match, it is allowed to provide fewer initializers for the
// std::array. Missing values are initialized with default values (as e.g. the third value in
// the example).
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename Other    // Data type of the std::array
        , size_t Dim >      // Dimension of the std::array
constexpr StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::operator=( const std::array<Other,Dim>& array )
{
   BLAZE_STATIC_ASSERT( Dim == N );

   for( size_t i=0UL; i<N; ++i )
      v_[i] = array[i];

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Copy assignment operator for StaticVector.
//
// \param rhs Vector to be copied.
// \return Reference to the assigned vector.
//
// Explicit definition of a copy assignment operator for performance reasons.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::operator=( const StaticVector& rhs )
{
   v_ = rhs.v_;

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Assignment operator for different StaticVector instances.
//
// \param rhs Vector to be copied.
// \return Reference to the assigned vector.
*/
template< typename Type      // Data type of the vector
        , size_t N           // Number of elements
        , bool TF            // Transpose flag
        , AlignmentFlag AF   // Alignment flag
        , PaddingFlag PF >   // Padding flag
template< typename Other     // Data type of the foreign vector
        , AlignmentFlag AF2  // Alignment flag of the foreign vector
        , PaddingFlag PF2 >  // Padding flag of the foreign vector
inline StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::operator=( const StaticVector<Other,N,TF,AF2,PF2>& rhs )
{
   using blaze::assign;

   assign( *this, ~rhs );

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Assignment operator for different vectors.
//
// \param rhs Vector to be copied.
// \return Reference to the assigned vector.
// \exception std::invalid_argument Invalid assignment to static vector.
//
// This constructor initializes the vector as a copy of the given vector. In case the
// size of the given vector is not N, a \a std::invalid_argument exception is thrown.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side vector
inline StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::operator=( const Vector<VT,TF>& rhs )
{
   using blaze::assign;

   if( (~rhs).size() != N ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid assignment to static vector" );
   }

   if( (~rhs).canAlias( this ) ) {
      StaticVector tmp( ~rhs );
      swap( tmp );
   }
   else {
      if( IsSparseVector_v<VT> )
         reset();
      assign( *this, ~rhs );
   }

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Addition assignment operator for the addition of a vector (\f$ \vec{a}+=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be added to the vector.
// \return Reference to the vector.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side vector
inline StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::operator+=( const Vector<VT,TF>& rhs )
{
   using blaze::addAssign;

   if( (~rhs).size() != N ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   if( (~rhs).canAlias( this ) ) {
      StaticVector tmp( ~rhs );
      addAssign( *this, tmp );
   }
   else {
      addAssign( *this, ~rhs );
   }

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Subtraction assignment operator for the subtraction of a vector (\f$ \vec{a}-=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be subtracted from the vector.
// \return Reference to the vector.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side vector
inline StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::operator-=( const Vector<VT,TF>& rhs )
{
   using blaze::subAssign;

   if( (~rhs).size() != N ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   if( (~rhs).canAlias( this ) ) {
      StaticVector tmp( ~rhs );
      subAssign( *this, tmp );
   }
   else {
      subAssign( *this, ~rhs );
   }

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Multiplication assignment operator for the multiplication of a vector
//        (\f$ \vec{a}*=\vec{b} \f$).
//
// \param rhs The right-hand side vector to be multiplied with the vector.
// \return Reference to the vector.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side vector
inline StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::operator*=( const Vector<VT,TF>& rhs )
{
   using blaze::assign;
   using blaze::multAssign;

   if( (~rhs).size() != N ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   if( IsSparseVector_v<VT> || (~rhs).canAlias( this ) ) {
      const StaticVector tmp( *this * (~rhs) );
      assign( *this, tmp );
   }
   else {
      multAssign( *this, ~rhs );
   }

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Division assignment operator for the division of a dense vector (\f$ \vec{a}/=\vec{b} \f$).
//
// \param rhs The right-hand side dense vector divisor.
// \return Reference to the vector.
// \exception std::invalid_argument Vector sizes do not match.
//
// In case the current sizes of the two vectors don't match, a \a std::invalid_argument exception
// is thrown.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side vector
inline StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::operator/=( const DenseVector<VT,TF>& rhs )
{
   using blaze::assign;
   using blaze::divAssign;

   if( (~rhs).size() != N ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Vector sizes do not match" );
   }

   if( (~rhs).canAlias( this ) ) {
      const StaticVector tmp( *this / (~rhs) );
      assign( *this, tmp );
   }
   else {
      divAssign( *this, ~rhs );
   }

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );

   return *this;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Cross product assignment operator for the multiplication of a vector
//        (\f$ \vec{a}\times=\vec{b} \f$).
//
// \param rhs The right-hand side vector for the cross product.
// \return Reference to the vector.
// \exception std::invalid_argument Invalid vector size for cross product.
//
// In case the current size of any of the two vectors is not equal to 3, a \a std::invalid_argument
// exception is thrown.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side vector
inline StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::operator%=( const Vector<VT,TF>& rhs )
{
   using blaze::assign;

   BLAZE_CONSTRAINT_MUST_BE_VECTOR_WITH_TRANSPOSE_FLAG( ResultType_t<VT>, TF );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( ResultType_t<VT> );

   using CrossType = CrossTrait_t< This, ResultType_t<VT> >;

   BLAZE_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE( CrossType );
   BLAZE_CONSTRAINT_MUST_BE_VECTOR_WITH_TRANSPOSE_FLAG( CrossType, TF );
   BLAZE_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION( CrossType );

   if( N != 3UL || (~rhs).size() != 3UL ) {
      BLAZE_THROW_INVALID_ARGUMENT( "Invalid vector size for cross product" );
   }

   const CrossType tmp( *this % (~rhs) );
   assign( *this, tmp );

   BLAZE_INTERNAL_ASSERT( isIntact(), "Invariant violation detected" );

   return *this;
}
//*************************************************************************************************




//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the current size/dimension of the vector.
//
// \return The size of the vector.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr size_t StaticVector<Type,N,TF,AF,PF>::size() noexcept
{
   return N;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns the minimum capacity of the vector.
//
// \return The minimum capacity of the vector.
//
// This function returns the minimum capacity of the vector, which corresponds to the current
// size plus padding.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr size_t StaticVector<Type,N,TF,AF,PF>::spacing() noexcept
{
   return NN;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns the maximum capacity of the vector.
//
// \return The maximum capacity of the vector.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr size_t StaticVector<Type,N,TF,AF,PF>::capacity() noexcept
{
   return NN;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns the number of non-zero elements in the vector.
//
// \return The number of non-zero elements in the vector.
//
// Note that the number of non-zero elements is always less than or equal to the current size
// of the vector.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline size_t StaticVector<Type,N,TF,AF,PF>::nonZeros() const
{
   size_t nonzeros( 0 );

   for( size_t i=0UL; i<N; ++i ) {
      if( !isDefault( v_[i] ) )
         ++nonzeros;
   }

   return nonzeros;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Reset to the default initial values.
//
// \return void
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr void StaticVector<Type,N,TF,AF,PF>::reset()
{
   using blaze::clear;
   for( size_t i=0UL; i<N; ++i )
      clear( v_[i] );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Swapping the contents of two static vectors.
//
// \param v The vector to be swapped.
// \return void
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline void StaticVector<Type,N,TF,AF,PF>::swap( StaticVector& v ) noexcept
{
   using std::swap;

   for( size_t i=0UL; i<N; ++i )
      swap( v_[i], v.v_[i] );
}
//*************************************************************************************************




//=================================================================================================
//
//  NUMERIC FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Scaling of the vector by the scalar value \a scalar (\f$ \vec{a}*=s \f$).
//
// \param scalar The scalar value for the vector scaling.
// \return Reference to the vector.
//
// This function scales the vector by applying the given scalar value \a scalar to each element
// of the vector. For built-in and \c complex data types it has the same effect as using the
// multiplication assignment operator:

   \code
   blaze::StaticVector<int,3> a;
   // ... Initialization
   a *= 4;        // Scaling of the vector
   a.scale( 4 );  // Same effect as above
   \endcode
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename Other >  // Data type of the scalar value
inline StaticVector<Type,N,TF,AF,PF>&
   StaticVector<Type,N,TF,AF,PF>::scale( const Other& scalar )
{
   for( size_t i=0; i<N; ++i )
      v_[i] *= scalar;
   return *this;
}
//*************************************************************************************************




//=================================================================================================
//
//  MEMORY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Class specific implementation of operator new.
//
// \param size The total number of bytes to be allocated.
// \return Pointer to the newly allocated memory.
// \exception std::bad_alloc Allocation failed.
//
// This class-specific implementation of operator new provides the functionality to allocate
// dynamic memory based on the alignment restrictions of the StaticVector class template.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline void* StaticVector<Type,N,TF,AF,PF>::operator new( std::size_t size )
{
   MAYBE_UNUSED( size );

   BLAZE_INTERNAL_ASSERT( size == sizeof( StaticVector ), "Invalid number of bytes detected" );

   return allocate<StaticVector>( 1UL );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Class specific implementation of operator new[].
//
// \param size The total number of bytes to be allocated.
// \return Pointer to the newly allocated memory.
// \exception std::bad_alloc Allocation failed.
//
// This class-specific implementation of operator new provides the functionality to allocate
// dynamic memory based on the alignment restrictions of the StaticVector class template.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline void* StaticVector<Type,N,TF,AF,PF>::operator new[]( std::size_t size )
{
   BLAZE_INTERNAL_ASSERT( size >= sizeof( StaticVector )       , "Invalid number of bytes detected" );
   BLAZE_INTERNAL_ASSERT( size %  sizeof( StaticVector ) == 0UL, "Invalid number of bytes detected" );

   return allocate<StaticVector>( size/sizeof(StaticVector) );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Class specific implementation of the no-throw operator new.
//
// \param size The total number of bytes to be allocated.
// \return Pointer to the newly allocated memory.
// \exception std::bad_alloc Allocation failed.
//
// This class-specific implementation of operator new provides the functionality to allocate
// dynamic memory based on the alignment restrictions of the StaticVector class template.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline void* StaticVector<Type,N,TF,AF,PF>::operator new( std::size_t size, const std::nothrow_t& )
{
   MAYBE_UNUSED( size );

   BLAZE_INTERNAL_ASSERT( size == sizeof( StaticVector ), "Invalid number of bytes detected" );

   return allocate<StaticVector>( 1UL );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Class specific implementation of the no-throw operator new[].
//
// \param size The total number of bytes to be allocated.
// \return Pointer to the newly allocated memory.
// \exception std::bad_alloc Allocation failed.
//
// This class-specific implementation of operator new provides the functionality to allocate
// dynamic memory based on the alignment restrictions of the StaticVector class template.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline void* StaticVector<Type,N,TF,AF,PF>::operator new[]( std::size_t size, const std::nothrow_t& )
{
   BLAZE_INTERNAL_ASSERT( size >= sizeof( StaticVector )       , "Invalid number of bytes detected" );
   BLAZE_INTERNAL_ASSERT( size %  sizeof( StaticVector ) == 0UL, "Invalid number of bytes detected" );

   return allocate<StaticVector>( size/sizeof(StaticVector) );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Class specific implementation of operator delete.
//
// \param ptr The memory to be deallocated.
// \return void
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline void StaticVector<Type,N,TF,AF,PF>::operator delete( void* ptr )
{
   deallocate( static_cast<StaticVector*>( ptr ) );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Class specific implementation of operator delete[].
//
// \param ptr The memory to be deallocated.
// \return void
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline void StaticVector<Type,N,TF,AF,PF>::operator delete[]( void* ptr )
{
   deallocate( static_cast<StaticVector*>( ptr ) );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Class specific implementation of no-throw operator delete.
//
// \param ptr The memory to be deallocated.
// \return void
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline void StaticVector<Type,N,TF,AF,PF>::operator delete( void* ptr, const std::nothrow_t& )
{
   deallocate( static_cast<StaticVector*>( ptr ) );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Class specific implementation of no-throw operator delete[].
//
// \param ptr The memory to be deallocated.
// \return void
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline void StaticVector<Type,N,TF,AF,PF>::operator delete[]( void* ptr, const std::nothrow_t& )
{
   deallocate( static_cast<StaticVector*>( ptr ) );
}
//*************************************************************************************************




//=================================================================================================
//
//  DEBUGGING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns whether the invariants of the static vector are intact.
//
// \return \a true in case the static vector's invariants are intact, \a false otherwise.
//
// This function checks whether the invariants of the static vector are intact, i.e. if its
// state is valid. In case the invariants are intact, the function returns \a true, else it
// will return \a false.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr bool StaticVector<Type,N,TF,AF,PF>::isIntact() const noexcept
{
   if( IsNumeric_v<Type> ) {
      for( size_t i=N; i<NN; ++i ) {
         if( v_[i] != Type() )
            return false;
      }
   }

   return true;
}
//*************************************************************************************************




//=================================================================================================
//
//  EXPRESSION TEMPLATE EVALUATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns whether the vector can alias with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this vector, \a false if not.
//
// This function returns whether the given address can alias with the vector. In contrast
// to the isAliased() function this function is allowed to use compile time expressions
// to optimize the evaluation.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename Other >  // Data type of the foreign expression
inline bool StaticVector<Type,N,TF,AF,PF>::canAlias( const Other* alias ) const noexcept
{
   return static_cast<const void*>( this ) == static_cast<const void*>( alias );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the vector is aliased with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this vector, \a false if not.
//
// This function returns whether the given address is aliased with the vector. In contrast
// to the canAlias() function this function is not allowed to use compile time expressions
// to optimize the evaluation.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename Other >  // Data type of the foreign expression
inline bool StaticVector<Type,N,TF,AF,PF>::isAliased( const Other* alias ) const noexcept
{
   return static_cast<const void*>( this ) == static_cast<const void*>( alias );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the vector is properly aligned in memory.
//
// \return \a true in case the vector is aligned, \a false if not.
//
// This function returns whether the vector is guaranteed to be properly aligned in memory, i.e.
// whether the beginning and the end of the vector are guaranteed to conform to the alignment
// restrictions of the element type \a Type.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr bool StaticVector<Type,N,TF,AF,PF>::isAligned() noexcept
{
   return AF == aligned;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Load of a SIMD element of the vector.
//
// \param index Access index. The index must be smaller than the number of vector elements.
// \return The loaded SIMD element.
//
// This function performs a load of a specific SIMD element of the dense vector. The index
// must be smaller than the number of vector elements and it must be a multiple of the number
// of values inside the SIMD element. This function must \b NOT be called explicitly! It is
// used internally for the performance optimized evaluation of expression templates. Calling
// this function explicitly might result in erroneous results and/or in compilation errors.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
BLAZE_ALWAYS_INLINE typename StaticVector<Type,N,TF,AF,PF>::SIMDType
   StaticVector<Type,N,TF,AF,PF>::load( size_t index ) const noexcept
{
   if( AF == aligned )
      return loada( index );
   else
      return loadu( index );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Aligned load of a SIMD element of the vector.
//
// \param index Access index. The index must be smaller than the number of vector elements.
// \return The loaded SIMD element.
//
// This function performs an aligned load of a specific SIMD element of the dense vector. The
// index must be smaller than the number of vector elements and it must be a multiple of the
// number of values inside the SIMD element. This function must \b NOT be called explicitly!
// It is used internally for the performance optimized evaluation of expression templates.
// Calling this function explicitly might result in erroneous results and/or in compilation
// errors.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
BLAZE_ALWAYS_INLINE typename StaticVector<Type,N,TF,AF,PF>::SIMDType
   StaticVector<Type,N,TF,AF,PF>::loada( size_t index ) const noexcept
{
   using blaze::loada;

   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( Type );

   BLAZE_INTERNAL_ASSERT( index < N, "Invalid vector access index" );
   BLAZE_INTERNAL_ASSERT( index + SIMDSIZE <= NN, "Invalid vector access index" );
   BLAZE_INTERNAL_ASSERT( index % SIMDSIZE == 0UL, "Invalid vector access index" );
   BLAZE_INTERNAL_ASSERT( checkAlignment( &v_[index] ), "Invalid alignment detected" );

   return loada( &v_[index] );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Unaligned load of a SIMD element of the vector.
//
// \param index Access index. The index must be smaller than the number of vector elements.
// \return The loaded SIMD element.
//
// This function performs an unaligned load of a specific SIMD element of the dense vector. The
// index must be smaller than the number of vector elements and it must be a multiple of the
// number of values inside the SIMD element. This function must \b NOT be called explicitly!
// It is used internally for the performance optimized evaluation of expression templates.
// Calling this function explicitly might result in erroneous results and/or in compilation
// errors.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
BLAZE_ALWAYS_INLINE typename StaticVector<Type,N,TF,AF,PF>::SIMDType
   StaticVector<Type,N,TF,AF,PF>::loadu( size_t index ) const noexcept
{
   using blaze::loadu;

   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( Type );

   BLAZE_INTERNAL_ASSERT( index < N, "Invalid vector access index" );
   BLAZE_INTERNAL_ASSERT( index + SIMDSIZE <= NN, "Invalid vector access index" );

   return loadu( &v_[index] );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Store of a SIMD element of the vector.
//
// \param index Access index. The index must be smaller than the number of vector elements.
// \param value The SIMD element to be stored.
// \return void
//
// This function performs a store of a specific SIMD element of the dense vector. The index
// must be smaller than the number of vector elements and it must be a multiple of the number
// of values inside the SIMD element. This function must \b NOT be called explicitly! It is
// used internally for the performance optimized evaluation of expression templates. Calling
// this function explicitly might result in erroneous results and/or in compilation errors.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
BLAZE_ALWAYS_INLINE void
   StaticVector<Type,N,TF,AF,PF>::store( size_t index, const SIMDType& value ) noexcept
{
   if( AF == aligned )
      storea( index, value );
   else
      storeu( index, value );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Aligned store of a SIMD element of the vector.
//
// \param index Access index. The index must be smaller than the number of vector elements.
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an aligned store of a specific SIMD element of the dense vector. The
// index must be smaller than the number of vector elements and it must be a multiple of the
// number of values inside the SIMD element. This function must \b NOT be called explicitly! It
// is used internally for the performance optimized evaluation of expression templates. Calling
// this function explicitly might result in erroneous results and/or in compilation errors.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
BLAZE_ALWAYS_INLINE void
   StaticVector<Type,N,TF,AF,PF>::storea( size_t index, const SIMDType& value ) noexcept
{
   using blaze::storea;

   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( Type );

   BLAZE_INTERNAL_ASSERT( index < N, "Invalid vector access index" );
   BLAZE_INTERNAL_ASSERT( index + SIMDSIZE <= NN , "Invalid vector access index" );
   BLAZE_INTERNAL_ASSERT( index % SIMDSIZE == 0UL, "Invalid vector access index" );
   BLAZE_INTERNAL_ASSERT( checkAlignment( &v_[index] ), "Invalid alignment detected" );

   storea( &v_[index], value );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Unaligned store of a SIMD element of the vector.
//
// \param index Access index. The index must be smaller than the number of vector elements.
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an unaligned store of a specific SIMD element of the dense vector.
// The index must be smaller than the number of vector elements and it must be a multiple of the
// number of values inside the SIMD element. This function must \b NOT be called explicitly! It
// is used internally for the performance optimized evaluation of expression templates. Calling
// this function explicitly might result in erroneous results and/or in compilation errors.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
BLAZE_ALWAYS_INLINE void
   StaticVector<Type,N,TF,AF,PF>::storeu( size_t index, const SIMDType& value ) noexcept
{
   using blaze::storeu;

   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( Type );

   BLAZE_INTERNAL_ASSERT( index < N, "Invalid vector access index" );
   BLAZE_INTERNAL_ASSERT( index + SIMDSIZE <= NN, "Invalid vector access index" );

   storeu( &v_[index], value );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Aligned, non-temporal store of a SIMD element of the vector.
//
// \param index Access index. The index must be smaller than the number of vector elements.
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an aligned, non-temporal store of a specific SIMD element of the
// dense vector. The index must be smaller than the number of vector elements and it must be
// a multiple of the number of values inside the SIMD element. This function must \b NOT be
// called explicitly! It is used internally for the performance optimized evaluation of
// expression templates. Calling this function explicitly might result in erroneous results
// and/or in compilation errors.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
BLAZE_ALWAYS_INLINE void
   StaticVector<Type,N,TF,AF,PF>::stream( size_t index, const SIMDType& value ) noexcept
{
   using blaze::stream;

   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( Type );

   BLAZE_INTERNAL_ASSERT( index < N, "Invalid vector access index" );
   BLAZE_INTERNAL_ASSERT( index + SIMDSIZE <= NN, "Invalid vector access index" );
   BLAZE_INTERNAL_ASSERT( index % SIMDSIZE == 0UL, "Invalid vector access index" );
   BLAZE_INTERNAL_ASSERT( checkAlignment( &v_[index] ), "Invalid alignment detected" );

   stream( &v_[index], value );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side dense vector
inline auto StaticVector<Type,N,TF,AF,PF>::assign( const DenseVector<VT,TF>& rhs )
   -> DisableIf_t< VectorizedAssign_v<VT> >
{
   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   for( size_t i=0UL; i<N; ++i )
      v_[i] = (~rhs)[i];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief SIMD optimized implementation of the assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side dense vector
inline auto StaticVector<Type,N,TF,AF,PF>::assign( const DenseVector<VT,TF>& rhs )
   -> EnableIf_t< VectorizedAssign_v<VT> >
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( Type );

   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   constexpr bool remainder( PF == unpadded || !IsPadded_v<VT> );

   constexpr size_t ipos( remainder ? prevMultiple( N, SIMDSIZE ) : N );
   BLAZE_INTERNAL_ASSERT( ipos <= N, "Invalid end calculation" );

   size_t i( 0UL );

   for( ; i<ipos; i+=SIMDSIZE ) {
      store( i, (~rhs).load(i) );
   }
   for( ; remainder && i<N; ++i ) {
      v_[i] = (~rhs)[i];
   }
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side sparse vector
inline void StaticVector<Type,N,TF,AF,PF>::assign( const SparseVector<VT,TF>& rhs )
{
   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   for( auto element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      v_[element->index()] = element->value();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the addition assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side dense vector
inline auto StaticVector<Type,N,TF,AF,PF>::addAssign( const DenseVector<VT,TF>& rhs )
   -> DisableIf_t< VectorizedAddAssign_v<VT> >
{
   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   for( size_t i=0UL; i<N; ++i )
      v_[i] += (~rhs)[i];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief SIMD optimized implementation of the addition assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side dense vector
inline auto StaticVector<Type,N,TF,AF,PF>::addAssign( const DenseVector<VT,TF>& rhs )
   -> EnableIf_t< VectorizedAddAssign_v<VT> >
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( Type );

   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   constexpr bool remainder( PF == unpadded || !IsPadded_v<VT> );

   constexpr size_t ipos( remainder ? prevMultiple( N, SIMDSIZE ) : N );
   BLAZE_INTERNAL_ASSERT( ipos <= N, "Invalid end calculation" );

   size_t i( 0UL );

   for( ; i<ipos; i+=SIMDSIZE ) {
      store( i, load(i) + (~rhs).load(i) );
   }
   for( ; remainder && i<N; ++i ) {
      v_[i] += (~rhs)[i];
   }
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the addition assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side sparse vector
inline void StaticVector<Type,N,TF,AF,PF>::addAssign( const SparseVector<VT,TF>& rhs )
{
   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   for( auto element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      v_[element->index()] += element->value();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the subtraction assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side dense vector
inline auto StaticVector<Type,N,TF,AF,PF>::subAssign( const DenseVector<VT,TF>& rhs )
   -> DisableIf_t< VectorizedSubAssign_v<VT> >
{
   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   for( size_t i=0UL; i<N; ++i )
      v_[i] -= (~rhs)[i];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief SIMD optimized implementation of the subtraction assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side dense vector
inline auto StaticVector<Type,N,TF,AF,PF>::subAssign( const DenseVector<VT,TF>& rhs )
   -> EnableIf_t< VectorizedSubAssign_v<VT> >
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( Type );

   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   constexpr bool remainder( PF == unpadded || !IsPadded_v<VT> );

   constexpr size_t ipos( remainder ? prevMultiple( N, SIMDSIZE ) : N );
   BLAZE_INTERNAL_ASSERT( ipos <= N, "Invalid end calculation" );

   size_t i( 0UL );

   for( ; i<ipos; i+=SIMDSIZE ) {
      store( i, load(i) - (~rhs).load(i) );
   }
   for( ; remainder && i<N; ++i ) {
      v_[i] -= (~rhs)[i];
   }
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the subtraction assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side sparse vector
inline void StaticVector<Type,N,TF,AF,PF>::subAssign( const SparseVector<VT,TF>& rhs )
{
   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   for( auto element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      v_[element->index()] -= element->value();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the multiplication assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be multiplied.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side dense vector
inline auto StaticVector<Type,N,TF,AF,PF>::multAssign( const DenseVector<VT,TF>& rhs )
   -> DisableIf_t< VectorizedMultAssign_v<VT> >
{
   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   for( size_t i=0UL; i<N; ++i )
      v_[i] *= (~rhs)[i];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief SIMD optimized implementation of the multiplication assignment of a dense vector.
//
// \param rhs The right-hand side dense vector to be multiplied.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side dense vector
inline auto StaticVector<Type,N,TF,AF,PF>::multAssign( const DenseVector<VT,TF>& rhs )
   -> EnableIf_t< VectorizedMultAssign_v<VT> >
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( Type );

   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   constexpr bool remainder( PF == unpadded || !IsPadded_v<VT> );

   constexpr size_t ipos( remainder ? prevMultiple( N, SIMDSIZE ) : N );
   BLAZE_INTERNAL_ASSERT( ipos <= N, "Invalid end calculation" );

   size_t i( 0UL );

   for( ; i<ipos; i+=SIMDSIZE ) {
      store( i, load(i) * (~rhs).load(i) );
   }
   for( ; remainder && i<N; ++i ) {
      v_[i] *= (~rhs)[i];
   }
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the multiplication assignment of a sparse vector.
//
// \param rhs The right-hand side sparse vector to be multiplied.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side sparse vector
inline void StaticVector<Type,N,TF,AF,PF>::multAssign( const SparseVector<VT,TF>& rhs )
{
   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   const StaticVector tmp( serial( *this ) );

   reset();

   for( auto element=(~rhs).begin(); element!=(~rhs).end(); ++element )
      v_[element->index()] = tmp[element->index()] * element->value();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Default implementation of the division assignment of a dense vector.
//
// \param rhs The right-hand side dense vector divisor.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side dense vector
inline auto StaticVector<Type,N,TF,AF,PF>::divAssign( const DenseVector<VT,TF>& rhs )
   -> DisableIf_t< VectorizedDivAssign_v<VT> >
{
   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   for( size_t i=0UL; i<N; ++i )
      v_[i] /= (~rhs)[i];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief SIMD optimized implementation of the division assignment of a dense vector.
//
// \param rhs The right-hand side dense vector divisor.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
template< typename VT >     // Type of the right-hand side dense vector
inline auto StaticVector<Type,N,TF,AF,PF>::divAssign( const DenseVector<VT,TF>& rhs )
   -> EnableIf_t< VectorizedDivAssign_v<VT> >
{
   BLAZE_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE( Type );

   BLAZE_INTERNAL_ASSERT( (~rhs).size() == N, "Invalid vector sizes" );

   constexpr size_t ipos( prevMultiple( N, SIMDSIZE ) );
   BLAZE_INTERNAL_ASSERT( ipos <= N, "Invalid end calculation" );

   size_t i( 0UL );

   for( ; i<ipos; i+=SIMDSIZE ) {
      store( i, load(i) / (~rhs).load(i) );
   }
   for( ; i<N; ++i ) {
      v_[i] /= (~rhs)[i];
   }
}
//*************************************************************************************************








//=================================================================================================
//
//  STATICVECTOR OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\name StaticVector operators */
//@{
template< typename Type, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
constexpr void reset( StaticVector<Type,N,TF,AF,PF>& v );

template< typename Type, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
constexpr void clear( StaticVector<Type,N,TF,AF,PF>& v );

template< RelaxationFlag RF, typename Type, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
bool isDefault( const StaticVector<Type,N,TF,AF,PF>& v );

template< typename Type, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
constexpr bool isIntact( const StaticVector<Type,N,TF,AF,PF>& v ) noexcept;

template< typename Type, bool TF, AlignmentFlag AF, PaddingFlag PF >
const StaticVector<Type,2UL,TF,AF,PF> perp( const StaticVector<Type,2UL,TF,AF,PF>& v );

template< typename Type, bool TF, AlignmentFlag AF, PaddingFlag PF >
const StaticVector<Type,3UL,TF,AF,PF> perp( const StaticVector<Type,3UL,TF,AF,PF>& v );

template< typename Type, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
void swap( StaticVector<Type,N,TF,AF,PF>& a, StaticVector<Type,N,TF,AF,PF>& b ) noexcept;

template< size_t I, typename Type, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
constexpr Type& get( StaticVector<Type,N,TF,AF,PF>& v ) noexcept;

template< size_t I, typename Type, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
constexpr Type&& get( StaticVector<Type,N,TF,AF,PF>&& v ) noexcept;

template< size_t I, typename Type, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
constexpr const Type& get( const StaticVector<Type,N,TF,AF,PF>& v) noexcept;

template< size_t I, typename Type, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
constexpr const Type&& get( const StaticVector<Type,N,TF,AF,PF>&& v ) noexcept;
//@}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Resetting the given static vector.
// \ingroup static_vector
//
// \param v The vector to be resetted.
// \return void
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr void reset( StaticVector<Type,N,TF,AF,PF>& v )
{
   v.reset();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Clearing the given static vector.
// \ingroup static_vector
//
// \param v The vector to be cleared.
// \return void
//
// Clearing a static vector is equivalent to resetting it via the reset() function.
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr void clear( StaticVector<Type,N,TF,AF,PF>& v )
{
   v.reset();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the given static vector is in default state.
// \ingroup static_vector
//
// \param v The vector to be tested for its default state.
// \return \a true in case the given vector is component-wise zero, \a false otherwise.
//
// This function checks whether the static vector is in default state. For instance, in case
// the static vector is instantiated for a built-in integral or floating point data type, the
// function returns \a true in case all vector elements are 0 and \a false in case any vector
// element is not 0. Following example demonstrates the use of the \a isDefault function:

   \code
   blaze::StaticVector<double,3> a;
   // ... Initialization
   if( isDefault( a ) ) { ... }
   \endcode

// Optionally, it is possible to switch between strict semantics (blaze::strict) and relaxed
// semantics (blaze::relaxed):

   \code
   if( isDefault<relaxed>( a ) ) { ... }
   \endcode
*/
template< RelaxationFlag RF  // Relaxation flag
        , typename Type      // Data type of the vector
        , size_t N           // Number of elements
        , bool TF            // Transpose flag
        , AlignmentFlag AF   // Alignment flag
        , PaddingFlag PF >   // Padding flag
inline bool isDefault( const StaticVector<Type,N,TF,AF,PF>& v )
{
   for( size_t i=0UL; i<N; ++i )
      if( !isDefault<RF>( v[i] ) ) return false;
   return true;
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Returns whether the invariants of the given static vector are intact.
// \ingroup static_vector
//
// \param v The static vector to be tested.
// \return \a true in case the given vector's invariants are intact, \a false otherwise.
//
// This function checks whether the invariants of the static vector are intact, i.e. if its
// state is valid. In case the invariants are intact, the function returns \a true, else it
// will return \a false. The following example demonstrates the use of the \a isIntact()
// function:

   \code
   blaze::StaticVector<double,3> a;
   // ... Resizing and initialization
   if( isIntact( a ) ) { ... }
   \endcode
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr bool isIntact( const StaticVector<Type,N,TF,AF,PF>& v ) noexcept
{
   return v.isIntact();
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Unary perp dot product operator for the calculation of a perpendicular vector
//        (\f$ \vec{a}=\vec{b}^\perp \f$).
//
// \param v The vector to be rotated.
// \return The perpendicular vector.
//
// The "perp dot product" \f$ \vec{a}^\perp \cdot b \f$ for the vectors \f$ \vec{a} \f$ and
// \f$ \vec{b} \f$ is a modification of the two-dimensional dot product in which \f$ \vec{a} \f$
// is replaced by the perpendicular vector rotated 90 degrees to the left defined by Hill (1994).
*/
template< typename Type     // Data type of the vector
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline const StaticVector<Type,2UL,TF,AF,PF> perp( const StaticVector<Type,2UL,TF,AF,PF>& v )
{
   return StaticVector<Type,2UL,TF,AF,PF>( -v[1UL], v[0UL] );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Creates a perpendicular vector b which satisfies \f$ \vec{a} \cdot \vec{b} = 0 \f$.
//
// \param v The vector to be rotated.
// \return The perpendicular vector.
//
// \note The perpendicular vector may have any length!
*/
template< typename Type     // Data type of the vector
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline const StaticVector<Type,3UL,TF,AF,PF> perp( const StaticVector<Type,3UL,TF,AF,PF>& v )
{
   if( v[0] != Type() || v[1] != Type() )
      return StaticVector<Type,3UL,TF,AF,PF>( v[1UL], -v[0UL], Type() );
   else
      return StaticVector<Type,3UL,TF,AF,PF>( Type(), v[2UL], -v[1UL] );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Swapping the contents of two static vectors.
// \ingroup static_vector
//
// \param a The first vector to be swapped.
// \param b The second vector to be swapped.
// \return void
*/
template< typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
inline void swap( StaticVector<Type,N,TF,AF,PF>& a, StaticVector<Type,N,TF,AF,PF>& b ) noexcept
{
   a.swap( b );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Tuple-like index-based access the contents of a static vector.
// \ingroup static_vector
//
// \param v The vector to be accessed.
// \return Reference to the accessed value.
*/
template< size_t I          // Compile time access index
        , typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr Type& get( StaticVector<Type,N,TF,AF,PF>& v ) noexcept
{
   BLAZE_STATIC_ASSERT_MSG( I < N, "Invalid vector access index" );
   return v[I];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Tuple-like index-based access the contents of a temporary static vector.
// \ingroup static_vector
//
// \param v The vector to be accessed.
// \return Reference to the accessed value.
*/
template< size_t I          // Compile time access index
        , typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr Type&& get( StaticVector<Type,N,TF,AF,PF>&& v ) noexcept
{
   BLAZE_STATIC_ASSERT_MSG( I < N, "Invalid vector access index" );
   return std::move( v[I] );
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Tuple-like index-based access the contents of a constant static vector.
// \ingroup static_vector
//
// \param v The vector to be accessed.
// \return Reference to the accessed value.
*/
template< size_t I          // Compile time access index
        , typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr const Type& get( const StaticVector<Type,N,TF,AF,PF>& v) noexcept
{
   BLAZE_STATIC_ASSERT_MSG( I < N, "Invalid vector access index" );
   return v[I];
}
//*************************************************************************************************


//*************************************************************************************************
/*!\brief Tuple-like index-based access the contents of a constant temporary static vector.
// \ingroup static_vector
//
// \param v The vector to be accessed.
// \return Reference to the accessed value.
*/
template< size_t I          // Compile time access index
        , typename Type     // Data type of the vector
        , size_t N          // Number of elements
        , bool TF           // Transpose flag
        , AlignmentFlag AF  // Alignment flag
        , PaddingFlag PF >  // Padding flag
constexpr const Type&& get( const StaticVector<Type,N,TF,AF,PF>&& v ) noexcept
{
   BLAZE_STATIC_ASSERT_MSG( I < N, "Invalid vector access index" );
   return std::move( v[I] );
}
//*************************************************************************************************




//=================================================================================================
//
//  SIZE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
struct Size< StaticVector<T,N,TF,AF,PF>, 0UL >
   : public Ptrdiff_t<N>
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  MAXSIZE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
struct MaxSize< StaticVector<T,N,TF,AF,PF>, 0UL >
   : public Ptrdiff_t<N>
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  HASCONSTDATAACCESS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
struct HasConstDataAccess< StaticVector<T,N,TF,AF,PF> >
   : public TrueType
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  HASMUTABLEDATAACCESS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
struct HasMutableDataAccess< StaticVector<T,N,TF,AF,PF> >
   : public TrueType
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISSTATIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
struct IsStatic< StaticVector<T,N,TF,AF,PF> >
   : public TrueType
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISALIGNED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
struct IsAligned< StaticVector<T,N,TF,AF,PF> >
   : public BoolConstant< AF == aligned >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISCONTIGUOUS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
struct IsContiguous< StaticVector<T,N,TF,AF,PF> >
   : public TrueType
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ISPADDED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF >
struct IsPadded< StaticVector<T,N,TF,AF,PF> >
   : public BoolConstant< PF == padded >
{};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ADDTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, typename T2 >
struct AddTraitEval2< T1, T2
                    , EnableIf_t< IsVector_v<T1> &&
                                  IsVector_v<T2> &&
                                  ( Size_v<T1,0UL> != DefaultSize_v ||
                                    Size_v<T2,0UL> != DefaultSize_v ) > >
{
   using ET1 = ElementType_t<T1>;
   using ET2 = ElementType_t<T2>;

   static constexpr size_t N = max( Size_v<T1,0UL>, Size_v<T2,0UL> );

   using Type = StaticVector< AddTrait_t<ET1,ET2>, N, TransposeFlag_v<T1> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SUBTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, typename T2 >
struct SubTraitEval2< T1, T2
                    , EnableIf_t< IsVector_v<T1> &&
                                  IsVector_v<T2> &&
                                  ( Size_v<T1,0UL> != DefaultSize_v ||
                                    Size_v<T2,0UL> != DefaultSize_v ) > >
{
   using ET1 = ElementType_t<T1>;
   using ET2 = ElementType_t<T2>;

   static constexpr size_t N = max( Size_v<T1,0UL>, Size_v<T2,0UL> );

   using Type = StaticVector< SubTrait_t<ET1,ET2>, N, TransposeFlag_v<T1> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  MULTTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, typename T2 >
struct MultTraitEval2< T1, T2
                     , EnableIf_t< IsVector_v<T1> &&
                                   IsNumeric_v<T2> &&
                                   ( Size_v<T1,0UL> != DefaultSize_v ) > >
{
   using ET1 = ElementType_t<T1>;

   static constexpr size_t N = Size_v<T1,0UL>;

   using Type = StaticVector< MultTrait_t<ET1,T2>, N, TransposeFlag_v<T1> >;
};

template< typename T1, typename T2 >
struct MultTraitEval2< T1, T2
                     , EnableIf_t< IsNumeric_v<T1> &&
                                   IsVector_v<T2> &&
                                   ( Size_v<T2,0UL> != DefaultSize_v ) > >
{
   using ET2 = ElementType_t<T2>;

   static constexpr size_t N = Size_v<T2,0UL>;

   using Type = StaticVector< MultTrait_t<T1,ET2>, N, TransposeFlag_v<T2> >;
};

template< typename T1, typename T2 >
struct MultTraitEval2< T1, T2
                     , EnableIf_t< ( ( IsRowVector_v<T1> && IsRowVector_v<T2> ) ||
                                     ( IsColumnVector_v<T1> && IsColumnVector_v<T2> ) ) &&
                                   IsDenseVector_v<T1> &&
                                   IsDenseVector_v<T2> &&
                                   ( Size_v<T1,0UL> != DefaultSize_v || Size_v<T2,0UL> != DefaultSize_v ) > >
{
   using ET1 = ElementType_t<T1>;
   using ET2 = ElementType_t<T2>;

   static constexpr size_t N = max( Size_v<T1,0UL>, Size_v<T2,0UL> );

   using Type = StaticVector< MultTrait_t<ET1,ET2>, N, TransposeFlag_v<T1> >;
};

template< typename T1, typename T2 >
struct MultTraitEval2< T1, T2
                     , EnableIf_t< IsMatrix_v<T1> &&
                                   IsColumnVector_v<T2> &&
                                   ( Size_v<T1,0UL> != DefaultSize_v ||
                                     ( IsSquare_v<T1> && Size_v<T2,0UL> != DefaultSize_v ) ) > >
{
   using ET1 = ElementType_t<T1>;
   using ET2 = ElementType_t<T2>;

   static constexpr size_t N = ( Size_v<T1,0UL> != DefaultSize_v ? Size_v<T1,0UL> : Size_v<T2,0UL> );

   using Type = StaticVector< MultTrait_t<ET1,ET2>, N, false >;
};

template< typename T1, typename T2 >
struct MultTraitEval2< T1, T2
                     , EnableIf_t< IsRowVector_v<T1> &&
                                   IsMatrix_v<T2> &&
                                   ( Size_v<T2,1UL> != DefaultSize_v ||
                                     ( IsSquare_v<T2> && Size_v<T1,0UL> != DefaultSize_v ) ) > >
{
   using ET1 = ElementType_t<T1>;
   using ET2 = ElementType_t<T2>;

   static constexpr size_t N = ( Size_v<T2,1UL> != DefaultSize_v ? Size_v<T2,1UL> : Size_v<T1,0UL> );

   using Type = StaticVector< MultTrait_t<ET1,ET2>, N, true >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  KRONTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, typename T2 >
struct KronTraitEval2< T1, T2
                     , EnableIf_t< IsDenseVector_v<T1> &&
                                   IsDenseVector_v<T2> &&
                                   ( Size_v<T1,0UL> != DefaultSize_v ) &&
                                   ( Size_v<T2,0UL> != DefaultSize_v ) > >
{
   using ET1 = ElementType_t<T1>;
   using ET2 = ElementType_t<T2>;

   static constexpr size_t N = Size_v<T1,0UL> * Size_v<T2,0UL>;

   using Type = StaticVector< MultTrait_t<ET1,ET2>, N, TransposeFlag_v<T2> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  DIVTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, typename T2 >
struct DivTraitEval2< T1, T2
                    , EnableIf_t< IsVector_v<T1> &&
                                  IsNumeric_v<T2> &&
                                  ( Size_v<T1,0UL> != DefaultSize_v ) > >
{
   using ET1 = ElementType_t<T1>;

   static constexpr size_t N = Size_v<T1,0UL>;

   using Type = StaticVector< DivTrait_t<ET1,T2>, N, TransposeFlag_v<T1> >;
};

template< typename T1, typename T2 >
struct DivTraitEval2< T1, T2
                    , EnableIf_t< IsDenseVector_v<T1> &&
                                  IsDenseVector_v<T2> &&
                                  ( Size_v<T1,0UL> != DefaultSize_v ||
                                    Size_v<T2,0UL> != DefaultSize_v ) > >
{
   using ET1 = ElementType_t<T1>;
   using ET2 = ElementType_t<T2>;

   static constexpr size_t N = max( Size_v<T1,0UL>, Size_v<T2,0UL> );

   using Type = StaticVector< DivTrait_t<ET1,ET2>, N, TransposeFlag_v<T1> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  CROSSTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, typename T2 >
struct CrossTraitEval2< T1, T2
                      , EnableIf_t< IsVector_v<T1> && IsVector_v<T2> > >
{
   using Tmp = MultTrait_t< ElementType_t<T1>, ElementType_t<T2> >;

   using Type = StaticVector< SubTrait_t<Tmp,Tmp>, 3UL, TransposeFlag_v<T1> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  MAPTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T, typename OP >
struct UnaryMapTraitEval2< T, OP
                         , EnableIf_t< IsVector_v<T> &&
                                       Size_v<T,0UL> != DefaultSize_v > >
{
   using ET = ElementType_t<T>;

   using Type = StaticVector< MapTrait_t<ET,OP>, Size_v<T,0UL>, TransposeFlag_v<T> >;
};
/*! \endcond */
//*************************************************************************************************


//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, typename T2, typename OP >
struct BinaryMapTraitEval2< T1, T2, OP
                          , EnableIf_t< ( ( IsRowVector_v<T1> && IsRowVector_v<T2> ) ||
                                          ( IsColumnVector_v<T1> && IsColumnVector_v<T2> ) ) &&
                                        ( Size_v<T1,0UL> != DefaultSize_v ||
                                          Size_v<T2,0UL> != DefaultSize_v ) > >
{
   using ET1 = ElementType_t<T1>;
   using ET2 = ElementType_t<T2>;

   static constexpr size_t N = max( Size_v<T1,0UL>, Size_v<T2,0UL> );

   using Type = StaticVector< MapTrait_t<ET1,ET2,OP>, N, TransposeFlag_v<T1> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  REDUCETRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T, typename OP, ReductionFlag RF >
struct PartialReduceTraitEval2< T, OP, RF
                              , EnableIf_t< IsMatrix_v<T> &&
                                            Size_v<T,0UL> != DefaultSize_v &&
                                            Size_v<T,1UL> != DefaultSize_v > >
{
   using ET = ElementType_t<T>;
   using RT = decltype( std::declval<OP>()( std::declval<ET>(), std::declval<ET>() ) );

   static constexpr bool TF = ( RF == columnwise );

   static constexpr size_t N = Size_v< T, TF ? 1UL : 0UL >;

   using Type = StaticVector<RT,N,TF>;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SOLVETRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, typename T2 >
struct SolveTraitEval2< T1, T2
                      , EnableIf_t< IsDenseMatrix_v<T1> &&
                                    IsDenseVector_v<T2> &&
                                    ( ( Size_v<T1,0UL> != DefaultSize_v ) ||
                                      ( Size_v<T1,1UL> != DefaultSize_v ) ||
                                      ( Size_v<T2,0UL> != DefaultSize_v ) ) > >
{
   static constexpr size_t N = max( Size_v<T1,0UL>, Size_v<T1,1UL>, Size_v<T2,0UL> );

   using Type = StaticVector< ElementType_t<T2>, N, TransposeFlag_v<T2> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  HIGHTYPE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF, typename T2 >
struct HighType< StaticVector<T1,N,TF,AF,PF>, StaticVector<T2,N,TF,AF,PF> >
{
   using Type = StaticVector< typename HighType<T1,T2>::Type, N, TF, AF, PF >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  LOWTYPE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename T1, size_t N, bool TF, AlignmentFlag AF, PaddingFlag PF, typename T2 >
struct LowType< StaticVector<T1,N,TF,AF,PF>, StaticVector<T2,N,TF,AF,PF> >
{
   using Type = StaticVector< typename LowType<T1,T2>::Type, N, TF, AF, PF >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  SUBVECTORTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename VT, size_t I, size_t N >
struct SubvectorTraitEval2< VT, I, N
                          , EnableIf_t< I != inf && N != inf &&
                                        IsDenseVector_v<VT> > >
{
   using Type = StaticVector< RemoveConst_t< ElementType_t<VT> >, N, TransposeFlag_v<VT> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ELEMENTSTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename VT, size_t N >
struct ElementsTraitEval2< VT, N
                         , EnableIf_t< N != 0UL &&
                                       IsDenseVector_v<VT> > >
{
   using Type = StaticVector< RemoveConst_t< ElementType_t<VT> >, N, TransposeFlag_v<VT> >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  ROWTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, size_t I >
struct RowTraitEval2< MT, I
                    , EnableIf_t< IsDenseMatrix_v<MT> &&
                                  Size_v<MT,1UL> != DefaultSize_v > >
{
   using Type = StaticVector< RemoveConst_t< ElementType_t<MT> >, Size_v<MT,1UL>, true >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  COLUMNTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, size_t I >
struct ColumnTraitEval2< MT, I
                       , EnableIf_t< IsDenseMatrix_v<MT> &&
                                     Size_v<MT,0UL> != DefaultSize_v > >
{
   using Type = StaticVector< RemoveConst_t< ElementType_t<MT> >, Size_v<MT,0UL>, false >;
};
/*! \endcond */
//*************************************************************************************************




//=================================================================================================
//
//  BANDTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
template< typename MT, ptrdiff_t I >
struct BandTraitEval2< MT, I
                     , EnableIf_t< IsDenseMatrix_v<MT> &&
                                   Size_v<MT,0UL> != DefaultSize_v &&
                                   Size_v<MT,1UL> != DefaultSize_v > >
{
   static constexpr size_t M   = Size_v<MT,0UL>;
   static constexpr size_t N   = Size_v<MT,1UL>;
   static constexpr size_t Min = min( M - ( I >= 0L ? 0UL : -I ), N - ( I >= 0L ? I : 0UL ) );

   using Type = StaticVector< RemoveConst_t< ElementType_t<MT> >, Min, defaultTransposeFlag >;
};
/*! \endcond */
//*************************************************************************************************

} // namespace blaze




//=================================================================================================
//
//  STD::TUPLE SUPPORT
//
//=================================================================================================

//*************************************************************************************************
/*! \cond BLAZE_INTERNAL */
namespace std
{
   template< typename Type, size_t N, bool TF, blaze::AlignmentFlag AF, blaze::PaddingFlag PF >
   class tuple_size< blaze::StaticVector<Type,N,TF,AF,PF> >
      : public integral_constant< size_t, N >
   {};

   template< size_t I, typename Type, size_t N, bool TF, blaze::AlignmentFlag AF, blaze::PaddingFlag PF >
   class tuple_element< I, blaze::StaticVector<Type,N,TF,AF,PF> >
   {
    public:
      using type = Type;
   };
}
/*! \endcond */
//*************************************************************************************************

#endif
