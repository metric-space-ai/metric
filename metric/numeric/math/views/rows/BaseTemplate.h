// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_ROWS_BASETEMPLATE_H
#define METRIC_NUMERIC_MATH_VIEWS_ROWS_BASETEMPLATE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDenseMatrix.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>
#include <metric/numeric/math/typetraits/IsSymmetric.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  mtrc::numeric NAMESPACE FORWARD DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Base template of the Rows class template.
// \ingroup rows
*/
template <typename MT // Type of the matrix
		  ,
		  bool SO = IsRowMajorMatrix_v<MT> // Storage order
		  ,
		  bool DF = IsDenseMatrix_v<MT> // Density flag
		  ,
		  bool SF = IsSymmetric_v<MT> // Symmetry flag
		  ,
		  typename... CRAs> // Compile time row arguments
class Rows {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ALIAS DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary alias declaration for the Rows class template.
// \ingroup rows
//
// The Rows_ alias declaration represents a convenient shortcut for the specification of the
// non-derived template arguments of the Rows class template.
*/
template <typename MT // Type of the matrix
		  ,
		  typename... CRAs> // Compile time row arguments
using Rows_ = Rows<MT, IsRowMajorMatrix_v<MT>, IsDenseMatrix_v<MT>, IsSymmetric_v<MT>, CRAs...>;
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
