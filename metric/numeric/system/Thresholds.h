// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_THRESHOLDS_H
#define METRIC_NUMERIC_SYSTEM_THRESHOLDS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Debugging.h>
#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/Types.h>

//=================================================================================================
//
//  THRESHOLDS
//
//=================================================================================================

#include <metric/numeric/config/Thresholds.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  BLAS THRESHOLDS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Row-major dense matrix/dense vector multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_DMATDVECMULT_THRESHOLD while the Metric numeric debug
// mode is active. It specifies the threshold between the application of the custom Metric numeric kernels
// and the BLAS kernels for the row-major dense matrix/dense vector multiplication. In case the
// number of elements in the dense matrix is equal or higher than this value, the BLAS kernels
// are preferred over the custom Metric numeric kernels. In case the number of elements in the dense
// matrix is smaller, the Metric numeric kernels are used.
*/
constexpr size_t DMATDVECMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Column-major dense matrix/dense vector multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_TDMATDVECMULT_THRESHOLD while the Metric numeric debug
// mode is active. It specifies the threshold between the application of the custom Metric numeric kernels
// and the BLAS kernels for the column-major dense matrix/dense vector multiplication. In case
// the number of elements in the dense matrix is equal or higher than this value, the BLAS
// kernels are preferred over the custom Metric numeric kernels. In case the number of elements in the
// dense matrix is smaller, the Metric numeric kernels are used.
*/
constexpr size_t TDMATDVECMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Dense Vector/row-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_TDVECDMATMULT_THRESHOLD while the Metric numeric debug
// mode is active. It specifies the threshold between the application of the custom Metric numeric kernels
// and the BLAS kernels for the dense vector/row-major dense matrix multiplication. In case the
// number of elements in the dense matrix is equal or higher than this value, the BLAS kernels
// are preferred over the custom Metric numeric kernels. In case the number of elements in the dense
// matrix is smaller, the Metric numeric kernels are used.
*/
constexpr size_t TDVECDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Dense Vector/column-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_TDVECTDMATMULT_THRESHOLD while the Metric numeric debug
// mode is active. It specifies the threshold between the application of the custom Metric numeric kernels
// and the BLAS kernels for the dense vector/column-major dense matrix multiplication. In case
// the number of elements in the dense matrix is equal or higher than this value, the BLAS
// kernels are preferred over the custom Metric numeric kernels. In case the number of elements in the
// dense matrix is smaller, the Metric numeric kernels are used.
*/
constexpr size_t TDVECTDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Row-major dense matrix/row-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_DMATDMATMULT_THRESHOLD while the Metric numeric debug
// mode is active. It specifies the threshold between the application of the custom Metric numeric kernels
// and the BLAS kernels for the row-major dense matrix/row-major dense matrix multiplication. In
// case the number of elements in the dense matrix is equal or higher than this value, the BLAS
// kernels are preferred over the custom Metric numeric kernels. In case the number of elements in the
// dense matrix is smaller, the Metric numeric kernels are used.
*/
constexpr size_t DMATDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Row-major dense matrix/column-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_DMATTDMATMULT_THRESHOLD while the Metric numeric debug
// mode is active. It specifies the threshold between the application of the custom Metric numeric kernels
// and the BLAS kernels for the row-major dense matrix/column-major dense matrix multiplication.
// In case the number of elements in the dense matrix is equal or higher than this value, the
// BLAS kernels are preferred over the custom Metric numeric kernels. In case the number of elements in
// the dense matrix is smaller, the Metric numeric kernels are used.
*/
constexpr size_t DMATTDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Column-major dense matrix/row-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_TDMATDMATMULT_THRESHOLD while the Metric numeric debug
// mode is active. It specifies the threshold between the application of the custom Metric numeric kernels
// and the BLAS kernels for the column-major dense matrix/row-major dense matrix multiplication.
// In case the number of elements in the dense matrix is equal or higher than this value, the
// BLAS kernels are preferred over the custom Metric numeric kernels. In case the number of elements in
// the dense matrix is smaller, the Metric numeric kernels are used.
*/
constexpr size_t TDMATDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Column-major dense matrix/column-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_TDMATTDMATMULT_THRESHOLD while the Metric numeric debug
// mode is active. It specifies the threshold between the application of the custom Metric numeric kernels
// and the BLAS kernels for the column-major dense matrix/column-major dense matrix multiplication.
// In case the number of elements in the dense matrix is equal or higher than this value, the BLAS
// kernels are preferred over the custom Metric numeric kernels. In case the number of elements in the
// dense matrix is smaller, the Metric numeric kernels are used.
*/
constexpr size_t TDMATTDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Row-major dense matrix/row-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_DMATSMATMULT_THRESHOLD while the Metric numeric debug
// mode is active. It specifies the threshold between the application of the Metric numeric kernels for
// small and for large row-major dense matrix/row-major sparse matrix multiplications. In case
// the number of elements of the target matrix is equal or higher than this value, the kernel for
// large matrices is preferred over the kernel for small matrices. In case the number of elements
// in the target matrix is smaller, the kernel for small matrices is used.
*/
constexpr size_t DMATSMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Column-major dense matrix/row-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_DMATSMATMULT_THRESHOLD while the Metric numeric debug
// mode is active. It specifies the threshold between the application of the Metric numeric kernels for
// small and for large column-major dense matrix/row-major sparse matrix multiplications. In
// case the number of elements of the target matrix is equal or higher than this value, the
// kernel for large matrices is preferred over the kernel for small matrices. In case the number
// of elements in the target matrix is smaller, the kernel for small matrices is used.
*/
constexpr size_t TDMATSMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Column-major sparse matrix/row-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_TSMATDMATMULT_THRESHOLD while the Metric numeric debug
// mode is active. It specifies the threshold between the application of the Metric numeric kernels for
// small and for large column-major sparse matrix/row-major dense matrix multiplications. In
// case the number of elements of the target matrix is equal or higher than this value, the
// kernel for large matrices is preferred over the kernel for small matrices. In case the number
// of elements in the target matrix is smaller, the kernel for small matrices is used.
*/
constexpr size_t TSMATDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Column-major sparse matrix/column-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_TSMATTDMATMULT_THRESHOLD while the Metric numeric debug
// mode is active. It specifies the threshold between the application of the Metric numeric kernels for
// small and for large column-major sparse matrix/column-major dense matrix multiplications. In
// case the number of elements of the target matrix is equal or higher than this value, the
// kernel for large matrices is preferred over the kernel for small matrices. In case the number
// of elements in the target matrix is smaller, the kernel for small matrices is used.
*/
constexpr size_t TSMATTDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
constexpr size_t DMATDVECMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? DMATDVECMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_DMATDVECMULT_THRESHOLD);
constexpr size_t TDMATDVECMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? TDMATDVECMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_TDMATDVECMULT_THRESHOLD);
constexpr size_t TDVECDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? TDVECDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_TDVECDMATMULT_THRESHOLD);
constexpr size_t TDVECTDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? TDVECTDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_TDVECTDMATMULT_THRESHOLD);
constexpr size_t DMATDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? DMATDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_DMATDMATMULT_THRESHOLD);
constexpr size_t DMATTDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? DMATTDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_DMATTDMATMULT_THRESHOLD);
constexpr size_t TDMATDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? TDMATDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_TDMATDMATMULT_THRESHOLD);
constexpr size_t TDMATTDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? TDMATTDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_TDMATTDMATMULT_THRESHOLD);
constexpr size_t DMATSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? DMATSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_DMATSMATMULT_THRESHOLD);
constexpr size_t TDMATSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? TDMATSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_TDMATSMATMULT_THRESHOLD);
constexpr size_t TSMATDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? TSMATDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_TSMATDMATMULT_THRESHOLD);
constexpr size_t TSMATTDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? TSMATTDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_TSMATTDMATMULT_THRESHOLD);
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SMP THRESHOLDS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief SMP dense vector assignment threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DVECASSIGN_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when an assignment of a simple dense vector can be executed
// in parallel. In case the number of elements of the target vector is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DVECASSIGN_DEBUG_THRESHOLD = 32UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/scalar multiplication/division threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DVECSCALARMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a dense vector/scalar multiplication/division can be
// executed in parallel. In case the number of elements of the target vector is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DVECSCALARMULT_DEBUG_THRESHOLD = 32UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/dense vector addition threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DVECDVECADD_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a dense vector/dense vector addition can be executed
// in parallel. In case the number of elements of the target vector is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DVECDVECADD_DEBUG_THRESHOLD = 32UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/dense vector subtraction threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DVECDVECSUB_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a dense vector/dense vector subtraction can be executed
// in parallel. In case the number of elements of the target vector is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DVECDVECSUB_DEBUG_THRESHOLD = 32UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/dense vector multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DVECDVECMULT_THRESHOLD while the
// Metric numeric debug mode is active. It specifies when a dense vector/dense vector multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger
// or equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DVECDVECMULT_DEBUG_THRESHOLD = 32UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/dense vector division threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DVECDVECDIV_THRESHOLD while the
// Metric numeric debug mode is active. It specifies when a dense vector/dense vector division can be
// executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements
// is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DVECDVECDIV_DEBUG_THRESHOLD = 32UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/dense vector outer product threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DVECDVECOUTER_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a dense vector/dense vector outer product can be executed
// in parallel. In case the number of elements of the target matrix is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DVECDVECOUTER_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/dense vector multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATDVECMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major dense matrix/dense vector multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DMATDVECMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major dense matrix/dense vector multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TDMATDVECMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a column-major dense matrix/dense vector multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TDMATDVECMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/row-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TDVECDMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a dense vector/row-major dense matrix multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TDVECDMATMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/column-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TDVECTDMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a dense vector/column-major dense matrix multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TDVECTDMATMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/sparse vector multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATSVECMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major dense matrix/sparse vector multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DMATSVECMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major dense matrix/sparse vector multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TDMATSVECMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a column-major dense matrix/sparse vector multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TDMATSVECMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP sparse vector/row-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TSVECDMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a sparse vector/row-major dense matrix multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TSVECDMATMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP sparse vector/column-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TSVECTDMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a sparse vector/column-major dense matrix multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TSVECTDMATMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major sparse matrix/dense vector multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_SMATDVECMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major sparse matrix/dense vector multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_SMATDVECMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major sparse matrix/dense vector multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TSMATDVECMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a column-major sparse matrix/dense vector multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TSMATDVECMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/row-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TDVECSMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a dense vector/row-major sparse matrix multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TDVECSMATMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense vector/column-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TDVECTSMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a dense vector/column-major sparse matrix multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TDVECTSMATMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major sparse matrix/sparse vector multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_SMATSVECMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major sparse matrix/sparse vector multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_SMATSVECMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major sparse matrix/sparse vector multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TSMATSVECMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a column-major sparse matrix/sparse vector multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TSMATSVECMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP sparse vector/row-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TSVECSMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a sparse vector/row-major sparse matrix multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TSVECSMATMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP sparse vector/column-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TSVECTSMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a sparse vector/column-major sparse matrix multiplication
// can be executed in parallel. In case the number of elements of the target vector is larger or
// equal to this threshold, the operation is executed in parallel. If the number of elements is
// below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TSVECTSMATMULT_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense matrix assignment threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATASSIGN_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when an assignment with a simple dense matrix can be executed
// in parallel. In case the number of elements of the target matrix is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DMATASSIGN_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense matrix/scalar multiplication/division threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATSCALARMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a dense matrix/scalar multiplication or division can be
// executed in parallel. In case the number of elements of the target matrix is larger or equal
// to this threshold, the operation is executed in parallel. If the number of elements is below
// this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DMATSCALARMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/row-major dense matrix addition threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATDMATADD_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major dense matrix/row-major dense matrix addition
// can be executed in parallel. This threshold affects both additions between two row-major matrices
// or two column-major dense matrices. In case the number of elements of the target matrix is
// larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DMATDMATADD_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/column-major dense matrix addition threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATTDMATADD_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major dense matrix/column-major dense matrix
// addition can be executed in parallel. This threshold affects both additions between a row-major
// matrix and a column-major matrix and a column-major matrix and a row-major matrix. In case the
// number of elements of the target matrix is larger or equal to this threshold, the operation
// is executed in parallel. If the number of elements is below this threshold the operation is
// executed single-threaded.
*/
constexpr size_t SMP_DMATTDMATADD_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/row-major dense matrix subtraction threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATDMATSUB_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major dense matrix/row-major dense matrix
// subtraction can be executed in parallel. This threshold affects both subtractions between two
// row-major matrices or two column-major dense matrices. In case the number of elements of
// the target matrix is larger or equal to this threshold, the operation is executed in parallel.
// If the number of elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DMATDMATSUB_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/column-major dense matrix subtraction threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATTDMATSUB_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major dense matrix/column-major dense matrix
// subtraction can be executed in parallel. This threshold affects both subtractions between a
// row-major matrix and a column-major matrix and a column-major matrix and a row-major matrix.
// In case the number of elements of the target matrix is larger or equal to this threshold, the
// operation is executed in parallel. If the number of elements is below this threshold the
// operation is executed single-threaded.
*/
constexpr size_t SMP_DMATTDMATSUB_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/row-major dense matrix Schur product threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATDMATSCHUR_THRESHOLD while the
// Metric numeric debug mode is active. It specifies when a row-major dense matrix/row-major dense matrix
// Schur product can be executed in parallel. This threshold affects both Schur products between
// two row-major matrices or two column-major dense matrices. In case the number of elements of
// the target matrix is larger or equal to this threshold, the operation is executed in parallel.
// If the number of elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DMATDMATSCHUR_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/column-major dense matrix Schur product threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATTDMATSCHUR_THRESHOLD while the
// Metric numeric debug mode is active. It specifies when a row-major dense matrix/column-major dense
// matrix Schur product can be executed in parallel. This threshold affects both Schur products
// between a row-major matrix and a column-major matrix and a column-major matrix and a row-major
// matrix. In case the number of elements of the target matrix is larger or equal to this
// threshold, the operation is executed in parallel. If the number of elements is below this
// threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DMATTDMATSCHUR_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/row-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATDMATMULT_THRESHOLD while the
// Metric numeric debug mode is active. It specifies when a row-major dense matrix/row-major dense matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DMATDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/column-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATTDMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major dense matrix/column-major dense matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DMATTDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major dense matrix/row-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TDMATDMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a column-major dense matrix/row-major dense matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TDMATDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major dense matrix/column-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TDMATTDMATMULT_THRESHOLD while the
// Metric numeric debug mode is active. It specifies when a column-major dense matrix/column-major dense
// matrix multiplication can be executed in parallel. In case the number of elements of the target
// matrix is larger or equal to this threshold, the operation is executed in parallel. If the
// number of elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TDMATTDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/row-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATSMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major dense matrix/row-major sparse matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DMATSMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major dense matrix/column-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATTSMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major dense matrix/column-major sparse matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_DMATTSMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major dense matrix/row-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TDMATSMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a column-major dense matrix/row-major sparse matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TDMATSMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major dense matrix/column-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TDMATTSMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a column-major dense matrix/column-major sparse matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TDMATTSMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major sparse matrix/row-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_SMATDMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major sparse matrix/row-major dense matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_SMATDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major sparse matrix/column-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_SMATTDMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major sparse matrix/column-major dense matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_SMATTDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major sparse matrix/row-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TSMATDMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a column-major sparse matrix/row-major dense matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TSMATDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major sparse matrix/column-major dense matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TSMATTDMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a column-major sparse matrix/column-major dense matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TSMATTDMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major sparse matrix/row-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_SMATSMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major sparse matrix/row-major sparse matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_SMATSMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP row-major sparse matrix/column-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_SMATTSMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a row-major sparse matrix/column-major sparse matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_SMATTSMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major sparse matrix/row-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TSMATSMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a column-major sparse matrix/row-major sparse matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TSMATSMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP column-major sparse matrix/column-major sparse matrix multiplication threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_TSMATTSMATMULT_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a column-major sparse matrix/column-major sparse matrix
// multiplication can be executed in parallel. In case the number of elements of the target matrix
// is larger or equal to this threshold, the operation is executed in parallel. If the number of
// elements is below this threshold the operation is executed single-threaded.
*/
constexpr size_t SMP_TSMATTSMATMULT_DEBUG_THRESHOLD = 256UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP dense matrix reduction threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_DMATREDUCE_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a dense matrix reduction can be executed in parallel.
// In case the number of elements of the target vector is larger or equal to this threshold,
// the operation is executed in parallel. If the number of elements is below this threshold the
// operation is executed single-threaded.
*/
constexpr size_t SMP_DMATREDUCE_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SMP sparse matrix reduction threshold.
// \ingroup system
//
// This debug value is used instead of the METRIC_NUMERIC_SMP_SMATREDUCE_THRESHOLD while the Metric numeric
// debug mode is active. It specifies when a sparse matrix reduction can be executed in parallel.
// In case the number of elements of the target vector is larger or equal to this threshold,
// the operation is executed in parallel. If the number of elements is below this threshold the
// operation is executed single-threaded.
*/
constexpr size_t SMP_SMATREDUCE_DEBUG_THRESHOLD = 16UL;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
constexpr size_t SMP_DVECASSIGN_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DVECASSIGN_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DVECASSIGN_THRESHOLD);
constexpr size_t SMP_DVECSCALARMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DVECSCALARMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DVECSCALARMULT_THRESHOLD);
constexpr size_t SMP_DVECDVECADD_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DVECDVECADD_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DVECDVECADD_THRESHOLD);
constexpr size_t SMP_DVECDVECSUB_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DVECDVECSUB_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DVECDVECSUB_THRESHOLD);
constexpr size_t SMP_DVECDVECMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DVECDVECMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DVECDVECMULT_THRESHOLD);
constexpr size_t SMP_DVECDVECDIV_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DVECDVECDIV_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DVECDVECDIV_THRESHOLD);
constexpr size_t SMP_DVECDVECOUTER_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DVECDVECOUTER_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DVECDVECOUTER_THRESHOLD);
constexpr size_t SMP_DMATDVECMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATDVECMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATDVECMULT_THRESHOLD);
constexpr size_t SMP_TDMATDVECMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TDMATDVECMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TDMATDVECMULT_THRESHOLD);
constexpr size_t SMP_TDVECDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TDVECDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TDVECDMATMULT_THRESHOLD);
constexpr size_t SMP_TDVECTDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TDVECTDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TDVECTDMATMULT_THRESHOLD);
constexpr size_t SMP_DMATSVECMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATSVECMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATSVECMULT_THRESHOLD);
constexpr size_t SMP_TDMATSVECMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TDMATSVECMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TDMATSVECMULT_THRESHOLD);
constexpr size_t SMP_TSVECDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TSVECDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TSVECDMATMULT_THRESHOLD);
constexpr size_t SMP_TSVECTDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TSVECTDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TSVECTDMATMULT_THRESHOLD);
constexpr size_t SMP_SMATDVECMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_SMATDVECMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_SMATDVECMULT_THRESHOLD);
constexpr size_t SMP_TSMATDVECMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TSMATDVECMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TSMATDVECMULT_THRESHOLD);
constexpr size_t SMP_TDVECSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TDVECSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TDVECSMATMULT_THRESHOLD);
constexpr size_t SMP_TDVECTSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TDVECTSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TDVECTSMATMULT_THRESHOLD);
constexpr size_t SMP_SMATSVECMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_SMATSVECMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_SMATSVECMULT_THRESHOLD);
constexpr size_t SMP_TSMATSVECMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TSMATSVECMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TSMATSVECMULT_THRESHOLD);
constexpr size_t SMP_TSVECSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TSVECSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TSVECSMATMULT_THRESHOLD);
constexpr size_t SMP_TSVECTSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TSVECTSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TSVECTSMATMULT_THRESHOLD);
constexpr size_t SMP_DMATASSIGN_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATASSIGN_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATASSIGN_THRESHOLD);
constexpr size_t SMP_DMATSCALARMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATSCALARMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATSCALARMULT_THRESHOLD);
constexpr size_t SMP_DMATDMATADD_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATDMATADD_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATDMATADD_THRESHOLD);
constexpr size_t SMP_DMATTDMATADD_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATTDMATADD_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATTDMATADD_THRESHOLD);
constexpr size_t SMP_DMATDMATSUB_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATDMATSUB_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATDMATSUB_THRESHOLD);
constexpr size_t SMP_DMATTDMATSUB_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATTDMATSUB_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATTDMATSUB_THRESHOLD);
constexpr size_t SMP_DMATDMATSCHUR_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATDMATSCHUR_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATDMATSCHUR_THRESHOLD);
constexpr size_t SMP_DMATTDMATSCHUR_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATTDMATSCHUR_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATTDMATSCHUR_THRESHOLD);
constexpr size_t SMP_DMATDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATDMATMULT_THRESHOLD);
constexpr size_t SMP_DMATTDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATTDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATTDMATMULT_THRESHOLD);
constexpr size_t SMP_TDMATDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TDMATDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TDMATDMATMULT_THRESHOLD);
constexpr size_t SMP_TDMATTDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TDMATTDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TDMATTDMATMULT_THRESHOLD);
constexpr size_t SMP_DMATSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATSMATMULT_THRESHOLD);
constexpr size_t SMP_DMATTSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATTSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATTSMATMULT_THRESHOLD);
constexpr size_t SMP_TDMATSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TDMATSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TDMATSMATMULT_THRESHOLD);
constexpr size_t SMP_TDMATTSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TDMATTSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TDMATTSMATMULT_THRESHOLD);
constexpr size_t SMP_SMATDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_SMATDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_SMATDMATMULT_THRESHOLD);
constexpr size_t SMP_SMATTDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_SMATTDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_SMATTDMATMULT_THRESHOLD);
constexpr size_t SMP_TSMATDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TSMATDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TSMATDMATMULT_THRESHOLD);
constexpr size_t SMP_TSMATTDMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TSMATTDMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TSMATTDMATMULT_THRESHOLD);
constexpr size_t SMP_SMATSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_SMATSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_SMATSMATMULT_THRESHOLD);
constexpr size_t SMP_SMATTSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_SMATTSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_SMATTSMATMULT_THRESHOLD);
constexpr size_t SMP_TSMATSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TSMATSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TSMATSMATMULT_THRESHOLD);
constexpr size_t SMP_TSMATTSMATMULT_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_TSMATTSMATMULT_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_TSMATTSMATMULT_THRESHOLD);
constexpr size_t SMP_DMATREDUCE_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_DMATREDUCE_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_DMATREDUCE_THRESHOLD);
constexpr size_t SMP_SMATREDUCE_THRESHOLD =
	(METRIC_NUMERIC_DEBUG_MODE ? SMP_SMATREDUCE_DEBUG_THRESHOLD : METRIC_NUMERIC_SMP_SMATREDUCE_THRESHOLD);
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

