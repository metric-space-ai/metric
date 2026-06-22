// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_COLUMNS_BASETEMPLATE_H
#define METRIC_NUMERIC_MATH_VIEWS_COLUMNS_BASETEMPLATE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsColumnMajorMatrix.h>
#include <metric/numeric/math/typetraits/IsDenseMatrix.h>
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
/*!\brief Base template of the Columns class template.
// \ingroup columns
*/
template <typename MT // Type of the matrix
		  ,
		  bool SO = IsColumnMajorMatrix_v<MT> // Storage order
		  ,
		  bool DF = IsDenseMatrix_v<MT> // Density flag
		  ,
		  bool SF = IsSymmetric_v<MT> // Symmetry flag
		  ,
		  typename... CCAs> // Compile time column arguments
class Columns {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ALIAS DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary alias declaration for the Columns class template.
// \ingroup columns
//
// The Columns_ alias declaration represents a convenient shortcut for the specification of the
// non-derived template arguments of the Columns class template.
*/
template <typename MT // Type of the matrix
		  ,
		  typename... CCAs> // Compile time column arguments
using Columns_ = Columns<MT, IsColumnMajorMatrix_v<MT>, IsDenseMatrix_v<MT>, IsSymmetric_v<MT>, CCAs...>;
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
