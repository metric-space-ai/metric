// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ISVECTORIZABLE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ISVECTORIZABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/typetraits/IsFloat.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>
#include <metric/numeric/util/typetraits/RemoveCV.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsVectorizable type trait.
// \ingroup type_traits
*/
template <typename T> struct IsVectorizableHelper {
  public:
	//**********************************************************************************************
	static constexpr bool value =
		(sizeof(T) <= 8UL) &&
		((bool(METRIC_NUMERIC_SSE_MODE) && IsFloat_v<T>) || (bool(METRIC_NUMERIC_SSE2_MODE) && IsNumeric_v<T>) ||
		 (bool(METRIC_NUMERIC_AVX512BW_MODE) && IsNumeric_v<T>) ||
		 (bool(METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE) && IsNumeric_v<T> && sizeof(T) >= 4UL));
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsVectorizableHelper class template for 'complex'.
// \ingroup type_traits
*/
template <typename T> struct IsVectorizableHelper<complex<T>> {
  public:
	//**********************************************************************************************
	static constexpr bool value = IsVectorizableHelper<RemoveCV_t<T>>::value;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsVectorizableHelper class template for 'void'.
// \ingroup type_traits
*/
template <> struct IsVectorizableHelper<void> {
  public:
	//**********************************************************************************************
	static constexpr bool value = false;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for vectorizable types.
// \ingroup type_traits
//
// Depending on the available instruction set (SSE, SSE2, SSE3, SSE4, AVX, AVX2, MIC, ...),
// this type trait tests whether or not the given template parameter is a vectorizable type,
// i.e. a type for which intrinsic vector operations and optimizations can be used. Currently,
// all built-in data types except \c bool and the according complex numbers are considered to
// be vectorizable types. In case the type is vectorizable, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives
// from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and the
// class derives from \a FalseType.

   \code
   mtrc::numeric::IsVectorizable< int >::value         // Evaluates to 'true'
   mtrc::numeric::IsVectorizable< const float >::Type  // Results in TrueType
   mtrc::numeric::IsVectorizable< volatile double >    // Is derived from TrueType
   mtrc::numeric::IsVectorizable< void >::value        // Evaluates to 'false'
   mtrc::numeric::IsVectorizable< const bool >::Type   // Results in FalseType
   mtrc::numeric::IsVectorizable< volatile MyClass >   // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsVectorizable : public BoolConstant<IsVectorizableHelper<RemoveCV_t<T>>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsVectorizable type trait.
// \ingroup type_traits
//
// The IsVectorizable_v variable template provides a convenient shortcut to access the nested
// \a value of the IsVectorizable class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsVectorizable<T>::value;
   constexpr bool value2 = mtrc::numeric::IsVectorizable_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsVectorizable_v = IsVectorizable<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
