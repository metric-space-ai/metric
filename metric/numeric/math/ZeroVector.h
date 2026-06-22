// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ZEROVECTOR_H
#define METRIC_NUMERIC_MATH_ZEROVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/CompressedVector.h>
#include <metric/numeric/math/ZeroMatrix.h>
#include <metric/numeric/math/sparse/ZeroVector.h>
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
/*!\brief Specialization of the Rand class template for ZeroVector.
// \ingroup random
//
// This specialization of the Rand class creates random instances of ZeroVector.
*/
template <typename Type // Data type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename Tag> // Type tag
class Rand<ZeroVector<Type, TF, Tag>> {
  public:
	//**********************************************************************************************
	/*!\brief Generation of a random ZeroVector.
	//
	// \param size The size of the random vector.
	// \return The generated random vector.
	*/
	inline const ZeroVector<Type, TF, Tag> generate(size_t size) const { return ZeroVector<Type, TF, Tag>(size); }
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
