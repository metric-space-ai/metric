// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_UNIFORMMATRIX_H
#define METRIC_NUMERIC_MATH_UNIFORMMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/DenseMatrix.h>
#include <metric/numeric/math/dense/UniformMatrix.h>
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
/*!\brief Specialization of the Rand class template for UniformMatrix.
// \ingroup random
//
// This specialization of the Rand class creates random instances of UniformMatrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  bool SO // Storage order
		  ,
		  typename Tag> // Type tag
class Rand<UniformMatrix<Type, SO, Tag>> {
  public:
	//**********************************************************************************************
	/*!\brief Generation of a random UniformMatrix.
	//
	// \param m The number of rows of the random matrix.
	// \param n The number of columns of the random matrix.
	// \return The generated random matrix.
	*/
	inline const UniformMatrix<Type, SO, Tag> generate(size_t m, size_t n) const
	{
		UniformMatrix<Type, SO, Tag> matrix(m, n);
		randomize(matrix);
		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random UniformMatrix.
	//
	// \param m The number of rows of the random matrix.
	// \param n The number of columns of the random matrix.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return The generated random matrix.
	*/
	template <typename Arg> // Min/max argument type
	inline const UniformMatrix<Type, SO, Tag> generate(size_t m, size_t n, const Arg &min, const Arg &max) const
	{
		UniformMatrix<Type, SO, Tag> matrix(m, n);
		randomize(matrix, min, max);
		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a UniformMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \return void
	*/
	inline void randomize(UniformMatrix<Type, SO, Tag> &matrix) const { matrix = rand<Type>(); }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a UniformMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(UniformMatrix<Type, SO, Tag> &matrix, const Arg &min, const Arg &max) const
	{
		matrix = rand<Type>(min, max);
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
