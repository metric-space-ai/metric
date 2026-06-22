// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDCONJ_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASSIMDCONJ_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/HasSIMDMult.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsFloatingPoint.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>
#include <metric/numeric/util/typetraits/IsSigned.h>
#include <metric/numeric/util/typetraits/RemoveCVRef.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the HasSIMDConj type trait.
// \ingroup math_type_traits
*/
template <typename T // Type of the operand
		  ,
		  typename = void> // Restricting condition
struct HasSIMDConjHelper : public BoolConstant<IsNumeric_v<T>> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T>
struct HasSIMDConjHelper<complex<T>>
	: public BoolConstant<IsNumeric_v<T> && IsSigned_v<T> &&
						  ((!bool(METRIC_NUMERIC_AVX512F_MODE) && HasSIMDMult_v<T, T> &&
							(IsFloatingPoint_v<T> || sizeof(T) <= 4UL)) ||
						   (bool(METRIC_NUMERIC_AVX512F_MODE) && IsFloatingPoint_v<T>) ||
						   (bool(METRIC_NUMERIC_AVX512BW_MODE) && sizeof(T) == 2UL) ||
						   (bool(METRIC_NUMERIC_AVX512F_MODE) && sizeof(T) >= 4UL))> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of a SIMD conjugate operation for the given data type.
// \ingroup math_type_traits
//
// Depending on the available instruction set (SSE, SSE2, SSE3, SSE4, AVX, AVX2, MIC, ...) and
// the used compiler, this type trait provides the information whether a SIMD conjugate operation
// exists for the given data type \a T (ignoring the cv-qualifiers). In case the SIMD operation
// is available, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to
// \a false, \a Type is \a FalseType, and the class derives from \a FalseType. The following
// example assumes that AVX is available:

   \code
   mtrc::numeric::HasSIMDConj< int >::value             // Evaluates to 1
   mtrc::numeric::HasSIMDConj< double >::Type           // Results in TrueType
   mtrc::numeric::HasSIMDConj< complex<float> >         // Is derived from TrueType
   mtrc::numeric::HasSIMDConj< complex<bool> >::value   // Evaluates to 0
   mtrc::numeric::HasSIMDConj< complex<int> >::Type     // Results in FalseType
   mtrc::numeric::HasSIMDConj< complex<unsigned int> >  // Is derived from FalseType
   \endcode
*/
template <typename T> // Type of the operand
struct HasSIMDConj : public BoolConstant<HasSIMDConjHelper<RemoveCVRef_t<T>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasSIMDConj type trait.
// \ingroup math_type_traits
//
// The HasSIMDConj_v variable template provides a convenient shortcut to access the nested
// \a value of the HasSIMDConj class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasSIMDConj<T>::value;
   constexpr bool value2 = mtrc::numeric::HasSIMDConj_v<T>;
   \endcode
*/
template <typename T> // Type of the operand
constexpr bool HasSIMDConj_v = HasSIMDConj<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
