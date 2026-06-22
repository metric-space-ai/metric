// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_EXPRESSION_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_EXPRESSION_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all expression templates.
// \ingroup math
//
// The Expression class is the base class for all expression templates. All classes that
// represent an expression and that are used within the expression template environment of
// the Metric numeric library have to derive publicly from this class in order to qualify as expression
// template. Only in case a class is derived publicly from the Expression base class, the
// IsExpression type trait recognizes the class as valid expression template.
*/
template <typename T> // Base type of the expression
struct Expression : public T {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
