// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_DECLIDTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_DECLIDTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/sparse/Forward.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/Size.h>
#include <metric/numeric/math/typetraits/StorageOrder.h>
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
template <typename> struct DeclIdTrait;
template <typename, typename = void> struct DeclIdTraitEval;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> auto evalDeclIdTrait(const volatile T &) -> DeclIdTraitEval<T>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the DeclIdTrait class.
// \ingroup math_traits
//
// \section declidtrait_general General
//
// The DeclIdTrait class template offers the possibility to select the resulting data type
// of a generic declid() operation on the given type \a MT. In case the given type \a MT is
// a dense or sparse matrix type, DeclIdTrait defines the nested type \a Type, which represents
// the resulting data type of the declid() operation. Otherwise there is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section declidtrait_specializations Creating custom specializations
//
// Per default, DeclIdTrait supports all matrix types of the Metric numeric library (including views and
// adaptors). For all other data types it is possible to specialize the DeclIdTrait template. The
// following example shows the according specialization for the SymmetricMatrix class template:

   \code
   template< typename MT, bool SO, bool DF, bool SF >
   struct DeclIdTrait< SymmetricMatrix<MT,SO,DF,SF> >
   {
	  using Type = IdentityMatrix< ElementType_t<MT>, SO >;
   };
   \endcode

// \n \section declidtrait_examples Examples
//
// The following example demonstrates the use of the DeclIdTrait template, where depending on
// the given matrix type the resulting type is selected:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::SymmetricMatrix;
   using mtrc::numeric::DeclIdTrait;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   // Definition of the resulting type of a row-major dynamic matrix
   using MatrixType1 = DynamicMatrix<int,rowMajor>;
   using DeclIdType1 = typename DeclIdTrait<MatrixType1>::Type;

   // Definition of the resulting type of a symmetric column-major static matrix
   using MatrixType2 = SymmetricMatrix< StaticMatrix<int,3UL,3UL,columnMajor> >;
   using DeclIdType2 = typename DeclIdTrait<MatrixType2>::Type;
   \endcode
*/
template <typename MT> // Type of the matrix
struct DeclIdTrait : public decltype(evalDeclIdTrait(std::declval<MT &>())){};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the DeclIdTrait type trait.
// \ingroup math_traits
//
// The DeclIdTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the DeclIdTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::DeclIdTrait<MT>::Type;
   using Type2 = mtrc::numeric::DeclIdTrait_t<MT>;
   \endcode
*/
template <typename MT> // Type of the matrix
using DeclIdTrait_t = typename DeclIdTrait<MT>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the DeclIdTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  typename> // Restricting condition
struct DeclIdTraitEval {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DeclIdTraitEval class template for square matrix types.
// \ingroup math_traits
*/
template <typename MT> // Type of the matrix
struct DeclIdTraitEval<
	MT, EnableIf_t<IsMatrix_v<MT> && (Size_v<MT, 0UL> == DefaultSize_v || Size_v<MT, 1UL> == DefaultSize_v ||
									  Size_v<MT, 0UL> == Size_v<MT, 1UL>)>> {
	using Type = IdentityMatrix<typename MT::ElementType, StorageOrder_v<MT>>;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
