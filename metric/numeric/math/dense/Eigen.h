// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_DENSE_EIGEN_H
#define METRIC_NUMERIC_MATH_DENSE_EIGEN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/constraints/Adaptor.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/MutableDataAccess.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/lapack/geev.h>
#include <metric/numeric/math/lapack/heevd.h>
#include <metric/numeric/math/lapack/syevd.h>
#include <metric/numeric/math/typetraits/IsContiguous.h>
#include <metric/numeric/math/typetraits/IsDiagonal.h>
#include <metric/numeric/math/typetraits/IsHermitian.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>
#include <metric/numeric/math/typetraits/IsSymmetric.h>
#include <metric/numeric/math/typetraits/IsTriangular.h>
#include <metric/numeric/math/typetraits/RemoveAdaptor.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/IsComplex.h>
#include <metric/numeric/util/typetraits/IsFloatingPoint.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  EIGENVALUE FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name Eigenvalue functions */
//@{
template <typename MT, bool SO, typename VT, bool TF>
inline void eigen(const DenseMatrix<MT, SO> &A, DenseVector<VT, TF> &w);

template <typename MT1, bool SO1, typename VT, bool TF, typename MT2, bool SO2>
inline void eigen(const DenseMatrix<MT1, SO1> &A, DenseVector<VT, TF> &w, DenseMatrix<MT2, SO2> &V);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense symmetric matrix.
// \ingroup dense_matrix
//
// \param A The given symmetric matrix.
// \param w The resulting vector of eigenvalues.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense symmetric matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template <typename MT // Type of the matrix A
		  ,
		  bool SO // Storage order of the matrix A
		  ,
		  typename VT // Type of the vector w
		  ,
		  bool TF> // Transpose flag of the vector w
inline auto eigen_backend(const DenseMatrix<MT, SO> &A, DenseVector<VT, TF> &w)
	-> EnableIf_t<IsSymmetric_v<MT> && !IsDiagonal_v<MT> && IsFloatingPoint_v<ElementType_t<MT>>>
{
	using ATmp = RemoveAdaptor_t<ResultType_t<MT>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<ATmp>);

	METRIC_NUMERIC_INTERNAL_ASSERT(isSquare(*A), "Non-square matrix detected");

	ATmp Atmp(*A);

	syevd(Atmp, *w, 'N', 'L');
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense Hermitian matrix.
// \ingroup dense_matrix
//
// \param A The given Hermitian matrix.
// \param w The resulting vector of eigenvalues.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense Hermitian matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template <typename MT // Type of the matrix A
		  ,
		  bool SO // Storage order of the matrix A
		  ,
		  typename VT // Type of the vector w
		  ,
		  bool TF> // Transpose flag of the vector w
inline auto eigen_backend(const DenseMatrix<MT, SO> &A, DenseVector<VT, TF> &w)
	-> EnableIf_t<IsHermitian_v<MT> && IsComplex_v<ElementType_t<MT>>>
{
	using ATmp = RemoveAdaptor_t<ResultType_t<MT>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<ATmp>);

	METRIC_NUMERIC_INTERNAL_ASSERT(isSquare(*A), "Non-square matrix detected");

	ATmp Atmp(*A);

	heevd(Atmp, *w, 'N', 'L');
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense triangular matrix.
// \ingroup dense_matrix
//
// \param A The given triangular matrix.
// \param w The resulting vector of eigenvalues.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense triangular matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template <typename MT // Type of the matrix A
		  ,
		  bool SO // Storage order of the matrix A
		  ,
		  typename VT // Type of the vector w
		  ,
		  bool TF> // Transpose flag of the vector w
