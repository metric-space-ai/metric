// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_ELEMENTS_BASETEMPLATE_H
#define METRIC_NUMERIC_MATH_VIEWS_ELEMENTS_BASETEMPLATE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

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
/*!\brief Base template of the Elements class template.
// \ingroup row
*/
template <typename VT // Type of the vector
		  ,
		  bool TF = IsRowVector_v<VT> // Transpose flag
		  ,
		  bool DF = IsDenseVector_v<VT> // Density flag
		  ,
		  typename... CEAs> // Compile time element arguments
class Elements {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ALIAS DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary alias declaration for the Elements class template.
// \ingroup elements
//
// The Elements_ alias declaration represents a convenient shortcut for the specification of the
// non-derived template arguments of the Elements class template.
*/
template <typename VT // Type of the vector
		  ,
		  typename... CEAs> // Compile time element arguments
using Elements_ = Elements<VT, IsRowVector_v<VT>, IsDenseVector_v<VT>, CEAs...>;
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
