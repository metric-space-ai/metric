// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_COMPRESSEDMATRIX_H
#define METRIC_NUMERIC_MATH_COMPRESSEDMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>
#include <metric/numeric/math/CompressedVector.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/IdentityMatrix.h>
#include <metric/numeric/math/SparseMatrix.h>
#include <metric/numeric/math/ZeroMatrix.h>
#include <metric/numeric/math/sparse/CompressedMatrix.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/Indices.h>
#include <metric/numeric/util/Random.h>
#include <vector>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for CompressedMatrix.
// \ingroup random
//
// This specialization of the Rand class creates random instances of CompressedMatrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  bool SO // Storage order
		  ,
		  typename Tag> // Tag type
class Rand<CompressedMatrix<Type, SO, Tag>> {
  public:
	//**********************************************************************************************
	/*!\brief Generation of a random CompressedMatrix.
	//
	// \param m The number of rows of the random matrix.
	// \param n The number of columns of the random matrix.
	// \return The generated random matrix.
	*/
	inline const CompressedMatrix<Type, SO, Tag> generate(size_t m, size_t n) const
	{
		CompressedMatrix<Type, SO, Tag> matrix(m, n);
		randomize(matrix);

		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random CompressedMatrix.
	//
	// \param m The number of rows of the random matrix.
	// \param n The number of columns of the random matrix.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \return The generated random matrix.
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	inline const CompressedMatrix<Type, SO, Tag> generate(size_t m, size_t n, size_t nonzeros) const
	{
		if (nonzeros > m * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		CompressedMatrix<Type, SO, Tag> matrix(m, n);
		randomize(matrix, nonzeros);

		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random CompressedMatrix.
	//
	// \param m The number of rows of the random matrix.
	// \param n The number of columns of the random matrix.
	// \param min The smallest possible value for a matrix element.
	// \return The generated random matrix.
	// \param max The largest possible value for a matrix element.
	*/
	template <typename Arg> // Min/max argument type
	inline const CompressedMatrix<Type, SO, Tag> generate(size_t m, size_t n, const Arg &min, const Arg &max) const
	{
		CompressedMatrix<Type, SO, Tag> matrix(m, n);
		randomize(matrix, min, max);

		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random CompressedMatrix.
	//
	// \param m The number of rows of the random matrix.
	// \param n The number of columns of the random matrix.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return The generated random matrix.
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename Arg> // Min/max argument type
	inline const CompressedMatrix<Type, SO, Tag> generate(size_t m, size_t n, size_t nonzeros, const Arg &min,
														  const Arg &max) const
	{
		if (nonzeros > m * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		CompressedMatrix<Type, SO, Tag> matrix(m, n);
		randomize(matrix, nonzeros, min, max);

		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a CompressedMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \return void
	*/
	inline void randomize(CompressedMatrix<Type, SO, Tag> &matrix) const
	{
		const size_t m(matrix.rows());
		const size_t n(matrix.columns());

		if (m == 0UL || n == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * m * n)));

		randomize(matrix, nonzeros);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a row-major CompressedMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	inline void randomize(CompressedMatrix<Type, false, Tag> &matrix, size_t nonzeros) const
	{
		const size_t m(matrix.rows());
		const size_t n(matrix.columns());

		if (nonzeros > m * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (m == 0UL || n == 0UL)
			return;

		matrix.reset();
		matrix.reserve(nonzeros);

		std::vector<size_t> dist(m);

		for (size_t nz = 0UL; nz < nonzeros;) {
			const size_t index = rand<size_t>(0UL, m - 1UL);
			if (dist[index] == n)
				continue;
			++dist[index];
			++nz;
		}

		for (size_t i = 0UL; i < m; ++i) {
			const Indices<size_t> indices(0UL, n - 1UL, dist[i]);
			for (size_t j : indices) {
				matrix.append(i, j, rand<Type>());
			}
			matrix.finalize(i);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a column-major CompressedMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	inline void randomize(CompressedMatrix<Type, true, Tag> &matrix, size_t nonzeros) const
	{
		const size_t m(matrix.rows());
		const size_t n(matrix.columns());

		if (nonzeros > m * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (m == 0UL || n == 0UL)
			return;

		matrix.reset();
		matrix.reserve(nonzeros);

		std::vector<size_t> dist(n);

		for (size_t nz = 0UL; nz < nonzeros;) {
			const size_t index = rand<size_t>(0UL, n - 1UL);
			if (dist[index] == m)
				continue;
			++dist[index];
			++nz;
		}

		for (size_t j = 0UL; j < n; ++j) {
			const Indices<size_t> indices(0UL, m - 1UL, dist[j]);
			for (size_t i : indices) {
				matrix.append(i, j, rand<Type>());
			}
			matrix.finalize(j);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a CompressedMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(CompressedMatrix<Type, SO, Tag> &matrix, const Arg &min, const Arg &max) const
	{
		const size_t m(matrix.rows());
		const size_t n(matrix.columns());

		if (m == 0UL || n == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * m * n)));

		randomize(matrix, nonzeros, min, max);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a row-major CompressedMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(CompressedMatrix<Type, false, Tag> &matrix, size_t nonzeros, const Arg &min,
						  const Arg &max) const
	{
		const size_t m(matrix.rows());
		const size_t n(matrix.columns());

		if (nonzeros > m * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (m == 0UL || n == 0UL)
			return;

		matrix.reset();
		matrix.reserve(nonzeros);

		std::vector<size_t> dist(m);

		for (size_t nz = 0UL; nz < nonzeros;) {
			const size_t index = rand<size_t>(0UL, m - 1UL);
			if (dist[index] == n)
				continue;
			++dist[index];
			++nz;
		}

		for (size_t i = 0UL; i < m; ++i) {
			const Indices<size_t> indices(0UL, n - 1UL, dist[i]);
			for (size_t j : indices) {
				matrix.append(i, j, rand<Type>(min, max));
			}
			matrix.finalize(i);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a column-major CompressedMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(CompressedMatrix<Type, true, Tag> &matrix, size_t nonzeros, const Arg &min,
						  const Arg &max) const
	{
		const size_t m(matrix.rows());
		const size_t n(matrix.columns());

		if (nonzeros > m * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (m == 0UL || n == 0UL)
			return;

		matrix.reset();
		matrix.reserve(nonzeros);

		std::vector<size_t> dist(n);

		for (size_t nz = 0UL; nz < nonzeros;) {
			const size_t index = rand<size_t>(0UL, n - 1UL);
			if (dist[index] == m)
				continue;
			++dist[index];
			++nz;
		}

		for (size_t j = 0UL; j < n; ++j) {
			const Indices<size_t> indices(0UL, m - 1UL, dist[j]);
			for (size_t i : indices) {
				matrix.append(i, j, rand<Type>(min, max));
			}
			matrix.finalize(j);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
