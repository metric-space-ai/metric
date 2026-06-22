// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VIEW_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VIEW_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Expression.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all views.
// \ingroup math
//
// The View class serves as a tag for all views (subvectors, submatrices, rows, columns, ...).
// All classes that represent a view and that are used within the expression template environment
// of the Metric numeric library have to derive publicly from this class in order to qualify as a view.
// Only in case a class is derived publicly from the View base class, the IsView type trait
// recognizes the class as valid view.
*/
template <typename T> // Base type of the expression
struct View : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
