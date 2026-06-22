// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DVECDVECINNEREXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DVECDVECINNEREXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/SIMD.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/shims/PrevMultiple.h>
#include <metric/numeric/math/traits/MultTrait.h>
#include <metric/numeric/math/typetraits/HasSIMDAdd.h>
#include <metric/numeric/math/typetraits/HasSIMDMult.h>
#include <metric/numeric/math/typetraits/IsPadded.h>
#include <metric/numeric/math/typetraits/IsSIMDCombinable.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/system/Optimizations.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/FunctionTrace.h>
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
/*!\brief Auxiliary helper struct for the dense vector/dense vector scalar multiplication.
// \ingroup dense_vector
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  typename VT2> // Type of the right-hand side dense vector
struct DVecDVecInnerExprHelper {
	//**Type definitions****************************************************************************
	//! Composite type of the left-hand side dense vector expression.
	using CT1 = RemoveReference_t<CompositeType_t<VT1>>;

	//! Composite type of the right-hand side dense vector expression.
	using CT2 = RemoveReference_t<CompositeType_t<VT2>>;
	//**********************************************************************************************

	//**********************************************************************************************
	static constexpr bool value =
		(useOptimizedKernels && CT1::simdEnabled && CT2::simdEnabled &&
		 IsSIMDCombinable_v<ElementType_t<CT1>, ElementType_t<CT2>> &&
		 HasSIMDAdd_v<ElementType_t<CT1>, ElementType_t<CT1>> && HasSIMDMult_v<ElementType_t<CT1>, ElementType_t<CT1>>);
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default backend implementation of the scalar product (inner product) of two dense
//        vectors (\f$ s=\vec{a}*\vec{b} \f$).
// \ingroup dense_vector
//
// \param lhs The left-hand side dense vector for the inner product.
// \param rhs The right-hand side dense vector for the inner product.
// \return The scalar product.
//
// This function implements the performance optimized scalar product of two dense vectors.
// Due to the explicit application of the SFINAE principle, this function can only be selected
// by the compiler in case vectorization cannot be applied.
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  typename VT2> // Type of the right-hand side dense vector
inline auto dvecdvecinner(const DenseVector<VT1, true> &lhs, const DenseVector<VT2, false> &rhs)
	-> DisableIf_t<DVecDVecInnerExprHelper<VT1, VT2>::value, const MultTrait_t<ElementType_t<VT1>, ElementType_t<VT2>>>
{
	using CT1 = CompositeType_t<VT1>;
	using CT2 = CompositeType_t<VT2>;
	using ET1 = ElementType_t<VT1>;
	using ET2 = ElementType_t<VT2>;
	using MultType = MultTrait_t<ET1, ET2>;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == (*rhs).size(), "Invalid vector sizes");

	if ((*lhs).size() == 0UL)
		return MultType();

	CT1 left(*lhs);
	CT2 right(*rhs);

	MultType sp(left[0UL] * right[0UL]);
	size_t i(1UL);

	for (; (i + 4UL) <= left.size(); i += 4UL) {
		sp += left[i] * right[i] + left[i + 1UL] * right[i + 1UL] + left[i + 2UL] * right[i + 2UL] +
			  left[i + 3UL] * right[i + 3UL];
	}
	for (; (i + 2UL) <= left.size(); i += 2UL) {
		sp += left[i] * right[i] + left[i + 1UL] * right[i + 1UL];
	}
	for (; i < left.size(); ++i) {
		sp += left[i] * right[i];
	}

	return sp;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief SIMD optimized backend implementation of the scalar product (inner product) of two
//        dense vectors (\f$ s=\vec{a}*\vec{b} \f$).
// \ingroup dense_vector
//
// \param lhs The left-hand side dense vector for the inner product.
// \param rhs The right-hand side dense vector for the inner product.
// \return The scalar product.
//
// This function implements the performance optimized scalar product of two dense vectors.
// Due to the explicit application of the SFINAE principle, this function can only be selected
// by the compiler in case vectorization can be applied.
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  typename VT2> // Type of the right-hand side dense vector
inline auto dvecdvecinner(const DenseVector<VT1, true> &lhs, const DenseVector<VT2, false> &rhs)
	-> EnableIf_t<DVecDVecInnerExprHelper<VT1, VT2>::value, const MultTrait_t<ElementType_t<VT1>, ElementType_t<VT2>>>
{
	using CT1 = CompositeType_t<VT1>;
	using CT2 = CompositeType_t<VT2>;
	using XT1 = RemoveReference_t<CT1>;
	using XT2 = RemoveReference_t<CT2>;
	using ET1 = ElementType_t<VT1>;
	using ET2 = ElementType_t<VT2>;
	using MultType = MultTrait_t<ET1, ET2>;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == (*rhs).size(), "Invalid vector sizes");

	if ((*lhs).size() == 0UL)
		return MultType();

	CT1 left(*lhs);
	CT2 right(*rhs);

	constexpr size_t SIMDSIZE = SIMDTrait<MultType>::size;
	constexpr bool remainder(!IsPadded_v<XT1> || !IsPadded_v<XT2>);

	const size_t N(left.size());

	const size_t ipos(remainder ? prevMultiple(N, SIMDSIZE) : N);
	METRIC_NUMERIC_INTERNAL_ASSERT(ipos <= N, "Invalid end calculation");

	SIMDTrait_t<MultType> xmm1, xmm2, xmm3, xmm4;
	size_t i(0UL);

	for (; (i + SIMDSIZE * 3UL) < ipos; i += SIMDSIZE * 4UL) {
		xmm1 = xmm1 + (left.load(i) * right.load(i));
		xmm2 = xmm2 + (left.load(i + SIMDSIZE) * right.load(i + SIMDSIZE));
		xmm3 = xmm3 + (left.load(i + SIMDSIZE * 2UL) * right.load(i + SIMDSIZE * 2UL));
		xmm4 = xmm4 + (left.load(i + SIMDSIZE * 3UL) * right.load(i + SIMDSIZE * 3UL));
	}
	for (; (i + SIMDSIZE) < ipos; i += SIMDSIZE * 2UL) {
		xmm1 = xmm1 + (left.load(i) * right.load(i));
		xmm2 = xmm2 + (left.load(i + SIMDSIZE) * right.load(i + SIMDSIZE));
	}
	for (; i < ipos; i += SIMDSIZE) {
		xmm1 = xmm1 + (left.load(i) * right.load(i));
	}

	MultType sp(sum(xmm1 + xmm2 + xmm3 + xmm4));

	for (; remainder && i < N; ++i) {
		sp += left[i] * right[i];
	}

	return sp;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication operator for the scalar product (inner product) of two dense vectors
//        (\f$ s=\vec{a}*\vec{b} \f$).
// \ingroup dense_vector
//
// \param lhs The left-hand side dense vector for the inner product.
// \param rhs The right-hand side dense vector for the inner product.
// \return The scalar product.
// \exception std::invalid_argument Vector sizes do not match.
//
// This operator represents the scalar product (inner product) of two dense vectors:

   \code
   mtrc::numeric::DynamicVector<double> a, b;
   mtrc::numeric::double res;
   // ... Resizing and initialization
   res = trans(a) * b;
   \endcode

// The operator returns a scalar value of the higher-order element type of the two involved
// vector element types \a VT1::ElementType and \a VT2::ElementType. Both vector types \a VT1
// and \a VT2 as well as the two element types \a VT1::ElementType and \a VT2::ElementType
// have to be supported by the MultTrait class template.\n
// In case the current sizes of the two given vectors don't match, a \a std::invalid_argument
// is thrown.
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  typename VT2> // Type of the right-hand side dense vector
inline decltype(auto) operator*(const DenseVector<VT1, true> &lhs, const DenseVector<VT2, false> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if ((*lhs).size() != (*rhs).size()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Vector sizes do not match");
	}

	return dvecdvecinner(*lhs, *rhs);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
