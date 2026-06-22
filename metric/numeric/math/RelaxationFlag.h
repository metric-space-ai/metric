// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_RELAXATIONFLAG_H
#define METRIC_NUMERIC_MATH_RELAXATIONFLAG_H
namespace mtrc::numeric {

//=================================================================================================
//
//  RELAXATION FLAG
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Relaxation flag for strict or relaxed semantics.
// \ingroup math
//
// Via these flags it is possible to specify that according operations should use strict
// semantics instead of relaxed semantics or vice versa. The following example demonstrates this
// by means of the isDefault() function template:

   \code
   using mtrc::numeric::strict;
   using mtrc::numeric::relaxed;

   mtrc::numeric::StaticVector<double,3UL> v{ 0.0, 1E-9, 0.0 };

   isDefault<strict> ( v );  // Returns false
   isDefault<relaxed>( v );  // Returns true
   \endcode
*/
enum RelaxationFlag : bool {
	strict = false, //!< Flag for strict semantics.
	relaxed = true	//!< Flag for relaxed semantics.
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Negating the given relaxation flag.
// \ingroup math
//
// \param flag The given relaxation flag to be negated.
// \return The negated relaxation flag.
//
// This logical NOT operator negates the given relaxation flag. In case the given flag represents
// \a strict, the function returns \a relaxed, in case it represents \a relaxed it returns
// \a strict.
*/
constexpr RelaxationFlag operator!(RelaxationFlag flag) noexcept
{
	return static_cast<RelaxationFlag>(!static_cast<bool>(flag));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
