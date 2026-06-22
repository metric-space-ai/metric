// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_DENSE_SVD_H
#define METRIC_NUMERIC_MATH_DENSE_SVD_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/Adaptor.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/MutableDataAccess.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/lapack/gesdd.h>
#include <metric/numeric/math/typetraits/IsContiguous.h>
#include <metric/numeric/math/typetraits/RemoveAdaptor.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  SINGULAR VALUE DECOMPOSITION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name Singular value decomposition functions */
//@{
template <typename MT, bool SO, typename VT, bool TF>
inline void svd(const DenseMatrix<MT, SO> &A, DenseVector<VT, TF> &s);

template <typename MT1, bool SO, typename VT, bool TF, typename MT2, typename MT3>
inline void svd(const DenseMatrix<MT1, SO> &A, DenseMatrix<MT2, SO> &U, DenseVector<VT, TF> &s, DenseMatrix<MT3, SO> &V,
				bool square = false);

template <typename MT, bool SO, typename VT, bool TF, typename ST>
inline size_t svd(const DenseMatrix<MT, SO> &A, DenseVector<VT, TF> &s, ST low, ST upp);

template <typename MT1, bool SO, typename VT, bool TF, typename MT2, typename MT3, typename ST>
inline size_t svd(const DenseMatrix<MT1, SO> &A, DenseMatrix<MT2, SO> &U, DenseVector<VT, TF> &s,
				  DenseMatrix<MT3, SO> &V, ST low, ST upp);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Singular value decomposition (SVD) of the given dense general matrix.
// \ingroup dense_matrix
//
// \param A The given general matrix.
// \param s The resulting vector of singular values.
// \return void
// \exception std::invalid_argument Size of fixed size vector does not match.
// \exception std::runtime_error Singular value decomposition failed.
//
// This function performs the singular value decomposition of a general \a m-by-\a n matrix.
// The resulting min(\a m,\a n) singular values are stored in the given vector \a s, which is
// resized to the correct size (if possible and necessary).
//
// The function fails if ...
//
//  - ... the given vector \a s is a fixed size vector and the size doesn't match;
//  - ... the singular value decomposition fails.
//
// In all failure cases an exception is thrown.
//
// Examples:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   DynamicMatrix<double,rowMajor>  A( 5UL, 8UL );  // The general matrix A
   // ... Initialization

   DynamicVector<double,columnVector> s;  // The vector for the singular values

   svd( A, s );
   \endcode

// \note This function only works for matrices with \c float, \c double, \c complex<float>, or
// \c complex<double> element type. The attempt to call the function with matrices of any other
// element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a call to this function will result in a linker error.
//
// \note Further options for computing singular values and singular vectors are available via the
// gesvd(), gesdd(), and gesvdx() functions.
*/
template <typename MT // Type of the matrix A
		  ,
		  bool SO // Storage order of the matrix A
		  ,
		  typename VT // Type of the vector s
		  ,
		  bool TF> // Transpose flag of the vector s
