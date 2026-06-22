// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_DECLDIAGTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_DECLDIAGTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/adaptors/diagonalmatrix/BaseTemplate.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/Size.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/InvalidType.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename> struct DeclDiagTrait;
template <typename, typename = void> struct DeclDiagTraitEval;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> auto evalDeclDiagTrait(const volatile T &) -> DeclDiagTraitEval<T>;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the DeclDiagTrait class.
// \ingroup math_traits
//
// \section decldiagtrait_general General
//
// The DeclDiagTrait class template offers the possibility to select the resulting data type
// of a generic decldiag() operation on the given type \a MT. In case the given type \a MT is
// a dense or sparse matrix type, DeclDiagTrait defines the nested type \a Type, which represents
// the resulting data type of the decldiag() operation. Otherwise there is no nested type \a Type.
// Note that \a const and \a volatile qualifiers and reference modifiers are generally ignored.
//
//
// \section decldiagtrait_specializations Creating custom specializations
//
// Per default, DeclDiagTrait supports all matrix types of the Metric numeric library (including views and
// adaptors). For all other data types it is possible to specialize the DeclDiagTrait template.
// The following example shows the according specialization for the SymmetricMatrix class template:

   \code
   template< typename MT, bool SO, bool DF, bool SF >
   struct DeclDiagTrait< SymmetricMatrix<MT,SO,DF,SF> >
   {
	  using Type = DiagonalMatrix<MT>;
   };
   \endcode

// \n \section decldiagtrait_examples Examples
//
// The following example demonstrates the use of the DeclDiagTrait template, where depending on
// the given matrix type the resulting type is selected:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::SymmetricMatrix;
   using mtrc::numeric::DeclDiagTrait;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   // Definition of the resulting type of a row-major dynamic matrix
   using MatrixType1   = DynamicMatrix<int,rowMajor>;
   using DeclDiagType1 = typename DeclDiagTrait<MatrixType1>::Type;

   // Definition of the resulting type of a symmetric column-major static matrix
   using MatrixType2   = SymmetricMatrix< StaticMatrix<int,3UL,3UL,columnMajor> >;
   using DeclDiagType2 = typename DeclDiagTrait<MatrixType2>::Type;
   \endcode
*/
template <typename MT> // Type of the matrix
struct DeclDiagTrait : public decltype(evalDeclDiagTrait(std::declval<MT &>())){};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the DeclDiagTrait type trait.
// \ingroup math_traits
//
// The DeclDiagTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the DeclDiagTrait class template. For instance, given the matrix type \a MT the
// following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::DeclDiagTrait<MT>::Type;
   using Type2 = mtrc::numeric::DeclDiagTrait_t<MT>;
   \endcode
*/
template <typename MT> // Type of the matrix
using DeclDiagTrait_t = typename DeclDiagTrait<MT>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the DeclDiagTrait type trait.
// \ingroup math_traits
*/
template <typename MT // Type of the matrix
		  ,
		  typename> // Restricting condition
struct DeclDiagTraitEval {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DeclDiagTraitEval class template for square matrix types.
// \ingroup math_traits
*/
template <typename MT> // Type of the matrix
struct DeclDiagTraitEval<
	MT, EnableIf_t<IsMatrix_v<MT> && (Size_v<MT, 0UL> == DefaultSize_v || Size_v<MT, 1UL> == DefaultSize_v ||
									  Size_v<MT, 0UL> == Size_v<MT, 1UL>)>> {
	using Type = DiagonalMatrix<typename MT::ResultType>;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
