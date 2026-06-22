// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CHECKEDDELETE_H
#define METRIC_NUMERIC_UTIL_CHECKEDDELETE_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CHECKED DELETE OPERATIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name Checked delete operations */
//@{
template <typename T> void checkedDelete(T *ptr);
template <typename T> void checkedArrayDelete(T *ptr);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Type-checked \c delete operation.
// \ingroup util
//
// \param ptr The pointer to be deleted.
// \return void
//
// This function frees the given pointer resource via \c delete. Note that the \c delete operation
// is NOT permitted for incomplete types (i.e. declared but undefined data types). The attempt
// to use this function for a pointer to an object of incomplete type results in a compile time
// error!
*/
template <typename T> void checkedDelete(T *ptr)
{
	using TypeMustBeComplete = T;
	(void)sizeof(TypeMustBeComplete);
	delete ptr;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Type-checked \c delete[] operation.
// \ingroup util
//
// \param ptr The pointer to the array to be deleted.
// \return void
//
// This function frees the given pointer resource via \c delete[]. Note that the \c delete[]
// operation is NOT permitted for incomplete types (i.e. declared but undefined data types).
// The attempt to use this function for a pointer to an array of objects of incomplete type
// results in a compile time error!
*/
template <typename T> void checkedArrayDelete(T *ptr)
{
	using TypeMustBeComplete = T;
	(void)sizeof(TypeMustBeComplete);
	delete[] ptr;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
