// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ALGORITHMS_CONSTRUCTAT_H
#define METRIC_NUMERIC_UTIL_ALGORITHMS_CONSTRUCTAT_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CONSTRUCT_AT ALGORITHM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constructs the object at the given address.
// \ingroup algorithms
//
// \param p The address of the object to be constructed.
// \param args The constructor arguments for the object to be constructed.
// \return void
//
// This function constructs an object of type \a T at the given address.
*/
template <typename T, typename... Args> void construct_at(T *p, Args &&...args)
{
	::new (static_cast<void *>(p)) T(std::forward<Args>(args)...);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
