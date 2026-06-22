// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SYMMETRICMATRIX_H
#define METRIC_NUMERIC_MATH_SYMMETRICMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cmath>
#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/DenseMatrix.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/SparseMatrix.h>
#include <metric/numeric/math/adaptors/DiagonalMatrix.h>
#include <metric/numeric/math/adaptors/SymmetricMatrix.h>
#include <metric/numeric/math/constraints/DenseMatrix.h>
#include <metric/numeric/math/constraints/Resizable.h>
#include <metric/numeric/math/constraints/Scalar.h>
#include <metric/numeric/math/constraints/SparseMatrix.h>
#include <metric/numeric/math/shims/Real.h>
#include <metric/numeric/math/typetraits/IsDenseMatrix.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/IntegralConstant.h>
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
/*!\brief Specialization of the Rand class template for SymmetricMatrix.
// \ingroup random
//
// This specialization of the Rand class creates random instances of SymmetricMatrix.
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  bool SF> // Scalar flag
class Rand<SymmetricMatrix<MT, SO, DF, SF>> {
  public:
	//**********************************************************************************************
	/*!\brief Generation of a random SymmetricMatrix.
	//
	// \return The generated random matrix.
	*/
	inline const SymmetricMatrix<MT, SO, DF, SF> generate() const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_RESIZABLE_TYPE(MT);

		SymmetricMatrix<MT, SO, DF, SF> matrix;
		randomize(matrix);
		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random SymmetricMatrix.
	//
	// \param n The number of rows and columns of the random matrix.
	// \return The generated random matrix.
	*/
	inline const SymmetricMatrix<MT, SO, DF, SF> generate(size_t n) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_RESIZABLE_TYPE(MT);

		SymmetricMatrix<MT, SO, DF, SF> matrix(n);
		randomize(matrix);
		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random SymmetricMatrix.
	//
	// \param n The number of rows and columns of the random matrix.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \return The generated random matrix.
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	inline const SymmetricMatrix<MT, SO, DF, SF> generate(size_t n, size_t nonzeros) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_RESIZABLE_TYPE(MT);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		if (nonzeros > n * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		SymmetricMatrix<MT, SO, DF, SF> matrix(n);
		randomize(matrix, nonzeros);

		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random SymmetricMatrix.
	//
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return The generated random matrix.
	*/
	template <typename Arg> // Min/max argument type
	inline const SymmetricMatrix<MT, SO, DF, SF> generate(const Arg &min, const Arg &max) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_RESIZABLE_TYPE(MT);

		SymmetricMatrix<MT, SO, DF, SF> matrix;
		randomize(matrix, min, max);
		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random SymmetricMatrix.
	//
	// \param n The number of rows and columns of the random matrix.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return The generated random matrix.
	*/
	template <typename Arg> // Min/max argument type
	inline const SymmetricMatrix<MT, SO, DF, SF> generate(size_t n, const Arg &min, const Arg &max) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_RESIZABLE_TYPE(MT);

		SymmetricMatrix<MT, SO, DF, SF> matrix(n);
		randomize(matrix, min, max);
		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random SymmetricMatrix.
	//
	// \param n The number of rows and columns of the random matrix.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return The generated random matrix.
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename Arg> // Min/max argument type
	inline const SymmetricMatrix<MT, SO, DF, SF> generate(size_t n, size_t nonzeros, const Arg &min,
														  const Arg &max) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_RESIZABLE_TYPE(MT);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		if (nonzeros > n * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		SymmetricMatrix<MT, SO, DF, SF> matrix(n);
		randomize(matrix, nonzeros, min, max);

		return matrix;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a SymmetricMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \return void
	*/
	inline void randomize(SymmetricMatrix<MT, SO, DF, SF> &matrix) const
	{
		randomize(matrix, typename IsDenseMatrix<MT>::Type());
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse SymmetricMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	inline void randomize(SymmetricMatrix<MT, SO, DF, SF> &matrix, size_t nonzeros) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		using ET = ElementType_t<MT>;

		const size_t n(matrix.rows());

		if (nonzeros > n * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (n == 0UL)
			return;

		matrix.reset();
		matrix.reserve(nonzeros + 1UL);

		while (matrix.nonZeros() < nonzeros) {
			matrix(rand<size_t>(0UL, n - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ET>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a SymmetricMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(SymmetricMatrix<MT, SO, DF, SF> &matrix, const Arg &min, const Arg &max) const
	{
		randomize(matrix, min, max, typename IsDenseMatrix<MT>::Type());
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse SymmetricMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param nonzeros The number of non-zero elements of the random matrix.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(SymmetricMatrix<MT, SO, DF, SF> &matrix, size_t nonzeros, const Arg &min,
						  const Arg &max) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		using ET = ElementType_t<MT>;

		const size_t n(matrix.rows());

		if (nonzeros > n * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (n == 0UL)
			return;

		std::vector<size_t> dist(n);
		std::vector<bool> structure(n * n);
		size_t nz(0UL);

		while (nz < nonzeros) {
			const size_t row = rand<size_t>(0UL, n - 1UL);
			const size_t col = rand<size_t>(0UL, n - 1UL);

			if (structure[row * n + col])
				continue;

			++dist[row];
			structure[row * n + col] = true;
			++nz;

			if (row != col) {
				++dist[col];
				structure[col * n + row] = true;
				++nz;
			}
		}

		matrix.reset();
		matrix.reserve(nz);

		for (size_t i = 0UL; i < n; ++i) {
			matrix.reserve(i, dist[i]);
		}

		for (size_t i = 0UL; i < n; ++i) {
			for (size_t j = i; j < n; ++j) {
				if (structure[i * n + j]) {
					matrix.append(i, j, rand<ET>(min, max));
				}
			}
		}
	}
	//**********************************************************************************************

  private:
	//*************************************************************************************************
	/*!\brief Randomization of a dense SymmetricMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \return void
	*/
	inline void randomize(SymmetricMatrix<MT, SO, DF, SF> &matrix, TrueType) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(MT);

		using ET = ElementType_t<MT>;

		const size_t n(matrix.rows());

		for (size_t i = 0UL; i < n; ++i) {
			for (size_t j = 0UL; j <= i; ++j) {
				matrix(i, j) = rand<ET>();
			}
		}
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Randomization of a sparse SymmetricMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \return void
	*/
	inline void randomize(SymmetricMatrix<MT, SO, DF, SF> &matrix, FalseType) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		const size_t n(matrix.rows());

		if (n == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * n * n)));

		randomize(matrix, nonzeros);
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Randomization of a dense SymmetricMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(SymmetricMatrix<MT, SO, DF, SF> &matrix, const Arg &min, const Arg &max, TrueType) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(MT);

		using ET = ElementType_t<MT>;

		const size_t n(matrix.rows());

		for (size_t i = 0UL; i < n; ++i) {
			for (size_t j = 0UL; j <= i; ++j) {
				matrix(i, j) = rand<ET>(min, max);
			}
		}
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Randomization of a sparse SymmetricMatrix.
	//
	// \param matrix The matrix to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(SymmetricMatrix<MT, SO, DF, SF> &matrix, const Arg &min, const Arg &max, FalseType) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);

		const size_t n(matrix.rows());

		if (n == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * n * n)));

		randomize(matrix, nonzeros, min, max);
	}
	//*************************************************************************************************
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
/*!\brief Setup of a random symmetric SymmetricMatrix.
//
// \param matrix The matrix to be randomized.
// \return void
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool SF> // Scalar flag
void makeSymmetric(SymmetricMatrix<MT, SO, true, SF> &matrix)
{
	using mtrc::numeric::randomize;

	randomize(matrix);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random symmetric SymmetricMatrix.
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
		  bool SF // Scalar flag
		  ,
		  typename Arg> // Min/max argument type
void makeSymmetric(SymmetricMatrix<MT, SO, true, SF> &matrix, const Arg &min, const Arg &max)
{
	using mtrc::numeric::randomize;

	randomize(matrix, min, max);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random Hermitian SymmetricMatrix.
//
// \param matrix The matrix to be randomized.
// \return void
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool SF> // Scalar flag
void makeHermitian(SymmetricMatrix<MT, SO, true, SF> &matrix)
{
	using BT = UnderlyingBuiltin_t<ElementType_t<MT>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SCALAR_TYPE(BT);

	const size_t n(matrix.rows());

	for (size_t i = 0UL; i < n; ++i) {
		for (size_t j = 0UL; j <= i; ++j) {
			matrix(i, j) = rand<BT>();
		}
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isHermitian(matrix), "Non-Hermitian matrix detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random Hermitian SymmetricMatrix.
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
		  bool SF // Scalar flag
		  ,
		  typename Arg> // Min/max argument type
void makeHermitian(SymmetricMatrix<MT, SO, true, SF> &matrix, const Arg &min, const Arg &max)
{
	using BT = UnderlyingBuiltin_t<ElementType_t<MT>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SCALAR_TYPE(BT);

	const size_t n(matrix.rows());

	for (size_t i = 0UL; i < n; ++i) {
		for (size_t j = 0UL; j <= i; ++j) {
			matrix(i, j) = rand<BT>(real(min), real(max));
		}
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isHermitian(matrix), "Non-Hermitian matrix detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Setup of a random (Hermitian) positive definite SymmetricMatrix.
//
// \param matrix The matrix to be randomized.
// \return void
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool SF> // Scalar flag
void makePositiveDefinite(SymmetricMatrix<MT, SO, true, SF> &matrix)
{
	using BT = UnderlyingBuiltin_t<ElementType_t<MT>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SCALAR_TYPE(BT);

	const size_t n(matrix.rows());

	makeHermitian(matrix);
	matrix *= matrix;

	for (size_t i = 0UL; i < n; ++i) {
		matrix(i, i) += BT(n);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isHermitian(matrix), "Non-Hermitian matrix detected");
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
