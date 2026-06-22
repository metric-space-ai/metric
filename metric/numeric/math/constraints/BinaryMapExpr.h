// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_BINARYMAPEXPR_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_BINARYMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsBinaryMapExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_BINARYMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is not a binary map expression (i.e. a type derived from the
// BinaryMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_BINARYMAPEXPR_TYPE(T)                                                        \
	static_assert(::mtrc::numeric::IsBinaryMapExpr_v<T>, "Non-binary map expression type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_BINARYMAPEXPR_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T is a binary map expression (i.e. a type derived from the
// BinaryMapExpr base class), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_BINARYMAPEXPR_TYPE(T)                                                    \
	static_assert(!::mtrc::numeric::IsBinaryMapExpr_v<T>, "Binary map expression type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
