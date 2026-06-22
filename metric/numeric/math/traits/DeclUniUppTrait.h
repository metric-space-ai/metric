// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_DECLUNIUPPTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_DECLUNIUPPTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/adaptors/uniuppermatrix/BaseTemplate.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/Size.h>
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
template <typename> struct DeclUniUppTrait;
template <typename, typename = void> struct DeclUniUppTraitEval;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> auto evalDeclUniUppTrait(const volatile T &) -> DeclUniUppTraitEval<T>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the DeclUniUppTrait class.
// \ingroup math_traits
//
// \section decluniupptrait_general General
//
// The DeclUniUppTrait class template offers the possibility to select the resulting data type
// of a generic decluniupp() operation on the given type \a MT. In case the given type \a MT
// is a dense or sparse matrix type, DeclUniUppTrait defines the nested type \a Type, which
// represents the resulting data type of the decluniupp() operation. Otherwise there is no
// nested type \a Type. Note that \a const and \a volatile qualifiers and reference modifiers
// are generally ignored.
//
//
// \section decluniupptrait_specializations Creating custom specializations
//
// Per default, DeclUniUppTrait supports all matrix types of the Metric numeric library (including views
// and adaptors). For all other data types it is possible to specialize the DeclUniUppTrait
// template. The following example shows the according specialization for the SymmetricMatrix
// class template:

   \code
   template< typename MT, bool SO, bool DF, bool SF >
   struct DeclUniUppTrait< SymmetricMatrix<MT,SO,DF,SF> >
   {
	  using Type = IdentityMatrix<ElemenType_t<MT>,SO>;
   };
   \endcode

// \n \section decluniupptrait_examples Examples
//
// The following example demonstrates the use of the DeclUniUppTrait template, where depending on
// the given matrix type the resulting type is selected:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::SymmetricMatrix;
   using mtrc::numeric::DeclUniUppTrait;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   // Definition of the resulting type of a row-major dynamic matrix
   using MatrixType1     = DynamicMatrix<int,rowMajor>;
   using DeclUniUppType1 = typename DeclUniUppTrait<MatrixType1>::Type;

   // Definition of the resulting type of a symmetric column-major static matrix
   using MatrixType2     = SymmetricMatrix< StaticMatrix<int,3UL,3UL,columnMajor> >;
   using DeclUniUppType2 = typename DeclUniUppTrait<MatrixType2>::Type;
   \endcode
*/
template <typename MT> // Type of the matrix
struct DeclUniUppTrait : public decltype(evalDeclUniUppTrait(std::declval<MT &>())){};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the DeclUniUppTrait type trait.
// \ingroup math_traits
//
// The DeclUniUppTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the DeclUniUppTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::DeclUniUppTrait<MT>::Type;
   using Type2 = mtrc::numeric::DeclUniUppTrait_t<MT>;
   \endcode
*/
template <typename MT> // Type of the matrix
using DeclUniUppTrait_t = typename DeclUniUppTrait<MT>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the DeclUniUppTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  typename> // Restricting condition
struct DeclUniUppTraitEval {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DeclUniUppTraitEval class template for square matrix types.
// \ingroup math_traits
*/
template <typename MT> // Type of the matrix
struct DeclUniUppTraitEval<
	MT, EnableIf_t<IsMatrix_v<MT> && (Size_v<MT, 0UL> == DefaultSize_v || Size_v<MT, 1UL> == DefaultSize_v ||
									  Size_v<MT, 0UL> == Size_v<MT, 1UL>)>> {
	using Type = UniUpperMatrix<typename MT::ResultType>;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
