// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ADAPTORS_DIAGONALMATRIX_H
#define METRIC_NUMERIC_MATH_ADAPTORS_DIAGONALMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/Forward.h>
#include <metric/numeric/math/InversionFlag.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/adaptors/diagonalmatrix/BaseTemplate.h>
#include <metric/numeric/math/adaptors/diagonalmatrix/Dense.h>
#include <metric/numeric/math/adaptors/diagonalmatrix/Sparse.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Hermitian.h>
#include <metric/numeric/math/constraints/Lower.h>
#include <metric/numeric/math/constraints/RequiresEvaluation.h>
#include <metric/numeric/math/constraints/Symmetric.h>
#include <metric/numeric/math/constraints/UniTriangular.h>
#include <metric/numeric/math/constraints/Upper.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/IsDivisor.h>
#include <metric/numeric/math/traits/AddTrait.h>
#include <metric/numeric/math/traits/DeclDiagTrait.h>
#include <metric/numeric/math/traits/DeclHermTrait.h>
#include <metric/numeric/math/traits/DeclLowTrait.h>
#include <metric/numeric/math/traits/DeclStrLowTrait.h>
#include <metric/numeric/math/traits/DeclStrUppTrait.h>
#include <metric/numeric/math/traits/DeclSymTrait.h>
#include <metric/numeric/math/traits/DeclUniLowTrait.h>
#include <metric/numeric/math/traits/DeclUniUppTrait.h>
#include <metric/numeric/math/traits/DeclUppTrait.h>
#include <metric/numeric/math/traits/DivTrait.h>
#include <metric/numeric/math/traits/KronTrait.h>
#include <metric/numeric/math/traits/MapTrait.h>
#include <metric/numeric/math/traits/MultTrait.h>
#include <metric/numeric/math/traits/SchurTrait.h>
#include <metric/numeric/math/traits/SubTrait.h>
#include <metric/numeric/math/traits/SubmatrixTrait.h>
#include <metric/numeric/math/typetraits/HasConstDataAccess.h>
#include <metric/numeric/math/typetraits/HighType.h>
#include <metric/numeric/math/typetraits/IsAdaptor.h>
#include <metric/numeric/math/typetraits/IsAligned.h>
#include <metric/numeric/math/typetraits/IsContiguous.h>
#include <metric/numeric/math/typetraits/IsDiagonal.h>
#include <metric/numeric/math/typetraits/IsHermitian.h>
#include <metric/numeric/math/typetraits/IsIdentity.h>
#include <metric/numeric/math/typetraits/IsLower.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsPadded.h>
#include <metric/numeric/math/typetraits/IsRestricted.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/math/typetraits/IsSquare.h>
#include <metric/numeric/math/typetraits/IsStrictlyLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/IsSymmetric.h>
#include <metric/numeric/math/typetraits/IsUniTriangular.h>
#include <metric/numeric/math/typetraits/IsUniform.h>
#include <metric/numeric/math/typetraits/IsUpper.h>
#include <metric/numeric/math/typetraits/IsZero.h>
#include <metric/numeric/math/typetraits/LowType.h>
#include <metric/numeric/math/typetraits/MaxSize.h>
#include <metric/numeric/math/typetraits/RemoveAdaptor.h>
#include <metric/numeric/math/typetraits/Size.h>
#include <metric/numeric/math/typetraits/StorageOrder.h>
#include <metric/numeric/math/typetraits/YieldsDiagonal.h>
#include <metric/numeric/math/typetraits/YieldsIdentity.h>
#include <metric/numeric/math/typetraits/YieldsZero.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  DIAGONALMATRIX OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\name DiagonalMatrix operators */
//@{
template <RelaxationFlag RF, typename MT, bool SO, bool DF> bool isDefault(const DiagonalMatrix<MT, SO, DF> &m);

template <typename MT, bool SO, bool DF> bool isIntact(const DiagonalMatrix<MT, SO, DF> &m);

template <typename MT, bool SO, bool DF>
void swap(DiagonalMatrix<MT, SO, DF> &a, DiagonalMatrix<MT, SO, DF> &b) noexcept;
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the given diagonal matrix is in default state.
// \ingroup diagonal_matrix
//
// \param m The diagonal matrix to be tested for its default state.
// \return \a true in case the given matrix is component-wise zero, \a false otherwise.
//
// This function checks whether the matrix is in default state. For instance, in case the
// matrix is instantiated for a built-in integral or floating point data type, the function
// returns \a true in case all matrix elements are 0 and \a false in case any matrix element
// is not 0. The following example demonstrates the use of the \a isDefault function:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DiagonalMatrix;

   DiagonalMatrix< DynamicMatrix<int> > A;
   // ... Resizing and initialization
   if( isDefault( A ) ) { ... }
   \endcode

