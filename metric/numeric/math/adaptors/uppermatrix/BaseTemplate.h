// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ADAPTORS_UPPERMATRIX_BASETEMPLATE_H
#define METRIC_NUMERIC_MATH_ADAPTORS_UPPERMATRIX_BASETEMPLATE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsDenseMatrix.h>
#include <metric/numeric/math/typetraits/StorageOrder.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\defgroup upper_matrix UpperMatrix
// \ingroup adaptors
*/
/*!\brief Matrix adapter for upper triangular \f$ N \times N \f$ matrices.
// \ingroup upper_matrix
//
// \section uppermatrix_general General
//
// The UpperMatrix class template is an adapter for existing dense and sparse matrix types. It
// inherits the properties and the interface of the given matrix type \a MT and extends it by
// enforcing the additional invariant that all matrix elements below the diagonal are 0 (upper
// triangular matrix). The type of the adapted matrix can be specified via the first template
// parameter:

   \code
   namespace mtrc::numeric {

   template< typename MT, bool SO, bool DF >
   class UpperMatrix;

   } // namespace mtrc::numeric
   \endcode

//  - MT: specifies the type of the matrix to be adapted. UpperMatrix can be used with any
//        non-cv-qualified, non-reference, non-pointer, non-expression dense or sparse matrix
//        type. Note that the given matrix type must be either resizable (as for instance
//        HybridMatrix or DynamicMatrix) or must be square at compile time (as for instance
//        StaticMatrix).
//  - SO: specifies the storage order (mtrc::numeric::rowMajor, mtrc::numeric::columnMajor) of the matrix. This
//        template parameter doesn't have to be explicitly defined, but is automatically derived
//        from the first template parameter.
//  - DF: specifies whether the given matrix type is a dense or sparse matrix type. This template
//        parameter doesn't have to be defined explicitly, it is automatically derived from the
//        first template parameter. Defining the parameter explicitly may result in a compilation
//        error!
//
// The following examples give an impression of several possible upper matrices:

   \code
   using mtrc::numeric::unaligned;
   using mtrc::numeric::unpadded;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   // Definition of a 3x3 row-major dense upper matrix with static memory
   mtrc::numeric::UpperMatrix< mtrc::numeric::StaticMatrix<int,3UL,3UL,rowMajor> > A;

   // Definition of a resizable column-major dense upper matrix based on HybridMatrix
   mtrc::numeric::UpperMatrix< mtrc::numeric::HybridMatrix<float,4UL,4UL,columnMajor> B;

   // Definition of a resizable row-major dense upper matrix based on DynamicMatrix
   mtrc::numeric::UpperMatrix< mtrc::numeric::DynamicMatrix<double,rowMajor> > C;

   // Definition of a fixed-size row-major dense upper matrix based on CustomMatrix
   mtrc::numeric::UpperMatrix< mtrc::numeric::CustomMatrix<double,unaligned,unpadded,rowMajor> > D;

   // Definition of a compressed row-major single precision upper matrix
   mtrc::numeric::UpperMatrix< mtrc::numeric::CompressedMatrix<float,rowMajor> > E;
   \endcode

// The storage order of an upper matrix is depending on the storage order of the adapted matrix
// type \a MT. In case the adapted matrix is stored in a row-wise fashion (i.e. is specified
// as mtrc::numeric::rowMajor), the upper matrix will also be a row-major matrix. Otherwise, if the
// adapted matrix is column-major (i.e. is specified as mtrc::numeric::columnMajor), the upper matrix
// will also be a column-major matrix.
//
//
// \n \section uppermatrix_special_properties Special Properties of Upper Triangular Matrices
//
// An upper matrix is used exactly like a matrix of the underlying, adapted matrix type \a MT. It
// also provides (nearly) the same interface as the underlying matrix type. However, there are
// some important exceptions resulting from the upper matrix constraint:
//
//  -# <b>\ref uppermatrix_square</b>
//  -# <b>\ref uppermatrix_upper</b>
//  -# <b>\ref uppermatrix_initialization</b>
//  -# <b>\ref uppermatrix_storage</b>
//
// \n \subsection uppermatrix_square Upper Matrices Must Always be Square!
//
// In case a resizable matrix is used (as for instance mtrc::numeric::HybridMatrix, mtrc::numeric::DynamicMatrix,
// or mtrc::numeric::CompressedMatrix), this means that the according constructors, the \c resize() and
// the \c extend() functions only expect a single parameter, which specifies both the number of
// rows and columns, instead of two (one for the number of rows and one for the number of columns):

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::UpperMatrix;
   using mtrc::numeric::rowMajor;

   // Default constructed, default initialized, row-major 3x3 upper dynamic matrix
   UpperMatrix< DynamicMatrix<double,rowMajor> > A( 3 );

   // Resizing the matrix to 5x5
   A.resize( 5 );

   // Extending the number of rows and columns by 2, resulting in a 7x7 matrix
   A.extend( 2 );
   \endcode

// In case a matrix with a fixed size is used (as for instance mtrc::numeric::StaticMatrix), the number
// of rows and number of columns must be specified equally:

   \code
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::UpperMatrix;
   using mtrc::numeric::columnMajor;

   // Correct setup of a fixed size column-major 3x3 upper static matrix
   UpperMatrix< StaticMatrix<int,3UL,3UL,columnMajor> > A;

   // Compilation error: the provided matrix type is not a square matrix type
   UpperMatrix< StaticMatrix<int,3UL,4UL,columnMajor> > B;
   \endcode

// \n \subsection uppermatrix_upper The Upper Matrix Property is Always Enforced!
//
// This means that it is only allowed to modify elements in the upper part or the diagonal of
// the matrix, but not the elements in the lower part of the matrix. Also, it is only possible
// to assign matrices that are upper matrices themselves:

   \code
   using mtrc::numeric::CompressedMatrix;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::UpperMatrix;
   using mtrc::numeric::rowMajor;

   using CompressedUpper = UpperMatrix< CompressedMatrix<double,rowMajor> >;

   // Default constructed, row-major 3x3 upper compressed matrix
   CompressedUpper A( 3 );

   // Initializing elements via the function call operator
   A(0,0) = 1.0;  // Initialization of the diagonal element (0,0)
   A(0,2) = 2.0;  // Initialization of the upper element (0,2)
   A(2,1) = 9.0;  // Throws an exception; invalid modification of lower element

   // Inserting two more elements via the insert() function
   A.insert( 0, 1, 3.0 );  // Inserting the upper element (0,1)
   A.insert( 1, 1, 4.0 );  // Inserting the diagonal element (1,1)
   A.insert( 2, 0, 9.0 );  // Throws an exception; invalid insertion of lower element

   // Appending an element via the append() function
   A.reserve( 1, 3 );      // Reserving enough capacity in row 1
   A.append( 1, 2, 5.0 );  // Appending the upper element (1,2)
   A.append( 2, 1, 9.0 );  // Throws an exception; appending an element in the lower part

   // Access via a non-const iterator
   CompressedUpper::Iterator it = A.begin(1);
   *it = 9.0;  // Modifies the diagonal element (1,1)
   ++it;
   *it = 6.0;  // Modifies the upper element (1,2)

   // Erasing elements via the erase() function
   A.erase( 0, 0 );  // Erasing the diagonal element (0,0)
   A.erase( 0, 2 );  // Erasing the upper element (0,2)

   // Construction from an upper dense matrix
   StaticMatrix<double,3UL,3UL> B( { {3.0,  8.0, -2.0 },
									 {0.0,  0.0, -1.0 },
									 {0.0,  0.0,  4.0 } } );

   UpperMatrix< DynamicMatrix<double,rowMajor> > C( B );  // OK

   // Assignment of a non-upper dense matrix
   StaticMatrix<double,3UL,3UL> D( { {  3.0,  8.0, -2.0 },
									 {  0.0,  0.0, -1.0 },
									 { -2.0,  0.0,  4.0 } } );

   C = D;  // Throws an exception; upper matrix invariant would be violated!
   \endcode

// The upper matrix property is also enforced for upper custom matrices: In case the given array
// of elements does not represent a upper matrix, a \a std::invalid_argument exception is thrown:

   \code
   using mtrc::numeric::CustomMatrix;
   using mtrc::numeric::UpperMatrix;
   using mtrc::numeric::unaligned;
   using mtrc::numeric::unpadded;
   using mtrc::numeric::rowMajor;

   using CustomUpper = UpperMatrix< CustomMatrix<double,unaligned,unpadded,rowMajor> >;

   // Creating a 3x3 upper custom matrix from a properly initialized array
   double array[9] = { 1.0, 2.0, 3.0,
					   0.0, 4.0, 5.0,
					   0.0, 0.0, 6.0 };
   CustomUpper A( array, 3UL );  // OK

   // Attempt to create a second 3x3 upper custom matrix from an uninitialized array
   CustomUpper B( new double[9UL], 3UL, mtrc::numeric::ArrayDelete() );  // Throws an exception
   \endcode

// Finally, the upper matrix property is enforced for views (rows, columns, submatrices, ...) on
// the upper matrix. The following example demonstrates that modifying the elements of an entire
// row and submatrix of an upper matrix only affects the upper and diagonal matrix elements:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::UpperMatrix;

   // Setup of the upper matrix
   //
   //       ( 0 1 0 4 )
   //   A = ( 0 2 3 0 )
   //       ( 0 0 0 5 )
   //       ( 0 0 0 0 )
   //
   UpperMatrix< DynamicMatrix<int> > A( 4 );
   A(0,1) = 1;
   A(0,3) = 4;
   A(1,1) = 2;
   A(1,2) = 3;
   A(2,3) = 5;

   // Setting the upper and diagonal elements in the 1st row to 9 results in the matrix
   //
   //       ( 0 1 0 4 )
   //   A = ( 0 9 9 9 )
   //       ( 0 0 0 5 )
   //       ( 0 0 0 0 )
   //
   row( A, 1 ) = 9;

   // Setting the upper and diagonal elements in the 1st and 2nd column to 7 results in
   //
   //       ( 0 7 7 4 )
   //   A = ( 0 7 7 9 )
   //       ( 0 0 7 5 )
   //       ( 0 0 0 0 )
   //
   submatrix( A, 0, 1, 4, 2 ) = 7;
   \endcode

// The next example demonstrates the (compound) assignment to rows/columns and submatrices of
// upper matrices. Since only upper and diagonal elements may be modified the matrix to be
// assigned must be structured such that the upper matrix invariant of the upper matrix is
// preserved. Otherwise a \a std::invalid_argument exception is thrown:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::UpperMatrix;
   using mtrc::numeric::rowVector;

   // Setup of two default 4x4 upper matrices
   UpperMatrix< DynamicMatrix<int> > A1( 4 ), A2( 4 );

   // Setup of a 4-dimensional vector
   //
   //   v = ( 0 1 2 3 )
   //
   DynamicVector<int,rowVector> v( 4, 0 );
   v[1] = 1;
   v[2] = 2;
   v[3] = 3;

   // OK: Assigning v to the 1st row of A1 preserves the upper matrix invariant
   //
   //        ( 0 0 0 0 )
   //   A1 = ( 0 1 2 3 )
   //        ( 0 0 0 0 )
   //        ( 0 0 0 0 )
   //
   row( A1, 1 ) = v;  // OK

   // Error: Assigning v to the 2nd row of A1 violates the upper matrix invariant! The element
   //   marked with X cannot be assigned and triggers an exception.
   //
   //        ( 0 0 0 0 )
   //   A1 = ( 0 1 2 3 )
   //        ( 0 X 2 3 )
   //        ( 0 0 0 0 )
   //
   row( A1, 2 ) = v;  // Assignment throws an exception!

   // Setup of the 3x2 dynamic matrix
   //
   //       ( 7 8 )
   //   B = ( 0 9 )
   //       ( 0 0 )
   //
   DynamicMatrix<int> B( 3UL, 2UL, 0 );
   B(0,0) = 7;
   B(0,1) = 8;
   B(1,1) = 9;

   // OK: Assigning B to a submatrix of A2 such that the upper matrix invariant can be preserved
   //
   //        ( 0 0 0 0 )
   //   A2 = ( 0 7 8 0 )
   //        ( 0 0 9 0 )
   //        ( 0 0 0 0 )
   //
   submatrix( A2, 1UL, 1UL, 3UL, 2UL ) = B;  // OK

   // Error: Assigning B to a submatrix of A2 such that the upper matrix invariant cannot be
   //   preserved! The elements marked with X cannot be assigned without violating the invariant!
   //
   //        ( 0 0 0 0 )
   //   A2 = ( X 8 8 0 )
   //        ( 0 X 9 0 )
   //        ( 0 0 0 0 )
   //
   submatrix( A2, 1UL, 0UL, 3UL, 2UL ) = B;  // Assignment throws an exception!
   \endcode

// \n \subsection uppermatrix_initialization The Lower Elements of a Dense Upper Matrix are Always Default Initialized!
//
// Although this results in a small loss of efficiency during the creation of a dense upper matrix
// this initialization is important since otherwise the upper matrix property of dense upper
// matrices would not be guaranteed:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::UpperMatrix;

   // Uninitialized, 5x5 row-major dynamic matrix
   DynamicMatrix<int,rowMajor> A( 5, 5 );

   // 5x5 row-major upper dynamic matrix with default initialized lower matrix
   UpperMatrix< DynamicMatrix<int,rowMajor> > B( 5 );
   \endcode

// \n \subsection uppermatrix_storage Dense Upper Matrices Also Store the Lower Elements!
//
// It is important to note that dense upper matrices store all elements, including the elements
// in the lower part of the matrix, and therefore don't provide any kind of memory reduction!
// There are two main reasons for this: First, storing also the lower elements guarantees maximum
// performance for many algorithms that perform vectorized operations on the upper matrix, which
// is especially true for small dense matrices. Second, conceptually the UpperMatrix adaptor
// merely restricts the interface to the matrix type \a MT and does not change the data layout
// or the underlying matrix type.
//
//
// \n \section uppermatrix_arithmetic_operations Arithmetic Operations
//
// An UpperMatrix matrix can participate in numerical operations in any way any other dense or
// sparse matrix can participate. It can also be combined with any other dense or sparse vector
// or matrix. The following code example gives an impression of the use of UpperMatrix within
// arithmetic operations:

   \code
   using mtrc::numeric::UpperMatrix;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::HybridMatrix;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::CompressedMatrix;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   DynamicMatrix<double,rowMajor> A( 3, 3 );
   CompressedMatrix<double,rowMajor> B( 3, 3 );

   UpperMatrix< DynamicMatrix<double,rowMajor> > C( 3 );
   UpperMatrix< CompressedMatrix<double,rowMajor> > D( 3 );

   UpperMatrix< HybridMatrix<float,3UL,3UL,rowMajor> > E;
   UpperMatrix< StaticMatrix<float,3UL,3UL,columnMajor> > F;

   E = A + B;     // Matrix addition and assignment to a row-major upper matrix (includes runtime check)
   F = C - D;     // Matrix subtraction and assignment to a column-major upper matrix (only compile time check)
   F = A * D;     // Matrix multiplication between a dense and a sparse matrix (includes runtime check)

   C *= 2.0;      // In-place scaling of matrix C
   E  = 2.0 * B;  // Scaling of matrix B (includes runtime check)
   F  = C * 2.0;  // Scaling of matrix C (only compile time check)

   E += A - B;    // Addition assignment (includes runtime check)
   F -= C + D;    // Subtraction assignment (only compile time check)
   F *= A * D;    // Multiplication assignment (includes runtime check)
   \endcode

// Note that it is possible to assign any kind of matrix to an upper matrix. In case the matrix
// to be assigned is not upper at compile time, a runtime check is performed.
//
//
// \n \section uppermatrix_block_structured Block-Structured Upper Matrices
//
// It is also possible to use block-structured upper matrices:

   \code
   using mtrc::numeric::CompressedMatrix;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::UpperMatrix;

   // Definition of a 5x5 block-structured upper matrix based on CompressedMatrix
   UpperMatrix< CompressedMatrix< StaticMatrix<int,3UL,3UL> > > A( 5 );
   \endcode

// Also in this case the upper matrix invariant is enforced, i.e. it is not possible to manipulate
// elements in the lower part of the matrix:

   \code
   const StaticMatrix<int,3UL,3UL> B( { { 1, -4,  5 },
										{ 6,  8, -3 },
										{ 2, -1,  2 } } )

   A.insert( 2, 4, B );  // Inserting the elements (2,4)
   A(4,2)(1,1) = -5;     // Invalid manipulation of lower matrix element; Results in an exception
   \endcode

// \n \section uppermatrix_performance Performance Considerations
//
// The \b Metric numeric library tries to exploit the properties of upper matrices whenever and wherever
// possible. Thus using an upper triangular matrix instead of a general matrix can result in a
// considerable performance improvement. However, there are also situations when using an upper
// triangular matrix introduces some overhead. The following examples demonstrate several common
// situations where upper matrices can positively or negatively impact performance.
//
// \n \subsection uppermatrix_matrix_matrix_multiplication Positive Impact: Matrix/Matrix Multiplication
//
// When multiplying two matrices, at least one of which is upper triangular, \b Metric numeric can exploit
// the fact that the lower part of the matrix contains only default elements and restrict the
// algorithm to the upper and diagonal elements. The following example demonstrates this by means
// of a dense matrix/dense matrix multiplication:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::UpperMatrix;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   UpperMatrix< DynamicMatrix<double,rowMajor> > A;
   UpperMatrix< DynamicMatrix<double,columnMajor> > B;
   DynamicMatrix<double,columnMajor> C;

   // ... Resizing and initialization

   C = A * B;
   \endcode

// In comparison to a general matrix multiplication, the performance advantage is significant,
// especially for large matrices. Therefore is it highly recommended to use the UpperMatrix
// adaptor when a matrix is known to be upper triangular. Note however that the performance
// advantage is most pronounced for dense matrices and much less so for sparse matrices.
//
// \n \subsection uppermatrix_matrix_vector_multiplication Positive Impact: Matrix/Vector Multiplication
//
// A similar performance improvement can be gained when using an upper matrix in a matrix/vector
// multiplication:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::UpperMatrix;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   UpperMatrix< DynamicMatrix<double,rowMajor> > A;
   DynamicVector<double,columnVector> x, y;

   // ... Resizing and initialization

   y = A * x;
   \endcode

// In this example, \b Metric numeric also exploits the structure of the matrix and approx. halves the
// runtime of the multiplication. Also in case of matrix/vector multiplications the performance
// improvement is most pronounced for dense matrices and much less so for sparse matrices.
//
// \n \subsection uppermatrix_assignment Negative Impact: Assignment of a General Matrix
//
// In contrast to using an upper triangular matrix on the right-hand side of an assignment (i.e.
// for read access), which introduces absolutely no performance penalty, using an upper matrix on
// the left-hand side of an assignment (i.e. for write access) may introduce additional overhead
// when it is assigned a general matrix, which is not upper triangular at compile time:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::UpperMatrix;

   UpperMatrix< DynamicMatrix<double> > A, C;
   DynamicMatrix<double> B;

   B = A;  // Only read-access to the upper matrix; no performance penalty
   C = A;  // Assignment of an upper matrix to another upper matrix; no runtime overhead
   C = B;  // Assignment of a general matrix to an upper matrix; some runtime overhead
   \endcode

// When assigning a general, potentially not upper matrix to an upper matrix it is necessary to
// check whether the matrix is upper at runtime in order to guarantee the upper triangular property
// of the upper matrix. In case it turns out to be upper triangular, it is assigned as efficiently
// as possible, if it is not, an exception is thrown. In order to prevent this runtime overhead it
// is therefore generally advisable to assign upper matrices to other upper matrices.\n
// In this context it is especially noteworthy that the addition, subtraction, and multiplication
// of two upper triangular matrices always results in another upper matrix:

   \code
   UpperMatrix< DynamicMatrix<double> > A, B, C;

   C = A + B;  // Results in an upper matrix; no runtime overhead
   C = A - B;  // Results in an upper matrix; no runtime overhead
   C = A * B;  // Results in an upper matrix; no runtime overhead
   \endcode
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO = StorageOrder_v<MT> // Storage order of the adapted matrix
		  ,
		  bool DF = IsDenseMatrix_v<MT>> // Density flag
class UpperMatrix {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
