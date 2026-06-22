// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_IDENTITYMATRIX_H
#define METRIC_NUMERIC_MATH_IDENTITYMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/CompressedMatrix.h>
#include <metric/numeric/math/CompressedVector.h>
#include <metric/numeric/math/sparse/IdentityMatrix.h>
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
/*!\brief Specialization of the Rand class template for IdentityMatrix.
// \ingroup random
//
// This specialization of the Rand class creates random instances of IdentityMatrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  bool SO // Storage order
		  ,
		  typename Tag> // Type tag
class Rand<IdentityMatrix<Type, SO, Tag>> {
  public:
	//**********************************************************************************************
	/*!\brief Generation of a random IdentityMatrix.
	//
	// \param n The number of rows and columns of the random matrix.
	// \return The generated random matrix.
	*/
	inline const IdentityMatrix<Type, SO, Tag> generate(size_t n) const { return IdentityMatrix<Type, SO, Tag>(n); }
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