// Optionally, it is possible to switch between strict semantics (mtrc::numeric::strict) and relaxed
// semantics (mtrc::numeric::relaxed):

   \code
   if( isDefault<relaxed>( A ) ) { ... }
   \endcode
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF> // Density flag
inline bool isDefault(const DiagonalMatrix<MT, SO, DF> &m)
{
	if (Size_v<MT, 0UL> == DefaultSize_v)
		return m.rows() == 0UL;

	for (size_t i = 0UL; i < m.rows(); ++i) {
		if (!isDefault<RF>(m(i, i)))
			return false;
	}

	return true;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the invariants of the given diagonal matrix are intact.
// \ingroup diagonal_matrix
//
// \param m The diagonal matrix to be tested.
// \return \a true in case the given matrix's invariants are intact, \a false otherwise.
//
// This function checks whether the invariants of the diagonal matrix are intact, i.e. if its
// state is valid. In case the invariants are intact, the function returns \a true, else it
// will return \a false. The following example demonstrates the use of the \a isIntact()
// function:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DiagonalMatrix;

   DiagonalMatrix< DynamicMatrix<int> > A;
   // ... Resizing and initialization
   if( isIntact( A ) ) { ... }
   \endcode
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF> // Density flag
inline bool isIntact(const DiagonalMatrix<MT, SO, DF> &m)
{
	return m.isIntact();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Swapping the contents of two matrices.
// \ingroup diagonal_matrix
//
// \param a The first matrix to be swapped.
// \param b The second matrix to be swapped.
// \return void
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF> // Density flag
inline void swap(DiagonalMatrix<MT, SO, DF> &a, DiagonalMatrix<MT, SO, DF> &b) noexcept
{
	a.swap(b);
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief In-place inversion of the given diagonal dense matrix.
// \ingroup diagonal_matrix
//
// \param m The diagonal dense matrix to be inverted.
// \return void
// \exception std::invalid_argument Inversion of singular matrix failed.
//
// This function inverts the given diagonal dense matrix by means of the specified matrix inversion
// algorithm \c IF. The The inversion fails if the given matrix is singular and not invertible.
// In this case a \a std::invalid_argument exception is thrown.
//
// \note The matrix inversion can only be used for dense matrices with \c float, \c double,
// \c complex<float> or \c complex<double> element type. The attempt to call the function with
// matrices of any other element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a linker error will be created.
//
// \note This function does only provide the basic exception safety guarantee, i.e. in case of an
// exception \a m may already have been modified.
*/
template <InversionFlag IF // Inversion algorithm
		  ,
		  typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order of the dense matrix
inline void invert(DiagonalMatrix<MT, SO, true> &m)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	if (IF == asUniLower || IF == asUniUpper) {
		METRIC_NUMERIC_INTERNAL_ASSERT(isIdentity(m), "Violation of preconditions detected");
		return;
	}

	invert<asDiagonal>(derestrict(m));

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(m), "Broken invariant detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief LU decomposition of the given diagonal dense matrix.
// \ingroup diagonal_matrix
//
// \param A The diagonal matrix to be decomposed.
// \param L The resulting lower triangular matrix.
// \param U The resulting upper triangular matrix.
// \param P The resulting permutation matrix.
// \return void
//
// This function performs the dense matrix (P)LU decomposition of a diagonal n-by-n matrix. The
// resulting decomposition is written to the three distinct matrices \c L, \c U, and \c P, which
// are resized to the correct dimensions (if possible and necessary).
//
// \note The LU decomposition will never fail, even for singular matrices. However, in case of a
// singular matrix the resulting decomposition cannot be used for a matrix inversion or solving
// a linear system of equations.
*/
template <typename MT1, bool SO1, typename MT2, typename MT3, typename MT4, bool SO2>
inline void lu(const DiagonalMatrix<MT1, SO1, true> &A, DenseMatrix<MT2, SO1> &L, DenseMatrix<MT3, SO1> &U,
			   Matrix<MT4, SO2> &P)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_HERMITIAN_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UPPER_MATRIX_TYPE(MT2);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_HERMITIAN_MATRIX_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_LOWER_MATRIX_TYPE(MT3);

	using ET3 = ElementType_t<MT3>;
	using ET4 = ElementType_t<MT4>;

	const size_t n((*A).rows());

	decltype(auto) U2(derestrict(*U));

	(*L) = A;

	resize(*U, n, n);
	reset(U2);

	resize(*P, n, n);
	reset(*P);

	for (size_t i = 0UL; i < n; ++i) {
		U2(i, i) = ET3(1);
		(*P)(i, i) = ET4(1);
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by setting a single element of a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param mat The target diagonal matrix.
// \param i The row index of the element to be set.
// \param j The column index of the element to be set.
// \param value The value to be set to the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename ET> // Type of the element
inline bool trySet(const DiagonalMatrix<MT, SO, DF> &mat, size_t i, size_t j, const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(i < (*mat).rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j < (*mat).columns(), "Invalid column access index");

	MAYBE_UNUSED(mat);

	return (i == j || isDefault(value));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by setting a range of elements of a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param mat The target diagonal matrix.
// \param row The index of the first row of the range to be multiplied.
// \param column The index of the first column of the range to be multiplied.
// \param m The number of rows of the range to be multiplied.
// \param n The number of columns of the range to be multiplied.
// \param value The value to be set to the range of elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool trySet(const DiagonalMatrix<MT, SO, DF> &mat, size_t row, size_t column, size_t m,
										 size_t n, const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(row <= (*mat).rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column <= (*mat).columns(), "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(row + m <= (*mat).rows(), "Invalid number of rows");
	METRIC_NUMERIC_INTERNAL_ASSERT(column + n <= (*mat).columns(), "Invalid number of columns");

	MAYBE_UNUSED(mat);

	return (m == 0UL) || (n == 0UL) || (row == column && m == 1UL && n == 1UL) || isDefault(value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by adding to a single element of a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param mat The target diagonal matrix.
// \param i The row index of the element to be modified.
// \param j The column index of the element to be modified.
// \param value The value to be added to the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename ET> // Type of the element
inline bool tryAdd(const DiagonalMatrix<MT, SO, DF> &mat, size_t i, size_t j, const ET &value)
{
	return trySet(mat, i, j, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by adding to a range of elements of a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param mat The target diagonal matrix.
// \param row The index of the first row of the range to be multiplied.
// \param column The index of the first column of the range to be multiplied.
// \param m The number of rows of the range to be multiplied.
// \param n The number of columns of the range to be multiplied.
// \param value The value to be added to the range of elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool tryAdd(const DiagonalMatrix<MT, SO, DF> &mat, size_t row, size_t column, size_t m,
										 size_t n, const ET &value)
{
	return trySet(mat, row, column, m, n, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by subtracting from a single element of a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param mat The target diagonal matrix.
// \param i The row index of the element to be modified.
// \param j The column index of the element to be modified.
// \param value The value to be subtracted from the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename ET> // Type of the element
inline bool trySub(const DiagonalMatrix<MT, SO, DF> &mat, size_t i, size_t j, const ET &value)
{
	return trySet(mat, i, j, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by subtracting from a range of elements of a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param mat The target diagonal matrix.
// \param row The index of the first row of the range to be multiplied.
// \param column The index of the first column of the range to be multiplied.
// \param m The number of rows of the range to be multiplied.
// \param n The number of columns of the range to be multiplied.
// \param value The value to be subtracted from the range of elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool trySub(const DiagonalMatrix<MT, SO, DF> &mat, size_t row, size_t column, size_t m,
										 size_t n, const ET &value)
{
	return trySet(mat, row, column, m, n, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by a bitwise OR on a single element of a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param mat The target diagonal matrix.
// \param i The row index of the element to be modified.
// \param j The column index of the element to be modified.
// \param value The bit pattern to be used on the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename ET> // Type of the element
inline bool tryBitor(const DiagonalMatrix<MT, SO, DF> &mat, size_t i, size_t j, const ET &value)
{
	return trySet(mat, i, j, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by a bitwise OR on a range of elements of a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param mat The target diagonal matrix.
// \param row The index of the first row of the range to be modified.
// \param column The index of the first column of the range to be modified.
// \param m The number of rows of the range to be modified.
// \param n The number of columns of the range to be modified.
// \param value The bit pattern to be used on the range of elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool tryBitor(const DiagonalMatrix<MT, SO, DF> &mat, size_t row, size_t column, size_t m,
										   size_t n, const ET &value)
{
	return trySet(mat, row, column, m, n, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by a bitwise XOR on a single element of a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param mat The target diagonal matrix.
// \param i The row index of the element to be modified.
// \param j The column index of the element to be modified.
// \param value The bit pattern to be used on the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename ET> // Type of the element
inline bool tryBitxor(const DiagonalMatrix<MT, SO, DF> &mat, size_t i, size_t j, const ET &value)
{
	return tryAdd(mat, i, j, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by a bitwise XOR on a range of elements of a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param mat The target diagonal matrix.
// \param row The index of the first row of the range to be modified.
// \param column The index of the first column of the range to be modified.
// \param m The number of rows of the range to be modified.
// \param n The number of columns of the range to be modified.
// \param value The bit pattern to be used on the range of elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool tryBitxor(const DiagonalMatrix<MT, SO, DF> &mat, size_t row, size_t column, size_t m,
											size_t n, const ET &value)
{
	return tryAdd(mat, row, column, m, n, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a dense vector to a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side dense vector to be assigned.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT> // Type of the right-hand side dense vector
inline bool tryAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const DenseVector<VT, false> &rhs, size_t row,
					  size_t column)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(VT);

	METRIC_NUMERIC_INTERNAL_ASSERT(row <= lhs.rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column <= lhs.columns(), "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(row + (*rhs).size() <= lhs.rows(), "Invalid number of rows");

	MAYBE_UNUSED(lhs);

	const size_t index((column <= row) ? (0UL) : (column - row));

	for (size_t i = 0UL; i < index; ++i) {
		if (!isDefault((*rhs)[i]))
			return false;
	}

	for (size_t i = index + 1UL; i < (*rhs).size(); ++i) {
		if (!isDefault((*rhs)[i]))
			return false;
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a dense vector to a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side dense vector to be assigned.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT> // Type of the right-hand side dense vector
inline bool tryAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const DenseVector<VT, true> &rhs, size_t row,
					  size_t column)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(VT);

	METRIC_NUMERIC_INTERNAL_ASSERT(row <= lhs.rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column <= lhs.columns(), "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column + (*rhs).size() <= lhs.columns(), "Invalid number of columns");

	MAYBE_UNUSED(lhs);

	const size_t index((row <= column) ? (0UL) : (row - column));

	for (size_t i = 0UL; i < index; ++i) {
		if (!isDefault((*rhs)[i]))
			return false;
	}

	for (size_t i = index + 1UL; i < (*rhs).size(); ++i) {
		if (!isDefault((*rhs)[i]))
			return false;
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a dense vector to a band of a
//        diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side dense vector to be assigned.
// \param band The index of the band the right-hand side vector is assigned to.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT // Type of the right-hand side dense vector
		  ,
		  bool TF> // Transpose flag of the right-hand side dense vector
inline bool tryAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const DenseVector<VT, TF> &rhs, ptrdiff_t band, size_t row,
					  size_t column)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(VT);

	METRIC_NUMERIC_INTERNAL_ASSERT(row <= lhs.rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column <= lhs.columns(), "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column + (*rhs).size() <= lhs.columns(), "Invalid number of columns");

	MAYBE_UNUSED(lhs, row, column);

	if (band != 0L) {
		for (size_t i = 0UL; i < (*rhs).size(); ++i) {
			if (!isDefault((*rhs)[i]))
				return false;
		}
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a sparse vector to a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side sparse vector to be assigned.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT> // Type of the right-hand side sparse vector
inline bool tryAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const SparseVector<VT, false> &rhs, size_t row,
					  size_t column)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(VT);

	METRIC_NUMERIC_INTERNAL_ASSERT(row <= lhs.rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column <= lhs.columns(), "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(row + (*rhs).size() <= lhs.rows(), "Invalid number of rows");

	MAYBE_UNUSED(lhs);

	const size_t index(column - row);

	for (auto element = (*rhs).begin(); element != (*rhs).end(); ++element) {
		if (element->index() != index && !isDefault(element->value()))
			return false;
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a sparse vector to a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side sparse vector to be assigned.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT> // Type of the right-hand side sparse vector
inline bool tryAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const SparseVector<VT, true> &rhs, size_t row,
					  size_t column)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(VT);

	METRIC_NUMERIC_INTERNAL_ASSERT(row <= lhs.rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column <= lhs.columns(), "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column + (*rhs).size() <= lhs.columns(), "Invalid number of columns");

	MAYBE_UNUSED(lhs);

	const size_t index(row - column);

	for (auto element = (*rhs).begin(); element != (*rhs).end(); ++element) {
		if (element->index() != index && !isDefault(element->value()))
			return false;
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a sparse vector to a band of a
//        diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side sparse vector to be assigned.
// \param band The index of the band the right-hand side vector is assigned to.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT // Type of the right-hand side sparse vector
		  ,
		  bool TF> // Transpose flag of the right-hand side sparse vector
inline bool tryAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const SparseVector<VT, TF> &rhs, ptrdiff_t band,
					  size_t row, size_t column)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(VT);

	METRIC_NUMERIC_INTERNAL_ASSERT(row <= lhs.rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column <= lhs.columns(), "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column + (*rhs).size() <= lhs.columns(), "Invalid number of columns");

	MAYBE_UNUSED(lhs, row, column);

	if (band != 0L) {
		for (const auto &element : *rhs) {
			if (!isDefault(element.value()))
				return false;
		}
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a dense matrix to a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side dense matrix to be assigned.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename MT2> // Type of the right-hand side dense matrix
inline bool tryAssign(const DiagonalMatrix<MT1, SO, DF> &lhs, const DenseMatrix<MT2, false> &rhs, size_t row,
					  size_t column)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(MT2);

	METRIC_NUMERIC_INTERNAL_ASSERT(row <= lhs.rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column <= lhs.columns(), "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(row + (*rhs).rows() <= lhs.rows(), "Invalid number of rows");
	METRIC_NUMERIC_INTERNAL_ASSERT(column + (*rhs).columns() <= lhs.columns(), "Invalid number of columns");

	MAYBE_UNUSED(lhs);

	const size_t M((*rhs).rows());
	const size_t N((*rhs).columns());

	for (size_t i = 0UL; i < M; ++i) {
		for (size_t j = 0UL; j < N; ++j) {
			if ((row + i != column + j) && !isDefault((*rhs)(i, j)))
				return false;
		}
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a dense matrix to a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side dense matrix to be assigned.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename MT2> // Type of the right-hand side dense matrix
inline bool tryAssign(const DiagonalMatrix<MT1, SO, DF> &lhs, const DenseMatrix<MT2, true> &rhs, size_t row,
					  size_t column)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(MT2);

	METRIC_NUMERIC_INTERNAL_ASSERT(row <= lhs.rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column <= lhs.columns(), "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(row + (*rhs).rows() <= lhs.rows(), "Invalid number of rows");
	METRIC_NUMERIC_INTERNAL_ASSERT(column + (*rhs).columns() <= lhs.columns(), "Invalid number of columns");

	MAYBE_UNUSED(lhs);

	const size_t M((*rhs).rows());
	const size_t N((*rhs).columns());

	for (size_t j = 0UL; j < N; ++j) {
		for (size_t i = 0UL; i < M; ++i) {
			if ((column + j != row + i) && !isDefault((*rhs)(i, j)))
				return false;
		}
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a sparse matrix to a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side sparse matrix to be assigned.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename MT2> // Type of the right-hand side sparse matrix
inline bool tryAssign(const DiagonalMatrix<MT1, SO, DF> &lhs, const SparseMatrix<MT2, false> &rhs, size_t row,
					  size_t column)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(MT2);

	METRIC_NUMERIC_INTERNAL_ASSERT(row <= lhs.rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column <= lhs.columns(), "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(row + (*rhs).rows() <= lhs.rows(), "Invalid number of rows");
	METRIC_NUMERIC_INTERNAL_ASSERT(column + (*rhs).columns() <= lhs.columns(), "Invalid number of columns");

	MAYBE_UNUSED(lhs);

	const size_t M((*rhs).rows());

	for (size_t i = 0UL; i < M; ++i) {
		for (auto element = (*rhs).begin(i); element != (*rhs).end(i); ++element) {
			if ((row + i != column + element->index()) && !isDefault(element->value()))
				return false;
		}
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a sparse matrix to a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side sparse matrix to be assigned.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename MT2> // Type of the right-hand side sparse matrix
inline bool tryAssign(const DiagonalMatrix<MT1, SO, DF> &lhs, const SparseMatrix<MT2, true> &rhs, size_t row,
					  size_t column)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(MT2);

	METRIC_NUMERIC_INTERNAL_ASSERT(row <= lhs.rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column <= lhs.columns(), "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(row + (*rhs).rows() <= lhs.rows(), "Invalid number of rows");
	METRIC_NUMERIC_INTERNAL_ASSERT(column + (*rhs).columns() <= lhs.columns(), "Invalid number of columns");

	MAYBE_UNUSED(lhs);

	const size_t N((*rhs).columns());

	for (size_t j = 0UL; j < N; ++j) {
		for (auto element = (*rhs).begin(j); element != (*rhs).end(j); ++element) {
			if ((column + j != row + element->index()) && !isDefault(element->value()))
				return false;
		}
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the addition assignment of a vector to a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side vector to be added.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT // Type of the right-hand side vector
		  ,
		  bool TF> // Transpose flag of the right-hand side vector
inline bool tryAddAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const Vector<VT, TF> &rhs, size_t row, size_t column)
{
	return tryAssign(lhs, *rhs, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the addition assignment of a vector to the band of
//        a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side vector to be added.
// \param band The index of the band the right-hand side vector is assigned to.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT // Type of the right-hand side vector
		  ,
		  bool TF> // Transpose flag of the right-hand side vector
inline bool tryAddAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const Vector<VT, TF> &rhs, ptrdiff_t band, size_t row,
						 size_t column)
{
	return tryAssign(lhs, *rhs, band, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the addition assignment of a matrix to a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side matrix to be added.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the adapted matrix
		  ,
		  bool SO1 // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline bool tryAddAssign(const DiagonalMatrix<MT1, SO1, DF> &lhs, const Matrix<MT2, SO2> &rhs, size_t row,
						 size_t column)
{
	return tryAssign(lhs, *rhs, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the subtraction assignment of a vector to a diagonal
//        matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side vector to be subtracted.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT // Type of the right-hand side vector
		  ,
		  bool TF> // Transpose flag of the right-hand side vector
inline bool trySubAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const Vector<VT, TF> &rhs, size_t row, size_t column)
{
	return tryAssign(lhs, *rhs, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the subtraction assignment of a vector to the band of
//        a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side vector to be subtracted.
// \param band The index of the band the right-hand side vector is assigned to.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT // Type of the right-hand side vector
		  ,
		  bool TF> // Transpose flag of the right-hand side vector
inline bool trySubAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const Vector<VT, TF> &rhs, ptrdiff_t band, size_t row,
						 size_t column)
{
	return tryAssign(lhs, *rhs, band, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the subtraction assignment of a matrix to a diagonal
//        matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side matrix to be subtracted.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the adapted matrix
		  ,
		  bool SO1 // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline bool trySubAssign(const DiagonalMatrix<MT1, SO1, DF> &lhs, const Matrix<MT2, SO2> &rhs, size_t row,
						 size_t column)
{
	return tryAssign(lhs, *rhs, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the bitwise OR assignment of a vector to a diagonal
//        matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side vector for the bitwise OR operation.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT // Type of the right-hand side vector
		  ,
		  bool TF> // Transpose flag of the right-hand side vector
inline bool tryBitorAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const Vector<VT, TF> &rhs, size_t row, size_t column)
{
	return tryAssign(lhs, *rhs, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the bitwise OR assignment of a vector to the band of
//        a diagonal matrix.
// \ingroup diagonal_matrix
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side vector for the bitwise OR operation.
// \param band The index of the band the right-hand side vector is assigned to.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT // Type of the right-hand side vector
		  ,
		  bool TF> // Transpose flag of the right-hand side vector
inline bool tryBitorAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const Vector<VT, TF> &rhs, ptrdiff_t band, size_t row,
						   size_t column)
{
	return tryAssign(lhs, *rhs, band, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the bitwise OR assignment of a matrix to a diagonal matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side matrix for the bitwise OR operation.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the adapted matrix
		  ,
		  bool SO1 // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline bool tryBitorAssign(const DiagonalMatrix<MT1, SO1, DF> &lhs, const Matrix<MT2, SO2> &rhs, size_t row,
						   size_t column)
{
	return tryAssign(lhs, *rhs, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the bitwise XOR assignment of a vector to a diagonal
//        matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side vector for the bitwise XOR operation.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT // Type of the right-hand side vector
		  ,
		  bool TF> // Transpose flag of the right-hand side vector
inline bool tryBitxorAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const Vector<VT, TF> &rhs, size_t row, size_t column)
{
	return tryAssign(lhs, *rhs, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the bitwise XOR assignment of a vector to the band of
//        a diagonal matrix.
// \ingroup diagonal_matrix
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side vector for the bitwise XOR operation.
// \param band The index of the band the right-hand side vector is assigned to.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename VT // Type of the right-hand side vector
		  ,
		  bool TF> // Transpose flag of the right-hand side vector
inline bool tryBitxorAssign(const DiagonalMatrix<MT, SO, DF> &lhs, const Vector<VT, TF> &rhs, ptrdiff_t band,
							size_t row, size_t column)
{
	return tryAssign(lhs, *rhs, band, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the bitwise XOR assignment of a matrix to a diagonal
//        matrix.
// \ingroup diagonal_matrix
//
// \param lhs The target left-hand side diagonal matrix.
// \param rhs The right-hand side matrix for the bitwise XOR operation.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the adapted matrix
		  ,
		  bool SO1 // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline bool tryBitxorAssign(const DiagonalMatrix<MT1, SO1, DF> &lhs, const Matrix<MT2, SO2> &rhs, size_t row,
							size_t column)
{
	return tryAssign(lhs, *rhs, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns a reference to the instance without the access restrictions to the lower and
//        upper part.
// \ingroup math_shims
//
// \param m The diagonal matrix to be derestricted.
// \return Reference to the matrix without access restrictions.
//
// This function returns a reference to the given diagonal matrix instance that has no access
// restrictions to the lower and upper part of the matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in the violation of invariants, erroneous results and/or in compilation errors.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF> // Density flag
inline MT &derestrict(DiagonalMatrix<MT, SO, DF> &m)
{
	return m.matrix_;
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SIZE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct Size<DiagonalMatrix<MT, SO, DF>, 0UL> : public Size<MT, 0UL> {};

template <typename MT, bool SO, bool DF> struct Size<DiagonalMatrix<MT, SO, DF>, 1UL> : public Size<MT, 1UL> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SIZE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct MaxSize<DiagonalMatrix<MT, SO, DF>, 0UL> : public MaxSize<MT, 0UL> {};

template <typename MT, bool SO, bool DF> struct MaxSize<DiagonalMatrix<MT, SO, DF>, 1UL> : public MaxSize<MT, 1UL> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISSQUARE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct IsSquare<DiagonalMatrix<MT, SO, DF>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISUNIFORM SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct IsUniform<DiagonalMatrix<MT, SO, DF>> : public IsUniform<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct IsSymmetric<DiagonalMatrix<MT, SO, DF>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISHERMITIAN SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF>
struct IsHermitian<DiagonalMatrix<MT, SO, DF>> : public IsBuiltin<ElementType_t<MT>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct IsLower<DiagonalMatrix<MT, SO, DF>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISSTRICTLYLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct IsStrictlyLower<DiagonalMatrix<MT, SO, DF>> : public IsZero<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct IsUpper<DiagonalMatrix<MT, SO, DF>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISSTRICTLYUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct IsStrictlyUpper<DiagonalMatrix<MT, SO, DF>> : public IsZero<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISADAPTOR SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct IsAdaptor<DiagonalMatrix<MT, SO, DF>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISRESTRICTED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct IsRestricted<DiagonalMatrix<MT, SO, DF>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  HASCONSTDATAACCESS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO> struct HasConstDataAccess<DiagonalMatrix<MT, SO, true>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISALIGNED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct IsAligned<DiagonalMatrix<MT, SO, DF>> : public IsAligned<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISCONTIGUOUS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct IsContiguous<DiagonalMatrix<MT, SO, DF>> : public IsContiguous<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISPADDED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct IsPadded<DiagonalMatrix<MT, SO, DF>> : public IsPadded<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  REMOVEADAPTOR SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct RemoveAdaptor<DiagonalMatrix<MT, SO, DF>> {
	using Type = MT;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ADDTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct AddTraitEval1<T1, T2,
					 EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> && IsDiagonal_v<T1> && IsDiagonal_v<T2> &&
								!(IsZero_v<T1> || IsZero_v<T2>)>> {
	using Type = DiagonalMatrix<typename AddTraitEval2<T1, T2>::Type>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SUBTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct SubTraitEval1<T1, T2,
					 EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> && IsDiagonal_v<T1> && IsDiagonal_v<T2> &&
								!(IsZero_v<T1> || IsZero_v<T2>)>> {
	using Type = DiagonalMatrix<typename SubTraitEval2<T1, T2>::Type>;
};

template <typename T1, typename T2>
struct SubTraitEval1<T1, T2, EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> && IsZero_v<T1> && IsIdentity_v<T2>>> {
	using Tmp = If_t<StorageOrder_v<T1> != StorageOrder_v<T2>, OppositeType_t<T1>, T1>;
	using Type = DiagonalMatrix<typename SubTraitEval2<Tmp, T2>::Type>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SCHURTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct SchurTraitEval1<
	T1, T2,
	EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> &&
			   (IsDiagonal_v<T1> || IsDiagonal_v<T2> || (IsLower_v<T1> && IsUpper_v<T2>) ||
				(IsUpper_v<T1> && IsLower_v<T2>)) &&
			   !(IsStrictlyLower_v<T1> && IsUpper_v<T2>) && !(IsStrictlyUpper_v<T1> && IsLower_v<T2>) &&
			   !(IsLower_v<T1> && IsStrictlyUpper_v<T2>) && !(IsUpper_v<T1> && IsStrictlyLower_v<T2>) &&
			   !(IsUniTriangular_v<T1> && IsUniTriangular_v<T2>) && !(IsZero_v<T1> || IsZero_v<T2>)>> {
	using Type = DiagonalMatrix<typename SchurTraitEval2<T1, T2>::Type>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  MULTTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct MultTraitEval1<T1, T2, EnableIf_t<IsMatrix_v<T1> && IsScalar_v<T2> && (IsDiagonal_v<T1> && !IsUniform_v<T1>)>> {
	using Type = DiagonalMatrix<typename MultTraitEval2<T1, T2>::Type>;
};

template <typename T1, typename T2>
struct MultTraitEval1<T1, T2, EnableIf_t<IsScalar_v<T1> && IsMatrix_v<T2> && (IsDiagonal_v<T2> && !IsUniform_v<T2>)>> {
	using Type = DiagonalMatrix<typename MultTraitEval2<T1, T2>::Type>;
};

template <typename T1, typename T2>
struct MultTraitEval1<T1, T2,
					  EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> && (IsDiagonal_v<T1> && IsDiagonal_v<T2>) &&
								 !(IsIdentity_v<T1> || IsIdentity_v<T2>) && !(IsZero_v<T1> || IsZero_v<T2>)>> {
	using Type = DiagonalMatrix<typename MultTraitEval2<T1, T2>::Type>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  KRONTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct KronTraitEval1<T1, T2,
					  EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> && (IsDiagonal_v<T1> && IsDiagonal_v<T2>) &&
								 !(IsIdentity_v<T1> && IsIdentity_v<T2>) && !(IsZero_v<T1> || IsZero_v<T2>)>> {
	using Type = DiagonalMatrix<typename KronTraitEval2<T1, T2>::Type>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DIVTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2> struct DivTraitEval1<T1, T2, EnableIf_t<IsDiagonal_v<T1> && IsScalar_v<T2>>> {
	using Type = DiagonalMatrix<typename DivTraitEval2<T1, T2>::Type>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  MAPTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T, typename OP>
struct UnaryMapTraitEval1<T, OP,
						  EnableIf_t<YieldsDiagonal_v<OP, T> && !YieldsIdentity_v<OP, T> && !YieldsZero_v<OP, T>>> {
	using Type = DiagonalMatrix<typename UnaryMapTraitEval2<T, OP>::Type, StorageOrder_v<T>>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2, typename OP>
struct BinaryMapTraitEval1<
	T1, T2, OP,
	EnableIf_t<YieldsDiagonal_v<OP, T1, T2> && !YieldsIdentity_v<OP, T1, T2> && !YieldsZero_v<OP, T1, T2>>> {
	using Type = DiagonalMatrix<typename BinaryMapTraitEval2<T1, T2, OP>::Type>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLSYMTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct DeclSymTrait<DiagonalMatrix<MT, SO, DF>> {
	using Type = DiagonalMatrix<MT, SO, DF>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLHERMTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct DeclHermTrait<DiagonalMatrix<MT, SO, DF>> {
	using Type = HermitianMatrix<MT, SO, DF>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLLOWTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct DeclLowTrait<DiagonalMatrix<MT, SO, DF>> {
	using Type = DiagonalMatrix<MT, SO, DF>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLUNILOWTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct DeclUniLowTrait<DiagonalMatrix<MT, SO, DF>> {
	using Type = IdentityMatrix<ElementType_t<MT>, SO>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLSTRLOWTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct DeclStrLowTrait<DiagonalMatrix<MT, SO, DF>> {
	using Type = ZeroMatrix<ElementType_t<MT>, SO>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLUPPTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct DeclUppTrait<DiagonalMatrix<MT, SO, DF>> {
	using Type = DiagonalMatrix<MT, SO, DF>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLUNIUPPTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct DeclUniUppTrait<DiagonalMatrix<MT, SO, DF>> {
	using Type = IdentityMatrix<ElementType_t<MT>, SO>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLSTRUPPTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct DeclStrUppTrait<DiagonalMatrix<MT, SO, DF>> {
	using Type = ZeroMatrix<ElementType_t<MT>, SO>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLDIAGTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF> struct DeclDiagTrait<DiagonalMatrix<MT, SO, DF>> {
	using Type = DiagonalMatrix<MT, SO, DF>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  HIGHTYPE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, bool SO1, bool DF1, typename MT2, bool SO2, bool DF2>
struct HighType<DiagonalMatrix<MT1, SO1, DF1>, DiagonalMatrix<MT2, SO2, DF2>> {
	using Type = DiagonalMatrix<typename HighType<MT1, MT2>::Type>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  LOWTYPE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, bool SO1, bool DF1, typename MT2, bool SO2, bool DF2>
struct LowType<DiagonalMatrix<MT1, SO1, DF1>, DiagonalMatrix<MT2, SO2, DF2>> {
	using Type = DiagonalMatrix<typename LowType<MT1, MT2>::Type>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SUBMATRIXTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, size_t I, size_t N>
struct SubmatrixTraitEval1<MT, I, I, N, N,
						   EnableIf_t<I != inf && N != inf && IsDiagonal_v<MT> && !IsIdentity_v<MT> && !IsZero_v<MT>>> {
	using Type = DiagonalMatrix<typename SubmatrixTraitEval2<MT, I, I, N, N>::Type>;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
