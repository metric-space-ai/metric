// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECSERIALEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECSERIALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/SerialExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all vector serial evaluation expression templates.
// \ingroup math
//
// The VecSerialExpr class serves as a tag for all expression templates that enforce a serial
// evaluation of a vector. All classes, that represent a vector serialization operation and
// that are used within the expression template environment of the Metric numeric library have to
// derive publicly from this class in order to qualify as vector serial evaluation expression
// template. Only in case a class is derived publicly from the VecSerialExpr base class, the
// IsVecSerialExpr type trait recognizes the class as valid vector serial evaluation expression
// template.
*/
template <typename VT> // Vector base type of the expression
struct VecSerialExpr : public SerialExpr<VT> {};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Evaluation of the given vector serial evaluation expression.
// \ingroup math
//
// \param vector The input serial evaluation expression.
// \return The evaluated vector.
//
// This function implements a performance optimized treatment of the serial evaluation of a
// vector serial evaluation expression.
*/
template <typename VT> // Vector base type of the expression
inline decltype(auto) serial(const VecSerialExpr<VT> &vector)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return *vector;
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
