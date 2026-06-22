// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SUBMATRIX_H
#define METRIC_NUMERIC_MATH_SUBMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/DenseMatrix.h>
#include <metric/numeric/math/constraints/SparseMatrix.h>
#include <metric/numeric/math/constraints/Submatrix.h>
#include <metric/numeric/math/smp/DenseMatrix.h>
#include <metric/numeric/math/smp/SparseMatrix.h>
#include <metric/numeric/math/views/Submatrix.h>
#include <metric/numeric/math/views/Subvector.h>
#include <metric/numeric/util/Random.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION FOR DENSE SUBMATRICES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for dense submatrices.
// \ingroup random
//
// This specialization of the Rand class randomizes dense submatrices.
*/
template <typename MT // Type of the matrix
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool SO // Storage order
		  ,
		  size_t... CSAs> // Compile time submatrix arguments
class Rand<Submatrix<MT, AF, SO, true, CSAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a dense submatrix.
	//
	// \param submatrix The submatrix to be randomized.
	// \return void
	*/
	template <typename SMT> // Type of the submatrix
	inline void randomize(SMT &&submatrix) const
	{
		using mtrc::numeric::randomize;

		using SubmatrixType = RemoveReference_t<SMT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBMATRIX_TYPE(SubmatrixType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(SubmatrixType);

		if (SO == rowMajor) {
			for (size_t i = 0UL; i < submatrix.rows(); ++i) {
				for (size_t j = 0UL; j < submatrix.columns(); ++j) {
					randomize(submatrix(i, j));
				}
			}
		} else {
			for (size_t j = 0UL; j < submatrix.columns(); ++j) {
				for (size_t i = 0UL; i < submatrix.rows(); ++i) {
					randomize(submatrix(i, j));
				}
			}
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a dense submatrix.
	//
	// \param submatrix The submatrix to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename SMT // Type of the submatrix
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(SMT &&submatrix, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		using SubmatrixType = RemoveReference_t<SMT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBMATRIX_TYPE(SubmatrixType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(SubmatrixType);

		if (SO == rowMajor) {
			for (size_t i = 0UL; i < submatrix.rows(); ++i) {
				for (size_t j = 0UL; j < submatrix.columns(); ++j) {
					randomize(submatrix(i, j), min, max);
				}
			}
		} else {
			for (size_t j = 0UL; j < submatrix.columns(); ++j) {
				for (size_t i = 0UL; i < submatrix.rows(); ++i) {
					randomize(submatrix(i, j), min, max);
				}
			}
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  RAND SPECIALIZATION FOR SPARSE SUBMATRICES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for sparse submatrices.
// \ingroup random
//
// This specialization of the Rand class randomizes sparse submatrices.
*/
template <typename MT // Type of the dense matrix
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool SO // Storage order
		  ,
		  size_t... CSAs> // Compile time submatrix arguments
class Rand<Submatrix<MT, AF, SO, false, CSAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a sparse submatrix.
	//
	// \param submatrix The submatrix to be randomized.
	// \return void
	*/
	template <typename SMT> // Type of the submatrix
	inline void randomize(SMT &&submatrix) const
	{
		using SubmatrixType = RemoveReference_t<SMT>;
		using ElementType = ElementType_t<SubmatrixType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBMATRIX_TYPE(SubmatrixType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(SubmatrixType);

		const size_t m(submatrix.rows());
		const size_t n(submatrix.columns());

		if (m == 0UL || n == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * m * n)));

		submatrix.reset();
		submatrix.reserve(nonzeros);

		while (submatrix.nonZeros() < nonzeros) {
			submatrix(rand<size_t>(0UL, m - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ElementType>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse submatrix.
	//
	// \param submatrix The submatrix to be randomized.
	// \param nonzeros The number of non-zero elements of the random submatrix.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename SMT> // Type of the submatrix
	inline void randomize(SMT &&submatrix, size_t nonzeros) const
	{
		using SubmatrixType = RemoveReference_t<SMT>;
		using ElementType = ElementType_t<SubmatrixType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBMATRIX_TYPE(SubmatrixType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(SubmatrixType);

		const size_t m(submatrix.rows());
		const size_t n(submatrix.columns());

		if (nonzeros > m * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (m == 0UL || n == 0UL)
			return;

		submatrix.reset();
		submatrix.reserve(nonzeros);

		while (submatrix.nonZeros() < nonzeros) {
			submatrix(rand<size_t>(0UL, m - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ElementType>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse submatrix.
	//
	// \param submatrix The submatrix to be randomized.
	// \param min The smallest possible value for a submatrix element.
	// \param max The largest possible value for a submatrix element.
	// \return void
	*/
	template <typename SMT // Type of the submatrix
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(SMT &&submatrix, const Arg &min, const Arg &max) const
	{
		using SubmatrixType = RemoveReference_t<SMT>;
		using ElementType = ElementType_t<SubmatrixType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBMATRIX_TYPE(SubmatrixType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(SubmatrixType);

		const size_t m(submatrix.rows());
		const size_t n(submatrix.columns());

		if (m == 0UL || n == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * m * n)));

		submatrix.reset();
		submatrix.reserve(nonzeros);

		while (submatrix.nonZeros() < nonzeros) {
			submatrix(rand<size_t>(0UL, m - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ElementType>(min, max);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse submatrix.
	//
	// \param submatrix The submatrix to be randomized.
	// \param nonzeros The number of non-zero elements of the random submatrix.
	// \param min The smallest possible value for a submatrix element.
	// \param max The largest possible value for a submatrix element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename SMT // Type of the submatrix
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(SMT &&submatrix, size_t nonzeros, const Arg &min, const Arg &max) const
	{
		using SubmatrixType = RemoveReference_t<SMT>;
		using ElementType = ElementType_t<SubmatrixType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBMATRIX_TYPE(SubmatrixType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(SubmatrixType);

		const size_t m(submatrix.rows());
		const size_t n(submatrix.columns());

		if (nonzeros > m * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (m == 0UL || n == 0UL)
			return;

		submatrix.reset();
		submatrix.reserve(nonzeros);

		while (submatrix.nonZeros() < nonzeros) {
			submatrix(rand<size_t>(0UL, m - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ElementType>(min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
