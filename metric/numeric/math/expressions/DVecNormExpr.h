// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DVECNORMEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DVECNORMEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/SIMD.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/functors/Abs.h>
#include <metric/numeric/math/functors/Bind2nd.h>
#include <metric/numeric/math/functors/Cbrt.h>
#include <metric/numeric/math/functors/L1Norm.h>
#include <metric/numeric/math/functors/L2Norm.h>
#include <metric/numeric/math/functors/L3Norm.h>
#include <metric/numeric/math/functors/L4Norm.h>
#include <metric/numeric/math/functors/LpNorm.h>
#include <metric/numeric/math/functors/Noop.h>
#include <metric/numeric/math/functors/Pow.h>
#include <metric/numeric/math/functors/Pow2.h>
#include <metric/numeric/math/functors/Pow3.h>
#include <metric/numeric/math/functors/Qdrt.h>
#include <metric/numeric/math/functors/SqrAbs.h>
#include <metric/numeric/math/functors/Sqrt.h>
#include <metric/numeric/math/shims/Evaluate.h>
#include <metric/numeric/math/shims/Invert.h>
#include <metric/numeric/math/shims/IsZero.h>
#include <metric/numeric/math/shims/PrevMultiple.h>
#include <metric/numeric/math/traits/MultTrait.h>
#include <metric/numeric/math/typetraits/HasLoad.h>
#include <metric/numeric/math/typetraits/HasSIMDAdd.h>
#include <metric/numeric/math/typetraits/IsPadded.h>
#include <metric/numeric/math/typetraits/IsSIMDEnabled.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
#include <metric/numeric/system/Optimizations.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/TypeList.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/algorithms/Min.h>
#include <metric/numeric/util/mpl/And.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/HasMember.h>
#include <metric/numeric/util/typetraits/RemoveCVRef.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the dense vector norms.
// \ingroup dense_vector
*/
template <typename VT // Type of the dense vector
		  ,
		  typename Abs // Type of the abs operation
		  ,
		  typename Power> // Type of the power operation
struct DVecNormHelper {
	//**Type definitions****************************************************************************
	//! Element type of the dense vector expression.
	using ET = ElementType_t<VT>;

	//! Composite type of the dense vector expression.
	using CT = RemoveReference_t<CompositeType_t<VT>>;
	//**********************************************************************************************

