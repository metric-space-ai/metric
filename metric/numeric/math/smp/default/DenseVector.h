// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SMP_DEFAULT_DENSEVECTOR_H
#define METRIC_NUMERIC_MATH_SMP_DEFAULT_DENSEVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Vector.h>
#include <metric/numeric/math/typetraits/IsDenseVector.h>
#include <metric/numeric/system/MacroDisable.h>
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
/*!\name Dense vector SMP functions */
//@{
template <typename VT1, bool TF1, typename VT2, bool TF2>
auto smpAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsDenseVector_v<VT1>>;

template <typename VT1, bool TF1, typename VT2, bool TF2>
auto smpAddAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsDenseVector_v<VT1>>;

template <typename VT1, bool TF1, typename VT2, bool TF2>
auto smpSubAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsDenseVector_v<VT1>>;

template <typename VT1, bool TF1, typename VT2, bool TF2>
auto smpMultAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsDenseVector_v<VT1>>;

template <typename VT1, bool TF1, typename VT2, bool TF2>
auto smpDivAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsDenseVector_v<VT1>>;
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP assignment of a vector to a dense vector.
// \ingroup smp
//
// \param lhs The target left-hand side dense vector.
// \param rhs The right-hand side vector to be assigned.
// \return void
//
// This function implements the default SMP assignment of a vector to a dense vector.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side dense vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline auto smpAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsDenseVector_v<VT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == (*rhs).size(), "Invalid vector sizes");
	assign(*lhs, *rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP addition assignment of a vector to a dense vector.
// \ingroup smp
//
// \param lhs The target left-hand side dense vector.
// \param rhs The right-hand side vector to be added.
// \return void
//
// This function implements the default SMP addition assignment of a vector to a dense vector.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side dense vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline auto smpAddAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsDenseVector_v<VT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == (*rhs).size(), "Invalid vector sizes");
	addAssign(*lhs, *rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP subtraction assignment of a vector to a dense vector.
// \ingroup smp
//
// \param lhs The target left-hand side dense vector.
// \param rhs The right-hand side vector to be subtracted.
// \return void
//
// This function implements the default SMP subtraction assignment of a vector to a dense vector.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side dense vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline auto smpSubAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsDenseVector_v<VT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == (*rhs).size(), "Invalid vector sizes");
	subAssign(*lhs, *rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP multiplication assignment of a vector to a dense vector.
// \ingroup smp
//
// \param lhs The target left-hand side dense vector.
// \param rhs The right-hand side vector to be multiplied.
// \return void
//
// This function implements the default SMP multiplication assignment of a vector to a dense vector.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side dense vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline auto smpMultAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsDenseVector_v<VT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == (*rhs).size(), "Invalid vector sizes");
	multAssign(*lhs, *rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP division assignment of a vector to a dense vector.
// \ingroup smp
//
// \param lhs The target left-hand side dense vector.
// \param rhs The right-hand side vector divisor.
// \return void
//
// This function implements the default SMP division assignment of a vector to a dense vector.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the left-hand side dense vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side dense vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline auto smpDivAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsDenseVector_v<VT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == (*rhs).size(), "Invalid vector sizes");
	divAssign(*lhs, *rhs);
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
