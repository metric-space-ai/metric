// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SMP_DEFAULT_DENSEMATRIX_H
#define METRIC_NUMERIC_MATH_SMP_DEFAULT_DENSEMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Matrix.h>
#include <metric/numeric/math/typetraits/IsDenseMatrix.h>
#include <metric/numeric/system/SMP.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/StaticAssert.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name Dense matrix SMP functions */
//@{
template <typename MT1, bool SO1, typename MT2, bool SO2>
auto smpAssign(Matrix<MT1, SO1> &lhs, const Matrix<MT2, SO2> &rhs) -> EnableIf_t<IsDenseMatrix_v<MT1>>;

template <typename MT1, bool SO1, typename MT2, bool SO2>
auto smpAddAssign(Matrix<MT1, SO1> &lhs, const Matrix<MT2, SO2> &rhs) -> EnableIf_t<IsDenseMatrix_v<MT1>>;

template <typename MT1, bool SO1, typename MT2, bool SO2>
auto smpSubAssign(Matrix<MT1, SO1> &lhs, const Matrix<MT2, SO2> &rhs) -> EnableIf_t<IsDenseMatrix_v<MT1>>;

template <typename MT1, bool SO1, typename MT2, bool SO2>
auto smpSchurAssign(Matrix<MT1, SO1> &lhs, const Matrix<MT2, SO2> &rhs) -> EnableIf_t<IsDenseMatrix_v<MT1>>;
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP assignment of a matrix to a dense matrix.
// \ingroup smp
//
// \param lhs The target left-hand side dense matrix.
// \param rhs The right-hand side matrix to be assigned.
// \return void
//
// This function implements the default SMP assignment of a matrix to a dense matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  bool SO1 // Storage order of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline auto smpAssign(Matrix<MT1, SO1> &lhs, const Matrix<MT2, SO2> &rhs) -> EnableIf_t<IsDenseMatrix_v<MT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == (*rhs).rows(), "Invalid number of rows");
	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == (*rhs).columns(), "Invalid number of columns");

	assign(*lhs, *rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP addition assignment of a matrix to a dense matrix.
// \ingroup smp
//
// \param lhs The target left-hand side dense matrix.
// \param rhs The right-hand side matrix to be added.
// \return void
//
// This function implements the default SMP addition assignment of a matrix to a dense matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  bool SO1 // Storage order of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline auto smpAddAssign(Matrix<MT1, SO1> &lhs, const Matrix<MT2, SO2> &rhs) -> EnableIf_t<IsDenseMatrix_v<MT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == (*rhs).rows(), "Invalid number of rows");
	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == (*rhs).columns(), "Invalid number of columns");

	addAssign(*lhs, *rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP subtraction assignment of a matrix to dense matrix.
// \ingroup smp
//
// \param lhs The target left-hand side dense matrix.
// \param rhs The right-hand side matrix to be subtracted.
// \return void
//
// This function implements the default SMP subtraction assignment of a matrix to a dense matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  bool SO1 // Storage order of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline auto smpSubAssign(Matrix<MT1, SO1> &lhs, const Matrix<MT2, SO2> &rhs) -> EnableIf_t<IsDenseMatrix_v<MT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == (*rhs).rows(), "Invalid number of rows");
	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == (*rhs).columns(), "Invalid number of columns");

	subAssign(*lhs, *rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP Schur product assignment of a matrix to dense matrix.
// \ingroup smp
//
// \param lhs The target left-hand side dense matrix.
// \param rhs The right-hand side matrix for the Schur product.
// \return void
//
// This function implements the default SMP Schur product assignment of a matrix to a dense
// matrix.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  bool SO1 // Storage order of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline auto smpSchurAssign(Matrix<MT1, SO1> &lhs, const Matrix<MT2, SO2> &rhs) -> EnableIf_t<IsDenseMatrix_v<MT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == (*rhs).rows(), "Invalid number of rows");
	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == (*rhs).columns(), "Invalid number of columns");

	schurAssign(*lhs, *rhs);
}
//*************************************************************************************************

//=================================================================================================
//
//  COMPILE TIME CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
namespace {

METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_HPX_PARALLEL_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_CPP_THREADS_PARALLEL_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_BOOST_THREADS_PARALLEL_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_OPENMP_PARALLEL_MODE);

} // namespace
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
