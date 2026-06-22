// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_ALIGNMENTOF_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_ALIGNMENTOF_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Vectorization.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typetraits/IsVectorizable.h>
#include <type_traits>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the AlignmentOf type trait.
// \ingroup type_traits
*/
template <typename T> struct AlignmentOfHelper {
  private:
	//**********************************************************************************************
	static constexpr size_t defaultAlignment = std::alignment_of<T>::value;
	//**********************************************************************************************

  public:
	//**********************************************************************************************
	static constexpr size_t value =
#if METRIC_NUMERIC_AVX512BW_MODE
		(IsVectorizable_v<T>) ? (64UL) : (defaultAlignment);
#elif METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
		(IsVectorizable_v<T>) ? (sizeof(T) >= 4UL ? 64UL : 32UL) : (defaultAlignment);
#elif METRIC_NUMERIC_AVX2_MODE
		(IsVectorizable_v<T>) ? (32UL) : (defaultAlignment);
#elif METRIC_NUMERIC_SSE2_MODE
		(IsVectorizable_v<T>) ? (16UL) : (defaultAlignment);
#else
		defaultAlignment;
#endif
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of \c AlignmentOfHelper for 'float'.
// \ingroup type_traits
*/
template <> struct AlignmentOfHelper<float> {
  public:
	//**********************************************************************************************
	static constexpr size_t value =
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
		64UL;
#elif METRIC_NUMERIC_AVX_MODE
		32UL;
#elif METRIC_NUMERIC_SSE_MODE
		16UL;
#else
		std::alignment_of<float>::value;
#endif
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of \c AlignmentOfHelper for 'double'.
// \ingroup type_traits
*/
template <> struct AlignmentOfHelper<double> {
  public:
	//**********************************************************************************************
	static constexpr size_t value =
#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE
		64UL;
#elif METRIC_NUMERIC_AVX_MODE
		32UL;
#elif METRIC_NUMERIC_SSE2_MODE
		16UL;
#else
		std::alignment_of<double>::value;
#endif
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of \c AlignmentOfHelper for 'complex<T>'.
// \ingroup type_traits
*/
template <typename T> struct AlignmentOfHelper<complex<T>> {
  public:
	//**********************************************************************************************
	static constexpr size_t value = AlignmentOfHelper<T>::value;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Evaluation of the required alignment of the given data type.
// \ingroup type_traits
//
// The AlignmentOf type trait template evaluates the required alignment for the given data type.
// For instance, for fundamental data types that can be vectorized via SSE or AVX instructions,
// the proper alignment is 16 or 32 bytes, respectively. For all other data types, a multiple
// of the alignment chosen by the compiler is returned. The evaluated alignment can be queried
// via the nested \a value member.

   \code
   AlignmentOf<unsigned int>::value  // Evaluates to 32 if AVX2 is available, to 16 if only
									 // SSE2 is available, and a multiple of the alignment
									 // chosen by the compiler otherwise.
   AlignmentOf<double>::value        // Evaluates to 32 if AVX is available, to 16 if only
									 // SSE is available, and a multiple of the alignment
									 // chosen by the compiler otherwise.
   \endcode
*/
template <typename T> struct AlignmentOf : public IntegralConstant<size_t, AlignmentOfHelper<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of \c AlignmentOf for 'const' data types.
// \ingroup type_traits
*/
template <typename T> struct AlignmentOf<const T> : public IntegralConstant<size_t, AlignmentOfHelper<T>::value> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of \c AlignmentOf for 'volatile' data types.
// \ingroup type_traits
*/
template <typename T> struct AlignmentOf<volatile T> : public IntegralConstant<size_t, AlignmentOfHelper<T>::value> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of \c AlignmentOf for 'const volatile' data types.
// \ingroup type_traits
*/
template <typename T>
struct AlignmentOf<const volatile T> : public IntegralConstant<size_t, AlignmentOfHelper<T>::value> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the AlignmentOf type trait.
// \ingroup type_traits
//
// The AlignmentOf_v variable template provides a convenient shortcut to access the nested
// \a value of the AlignmentOf class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr size_t value1 = mtrc::numeric::AlignmentOf<T>::value;
   constexpr size_t value2 = mtrc::numeric::AlignmentOf_v<T>;
   \endcode
*/
template <typename T> constexpr size_t AlignmentOf_v = AlignmentOf<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
