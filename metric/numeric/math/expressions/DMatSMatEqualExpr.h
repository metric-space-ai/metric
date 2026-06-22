// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DMATSMATEQUALEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DMATSMATEQUALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/expressions/SparseMatrix.h>
#include <metric/numeric/math/shims/Equal.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL BINARY RELATIONAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality operator for the comparison of a dense matrix and a row-major sparse matrix.
// \ingroup dense_matrix
//
// \param lhs The left-hand side dense matrix for the comparison.
// \param rhs The right-hand side row-major sparse matrix for the comparison.
// \return \a true if the two matrices are equal, \a false if not.
//
// Equal function for the comparison of a dense matrix and a sparse matrix. Due to the limited
// machine accuracy, a direct comparison of two floating point numbers should be avoided. This
// function offers the possibility to compare two floating-point matrices with a certain accuracy
// margin.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename MT1 // Type of the left-hand side dense matrix
		  ,
		  bool SO // Storage order of the left-hand side dense matrix
		  ,
		  typename MT2> // Type of the right-hand side sparse matrix
inline bool equal(const DenseMatrix<MT1, SO> &lhs, const SparseMatrix<MT2, false> &rhs)
{
	using CT1 = CompositeType_t<MT1>;
	using CT2 = CompositeType_t<MT2>;

	// Early exit in case the matrix sizes don't match
	if ((*lhs).rows() != (*rhs).rows() || (*lhs).columns() != (*rhs).columns())
		return false;

	// Evaluation of the dense matrix and sparse matrix operand
	CT1 A(*lhs);
	CT2 B(*rhs);

	// In order to compare the two matrices, the data values of the lower-order data
	// type are converted to the higher-order data type within the equal function.
	size_t j(0UL);

	for (size_t i = 0UL; i < B.rows(); ++i) {
		j = 0UL;
		for (auto element = B.begin(i); element != B.end(i); ++element, ++j) {
			for (; j < element->index(); ++j) {
				if (!isDefault<RF>(A(i, j)))
					return false;
			}
			if (!equal<RF>(element->value(), A(i, j)))
				return false;
		}
		for (; j < A.columns(); ++j) {
			if (!isDefault<RF>(A(i, j)))
				return false;
		}
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality operator for the comparison of a dense matrix and a column-major sparse matrix.
// \ingroup dense_matrix
//
// \param lhs The left-hand side dense matrix for the comparison.
// \param rhs The right-hand side column-major sparse matrix for the comparison.
// \return \a true if the two matrices are equal, \a false if not.
//
// Equal function for the comparison of a dense matrix and a sparse matrix. Due to the limited
// machine accuracy, a direct comparison of two floating point numbers should be avoided. This
// function offers the possibility to compare two floating-point matrices with a certain accuracy
// margin.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename MT1 // Type of the left-hand side dense matrix
		  ,
		  bool SO // Storage order of the left-hand side dense matrix
		  ,
		  typename MT2> // Type of the right-hand side sparse matrix
inline bool equal(const DenseMatrix<MT1, SO> &lhs, const SparseMatrix<MT2, true> &rhs)
{
	using CT1 = CompositeType_t<MT1>;
	using CT2 = CompositeType_t<MT2>;

	// Early exit in case the matrix sizes don't match
	if ((*lhs).rows() != (*rhs).rows() || (*lhs).columns() != (*rhs).columns())
		return false;

	// Evaluation of the dense matrix and sparse matrix operand
	CT1 A(*lhs);
	CT2 B(*rhs);

	// In order to compare the two matrices, the data values of the lower-order data
	// type are converted to the higher-order data type within the equal function.
	size_t i(0UL);

	for (size_t j = 0UL; j < B.columns(); ++j) {
		i = 0UL;
		for (auto element = B.begin(j); element != B.end(j); ++element, ++i) {
			for (; i < element->index(); ++i) {
				if (!isDefault<RF>(A(i, j)))
					return false;
			}
			if (!equal<RF>(element->value(), A(i, j)))
				return false;
		}
		for (; i < A.rows(); ++i) {
			if (!isDefault<RF>(A(i, j)))
				return false;
		}
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Equality operator for the comparison of a sparse matrix and a dense matrix.
// \ingroup dense_matrix
//
// \param lhs The left-hand side sparse matrix for the comparison.
// \param rhs The right-hand side dense matrix for the comparison.
// \return \a true if the two matrices are equal, \a false if not.
//
// Equal function for the comparison of a sparse matrix and a dense matrix. Due to the limited
// machine accuracy, a direct comparison of two floating point numbers should be avoided. This
// function offers the possibility to compare two floating-point matrices with a certain accuracy
// margin.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename MT1 // Type of the left-hand side sparse matrix
		  ,
		  bool SO1 // Storage order of the left-hand side sparse matrix
		  ,
		  typename MT2 // Type of the right-hand side dense matrix
		  ,
		  bool SO2> // Storage order of the right-hand side dense matrix
inline bool equal(const SparseMatrix<MT1, SO1> &lhs, const DenseMatrix<MT2, SO2> &rhs)
{
	return equal<RF>(rhs, lhs);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Equality operator for the comparison of a dense matrix and a sparse matrix.
// \ingroup dense_matrix
//
// \param lhs The left-hand side dense matrix for the comparison.
// \param rhs The right-hand side sparse matrix for the comparison.
// \return \a true if the two matrices are equal, \a false if not.
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  bool SO1 // Storage order of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SO2> // Storage order of the right-hand side dense matrix
inline bool operator==(const DenseMatrix<MT1, SO1> &lhs, const SparseMatrix<MT2, SO2> &rhs)
{
	return equal<relaxed>(lhs, rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Equality operator for the comparison of a sparse matrix and a dense matrix.
// \ingroup dense_matrix
//
// \param lhs The left-hand side sparse matrix for the comparison.
// \param rhs The right-hand side dense matrix for the comparison.
// \return \a true if the two matrices are equal, \a false if not.
*/
template <typename MT1 // Type of the left-hand side sparse matrix
		  ,
		  bool SO1 // Storage order of the left-hand side sparse matrix
		  ,
		  typename MT2 // Type of the right-hand side dense matrix
		  ,
		  bool SO2> // Storage order of the right-hand side dense matrix
inline bool operator==(const SparseMatrix<MT1, SO1> &lhs, const DenseMatrix<MT2, SO2> &rhs)
{
	return equal<relaxed>(rhs, lhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inequality operator for the comparison of a dense matrix and a sparse matrix.
// \ingroup dense_matrix
//
// \param lhs The left-hand side dense matrix for the comparison.
// \param rhs The right-hand side sparse matrix for the comparison.
// \return \a true if the two matrices are not equal, \a false if they are equal.
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  bool SO1 // Storage order of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SO2> // Storage order of the right-hand side sparse matrix
inline bool operator!=(const DenseMatrix<MT1, SO1> &lhs, const SparseMatrix<MT2, SO2> &rhs)
{
	return !equal(lhs, rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inequality operator for the comparison of a sparse matrix and a dense matrix.
// \ingroup dense_matrix
//
// \param lhs The left-hand side sparse matrix for the comparison.
// \param rhs The right-hand side dense matrix for the comparison.
// \return \a true if the two matrices are not equal, \a false if they are equal.
*/
template <typename MT1 // Type of the left-hand side sparse matrix
		  ,
		  bool SO1 // Storage order of the left-hand side sparse matrix
		  ,
		  typename MT2 // Type of the right-hand side dense matrix
		  ,
		  bool SO2> // Storage order right-hand side dense matrix
inline bool operator!=(const SparseMatrix<MT1, SO1> &lhs, const DenseMatrix<MT2, SO2> &rhs)
{
	return !equal(rhs, lhs);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
