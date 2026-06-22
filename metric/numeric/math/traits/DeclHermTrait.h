// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_DECLHERMTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_DECLHERMTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/adaptors/hermitianmatrix/BaseTemplate.h>
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
template <typename> struct DeclHermTrait;
template <typename, typename = void> struct DeclHermTraitEval;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> auto evalDeclHermTrait(const volatile T &) -> DeclHermTraitEval<T>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the DeclHermTrait class.
// \ingroup math_traits
//
// \section declhermtrait_general General
//
// The DeclHermTrait class template offers the possibility to select the resulting data type
// of a generic declherm() operation on the given type \a MT. In case the given type \a MT is
// a dense or sparse matrix type, DeclHermTrait defines the nested type \a Type, which represents
// the resulting data type of the declherm() operation. Otherwise there is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section declhermtrait_specializations Creating custom specializations
//
// Per default, DeclHermTrait supports all matrix types of the Metric numeric library (including views and
// adaptors). For all other data types it is possible to specialize the DeclHermTrait template.
// The following example shows the according specialization for the LowerMatrix class template:

   \code
   template< typename MT, bool SO, bool DF >
   struct DeclHermTrait< LowerMatrix<MT,SO,DF> >
   {
	  using Type = HermitianMatrix<MT>;
   };
   \endcode

// \n \section declhermtrait_examples Examples
//
// The following example demonstrates the use of the DeclHermTrait template, where depending on
// the given matrix type the resulting type is selected:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::LowerMatrix;
   using mtrc::numeric::DeclHermTrait;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   // Definition of the resulting type of a row-major dynamic matrix
   using MatrixType1   = DynamicMatrix<int,rowMajor>;
   using DeclHermType1 = typename DeclHermTrait<MatrixType1>::Type;

   // Definition of the resulting type of a lower column-major static matrix
   using MatrixType2   = LowerMatrix< StaticMatrix<int,3UL,3UL,columnMajor> >;
   using DeclHermType2 = typename DeclHermTrait<MatrixType2>::Type;
   \endcode
*/
template <typename MT> // Type of the matrix
struct DeclHermTrait : public decltype(evalDeclHermTrait(std::declval<MT &>())){};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the DeclHermTrait type trait.
// \ingroup math_traits
//
// The DeclHermTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the DeclHermTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::DeclHermTrait<MT>::Type;
   using Type2 = mtrc::numeric::DeclHermTrait_t<MT>;
   \endcode
*/
template <typename MT> // Type of the matrix
using DeclHermTrait_t = typename DeclHermTrait<MT>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the DeclHermTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  typename> // Restricting condition
struct DeclHermTraitEval {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DeclHermTraitEval class template for square matrix types.
// \ingroup math_traits
*/
template <typename MT> // Type of the matrix
struct DeclHermTraitEval<
	MT, EnableIf_t<IsMatrix_v<MT> && (Size_v<MT, 0UL> == DefaultSize_v || Size_v<MT, 1UL> == DefaultSize_v ||
									  Size_v<MT, 0UL> == Size_v<MT, 1UL>)>> {
	using Type = HermitianMatrix<typename MT::ResultType>;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
