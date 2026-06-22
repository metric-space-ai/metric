// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_UNILOWERMATRIX_H
#define METRIC_NUMERIC_MATH_UNILOWERMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>
#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/DenseMatrix.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/SparseMatrix.h>
#include <metric/numeric/math/UniUpperMatrix.h>
#include <metric/numeric/math/adaptors/LowerMatrix.h>
#include <metric/numeric/math/adaptors/StrictlyLowerMatrix.h>
#include <metric/numeric/math/adaptors/UniLowerMatrix.h>
#include <metric/numeric/math/constraints/DenseMatrix.h>
#include <metric/numeric/math/constraints/Resizable.h>
#include <metric/numeric/math/constraints/SparseMatrix.h>
#include <metric/numeric/math/typetraits/IsDenseMatrix.h>
#include <metric/numeric/util/Indices.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/Random.h>
#include <metric/numeric/util/Types.h>
#include <vector>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for UniLowerMatrix.
// \ingroup random
//
// This specialization of the Rand class creates random instances of UniLowerMatrix.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF> // Density flag
class Rand<UniLowerMatrix<MT, SO, DF>> {
  public:
	//**********************************************************************************************
	/*!\brief Generation of a random UniLowerMatrix.
	//
	// \return The generated random matrix.
	*/
	inline const UniLowerMatrix<MT, SO, DF> generate() const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_RESIZABLE_TYPE(MT);

		UniLowerMatrix<MT, SO, DF> matrix;
		randomize(matrix);
		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random UniLowerMatrix.
	//
	// \param n The number of rows and columns of the random matrix.
	// \return The generated random matrix.
	*/
	inline const UniLowerMatrix<MT, SO, DF> generate(size_t n) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_RESIZABLE_TYPE(MT);

		UniLowerMatrix<MT, SO, DF> matrix(n);
		randomize(matrix);
		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random UniLowerMatrix.
	//
	// \param n The number of rows and columns of the random matrix.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \return The generated random matrix.
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	inline const UniLowerMatrix<MT, SO, DF> generate(size_t n, size_t nonzeros) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_RESIZABLE_TYPE(MT);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		if (nonzeros > UniLowerMatrix<MT, SO, DF>::maxNonZeros(n)) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		UniLowerMatrix<MT, SO, DF> matrix(n);
		randomize(matrix, nonzeros);

		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random UniLowerMatrix.
	//
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return The generated random matrix.
	*/
	template <typename Arg> // Min/max argument type
	inline const UniLowerMatrix<MT, SO, DF> generate(const Arg &min, const Arg &max) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_RESIZABLE_TYPE(MT);

		UniLowerMatrix<MT, SO, DF> matrix;
		randomize(matrix, min, max);
		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random UniLowerMatrix.
	//
	// \param n The number of rows and columns of the random matrix.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return The generated random matrix.
	*/
	template <typename Arg> // Min/max argument type
	inline const UniLowerMatrix<MT, SO, DF> generate(size_t n, const Arg &min, const Arg &max) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_RESIZABLE_TYPE(MT);

