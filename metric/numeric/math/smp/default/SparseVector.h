// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SMP_DEFAULT_SPARSEVECTOR_H
#define METRIC_NUMERIC_MATH_SMP_DEFAULT_SPARSEVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Vector.h>
#include <metric/numeric/math/typetraits/IsSparseVector.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/FunctionTrace.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name Sparse vector SMP functions */
//@{
template <typename VT1, bool TF1, typename VT2, bool TF2>
auto smpAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsSparseVector_v<VT1>>;

template <typename VT1, bool TF1, typename VT2, bool TF2>
auto smpAddAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsSparseVector_v<VT1>>;

template <typename VT1, bool TF1, typename VT2, bool TF2>
auto smpSubAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsSparseVector_v<VT1>>;

template <typename VT1, bool TF1, typename VT2, bool TF2>
auto smpMultAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsSparseVector_v<VT1>>;
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP assignment of a vector to a sparse vector.
// \ingroup smp
//
// \param lhs The target left-hand side sparse vector.
// \param rhs The right-hand side vector to be assigned.
// \return void
//
// This function implements the default SMP assignment of a vector to a sparse vector.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the left-hand side sparse vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side sparse vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline auto smpAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsSparseVector_v<VT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == (*rhs).size(), "Invalid vector sizes");
	assign(*lhs, *rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP addition assignment of a vector to a sparse vector.
// \ingroup smp
//
// \param lhs The target left-hand side sparse vector.
// \param rhs The right-hand side vector to be added.
// \return void
//
// This function implements the default SMP addition assignment of a vector to a sparse vector.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the left-hand side sparse vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side sparse vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline auto smpAddAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsSparseVector_v<VT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == (*rhs).size(), "Invalid vector sizes");
	addAssign(*lhs, *rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP subtraction assignment of a vector to a sparse vector.
// \ingroup smp
//
// \param lhs The target left-hand side sparse vector.
// \param rhs The right-hand side vector to be subtracted.
// \return void
//
// This function implements the default SMP subtraction assignment of a vector to a sparse
// vector.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the left-hand side sparse vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side sparse vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline auto smpSubAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsSparseVector_v<VT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == (*rhs).size(), "Invalid vector sizes");
	subAssign(*lhs, *rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the SMP multiplication assignment of a vector to a sparse vector.
// \ingroup smp
//
// \param lhs The target left-hand side sparse vector.
// \param rhs The right-hand side vector to be multiplied.
// \return void
//
// This function implements the default SMP multiplication assignment of a vector to a sparse
// vector.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the left-hand side sparse vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side sparse vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline auto smpMultAssign(Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs) -> EnableIf_t<IsSparseVector_v<VT1>>
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == (*rhs).size(), "Invalid vector sizes");
	multAssign(*lhs, *rhs);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
