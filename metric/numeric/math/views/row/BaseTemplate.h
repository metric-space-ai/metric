// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_ROW_BASETEMPLATE_H
#define METRIC_NUMERIC_MATH_VIEWS_ROW_BASETEMPLATE_H
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
/*!\brief Base template of the Row class template.
// \ingroup row
*/
template <typename MT // Type of the matrix
		  ,
		  bool SO = IsRowMajorMatrix_v<MT> // Storage order
		  ,
		  bool DF = IsDenseMatrix_v<MT> // Density flag
		  ,
		  bool SF = IsSymmetric_v<MT> // Symmetry flag
		  ,
		  size_t... CRAs> // Compile time row arguments
class Row {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ALIAS DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary alias declaration for the Row class template.
// \ingroup row
//
// The Row_ alias declaration represents a convenient shortcut for the specification of the
// non-derived template arguments of the Row class template.
*/
template <typename MT // Type of the matrix
		  ,
		  size_t... CRAs> // Compile time row arguments
using Row_ = Row<MT, IsRowMajorMatrix_v<MT>, IsDenseMatrix_v<MT>, IsSymmetric_v<MT>, CRAs...>;
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