//=================================================================================================
//
//  COMPILE TIME CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
namespace {

METRIC_NUMERIC_STATIC_ASSERT(mtrc::numeric::DMATDVECMULT_THRESHOLD > 0UL);
METRIC_NUMERIC_STATIC_ASSERT(mtrc::numeric::TDMATDVECMULT_THRESHOLD > 0UL);
METRIC_NUMERIC_STATIC_ASSERT(mtrc::numeric::TDVECDMATMULT_THRESHOLD > 0UL);
METRIC_NUMERIC_STATIC_ASSERT(mtrc::numeric::TDVECTDMATMULT_THRESHOLD > 0UL);
METRIC_NUMERIC_STATIC_ASSERT(mtrc::numeric::DMATDMATMULT_THRESHOLD > 0UL);
METRIC_NUMERIC_STATIC_ASSERT(mtrc::numeric::DMATTDMATMULT_THRESHOLD > 0UL);
METRIC_NUMERIC_STATIC_ASSERT(mtrc::numeric::TDMATDMATMULT_THRESHOLD > 0UL);
METRIC_NUMERIC_STATIC_ASSERT(mtrc::numeric::TDMATTDMATMULT_THRESHOLD > 0UL);

} // namespace
/*! \endcond */
//*************************************************************************************************

#endif
