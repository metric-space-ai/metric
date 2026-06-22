// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_HESV_H
#define METRIC_NUMERIC_MATH_LAPACK_HESV_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <memory>
#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/Adaptor.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/Contiguous.h>
#include <metric/numeric/math/constraints/MutableDataAccess.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/lapack/clapack/hesv.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/NumericCast.h>
#include <metric/numeric/util/constraints/SameType.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK HERMITIAN INDEFINITE LINEAR SYSTEM FUNCTIONS (HESV)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK Hermitian indefinite linear system functions (hesv) */
//@{
template <typename MT, bool SO, typename VT, bool TF>
void hesv(DenseMatrix<MT, SO> &A, DenseVector<VT, TF> &b, char uplo, blas_int_t *ipiv);

template <typename MT1, bool SO1, typename MT2, bool SO2>
void hesv(DenseMatrix<MT1, SO1> &A, DenseMatrix<MT2, SO2> &B, char uplo, blas_int_t *ipiv);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for solving a Hermitian indefinite linear system of equations (\f$ A*x=b \f$).
// \ingroup lapack_solver
//
// \param A The column-major system matrix.
// \param b The right-hand side vector.
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param ipiv Auxiliary array of size \a n for the pivot indices.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Invalid right-hand side vector provided.
// \exception std::invalid_argument Invalid uplo argument provided.
// \exception std::runtime_error Inversion of singular matrix failed.
//
// This function uses the LAPACK hesv() functions to compute the solution to the Hermitian
// indefinite system of linear equations;
//
//  - \f$ A  *x=b \f$ if \a A is column-major
//  - \f$ A^T*x=b \f$ if \a A is row-major
//
// In this context the Hermitian indefinite system matrix \a A is a \a n-by-\a n matrix and \a x
// and \a b are n-dimensional vectors. Note that the function only works for general, non-adapted
// matrices with \c complex<float>, or \c complex<double> element type. The attempt to call the
// function with adaptors or matrices of any other element type results in a compile time error!
//
// If the function exits successfully, the vector \a b contains the solution of the linear system
// of equations and \a A has been decomposed by means of the Bunch-Kaufman decomposition. The
// decomposition has the form

					  \f[ A = U D U^{H} \texttt{ (if uplo = 'U'), or }
						  A = L D L^{H} \texttt{ (if uplo = 'L'), } \f]

// where \c U (or \c L) is a product of permutation and unit upper (lower) triangular matrices,
// and \c D is Hermitian and block diagonal with 1-by-1 and 2-by-2 diagonal blocks. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set to
// \c 'U' the result is stored in the upper part and the lower part remains untouched. The factored
// form of \a A is then used to solve the system of equations.
//
// The function fails if ...
//
//  - ... the given system matrix is not a square matrix;
//  - ... the given \a uplo argument is neither \c 'L' nor \c 'U';
//  - ... the given system matrix is singular and not invertible.
//
// In all failure cases an exception is thrown.
//
// For more information on the hesv() functions (i.e. chesv() and zhesv()) see the LAPACK online
// documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
//
// \note This function does only provide the basic exception safety guarantee, i.e. in case of an
// exception \a A may already have been modified.
*/
template <typename MT // Type of the system matrix
		  ,
		  bool SO // Storage order of the system matrix
		  ,
		  typename VT // Type of the right-hand side vector
		  ,
		  bool TF> // Transpose flag of the right-hand side vector
