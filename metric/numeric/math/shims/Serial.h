// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_SERIAL_H
#define METRIC_NUMERIC_MATH_SHIMS_SERIAL_H
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
//  SERIAL SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Formal serialization of the evaluation of the given argument.
// \ingroup math_shims
//
// \param a The value/object to be evaluated serially.
// \return The serialized operation.
//
// The \a serial shim represents an abstract interface for the serialization of the evaluation of
// a value/object of any given data type. For data types that are per default evaluated serially,
// as for instance built-in data types, the default behavior is not changed.
*/
template <typename T>
METRIC_NUMERIC_ALWAYS_INLINE constexpr EnableIf_t<IsBuiltin_v<T> || IsComplex_v<T>, const T &>
serial(const T &a) noexcept
{
	return a;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
