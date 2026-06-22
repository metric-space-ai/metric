// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_CLAMP_H
#define METRIC_NUMERIC_MATH_SHIMS_CLAMP_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLAMP SHIM
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Restricts the given value to the range \f$[min..max]\f$.
// \ingroup math_shims
//
// \param a The given value/object.
// \param min The lower limit of the range.
// \param max The upper limit of the range.
// \return The clamped value.
//
// The \a clamp shim represents an abstract interface for restricting a value to the specified
// range \f$[min..max]\f$:

   \code
   double d1 =  0.5;
   double d2 =  1.5;
   double d3 = -1.5;

   clamp( d1, -1.0, 1.0 );  // Results in 0.5
   clamp( d2, -1.0, 1.0 );  // Results in 1.0
   clamp( d3, -1.0, 1.0 );  // Results in -1.0
   \endcode
*/
template <typename T> inline const T &clamp(const T &a, const T &min, const T &max) noexcept
{
	if (a < min)
		return min;
	else if (max < a)
		return max;
	else
		return a;
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
