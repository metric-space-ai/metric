// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_BAND_BASETEMPLATE_H
#define METRIC_NUMERIC_MATH_VIEWS_BAND_BASETEMPLATE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDenseMatrix.h>
#include <metric/numeric/math/typetraits/IsMatMatMultExpr.h>
#include <metric/numeric/system/TransposeFlag.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  mtrc::numeric NAMESPACE FORWARD DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Base template of the Band class template.
// \ingroup band
*/
template <typename MT // Type of the matrix
		  ,
		  bool TF = defaultTransposeFlag // Transpose flag
		  ,
		  bool DF = IsDenseMatrix_v<MT> // Density flag
		  ,
		  bool MF = IsMatMatMultExpr_v<MT> // Multiplication flag
		  ,
		  ptrdiff_t... CBAs> // Compile time band arguments
class Band {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ALIAS DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary alias declaration for the Band class template.
// \ingroup band
//
// The Band_ alias declaration represents a convenient shortcut for the specification of the
// non-derived template arguments of the Band class template.
*/
template <typename MT // Type of the matrix
		  ,
		  ptrdiff_t... CBAs> // Compile time band arguments
using Band_ = Band<MT, defaultTransposeFlag, IsDenseMatrix_v<MT>, IsMatMatMultExpr_v<MT>, CBAs...>;
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
