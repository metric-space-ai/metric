// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_COLUMNS_H
#define METRIC_NUMERIC_MATH_COLUMNS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/Columns.h>
#include <metric/numeric/math/constraints/DenseMatrix.h>
#include <metric/numeric/math/constraints/SparseMatrix.h>
#include <metric/numeric/math/smp/DenseMatrix.h>
#include <metric/numeric/math/smp/SparseMatrix.h>
#include <metric/numeric/math/views/Columns.h>
#include <metric/numeric/math/views/Rows.h>
#include <metric/numeric/util/Random.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION FOR DENSE COLUMN SELECTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for dense column selections.
// \ingroup random
//
// This specialization of the Rand class randomizes dense column selections.
*/
template <typename MT // Type of the matrix
		  ,
		  bool SO // Storage order
		  ,
		  bool SF // Symmetry flag
		  ,
		  typename... CCAs> // Compile time column arguments
class Rand<Columns<MT, SO, true, SF, CCAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a dense column selection.
	//
	// \param columns The column selection to be randomized.
	// \return void
	*/
	template <typename CT> // Type of the column selection
	inline void randomize(CT &&columns) const
	{
		using mtrc::numeric::randomize;

		using ColumnsType = RemoveReference_t<CT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMNS_TYPE(ColumnsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(ColumnsType);

		if (SO == false) {
			for (size_t i = 0UL; i < columns.rows(); ++i) {
				for (size_t j = 0UL; j < columns.columns(); ++j) {
					randomize(columns(i, j));
				}
			}
		} else {
			for (size_t j = 0UL; j < columns.columns(); ++j) {
				for (size_t i = 0UL; i < columns.rows(); ++i) {
					randomize(columns(i, j));
				}
			}
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a dense column selection.
	//
	// \param columns The column selection to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename CT // Type of the column selection
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(CT &&columns, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		using ColumnsType = RemoveReference_t<CT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMNS_TYPE(ColumnsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(ColumnsType);

		if (SO == false) {
			for (size_t i = 0UL; i < columns.rows(); ++i) {
				for (size_t j = 0UL; j < columns.columns(); ++j) {
					randomize(columns(i, j), min, max);
				}
			}
		} else {
			for (size_t j = 0UL; j < columns.columns(); ++j) {
				for (size_t i = 0UL; i < columns.rows(); ++i) {
					randomize(columns(i, j), min, max);
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
//  RAND SPECIALIZATION FOR SPARSE COLUMN SELECTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for sparse column selections.
// \ingroup random
//
// This specialization of the Rand class randomizes sparse column selections.
*/
template <typename MT // Type of the matrix
		  ,
		  bool SO // Storage order
		  ,
		  bool SF // Symmetry flag
		  ,
		  typename... CCAs> // Compile time column arguments
class Rand<Columns<MT, SO, false, SF, CCAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a sparse column selection.
	//
	// \param columns The column selection to be randomized.
	// \return void
	*/
	template <typename CT> // Type of the column selection
	inline void randomize(CT &&columns) const
	{
		using ColumnsType = RemoveReference_t<CT>;
		using ElementType = ElementType_t<ColumnsType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMNS_TYPE(ColumnsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(ColumnsType);

		const size_t m(columns.rows());
		const size_t n(columns.columns());

		if (m == 0UL || n == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * m * n)));

		columns.reset();
		columns.reserve(nonzeros);

		while (columns.nonZeros() < nonzeros) {
			columns(rand<size_t>(0UL, m - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ElementType>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse column selection.
	//
	// \param columns The column selection to be randomized.
	// \param nonzeros The number of non-zero elements of the random column selection.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename CT> // Type of the column selection
	inline void randomize(CT &&columns, size_t nonzeros) const
	{
		using ColumnsType = RemoveReference_t<CT>;
		using ElementType = ElementType_t<ColumnsType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMNS_TYPE(ColumnsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(ColumnsType);

		const size_t m(columns.rows());
		const size_t n(columns.columns());

		if (nonzeros > m * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (m == 0UL || n == 0UL)
			return;

		columns.reset();
		columns.reserve(nonzeros);

		while (columns.nonZeros() < nonzeros) {
			columns(rand<size_t>(0UL, m - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ElementType>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse column selection.
	//
	// \param columns The column selection to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename CT // Type of the column selection
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(CT &&columns, const Arg &min, const Arg &max) const
	{
		using ColumnsType = RemoveReference_t<CT>;
		using ElementType = ElementType_t<ColumnsType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMNS_TYPE(ColumnsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(ColumnsType);

		const size_t m(columns.rows());
		const size_t n(columns.columns());

		if (m == 0UL || n == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * m * n)));

		columns.reset();
		columns.reserve(nonzeros);

		while (columns.nonZeros() < nonzeros) {
			columns(rand<size_t>(0UL, m - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ElementType>(min, max);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse column selection.
	//
	// \param columns The column selection to be randomized.
	// \param nonzeros The number of non-zero elements of the random column selection.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename CT // Type of the column selection
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(CT &&columns, size_t nonzeros, const Arg &min, const Arg &max) const
	{
		using ColumnsType = RemoveReference_t<CT>;
		using ElementType = ElementType_t<ColumnsType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMNS_TYPE(ColumnsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(ColumnsType);

		const size_t m(columns.rows());
		const size_t n(columns.columns());

		if (nonzeros > m * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (m == 0UL || n == 0UL)
			return;

		columns.reset();
		columns.reserve(nonzeros);

		while (columns.nonZeros() < nonzeros) {
			columns(rand<size_t>(0UL, m - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ElementType>(min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
