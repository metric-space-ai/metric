// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_NULLTYPE_H
#define METRIC_NUMERIC_UTIL_NULLTYPE_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Utility type for generic codes.
// \ingroup util
//
// The NullType class represents an invalid or terminating data type for generic codes. For
// instance, the TypeList class uses the NullType as terminating data type for the type list.
*/
class NullType {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
