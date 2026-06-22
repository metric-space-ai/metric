// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATSERIALEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATSERIALEXPR_H
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
/*!\brief Base class for all matrix serial evaluation expression templates.
// \ingroup math
//
// The MatSerialExpr class serves as a tag for all expression templates that enforce a serial
// evaluation of a matrix. All classes, that represent a matrix serialization operation and
// that are used within the expression template environment of the Metric numeric library have to
// derive publicly from this class in order to qualify as matrix serial evaluation expression
// template. Only in case a class is derived publicly from the MatSerialExpr base class, the
// IsMatSerialExpr type trait recognizes the class as valid matrix serial evaluation expression
// template.
*/
template <typename MT> // Matrix base type of the expression
struct MatSerialExpr : public SerialExpr<MT> {};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Evaluation of the given matrix serial evaluation expression.
// \ingroup math
//
// \param matrix The input serial evaluation expression.
// \return The evaluated matrix.
//
// This function implements a performance optimized treatment of the serial evaluation of a
// matrix serial evaluation expression.
*/
template <typename MT> // Matrix base type of the expression
inline decltype(auto) serial(const MatSerialExpr<MT> &matrix)
{
	return *matrix;
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
