// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_SUBVECTOR_BASETEMPLATE_H
#define METRIC_NUMERIC_MATH_VIEWS_SUBVECTOR_BASETEMPLATE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/AlignmentFlag.h>
#include <metric/numeric/math/typetraits/IsDenseVector.h>
#include <metric/numeric/math/typetraits/IsRowVector.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  mtrc::numeric NAMESPACE FORWARD DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Base template of the Subvector class template.
// \ingroup subvector
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF = unaligned // Alignment flag
		  ,
		  bool TF = IsRowVector_v<VT> // Transpose flag
		  ,
		  bool DF = IsDenseVector_v<VT> // Density flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
class Subvector {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ALIAS DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary alias declaration for the Subvector class template.
// \ingroup subvector
//
// The Subvector_ alias declaration represents a convenient shortcut for the specification of the
// non-derived template arguments of the Subvector class template.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF = unaligned // Alignment flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
using Subvector_ = Subvector<VT, AF, IsRowVector_v<VT>, IsDenseVector_v<VT>, CSAs...>;
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