		UniLowerMatrix<MT, SO, DF> matrix(n);
		randomize(matrix, min, max);
		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random UniLowerMatrix.
	//
	// \param n The number of rows and columns of the random matrix.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return The generated random matrix.
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename Arg> // Min/max argument type
	inline const UniLowerMatrix<MT, SO, DF> generate(size_t n, size_t nonzeros, const Arg &min, const Arg &max) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_RESIZABLE_TYPE(MT);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		if (nonzeros > UniLowerMatrix<MT, SO, DF>::maxNonZeros(n)) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		UniLowerMatrix<MT, SO, DF> matrix(n);
		randomize(matrix, nonzeros, min, max);

		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a UniLowerMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \return void
	*/
	inline void randomize(UniLowerMatrix<MT, SO, DF> &matrix) const
	{
		randomize(matrix, typename IsDenseMatrix<MT>::Type());
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a row-major sparse UniLowerMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	inline void randomize(UniLowerMatrix<MT, false, DF> &matrix, size_t nonzeros) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		using ET = ElementType_t<MT>;

		const size_t n(matrix.rows());

		if (nonzeros > UniLowerMatrix<MT, SO, DF>::maxNonZeros(n)) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (n == 0UL || n == 1UL)
			return;

		matrix.reset();
		matrix.reserve(nonzeros);
		matrix.finalize(0UL);

		std::vector<size_t> dist(n);

		for (size_t nz = 0UL; nz < nonzeros;) {
			const size_t index = rand<size_t>(1UL, n - 1UL);
			if (dist[index] == index)
				continue;
			++dist[index];
			++nz;
		}

		for (size_t i = 1UL; i < n; ++i) {
			const Indices<size_t> indices(0UL, i - 1UL, dist[i]);
			for (size_t j : indices) {
				matrix.append(i, j, rand<ET>());
			}
			matrix.finalize(i);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a column-major sparse UniLowerMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	inline void randomize(UniLowerMatrix<MT, true, DF> &matrix, size_t nonzeros) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		using ET = ElementType_t<MT>;

		const size_t n(matrix.rows());

		if (nonzeros > UniLowerMatrix<MT, SO, DF>::maxNonZeros(n)) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (n == 0UL || n == 1UL)
			return;

		matrix.reset();
		matrix.reserve(nonzeros);

		std::vector<size_t> dist(n - 1UL);

		for (size_t nz = 0UL; nz < nonzeros;) {
			const size_t index = rand<size_t>(0UL, n - 2UL);
			if (dist[index] == n - index - 1UL)
				continue;
			++dist[index];
			++nz;
		}

		for (size_t j = 0UL; j < n - 1UL; ++j) {
			const Indices<size_t> indices(j + 1UL, n - 1UL, dist[j]);
			for (size_t i : indices) {
				matrix.append(i, j, rand<ET>());
			}
			matrix.finalize(j);
		}

		matrix.finalize(n - 1UL);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a UniLowerMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(UniLowerMatrix<MT, SO, DF> &matrix, const Arg &min, const Arg &max) const
	{
		randomize(matrix, min, max, typename IsDenseMatrix<MT>::Type());
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a row-major sparse UniLowerMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(UniLowerMatrix<MT, false, DF> &matrix, size_t nonzeros, const Arg &min, const Arg &max) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		using ET = ElementType_t<MT>;

		const size_t n(matrix.rows());

		if (nonzeros > UniLowerMatrix<MT, SO, DF>::maxNonZeros(n)) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (n == 0UL || n == 1UL)
			return;

		matrix.reset();
		matrix.reserve(nonzeros);
		matrix.finalize(0UL);

		std::vector<size_t> dist(n);

		for (size_t nz = 0UL; nz < nonzeros;) {
			const size_t index = rand<size_t>(1UL, n - 1UL);
			if (dist[index] == index)
				continue;
			++dist[index];
			++nz;
		}

		for (size_t i = 1UL; i < n; ++i) {
			const Indices<size_t> indices(0UL, i - 1UL, dist[i]);
			for (size_t j : indices) {
				matrix.append(i, j, rand<ET>(min, max));
			}
			matrix.finalize(i);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a column-major sparse UniLowerMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(UniLowerMatrix<MT, true, DF> &matrix, size_t nonzeros, const Arg &min, const Arg &max) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		using ET = ElementType_t<MT>;

		const size_t n(matrix.rows());

		if (nonzeros > UniLowerMatrix<MT, SO, DF>::maxNonZeros(n)) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (n == 0UL || n == 1UL)
			return;

		matrix.reset();
		matrix.reserve(nonzeros);

		std::vector<size_t> dist(n - 1UL);

		for (size_t nz = 0UL; nz < nonzeros;) {
			const size_t index = rand<size_t>(0UL, n - 2UL);
			if (dist[index] == n - index - 1UL)
				continue;
			++dist[index];
			++nz;
		}

		for (size_t j = 0UL; j < n - 1UL; ++j) {
			const Indices<size_t> indices(j + 1UL, n - 1UL, dist[j]);
			for (size_t i : indices) {
				matrix.append(i, j, rand<ET>(min, max));
			}
			matrix.finalize(j);
		}

		matrix.finalize(n - 1UL);
	}
	//**********************************************************************************************

  private:
	//**********************************************************************************************
	/*!\brief Randomization of a dense UniLowerMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \return void
	*/
	inline void randomize(UniLowerMatrix<MT, SO, DF> &matrix, TrueType) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(MT);

		using ET = ElementType_t<MT>;

		const size_t n(matrix.rows());

		for (size_t i = 1UL; i < n; ++i) {
			for (size_t j = 0UL; j < i; ++j) {
				matrix(i, j) = rand<ET>();
			}
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse UniLowerMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \return void
	*/
	inline void randomize(UniLowerMatrix<MT, SO, DF> &matrix, FalseType) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		const size_t n(matrix.rows());

		if (n == 0UL || n == 1UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.2 * n * n)));

		randomize(matrix, nonzeros);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a dense UniLowerMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(UniLowerMatrix<MT, SO, DF> &matrix, const Arg &min, const Arg &max, TrueType) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(MT);

		using ET = ElementType_t<MT>;

		const size_t n(matrix.rows());

		for (size_t i = 1UL; i < n; ++i) {
			for (size_t j = 0UL; j < i; ++j) {
				matrix(i, j) = rand<ET>(min, max);
			}
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse UniLowerMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(UniLowerMatrix<MT, SO, DF> &matrix, const Arg &min, const Arg &max, FalseType) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		const size_t n(matrix.rows());

		if (n == 0UL || n == 1UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.2 * n * n)));

		randomize(matrix, nonzeros, min, max);
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  MAKE FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random symmetric UniLowerMatrix.
//
// \param matrix The matrix to be randomized.
// \return void
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF> // Density flag
void makeSymmetric(UniLowerMatrix<MT, SO, DF> &matrix)
{
	reset(matrix);

	METRIC_NUMERIC_INTERNAL_ASSERT(isSymmetric(matrix), "Non-symmetric matrix detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random symmetric UniLowerMatrix.
//
// \param matrix The matrix to be randomized.
// \param min The smallest possible value for a matrix element.
// \param max The largest possible value for a matrix element.
// \return void
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename Arg> // Min/max argument type
void makeSymmetric(UniLowerMatrix<MT, SO, DF> &matrix, const Arg &min, const Arg &max)
{
	MAYBE_UNUSED(min, max);

	makeSymmetric(matrix);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random Hermitian UniLowerMatrix.
//
// \param matrix The matrix to be randomized.
// \return void
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF> // Density flag
void makeHermitian(UniLowerMatrix<MT, SO, DF> &matrix)
{
	reset(matrix);

	METRIC_NUMERIC_INTERNAL_ASSERT(isHermitian(matrix), "Non-Hermitian matrix detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random Hermitian UniLowerMatrix.
//
// \param matrix The matrix to be randomized.
// \param min The smallest possible value for a matrix element.
// \param max The largest possible value for a matrix element.
// \return void
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  typename Arg> // Min/max argument type
void makeHermitian(UniLowerMatrix<MT, SO, DF> &matrix, const Arg &min, const Arg &max)
{
	MAYBE_UNUSED(min, max);

	makeHermitian(matrix);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random (Hermitian) positive definite UniLowerMatrix.
//
// \param matrix The matrix to be randomized.
// \return void
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF> // Density flag
void makePositiveDefinite(UniLowerMatrix<MT, SO, DF> &matrix)
{
	makeHermitian(matrix);
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