	//**********************************************************************************************
	static constexpr bool value =
		(useOptimizedKernels && CT::simdEnabled && If_t < HasSIMDEnabled_v<Abs> && HasSIMDEnabled_v<Power>,
		 And_t<GetSIMDEnabled<Abs, ET>, GetSIMDEnabled<Power, ET>>,
		 And_t<HasLoad<Abs>, HasLoad<Power>> > ::value && HasSIMDAdd_v<ElementType_t<CT>, ElementType_t<CT>>);
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default backend implementation of the norm of a dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \param abs The functor for the abs operation.
// \param power The functor for the power operation.
// \param root The functor for the root operation.
// \return The norm of the given vector.
//
// This function implements the performance optimized norm of a dense vector. Due to the
// explicit application of the SFINAE principle, this function can only be selected by the
// compiler in case vectorization cannot be applied.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename Abs // Type of the abs operation
		  ,
		  typename Power // Type of the power operation
		  ,
		  typename Root> // Type of the root operation
inline decltype(auto) norm_backend(const DenseVector<VT, TF> &dv, Abs abs, Power power, Root root, FalseType)
{
	using CT = CompositeType_t<VT>;
	using ET = ElementType_t<VT>;
	using PT = RemoveCVRef_t<decltype(power(abs(std::declval<ET>())))>;
	using RT = RemoveCVRef_t<decltype(evaluate(root(std::declval<PT>())))>;

	if ((*dv).size() == 0UL)
		return RT{};

	CT tmp(*dv);

	const size_t N(tmp.size());

	PT norm(power(abs(tmp[0UL])));
	size_t i(1UL);

	for (; (i + 4UL) <= N; i += 4UL) {
		norm += power(abs(tmp[i])) + power(abs(tmp[i + 1UL])) + power(abs(tmp[i + 2UL])) + power(abs(tmp[i + 3UL]));
	}
	for (; (i + 2UL) <= N; i += 2UL) {
		norm += power(abs(tmp[i])) + power(abs(tmp[i + 1UL]));
	}
	for (; i < N; ++i) {
		norm += power(abs(tmp[i]));
	}

	return evaluate(root(norm));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief SIMD optimized backend implementation of the norm of a dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \param abs The functor for the abs operation.
// \param power The functor for the power operation.
// \param root The functor for the root operation.
// \return The norm of the given vector.
//
// This function implements the performance optimized norm of a dense vector. Due to the
// explicit application of the SFINAE principle, this function can only be selected by the
// compiler in case vectorization can be applied.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename Abs // Type of the abs operation
		  ,
		  typename Power // Type of the power operation
		  ,
		  typename Root> // Type of the root operation
inline decltype(auto) norm_backend(const DenseVector<VT, TF> &dv, Abs abs, Power power, Root root, TrueType)
{
	using CT = CompositeType_t<VT>;
	using ET = ElementType_t<VT>;
	using RT = decltype(evaluate(root(std::declval<ET>())));

	static constexpr size_t SIMDSIZE = SIMDTrait<ET>::size;

	if ((*dv).size() == 0UL)
		return RT{};

	CT tmp(*dv);

	const size_t N(tmp.size());

	constexpr bool remainder(!IsPadded_v<RemoveReference_t<VT>>);

	const size_t ipos(remainder ? prevMultiple(N, SIMDSIZE) : N);
	METRIC_NUMERIC_INTERNAL_ASSERT(ipos <= N, "Invalid end calculation");

	size_t i(0UL);
	ET norm{};

	if (SIMDSIZE * 3UL < ipos) {
		SIMDTrait_t<ET> xmm1{}, xmm2{}, xmm3{}, xmm4{};

		for (; (i + SIMDSIZE * 3UL) < ipos; i += SIMDSIZE * 4UL) {
			xmm1 += power(abs(tmp.load(i)));
			xmm2 += power(abs(tmp.load(i + SIMDSIZE)));
			xmm3 += power(abs(tmp.load(i + SIMDSIZE * 2UL)));
			xmm4 += power(abs(tmp.load(i + SIMDSIZE * 3UL)));
		}
		for (; (i + SIMDSIZE) < ipos; i += SIMDSIZE * 2UL) {
			xmm1 += power(abs(tmp.load(i)));
			xmm2 += power(abs(tmp.load(i + SIMDSIZE)));
		}
		for (; i < ipos; i += SIMDSIZE) {
			xmm1 += power(abs(tmp.load(i)));
		}

		norm = sum(xmm1 + xmm2 + xmm3 + xmm4);
	} else if (SIMDSIZE < ipos) {
		SIMDTrait_t<ET> xmm1{}, xmm2{};

		for (; (i + SIMDSIZE) < ipos; i += SIMDSIZE * 2UL) {
			xmm1 += power(abs(tmp.load(i)));
			xmm2 += power(abs(tmp.load(i + SIMDSIZE)));
		}
		for (; i < ipos; i += SIMDSIZE) {
			xmm1 += power(abs(tmp.load(i)));
		}

		norm = sum(xmm1 + xmm2);
	} else {
		SIMDTrait_t<ET> xmm1{};

		for (; i < ipos; i += SIMDSIZE) {
			xmm1 += power(abs(tmp.load(i)));
		}

		norm = sum(xmm1);
	}

	for (; remainder && i < N; ++i) {
		norm += power(abs(tmp[i]));
	}

	return evaluate(root(norm));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Computes a custom norm for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \param abs The functor for the abs operation.
// \param power The functor for the power operation.
// \param root The functor for the root operation.
// \return The norm of the given dense vector.
//
// This function computes a custom norm of the given dense vector by means of the given functors.
// The following example demonstrates the computation of the L2 norm by means of the mtrc::numeric::Noop,
// mtrc::numeric::Pow2 and mtrc::numeric::Sqrt functors:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization
   const double l2 = norm( a, mtrc::numeric::Noop(), mtrc::numeric::Pow2(), mtrc::numeric::Sqrt() );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename Abs // Type of the abs operation
		  ,
		  typename Power // Type of the power operation
		  ,
		  typename Root> // Type of the root operation
inline decltype(auto) norm_backend(const DenseVector<VT, TF> &dv, Abs abs, Power power, Root root)
{
	return norm_backend(*dv, abs, power, root, Bool_t<DVecNormHelper<VT, Abs, Power>::value>());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L2 norm for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \return The L2 norm of the given dense vector.
//
// This function computes the L2 norm of the given dense vector:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization
   const double l2 = norm( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) norm(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*dv, SqrAbs(), Noop(), Sqrt());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the squared L2 norm for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \return The squared L2 norm of the given dense vector.
//
// This function computes the squared L2 norm of the given dense vector:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization
   const double l2 = sqrNorm( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) sqrNorm(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*dv, SqrAbs(), Noop(), Noop());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L1 norm for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \return The L1 norm of the given dense vector.
//
// This function computes the L1 norm of the given dense vector:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization
   const double l1 = l1Norm( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) l1Norm(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*dv, Abs(), Noop(), Noop());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L2 norm for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \return The L2 norm of the given dense vector.
//
// This function computes the L2 norm of the given dense vector:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization
   const double l2 = l2Norm( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) l2Norm(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*dv, SqrAbs(), Noop(), Sqrt());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L3 norm for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \return The L3 norm of the given dense vector.
//
// This function computes the L3 norm of the given dense vector:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization
   const double l3 = l3Norm( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) l3Norm(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*dv, Abs(), Pow3(), Cbrt());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L4 norm for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \return The L4 norm of the given dense vector.
//
// This function computes the L4 norm of the given dense vector:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization
   const double l4 = l4Norm( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) l4Norm(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*dv, SqrAbs(), Pow2(), Qdrt());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the Lp norm for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \param p The norm parameter (p > 0).
// \return The Lp norm of the given dense vector.
//
// This function computes the Lp norm of the given dense vector, where the norm is specified by
// the runtime argument \a p:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization
   const double lp = lpNorm( a, 2.3 );
   \endcode

// \note The norm parameter \a p is expected to be larger than 0. This precondition is only checked
// by a user assertion.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename ST> // Type of the norm parameter
inline decltype(auto) lpNorm(const DenseVector<VT, TF> &dv, ST p)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_USER_ASSERT(!isZero(p), "Invalid p for Lp norm detected");

	using ScalarType = MultTrait_t<UnderlyingBuiltin_t<VT>, decltype(inv(p))>;
	using UnaryPow = Bind2nd<Pow, ScalarType>;
	return norm_backend(*dv, Abs(), UnaryPow(Pow(), p), UnaryPow(Pow(), inv(p)));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the Lp norm for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \return The Lp norm of the given dense vector.
//
// This function computes the Lp norm of the given dense vector, where the norm is specified by
// the runtime argument \a P:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization
   const double lp = lpNorm<2>( a );
   \endcode

// \note The norm parameter \a P is expected to be larger than 0. A value of 0 results in a
// compile time error!.
*/
template <size_t P // Compile time norm parameter
		  ,
		  typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) lpNorm(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_STATIC_ASSERT_MSG(P > 0UL, "Invalid norm parameter detected");

	using Norms = TypeList<L1Norm, L2Norm, L3Norm, L4Norm, LpNorm<P>>;
	using Norm = typename TypeAt<Norms, min(P - 1UL, 4UL)>::Type;

	return Norm()(*dv);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the infinity norm for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \return The infinity norm of the given dense vector.
//
// This function computes the infinity norm of the given dense vector:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization
   const double linf = linfNorm( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) linfNorm(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return max(abs(*dv));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the maximum norm for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \return The maximum norm of the given dense vector.
//
// This function computes the maximum norm of the given dense vector:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization
   const double max = maxNorm( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) maxNorm(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return linfNorm(*dv);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the minimum norm for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the norm computation.
// \return The minimum norm of the given dense vector.
//
// This function computes the minimum norm of the given dense vector:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization
   const double min = minNorm( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) minNorm(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return min(abs(*dv));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Calculation of the square length (magnitude) of the dense vector \f$|\vec{a}|^2\f$.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return The square length (magnitude) of the dense vector.
//
// This function calculates the actual square length (magnitude) of the dense vector. The
// function has the same effect as calling the \a sqrNorm() function on the dense vector.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) sqrLength(const DenseVector<VT, TF> &dv)
{
	return sqrNorm(*dv);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Calculation of the length (magnitude) of the dense vector \f$|\vec{a}|\f$.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return The length (magnitude) of the dense vector.
//
// This function calculates the actual length (magnitude) of the dense vector. The function has
// the same effect as calling the \a norm() function on the dense vector.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) length(const DenseVector<VT, TF> &dv)
{
	return norm(*dv);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