inline void svd(const DenseMatrix<MT, SO> &A, DenseVector<VT, TF> &s)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<VT>);

	using ATmp = ResultType_t<RemoveAdaptor_t<MT>>;
	using STmp = If_t<IsContiguous_v<VT>, VT &, ResultType_t<VT>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<ATmp>);

	ATmp Atmp(*A);
	STmp stmp(*s);

	gesdd(Atmp, stmp);

	if (!IsContiguous_v<VT>) {
		(*s) = stmp;
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Singular value decomposition (SVD) of the given dense general matrix.
// \ingroup dense_matrix
//
// \param A The given general matrix.
// \param U The resulting matrix of left singular vectors.
// \param s The resulting vector of singular values.
// \param V The resulting matrix of right singular vectors.
// \param square Defaults to False.  If True, will make sure U and V are square matrices.
// \return void
// \exception std::invalid_argument Dimensions of fixed size matrix U do not match.
// \exception std::invalid_argument Size of fixed size vector does not match.
// \exception std::invalid_argument Dimensions of fixed size matrix V do not match.
// \exception std::runtime_error Singular value decomposition failed.
//
// This function performs the singular value decomposition of a general \a m-by-\a n matrix. The
// resulting min(\a m,\a n) singular values are stored in the given vector \a s, the resulting
// left singular vectors are stored in the given matrix \a U, and the resulting right singular
// vectors are stored in the given matrix \a V. \a s, \a U and \a V are resized to the correct
// dimensions (if possible and necessary).
//
// The function fails if ...
//
//  - ... the given matrix \a U is a fixed size matrix and the dimensions don't match;
//  - ... the given vector \a s is a fixed size vector and the size doesn't match;
//  - ... the given matrix \a V is a fixed size matrix and the dimensions don't match;
//  - ... the singular value decomposition fails.
//
// In all failure cases an exception is thrown.
//
// Examples:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   DynamicMatrix<double,rowMajor>  A( 5UL, 8UL );  // The general matrix A
   // ... Initialization

   DynamicMatrix<double,rowMajor>     U;  // The matrix for the left singular vectors
   DynamicVector<double,columnVector> s;  // The vector for the singular values
   DynamicMatrix<double,rowMajor>     V;  // The matrix for the right singular vectors

   svd( A, U, s, V );
   \endcode

// \note This function only works for matrices with \c float, \c double, \c complex<float>, or
// \c complex<double> element type. The attempt to call the function with matrices of any other
// element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a call to this function will result in a linker error.
//
// \note Further options for computing singular values and singular vectors are available via the
// gesvd(), gesdd(), and gesvdx() functions.
*/
template <typename MT1 // Type of the matrix A
		  ,
		  bool SO // Storage order of all matrices
		  ,
		  typename VT // Type of the vector s
		  ,
		  bool TF // Transpose flag of the vector s
		  ,
		  typename MT2 // Type of the matrix U
		  ,
		  typename MT3> // Type of the matrix V
inline void svd(const DenseMatrix<MT1, SO> &A, DenseMatrix<MT2, SO> &U, DenseVector<VT, TF> &s, DenseMatrix<MT3, SO> &V,
				bool square)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT2>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<VT>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT3>);

	using ATmp = ResultType_t<RemoveAdaptor_t<MT1>>;
	using UTmp = If_t<IsContiguous_v<MT2>, MT2 &, ResultType_t<MT2>>;
	using STmp = If_t<IsContiguous_v<VT>, VT &, ResultType_t<VT>>;
	using VTmp = If_t<IsContiguous_v<MT3>, MT3 &, ResultType_t<MT3>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<ATmp>);

	ATmp Atmp(*A);
	UTmp Utmp(*U);
	STmp stmp(*s);
	VTmp Vtmp(*V);

	gesdd(Atmp, Utmp, stmp, Vtmp, (square) ? 'A' : 'S');

	if (!IsContiguous_v<MT2>) {
		(*U) = Utmp;
	}

	if (!IsContiguous_v<VT>) {
		(*s) = stmp;
	}

	if (!IsContiguous_v<MT3>) {
		(*V) = Vtmp;
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Singular value decomposition (SVD) of the given dense general matrix.
// \ingroup dense_matrix
//
// \param A The given general matrix.
// \param s The resulting vector of singular values.
// \param low The lower bound of the interval to be searched for singular values.
// \param upp The upper bound of the interval to be searched for singular values.
// \return The total number of singular values found.
// \exception std::invalid_argument Size of fixed size vector does not match.
// \exception std::invalid_argument Invalid value range provided.
// \exception std::invalid_argument Invalid index range provided.
// \exception std::runtime_error Singular value decomposition failed.
//
// This function computes a specified number of singular values of the given general \a m-by-\a n
// matrix. The number of singular values to be computed is specified by the lower bound \a low and
// the upper bound \a upp, which either form an integral or a floating point range.
//
// In case \a low and \a upp are of integral type, the function computes all singular values in
// the index range \f$[low..upp]\f$. The \a num resulting real and non-negative singular values
// are stored in descending order in the given vector \a s, which is either resized (if possible)
// or expected to be a \a num-dimensional vector.
//
// In case \a low and \a upp are of floating point type, the function computes all singular values
// in the half-open interval \f$(low..upp]\f$. The resulting real and non-negative singular values
// are stored in descending order in the given vector \a s, which is either resized (if possible)
// or expected to be a min(\a m,\a n)-dimensional vector.
//
// The function fails if ...
//
//  - ... the given vector \a s is a fixed size vector and the size doesn't match;
//  - ... the given scalar values don't form a proper range;
//  - ... the singular value decomposition fails.
//
// In all failure cases an exception is thrown.
//
// Examples:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   DynamicMatrix<double,rowMajor>  A( 5UL, 8UL );  // The general matrix A
   // ... Initialization

   DynamicVector<double,columnVector> s;  // The vector for the singular values

   svd( A, s, 0, 2 );
   \endcode

// \note This function only works for matrices with \c float, \c double, \c complex<float>, or
// \c complex<double> element type. The attempt to call the function with matrices of any other
// element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a call to this function will result in a linker error.
//
// \note Further options for computing singular values and singular vectors are available via the
// gesvd(), gesdd(), and gesvdx() functions.
*/
template <typename MT // Type of the matrix A
		  ,
		  bool SO // Storage order of the matrix A
		  ,
		  typename VT // Type of the vector s
		  ,
		  bool TF // Transpose flag of the vector s
		  ,
		  typename ST> // Type of the scalar boundary values
inline size_t svd(const DenseMatrix<MT, SO> &A, DenseVector<VT, TF> &s, ST low, ST upp)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<VT>);

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BUILTIN_TYPE(ST);

	ResultType_t<VT> all;
	svd(*A, all);

	if constexpr (IsIntegral_v<ST>) {
		if (low < ST(0) || upp < low) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid index range provided");
		}

		const size_t first(static_cast<size_t>(low));
		const size_t last(static_cast<size_t>(upp));

		if (last >= all.size()) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid index range provided");
		}

		const size_t num(last - first + 1UL);
		resize(*s, num, false);

		for (size_t index = 0UL; index < num; ++index) {
			(*s)[index] = all[first + index];
		}

		return num;
	} else {
		if (!(low < upp)) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid value range provided");
		}

		size_t num(0UL);
		for (size_t index = 0UL; index < all.size(); ++index) {
			if (all[index] > low && all[index] <= upp) {
				++num;
			}
		}

		resize(*s, num, false);

		size_t out(0UL);
		for (size_t index = 0UL; index < all.size(); ++index) {
			if (all[index] > low && all[index] <= upp) {
				(*s)[out++] = all[index];
			}
		}

		return num;
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Singular value decomposition (SVD) of the given dense general matrix.
// \ingroup dense_matrix
//
// \param A The given general matrix.
// \param U The resulting matrix of left singular vectors.
// \param s The resulting vector of singular values.
// \param V The resulting matrix of right singular vectors.
// \param low The lower bound of the interval to be searched for singular values.
// \param upp The upper bound of the interval to be searched for singular values.
// \return The total number of singular values found.
// \exception std::invalid_argument Dimensions of fixed size matrix U do not match.
// \exception std::invalid_argument Size of fixed size vector does not match.
// \exception std::invalid_argument Dimensions of fixed size matrix V do not match.
// \exception std::invalid_argument Invalid value range provided.
// \exception std::invalid_argument Invalid index range provided.
// \exception std::runtime_error Singular value decomposition failed.
//
// This function computes a specified number of singular values and singular vectors of the given
// general \a m-by-\a n matrix. The number of singular values and vectors to be computed is
// specified by the lower bound \a low and the upper bound \a upp, which either form an integral
// or a floating point range.
//
// In case \a low and \a upp form are of integral type, the function computes all singular values
// in the index range \f$[low..upp]\f$. The \a num resulting real and non-negative singular values
// are stored in descending order in the given vector \a s, which is either resized (if possible)
// or expected to be a \a num-dimensional vector. The resulting left singular vectors are stored
// in the given matrix \a U, which is either resized (if possible) or expected to be a
// \a m-by-\a num matrix. The resulting right singular vectors are stored in the given matrix \a V,
// which is either resized (if possible) or expected to be a \a num-by-\a n matrix.
//
// In case \a low and \a upp are of floating point type, the function computes all singular values
// in the half-open interval \f$(low..upp]\f$. The resulting real and non-negative singular values
// are stored in descending order in the given vector \a s, which is either resized (if possible)
// or expected to be a min(\a m,\a n)-dimensional vector. The resulting left singular vectors are
// stored in the given matrix \a U, which is either resized (if possible) or expected to be a
// \a m-by-min(\a m,\a n) matrix. The resulting right singular vectors are stored in the given
// matrix \a V, which is either resized (if possible) or expected to be a \a min(\a m,\a n)-by-\a n
// matrix.
//
// The function fails if ...
//
//  - ... the given matrix \a U is a fixed size matrix and the dimensions don't match;
//  - ... the given vector \a s is a fixed size vector and the size doesn't match;
//  - ... the given matrix \a V is a fixed size matrix and the dimensions don't match;
//  - ... the given scalar values don't form a proper range;
//  - ... the singular value decomposition fails.
//
// In all failure cases an exception is thrown.
//
// Examples:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   DynamicMatrix<double,rowMajor>  A( 5UL, 8UL );  // The general matrix A
   // ... Initialization

   DynamicMatrix<double,rowMajor>     U;  // The matrix for the left singular vectors
   DynamicVector<double,columnVector> s;  // The vector for the singular values
   DynamicMatrix<double,rowMajor>     V;  // The matrix for the right singular vectors

   svd( A, U, s, V, 0, 2 );
   \endcode

