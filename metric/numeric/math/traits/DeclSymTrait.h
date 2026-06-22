// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_DECLSYMTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_DECLSYMTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/adaptors/symmetricmatrix/BaseTemplate.h>
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
template <typename> struct DeclSymTrait;
template <typename, typename = void> struct DeclSymTraitEval;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> auto evalDeclSymTrait(const volatile T &) -> DeclSymTraitEval<T>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the DeclSymTrait class.
// \ingroup math_traits
//
// \section declsymtrait_general General
//
// The DeclSymTrait class template offers the possibility to select the resulting data type
// of a generic declsym() operation on the given type \a MT. In case the given type \a MT is
// a dense or sparse matrix type, DeclSymTrait defines the nested type \a Type, which represents
// the resulting data type of the declsym() operation. Otherwise there is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section declsymtrait_specializations Creating custom specializations
//
// Per default, DeclSymTrait supports all matrix types of the Metric numeric library (including views and
// adaptors). For all other data types it is possible to specialize the DeclSymTrait template.
// The following example shows the according specialization for the LowerMatrix class template:

   \code
   template< typename MT, bool SO, bool DF >
   struct DeclSymTrait< LowerMatrix<MT,SO,DF> >
   {
	  using Type = DiagonalMatrix<MT>;
   };
   \endcode

// \n \section declsymtrait_examples Examples
//
// The following example demonstrates the use of the DeclSymTrait template, where depending on
// the given matrix type the resulting type is selected:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::LowerMatrix;
   using mtrc::numeric::DeclSymTrait;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   // Definition of the resulting type of a row-major dynamic matrix
   using MatrixType1  = DynamicMatrix<int,rowMajor>;
   using DeclSymType1 = typename DeclSymTrait<MatrixType1>::Type;

   // Definition of the resulting type of a lower column-major static matrix
   using MatrixType2  = LowerMatrix< StaticMatrix<int,3UL,3UL,columnMajor> >;
   using DeclSymType2 = typename DeclSymTrait<MatrixType2>::Type;
   \endcode
*/
template <typename MT> // Type of the matrix
struct DeclSymTrait : public decltype(evalDeclSymTrait(std::declval<MT &>())){};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the DeclSymTrait type trait.
// \ingroup math_traits
//
// The DeclSymTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the DeclSymTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::DeclSymTrait<MT>::Type;
   using Type2 = mtrc::numeric::DeclSymTrait_t<MT>;
   \endcode
*/
template <typename MT> // Type of the matrix
using DeclSymTrait_t = typename DeclSymTrait<MT>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the DeclSymTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  typename> // Restricting condition
struct DeclSymTraitEval {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DeclSymTraitEval class template for square matrix types.
// \ingroup math_traits
*/
template <typename MT> // Type of the matrix
struct DeclSymTraitEval<
	MT, EnableIf_t<IsMatrix_v<MT> && (Size_v<MT, 0UL> == DefaultSize_v || Size_v<MT, 1UL> == DefaultSize_v ||
									  Size_v<MT, 0UL> == Size_v<MT, 1UL>)>> {
	using Type = SymmetricMatrix<typename MT::ResultType>;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
