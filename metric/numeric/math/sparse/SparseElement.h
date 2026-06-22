// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SPARSE_SPARSEELEMENT_H
#define METRIC_NUMERIC_MATH_SPARSE_SPARSEELEMENT_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all sparse element types.
// \ingroup math
//
// The SparseElement class is the base class for all sparse element types. All types that
// conceptually represent a sparse vector or matrix element and that provide a \c value()
// and an \c index() member function have to derive from this class in order to qualify
// as sparse element. Only in case a class is derived from the SparseElement base class,
// the IsSparseElement type trait recognizes the class as valid sparse element.
*/
struct SparseElement {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