// \note This function only works for matrices with \c float, \c double, \c complex<float>, or
// \c complex<double> element type. The attempt to call the function with matrices of any other
// element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a call to this function will result in a linker error.
//
// \note Further options for computing singular values and singular vectors are available via the
// gesvd(), gesdd(), and gesvdx() functions.
*/
template <typename MT1 // Type of the matrix A
		  ,
		  bool SO // Storage order of all matrices
		  ,
		  typename VT // Type of the vector s
		  ,
		  bool TF // Transpose flag of the vector s
		  ,
		  typename MT2 // Type of the matrix U
		  ,
		  typename MT3 // Type of the matrix V
		  ,
		  typename ST> // Type of the scalar boundary values
inline size_t svd(const DenseMatrix<MT1, SO> &A, DenseMatrix<MT2, SO> &U, DenseVector<VT, TF> &s,
				  DenseMatrix<MT3, SO> &V, ST low, ST upp)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT2>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<VT>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT3>);

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BUILTIN_TYPE(ST);

	ResultType_t<MT2> allU;
	ResultType_t<VT> allS;
	ResultType_t<MT3> allV;
	svd(*A, allU, allS, allV);

	auto copy_index = [&](size_t source, size_t target) {
		(*s)[target] = allS[source];

		for (size_t row = 0UL; row < (*A).rows(); ++row) {
			(*U)(row, target) = allU(row, source);
		}

		for (size_t column = 0UL; column < (*A).columns(); ++column) {
			(*V)(target, column) = allV(source, column);
		}
	};

	if constexpr (IsIntegral_v<ST>) {
		if (low < ST(0) || upp < low) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid index range provided");
		}

		const size_t first(static_cast<size_t>(low));
		const size_t last(static_cast<size_t>(upp));

		if (last >= allS.size()) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid index range provided");
		}

		const size_t num(last - first + 1UL);
		resize(*U, (*A).rows(), num, false);
		resize(*s, num, false);
		resize(*V, num, (*A).columns(), false);

		for (size_t index = 0UL; index < num; ++index) {
			copy_index(first + index, index);
		}

		return num;
	} else {
		if (!(low < upp)) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid value range provided");
		}

		size_t num(0UL);
		for (size_t index = 0UL; index < allS.size(); ++index) {
			if (allS[index] > low && allS[index] <= upp) {
				++num;
			}
		}

		resize(*U, (*A).rows(), num, false);
		resize(*s, num, false);
		resize(*V, num, (*A).columns(), false);

		size_t out(0UL);
		for (size_t index = 0UL; index < allS.size(); ++index) {
			if (allS[index] > low && allS[index] <= upp) {
				copy_index(index, out++);
			}
		}

		return num;
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
