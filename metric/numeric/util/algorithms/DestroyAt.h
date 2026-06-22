// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ALGORITHMS_DESTROYAT_H
#define METRIC_NUMERIC_UTIL_ALGORITHMS_DESTROYAT_H
namespace mtrc::numeric {

//=================================================================================================
//
//  DESTROY_AT ALGORITHM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Destroys the object at the given address.
// \ingroup algorithms
//
// \param p The address of the object to be destroyed.
// \return void
//
// This function explicitly calls the destructor of the object at the given address.
*/
template <typename T> void destroy_at(T *p) noexcept { p->~T(); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
