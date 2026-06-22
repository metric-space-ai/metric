// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DVECSVECEQUALEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DVECSVECEQUALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/shims/Equal.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL BINARY RELATIONAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check of a dense vector and a sparse vector.
// \ingroup dense_vector
//
// \param a The left-hand side dense vector for the comparison.
// \param b The right-hand side sparse vector for the comparison.
// \return \a true if the two vectors are equal, \a false if not.
//
// Equal function for the comparison of a dense vector and a sparse vector. Due to the limited
// machine accuracy, a direct comparison of two floating point numbers should be avoided. This
// function offers the possibility to compare two floating-point vectors with a certain accuracy
// margin.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename VT1 // Type of the left-hand side dense vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side dense vector
		  ,
		  typename VT2 // Type of the right-hand side sparse vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side sparse vector
inline bool equal(const DenseVector<VT1, TF1> &lhs, const SparseVector<VT2, TF2> &rhs)
{
	using CT1 = CompositeType_t<VT1>;
	using CT2 = CompositeType_t<VT2>;

	// Early exit in case the vector sizes don't match
	if ((*lhs).size() != (*rhs).size())
		return false;

	// Evaluation of the dense vector and sparse vector operand
	CT1 a(*lhs);
	CT2 b(*rhs);

	// In order to compare the two vectors, the data values of the lower-order data
	// type are converted to the higher-order data type within the equal function.
	size_t i(0UL);

	for (auto element = b.begin(); element != b.end(); ++element, ++i) {
		for (; i < element->index(); ++i) {
			if (!isDefault<RF>(a[i]))
				return false;
		}
		if (!equal<RF>(element->value(), a[i]))
			return false;
	}
	for (; i < a.size(); ++i) {
		if (!isDefault<RF>(a[i]))
			return false;
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality check of a sparse vector and a dense vector.
// \ingroup dense_vector
//
// \param a The left-hand side sparse vector for the comparison.
// \param b The right-hand side dense vector for the comparison.
// \return \a true if the two vectors are equal, \a false if not.
//
// Equal function for the comparison of a sparse vector and a dense vector. Due to the limited
// machine accuracy, a direct comparison of two floating point numbers should be avoided. This
// function offers the possibility to compare two floating-point vectors with a certain accuracy
// margin.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename VT1 // Type of the left-hand side sparse vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side sparse vector
		  ,
		  typename VT2 // Type of the right-hand side dense vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side dense vector
inline bool equal(const SparseVector<VT1, TF1> &lhs, const DenseVector<VT2, TF2> &rhs)
{
	return equal<RF>(rhs, lhs);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Equality operator for the comparison of a dense vector and a sparse vector.
// \ingroup dense_vector
//
// \param lhs The left-hand side dense vector for the comparison.
// \param rhs The right-hand side sparse vector for the comparison.
// \return \a true if the two vectors are equal, \a false if not.
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side dense vector
		  ,
		  typename VT2 // Type of the right-hand side sparse vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side sparse vector
inline bool operator==(const DenseVector<VT1, TF1> &lhs, const SparseVector<VT2, TF2> &rhs)
{
	return equal<relaxed>(lhs, rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Equality operator for the comparison of a sparse vector and a dense vector.
// \ingroup dense_vector
//
// \param lhs The left-hand side sparse vector for the comparison.
// \param rhs The right-hand side dense vector for the comparison.
// \return \a true if the two vectors are equal, \a false if not.
*/
template <typename VT1 // Type of the left-hand side sparse vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side sparse vector
		  ,
		  typename VT2 // Type of the right-hand side dense vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side dense vector
inline bool operator==(const SparseVector<VT1, TF1> &lhs, const DenseVector<VT2, TF2> &rhs)
{
	return equal<relaxed>(rhs, lhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inequality operator for the comparison of a dense vector and a sparse vector.
// \ingroup dense_vector
//
// \param lhs The left-hand side dense vector for the comparison.
// \param rhs The right-hand side sparse vector for the comparison.
// \return \a true if the two vectors are not equal, \a false if they are equal.
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side dense vector
		  ,
		  typename VT2 // Type of the right-hand side sparse vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side sparse vector
inline bool operator!=(const DenseVector<VT1, TF1> &lhs, const SparseVector<VT2, TF2> &rhs)
{
	return !equal<relaxed>(lhs, rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inequality operator for the comparison of a sparse vector and a dense vector.
// \ingroup dense_vector
//
// \param lhs The left-hand side sparse vector for the comparison.
// \param rhs The right-hand side dense vector for the comparison.
// \return \a true if the two vectors are not equal, \a false if they are equal.
*/
template <typename VT1 // Type of the left-hand side sparse vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side sparse vector
		  ,
		  typename VT2 // Type of the right-hand side dense vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side dense vector
inline bool operator!=(const SparseVector<VT1, TF1> &lhs, const DenseVector<VT2, TF2> &rhs)
{
	return !equal<relaxed>(rhs, lhs);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
