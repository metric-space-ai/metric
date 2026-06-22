// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECSVECINNEREXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECSVECINNEREXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/ColumnVector.h>
#include <metric/numeric/math/constraints/RowVector.h>
#include <metric/numeric/math/constraints/SparseVector.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/traits/MultTrait.h>
#include <metric/numeric/math/typetraits/IsOpposedView.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Multiplication operator for the scalar product (inner product) of two sparse vectors
//        (\f$ s=\vec{a}*\vec{b} \f$).
// \ingroup sparse_vector
//
// \param lhs The left-hand side sparse vector for the inner product.
// \param rhs The right-hand side sparse vector for the inner product.
// \return The scalar product.
// \exception std::invalid_argument Vector sizes do not match.
//
// This operator represents the scalar product (inner product) of two sparse vectors:

   \code
   using mtrc::numeric::columnVector;

   mtrc::numeric::CompressedVector<double,columnVector> a, b;
   mtrc::numeric::real res;
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
template <typename VT1 // Type of the left-hand side sparse vector
		  ,
		  typename VT2> // Type of the right-hand side sparse vector
inline decltype(auto) operator*(const SparseVector<VT1, true> &lhs, const SparseVector<VT2, false> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using CT1 = CompositeType_t<VT1>;		// Composite type of the left-hand side sparse vector expression
	using CT2 = CompositeType_t<VT2>;		// Composite type of the right-hand side sparse vector expression
	using XT1 = RemoveReference_t<CT1>;		// Auxiliary type for the left-hand side composite type
	using XT2 = RemoveReference_t<CT2>;		// Auxiliary type for the right-hand side composite type
	using ET1 = ElementType_t<XT1>;			// Element type of the left-hand side sparse vector expression
	using ET2 = ElementType_t<XT2>;			// Element type of the right-hand side sparse vector expression
	using MultType = MultTrait_t<ET1, ET2>; // Multiplication result typen

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(VT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(VT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_VECTOR_TYPE(VT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE(VT2);

	if ((*lhs).size() != (*rhs).size()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Vector sizes do not match");
	}

	CT1 left(*lhs);	 // Evaluation of the left-hand side sparse vector operand
	CT2 right(*rhs); // Evaluation of the right-hand side sparse vector operand

	METRIC_NUMERIC_INTERNAL_ASSERT(left.size() == (*lhs).size(), "Invalid vector size");
	METRIC_NUMERIC_INTERNAL_ASSERT(right.size() == (*rhs).size(), "Invalid vector size");

	MultType sp{};

	if (IsOpposedView_v<VT1> && IsOpposedView_v<VT2>) {
		if (left.size() == 0UL)
			return sp;

		sp = left[0UL] * right[0UL];
		for (size_t i = 1UL; i < left.size(); ++i) {
			sp += left[i] * right[i];
		}
	} else if (IsOpposedView_v<VT1>) {
		const auto rend(right.end());
		auto r(right.begin());

		if (r == rend)
			return sp;

		sp = left[r->index()] * r->value();
		++r;
		for (; r != rend; ++r) {
			sp += left[r->index()] * r->value();
		}
	} else if (IsOpposedView_v<VT2>) {
		const auto lend(left.end());
		auto l(left.begin());

		if (l == lend)
			return sp;

		sp = l->value() * right[l->index()];
		++l;
		for (; l != lend; ++l) {
			sp += l->value() * right[l->index()];
		}
	} else {
		const auto lend(left.end());
		const auto rend(right.end());
		auto l(left.begin());
		auto r(right.begin());

		if (l == lend || r == rend)
			return sp;

		while (true) {
			if (l->index() < r->index()) {
				++l;
				if (l == lend)
					break;
			} else if (r->index() < l->index()) {
				++r;
				if (r == rend)
					break;
			} else {
				sp = l->value() * r->value();
				++l;
				++r;
				break;
			}
		}

		if (l != lend && r != rend) {
			while (true) {
				if (l->index() < r->index()) {
					++l;
					if (l == lend)
						break;
				} else if (r->index() < l->index()) {
					++r;
					if (r == rend)
						break;
				} else {
					sp += l->value() * r->value();
					++l;
					if (l == lend)
						break;
					++r;
					if (r == rend)
						break;
				}
			}
		}
	}

	return sp;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
