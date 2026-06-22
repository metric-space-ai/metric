// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ZEROMATRIX_H
#define METRIC_NUMERIC_MATH_ZEROMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/CompressedMatrix.h>
#include <metric/numeric/math/CompressedVector.h>
#include <metric/numeric/math/ZeroVector.h>
#include <metric/numeric/math/sparse/ZeroMatrix.h>
#include <metric/numeric/util/Random.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for ZeroMatrix.
// \ingroup random
//
// This specialization of the Rand class creates random instances of ZeroMatrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  bool SO // Storage order
		  ,
		  typename Tag> // Type tag
class Rand<ZeroMatrix<Type, SO, Tag>> {
  public:
	//**********************************************************************************************
	/*!\brief Generation of a random ZeroMatrix.
	//
	// \param m The number of rows of the random matrix.
	// \param n The number of columns of the random matrix.
	// \return The generated random matrix.
	*/
	inline const ZeroMatrix<Type, SO, Tag> generate(size_t m, size_t n) const
	{
		return ZeroMatrix<Type, SO, Tag>(m, n);
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
