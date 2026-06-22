// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DMATNORMEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DMATNORMEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/SIMD.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
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
/*!\brief Auxiliary helper struct for the dense matrix norms.
// \ingroup dense_matrix
*/
template <typename MT // Type of the dense matrix
		  ,
		  typename Abs // Type of the abs operation
		  ,
		  typename Power> // Type of the power operation
struct DMatNormHelper {
	//**Type definitions****************************************************************************
	//! Element type of the dense matrix expression.
	using ET = ElementType_t<MT>;

	//! Composite type of the dense matrix expression.
	using CT = RemoveReference_t<CompositeType_t<MT>>;
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
/*!\brief Default backend implementation of the norm of a row-major dense matrix.
// \ingroup dense_matrix
//
// \param dm The given row-major dense matrix for the norm computation.
// \param abs The functor for the abs operation.
// \param power The functor for the power operation.
// \param root The functor for the root operation.
// \return The norm of the given matrix.
//
// This function implements the performance optimized norm of a row-major dense matrix. Due to
// the explicit application of the SFINAE principle, this function can only be selected by the
// compiler in case vectorization cannot be applied.
*/
template <typename MT // Type of the dense matrix
		  ,
		  typename Abs // Type of the abs opertaion
		  ,
		  typename Power // Type of the power operation
		  ,
		  typename Root> // Type of the root operation
inline decltype(auto) norm_backend(const DenseMatrix<MT, false> &dm, Abs abs, Power power, Root root, FalseType)
{
	using CT = CompositeType_t<MT>;
	using ET = ElementType_t<MT>;
	using PT = RemoveCVRef_t<decltype(power(abs(std::declval<ET>())))>;
	using RT = RemoveCVRef_t<decltype(evaluate(root(std::declval<PT>())))>;

	if ((*dm).rows() == 0UL || (*dm).columns() == 0UL)
		return RT{};

	CT tmp(*dm);

	const size_t M(tmp.rows());
	const size_t N(tmp.columns());

	PT norm(power(abs(tmp(0UL, 0UL))));

	{
		size_t j(1UL);

		for (; (j + 4UL) <= N; j += 4UL) {
			norm += power(abs(tmp(0UL, j))) + power(abs(tmp(0UL, j + 1UL))) + power(abs(tmp(0UL, j + 2UL))) +
					power(abs(tmp(0UL, j + 3UL)));
		}
		for (; (j + 2UL) <= N; j += 2UL) {
			norm += power(abs(tmp(0UL, j))) + power(abs(tmp(0UL, j + 1UL)));
		}
		for (; j < N; ++j) {
			norm += power(abs(tmp(0UL, j)));
		}
	}

	for (size_t i = 1UL; i < M; ++i) {
		size_t j(0UL);

		for (; (j + 4UL) <= N; j += 4UL) {
			norm += power(abs(tmp(i, j))) + power(abs(tmp(i, j + 1UL))) + power(abs(tmp(i, j + 2UL))) +
					power(abs(tmp(i, j + 3UL)));
		}
		for (; (j + 2UL) <= N; j += 2UL) {
			norm += power(abs(tmp(i, j))) + power(abs(tmp(i, j + 1UL)));
		}
		for (; j < N; ++j) {
			norm += power(abs(tmp(i, j)));
		}
	}

	return evaluate(root(norm));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default backend implementation of the norm of a column-major dense matrix.
// \ingroup dense_matrix
//
// \param dm The given column-major dense matrix for the norm computation.
// \param abs The functor for the abs operation.
// \param power The functor for the power operation.
// \param root The functor for the root operation.
// \return The norm of the given matrix.
//
// This function implements the performance optimized norm of a column-major dense matrix. Due
// to the explicit application of the SFINAE principle, this function can only be selected by
// the compiler in case vectorization cannot be applied.
*/
template <typename MT // Type of the dense matrix
		  ,
		  typename Abs // Type of the abs operation
		  ,
		  typename Power // Type of the power operation
		  ,
		  typename Root> // Type of the root operation
inline decltype(auto) norm_backend(const DenseMatrix<MT, true> &dm, Abs abs, Power power, Root root, FalseType)
{
	using CT = CompositeType_t<MT>;
	using ET = ElementType_t<MT>;
	using PT = RemoveCVRef_t<decltype(power(abs(std::declval<ET>())))>;
	using RT = RemoveCVRef_t<decltype(evaluate(root(std::declval<PT>())))>;

	if ((*dm).rows() == 0UL || (*dm).columns() == 0UL)
		return RT{};

	CT tmp(*dm);

	const size_t M(tmp.rows());
	const size_t N(tmp.columns());

	PT norm(power(abs(tmp(0UL, 0UL))));

	{
		size_t i(1UL);

		for (; (i + 4UL) <= M; i += 4UL) {
			norm += power(abs(tmp(i, 0UL))) + power(abs(tmp(i + 1UL, 0UL))) + power(abs(tmp(i + 2UL, 0UL))) +
					power(abs(tmp(i + 3UL, 0UL)));
		}
		for (; (i + 2UL) <= M; i += 2UL) {
			norm += power(abs(tmp(i, 0UL))) + power(abs(tmp(i + 1UL, 0UL)));
		}
		for (; i < M; ++i) {
			norm += power(abs(tmp(i, 0UL)));
		}
	}

	for (size_t j = 1UL; j < N; ++j) {
		size_t i(0UL);

		for (; (i + 4UL) <= M; i += 4UL) {
			norm += power(abs(tmp(i, j))) + power(abs(tmp(i + 1UL, j))) + power(abs(tmp(i + 2UL, j))) +
					power(abs(tmp(i + 3UL, j)));
		}
		for (; (i + 2UL) <= M; i += 2UL) {
			norm += power(abs(tmp(i, j))) + power(abs(tmp(i + 1UL, j)));
		}
		for (; i < M; ++i) {
			norm += power(abs(tmp(i, j)));
		}
	}

	return evaluate(root(norm));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief SIMD optimized backend implementation of the norm of a row-major dense matrix.
// \ingroup dense_matrix
//
// \param dm The given row-major dense matrix for the norm computation.
// \param abs The functor for the abs operation.
// \param power The functor for the power operation.
// \param root The functor for the root operation.
// \return The norm of the given matrix.
//
// This function implements the performance optimized norm of a row-major dense matrix. Due to
// the explicit application of the SFINAE principle, this function can only be selected by the
// compiler in case vectorization can be applied.
*/
template <typename MT // Type of the dense matrix
		  ,
		  typename Abs // Type of the abs operation
		  ,
		  typename Power // Type of the power operation
		  ,
		  typename Root> // Type of the root operation
inline decltype(auto) norm_backend(const DenseMatrix<MT, false> &dm, Abs abs, Power power, Root root, TrueType)
{
	using CT = CompositeType_t<MT>;
	using ET = ElementType_t<MT>;
	using RT = decltype(evaluate(root(std::declval<ET>())));

	static constexpr size_t SIMDSIZE = SIMDTrait<ET>::size;

	if ((*dm).rows() == 0UL || (*dm).columns() == 0UL)
		return RT{};

	CT tmp(*dm);

	const size_t M(tmp.rows());
	const size_t N(tmp.columns());

	constexpr bool remainder(!IsPadded_v<RemoveReference_t<CT>>);

	ET norm{};

	if (!remainder || N >= SIMDSIZE) {
		const size_t jpos(remainder ? prevMultiple(N, SIMDSIZE) : N);
		METRIC_NUMERIC_INTERNAL_ASSERT(jpos <= N, "Invalid end calculation");

		SIMDTrait_t<ET> xmm1{};
		size_t i(0UL);

		for (; (i + 4UL) <= M; i += 4UL) {
			xmm1 += power(abs(tmp.load(i, 0UL)));
			SIMDTrait_t<ET> xmm2(power(abs(tmp.load(i + 1UL, 0UL))));
			SIMDTrait_t<ET> xmm3(power(abs(tmp.load(i + 2UL, 0UL))));
			SIMDTrait_t<ET> xmm4(power(abs(tmp.load(i + 3UL, 0UL))));
			size_t j(SIMDSIZE);

			for (; j < jpos; j += SIMDSIZE) {
				xmm1 += power(abs(tmp.load(i, j)));
				xmm2 += power(abs(tmp.load(i + 1UL, j)));
				xmm3 += power(abs(tmp.load(i + 2UL, j)));
				xmm4 += power(abs(tmp.load(i + 3UL, j)));
			}
			for (; remainder && j < N; ++j) {
				norm += power(abs(tmp(i, j)));
				norm += power(abs(tmp(i + 1UL, j)));
				norm += power(abs(tmp(i + 2UL, j)));
				norm += power(abs(tmp(i + 3UL, j)));
			}

			xmm1 += xmm2;
			xmm3 += xmm4;
			xmm1 += xmm3;
		}

		if (i + 2UL <= M) {
			xmm1 += power(abs(tmp.load(i, 0UL)));
			SIMDTrait_t<ET> xmm2(power(abs(tmp.load(i + 1UL, 0UL))));
			size_t j(SIMDSIZE);

			for (; j < jpos; j += SIMDSIZE) {
				xmm1 += power(abs(tmp.load(i, j)));
				xmm2 += power(abs(tmp.load(i + 1UL, j)));
			}
			for (; remainder && j < N; ++j) {
				norm += power(abs(tmp(i, j)));
				norm += power(abs(tmp(i + 1UL, j)));
			}

			xmm1 += xmm2;

			i += 2UL;
		}

		if (i < M) {
			xmm1 += power(abs(tmp.load(i, 0UL)));
			size_t j(SIMDSIZE);

			for (; j < jpos; j += SIMDSIZE) {
				xmm1 += power(abs(tmp.load(i, j)));
			}
			for (; remainder && j < N; ++j) {
				norm += power(abs(tmp(i, j)));
			}
		}

		norm += sum(xmm1);
	} else {
		for (size_t i = 0UL; i < M; ++i) {
			for (size_t j = 0UL; j < N; ++j) {
				norm += power(abs(tmp(i, j)));
			}
		}
	}

	return evaluate(root(norm));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief SIMD optimized backend implementation of the norm of a column-major dense matrix.
// \ingroup dense_matrix
//
// \param dm The given column-major dense matrix for the norm computation.
// \param abs The functor for the abs operation.
// \param power The functor for the power operation.
// \param root The functor for the root operation.
// \return The norm of the given matrix.
//
// This function implements the performance optimized norm of a column-major dense matrix. Due
// to the explicit application of the SFINAE principle, this function can only be selected by
// the compiler in case vectorization can be applied.
*/
template <typename MT // Type of the dense matrix
		  ,
		  typename Abs // Type of the abs operation
		  ,
		  typename Power // Type of the power operation
		  ,
		  typename Root> // Type of the root operation
inline decltype(auto) norm_backend(const DenseMatrix<MT, true> &dm, Abs abs, Power power, Root root, TrueType)
{
	using CT = CompositeType_t<MT>;
	using ET = ElementType_t<MT>;
	using RT = decltype(evaluate(root(std::declval<ET>())));

	static constexpr size_t SIMDSIZE = SIMDTrait<ET>::size;

	if ((*dm).rows() == 0UL || (*dm).columns() == 0UL)
		return RT{};

	CT tmp(*dm);

	const size_t M(tmp.rows());
	const size_t N(tmp.columns());

	constexpr bool remainder(!IsPadded_v<RemoveReference_t<CT>>);

	ET norm{};

	if (!remainder || N >= SIMDSIZE) {
		const size_t ipos(remainder ? prevMultiple(M, SIMDSIZE) : M);
		METRIC_NUMERIC_INTERNAL_ASSERT(ipos <= M, "Invalid end calculation");

		SIMDTrait_t<ET> xmm1{};
		size_t j(0UL);

		for (; (j + 4UL) <= N; j += 4UL) {
			xmm1 += power(abs(tmp.load(0UL, j)));
			SIMDTrait_t<ET> xmm2(power(abs(tmp.load(0UL, j + 1UL))));
			SIMDTrait_t<ET> xmm3(power(abs(tmp.load(0UL, j + 2UL))));
			SIMDTrait_t<ET> xmm4(power(abs(tmp.load(0UL, j + 3UL))));
			size_t i(SIMDSIZE);

			for (; i < ipos; i += SIMDSIZE) {
				xmm1 += power(abs(tmp.load(i, j)));
				xmm2 += power(abs(tmp.load(i, j + 1UL)));
				xmm3 += power(abs(tmp.load(i, j + 2UL)));
				xmm4 += power(abs(tmp.load(i, j + 3UL)));
			}
			for (; remainder && i < M; ++i) {
				norm += power(abs(tmp(i, j)));
				norm += power(abs(tmp(i, j + 1UL)));
				norm += power(abs(tmp(i, j + 2UL)));
				norm += power(abs(tmp(i, j + 3UL)));
			}

			xmm1 += xmm2;
			xmm3 += xmm4;
			xmm1 += xmm3;
		}

		if (j + 2UL <= N) {
			xmm1 += power(abs(tmp.load(0UL, j)));
			SIMDTrait_t<ET> xmm2(power(abs(tmp.load(0UL, j + 1UL))));
			size_t i(SIMDSIZE);

			for (; i < ipos; i += SIMDSIZE) {
				xmm1 += power(abs(tmp.load(i, j)));
				xmm2 += power(abs(tmp.load(i, j + 1UL)));
			}
			for (; remainder && i < M; ++i) {
				norm += power(abs(tmp(i, j)));
				norm += power(abs(tmp(i, j + 1UL)));
			}

			xmm1 += xmm2;

			j += 2UL;
		}

		if (j < N) {
			xmm1 += power(abs(tmp.load(0UL, j)));
			size_t i(SIMDSIZE);

			for (; i < ipos; i += SIMDSIZE) {
				xmm1 += power(abs(tmp.load(i, j)));
			}
			for (; remainder && i < M; ++i) {
				norm += power(abs(tmp(i, j)));
			}
		}

		norm += sum(xmm1);
	} else {
		for (size_t j = 0UL; j < N; ++j) {
			for (size_t i = 0UL; i < M; ++i) {
				norm += power(abs(tmp(i, j)));
			}
		}
	}

	return evaluate(root(norm));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Computes a custom norm for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the norm computation.
// \param abs The functor for the abs operation.
// \param power The functor for the power operation.
// \param root The functor for the root operation.
// \return The norm of the given dense matrix.
//
// This function computes a custom norm of the given dense matrix by means of the given functors.
// The following example demonstrates the computation of the L2 norm by means of the mtrc::numeric::Pow2
// and mtrc::numeric::Sqrt functors:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   // ... Resizing and initialization
   const double l2 = norm( A, mtrc::numeric::Pow2(), mtrc::numeric::Sqrt() );
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO // Storage order
		  ,
		  typename Abs // Type of the abs operation
		  ,
		  typename Power // Type of the power operation
		  ,
		  typename Root> // Type of the root operation
decltype(auto) norm_backend(const DenseMatrix<MT, SO> &dm, Abs abs, Power power, Root root)
{
	return norm_backend(*dm, abs, power, root, Bool_t<DMatNormHelper<MT, Abs, Power>::value>());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L2 norm for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the norm computation.
// \return The L2 norm of the given dense matrix.
//
// This function computes the L2 norm of the given dense matrix:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   // ... Resizing and initialization
   const double l2 = norm( A );
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
decltype(auto) norm(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*dm, SqrAbs(), Noop(), Sqrt());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the squared L2 norm for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the norm computation.
// \return The squared L2 norm of the given dense matrix.
//
// This function computes the squared L2 norm of the given dense matrix:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   // ... Resizing and initialization
   const double l2 = sqrNorm( A );
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
decltype(auto) sqrNorm(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*dm, SqrAbs(), Noop(), Noop());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L1 norm for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the norm computation.
// \return The L1 norm of the given dense matrix.
//
// This function computes the L1 norm of the given dense matrix:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   // ... Resizing and initialization
   const double l1 = l1Norm( A );
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
decltype(auto) l1Norm(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*dm, Abs(), Noop(), Noop());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L2 norm for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the norm computation.
// \return The L2 norm of the given dense matrix.
//
// This function computes the L2 norm of the given dense matrix:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   // ... Resizing and initialization
   const double l2 = l2Norm( A );
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
decltype(auto) l2Norm(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*dm, SqrAbs(), Noop(), Sqrt());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L3 norm for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the norm computation.
// \return The L3 norm of the given dense matrix.
//
// This function computes the L3 norm of the given dense matrix:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   // ... Resizing and initialization
   const double l3 = l3Norm( A );
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
decltype(auto) l3Norm(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*dm, Abs(), Pow3(), Cbrt());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L4 norm for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the norm computation.
// \return The L4 norm of the given dense matrix.
//
// This function computes the L4 norm of the given dense matrix:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   // ... Resizing and initialization
   const double l4 = l4Norm( A );
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
decltype(auto) l4Norm(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*dm, SqrAbs(), Pow2(), Qdrt());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the Lp norm for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the norm computation.
// \param p The norm parameter (p > 0).
// \return The Lp norm of the given dense matrix.
//
// This function computes the Lp norm of the given dense matrix, where the norm is specified by
// the runtime argument \a p:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   // ... Resizing and initialization
   const double lp = lpNorm( A, 2.3 );
   \endcode

// \note The norm parameter \a p is expected to be larger than 0. This precondition is only checked
// by a user assertion.
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO // Storage order
		  ,
		  typename ST> // Type of the norm parameter
decltype(auto) lpNorm(const DenseMatrix<MT, SO> &dm, ST p)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_USER_ASSERT(!isZero(p), "Invalid p for Lp norm detected");

	using ScalarType = MultTrait_t<UnderlyingBuiltin_t<MT>, decltype(inv(p))>;
	using UnaryPow = Bind2nd<Pow, ScalarType>;
	return norm_backend(*dm, Abs(), UnaryPow(Pow(), p), UnaryPow(Pow(), inv(p)));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the Lp norm for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the norm computation.
// \return The Lp norm of the given dense matrix.
//
// This function computes the Lp norm of the given dense matrix, where the norm is specified by
// the runtime argument \a P:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   // ... Resizing and initialization
   const double lp = lpNorm<2>( A );
   \endcode

// \note The norm parameter \a P is expected to be larger than 0. A value of 0 results in a
// compile time error!.
*/
template <size_t P // Compile time norm parameter
		  ,
		  typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) lpNorm(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_STATIC_ASSERT_MSG(P > 0UL, "Invalid norm parameter detected");

	using Norms = TypeList<L1Norm, L2Norm, L3Norm, L4Norm, LpNorm<P>>;
	using Norm = typename TypeAt<Norms, min(P - 1UL, 4UL)>::Type;

	return Norm()(*dm);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the infinity norm for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the norm computation.
// \return The infinity norm of the given dense matrix.
//
// This function computes the infinity norm of the given dense matrix:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   // ... Resizing and initialization
   const double linf = linfNorm( A );
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
decltype(auto) linfNorm(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return max(abs(*dm));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the maximum norm for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the norm computation.
// \return The maximum norm of the given dense matrix.
//
// This function computes the maximum norm of the given dense matrix:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   // ... Resizing and initialization
   const double max = maxNorm( A );
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
decltype(auto) maxNorm(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return linfNorm(*dm);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the minimum norm for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the norm computation.
// \return The minimum norm of the given dense matrix.
//
// This function computes the minimum norm of the given dense matrix:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   // ... Resizing and initialization
   const double min = minNorm( A );
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
decltype(auto) minNorm(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return min(abs(*dm));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