inline auto eigen_backend(const DenseMatrix<MT, SO> &A, DenseVector<VT, TF> &w) -> EnableIf_t<IsTriangular_v<MT>>
{
	METRIC_NUMERIC_INTERNAL_ASSERT(isSquare(*A), "Non-square matrix detected");

	const size_t N((*A).rows());

	CompositeType_t<MT> Atmp(*A);

	resize(*w, N, false);

	for (size_t i = 0UL; i < N; ++i) {
		(*w)[i] = Atmp(i, i);
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense general matrix.
// \ingroup dense_matrix
//
// \param A The given general matrix.
// \param w The resulting vector of eigenvalues.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense general matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template <typename MT // Type of the matrix A
		  ,
		  bool SO // Storage order of the matrix A
		  ,
		  typename VT // Type of the vector w
		  ,
		  bool TF> // Transpose flag of the vector w
inline auto eigen_backend(const DenseMatrix<MT, SO> &A, DenseVector<VT, TF> &w)
	-> DisableIf_t<(IsSymmetric_v<MT> && !IsDiagonal_v<MT> && IsFloatingPoint_v<ElementType_t<MT>>) ||
				   (IsHermitian_v<MT> && IsComplex_v<ElementType_t<MT>>) || (IsTriangular_v<MT>)>
{
	using ATmp = RemoveAdaptor_t<ResultType_t<MT>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<ATmp>);

	METRIC_NUMERIC_INTERNAL_ASSERT(isSquare(*A), "Non-square matrix detected");

	ATmp Atmp(*A);

	geev(Atmp, *w);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Eigenvalue computation of the given dense matrix.
// \ingroup dense_matrix
//
// \param A The given general matrix.
// \param w The resulting vector of eigenvalues.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function computes the eigenvalues of the given \a n-by-\a n matrix. The eigenvalues are
// returned in the given vector \a w, which is resized to the correct size (if possible and
// necessary).
//
// Please note that in case the given matrix is either a compile time symmetric matrix with
// floating point elements or an Hermitian matrix with complex elements, the resulting eigenvalues
// will be of floating point type and therefore the elements of the given eigenvalue vector are
// expected to be of floating point type. In all other cases they are expected to be of complex
// type. Also please note that for complex eigenvalues no order of eigenvalues can be assumed,
// except that complex conjugate pairs of eigenvalues appear consecutively with the eigenvalue
// having the positive imaginary part first.
//
// The function fails if ...
//
//  - ... the given matrix \a A is not a square matrix;
//  - ... the given vector \a w is a fixed size vector and the size doesn't match;
//  - ... the eigenvalue computation fails.
//
// In all failure cases an exception is thrown.
//
// Examples:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   DynamicMatrix<double,rowMajor> A( 5UL, 5UL );  // The general matrix A
   // ... Initialization

   DynamicVector<complex<double>,columnVector> w( 5UL );  // The vector for the complex eigenvalues

   eigen( A, w );
   \endcode

   \code
   using mtrc::numeric::SymmetricMatrix;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   SymmetricMatrix< DynamicMatrix<double,rowMajor> > A( 5UL );  // The symmetric matrix A
   // ... Initialization

   DynamicVector<double,columnVector> w( 5UL );  // The vector for the real eigenvalues

   eigen( A, w );
   \endcode

   \code
   using mtrc::numeric::HermitianMatrix;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   HermitianMatrix< DynamicMatrix<complex<double>,rowMajor> > A( 5UL );  // The Hermitian matrix A
   // ... Initialization

   DynamicVector<double,columnVector> w( 5UL );  // The vector for the real eigenvalues

   eigen( A, w );
   \endcode

// \note This function only works for matrices with \c float, \c double, \c complex<float>, or
// \c complex<double> element type. The attempt to call the function with matrices of any other
// element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a call to this function will result in a linker error.
//
// \note Further options for computing eigenvalues and eigenvectors are available via the geev(),
// syev(), syevd(), syevx(), heev(), heevd(), and heevx() functions.
*/
template <typename MT // Type of the matrix A
		  ,
		  bool SO // Storage order of the matrix A
		  ,
		  typename VT // Type of the vector w
		  ,
		  bool TF> // Transpose flag of the vector w
inline void eigen(const DenseMatrix<MT, SO> &A, DenseVector<VT, TF> &w)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<VT>);

	if (!isSquare(*A)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	using WTmp = If_t<IsContiguous_v<VT>, VT &, ResultType_t<VT>>;
	WTmp wtmp(*w);

	eigen_backend(*A, wtmp);

	if (IsContiguous_v<VT>) {
		(*w) = wtmp;
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense symmetric matrix.
// \ingroup dense_matrix
//
// \param A The given symmetric matrix.
// \param w The resulting vector of eigenvalues.
// \param V The resulting matrix of eigenvectors.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Matrix cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense symmetric matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template <typename MT1 // Type of the matrix A
		  ,
		  bool SO1 // Storage order of the matrix A
		  ,
		  typename VT // Type of the vector w
		  ,
		  bool TF // Transpose flag of the vector w
		  ,
		  typename MT2 // Type of the matrix V
		  ,
		  bool SO2> // Storage order of the matrix V
inline auto eigen_backend(const DenseMatrix<MT1, SO1> &A, DenseVector<VT, TF> &w, DenseMatrix<MT2, SO2> &V)
	-> EnableIf_t<IsSymmetric_v<MT1> && !IsDiagonal_v<MT1> && IsFloatingPoint_v<ElementType_t<MT1>>>
{
	using ATmp = RemoveAdaptor_t<ResultType_t<MT1>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<ATmp>);

	METRIC_NUMERIC_INTERNAL_ASSERT(isSquare(*A), "Non-square matrix detected");

	ATmp Atmp(*A);

	syevd(Atmp, *w, 'V', 'L');

	if (SO1 == SO2)
		(*V) = Atmp;
	else
		(*V) = trans(Atmp);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense Hermitian matrix.
// \ingroup dense_matrix
//
// \param A The given Hermitian matrix.
// \param w The resulting vector of eigenvalues.
// \param V The resulting matrix of eigenvectors.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Matrix cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense Hermitian matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template <typename MT1 // Type of the matrix A
		  ,
		  bool SO1 // Storage order of the matrix A
		  ,
		  typename VT // Type of the vector w
		  ,
		  bool TF // Transpose flag of the vector w
		  ,
		  typename MT2 // Type of the matrix V
		  ,
		  bool SO2> // Storage order of the matrix V
inline auto eigen_backend(const DenseMatrix<MT1, SO1> &A, DenseVector<VT, TF> &w, DenseMatrix<MT2, SO2> &V)
	-> EnableIf_t<IsHermitian_v<MT1> && IsComplex_v<ElementType_t<MT1>>>
{
	using ATmp = RemoveAdaptor_t<ResultType_t<MT1>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<ATmp>);

	METRIC_NUMERIC_INTERNAL_ASSERT(isSquare(*A), "Non-square matrix detected");

	ATmp Atmp(*A);

	heevd(Atmp, *w, 'V', 'L');

	if (SO1 == SO2)
		(*V) = Atmp;
	else
		(*V) = trans(Atmp);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense diagonal matrix.
// \ingroup dense_matrix
//
// \param A The given diagonal matrix.
// \param w The resulting vector of eigenvalues.
// \param V The resulting matrix of eigenvectors.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Matrix cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense diagonal matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template <typename MT1 // Type of the matrix A
		  ,
		  bool SO1 // Storage order of the matrix A
		  ,
		  typename VT // Type of the vector w
		  ,
		  bool TF // Transpose flag of the vector w
		  ,
		  typename MT2 // Type of the matrix V
		  ,
		  bool SO2> // Storage order of the matrix V
inline auto eigen_backend(const DenseMatrix<MT1, SO1> &A, DenseVector<VT, TF> &w, DenseMatrix<MT2, SO2> &V)
	-> EnableIf_t<IsDiagonal_v<MT1>>
{
	METRIC_NUMERIC_INTERNAL_ASSERT(isSquare(*A), "Non-square matrix detected");

	const size_t N((*A).rows());

	CompositeType_t<MT1> Atmp(*A);

	resize(*w, N, false);
	resize(*V, N, N, false);
	reset(*V);

	for (size_t i = 0UL; i < N; ++i) {
		(*w)[i] = Atmp(i, i);
		(*V)(i, i) = ElementType_t<MT2>(1);
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend for the eigenvalue computation of the given dense general matrix.
// \ingroup dense_matrix
//
// \param A The given general matrix.
// \param w The resulting vector of eigenvalues.
// \param V The resulting matrix of eigenvectors.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Matrix cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function is the backend implementation for computing the eigenvalues of the given
// dense general matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the dispatch to
// the correct LAPACK function. Calling this function explicitly might result in erroneous
// results and/or in compilation errors. Instead of using this function use the according
// eigen() function.
*/
template <typename MT1 // Type of the matrix A
		  ,
		  bool SO1 // Storage order of the matrix A
		  ,
		  typename VT // Type of the vector w
		  ,
		  bool TF // Transpose flag of the vector w
		  ,
		  typename MT2 // Type of the matrix V
		  ,
		  bool SO2> // Storage order of the matrix V
inline auto eigen_backend(const DenseMatrix<MT1, SO1> &A, DenseVector<VT, TF> &w, DenseMatrix<MT2, SO2> &V)
	-> DisableIf_t<(IsSymmetric_v<MT1> && !IsDiagonal_v<MT1> && IsFloatingPoint_v<ElementType_t<MT1>>) ||
				   (IsHermitian_v<MT1> && IsComplex_v<ElementType_t<MT1>>) || (IsDiagonal_v<MT1>)>
{
	using ATmp = RemoveAdaptor_t<ResultType_t<MT1>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(ATmp);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<ATmp>);

	METRIC_NUMERIC_INTERNAL_ASSERT(isSquare(*A), "Non-square matrix detected");

	ATmp Atmp(*A);

	if (IsRowMajorMatrix_v<MT1>)
		geev(Atmp, *V, *w);
	else
		geev(Atmp, *w, *V);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Eigenvalue computation of the given dense matrix.
// \ingroup dense_matrix
//
// \param A The given general matrix.
// \param w The resulting vector of eigenvalues.
// \param V The resulting matrix of eigenvectors.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Vector cannot be resized.
// \exception std::invalid_argument Matrix cannot be resized.
// \exception std::runtime_error Eigenvalue computation failed.
//
// This function computes the eigenvalues and eigenvectors of the given \a n-by-\a n matrix.
// The eigenvalues are returned in the given vector \a w and the eigenvectors are returned in the
// given matrix \a V, which are both resized to the correct dimensions (if possible and necessary).
//
// Please note that in case the given matrix is either a compile time symmetric matrix with
// floating point elements or an Hermitian matrix with complex elements, the resulting eigenvalues
// will be of floating point type and therefore the elements of the given eigenvalue vector are
// expected to be of floating point type. In all other cases they are expected to be of complex
// type. Also please note that for complex eigenvalues no order of eigenvalues can be assumed,
// except that complex conjugate pairs of eigenvalues appear consecutively with the eigenvalue
// having the positive imaginary part first.
//
// In case \a A is a row-major matrix, \a V will contain the left eigenvectors, otherwise \a V
// will contain the right eigenvectors. In case \a V is a row-major matrix the eigenvectors are
// returned in the rows of \a V, in case \a V is a column-major matrix the eigenvectors are
// returned in the columns of \a V. In case the given matrix is a compile time symmetric matrix
// with floating point elements, the resulting eigenvectors will be of floating point type and
// therefore the elements of the given eigenvector matrix are expected to be of floating point
// type. In all other cases they are expected to be of complex type.
//
// The function fails if ...
//
//  - ... the given matrix \a A is not a square matrix;
//  - ... the given vector \a w is a fixed size vector and the size doesn't match;
//  - ... the given matrix \a V is a fixed size matrix and the dimensions don't match;
//  - ... the eigenvalue computation fails.
//
// In all failure cases an exception is thrown.
//
// Examples:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   DynamicMatrix<double,rowMajor> A( 5UL, 5UL );  // The general matrix A
   // ... Initialization

   DynamicVector<complex<double>,columnVector> w( 5UL );   // The vector for the complex eigenvalues
   DynamicMatrix<complex<double>,rowMajor> V( 5UL, 5UL );  // The matrix for the left eigenvectors

   eigen( A, w, V );
   \endcode

   \code
   using mtrc::numeric::SymmetricMatrix;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   SymmetricMatrix< DynamicMatrix<double,rowMajor> > A( 5UL );  // The symmetric matrix A
   // ... Initialization

   DynamicVector<double,columnVector> w( 5UL );       // The vector for the real eigenvalues
   DynamicMatrix<double,rowMajor>     V( 5UL, 5UL );  // The matrix for the left eigenvectors

   eigen( A, w, V );
   \endcode

   \code
   using mtrc::numeric::HermitianMatrix;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   HermitianMatrix< DynamicMatrix<complex<double>,rowMajor> > A( 5UL );  // The Hermitian matrix A
   // ... Initialization

   DynamicVector<double,columnVector>      w( 5UL );       // The vector for the real eigenvalues
   DynamicMatrix<complex<double>,rowMajor> V( 5UL, 5UL );  // The matrix for the left eigenvectors

   eigen( A, w, V );
   \endcode

// \note This function only works for matrices with \c float, \c double, \c complex<float>, or
// \c complex<double> element type. The attempt to call the function with matrices of any other
// element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a call to this function will result in a linker error.
//
// \note Further options for computing eigenvalues and eigenvectors are available via the geev(),
// syev(), syevd(), syevx(), heev(), heevd(), and heevx() functions.
*/
template <typename MT1 // Type of the matrix A
		  ,
		  bool SO1 // Storage order of the matrix A
		  ,
		  typename VT // Type of the vector w
		  ,
		  bool TF // Transpose flag of the vector w
		  ,
		  typename MT2 // Type of the matrix V
		  ,
		  bool SO2> // Storage order of the matrix V
inline void eigen(const DenseMatrix<MT1, SO1> &A, DenseVector<VT, TF> &w, DenseMatrix<MT2, SO2> &V)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<VT>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT2>);

	using WTmp = If_t<IsContiguous_v<VT>, VT &, ResultType_t<VT>>;
	using VTmp = If_t<IsContiguous_v<MT2>, MT2 &, ResultType_t<MT2>>;

	if (!isSquare(*A)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	WTmp wtmp(*w);
	VTmp Vtmp(*V);

	eigen_backend(*A, wtmp, Vtmp);

	if (!IsContiguous_v<VT>) {
		(*w) = wtmp;
	}

	if (!IsContiguous_v<MT2>) {
		(*V) = Vtmp;
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
