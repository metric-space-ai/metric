// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_PADDINGFLAG_H
#define METRIC_NUMERIC_MATH_PADDINGFLAG_H
namespace mtrc::numeric {

//=================================================================================================
//
//  PADDING FLAG VALUES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Padding flag for (un-)padded vectors and matrices.
// \ingroup math
//
// Via these flags it is possible to specify custom vectors and matrices as unpadded or padded.
// The following examples demonstrate the setup of an unaligned, unpadded and aligned, padded
// custom column vector of size 7, respectively:

   \code
   using mtrc::numeric::CustomVector;
   using mtrc::numeric::unaligned;
   using mtrc::numeric::unpadded;
   using mtrc::numeric::columnVector;

   std::vector<int> vec( 7UL );
   CustomVector<int,unaligned,unpadded,columnVector> a( &vec[0], 7UL );
   \endcode

   \code
   using mtrc::numeric::CustomVector;
   using mtrc::numeric::ArrayDelete;
   using mtrc::numeric::aligned;
   using mtrc::numeric::padded;
   using mtrc::numeric::columnVector;

   std::vector<int> vec( 16UL );
   CustomVector<int,aligned,padded,columnVector> a( &vec[0], 7UL, 16UL );
   \endcode
*/
enum PaddingFlag : bool {
	unpadded = false, //!< Flag for unpadded vectors and matrices.
	padded = true	  //!< Flag for padded vectors and matrices.
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Negating the given padding flag.
// \ingroup math
//
// \param flag The given padding flag to be negated.
// \return The negated padding flag.
//
// This logical NOT operator negates the given padding flag. In case the given flag represents
// \a unpadded, the function returns \a padded, in case it represents \a padded it returns
// \a unpadded.
*/
constexpr PaddingFlag operator!(PaddingFlag flag) noexcept
{
	return static_cast<PaddingFlag>(!static_cast<bool>(flag));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
