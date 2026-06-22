// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_STORAGEORDER_H
#define METRIC_NUMERIC_MATH_STORAGEORDER_H
namespace mtrc::numeric {

//=================================================================================================
//
//  MATRIX STORAGE ORDER TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Storage order flag for row-major matrices.
//
// Via this flag it is possible to specify the storage order of matrices as row-major. For
// instance, given the following matrix

						  \f[\left(\begin{array}{*{3}{c}}
						  1 & 2 & 3 \\
						  4 & 5 & 6 \\
						  \end{array}\right)\f]\n

// in case of row-major order the elements are stored in the order

						  \f[\left(\begin{array}{*{6}{c}}
						  1 & 2 & 3 & 4 & 5 & 6. \\
						  \end{array}\right)\f]

// The following example demonstrates the setup of this \f$ 2 \times 3 \f$ matrix:

   \code
   using mtrc::numeric::rowMajor;
   mtrc::numeric::StaticMatrix<int,2UL,3UL,rowMajor> A( { { 1, 2, 3 }, { 4, 5, 6 } } );
   \endcode
*/
constexpr bool rowMajor = false;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Storage order flag for column-major matrices.
//
// Via this flag it is possible to specify the storage order of matrices as column-major. For
// instance, given the following matrix

						  \f[\left(\begin{array}{*{3}{c}}
						  1 & 2 & 3 \\
						  4 & 5 & 6 \\
						  \end{array}\right)\f]\n

// in case of column-major order the elements are stored in the order

						  \f[\left(\begin{array}{*{6}{c}}
						  1 & 4 & 2 & 5 & 3 & 6. \\
						  \end{array}\right)\f]

// The following example demonstrates the setup of this \f$ 2 \times 3 \f$ matrix:

   \code
   using mtrc::numeric::columnMajor;
   mtrc::numeric::StaticMatrix<int,2UL,3UL,columnMajor> A( { { 1, 2, 3 }, { 4, 5, 6 } } );
   \endcode
*/
constexpr bool columnMajor = true;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
