// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ALIGNMENTFLAG_H
#define METRIC_NUMERIC_MATH_ALIGNMENTFLAG_H
namespace mtrc::numeric {

//=================================================================================================
//
//  ALIGNMENT FLAG
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Alignment flag for (un-)aligned vectors and matrices.
// \ingroup math
//
// Via these flags it is possible to specify subvectors, submatrices, custom vectors and matrices
// as unaligned or aligned. The following example demonstrates the setup of an unaligned subvector:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::unaligned;

   mtrc::numeric::DynamicVector<int,columnVector> v( 100UL );
   auto sv = subvector<unaligned>( v, 10UL, 20UL );
   \endcode
*/
enum AlignmentFlag : bool {
	unaligned = false, //!< Flag for unaligned vectors and matrices.
	aligned = true	   //!< Flag for aligned vectors and matrices.
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Negating the given alignment flag.
// \ingroup math
//
// \param flag The given alignment flag to be negated.
// \return The negated alignment flag.
//
// This logical NOT operator negates the given alignment flag. In case the given flag represents
// \a unaligned, the function returns \a aligned, in case it represents \a aligned it returns
// \a unaligned.
*/
constexpr AlignmentFlag operator!(AlignmentFlag flag) noexcept
{
	return static_cast<AlignmentFlag>(!static_cast<bool>(flag));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
