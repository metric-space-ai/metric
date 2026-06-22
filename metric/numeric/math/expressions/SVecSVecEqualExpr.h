// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECSVECEQUALEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECSVECEQUALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/shims/Equal.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/system/MacroDisable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL BINARY RELATIONAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check of two sparse vectors.
// \ingroup sparse_vector
//
// \param a The left-hand side sparse vector for the comparison.
// \param b The right-hand side sparse vector for the comparison.
// \return \a true if the two vectors are equal, \a false if not.
//
// Equal function for the comparison of two sparse vectors. Due to the limited machine accuracy,
// a direct comparison of two floating point numbers should be avoided. This function offers the
// possibility to compare two floating-point vectors with a certain accuracy margin.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename VT1 // Type of the left-hand side sparse vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side sparse vector
		  ,
		  typename VT2 // Type of the right-hand side sparse vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side sparse vector
inline bool equal(const SparseVector<VT1, TF1> &lhs, const SparseVector<VT2, TF2> &rhs)
{
	using CT1 = CompositeType_t<VT1>;
	using CT2 = CompositeType_t<VT2>;

	// Early exit in case the vector sizes don't match
	if ((*lhs).size() != (*rhs).size())
		return false;

	// Evaluation of the two sparse vector operands
	CT1 a(*lhs);
	CT2 b(*rhs);

	// In order to compare the two vectors, the data values of the lower-order data
	// type are converted to the higher-order data type within the equal function.
	const auto lend(a.end());
	const auto rend(b.end());

	auto lelem(a.begin());
	auto relem(b.begin());

	while (lelem != lend && relem != rend) {
		if (isDefault<RF>(lelem->value())) {
			++lelem;
			continue;
		}
		if (isDefault<RF>(relem->value())) {
			++relem;
			continue;
		}

		if (lelem->index() != relem->index() || !equal<RF>(lelem->value(), relem->value())) {
			return false;
		} else {
			++lelem;
			++relem;
		}
	}

	while (lelem != lend) {
		if (!isDefault<RF>(lelem->value()))
			return false;
		++lelem;
	}

	while (relem != rend) {
		if (!isDefault<RF>(relem->value()))
			return false;
		++relem;
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Equality operator for the comparison of two sparse vectors.
// \ingroup sparse_vector
//
// \param lhs The left-hand side sparse vector for the comparison.
// \param rhs The right-hand side sparse vector for the comparison.
// \return \a true if the two sparse vectors are equal, \a false if not.
*/
template <typename VT1 // Type of the left-hand side sparse vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side sparse vector
		  ,
		  typename VT2 // Type of the right-hand side sparse vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side sparse vector
inline bool operator==(const SparseVector<VT1, TF1> &lhs, const SparseVector<VT2, TF2> &rhs)
{
	return equal<relaxed>(lhs, rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inequality operator for the comparison of two sparse vectors.
// \ingroup sparse_vector
//
// \param lhs The left-hand side sparse vector for the comparison.
// \param rhs The right-hand side sparse vector for the comparison.
// \return \a true if the two vectors are not equal, \a false if they are equal.
*/
template <typename VT1 // Type of the left-hand side sparse vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side sparse vector
		  ,
		  typename VT2 // Type of the right-hand side sparse vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side sparse vector
inline bool operator!=(const SparseVector<VT1, TF1> &lhs, const SparseVector<VT2, TF2> &rhs)
{
	return !equal<relaxed>(lhs, rhs);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
