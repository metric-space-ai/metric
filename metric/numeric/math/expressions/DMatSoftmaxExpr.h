// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DMATSOFTMAXEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DMATSOFTMAXEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/ReductionFlag.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/views/Check.h>
#include <metric/numeric/math/views/Column.h>
#include <metric/numeric/math/views/Row.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Computes the softmax function for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the softmax computation.
// \return The resulting matrix.
//
// This function computes the softmax function (i.e. the normalized exponential function) for
// the given dense matrix \a dm (see also https://en.wikipedia.org/wiki/Softmax_function). The
// resulting dense matrix consists of real values in the range (0..1], which add up to 1.

   \code
   // Creating the matrix
   //    ( 1  2  3 )
   //    ( 4  1  2 )
   //    ( 3  4  1 )
   mtrc::numeric::StaticMatrix<double,3UL,3UL> A{ { 1.0, 2.0, 3.0 }
										, { 4.0, 1.0, 2.0 }
										, { 3.0, 4.0, 1.0 } };

   // Computing the total softmax of A (sum(B) == 1)
   //    ( 0.0157764  0.0428847  0.116573  )
   //    ( 0.316878   0.0157764  0.0428847 )
   //    ( 0.116573   0.316878   0.0157764 )
   mtrc::numeric::StaticMatrix<double,3UL,3UL> B;
   B = softmax( A );
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
auto softmax(const DenseMatrix<MT, SO> &dm)
{
	auto tmp(evaluate(exp(*dm - max(*dm))));
	const auto scalar(sum(tmp));
	tmp /= scalar;
	return tmp;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the row-/columnwise softmax function for the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The given dense matrix for the softmax computation.
// \return The resulting matrix.
//
// This function computes the row-/columnwise softmax function (i.e. the normalized exponential
// function) for the given dense matrix \a dm (see also https://en.wikipedia.org/wiki/Softmax_function).
// The resulting dense matrix consists of real values in the range (0..1], which add up to the
// numbers of rows or columns, respectively.

   \code
   // Creating the matrix
   //    ( 1  2  3 )
   //    ( 4  1  2 )
   //    ( 3  4  1 )
   mtrc::numeric::StaticMatrix<double,3UL,3UL> A{ { 1.0, 2.0, 3.0 }
										, { 4.0, 1.0, 2.0 }
										, { 3.0, 4.0, 1.0 } };

   // Computing the rowwise softmax of A (sum(B) == 3)
   //    ( 0.0900306  0.244728   0.665241 )
   //    ( 0.843795   0.0420101  0.114195 )
   //    ( 0.259496   0.705385   0.035119 )
   mtrc::numeric::StaticMatrix<double,3UL,3UL> B;
   B = softmax<rowwise>( A );

   // Computing the columnwise softmax of A (sum(C) == 3)
   //    ( 0.035119  0.114195   0.665241  )
   //    ( 0.705385  0.0420101  0.244728  )
   //    ( 0.259496  0.843795   0.0900306 )
   mtrc::numeric::StaticMatrix<double,3UL,3UL> C;
   C = softmax<columnwise>( A );
   \endcode
*/
template <ReductionFlag RF // Reduction flag
		  ,
		  typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
auto softmax(const DenseMatrix<MT, SO> &dm)
{
	const size_t expansion((RF == rowwise) ? (*dm).columns() : (*dm).rows());
	auto tmp(evaluate(exp(*dm - expand(max<RF>(*dm), expansion))));

	if (RF == rowwise) {
		for (size_t i = 0UL; i < tmp.rows(); ++i) {
			auto r = row(tmp, i, unchecked);
			const auto scalar(sum(r));
			r /= scalar;
		}
	} else {
		for (size_t j = 0UL; j < tmp.columns(); ++j) {
			auto c = column(tmp, j, unchecked);
			const auto scalar(sum(c));
			c /= scalar;
		}
	}

	return tmp;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
