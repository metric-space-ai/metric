// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ROWS_H
#define METRIC_NUMERIC_MATH_ROWS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/DenseMatrix.h>
#include <metric/numeric/math/constraints/Rows.h>
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
//  RAND SPECIALIZATION FOR DENSE ROW SELECTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for dense row selections.
// \ingroup random
//
// This specialization of the Rand class randomizes dense row selections.
*/
template <typename MT // Type of the matrix
		  ,
		  bool SO // Storage order
		  ,
		  bool SF // Symmetry flag
		  ,
		  typename... CRAs> // Compile time row arguments
class Rand<Rows<MT, SO, true, SF, CRAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a dense row selection.
	//
	// \param rows The row selection to be randomized.
	// \return void
	*/
	template <typename RT> // Type of the row selection
	inline void randomize(RT &&rows) const
	{
		using mtrc::numeric::randomize;

		using RowsType = RemoveReference_t<RT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROWS_TYPE(RowsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(RowsType);

		if (SO == true) {
			for (size_t i = 0UL; i < rows.rows(); ++i) {
				for (size_t j = 0UL; j < rows.columns(); ++j) {
					randomize(rows(i, j));
				}
			}
		} else {
			for (size_t j = 0UL; j < rows.columns(); ++j) {
				for (size_t i = 0UL; i < rows.rows(); ++i) {
					randomize(rows(i, j));
				}
			}
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a dense row selection.
	//
	// \param rows The row selection to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename RT // Type of the row selection
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(RT &&rows, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		using RowsType = RemoveReference_t<RT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROWS_TYPE(RowsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(RowsType);

		if (SO == true) {
			for (size_t i = 0UL; i < rows.rows(); ++i) {
				for (size_t j = 0UL; j < rows.columns(); ++j) {
					randomize(rows(i, j), min, max);
				}
			}
		} else {
			for (size_t j = 0UL; j < rows.columns(); ++j) {
				for (size_t i = 0UL; i < rows.rows(); ++i) {
					randomize(rows(i, j), min, max);
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
//  RAND SPECIALIZATION FOR SPARSE ROW SELECTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for sparse row selections.
// \ingroup random
//
// This specialization of the Rand class randomizes sparse row selections.
*/
template <typename MT // Type of the matrix
		  ,
		  bool SO // Storage order
		  ,
		  bool SF // Symmetry flag
		  ,
		  typename... CRAs> // Compile time row arguments
class Rand<Rows<MT, SO, false, SF, CRAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a sparse row selection.
	//
	// \param rows The row selection to be randomized.
	// \return void
	*/
	template <typename RT> // Type of the row selection
	inline void randomize(RT &&rows) const
	{
		using RowsType = RemoveReference_t<RT>;
		using ElementType = ElementType_t<RowsType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROWS_TYPE(RowsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(RowsType);

		const size_t m(rows.rows());
		const size_t n(rows.columns());

		if (m == 0UL || n == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * m * n)));

		rows.reset();
		rows.reserve(nonzeros);

		while (rows.nonZeros() < nonzeros) {
			rows(rand<size_t>(0UL, m - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ElementType>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse row selection.
	//
	// \param rows The row selection to be randomized.
	// \param nonzeros The number of non-zero elements of the random row selection.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename RT> // Type of the row selection
	inline void randomize(RT &&rows, size_t nonzeros) const
	{
		using RowsType = RemoveReference_t<RT>;
		using ElementType = ElementType_t<RowsType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROWS_TYPE(RowsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(RowsType);

		const size_t m(rows.rows());
		const size_t n(rows.columns());

		if (nonzeros > m * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (m == 0UL || n == 0UL)
			return;

		rows.reset();
		rows.reserve(nonzeros);

		while (rows.nonZeros() < nonzeros) {
			rows(rand<size_t>(0UL, m - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ElementType>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse row selection.
	//
	// \param rows The row selection to be randomized.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	*/
	template <typename RT // Type of the row selection
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(RT &&rows, const Arg &min, const Arg &max) const
	{
		using RowsType = RemoveReference_t<RT>;
		using ElementType = ElementType_t<RowsType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROWS_TYPE(RowsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(RowsType);

		const size_t m(rows.rows());
		const size_t n(rows.columns());

		if (m == 0UL || n == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * m * n)));

		rows.reset();
		rows.reserve(nonzeros);

		while (rows.nonZeros() < nonzeros) {
			rows(rand<size_t>(0UL, m - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ElementType>(min, max);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse row selection.
	//
	// \param rows The row selection to be randomized.
	// \param nonzeros The number of non-zero elements of the random row selection.
	// \param min The smallest possible value for a matrix element.
	// \param max The largest possible value for a matrix element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename RT // Type of the row selection
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(RT &&rows, size_t nonzeros, const Arg &min, const Arg &max) const
	{
		using RowsType = RemoveReference_t<RT>;
		using ElementType = ElementType_t<RowsType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROWS_TYPE(RowsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(RowsType);

		const size_t m(rows.rows());
		const size_t n(rows.columns());

		if (nonzeros > m * n) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (m == 0UL || n == 0UL)
			return;

		rows.reset();
		rows.reserve(nonzeros);

		while (rows.nonZeros() < nonzeros) {
			rows(rand<size_t>(0UL, m - 1UL), rand<size_t>(0UL, n - 1UL)) = rand<ElementType>(min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
