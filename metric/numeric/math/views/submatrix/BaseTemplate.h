// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_SUBMATRIX_BASETEMPLATE_H
#define METRIC_NUMERIC_MATH_VIEWS_SUBMATRIX_BASETEMPLATE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/AlignmentFlag.h>
#include <metric/numeric/math/typetraits/IsColumnMajorMatrix.h>
#include <metric/numeric/math/typetraits/IsDenseMatrix.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  mtrc::numeric NAMESPACE FORWARD DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Base template of the Submatrix class template.
// \ingroup submatrix
*/
template <typename MT // Type of the matrix
		  ,
		  AlignmentFlag AF = unaligned // Alignment flag
		  ,
		  bool SO = IsColumnMajorMatrix_v<MT> // Storage order
		  ,
		  bool DF = IsDenseMatrix_v<MT> // Density flag
		  ,
		  size_t... CSAs> // Compile time submatrix arguments
class Submatrix {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ALIAS DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary alias declaration for the Submatrix class template.
// \ingroup submatrix
//
// The Submatrix_ alias declaration represents a convenient shortcut for the specification of the
// non-derived template arguments of the Submatrix class template.
*/
template <typename MT // Type of the matrix
		  ,
		  AlignmentFlag AF = unaligned // Alignment flag
		  ,
		  size_t... CSAs> // Compile time submatrix arguments
using Submatrix_ = Submatrix<MT, AF, IsColumnMajorMatrix_v<MT>, IsDenseMatrix_v<MT>, CSAs...>;
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
