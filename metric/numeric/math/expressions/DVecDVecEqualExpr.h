// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DVECDVECEQUALEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DVECDVECEQUALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/SIMD.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/shims/Equal.h>
#include <metric/numeric/math/shims/PrevMultiple.h>
#include <metric/numeric/math/typetraits/HasSIMDEqual.h>
#include <metric/numeric/math/typetraits/IsPadded.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/system/Optimizations.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the dense vector/dense vector equality comparison.
// \ingroup dense_vector
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  typename VT2> // Type of the right-hand side dense vector
struct DVecDVecEqualExprHelper {
	//**Type definitions****************************************************************************
	//! Composite type of the left-hand side dense vector expression.
	using CT1 = RemoveReference_t<CompositeType_t<VT1>>;

	//! Composite type of the right-hand side dense vector expression.
	using CT2 = RemoveReference_t<CompositeType_t<VT2>>;
	//**********************************************************************************************

	//**********************************************************************************************
	static constexpr bool value = (useOptimizedKernels && CT1::simdEnabled && CT2::simdEnabled &&
								   HasSIMDEqual_v<ElementType_t<CT1>, ElementType_t<CT2>>);
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL BINARY RELATIONAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default equality check of two dense vectors.
// \ingroup dense_vector
//
// \param a The left-hand side dense vector for the comparison.
// \param b The right-hand side dense vector for the comparison.
// \return \a true if the two vectors are equal, \a false if not.
//
// Equal function for the comparison of two dense vectors. Due to the limited machine accuracy,
// a direct comparison of two floating point numbers should be avoided. This function offers the
// possibility to compare two floating-point vectors with a certain accuracy margin.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename VT1 // Type of the left-hand side dense vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side dense vector
		  ,
		  typename VT2 // Type of the right-hand side dense vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side dense vector
inline auto equal(const DenseVector<VT1, TF1> &lhs, const DenseVector<VT2, TF2> &rhs)
	-> DisableIf_t<DVecDVecEqualExprHelper<VT1, VT2>::value, bool>
{
	using CT1 = CompositeType_t<VT1>;
	using CT2 = CompositeType_t<VT2>;

	// Early exit in case the vector sizes don't match
	if ((*lhs).size() != (*rhs).size())
		return false;

	// Evaluation of the two dense vector operands
	CT1 a(*lhs);
	CT2 b(*rhs);

	// In order to compare the two vectors, the data values of the lower-order data
	// type are converted to the higher-order data type within the equal function.
	for (size_t i = 0UL; i < a.size(); ++i)
		if (!equal<RF>(a[i], b[i]))
			return false;
	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief SIMD optimized equality check of two dense vectors.
// \ingroup dense_vector
//
// \param a The left-hand side dense vector for the comparison.
// \param b The right-hand side dense vector for the comparison.
// \return \a true if the two vectors are equal, \a false if not.
//
// Equal function for the comparison of two dense vectors. Due to the limited machine accuracy,
// a direct comparison of two floating point numbers should be avoided. This function offers the
// possibility to compare two floating-point vectors with a certain accuracy margin.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename VT1 // Type of the left-hand side dense vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side dense vector
		  ,
		  typename VT2 // Type of the right-hand side dense vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side dense vector
inline auto equal(const DenseVector<VT1, TF1> &lhs, const DenseVector<VT2, TF2> &rhs)
	-> EnableIf_t<DVecDVecEqualExprHelper<VT1, VT2>::value, bool>
{
	using CT1 = CompositeType_t<VT1>;
	using CT2 = CompositeType_t<VT2>;
	using XT1 = RemoveReference_t<CT1>;
	using XT2 = RemoveReference_t<CT2>;

	// Early exit in case the vector sizes don't match
	if ((*lhs).size() != (*rhs).size())
		return false;

	// Evaluation of the two dense vector operands
	CT1 a(*lhs);
	CT2 b(*rhs);

	constexpr size_t SIMDSIZE = SIMDTrait<ElementType_t<VT1>>::size;
	constexpr bool remainder(!IsPadded_v<XT1> || !IsPadded_v<XT2>);

	const size_t N(a.size());

	const size_t ipos(remainder ? prevMultiple(N, SIMDSIZE) : N);
	METRIC_NUMERIC_INTERNAL_ASSERT(ipos <= N, "Invalid end calculation");

	size_t i(0UL);

	for (; (i + SIMDSIZE * 3UL) < ipos; i += SIMDSIZE * 4UL) {
		if (!equal<RF>(a.load(i), b.load(i)))
			return false;
		if (!equal<RF>(a.load(i + SIMDSIZE), b.load(i + SIMDSIZE)))
			return false;
		if (!equal<RF>(a.load(i + SIMDSIZE * 2UL), b.load(i + SIMDSIZE * 2UL)))
			return false;
		if (!equal<RF>(a.load(i + SIMDSIZE * 3UL), b.load(i + SIMDSIZE * 3UL)))
			return false;
	}
	for (; (i + SIMDSIZE) < ipos; i += SIMDSIZE * 2UL) {
		if (!equal<RF>(a.load(i), b.load(i)))
			return false;
		if (!equal<RF>(a.load(i + SIMDSIZE), b.load(i + SIMDSIZE)))
			return false;
	}
	for (; i < ipos; i += SIMDSIZE) {
		if (!equal<RF>(a.load(i), b.load(i)))
			return false;
	}
	for (; remainder && i < N; ++i) {
		if (!equal<RF>(a[i], b[i]))
			return false;
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Equality operator for the comparison of two dense vectors.
// \ingroup dense_vector
//
// \param lhs The left-hand side dense vector for the comparison.
// \param rhs The right-hand side dense vector for the comparison.
// \return \a true if the two vectors are equal, \a false if not.
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side dense vector
		  ,
		  typename VT2 // Type of the right-hand side dense vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side dense vector
inline bool operator==(const DenseVector<VT1, TF1> &lhs, const DenseVector<VT2, TF2> &rhs)
{
	return equal<relaxed>(lhs, rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inequality operator for the comparison of two dense vectors.
// \ingroup dense_vector
//
// \param lhs The left-hand side dense vector for the comparison.
// \param rhs The right-hand side dense vector for the comparison.
// \return \a true if the two vectors are not equal, \a false if they are equal.
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side dense vector
		  ,
		  typename VT2 // Type of the right-hand side dense vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side dense vector
inline bool operator!=(const DenseVector<VT1, TF1> &lhs, const DenseVector<VT2, TF2> &rhs)
{
	return !equal<relaxed>(lhs, rhs);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