inline void hesv(DenseMatrix<MT, SO> &A, DenseVector<VT, TF> &b, char uplo, blas_int_t *ipiv)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_CONTIGUOUS_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_CONTIGUOUS_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TYPE(ElementType_t<MT>, ElementType_t<VT>);

	using ET = ElementType_t<MT>;

	if (!isSquare(*A)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	if ((*b).size() != (*A).rows()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid right-hand side vector provided");
	}

	if (uplo != 'L' && uplo != 'U') {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid uplo argument provided");
	}

	blas_int_t n(numeric_cast<blas_int_t>((*A).rows()));
	blas_int_t nrhs(1);
	blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));
	blas_int_t ldb(numeric_cast<blas_int_t>((*b).size()));
	blas_int_t info(0);

	if (n == 0) {
		return;
	}

	if (IsRowMajorMatrix_v<MT>) {
		(uplo == 'L') ? (uplo = 'U') : (uplo = 'L');
	}

	blas_int_t lwork(n * lda);
	const std::unique_ptr<ET[]> work(new ET[lwork]);

	hesv(uplo, n, nrhs, (*A).data(), lda, ipiv, (*b).data(), ldb, work.get(), lwork, &info);

	METRIC_NUMERIC_INTERNAL_ASSERT(info >= 0, "Invalid function argument");

	if (info > 0) {
		METRIC_NUMERIC_THROW_LAPACK_ERROR("Inversion of singular matrix failed");
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for solving a Hermitian indefinite linear system of equations (\f$ A*X=B \f$).
// \ingroup lapack_solver
//
// \param A The system matrix.
// \param B The matrix of right-hand sides.
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param ipiv Auxiliary array of size \a n for the pivot indices.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Invalid right-hand side matrix provided.
// \exception std::invalid_argument Invalid uplo argument provided.
// \exception std::runtime_error Inversion of singular matrix failed.
//
// This function uses the LAPACK hesv() functions to compute the solution to the Hermitian
// indefinite system of linear equations:
//
//  - \f$ A  *X  =B   \f$ if both \a A and \a B are column-major
//  - \f$ A^T*X  =B   \f$ if \a A is row-major and \a B is column-major
//  - \f$ A  *X^T=B^T \f$ if \a A is column-major and \a B is row-major
//  - \f$ A^T*X^T=B^T \f$ if both \a A and \a B are row-major
//
// In this context the Hermitian indefinite system matrix \a A is a \a n-by-\a n matrix and \a X
// and \a B are either row-major \a m-by-\a n matrices or column-major \a n-by-\a m matrices.
// Note that the function only works for general, non-adapted matrices with \c complex<float> or
// \c complex<double> element type. The attempt to call the function with adaptors or matrices
// of any other element type results in a compile time error!
//
// If the function exits successfully, the matrix \a B contains the solution of the linear system
// of equations and \a A has been decomposed by means of a Bunch-Kaufman decomposition. The
// decomposition has the form

					  \f[ A = U D U^{H} \texttt{ (if uplo = 'U'), or }
						  A = L D L^{H} \texttt{ (if uplo = 'L'), } \f]

// where \c U (or \c L) is a product of permutation and unit upper (lower) triangular matrices,
// and \c D is Hermitian and block diagonal with 1-by-1 and 2-by-2 diagonal blocks. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set to
// \c 'U' the result is stored in the upper part and the lower part remains untouched. The factored
// form of \a A is then used to solve the system of equations.
//
// The function fails if ...
//
//  - ... the given system matrix is not a square matrix;
//  - ... the given \a uplo argument is neither \c 'L' nor \c 'U';
//  - ... the sizes of the two given matrices do not match;
//  - ... the given system matrix is singular and not invertible.
//
// In all failure cases an exception is thrown.
//
// For more information on the hesv() functions (i.e. chesv() and zhesv()) see the LAPACK online
// documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
//
// \note This function does only provide the basic exception safety guarantee, i.e. in case of an
// exception \a A may already have been modified.
*/
template <typename MT1 // Type of the system matrix
		  ,
		  bool SO1 // Storage order of the system matrix
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline void hesv(DenseMatrix<MT1, SO1> &A, DenseMatrix<MT2, SO2> &B, char uplo, blas_int_t *ipiv)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_CONTIGUOUS_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_CONTIGUOUS_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TYPE(ElementType_t<MT1>, ElementType_t<MT2>);

	using ET = ElementType_t<MT1>;

	if (!isSquare(*A)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	if (uplo != 'L' && uplo != 'U') {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid uplo argument provided");
	}

	blas_int_t n(numeric_cast<blas_int_t>((*A).rows()));
	blas_int_t mrhs(numeric_cast<blas_int_t>(SO2 ? (*B).rows() : (*B).columns()));
	blas_int_t nrhs(numeric_cast<blas_int_t>(SO2 ? (*B).columns() : (*B).rows()));
	blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));
	blas_int_t ldb(numeric_cast<blas_int_t>((*B).spacing()));
	blas_int_t info(0);

	if (n != mrhs) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid right-hand side matrix provided");
	}

	if (n == 0) {
		return;
	}

	if (IsRowMajorMatrix_v<MT1>) {
		(uplo == 'L') ? (uplo = 'U') : (uplo = 'L');
	}

	blas_int_t lwork(n * lda);
	const std::unique_ptr<ET[]> work(new ET[lwork]);

	hesv(uplo, n, nrhs, (*A).data(), lda, ipiv, (*B).data(), ldb, work.get(), lwork, &info);

	METRIC_NUMERIC_INTERNAL_ASSERT(info >= 0, "Invalid function argument");

	if (info > 0) {
		METRIC_NUMERIC_THROW_LAPACK_ERROR("Inversion of singular matrix failed");
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
