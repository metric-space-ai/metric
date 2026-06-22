// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CUSTOMVECTOR_H
#define METRIC_NUMERIC_MATH_CUSTOMVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/AlignmentFlag.h>
#include <metric/numeric/math/DenseVector.h>
#include <metric/numeric/math/DynamicMatrix.h>
#include <metric/numeric/math/ZeroVector.h>
#include <metric/numeric/math/dense/CustomVector.h>
#include <metric/numeric/math/dense/DynamicVector.h>
#include <metric/numeric/math/dense/StaticVector.h>
#include <metric/numeric/util/Random.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for CustomVector.
// \ingroup random
//
// This specialization of the Rand class randomizes instances of CustomVector.
*/
template <typename Type // Data type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  bool TF // Transpose flag
		  ,
		  typename Tag // Type tag
		  ,
		  typename RT> // Result type
class Rand<CustomVector<Type, AF, PF, TF, Tag, RT>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a CustomVector.
	//
	// \param vector The vector to be randomized.
	// \return void
	*/
	inline void randomize(CustomVector<Type, AF, PF, TF, Tag, RT> &vector) const
	{
		using mtrc::numeric::randomize;

		const size_t size(vector.size());
		for (size_t i = 0UL; i < size; ++i) {
			randomize(vector[i]);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a CustomVector.
	//
	// \param vector The vector to be randomized.
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(CustomVector<Type, AF, PF, TF, Tag, RT> &vector, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		const size_t size(vector.size());
		for (size_t i = 0UL; i < size; ++i) {
			randomize(vector[i], min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
