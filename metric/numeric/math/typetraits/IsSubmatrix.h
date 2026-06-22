// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSUBMATRIX_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSUBMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/views/Forward.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for submatrices.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a submatrix (i.e. a view
// on the part of a dense or sparse matrix). In case the type is a submatrix, the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the class
// derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and
// the class derives from \a FalseType.

   \code
   using mtrc::numeric::aligned;

   using MatrixType1 = mtrc::numeric::StaticMatrix<int,10UL,16UL>;
   using MatrixType2 = mtrc::numeric::DynamicMatrix<double>;
   using MatrixType3 = mtrc::numeric::CompressedMatrix<float>;

   MatrixType1 A;
   MatrixType2 B( 100UL, 200UL );
   MatrixType3 C( 200UL, 250UL );

   using SubmatrixType1 = decltype( mtrc::numeric::submatrix<2UL,2UL,4UL,8UL>( A ) );
   using SubmatrixType2 = decltype( mtrc::numeric::submatrix<aligned>( B, 8UL, 8UL, 24UL, 32UL ) );
   using SubmatrixType3 = decltype( mtrc::numeric::submatrix( C, 5UL, 7UL, 13UL, 17UL ) );

   mtrc::numeric::IsSubmatrix< SubmatrixType1 >::value       // Evaluates to 1
   mtrc::numeric::IsSubmatrix< const SubmatrixType2 >::Type  // Results in TrueType
   mtrc::numeric::IsSubmatrix< volatile SubmatrixType3 >     // Is derived from TrueType
   mtrc::numeric::IsSubmatrix< MatrixType1 >::value          // Evaluates to 0
   mtrc::numeric::IsSubmatrix< const MatrixType2 >::Type     // Results in FalseType
   mtrc::numeric::IsSubmatrix< volatile MatrixType3 >        // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsSubmatrix : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSubmatrix type trait for 'Submatrix'.
// \ingroup math_type_traits
*/
template <typename MT, AlignmentFlag AF, bool SO, bool DF, size_t... CSAs>
struct IsSubmatrix<Submatrix<MT, AF, SO, DF, CSAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSubmatrix type trait for 'const Submatrix'.
// \ingroup math_type_traits
*/
template <typename MT, AlignmentFlag AF, bool SO, bool DF, size_t... CSAs>
struct IsSubmatrix<const Submatrix<MT, AF, SO, DF, CSAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSubmatrix type trait for 'volatile Submatrix'.
// \ingroup math_type_traits
*/
template <typename MT, AlignmentFlag AF, bool SO, bool DF, size_t... CSAs>
struct IsSubmatrix<volatile Submatrix<MT, AF, SO, DF, CSAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSubmatrix type trait for 'const volatile Submatrix'.
// \ingroup math_type_traits
*/
template <typename MT, AlignmentFlag AF, bool SO, bool DF, size_t... CSAs>
struct IsSubmatrix<const volatile Submatrix<MT, AF, SO, DF, CSAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSubmatrix type trait.
// \ingroup math_type_traits
//
// The IsSubmatrix_v variable template provides a convenient shortcut to access the nested
// \a value of the IsSubmatrix class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSubmatrix<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSubmatrix_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSubmatrix_v = IsSubmatrix<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
