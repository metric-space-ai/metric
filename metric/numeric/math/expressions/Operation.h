// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_OPERATION_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_OPERATION_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all operational expression templates.
// \ingroup math
//
// The Operation class serves as a tag for all operational expression templates. All classes
// that represent either a transformation (transpositions, ...) or a mathematical computation
// (addition, subtraction, multiplication, division, absolute value calculation, ...) and that
// are used within the expression template environment of the Metric numeric library have to derive from
// this class in order to qualify as operational expression template. Only in case a class is
// derived from the Operation base class, the IsOperation type trait recognizes the class as
// valid operational expression template.
*/
struct Operation {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
