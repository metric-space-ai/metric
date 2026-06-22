// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_EVAL_H
#define METRIC_NUMERIC_MATH_SHIMS_EVAL_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>
#include <metric/numeric/util/typetraits/IsComplex.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  EVAL SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Formal evaluation of the given argument.
// \ingroup math_shims
//
// \param a The value/object to be evaluated.
// \return The evaluated value/object.
//
// The \a eval shim represents an abstract interface for enforcing the evaluation of a
// value/object of any given data type. For data types that don't require an evaluation,
// as for instance built-in data types, the default behavior is not changed.
*/
template <typename T>
METRIC_NUMERIC_ALWAYS_INLINE constexpr EnableIf_t<IsBuiltin_v<T> || IsComplex_v<T>, const T &> eval(const T &a) noexcept
{
	return a;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
