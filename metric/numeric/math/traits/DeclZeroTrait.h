// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_DECLZEROTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_DECLZEROTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/sparse/Forward.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/math/typetraits/Size.h>
#include <metric/numeric/math/typetraits/StorageOrder.h>
#include <metric/numeric/math/typetraits/TransposeFlag.h>
#include <metric/numeric/util/EnableIf.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename> struct DeclZeroTrait;
template <typename, typename = void> struct DeclZeroTraitEval;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> auto evalDeclZeroTrait(const volatile T &) -> DeclZeroTraitEval<T>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the DeclZeroTrait class.
// \ingroup math_traits
//
// \section declzerotrait_general General
//
// The DeclZeroTrait class template offers the possibility to select the resulting data type
// of a generic declzero() operation on the given type \a T. In case the given type \a T is a
// fitting data type, DeclZeroTrait defines the nested type \a Type, which represents the
// resulting data type of the declzero() operation. Otherwise there is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section declzerotrait_specializations Creating custom specializations
//
// Per default, DeclZeroTrait supports all vector and matrix types of the Metric numeric library (including
// views and adaptors). For all other data types it is possible to specialize the DeclZeroTrait
// template. The following example shows the according specialization for the SymmetricMatrix
// class template:

   \code
   template< typename MT, bool SO, bool DF, bool SF >
   struct DeclZeroTrait< SymmetricMatrix<MT,SO,DF,SF> >
   {
	  using Type = ZeroMatrix< ElementType_t<MT>, SO >;
   };
   \endcode

// \n \section declzerotrait_examples Examples
//
// The following example demonstrates the use of the DeclZeroTrait template, where depending on
// the given vector or matrix type the resulting type is selected:

   \code
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::SymmetricMatrix;
   using mtrc::numeric::DeclZeroTrait;
   using mtrc::numeric::rowVector;
   using mtrc::numeric::columnMajor;

   // Definition of the resulting type of a row-major dynamic matrix
   using VectorType    = DynamicVector<int,rowVector>;
   using DeclZeroType1 = typename DeclZeroTrait<VectorType>::Type;

   // Definition of the resulting type of a symmetric column-major static matrix
   using MatrixType    = SymmetricMatrix< StaticMatrix<int,3UL,3UL,columnMajor> >;
   using DeclZeroType2 = typename DeclZeroTrait<MatrixType>::Type;
   \endcode
*/
template <typename T> // Type of the vector or matrix
struct DeclZeroTrait : public decltype(evalDeclZeroTrait(std::declval<T &>())){};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the DeclZeroTrait type trait.
// \ingroup math_traits
//
// The DeclZeroTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the DeclZeroTrait class template. For instance, given the vector or matrix type
// \a T the following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::DeclZeroTrait<T>::Type;
   using Type2 = mtrc::numeric::DeclZeroTrait_t<T>;
   \endcode
*/
template <typename T> // Type of the matrix
using DeclZeroTrait_t = typename DeclZeroTrait<T>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the DeclZeroTrait type trait.
// \ingroup math_traits
*/
template <typename T // Type of the vector or matrix
		  ,
		  typename> // Restricting condition
struct DeclZeroTraitEval {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DeclZeroTraitEval class template for vector types.
// \ingroup math_traits
*/
template <typename T> // Type of the vector
struct DeclZeroTraitEval<T, EnableIf_t<IsVector_v<T>>> {
	using Type = ZeroVector<typename T::ElementType, TransposeFlag_v<T>>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DeclZeroTraitEval class template for matrix types.
// \ingroup math_traits
*/
template <typename T> // Type of the matrix
struct DeclZeroTraitEval<T, EnableIf_t<IsMatrix_v<T>>> {
	using Type = ZeroMatrix<typename T::ElementType, StorageOrder_v<T>>;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
