// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_DENSE_STATICMATRIX_H
#define METRIC_NUMERIC_MATH_DENSE_STATICMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <array>
#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/AlignmentFlag.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/Forward.h>
#include <metric/numeric/math/InitializerList.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/SIMD.h>
#include <metric/numeric/math/constraints/Diagonal.h>
#include <metric/numeric/math/constraints/SameTag.h>
#include <metric/numeric/math/constraints/Symmetric.h>
#include <metric/numeric/math/dense/DenseIterator.h>
#include <metric/numeric/math/dense/Forward.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/expressions/SparseMatrix.h>
#include <metric/numeric/math/shims/Clear.h>
#include <metric/numeric/math/shims/Conjugate.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/NextMultiple.h>
#include <metric/numeric/math/shims/PrevMultiple.h>
#include <metric/numeric/math/shims/Serial.h>
#include <metric/numeric/math/traits/AddTrait.h>
#include <metric/numeric/math/traits/ColumnsTrait.h>
#include <metric/numeric/math/traits/DivTrait.h>
#include <metric/numeric/math/traits/EvaluateTrait.h>
#include <metric/numeric/math/traits/ExpandTrait.h>
#include <metric/numeric/math/traits/KronTrait.h>
#include <metric/numeric/math/traits/MapTrait.h>
#include <metric/numeric/math/traits/MultTrait.h>
#include <metric/numeric/math/traits/RepeatTrait.h>
#include <metric/numeric/math/traits/RowsTrait.h>
#include <metric/numeric/math/traits/SchurTrait.h>
#include <metric/numeric/math/traits/SolveTrait.h>
#include <metric/numeric/math/traits/SubTrait.h>
#include <metric/numeric/math/traits/SubmatrixTrait.h>
#include <metric/numeric/math/typetraits/HasConstDataAccess.h>
#include <metric/numeric/math/typetraits/HasMutableDataAccess.h>
#include <metric/numeric/math/typetraits/HasSIMDAdd.h>
#include <metric/numeric/math/typetraits/HasSIMDMult.h>
#include <metric/numeric/math/typetraits/HasSIMDSub.h>
#include <metric/numeric/math/typetraits/HighType.h>
#include <metric/numeric/math/typetraits/IsAligned.h>
#include <metric/numeric/math/typetraits/IsColumnMajorMatrix.h>
#include <metric/numeric/math/typetraits/IsColumnVector.h>
#include <metric/numeric/math/typetraits/IsContiguous.h>
#include <metric/numeric/math/typetraits/IsDenseMatrix.h>
#include <metric/numeric/math/typetraits/IsDenseVector.h>
#include <metric/numeric/math/typetraits/IsDiagonal.h>
#include <metric/numeric/math/typetraits/IsLower.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsPadded.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>
#include <metric/numeric/math/typetraits/IsRowVector.h>
#include <metric/numeric/math/typetraits/IsSIMDCombinable.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/math/typetraits/IsSparseMatrix.h>
#include <metric/numeric/math/typetraits/IsSquare.h>
#include <metric/numeric/math/typetraits/IsStrictlyLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/IsSymmetric.h>
#include <metric/numeric/math/typetraits/IsUpper.h>
#include <metric/numeric/math/typetraits/LowType.h>
#include <metric/numeric/math/typetraits/MaxSize.h>
#include <metric/numeric/math/typetraits/Size.h>
#include <metric/numeric/math/typetraits/StorageOrder.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/system/Optimizations.h>
#include <metric/numeric/util/AlignedArray.h>
#include <metric/numeric/util/AlignmentCheck.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/Memory.h>
#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/algorithms/Max.h>
#include <metric/numeric/util/algorithms/Min.h>
#include <metric/numeric/util/constraints/Const.h>
#include <metric/numeric/util/constraints/FloatingPoint.h>
#include <metric/numeric/util/constraints/Pointer.h>
#include <metric/numeric/util/constraints/Reference.h>
#include <metric/numeric/util/constraints/Vectorizable.h>
#include <metric/numeric/util/constraints/Volatile.h>
#include <metric/numeric/util/typetraits/AlignmentOf.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>
#include <metric/numeric/util/typetraits/IsSame.h>
#include <metric/numeric/util/typetraits/IsVectorizable.h>
#include <metric/numeric/util/typetraits/RemoveCV.h>
#include <metric/numeric/util/typetraits/RemoveConst.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\defgroup static_matrix StaticMatrix
// \ingroup dense_matrix
*/
/*!\brief Efficient implementation of a fixed-sized matrix.
// \ingroup static_matrix
//
// The StaticMatrix class template is the representation of a fixed-size matrix with statically
// allocated elements of arbitrary type. The type of the elements, the number of rows and columns,
// the storage order of the matrix, the alignment, the padding, and the group tag of the matrix
// can be specified via the seven template parameters:

   \code
   namespace mtrc::numeric {

   template< typename Type, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag >
   class StaticMatrix;

   } // namespace mtrc::numeric
   \endcode

//  - Type: specifies the type of the matrix elements. StaticMatrix can be used with any
//          non-cv-qualified, non-reference, non-pointer element type.
//  - M   : specifies the total number of rows of the matrix.
//  - N   : specifies the total number of columns of the matrix. Note that it is expected
//          that StaticMatrix is only used for tiny and small matrices.
//  - SO  : specifies the storage order (\c mtrc::numeric::rowMajor, \c mtrc::numeric::columnMajor) of the matrix.
//          The default value is \c mtrc::numeric::defaultStorageOrder.
//  - AF  : specifies whether the first element of every row/column is properly aligned with
//          respect to the available instruction set (SSE, AVX, ...). Possible values are
//          \c mtrc::numeric::aligned and \c mtrc::numeric::unaligned. The default value is
//          \c mtrc::numeric::defaultAlignmentFlag.
//  - PF  : specifies whether every row/column of the matrix should be padded to maximize the
//          efficiency of vectorized operations. Possible values are \c mtrc::numeric::padded and
//          \c mtrc::numeric::unpadded. The default value is \c mtrc::numeric::defaultPaddingFlag.
//  - Tag : optional type parameter to tag the matrix. The default type is \c mtrc::numeric::Group0.
//          See \ref grouping_tagging for details.
//
// Depending on the storage order, the matrix elements are either stored in a row-wise fashion
// or in a column-wise fashion. Given the 2x3 matrix

						  \f[\left(\begin{array}{*{3}{c}}
						  1 & 2 & 3 \\
						  4 & 5 & 6 \\
						  \end{array}\right)\f]\n

// in case of row-major order the elements are stored in the order

						  \f[\left(\begin{array}{*{6}{c}}
						  1 & 2 & 3 & 4 & 5 & 6. \\
						  \end{array}\right)\f]

// In case of column-major order the elements are stored in the order

						  \f[\left(\begin{array}{*{6}{c}}
						  1 & 4 & 2 & 5 & 3 & 6. \\
						  \end{array}\right)\f]

// The use of StaticMatrix is very natural and intuitive. All operations (addition, subtraction,
// multiplication, scaling, ...) can be performed on all possible combinations of row-major and
// column-major dense and sparse matrices with fitting element types. The following example gives
// an impression of the use of StaticMatrix:

   \code
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::CompressedMatrix;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   StaticMatrix<double,2UL,3UL,rowMajor> A;   // Default constructed, non-initialized, row-major 2x3 matrix
   A(0,0) = 1.0; A(0,1) = 2.0; A(0,2) = 3.0;  // Initialization of the first row
   A(1,0) = 4.0; A(1,1) = 5.0; A(1,2) = 6.0;  // Initialization of the second row

   DynamicMatrix<float,2UL,3UL,columnMajor> B;  // Default constructed column-major single precision 2x3 matrix
   B(0,0) = 1.0; B(0,1) = 3.0; B(0,2) = 5.0;    // Initialization of the first row
   B(1,0) = 2.0; B(1,1) = 4.0; B(1,2) = 6.0;    // Initialization of the second row

   CompressedMatrix<float>     C( 2, 3 );  // Empty row-major sparse single precision matrix
   StaticMatrix<float,3UL,2UL> D( 4.0F );  // Directly, homogeneously initialized single precision 3x2 matrix

   StaticMatrix<double,2UL,3UL,rowMajor>    E( A );  // Creation of a new row-major matrix as a copy of A
   StaticMatrix<double,2UL,2UL,columnMajor> F;       // Creation of a default column-major matrix

   E = A + B;     // Matrix addition and assignment to a row-major matrix
   F = A - C;     // Matrix subtraction and assignment to a column-major matrix
   F = A * D;     // Matrix multiplication between two matrices of different element types

   A *= 2.0;      // In-place scaling of matrix A
   E  = 2.0 * B;  // Scaling of matrix B
   E  = B * 2.0;  // Scaling of matrix B

   E += A - B;    // Addition assignment
   E -= A + C;    // Subtraction assignment
   F *= A * D;    // Multiplication assignment
   \endcode
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
class StaticMatrix : public DenseMatrix<StaticMatrix<Type, M, N, SO, AF, PF, Tag>, SO> {
  public:
	//**Type definitions****************************************************************************
	//! Type of this StaticMatrix instance.
	using This = StaticMatrix<Type, M, N, SO, AF, PF, Tag>;

	using BaseType = DenseMatrix<This, SO>; //!< Base type of this StaticMatrix instance.
	using ResultType = This;				//!< Result type for expression template evaluations.

	//! Result type with opposite storage order for expression template evaluations.
	using OppositeType = StaticMatrix<Type, M, N, !SO, AF, PF, Tag>;

	//! Transpose type for expression template evaluations.
	using TransposeType = StaticMatrix<Type, N, M, !SO, AF, PF, Tag>;

	using ElementType = Type;				   //!< Type of the matrix elements.
	using SIMDType = SIMDTrait_t<ElementType>; //!< SIMD type of the matrix elements.
	using TagType = Tag;					   //!< Tag type of this StaticMatrix instance.
	using ReturnType = const Type &;		   //!< Return type for expression template evaluations.
	using CompositeType = const This &;		   //!< Data type for composite expression templates.

	using Reference = Type &;			 //!< Reference to a non-constant matrix value.
	using ConstReference = const Type &; //!< Reference to a constant matrix value.
	using Pointer = Type *;				 //!< Pointer to a non-constant matrix value.
	using ConstPointer = const Type *;	 //!< Pointer to a constant matrix value.

	using Iterator = DenseIterator<Type, AF>;			 //!< Iterator over non-constant elements.
	using ConstIterator = DenseIterator<const Type, AF>; //!< Iterator over constant elements.
	//**********************************************************************************************

	//**Rebind struct definition********************************************************************
	/*!\brief Rebind mechanism to obtain a StaticMatrix with different data/element type.
	 */
	template <typename NewType> // Data type of the other matrix
	struct Rebind {
		using Other = StaticMatrix<NewType, M, N, SO, AF, PF, Tag>; //!< The type of the other StaticMatrix.
	};
	//**********************************************************************************************

	//**Resize struct definition********************************************************************
	/*!\brief Resize mechanism to obtain a StaticMatrix with different fixed dimensions.
	 */
	template <size_t NewM // Number of rows of the other matrix
			  ,
			  size_t NewN> // Number of columns of the other matrix
	struct Resize {
		using Other = StaticMatrix<Type, NewM, NewN, SO, AF, PF, Tag>; //!< The type of the other StaticMatrix.
	};
	//**********************************************************************************************

	//**Compilation flags***************************************************************************
	//! Compilation flag for SIMD optimization.
	/*! The \a simdEnabled compilation flag indicates whether expressions the matrix is involved
		in can be optimized via SIMD operations. In case the element type of the matrix is a
		vectorizable data type, the \a simdEnabled compilation flag is set to \a true, otherwise
		it is set to \a false. */
	static constexpr bool simdEnabled = IsVectorizable_v<Type>;

	//! Compilation flag for SMP assignments.
	/*! The \a smpAssignable compilation flag indicates whether the matrix can be used in SMP
		(shared memory parallel) assignments (both on the left-hand and right-hand side of the
		assignment). */
	static constexpr bool smpAssignable = false;
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline StaticMatrix();
	explicit inline StaticMatrix(const Type &init);
	constexpr StaticMatrix(initializer_list<initializer_list<Type>> list);

	template <typename Other> inline StaticMatrix(size_t m, size_t n, const Other *array);

	template <typename Other, size_t Rows, size_t Cols> inline StaticMatrix(const Other (&array)[Rows][Cols]);

	template <typename Other, size_t Rows, size_t Cols>
	StaticMatrix(const std::array<std::array<Other, Cols>, Rows> &array);

	constexpr StaticMatrix(const StaticMatrix &m);

	template <typename Other, bool SO2, AlignmentFlag AF2, PaddingFlag PF2>
	inline StaticMatrix(const StaticMatrix<Other, M, N, SO2, AF2, PF2, Tag> &m);

	template <typename MT, bool SO2> inline StaticMatrix(const Matrix<MT, SO2> &m);
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~StaticMatrix() = default;
	//@}
	//**********************************************************************************************

	//**Data access functions***********************************************************************
	/*!\name Data access functions */
	//@{
	constexpr Reference operator()(size_t i, size_t j) noexcept;
	constexpr ConstReference operator()(size_t i, size_t j) const noexcept;
	inline Reference at(size_t i, size_t j);
	inline ConstReference at(size_t i, size_t j) const;
	constexpr Pointer data() noexcept;
	constexpr ConstPointer data() const noexcept;
	constexpr Pointer data(size_t i) noexcept;
	constexpr ConstPointer data(size_t i) const noexcept;
	constexpr Iterator begin(size_t i) noexcept;
	constexpr ConstIterator begin(size_t i) const noexcept;
	constexpr ConstIterator cbegin(size_t i) const noexcept;
	constexpr Iterator end(size_t i) noexcept;
	constexpr ConstIterator end(size_t i) const noexcept;
	constexpr ConstIterator cend(size_t i) const noexcept;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	constexpr StaticMatrix &operator=(const Type &set) &;
	constexpr StaticMatrix &operator=(initializer_list<initializer_list<Type>> list) &;

	template <typename Other, size_t Rows, size_t Cols>
	inline StaticMatrix &operator=(const Other (&array)[Rows][Cols]) &;

	template <typename Other, size_t Rows, size_t Cols>
	inline StaticMatrix &operator=(const std::array<std::array<Other, Cols>, Rows> &array) &;

	constexpr StaticMatrix &operator=(const StaticMatrix &rhs) &;

	template <typename Other, bool SO2, AlignmentFlag AF2, PaddingFlag PF2>
	inline StaticMatrix &operator=(const StaticMatrix<Other, M, N, SO2, AF2, PF2, Tag> &rhs) &;

	template <typename MT, bool SO2> inline StaticMatrix &operator=(const Matrix<MT, SO2> &rhs) &;
	template <typename MT, bool SO2> inline StaticMatrix &operator+=(const Matrix<MT, SO2> &rhs) &;
	template <typename MT, bool SO2> inline StaticMatrix &operator-=(const Matrix<MT, SO2> &rhs) &;
	template <typename MT, bool SO2> inline StaticMatrix &operator%=(const Matrix<MT, SO2> &rhs) &;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	static constexpr size_t rows() noexcept;
	static constexpr size_t columns() noexcept;
	static constexpr size_t spacing() noexcept;
	static constexpr size_t capacity() noexcept;
	inline size_t capacity(size_t i) const noexcept;
	inline size_t nonZeros() const;
	inline size_t nonZeros(size_t i) const;
	constexpr void reset();
	inline void reset(size_t i);
	inline void swap(StaticMatrix &m) noexcept;
	//@}
	//**********************************************************************************************

	//**Numeric functions***************************************************************************
	/*!\name Numeric functions */
	//@{
	inline StaticMatrix &transpose();
	inline StaticMatrix &ctranspose();

	template <typename Other> inline StaticMatrix &scale(const Other &scalar);
	//@}
	//**********************************************************************************************

	//**Memory functions****************************************************************************
	/*!\name Memory functions */
	//@{
	static inline void *operator new(std::size_t size);
	static inline void *operator new[](std::size_t size);
	static inline void *operator new(std::size_t size, const std::nothrow_t &);
	static inline void *operator new[](std::size_t size, const std::nothrow_t &);

	static inline void operator delete(void *ptr);
	static inline void operator delete[](void *ptr);
	static inline void operator delete(void *ptr, const std::nothrow_t &);
	static inline void operator delete[](void *ptr, const std::nothrow_t &);
	//@}
	//**********************************************************************************************

  private:
	//**********************************************************************************************
	//! The number of elements packed within a single SIMD vector.
	static constexpr size_t SIMDSIZE = SIMDTrait<Type>::size;

	//! Alignment adjustment.
	static constexpr size_t NN = (PF == padded ? nextMultiple(N, SIMDSIZE) : N);
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Helper variable template for the explicit application of the SFINAE principle.
	template <typename MT>
	static constexpr bool VectorizedAssign_v =
		(useOptimizedKernels && NN >= SIMDSIZE && simdEnabled && MT::simdEnabled &&
		 IsSIMDCombinable_v<Type, ElementType_t<MT>> && IsRowMajorMatrix_v<MT>);
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Helper variable template for the explicit application of the SFINAE principle.
	template <typename MT>
	static constexpr bool VectorizedAddAssign_v =
		(VectorizedAssign_v<MT> && HasSIMDAdd_v<Type, ElementType_t<MT>> && !IsDiagonal_v<MT>);
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Helper variable template for the explicit application of the SFINAE principle.
	template <typename MT>
	static constexpr bool VectorizedSubAssign_v =
		(VectorizedAssign_v<MT> && HasSIMDSub_v<Type, ElementType_t<MT>> && !IsDiagonal_v<MT>);
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Helper variable template for the explicit application of the SFINAE principle.
	template <typename MT>
	static constexpr bool VectorizedSchurAssign_v = (VectorizedAssign_v<MT> && HasSIMDMult_v<Type, ElementType_t<MT>>);
	/*! \endcond */
	//**********************************************************************************************

  public:
	//**Debugging functions*************************************************************************
	/*!\name Debugging functions */
	//@{
	constexpr bool isIntact() const noexcept;
	//@}
	//**********************************************************************************************

	//**Expression template evaluation functions****************************************************
	/*!\name Expression template evaluation functions */
	//@{
	template <typename Other> inline bool canAlias(const Other *alias) const noexcept;
	template <typename Other> inline bool isAliased(const Other *alias) const noexcept;

	static constexpr bool isAligned() noexcept;

	METRIC_NUMERIC_ALWAYS_INLINE SIMDType load(size_t i, size_t j) const noexcept;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDType loada(size_t i, size_t j) const noexcept;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDType loadu(size_t i, size_t j) const noexcept;

	METRIC_NUMERIC_ALWAYS_INLINE void store(size_t i, size_t j, const SIMDType &value) noexcept;
	METRIC_NUMERIC_ALWAYS_INLINE void storea(size_t i, size_t j, const SIMDType &value) noexcept;
	METRIC_NUMERIC_ALWAYS_INLINE void storeu(size_t i, size_t j, const SIMDType &value) noexcept;
	METRIC_NUMERIC_ALWAYS_INLINE void stream(size_t i, size_t j, const SIMDType &value) noexcept;

	template <typename MT, bool SO2>
	inline auto assign(const DenseMatrix<MT, SO2> &rhs) -> DisableIf_t<VectorizedAssign_v<MT>>;

	template <typename MT, bool SO2>
	inline auto assign(const DenseMatrix<MT, SO2> &rhs) -> EnableIf_t<VectorizedAssign_v<MT>>;

	template <typename MT> inline void assign(const SparseMatrix<MT, SO> &rhs);
	template <typename MT> inline void assign(const SparseMatrix<MT, !SO> &rhs);

	template <typename MT, bool SO2>
	inline auto addAssign(const DenseMatrix<MT, SO2> &rhs) -> DisableIf_t<VectorizedAddAssign_v<MT>>;

	template <typename MT, bool SO2>
	inline auto addAssign(const DenseMatrix<MT, SO2> &rhs) -> EnableIf_t<VectorizedAddAssign_v<MT>>;

	template <typename MT> inline void addAssign(const SparseMatrix<MT, SO> &rhs);
	template <typename MT> inline void addAssign(const SparseMatrix<MT, !SO> &rhs);

	template <typename MT, bool SO2>
	inline auto subAssign(const DenseMatrix<MT, SO2> &rhs) -> DisableIf_t<VectorizedSubAssign_v<MT>>;

	template <typename MT, bool SO2>
	inline auto subAssign(const DenseMatrix<MT, SO2> &rhs) -> EnableIf_t<VectorizedSubAssign_v<MT>>;

	template <typename MT> inline void subAssign(const SparseMatrix<MT, SO> &rhs);
	template <typename MT> inline void subAssign(const SparseMatrix<MT, !SO> &rhs);

	template <typename MT, bool SO2>
	inline auto schurAssign(const DenseMatrix<MT, SO2> &rhs) -> DisableIf_t<VectorizedSchurAssign_v<MT>>;

	template <typename MT, bool SO2>
	inline auto schurAssign(const DenseMatrix<MT, SO2> &rhs) -> EnableIf_t<VectorizedSchurAssign_v<MT>>;

	template <typename MT> inline void schurAssign(const SparseMatrix<MT, SO> &rhs);
	template <typename MT> inline void schurAssign(const SparseMatrix<MT, !SO> &rhs);
	//@}
	//**********************************************************************************************

  private:
	//**Utility functions***************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	inline void transpose(TrueType);
	inline void transpose(FalseType);
	inline void ctranspose(TrueType);
	inline void ctranspose(FalseType);
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	//! Alignment of the data elements.
	static constexpr size_t Alignment = (AF == aligned ? AlignmentOf_v<Type> : std::alignment_of<Type>::value);

	//! Type of the aligned storage.
	using AlignedStorage = AlignedArray<Type, M * NN, Alignment>;
	//**********************************************************************************************

	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	AlignedStorage v_; //!< The statically allocated matrix elements.
					   /*!< Access to the matrix elements is gained via the function call
							operator. In case of row-major order the memory layout of the
							elements is
							\f[\left(\begin{array}{*{5}{c}}
							0            & 1             & 2             & \cdots & N-1         \\
							N            & N+1           & N+2           & \cdots & 2 \cdot N-1 \\
							\vdots       & \vdots        & \vdots        & \ddots & \vdots      \\
							M \cdot N-N  & M \cdot N-N+1 & M \cdot N-N+2 & \cdots & M \cdot N-1 \\
							\end{array}\right)\f]. */
	//@}
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_POINTER_TYPE(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_REFERENCE_TYPE(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CONST(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VOLATILE(Type);
	METRIC_NUMERIC_STATIC_ASSERT(AF == unaligned || PF == padded || N % SIMDSIZE == 0UL);
	METRIC_NUMERIC_STATIC_ASSERT(PF == unpadded || NN % SIMDSIZE == 0UL);
	METRIC_NUMERIC_STATIC_ASSERT(NN >= N);
	METRIC_NUMERIC_STATIC_ASSERT(IsVectorizable_v<Type> || NN == N);
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  DEDUCTION GUIDES
//
//=================================================================================================

//*************************************************************************************************
#if METRIC_NUMERIC_CPP17_MODE

template <typename Type, size_t M, size_t N> StaticMatrix(Type (&)[M][N]) -> StaticMatrix<RemoveCV_t<Type>, M, N>;

template <typename Type, size_t M, size_t N>
StaticMatrix(std::array<std::array<Type, N>, M>) -> StaticMatrix<Type, M, N>;

#endif
//*************************************************************************************************

//=================================================================================================
//
//  CONSTRUCTORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief The default constructor for StaticMatrix.
//
// All matrix elements are initialized to the default value (i.e. 0 for integral data types).
//
// Note that it is possible to skip the default initialization by means of the
// \a METRIC_NUMERIC_USE_DEFAULT_INITIALIZATION configuration switch. In case the switch is set to 1
// all elements are initialized to their respective default. In case the switch is set to 0 the
// default initialization is skipped and the elements are not initialized. Please note that this
// switch is only effective in case the elements are of fundamental type (i.e. integral or
// floating point). In case the elements are of class type, this switch has no effect. See
// the <tt><metric/numeric/config/Optimizations.h></tt> configuration file for more details.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag>::StaticMatrix()
#if METRIC_NUMERIC_USE_DEFAULT_INITIALIZATION
	: v_() // The statically allocated matrix elements
#endif
{
#if !METRIC_NUMERIC_USE_DEFAULT_INITIALIZATION
	using mtrc::numeric::clear;

	if (IsNumeric_v<Type> && PF == padded) {
		for (size_t i = 0UL; i < M; ++i)
			for (size_t j = N; j < NN; ++j)
				clear(v_[i * NN + j]);
	}
#endif

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Constructor for a homogenous initialization of all elements.
//
// \param init Initial value for all matrix elements.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag>::StaticMatrix(const Type &init)
// v_ is intentionally left uninitialized
{
	using mtrc::numeric::clear;

	for (size_t i = 0UL; i < M; ++i) {
		for (size_t j = 0UL; j < N; ++j)
			v_[i * NN + j] = init;

		for (size_t j = N; j < NN; ++j)
			clear(v_[i * NN + j]);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief List initialization of all matrix elements.
//
// \param list The initializer list.
// \exception std::invalid_argument Invalid setup of static matrix.
//
// This constructor provides the option to explicitly initialize the elements of the matrix by
// means of an initializer list:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::StaticMatrix<int,3,3,rowMajor> A{ { 1, 2, 3 },
											{ 4, 5 },
											{ 7, 8, 9 } };
   \endcode

// The matrix elements are (copy) assigned the values of the given initializer list. Missing
// values are initialized as default (as e.g. the value 6 in the example). Note that in case the
// size of the top-level initializer list does not match the number of rows of the matrix or the
// size of any nested list exceeds the number of columns, a \a std::invalid_argument exception
// is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr StaticMatrix<Type, M, N, SO, AF, PF, Tag>::StaticMatrix(initializer_list<initializer_list<Type>> list)
	: v_() // The statically allocated matrix elements
{
	if (list.size() != M || determineColumns(list) > N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid setup of static matrix");
	}

	size_t i(0UL);

	for (const auto &rowList : list) {
		size_t j(0UL);
		for (const auto &element : rowList) {
			v_[i * NN + j] = element;
			++j;
		}
		++i;
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Array initialization of all matrix elements.
//
// \param m The number of rows of the matrix.
// \param n The number of columns of the matrix.
// \param array Dynamic array for the initialization.
//
// This constructor offers the option to directly initialize the elements of the matrix with
// a dynamic array:

   \code
   using mtrc::numeric::rowMajor;

   int* array = new int[6];
   // ... Initialization of the dynamic array
   mtrc::numeric::StaticMatrix<int,3,4,rowMajor> v( 2UL, 3UL, array );
   delete[] array;
   \endcode

// The matrix is initialized with the values from the given array. Missing values are initialized
// with default values. In case the specified number of rows and/or columns exceeds the maximum
// number of rows/column of the static matrix (i.e. \a m is larger than M or \a n is larger than
// N), a \a std::invalid_argument exception is thrown.\n
// Note that it is expected that the given \a array has at least \a m by \a n elements. Providing
// an array with less elements results in undefined behavior!
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	  // Type tag
template <typename Other> // Data type of the initialization array
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag>::StaticMatrix(size_t m, size_t n, const Other *array)
// v_ is intentionally left uninitialized
{
	using mtrc::numeric::clear;

	if (m > M || n > N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid setup of static matrix");
	}

	for (size_t i = 0UL; i < m; ++i) {
		for (size_t j = 0UL; j < n; ++j)
			v_[i * NN + j] = array[i * n + j];

		if (IsNumeric_v<Type>) {
			for (size_t j = n; j < NN; ++j)
				clear(v_[i * NN + j]);
		}
	}

	if (IsNumeric_v<Type>) {
		for (size_t i = m; i < M; ++i) {
			for (size_t j = 0UL; j < NN; ++j)
				clear(v_[i * NN + j]);
		}
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Array initialization of all matrix elements.
//
// \param array Static array for the initialization.
//
// This constructor offers the option to directly initialize the elements of the matrix with
// a static array:

   \code
   using mtrc::numeric::rowMajor;

   const int init[3][3] = { { 1, 2, 3 },
							{ 4, 5 },
							{ 7, 8, 9 } };
   mtrc::numeric::StaticMatrix<int,3,3,rowMajor> A( init );
   \endcode

// The matrix is initialized with the values from the given static array. Missing values are
// initialized with default values (as e.g. the value 6 in the example).
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	 // Type tag
template <typename Other // Data type of the static array
		  ,
		  size_t Rows // Number of rows of the static array
		  ,
		  size_t Cols> // Number of columns of the static array
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag>::StaticMatrix(const Other (&array)[Rows][Cols])
// v_ is intentionally left uninitialized
{
	using mtrc::numeric::clear;

	METRIC_NUMERIC_STATIC_ASSERT(Rows == M && Cols == N);

	for (size_t i = 0UL; i < M; ++i) {
		for (size_t j = 0UL; j < N; ++j)
			v_[i * NN + j] = array[i][j];

		for (size_t j = N; j < NN; ++j)
			clear(v_[i * NN + j]);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Initialization of all matrix elements from the given std::array.
//
// \param array The given std::array for the initialization.
//
// This constructor offers the option to directly initialize the elements of the matrix with
// a std::array:

   \code
   using mtrc::numeric::rowMajor;

   const std::array<std::array<int,3>,3> init = { { { 1, 2, 3 },
													{ 4, 5 },
													{ 7, 8, 9 } } };
   mtrc::numeric::StaticMatrix<int,3,3,rowMajor> A( init );
   \endcode

// The matrix is initialized with the values from the given std::array. Missing values are
// initialized with default values (as e.g. the value 6 in the example).
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	 // Type tag
template <typename Other // Data type of the std::array
		  ,
		  size_t Rows // Number of rows of the std::array
		  ,
		  size_t Cols> // Number of columns of the std::array
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag>::StaticMatrix(const std::array<std::array<Other, Cols>, Rows> &array)
// v_ is intentionally left uninitialized
{
	using mtrc::numeric::clear;

	METRIC_NUMERIC_STATIC_ASSERT(Rows == M && Cols == N);

	for (size_t i = 0UL; i < M; ++i) {
		for (size_t j = 0UL; j < N; ++j)
			v_[i * NN + j] = array[i][j];

		for (size_t j = N; j < NN; ++j)
			clear(v_[i * NN + j]);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief The copy constructor for StaticMatrix.
//
// \param m Matrix to be copied.
//
// The copy constructor is explicitly defined in order to enable/facilitate NRV optimization.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr StaticMatrix<Type, M, N, SO, AF, PF, Tag>::StaticMatrix(const StaticMatrix &m)
	: BaseType() // Initialization of the base class
	  ,
	  v_(m.v_) // The statically allocated matrix elements
{
	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Conversion constructor from different StaticMatrix instances.
//
// \param m Matrix to be copied.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	 // Type tag
template <typename Other // Data type of the other matrix
		  ,
		  bool SO2 // Storage order of the other matrix
		  ,
		  AlignmentFlag AF2 // Alignment flag of the other matrix
		  ,
		  PaddingFlag PF2> // Padding flag of the other matrix
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag>::StaticMatrix(const StaticMatrix<Other, M, N, SO2, AF2, PF2, Tag> &m)
// v_ is intentionally left uninitialized
{
	for (size_t i = 0UL; i < M; ++i) {
		for (size_t j = 0UL; j < N; ++j)
			v_[i * NN + j] = m(i, j);

		for (size_t j = N; j < NN; ++j)
			clear(v_[i * NN + j]);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Conversion constructor from different matrices.
//
// \param m Matrix to be copied.
// \exception std::invalid_argument Invalid setup of static matrix.
//
// This constructor initializes the static matrix from the given matrix. In case the size of
// the given matrix does not match the size of the static matrix (i.e. the number of rows is
// not M or the number of columns is not N), a \a std::invalid_argument exception is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the other matrix
		  ,
		  bool SO2> // Storage order of the other matrix
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag>::StaticMatrix(const Matrix<MT, SO2> &m)
// v_ is intentionally left uninitialized
{
	using mtrc::numeric::assign;
	using mtrc::numeric::clear;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TAG(Tag, TagType_t<MT>);

	if ((*m).rows() != M || (*m).columns() != N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid setup of static matrix");
	}

	for (size_t i = 0UL; i < M; ++i) {
		for (size_t j = (IsSparseMatrix_v<MT> ? 0UL : N); j < NN; ++j) {
			clear(v_[i * NN + j]);
		}
	}

	assign(*this, *m);

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
//*************************************************************************************************

//=================================================================================================
//
//  DATA ACCESS FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief 2D-access to the matrix elements.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \return Reference to the accessed value.
//
// This function only performs an index check in case METRIC_NUMERIC_USER_ASSERT() is active. In contrast,
// the at() function is guaranteed to perform a check of the given access indices.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::Reference
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator()(size_t i, size_t j) noexcept
{
	METRIC_NUMERIC_USER_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_USER_ASSERT(j < N, "Invalid column access index");
	return v_[i * NN + j];
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief 2D-access to the matrix elements.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \return Reference-to-const to the accessed value.
//
// This function only performs an index check in case METRIC_NUMERIC_USER_ASSERT() is active. In contrast,
// the at() function is guaranteed to perform a check of the given access indices.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::ConstReference
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator()(size_t i, size_t j) const noexcept
{
	METRIC_NUMERIC_USER_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_USER_ASSERT(j < N, "Invalid column access index");
	return v_[i * NN + j];
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Checked access to the matrix elements.
//
// \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
// \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
// \return Reference to the accessed value.
// \exception std::out_of_range Invalid matrix access index.
//
// In contrast to the subscript operator this function always performs a check of the given
// access indices.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::Reference
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::at(size_t i, size_t j)
{
	if (i >= M) {
		METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid row access index");
	}
	if (j >= N) {
		METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid column access index");
	}
	return (*this)(i, j);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Checked access to the matrix elements.
//
// \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
// \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
// \return Reference to the accessed value.
// \exception std::out_of_range Invalid matrix access index.
//
// In contrast to the subscript operator this function always performs a check of the given
// access indices.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::ConstReference
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::at(size_t i, size_t j) const
{
	if (i >= M) {
		METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid row access index");
	}
	if (j >= N) {
		METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid column access index");
	}
	return (*this)(i, j);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Low-level data access to the matrix elements.
//
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage of the static matrix. Note that you
// can NOT assume that all matrix elements lie adjacent to each other! The static matrix may
// use techniques such as padding to improve the alignment of the data. Whereas the number of
// elements within a row/column are given by the \c rows() and \c columns() member functions,
// respectively, the total number of elements including padding is given by the \c spacing()
// member function.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::Pointer
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::data() noexcept
{
	return v_;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Low-level data access to the matrix elements.
//
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage of the static matrix. Note that you
// can NOT assume that all matrix elements lie adjacent to each other! The static matrix may
// use techniques such as padding to improve the alignment of the data. Whereas the number of
// elements within a row/column are given by the \c rows() and \c columns() member functions,
// respectively, the total number of elements including padding is given by the \c spacing()
// member function.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::ConstPointer
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::data() const noexcept
{
	return v_;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Low-level data access to the matrix elements of row/column \a i.
//
// \param i The row/column index.
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage for the elements in row/column \a i.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::Pointer
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::data(size_t i) noexcept
{
	METRIC_NUMERIC_USER_ASSERT(i < M, "Invalid dense matrix row access index");
	return v_ + i * NN;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Low-level data access to the matrix elements of row/column \a i.
//
// \param i The row/column index.
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage for the elements in row/column \a i.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::ConstPointer
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::data(size_t i) const noexcept
{
	METRIC_NUMERIC_USER_ASSERT(i < M, "Invalid dense matrix row access index");
	return v_ + i * NN;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns an iterator to the first element of row/column \a i.
//
// \param i The row/column index.
// \return Iterator to the first element of row/column \a i.
//
// This function returns a row/column iterator to the first element of row/column \a i. In case
// the storage order is set to \a rowMajor the function returns an iterator to the first element
// of row \a i, in case the storage flag is set to \a columnMajor the function returns an iterator
// to the first element of column \a i.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::Iterator
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::begin(size_t i) noexcept
{
	METRIC_NUMERIC_USER_ASSERT(i < M, "Invalid dense matrix row access index");
	return Iterator(v_ + i * NN);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns an iterator to the first element of row/column \a i.
//
// \param i The row/column index.
// \return Iterator to the first element of row/column \a i.
//
// This function returns a row/column iterator to the first element of row/column \a i. In case
// the storage order is set to \a rowMajor the function returns an iterator to the first element
// of row \a i, in case the storage flag is set to \a columnMajor the function returns an iterator
// to the first element of column \a i.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::ConstIterator
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::begin(size_t i) const noexcept
{
	METRIC_NUMERIC_USER_ASSERT(i < M, "Invalid dense matrix row access index");
	return ConstIterator(v_ + i * NN);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns an iterator to the first element of row/column \a i.
//
// \param i The row/column index.
// \return Iterator to the first element of row/column \a i.
//
// This function returns a row/column iterator to the first element of row/column \a i. In case
// the storage order is set to \a rowMajor the function returns an iterator to the first element
// of row \a i, in case the storage flag is set to \a columnMajor the function returns an iterator
// to the first element of column \a i.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::ConstIterator
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::cbegin(size_t i) const noexcept
{
	METRIC_NUMERIC_USER_ASSERT(i < M, "Invalid dense matrix row access index");
	return ConstIterator(v_ + i * NN);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of row/column \a i.
//
// \param i The row/column index.
// \return Iterator just past the last element of row/column \a i.
//
// This function returns an row/column iterator just past the last element of row/column \a i.
// In case the storage order is set to \a rowMajor the function returns an iterator just past
// the last element of row \a i, in case the storage flag is set to \a columnMajor the function
// returns an iterator just past the last element of column \a i.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::Iterator
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::end(size_t i) noexcept
{
	METRIC_NUMERIC_USER_ASSERT(i < M, "Invalid dense matrix row access index");
	return Iterator(v_ + i * NN + N);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of row/column \a i.
//
// \param i The row/column index.
// \return Iterator just past the last element of row/column \a i.
//
// This function returns an row/column iterator just past the last element of row/column \a i.
// In case the storage order is set to \a rowMajor the function returns an iterator just past
// the last element of row \a i, in case the storage flag is set to \a columnMajor the function
// returns an iterator just past the last element of column \a i.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::ConstIterator
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::end(size_t i) const noexcept
{
	METRIC_NUMERIC_USER_ASSERT(i < M, "Invalid dense matrix row access index");
	return ConstIterator(v_ + i * NN + N);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of row/column \a i.
//
// \param i The row/column index.
// \return Iterator just past the last element of row/column \a i.
//
// This function returns an row/column iterator just past the last element of row/column \a i.
// In case the storage order is set to \a rowMajor the function returns an iterator just past
// the last element of row \a i, in case the storage flag is set to \a columnMajor the function
// returns an iterator just past the last element of column \a i.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::ConstIterator
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::cend(size_t i) const noexcept
{
	METRIC_NUMERIC_USER_ASSERT(i < M, "Invalid dense matrix row access index");
	return ConstIterator(v_ + i * NN + N);
}
//*************************************************************************************************

//=================================================================================================
//
//  ASSIGNMENT OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Homogenous assignment to all matrix elements.
//
// \param set Scalar value to be assigned to all matrix elements.
// \return Reference to the assigned matrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr StaticMatrix<Type, M, N, SO, AF, PF, Tag> &
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator=(const Type &set) &
{
	for (size_t i = 0UL; i < M; ++i)
		for (size_t j = 0UL; j < N; ++j)
			v_[i * NN + j] = set;

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief List assignment to all matrix elements.
//
// \param list The initializer list.
// \exception std::invalid_argument Invalid assignment to static matrix.
//
// This assignment operator offers the option to directly assign to all elements of the matrix
// by means of an initializer list:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::StaticMatrix<int,3,3,rowMajor> A;
   A = { { 1, 2, 3 },
		 { 4, 5 },
		 { 7, 8, 9 } };
   \endcode

// The matrix elements are (copy) assigned the values from the given initializer list. Missing
// values are initialized as default (as e.g. the value 6 in the example). Note that in case the
// size of the top-level initializer list does not match the number of rows of the matrix or the
// size of any nested list exceeds the number of columns, a \a std::invalid_argument exception
// is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr StaticMatrix<Type, M, N, SO, AF, PF, Tag> &
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator=(initializer_list<initializer_list<Type>> list) &
{
	using mtrc::numeric::clear;

	if (list.size() != M || determineColumns(list) > N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to static matrix");
	}

	size_t i(0UL);

	for (const auto &rowList : list) {
		size_t j(0UL);
		for (const auto &element : rowList) {
			v_[i * NN + j] = element;
			++j;
		}
		for (; j < N; ++j) {
			clear(v_[i * NN + j]);
		}
		++i;
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Array assignment to all matrix elements.
//
// \param array Static array for the assignment.
// \return Reference to the assigned matrix.
//
// This assignment operator offers the option to directly set all elements of the matrix:

   \code
   using mtrc::numeric::rowMajor;

   const int init[3][3] = { { 1, 2, 3 },
							{ 4, 5 },
							{ 7, 8, 9 } };
   mtrc::numeric::StaticMatrix<int,3UL,3UL,rowMajor> A;
   A = init;
   \endcode

// The matrix is assigned the values from the given static array. Missing values are initialized
// with default values (as e.g. the value 6 in the example).
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	 // Type tag
template <typename Other // Data type of the static array
		  ,
		  size_t Rows // Number of rows of the static array
		  ,
		  size_t Cols> // Number of columns of the static array
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag> &
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator=(const Other (&array)[Rows][Cols]) &
{
	METRIC_NUMERIC_STATIC_ASSERT(Rows == M && Cols == N);

	for (size_t i = 0UL; i < M; ++i)
		for (size_t j = 0UL; j < N; ++j)
			v_[i * NN + j] = array[i][j];

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Array assignment to all matrix elements.
//
// \param array The given std::array for the assignment.
// \return Reference to the assigned matrix.
//
// This assignment operator offers the option to directly set all elements of the matrix:

   \code
   using mtrc::numeric::rowMajor;

   const std::array<std::array<int,3UL>,3UL> init = {{ { 1, 2, 3 },
														{ 4, 5 },
														{ 7, 8, 9 } };
   mtrc::numeric::StaticMatrix<int,3UL,3UL,rowMajor> A;
   A = init;
   \endcode

// The matrix is assigned the values from the given std::array. Missing values are initialized
// with default values (as e.g. the value 6 in the example).
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	 // Type tag
template <typename Other // Data type of the static array
		  ,
		  size_t Rows // Number of rows of the static array
		  ,
		  size_t Cols> // Number of columns of the static array
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag> &
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator=(const std::array<std::array<Other, Cols>, Rows> &array) &
{
	METRIC_NUMERIC_STATIC_ASSERT(Rows == M && Cols == N);

	for (size_t i = 0UL; i < M; ++i)
		for (size_t j = 0UL; j < N; ++j)
			v_[i * NN + j] = array[i][j];

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Copy assignment operator for StaticMatrix.
//
// \param rhs Matrix to be copied.
// \return Reference to the assigned matrix.
//
// Explicit definition of a copy assignment operator for performance reasons.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr StaticMatrix<Type, M, N, SO, AF, PF, Tag> &
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator=(const StaticMatrix &rhs) &
{
	v_ = rhs.v_;

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Assignment operator for different StaticMatrix instances.
//
// \param rhs Matrix to be copied.
// \return Reference to the assigned matrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	 // Type tag
template <typename Other // Data type of the other matrix
		  ,
		  bool SO2 // Storage order of the other matrix
		  ,
		  AlignmentFlag AF2 // Alignment flag of the other matrix
		  ,
		  PaddingFlag PF2> // Padding flag of the other matrix
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag> &
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator=(const StaticMatrix<Other, M, N, SO2, AF2, PF2, Tag> &rhs) &
{
	using mtrc::numeric::assign;

	assign(*this, *rhs);

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Assignment operator for different matrices.
//
// \param rhs Matrix to be copied.
// \return Reference to the assigned matrix.
// \exception std::invalid_argument Invalid assignment to static matrix.
//
// This constructor initializes the matrix as a copy of the given matrix. In case the
// number of rows of the given matrix is not M or the number of columns is not N, a
// \a std::invalid_argument exception is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag> &
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator=(const Matrix<MT, SO2> &rhs) &
{
	using mtrc::numeric::assign;

	using TT = decltype(trans(*this));
	using CT = decltype(ctrans(*this));
	using IT = decltype(inv(*this));

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TAG(Tag, TagType_t<MT>);

	if ((*rhs).rows() != M || (*rhs).columns() != N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to static matrix");
	}

	if (IsSame_v<MT, TT> && (*rhs).isAliased(this)) {
		transpose(typename IsSquare<This>::Type());
	} else if (IsSame_v<MT, CT> && (*rhs).isAliased(this)) {
		ctranspose(typename IsSquare<This>::Type());
	} else if (!IsSame_v<MT, IT> && (*rhs).canAlias(this)) {
		StaticMatrix tmp(*rhs);
		assign(*this, tmp);
	} else {
		if (IsSparseMatrix_v<MT>)
			reset();
		assign(*this, *rhs);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition assignment operator for the addition of a matrix (\f$ A+=B \f$).
//
// \param rhs The right-hand side matrix to be added to the matrix.
// \return Reference to the matrix.
// \exception std::invalid_argument Matrix sizes do not match.
//
// In case the current sizes of the two matrices don't match, a \a std::invalid_argument exception
// is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag> &
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator+=(const Matrix<MT, SO2> &rhs) &
{
	using mtrc::numeric::addAssign;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TAG(Tag, TagType_t<MT>);

	if ((*rhs).rows() != M || (*rhs).columns() != N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Matrix sizes do not match");
	}

	if ((*rhs).canAlias(this)) {
		const ResultType_t<MT> tmp(*rhs);
		addAssign(*this, tmp);
	} else {
		addAssign(*this, *rhs);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Subtraction assignment operator for the subtraction of a matrix (\f$ A-=B \f$).
//
// \param rhs The right-hand side matrix to be subtracted from the matrix.
// \return Reference to the matrix.
// \exception std::invalid_argument Matrix sizes do not match.
//
// In case the current sizes of the two matrices don't match, a \a std::invalid_argument exception
// is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag> &
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator-=(const Matrix<MT, SO2> &rhs) &
{
	using mtrc::numeric::subAssign;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TAG(Tag, TagType_t<MT>);

	if ((*rhs).rows() != M || (*rhs).columns() != N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Matrix sizes do not match");
	}

	if ((*rhs).canAlias(this)) {
		const ResultType_t<MT> tmp(*rhs);
		subAssign(*this, tmp);
	} else {
		subAssign(*this, *rhs);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Schur product assignment operator for the multiplication of a matrix (\f$ A\circ=B \f$).
//
// \param rhs The right-hand side matrix for the Schur product.
// \return Reference to the matrix.
// \exception std::invalid_argument Matrix sizes do not match.
//
// In case the current sizes of the two matrices don't match, a \a std::invalid_argument exception
// is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag> &
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator%=(const Matrix<MT, SO2> &rhs) &
{
	using mtrc::numeric::schurAssign;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TAG(Tag, TagType_t<MT>);

	if ((*rhs).rows() != M || (*rhs).columns() != N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Matrix sizes do not match");
	}

	if ((*rhs).canAlias(this)) {
		const ResultType_t<MT> tmp(*rhs);
		schurAssign(*this, tmp);
	} else {
		schurAssign(*this, *rhs);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
//*************************************************************************************************

//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the current number of rows of the matrix.
//
// \return The number of rows of the matrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr size_t StaticMatrix<Type, M, N, SO, AF, PF, Tag>::rows() noexcept
{
	return M;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the current number of columns of the matrix.
//
// \return The number of columns of the matrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr size_t StaticMatrix<Type, M, N, SO, AF, PF, Tag>::columns() noexcept
{
	return N;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the spacing between the beginning of two rows.
//
// \return The spacing between the beginning of two rows.
//
// This function returns the spacing between the beginning of two rows, i.e. the total number
// of elements of a row.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr size_t StaticMatrix<Type, M, N, SO, AF, PF, Tag>::spacing() noexcept
{
	return NN;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the maximum capacity of the matrix.
//
// \return The capacity of the matrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr size_t StaticMatrix<Type, M, N, SO, AF, PF, Tag>::capacity() noexcept
{
	return M * NN;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the current capacity of the specified row/column.
//
// \param i The index of the row/column.
// \return The current capacity of row/column \a i.
//
// This function returns the current capacity of the specified row/column. In case the
// storage order is set to \a rowMajor the function returns the capacity of row \a i,
// in case the storage flag is set to \a columnMajor the function returns the capacity
// of column \a i.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline size_t StaticMatrix<Type, M, N, SO, AF, PF, Tag>::capacity(size_t i) const noexcept
{
	MAYBE_UNUSED(i);

	METRIC_NUMERIC_USER_ASSERT(i < rows(), "Invalid row access index");

	return NN;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the total number of non-zero elements in the matrix
//
// \return The number of non-zero elements in the matrix.
//
// This function returns the number of non-zero elements in the matrix (i.e. the elements that
// compare unequal to their default value). Note that the number of non-zero elements is always
// less than or equal to the total number of elements in the matrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline size_t StaticMatrix<Type, M, N, SO, AF, PF, Tag>::nonZeros() const
{
	size_t nonzeros(0UL);

	for (size_t i = 0UL; i < M; ++i)
		for (size_t j = 0UL; j < N; ++j)
			if (!isDefault<strict>(v_[i * NN + j]))
				++nonzeros;

	return nonzeros;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the number of non-zero elements in the specified row/column.
//
// \param i The index of the row/column.
// \return The number of non-zero elements of row/column \a i.
//
// This function returns the current number of non-zero elements in the specified row/column
// (i.e. the elements that compare unequal to their default value). In case the storage order
// is set to \a rowMajor the function returns the number of non-zero elements in row \a i, in
// case the storage flag is set to \a columnMajor the function returns the number of non-zero
// elements in column \a i.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline size_t StaticMatrix<Type, M, N, SO, AF, PF, Tag>::nonZeros(size_t i) const
{
	METRIC_NUMERIC_USER_ASSERT(i < rows(), "Invalid row access index");

	const size_t jend(i * NN + N);
	size_t nonzeros(0UL);

	for (size_t j = i * NN; j < jend; ++j)
		if (!isDefault<strict>(v_[j]))
			++nonzeros;

	return nonzeros;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Reset to the default initial values.
//
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::reset()
{
	using mtrc::numeric::clear;

	for (size_t i = 0UL; i < M; ++i)
		for (size_t j = 0UL; j < N; ++j)
			clear(v_[i * NN + j]);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Reset the specified row/column to the default initial values.
//
// \param i The index of the row/column.
// \return void
//
// This function resets the values in the specified row/column to their default value. In case
// the storage order is set to \a rowMajor the function resets the values in row \a i, in case
// the storage order is set to \a columnMajor the function resets the values in column \a i.
// Note that the capacity of the row/column remains unchanged.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::reset(size_t i)
{
	using mtrc::numeric::clear;

	METRIC_NUMERIC_USER_ASSERT(i < rows(), "Invalid row access index");
	for (size_t j = 0UL; j < N; ++j)
		clear(v_[i * NN + j]);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Swapping the contents of two static matrices.
//
// \param m The matrix to be swapped.
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::swap(StaticMatrix &m) noexcept
{
	using std::swap;

	for (size_t i = 0UL; i < M; ++i) {
		for (size_t j = 0UL; j < N; ++j) {
			swap(v_[i * NN + j], m(i, j));
		}
	}
}
//*************************************************************************************************

//=================================================================================================
//
//  NUMERIC FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief In-place transpose of the matrix.
//
// \return Reference to the transposed matrix.
//
// This function transposes the static matrix in-place. Note that this function can only be used
// for square static matrices, i.e. if \a M is equal to N.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag> &StaticMatrix<Type, M, N, SO, AF, PF, Tag>::transpose()
{
	using std::swap;

	METRIC_NUMERIC_STATIC_ASSERT(M == N);

	for (size_t i = 1UL; i < M; ++i)
		for (size_t j = 0UL; j < i; ++j)
			swap(v_[i * NN + j], v_[j * NN + i]);

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Helper function for self-transpose via the trans() function.
//
// \return void
//
// This function assists in the evaluation of self-transpose via the trans() function:

   \code
   mtrc::numeric::StaticMatrix<int,3UL,3UL,mtrc::numeric::rowMajor> A;

   A = trans( A );
   \endcode
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::transpose(TrueType)
{
	transpose();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Helper function for self-transpose via the trans() function.
//
// \return void
//
// This function assists in the evaluation of self-transpose via the trans() function:

   \code
   mtrc::numeric::StaticMatrix<int,3UL,3UL,mtrc::numeric::rowMajor> A;

   A = trans( A );
   \endcode
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::transpose(FalseType)
{
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief In-place conjugate transpose of the matrix.
//
// \return Reference to the transposed matrix.
//
// This function transposes the static matrix in-place. Note that this function can only be used
// for square static matrices, i.e. if \a M is equal to N.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag> &StaticMatrix<Type, M, N, SO, AF, PF, Tag>::ctranspose()
{
	METRIC_NUMERIC_STATIC_ASSERT(M == N);

	for (size_t i = 0UL; i < M; ++i) {
		for (size_t j = 0UL; j < i; ++j) {
			cswap(v_[i * NN + j], v_[j * NN + i]);
		}
		conjugate(v_[i * NN + i]);
	}

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Helper function for self-transpose via the ctrans() function.
//
// \return void
//
// This function assists in the evaluation of self-transpose via the ctrans() function:

   \code
   mtrc::numeric::StaticMatrix<int,3UL,3UL,mtrc::numeric::rowMajor> A;

   A = ctrans( A );
   \endcode
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::ctranspose(TrueType)
{
	ctranspose();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Helper function for self-transpose via the ctrans() function.
//
// \return void
//
// This function assists in the evaluation of self-transpose via the ctrans() function:

   \code
   mtrc::numeric::StaticMatrix<int,3UL,3UL,mtrc::numeric::rowMajor> A;

   A = ctrans( A );
   \endcode
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::ctranspose(FalseType)
{
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scaling of the matrix by the scalar value \a scalar (\f$ A*=s \f$).
//
// \param scalar The scalar value for the matrix scaling.
// \return Reference to the matrix.
//
// This function scales the matrix by applying the given scalar value \a scalar to each element
// of the matrix. For built-in and \c complex data types it has the same effect as using the
// multiplication assignment operator:

   \code
   mtrc::numeric::StaticMatrix<int,2,3> A;
   // ... Initialization
   A *= 4;        // Scaling of the matrix
   A.scale( 4 );  // Same effect as above
   \endcode
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	  // Type tag
template <typename Other> // Data type of the scalar value
inline StaticMatrix<Type, M, N, SO, AF, PF, Tag> &StaticMatrix<Type, M, N, SO, AF, PF, Tag>::scale(const Other &scalar)
{
	for (size_t i = 0UL; i < M; ++i)
		for (size_t j = 0UL; j < N; ++j)
			v_[i * NN + j] *= scalar;

	return *this;
}
//*************************************************************************************************

//=================================================================================================
//
//  MEMORY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Class specific implementation of operator new.
//
// \param size The total number of bytes to be allocated.
// \return Pointer to the newly allocated memory.
// \exception std::bad_alloc Allocation failed.
//
// This class-specific implementation of operator new provides the functionality to allocate
// dynamic memory based on the alignment restrictions of the StaticMatrix class template.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void *StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator new(std::size_t size)
{
	MAYBE_UNUSED(size);

	METRIC_NUMERIC_INTERNAL_ASSERT(size == sizeof(StaticMatrix), "Invalid number of bytes detected");

	return allocate<StaticMatrix>(1UL);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Class specific implementation of operator new[].
//
// \param size The total number of bytes to be allocated.
// \return Pointer to the newly allocated memory.
// \exception std::bad_alloc Allocation failed.
//
// This class-specific implementation of operator new provides the functionality to allocate
// dynamic memory based on the alignment restrictions of the StaticMatrix class template.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void *StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator new[](std::size_t size)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(size >= sizeof(StaticMatrix), "Invalid number of bytes detected");
	METRIC_NUMERIC_INTERNAL_ASSERT(size % sizeof(StaticMatrix) == 0UL, "Invalid number of bytes detected");

	return allocate<StaticMatrix>(size / sizeof(StaticMatrix));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Class specific implementation of the no-throw operator new.
//
// \param size The total number of bytes to be allocated.
// \return Pointer to the newly allocated memory.
// \exception std::bad_alloc Allocation failed.
//
// This class-specific implementation of operator new provides the functionality to allocate
// dynamic memory based on the alignment restrictions of the StaticMatrix class template.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void *StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator new(std::size_t size, const std::nothrow_t &)
{
	MAYBE_UNUSED(size);

	METRIC_NUMERIC_INTERNAL_ASSERT(size == sizeof(StaticMatrix), "Invalid number of bytes detected");

	return allocate<StaticMatrix>(1UL);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Class specific implementation of the no-throw operator new[].
//
// \param size The total number of bytes to be allocated.
// \return Pointer to the newly allocated memory.
// \exception std::bad_alloc Allocation failed.
//
// This class-specific implementation of operator new provides the functionality to allocate
// dynamic memory based on the alignment restrictions of the StaticMatrix class template.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void *StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator new[](std::size_t size, const std::nothrow_t &)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(size >= sizeof(StaticMatrix), "Invalid number of bytes detected");
	METRIC_NUMERIC_INTERNAL_ASSERT(size % sizeof(StaticMatrix) == 0UL, "Invalid number of bytes detected");

	return allocate<StaticMatrix>(size / sizeof(StaticMatrix));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Class specific implementation of operator delete.
//
// \param ptr The memory to be deallocated.
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator delete(void *ptr)
{
	deallocate(static_cast<StaticMatrix *>(ptr));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Class specific implementation of operator delete[].
//
// \param ptr The memory to be deallocated.
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator delete[](void *ptr)
{
	deallocate(static_cast<StaticMatrix *>(ptr));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Class specific implementation of no-throw operator delete.
//
// \param ptr The memory to be deallocated.
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator delete(void *ptr, const std::nothrow_t &)
{
	deallocate(static_cast<StaticMatrix *>(ptr));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Class specific implementation of no-throw operator delete[].
//
// \param ptr The memory to be deallocated.
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::operator delete[](void *ptr, const std::nothrow_t &)
{
	deallocate(static_cast<StaticMatrix *>(ptr));
}
//*************************************************************************************************

//=================================================================================================
//
//  DEBUGGING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns whether the invariants of the static matrix are intact.
//
// \return \a true in case the static matrix's invariants are intact, \a false otherwise.
//
// This function checks whether the invariants of the static matrix are intact, i.e. if its
// state is valid. In case the invariants are intact, the function returns \a true, else it
// will return \a false.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr bool StaticMatrix<Type, M, N, SO, AF, PF, Tag>::isIntact() const noexcept
{
	if (IsNumeric_v<Type>) {
		for (size_t i = 0UL; i < M; ++i) {
			for (size_t j = N; j < NN; ++j) {
				if (!isDefault<strict>(v_[i * NN + j]))
					return false;
			}
		}
	}

	return true;
}
//*************************************************************************************************

//=================================================================================================
//
//  EXPRESSION TEMPLATE EVALUATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns whether the matrix can alias with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this matrix, \a false if not.
//
// This function returns whether the given address can alias with the matrix. In contrast
// to the isAliased() function this function is allowed to use compile time expressions
// to optimize the evaluation.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	  // Type tag
template <typename Other> // Data type of the other expression
inline bool StaticMatrix<Type, M, N, SO, AF, PF, Tag>::canAlias(const Other *alias) const noexcept
{
	return static_cast<const void *>(this) == static_cast<const void *>(alias);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the matrix is aliased with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this matrix, \a false if not.
//
// This function returns whether the given address is aliased with the matrix. In contrast
// to the canAlias() function this function is not allowed to use compile time expressions
// to optimize the evaluation.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	  // Type tag
template <typename Other> // Data type of the other expression
inline bool StaticMatrix<Type, M, N, SO, AF, PF, Tag>::isAliased(const Other *alias) const noexcept
{
	return static_cast<const void *>(this) == static_cast<const void *>(alias);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the matrix is properly aligned in memory.
//
// \return \a true in case the matrix is aligned, \a false if not.
//
// This function returns whether the matrix is guaranteed to be properly aligned in memory, i.e.
// whether the beginning and the end of each row/column of the matrix are guaranteed to conform
// to the alignment restrictions of the element type \a Type.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr bool StaticMatrix<Type, M, N, SO, AF, PF, Tag>::isAligned() noexcept
{
	return AF == aligned;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Load of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \return The loaded SIMD element.
//
// This function performs a load of a specific SIMD element of the dense matrix. The row index
// must be smaller than the number of rows and the column index must be smaller then the number
// of columns. Additionally, the column index (in case of a row-major matrix) or the row index
// (in case of a column-major matrix) must be a multiple of the number of values inside the
// SIMD element. This function must \b NOT be called explicitly! It is used internally for the
// performance optimized evaluation of expression templates. Calling this function explicitly
// might result in erroneous results and/or in compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::SIMDType
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::load(size_t i, size_t j) const noexcept
{
	if (AF == aligned)
		return loada(i, j);
	else
		return loadu(i, j);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Aligned load of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \return The loaded SIMD element.
//
// This function performs an aligned load of a specific SIMD element of the dense matrix.
// The row index must be smaller than the number of rows and the column index must be smaller
// than the number of columns. Additionally, the column index (in case of a row-major matrix)
// or the row index (in case of a column-major matrix) must be a multiple of the number of
// values inside the SIMD element. This function must \b NOT be called explicitly! It is used
// internally for the performance optimized evaluation of expression templates. Calling this
// function explicitly might result in erroneous results and/or in compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::SIMDType
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::loada(size_t i, size_t j) const noexcept
{
	using mtrc::numeric::loada;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j < N, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j + SIMDSIZE <= NN, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(PF == unpadded || j % SIMDSIZE == 0UL, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(checkAlignment(&v_[i * NN + j]), "Invalid alignment detected");

	return loada(&v_[i * NN + j]);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Unaligned load of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \return The loaded SIMD element.
//
// This function performs an unaligned load of a specific SIMD element of the dense matrix.
// The row index must be smaller than the number of rows and the column index must be smaller
// than the number of columns. Additionally, the column index (in case of a row-major matrix)
// or the row index (in case of a column-major matrix) must be a multiple of the number of
// values inside the SIMD element. This function must \b NOT be called explicitly! It is used
// internally for the performance optimized evaluation of expression templates. Calling this
// function explicitly might result in erroneous results and/or in compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE typename StaticMatrix<Type, M, N, SO, AF, PF, Tag>::SIMDType
StaticMatrix<Type, M, N, SO, AF, PF, Tag>::loadu(size_t i, size_t j) const noexcept
{
	using mtrc::numeric::loadu;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j < N, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j + SIMDSIZE <= NN, "Invalid column access index");

	return loadu(&v_[i * NN + j]);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Store of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \param value The SIMD element to be stored.
// \return void
//
// This function performs a store of a specific SIMD element of the dense matrix. The row index
// must be smaller than the number of rows and the column index must be smaller than the number
// of columns. Additionally, the column index (in case of a row-major matrix) or the row index
// (in case of a column-major matrix) must be a multiple of the number of values inside the
// SIMD element. This function must \b NOT be called explicitly! It is used internally for the
// performance optimized evaluation of expression templates. Calling this function explicitly
// might result in erroneous results and/or in compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::store(size_t i, size_t j,
																				   const SIMDType &value) noexcept
{
	if (AF == aligned)
		storea(i, j, value);
	else
		storeu(i, j, value);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Aligned store of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an aligned store of a specific SIMD element of the dense matrix.
// The row index must be smaller than the number of rows and the column index must be smaller
// than the number of columns. Additionally, the column index (in case of a row-major matrix)
// or the row index (in case of a column-major matrix) must be a multiple of the number of
// values inside the SIMD element. This function must \b NOT be called explicitly! It is used
// internally for the performance optimized evaluation of expression templates. Calling this
// function explicitly might result in erroneous results and/or in compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::storea(size_t i, size_t j,
																					const SIMDType &value) noexcept
{
	using mtrc::numeric::storea;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j < N, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j + SIMDSIZE <= NN, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(PF == unpadded || j % SIMDSIZE == 0UL, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(checkAlignment(&v_[i * NN + j]), "Invalid alignment detected");

	storea(&v_[i * NN + j], value);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Unaligned store of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an unaligned store of a specific SIMD element of the dense matrix.
// The row index must be smaller than the number of rows and the column index must be smaller
// than the number of columns. Additionally, the column index (in case of a row-major matrix)
// or the row index (in case of a column-major matrix) must be a multiple of the number of
// values inside the SIMD element. This function must \b NOT be called explicitly! It is used
// internally for the performance optimized evaluation of expression templates. Calling this
// function explicitly might result in erroneous results and/or in compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::storeu(size_t i, size_t j,
																					const SIMDType &value) noexcept
{
	using mtrc::numeric::storeu;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j < N, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j + SIMDSIZE <= NN, "Invalid column access index");

	storeu(&v_[i * NN + j], value);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Aligned, non-temporal store of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an aligned, non-temporal store of a specific SIMD element of the
// dense matrix. The row index must be smaller than the number of rows and the column index
// must be smaller than the number of columns. Additionally, the column index (in case of a
// row-major matrix) or the row index (in case of a column-major matrix) must be a multiple
// of the number of values inside the SIMD element. This function must \b NOT be called
// explicitly! It is used internally for the performance optimized evaluation of expression
// templates. Calling this function explicitly might result in erroneous results and/or in
// compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::stream(size_t i, size_t j,
																					const SIMDType &value) noexcept
{
	using mtrc::numeric::stream;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j < N, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j + SIMDSIZE <= NN, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(PF == unpadded || j % SIMDSIZE == 0UL, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(checkAlignment(&v_[i * NN + j]), "Invalid alignment detected");

	stream(&v_[i * NN + j], value);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO2> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, SO, AF, PF, Tag>::assign(const DenseMatrix<MT, SO2> &rhs)
	-> DisableIf_t<VectorizedAssign_v<MT>>
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t i = 0UL; i < M; ++i) {
		for (size_t j = 0UL; j < N; ++j) {
			v_[i * NN + j] = (*rhs)(i, j);
		}
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SIMD optimized implementation of the assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO2> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, SO, AF, PF, Tag>::assign(const DenseMatrix<MT, SO2> &rhs)
	-> EnableIf_t<VectorizedAssign_v<MT>>
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	constexpr bool remainder(PF == unpadded || !IsPadded_v<MT>);

	constexpr size_t jpos(remainder ? prevMultiple(N, SIMDSIZE) : N);
	METRIC_NUMERIC_INTERNAL_ASSERT(jpos <= N, "Invalid end calculation");

	for (size_t i = 0UL; i < M; ++i) {
		size_t j(0UL);

		for (; j < jpos; j += SIMDSIZE) {
			store(i, j, (*rhs).load(i, j));
		}
		for (; remainder && j < N; ++j) {
			v_[i * NN + j] = (*rhs)(i, j);
		}
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the assignment of a row-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::assign(const SparseMatrix<MT, SO> &rhs)
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t i = 0UL; i < M; ++i)
		for (auto element = (*rhs).begin(i); element != (*rhs).end(i); ++element)
			v_[i * NN + element->index()] = element->value();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the assignment of a column-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::assign(const SparseMatrix<MT, !SO> &rhs)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t j = 0UL; j < N; ++j)
		for (auto element = (*rhs).begin(j); element != (*rhs).end(j); ++element)
			v_[element->index() * NN + j] = element->value();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the addition assignment of a row-major dense matrix.
//
// \param rhs The right-hand side dense matrix to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO2> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, SO, AF, PF, Tag>::addAssign(const DenseMatrix<MT, SO2> &rhs)
	-> DisableIf_t<VectorizedAddAssign_v<MT>>
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t i = 0UL; i < M; ++i) {
		if (IsDiagonal_v<MT>) {
			v_[i * NN + i] += (*rhs)(i, i);
		} else {
			const size_t jbegin((IsUpper_v<MT>) ? (IsStrictlyUpper_v<MT> ? i + 1UL : i) : (0UL));
			const size_t jend((IsLower_v<MT>) ? (IsStrictlyLower_v<MT> ? i : i + 1UL) : (N));
			METRIC_NUMERIC_INTERNAL_ASSERT(jbegin <= jend, "Invalid loop indices detected");

			for (size_t j = jbegin; j < jend; ++j) {
				v_[i * NN + j] += (*rhs)(i, j);
			}
		}
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SIMD optimized implementation of the addition assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO2> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, SO, AF, PF, Tag>::addAssign(const DenseMatrix<MT, SO2> &rhs)
	-> EnableIf_t<VectorizedAddAssign_v<MT>>
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DIAGONAL_MATRIX_TYPE(MT);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	constexpr bool remainder(PF == unpadded || !IsPadded_v<MT>);

	for (size_t i = 0UL; i < M; ++i) {
		const size_t jbegin((IsUpper_v<MT>) ? (prevMultiple((IsStrictlyUpper_v<MT> ? i + 1UL : i), SIMDSIZE)) : (0UL));
		const size_t jend((IsLower_v<MT>) ? (IsStrictlyLower_v<MT> ? i : i + 1UL) : (N));
		METRIC_NUMERIC_INTERNAL_ASSERT(jbegin <= jend, "Invalid loop indices detected");

		const size_t jpos(remainder ? prevMultiple(jend, SIMDSIZE) : jend);
		METRIC_NUMERIC_INTERNAL_ASSERT(jpos <= jend, "Invalid end calculation");

		size_t j(jbegin);

		for (; j < jpos; j += SIMDSIZE) {
			store(i, j, load(i, j) + (*rhs).load(i, j));
		}
		for (; remainder && j < jend; ++j) {
			v_[i * NN + j] += (*rhs)(i, j);
		}
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the addition assignment of a row-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::addAssign(const SparseMatrix<MT, SO> &rhs)
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t i = 0UL; i < M; ++i)
		for (auto element = (*rhs).begin(i); element != (*rhs).end(i); ++element)
			v_[i * NN + element->index()] += element->value();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the addition assignment of a column-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::addAssign(const SparseMatrix<MT, !SO> &rhs)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t j = 0UL; j < N; ++j)
		for (auto element = (*rhs).begin(j); element != (*rhs).end(j); ++element)
			v_[element->index() * NN + j] += element->value();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the subtraction assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO2> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, SO, AF, PF, Tag>::subAssign(const DenseMatrix<MT, SO2> &rhs)
	-> DisableIf_t<VectorizedSubAssign_v<MT>>
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t i = 0UL; i < M; ++i) {
		if (IsDiagonal_v<MT>) {
			v_[i * NN + i] -= (*rhs)(i, i);
		} else {
			const size_t jbegin((IsUpper_v<MT>) ? (IsStrictlyUpper_v<MT> ? i + 1UL : i) : (0UL));
			const size_t jend((IsLower_v<MT>) ? (IsStrictlyLower_v<MT> ? i : i + 1UL) : (N));
			METRIC_NUMERIC_INTERNAL_ASSERT(jbegin <= jend, "Invalid loop indices detected");

			for (size_t j = jbegin; j < jend; ++j) {
				v_[i * NN + j] -= (*rhs)(i, j);
			}
		}
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SIMD optimized implementation of the subtraction assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO2> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, SO, AF, PF, Tag>::subAssign(const DenseMatrix<MT, SO2> &rhs)
	-> EnableIf_t<VectorizedSubAssign_v<MT>>
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DIAGONAL_MATRIX_TYPE(MT);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	constexpr bool remainder(PF == unpadded || !IsPadded_v<MT>);

	for (size_t i = 0UL; i < M; ++i) {
		const size_t jbegin((IsUpper_v<MT>) ? (prevMultiple((IsStrictlyUpper_v<MT> ? i + 1UL : i), SIMDSIZE)) : (0UL));
		const size_t jend((IsLower_v<MT>) ? (IsStrictlyLower_v<MT> ? i : i + 1UL) : (N));
		METRIC_NUMERIC_INTERNAL_ASSERT(jbegin <= jend, "Invalid loop indices detected");

		const size_t jpos(remainder ? prevMultiple(jend, SIMDSIZE) : jend);
		METRIC_NUMERIC_INTERNAL_ASSERT(jpos <= jend, "Invalid end calculation");

		size_t j(jbegin);

		for (; j < jpos; j += SIMDSIZE) {
			store(i, j, load(i, j) - (*rhs).load(i, j));
		}
		for (; remainder && j < jend; ++j) {
			v_[i * NN + j] -= (*rhs)(i, j);
		}
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the subtraction assignment of a row-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::subAssign(const SparseMatrix<MT, SO> &rhs)
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t i = 0UL; i < M; ++i)
		for (auto element = (*rhs).begin(i); element != (*rhs).end(i); ++element)
			v_[i * NN + element->index()] -= element->value();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the subtraction assignment of a column-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::subAssign(const SparseMatrix<MT, !SO> &rhs)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t j = 0UL; j < N; ++j)
		for (auto element = (*rhs).begin(j); element != (*rhs).end(j); ++element)
			v_[element->index() * NN + j] -= element->value();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the Schur product assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix for the Schur product.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO2> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, SO, AF, PF, Tag>::schurAssign(const DenseMatrix<MT, SO2> &rhs)
	-> DisableIf_t<VectorizedSchurAssign_v<MT>>
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t i = 0UL; i < M; ++i) {
		for (size_t j = 0UL; j < N; ++j) {
			v_[i * NN + j] *= (*rhs)(i, j);
		}
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief SIMD optimized implementation of the Schur product assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix for the Schur product.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO2> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, SO, AF, PF, Tag>::schurAssign(const DenseMatrix<MT, SO2> &rhs)
	-> EnableIf_t<VectorizedSchurAssign_v<MT>>
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	constexpr bool remainder(PF == unpadded || !IsPadded_v<MT>);

	constexpr size_t jpos(remainder ? prevMultiple(N, SIMDSIZE) : N);
	METRIC_NUMERIC_INTERNAL_ASSERT(jpos <= N, "Invalid end calculation");

	for (size_t i = 0UL; i < M; ++i) {
		size_t j(0UL);

		for (; j < jpos; j += SIMDSIZE) {
			store(i, j, load(i, j) * (*rhs).load(i, j));
		}
		for (; remainder && j < N; ++j) {
			v_[i * NN + j] *= (*rhs)(i, j);
		}
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the Schur product assignment of a row-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix for the Schur product.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::schurAssign(const SparseMatrix<MT, SO> &rhs)
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	const StaticMatrix tmp(serial(*this));

	reset();

	for (size_t i = 0UL; i < M; ++i)
		for (auto element = (*rhs).begin(i); element != (*rhs).end(i); ++element)
			v_[i * NN + element->index()] = tmp.v_[i * NN + element->index()] * element->value();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Default implementation of the Schur product assignment of a column-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix for the Schur product.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, SO, AF, PF, Tag>::schurAssign(const SparseMatrix<MT, !SO> &rhs)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	const StaticMatrix tmp(serial(*this));

	reset();

	for (size_t j = 0UL; j < N; ++j)
		for (auto element = (*rhs).begin(j); element != (*rhs).end(j); ++element)
			v_[element->index() * NN + j] = tmp.v_[element->index() * NN + j] * element->value();
}
//*************************************************************************************************

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR COLUMN-MAJOR MATRICES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of StaticMatrix for column-major matrices.
// \ingroup static_matrix
//
// This specialization of StaticMatrix adapts the class template to the requirements of
// column-major matrices.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
class StaticMatrix<Type, M, N, true, AF, PF, Tag>
	: public DenseMatrix<StaticMatrix<Type, M, N, true, AF, PF, Tag>, true> {
  public:
	//**Type definitions****************************************************************************
	//! Type of this StaticMatrix instance.
	using This = StaticMatrix<Type, M, N, true, AF, PF, Tag>;

	using BaseType = DenseMatrix<This, true>; //!< Base type of this StaticMatrix instance.
	using ResultType = This;				  //!< Result type for expression template evaluations.

	//! Result type with opposite storage order for expression template evaluations.
	using OppositeType = StaticMatrix<Type, M, N, false, AF, PF, Tag>;

	//! Transpose type for expression template evaluations.
	using TransposeType = StaticMatrix<Type, N, M, false, AF, PF, Tag>;

	using ElementType = Type;				   //!< Type of the matrix elements.
	using SIMDType = SIMDTrait_t<ElementType>; //!< SIMD type of the matrix elements.
	using TagType = Tag;					   //!< Tag type of this StaticMatrix instance.
	using ReturnType = const Type &;		   //!< Return type for expression template evaluations.
	using CompositeType = const This &;		   //!< Data type for composite expression templates.

	using Reference = Type &;			 //!< Reference to a non-constant matrix value.
	using ConstReference = const Type &; //!< Reference to a constant matrix value.
	using Pointer = Type *;				 //!< Pointer to a non-constant matrix value.
	using ConstPointer = const Type *;	 //!< Pointer to a constant matrix value.

	using Iterator = DenseIterator<Type, AF>;			 //!< Iterator over non-constant elements.
	using ConstIterator = DenseIterator<const Type, AF>; //!< Iterator over constant elements.
	//**********************************************************************************************

	//**Rebind struct definition********************************************************************
	/*!\brief Rebind mechanism to obtain a StaticMatrix with different data/element type.
	 */
	template <typename NewType> // Data type of the other matrix
	struct Rebind {
		using Other = StaticMatrix<NewType, M, N, true, AF, PF, Tag>; //!< The type of the other StaticMatrix.
	};
	//**********************************************************************************************

	//**Resize struct definition********************************************************************
	/*!\brief Resize mechanism to obtain a StaticMatrix with different fixed dimensions.
	 */
	template <size_t NewM // Number of rows of the other matrix
			  ,
			  size_t NewN> // Number of columns of the other matrix
	struct Resize {
		using Other = StaticMatrix<Type, NewM, NewN, true, AF, PF, Tag>; //!< The type of the other StaticMatrix.
	};
	//**********************************************************************************************

	//**Compilation flags***************************************************************************
	//! Compilation flag for SIMD optimization.
	/*! The \a simdEnabled compilation flag indicates whether expressions the matrix is involved
		in can be optimized via SIMD operations. In case the element type of the matrix is a
		vectorizable data type, the \a simdEnabled compilation flag is set to \a true, otherwise
		it is set to \a false. */
	static constexpr bool simdEnabled = IsVectorizable_v<Type>;

	//! Compilation flag for SMP assignments.
	/*! The \a smpAssignable compilation flag indicates whether the matrix can be used in SMP
		(shared memory parallel) assignments (both on the left-hand and right-hand side of the
		assignment). */
	static constexpr bool smpAssignable = false;
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline StaticMatrix();
	explicit inline StaticMatrix(const Type &init);
	constexpr StaticMatrix(initializer_list<initializer_list<Type>> list);

	template <typename Other> inline StaticMatrix(size_t m, size_t n, const Other *array);

	template <typename Other, size_t Rows, size_t Cols> inline StaticMatrix(const Other (&array)[Rows][Cols]);

	template <typename Other, size_t Rows, size_t Cols>
	StaticMatrix(const std::array<std::array<Other, Cols>, Rows> &array);

	constexpr StaticMatrix(const StaticMatrix &m);

	template <typename Other, bool SO2, AlignmentFlag AF2, PaddingFlag PF2>
	inline StaticMatrix(const StaticMatrix<Other, M, N, SO2, AF2, PF2, Tag> &m);

	template <typename MT, bool SO> inline StaticMatrix(const Matrix<MT, SO> &m);
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~StaticMatrix() = default;
	//@}
	//**********************************************************************************************

	//**Data access functions***********************************************************************
	/*!\name Data access functions */
	//@{
	constexpr Reference operator()(size_t i, size_t j) noexcept;
	constexpr ConstReference operator()(size_t i, size_t j) const noexcept;
	inline Reference at(size_t i, size_t j);
	inline ConstReference at(size_t i, size_t j) const;
	constexpr Pointer data() noexcept;
	constexpr ConstPointer data() const noexcept;
	constexpr Pointer data(size_t j) noexcept;
	constexpr ConstPointer data(size_t j) const noexcept;
	constexpr Iterator begin(size_t j) noexcept;
	constexpr ConstIterator begin(size_t j) const noexcept;
	constexpr ConstIterator cbegin(size_t j) const noexcept;
	constexpr Iterator end(size_t j) noexcept;
	constexpr ConstIterator end(size_t j) const noexcept;
	constexpr ConstIterator cend(size_t j) const noexcept;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	constexpr StaticMatrix &operator=(const Type &set) &;
	constexpr StaticMatrix &operator=(initializer_list<initializer_list<Type>> list) &;

	template <typename Other, size_t Rows, size_t Cols>
	inline StaticMatrix &operator=(const Other (&array)[Rows][Cols]) &;

	template <typename Other, size_t Rows, size_t Cols>
	inline StaticMatrix &operator=(const std::array<std::array<Other, Cols>, Rows> &array) &;

	constexpr StaticMatrix &operator=(const StaticMatrix &rhs) &;

	template <typename Other, bool SO2, AlignmentFlag AF2, PaddingFlag PF2>
	inline StaticMatrix &operator=(const StaticMatrix<Other, M, N, SO2, AF2, PF2, Tag> &rhs) &;

	template <typename MT, bool SO> inline StaticMatrix &operator=(const Matrix<MT, SO> &rhs) &;
	template <typename MT, bool SO> inline StaticMatrix &operator+=(const Matrix<MT, SO> &rhs) &;
	template <typename MT, bool SO> inline StaticMatrix &operator-=(const Matrix<MT, SO> &rhs) &;
	template <typename MT, bool SO> inline StaticMatrix &operator%=(const Matrix<MT, SO> &rhs) &;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	static constexpr size_t rows() noexcept;
	static constexpr size_t columns() noexcept;
	static constexpr size_t spacing() noexcept;
	static constexpr size_t capacity() noexcept;
	inline size_t capacity(size_t j) const noexcept;
	inline size_t nonZeros() const;
	inline size_t nonZeros(size_t j) const;
	constexpr void reset();
	inline void reset(size_t i);
	inline void swap(StaticMatrix &m) noexcept;
	//@}
	//**********************************************************************************************

	//**Numeric functions***************************************************************************
	/*!\name Numeric functions */
	//@{
	inline StaticMatrix &transpose();
	inline StaticMatrix &ctranspose();

	template <typename Other> inline StaticMatrix &scale(const Other &scalar);
	//@}
	//**********************************************************************************************

	//**Memory functions****************************************************************************
	/*!\name Memory functions */
	//@{
	static inline void *operator new(std::size_t size);
	static inline void *operator new[](std::size_t size);
	static inline void *operator new(std::size_t size, const std::nothrow_t &);
	static inline void *operator new[](std::size_t size, const std::nothrow_t &);

	static inline void operator delete(void *ptr);
	static inline void operator delete[](void *ptr);
	static inline void operator delete(void *ptr, const std::nothrow_t &);
	static inline void operator delete[](void *ptr, const std::nothrow_t &);
	//@}
	//**********************************************************************************************

  private:
	//**********************************************************************************************
	//! The number of elements packed within a single SIMD element.
	static constexpr size_t SIMDSIZE = SIMDTrait<Type>::size;

	//! Alignment adjustment.
	static constexpr size_t MM = (PF == padded ? nextMultiple(M, SIMDSIZE) : M);
	//**********************************************************************************************

	//**********************************************************************************************
	//! Helper variable template for the explicit application of the SFINAE principle.
	template <typename MT>
	static constexpr bool VectorizedAssign_v =
		(useOptimizedKernels && MM >= SIMDSIZE && simdEnabled && MT::simdEnabled &&
		 IsSIMDCombinable_v<Type, ElementType_t<MT>> && IsColumnMajorMatrix_v<MT>);
	//**********************************************************************************************

	//**********************************************************************************************
	//! Helper variable template for the explicit application of the SFINAE principle.
	template <typename MT>
	static constexpr bool VectorizedAddAssign_v =
		(VectorizedAssign_v<MT> && HasSIMDAdd_v<Type, ElementType_t<MT>> && !IsDiagonal_v<MT>);
	//**********************************************************************************************

	//**********************************************************************************************
	//! Helper variable template for the explicit application of the SFINAE principle.
	template <typename MT>
	static constexpr bool VectorizedSubAssign_v =
		(VectorizedAssign_v<MT> && HasSIMDSub_v<Type, ElementType_t<MT>> && !IsDiagonal_v<MT>);
	//**********************************************************************************************

	//**********************************************************************************************
	//! Helper variable template for the explicit application of the SFINAE principle.
	template <typename MT>
	static constexpr bool VectorizedSchurAssign_v = (VectorizedAssign_v<MT> && HasSIMDMult_v<Type, ElementType_t<MT>>);
	//**********************************************************************************************

  public:
	//**Debugging functions*************************************************************************
	/*!\name Debugging functions */
	//@{
	constexpr bool isIntact() const noexcept;
	//@}
	//**********************************************************************************************

	//**Expression template evaluation functions****************************************************
	/*!\name Expression template evaluation functions */
	//@{
	template <typename Other> inline bool canAlias(const Other *alias) const noexcept;
	template <typename Other> inline bool isAliased(const Other *alias) const noexcept;

	static constexpr bool isAligned() noexcept;

	METRIC_NUMERIC_ALWAYS_INLINE SIMDType load(size_t i, size_t j) const noexcept;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDType loada(size_t i, size_t j) const noexcept;
	METRIC_NUMERIC_ALWAYS_INLINE SIMDType loadu(size_t i, size_t j) const noexcept;

	METRIC_NUMERIC_ALWAYS_INLINE void store(size_t i, size_t j, const SIMDType &value) noexcept;
	METRIC_NUMERIC_ALWAYS_INLINE void storea(size_t i, size_t j, const SIMDType &value) noexcept;
	METRIC_NUMERIC_ALWAYS_INLINE void storeu(size_t i, size_t j, const SIMDType &value) noexcept;
	METRIC_NUMERIC_ALWAYS_INLINE void stream(size_t i, size_t j, const SIMDType &value) noexcept;

	template <typename MT, bool SO>
	inline auto assign(const DenseMatrix<MT, SO> &rhs) -> DisableIf_t<VectorizedAssign_v<MT>>;

	template <typename MT, bool SO>
	inline auto assign(const DenseMatrix<MT, SO> &rhs) -> EnableIf_t<VectorizedAssign_v<MT>>;

	template <typename MT> inline void assign(const SparseMatrix<MT, true> &rhs);
	template <typename MT> inline void assign(const SparseMatrix<MT, false> &rhs);

	template <typename MT, bool SO>
	inline auto addAssign(const DenseMatrix<MT, SO> &rhs) -> DisableIf_t<VectorizedAddAssign_v<MT>>;

	template <typename MT, bool SO>
	inline auto addAssign(const DenseMatrix<MT, SO> &rhs) -> EnableIf_t<VectorizedAddAssign_v<MT>>;

	template <typename MT> inline void addAssign(const SparseMatrix<MT, true> &rhs);
	template <typename MT> inline void addAssign(const SparseMatrix<MT, false> &rhs);

	template <typename MT, bool SO>
	inline auto subAssign(const DenseMatrix<MT, SO> &rhs) -> DisableIf_t<VectorizedSubAssign_v<MT>>;

	template <typename MT, bool SO>
	inline auto subAssign(const DenseMatrix<MT, SO> &rhs) -> EnableIf_t<VectorizedSubAssign_v<MT>>;

	template <typename MT> inline void subAssign(const SparseMatrix<MT, true> &rhs);
	template <typename MT> inline void subAssign(const SparseMatrix<MT, false> &rhs);

	template <typename MT, bool SO>
	inline auto schurAssign(const DenseMatrix<MT, SO> &rhs) -> DisableIf_t<VectorizedSchurAssign_v<MT>>;

	template <typename MT, bool SO>
	inline auto schurAssign(const DenseMatrix<MT, SO> &rhs) -> EnableIf_t<VectorizedSchurAssign_v<MT>>;

	template <typename MT> inline void schurAssign(const SparseMatrix<MT, true> &rhs);
	template <typename MT> inline void schurAssign(const SparseMatrix<MT, false> &rhs);
	//@}
	//**********************************************************************************************

  private:
	//**Utility functions***************************************************************************
	inline void transpose(TrueType);
	inline void transpose(FalseType);
	inline void ctranspose(TrueType);
	inline void ctranspose(FalseType);
	//**********************************************************************************************

	//**********************************************************************************************
	//! Alignment of the data elements.
	static constexpr size_t Alignment = (AF == aligned ? AlignmentOf_v<Type> : std::alignment_of<Type>::value);

	//! Type of the aligned storage.
	using AlignedStorage = AlignedArray<Type, MM * N, Alignment>;
	//**********************************************************************************************

	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	AlignedStorage v_; //!< The statically allocated matrix elements.
					   /*!< Access to the matrix elements is gained via the function call operator. */
	//@}
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_POINTER_TYPE(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_REFERENCE_TYPE(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CONST(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VOLATILE(Type);
	METRIC_NUMERIC_STATIC_ASSERT(AF == unaligned || PF == padded || M % SIMDSIZE == 0UL);
	METRIC_NUMERIC_STATIC_ASSERT(PF == unpadded || MM % SIMDSIZE == 0UL);
	METRIC_NUMERIC_STATIC_ASSERT(MM >= M);
	METRIC_NUMERIC_STATIC_ASSERT(IsVectorizable_v<Type> || MM == M);
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  xex
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief The default constructor for StaticMatrix.
//
// All matrix elements are initialized to the default value (i.e. 0 for integral data types).
//
// Note that it is possible to skip the default initialization by means of the
// \a METRIC_NUMERIC_USE_DEFAULT_INITIALIZATION configuration switch. In case the switch is set to 1
// all elements are initialized to their respective default. In case the switch is set to 0 the
// default initialization is skipped and the elements are not initialized. Please note that this
// switch is only effective in case the elements are of fundamental type (i.e. integral or
// floating point). In case the elements are of class type, this switch has no effect. See
// the <tt><metric/numeric/config/Optimizations.h></tt> configuration file for more details.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline StaticMatrix<Type, M, N, true, AF, PF, Tag>::StaticMatrix()
#if METRIC_NUMERIC_USE_DEFAULT_INITIALIZATION
	: v_() // The statically allocated matrix elements
#endif
{
#if !METRIC_NUMERIC_USE_DEFAULT_INITIALIZATION
	using mtrc::numeric::clear;

	if (IsNumeric_v<Type> && PF == padded) {
		for (size_t j = 0UL; j < N; ++j)
			for (size_t i = M; i < MM; ++i)
				clear(v_[i + j * MM]);
	}
#endif

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Constructor for a homogenous initialization of all elements.
//
// \param init Initial value for all matrix elements.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline StaticMatrix<Type, M, N, true, AF, PF, Tag>::StaticMatrix(const Type &init)
// v_ is intentionally left uninitialized
{
	using mtrc::numeric::clear;

	for (size_t j = 0UL; j < N; ++j) {
		for (size_t i = 0UL; i < M; ++i)
			v_[i + j * MM] = init;

		for (size_t i = M; i < MM; ++i)
			clear(v_[i + j * MM]);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief List initialization of all matrix elements.
//
// \param list The initializer list.
// \exception std::invalid_argument Invalid setup of static matrix.
//
// This constructor provides the option to explicitly initialize the elements of the matrix by
// means of an initializer list:

   \code
   using mtrc::numeric::columnMajor;

   mtrc::numeric::StaticMatrix<int,3,3,columnMajor> A{ { 1, 2, 3 },
											   { 4, 5 },
											   { 7, 8, 9 } };
   \endcode

// The matrix elements are (copy) assigned the values of the given initializer list. Missing
// values are initialized as default (as e.g. the value 6 in the example). Note that in case the
// size of the top-level initializer list exceeds the number of rows or the size of any nested
// list exceeds the number of columns, a \a std::invalid_argument exception is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr StaticMatrix<Type, M, N, true, AF, PF, Tag>::StaticMatrix(initializer_list<initializer_list<Type>> list)
	: v_() // The statically allocated matrix elements
{
	if (list.size() != M || determineColumns(list) > N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid setup of static matrix");
	}

	size_t i(0UL);

	for (const auto &rowList : list) {
		size_t j(0UL);
		for (const auto &element : rowList) {
			v_[i + j * MM] = element;
			++j;
		}
		++i;
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Array initialization of all matrix elements.
//
// \param m The number of rows of the matrix.
// \param n The number of columns of the matrix.
// \param array Dynamic array for the initialization.
//
// This constructor offers the option to directly initialize the elements of the matrix with
// a dynamic array:

   \code
   using mtrc::numeric::columnMajor;

   int* array = new int[6];
   // ... Initialization of the dynamic array
   mtrc::numeric::StaticMatrix<int,3,4,columnMajor> v( 2UL, 3UL, array );
   delete[] array;
   \endcode

// The matrix is initialized with the values from the given array. Missing values are initialized
// with default values. In case the specified number of rows and/or columns exceeds the maximum
// number of rows/column of the static matrix (i.e. \m is larger than M or \a n is larger than N),
// a \a std::invalid_argument exception is thrown.\n
// Note that it is expected that the given \a array has at least \a m by \a n elements. Providing
// an array with less elements results in undefined behavior!
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	  // Type tag
template <typename Other> // Data type of the initialization array
inline StaticMatrix<Type, M, N, true, AF, PF, Tag>::StaticMatrix(size_t m, size_t n, const Other *array)
// v_ is intentionally left uninitialized
{
	using mtrc::numeric::clear;

	if (m > M || n > N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid setup of static matrix");
	}

	for (size_t j = 0UL; j < n; ++j) {
		for (size_t i = 0UL; i < m; ++i)
			v_[i + j * MM] = array[i + j * m];

		if (IsNumeric_v<Type>) {
			for (size_t i = m; i < MM; ++i)
				clear(v_[i + j * MM]);
		}
	}

	if (IsNumeric_v<Type>) {
		for (size_t j = n; j < N; ++j) {
			for (size_t i = 0UL; i < MM; ++i)
				clear(v_[i + j * MM]);
		}
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Array initialization of all matrix elements.
//
// \param array Static array for the initialization.
//
// This constructor offers the option to directly initialize the elements of the matrix with
// a static array:

   \code
   using mtrc::numeric::columnMajor;

   const int init[3][3] = { { 1, 2, 3 },
							{ 4, 5 },
							{ 7, 8, 9 } };
   mtrc::numeric::StaticMatrix<int,3,3,columnMajor> A( init );
   \endcode

// The matrix is initialized with the values from the given static array. Missing values are
// initialized with default values (as e.g. the value 6 in the example).
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	 // Type tag
template <typename Other // Data type of the static array
		  ,
		  size_t Rows // Number of rows of the static array
		  ,
		  size_t Cols> // Number of columns of the static array
inline StaticMatrix<Type, M, N, true, AF, PF, Tag>::StaticMatrix(const Other (&array)[Rows][Cols])
// v_ is intentionally left uninitialized
{
	using mtrc::numeric::clear;

	METRIC_NUMERIC_STATIC_ASSERT(Rows == M && Cols == N);

	for (size_t j = 0UL; j < N; ++j) {
		for (size_t i = 0UL; i < M; ++i)
			v_[i + j * MM] = array[i][j];

		for (size_t i = M; i < MM; ++i)
			clear(v_[i + j * MM]);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Initialization of all matrix elements from the given std::array.
//
// \param array The given std::array for the initialization.
//
// This constructor offers the option to directly initialize the elements of the matrix with
// a std::array:

   \code
   using mtrc::numeric::columnMajor;

   const std::array<std::array<int,3>,3> init = { { { 1, 2, 3 },
													{ 4, 5 },
													{ 7, 8, 9 } } };
   mtrc::numeric::StaticMatrix<int,3,3,columnMajor> A( init );
   \endcode

// The matrix is initialized with the values from the given std::array. Missing values are
// initialized with default values (as e.g. the value 6 in the example).
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	 // Type tag
template <typename Other // Data type of the std::array
		  ,
		  size_t Rows // Number of rows of the std::array
		  ,
		  size_t Cols> // Number of columns of the std::array
inline StaticMatrix<Type, M, N, true, AF, PF, Tag>::StaticMatrix(const std::array<std::array<Other, Cols>, Rows> &array)
// v_ is intentionally left uninitialized
{
	using mtrc::numeric::clear;

	METRIC_NUMERIC_STATIC_ASSERT(Rows == M && Cols == N);

	for (size_t j = 0UL; j < N; ++j) {
		for (size_t i = 0UL; i < M; ++i)
			v_[i + j * MM] = array[i][j];

		for (size_t i = M; i < MM; ++i)
			clear(v_[i + j * MM]);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief The copy constructor for StaticMatrix.
//
// \param m Matrix to be copied.
//
// The copy constructor is explicitly defined in order to enable/facilitate NRV optimization.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr StaticMatrix<Type, M, N, true, AF, PF, Tag>::StaticMatrix(const StaticMatrix &m)
	: BaseType() // Initialization of the base class
	  ,
	  v_(m.v_) // The statically allocated matrix elements
{
	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Conversion constructor from different StaticMatrix instances.
//
// \param m Matrix to be copied.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	 // Type tag
template <typename Other // Data type of the other matrix
		  ,
		  bool SO2 // Storage order of the other matrix
		  ,
		  AlignmentFlag AF2 // Alignment flag of the other matrix
		  ,
		  PaddingFlag PF2> // Padding flag of the other matrix
inline StaticMatrix<Type, M, N, true, AF, PF, Tag>::StaticMatrix(const StaticMatrix<Other, M, N, SO2, AF2, PF2, Tag> &m)
// v_ is intentionally left uninitialized
{
	using mtrc::numeric::clear;

	for (size_t j = 0UL; j < N; ++j) {
		for (size_t i = 0UL; i < M; ++i)
			v_[i + j * MM] = m(i, j);

		for (size_t i = M; i < MM; ++i)
			clear(v_[i + j * MM]);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Conversion constructor from different matrices.
//
// \param m Matrix to be copied.
// \exception std::invalid_argument Invalid setup of static matrix.
//
// This constructor initializes the static matrix from the given matrix. In case the size of
// the given matrix does not match the size of the static matrix (i.e. the number of rows is
// not M or the number of columns is not N), a \a std::invalid_argument exception is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the other matrix
		  ,
		  bool SO> // Storage order of the other matrix
inline StaticMatrix<Type, M, N, true, AF, PF, Tag>::StaticMatrix(const Matrix<MT, SO> &m)
// v_ is intentionally left uninitialized
{
	using mtrc::numeric::assign;
	using mtrc::numeric::clear;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TAG(Tag, TagType_t<MT>);

	if ((*m).rows() != M || (*m).columns() != N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid setup of static matrix");
	}

	for (size_t j = 0UL; j < N; ++j) {
		for (size_t i = (IsSparseMatrix_v<MT> ? 0UL : M); i < MM; ++i) {
			clear(v_[i + j * MM]);
		}
	}

	assign(*this, *m);

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DATA ACCESS FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief 2D-access to the matrix elements.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \return Reference to the accessed value.
//
// This function only performs an index check in case METRIC_NUMERIC_USER_ASSERT() is active. In contrast,
// the at() function is guaranteed to perform a check of the given access indices.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::Reference
StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator()(size_t i, size_t j) noexcept
{
	METRIC_NUMERIC_USER_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_USER_ASSERT(j < N, "Invalid column access index");
	return v_[i + j * MM];
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief 2D-access to the matrix elements.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \return Reference-to-const to the accessed value.
//
// This function only performs an index check in case METRIC_NUMERIC_USER_ASSERT() is active. In contrast,
// the at() function is guaranteed to perform a check of the given access indices.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::ConstReference
StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator()(size_t i, size_t j) const noexcept
{
	METRIC_NUMERIC_USER_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_USER_ASSERT(j < N, "Invalid column access index");
	return v_[i + j * MM];
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Checked access to the matrix elements.
//
// \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
// \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
// \return Reference to the accessed value.
// \exception std::out_of_range Invalid matrix access index.
//
// In contrast to the subscript operator this function always performs a check of the given
// access indices.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::Reference
StaticMatrix<Type, M, N, true, AF, PF, Tag>::at(size_t i, size_t j)
{
	if (i >= M) {
		METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid row access index");
	}
	if (j >= N) {
		METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid column access index");
	}
	return (*this)(i, j);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Checked access to the matrix elements.
//
// \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
// \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
// \return Reference to the accessed value.
// \exception std::out_of_range Invalid matrix access index.
//
// In contrast to the subscript operator this function always performs a check of the given
// access indices.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::ConstReference
StaticMatrix<Type, M, N, true, AF, PF, Tag>::at(size_t i, size_t j) const
{
	if (i >= M) {
		METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid row access index");
	}
	if (j >= N) {
		METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid column access index");
	}
	return (*this)(i, j);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Low-level data access to the matrix elements.
//
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage of the static matrix. Note that you
// can NOT assume that all matrix elements lie adjacent to each other! The static matrix may
// use techniques such as padding to improve the alignment of the data. Whereas the number of
// elements within a column are given by the \c columns() member functions, the total number
// of elements including padding is given by the \c spacing() member function.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::Pointer
StaticMatrix<Type, M, N, true, AF, PF, Tag>::data() noexcept
{
	return v_;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Low-level data access to the matrix elements.
//
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage of the static matrix. Note that you
// can NOT assume that all matrix elements lie adjacent to each other! The static matrix may
// use techniques such as padding to improve the alignment of the data. Whereas the number of
// elements within a column are given by the \c columns() member functions, the total number
// of elements including padding is given by the \c spacing() member function.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::ConstPointer
StaticMatrix<Type, M, N, true, AF, PF, Tag>::data() const noexcept
{
	return v_;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Low-level data access to the matrix elements of column \a j.
//
// \param j The column index.
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage for the elements in column \a j.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::Pointer
StaticMatrix<Type, M, N, true, AF, PF, Tag>::data(size_t j) noexcept
{
	METRIC_NUMERIC_USER_ASSERT(j < N, "Invalid dense matrix column access index");
	return v_ + j * MM;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Low-level data access to the matrix elements of column \a j.
//
// \param j The column index.
// \return Pointer to the internal element storage.
//
// This function returns a pointer to the internal storage for the elements in column \a j
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::ConstPointer
StaticMatrix<Type, M, N, true, AF, PF, Tag>::data(size_t j) const noexcept
{
	METRIC_NUMERIC_USER_ASSERT(j < N, "Invalid dense matrix column access index");
	return v_ + j * MM;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns an iterator to the first element of column \a j.
//
// \param j The column index.
// \return Iterator to the first element of column \a j.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::Iterator
StaticMatrix<Type, M, N, true, AF, PF, Tag>::begin(size_t j) noexcept
{
	METRIC_NUMERIC_USER_ASSERT(j < N, "Invalid dense matrix column access index");
	return Iterator(v_ + j * MM);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns an iterator to the first element of column \a j.
//
// \param j The column index.
// \return Iterator to the first element of column \a j.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::ConstIterator
StaticMatrix<Type, M, N, true, AF, PF, Tag>::begin(size_t j) const noexcept
{
	METRIC_NUMERIC_USER_ASSERT(j < N, "Invalid dense matrix column access index");
	return ConstIterator(v_ + j * MM);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns an iterator to the first element of column \a j.
//
// \param j The column index.
// \return Iterator to the first element of column \a j.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::ConstIterator
StaticMatrix<Type, M, N, true, AF, PF, Tag>::cbegin(size_t j) const noexcept
{
	METRIC_NUMERIC_USER_ASSERT(j < N, "Invalid dense matrix column access index");
	return ConstIterator(v_ + j * MM);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns an iterator just past the last element of column \a j.
//
// \param j The column index.
// \return Iterator just past the last element of column \a j.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::Iterator
StaticMatrix<Type, M, N, true, AF, PF, Tag>::end(size_t j) noexcept
{
	METRIC_NUMERIC_USER_ASSERT(j < N, "Invalid dense matrix column access index");
	return Iterator(v_ + j * MM + M);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns an iterator just past the last element of column \a j.
//
// \param j The column index.
// \return Iterator just past the last element of column \a j.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::ConstIterator
StaticMatrix<Type, M, N, true, AF, PF, Tag>::end(size_t j) const noexcept
{
	METRIC_NUMERIC_USER_ASSERT(j < N, "Invalid dense matrix column access index");
	return ConstIterator(v_ + j * MM + M);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns an iterator just past the last element of column \a j.
//
// \param j The column index.
// \return Iterator just past the last element of column \a j.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::ConstIterator
StaticMatrix<Type, M, N, true, AF, PF, Tag>::cend(size_t j) const noexcept
{
	METRIC_NUMERIC_USER_ASSERT(j < N, "Invalid dense matrix column access index");
	return ConstIterator(v_ + j * MM + M);
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ASSIGNMENT OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Homogenous assignment to all matrix elements.
//
// \param set Scalar value to be assigned to all matrix elements.
// \return Reference to the assigned matrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr StaticMatrix<Type, M, N, true, AF, PF, Tag> &
StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator=(const Type &set) &
{
	for (size_t j = 0UL; j < N; ++j)
		for (size_t i = 0UL; i < M; ++i)
			v_[i + j * MM] = set;

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief List assignment to all matrix elements.
//
// \param list The initializer list.
// \exception std::invalid_argument Invalid assignment to static matrix.
//
// This assignment operator offers the option to directly assign to all elements of the matrix
// by means of an initializer list:

   \code
   using mtrc::numeric::columnMajor;

   mtrc::numeric::StaticMatrix<int,3,3,columnMajor> A;
   A = { { 1, 2, 3 },
		 { 4, 5 },
		 { 7, 8, 9 } };
   \endcode

// The matrix elements are (copy) assigned the values from the given initializer list. Missing
// values are initialized as default (as e.g. the value 6 in the example). Note that in case the
// size of the top-level initializer list exceeds the number of rows or the size of any nested
// list exceeds the number of columns, a \a std::invalid_argument exception is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr StaticMatrix<Type, M, N, true, AF, PF, Tag> &
StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator=(initializer_list<initializer_list<Type>> list) &
{
	using mtrc::numeric::clear;

	if (list.size() != M || determineColumns(list) > N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to static matrix");
	}

	size_t i(0UL);

	for (const auto &rowList : list) {
		size_t j(0UL);
		for (const auto &element : rowList) {
			v_[i + j * MM] = element;
			++j;
		}
		for (; j < N; ++j) {
			clear(v_[i + j * MM]);
		}
		++i;
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Array assignment to all matrix elements.
//
// \param array Static array for the assignment.
// \return Reference to the assigned matrix.
//
// This assignment operator offers the option to directly set all elements of the matrix:

   \code
   using mtrc::numeric::columnMajor;

   const int init[3][3] = { { 1, 2, 3 },
							{ 4, 5 },
							{ 7, 8, 9 } };
   mtrc::numeric::StaticMatrix<int,3UL,3UL,columnMajor> A;
   A = init;
   \endcode

// The matrix is assigned the values from the given static array. Missing values are initialized
// with default values (as e.g. the value 6 in the example).
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	 // Type tag
template <typename Other // Data type of the static array
		  ,
		  size_t Rows // Number of rows of the static array
		  ,
		  size_t Cols> // Number of columns of the static array
inline StaticMatrix<Type, M, N, true, AF, PF, Tag> &
StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator=(const Other (&array)[Rows][Cols]) &
{
	METRIC_NUMERIC_STATIC_ASSERT(Rows == M && Cols == N);

	for (size_t j = 0UL; j < N; ++j)
		for (size_t i = 0UL; i < M; ++i)
			v_[i + j * MM] = array[i][j];

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Array assignment to all matrix elements.
//
// \param array The given std::array for the assignment.
// \return Reference to the assigned matrix.
//
// This assignment operator offers the option to directly set all elements of the matrix:

   \code
   using mtrc::numeric::rowMajor;

   const std::array<std::array<int,3UL>,3UL> init = {{ { 1, 2, 3 },
														{ 4, 5 },
														{ 7, 8, 9 } };
   mtrc::numeric::StaticMatrix<int,3UL,3UL,rowMajor> A;
   A = init;
   \endcode

// The matrix is assigned the values from the given std::array. Missing values are initialized
// with default values (as e.g. the value 6 in the example).
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	 // Type tag
template <typename Other // Data type of the static array
		  ,
		  size_t Rows // Number of rows of the static array
		  ,
		  size_t Cols> // Number of columns of the static array
inline StaticMatrix<Type, M, N, true, AF, PF, Tag> &
StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator=(const std::array<std::array<Other, Cols>, Rows> &array) &
{
	METRIC_NUMERIC_STATIC_ASSERT(Rows == M && Cols == N);

	for (size_t j = 0UL; j < N; ++j)
		for (size_t i = 0UL; i < M; ++i)
			v_[i + j * MM] = array[i][j];

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Copy assignment operator for StaticMatrix.
//
// \param rhs Matrix to be copied.
// \return Reference to the assigned matrix.
//
// Explicit definition of a copy assignment operator for performance reasons.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr StaticMatrix<Type, M, N, true, AF, PF, Tag> &
StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator=(const StaticMatrix &rhs) &
{
	v_ = rhs.v_;

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Assignment operator for different StaticMatrix instances.
//
// \param rhs Matrix to be copied.
// \return Reference to the assigned matrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	 // Type tag
template <typename Other // Data type of the other matrix
		  ,
		  bool SO2 // Storage order of the other matrix
		  ,
		  AlignmentFlag AF2 // Alignment flag of the other matrix
		  ,
		  PaddingFlag PF2> // Padding flag of the other matrix
inline StaticMatrix<Type, M, N, true, AF, PF, Tag> &
StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator=(const StaticMatrix<Other, M, N, SO2, AF2, PF2, Tag> &rhs) &
{
	using mtrc::numeric::assign;

	assign(*this, *rhs);

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Assignment operator for different matrices.
//
// \param rhs Matrix to be copied.
// \return Reference to the assigned matrix.
// \exception std::invalid_argument Invalid assignment to static matrix.
//
// This constructor initializes the matrix as a copy of the given matrix. In case the
// number of rows of the given matrix is not M or the number of columns is not N, a
// \a std::invalid_argument exception is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side matrix
		  ,
		  bool SO> // Storage order of the right-hand side matrix
inline StaticMatrix<Type, M, N, true, AF, PF, Tag> &
StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator=(const Matrix<MT, SO> &rhs) &
{
	using mtrc::numeric::assign;

	using TT = decltype(trans(*this));
	using CT = decltype(ctrans(*this));
	using IT = decltype(inv(*this));

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TAG(Tag, TagType_t<MT>);

	if ((*rhs).rows() != M || (*rhs).columns() != N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to static matrix");
	}

	if (IsSame_v<MT, TT> && (*rhs).isAliased(this)) {
		transpose(typename IsSquare<This>::Type());
	} else if (IsSame_v<MT, CT> && (*rhs).isAliased(this)) {
		ctranspose(typename IsSquare<This>::Type());
	} else if (!IsSame_v<MT, IT> && (*rhs).canAlias(this)) {
		StaticMatrix tmp(*rhs);
		assign(*this, tmp);
	} else {
		if (IsSparseMatrix_v<MT>)
			reset();
		assign(*this, *rhs);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Addition assignment operator for the addition of a matrix (\f$ A+=B \f$).
//
// \param rhs The right-hand side matrix to be added to the matrix.
// \return Reference to the matrix.
// \exception std::invalid_argument Matrix sizes do not match.
//
// In case the current sizes of the two matrices don't match, a \a std::invalid_argument exception
// is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side matrix
		  ,
		  bool SO> // Storage order of the right-hand side matrix
inline StaticMatrix<Type, M, N, true, AF, PF, Tag> &
StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator+=(const Matrix<MT, SO> &rhs) &
{
	using mtrc::numeric::addAssign;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TAG(Tag, TagType_t<MT>);

	if ((*rhs).rows() != M || (*rhs).columns() != N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Matrix sizes do not match");
	}

	if ((*rhs).canAlias(this)) {
		const ResultType_t<MT> tmp(*rhs);
		addAssign(*this, tmp);
	} else {
		addAssign(*this, *rhs);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Subtraction assignment operator for the subtraction of a matrix (\f$ A-=B \f$).
//
// \param rhs The right-hand side matrix to be subtracted from the matrix.
// \return Reference to the matrix.
// \exception std::invalid_argument Matrix sizes do not match.
//
// In case the current sizes of the two matrices don't match, a \a std::invalid_argument exception
// is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side matrix
		  ,
		  bool SO> // Storage order of the right-hand side matrix
inline StaticMatrix<Type, M, N, true, AF, PF, Tag> &
StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator-=(const Matrix<MT, SO> &rhs) &
{
	using mtrc::numeric::subAssign;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TAG(Tag, TagType_t<MT>);

	if ((*rhs).rows() != M || (*rhs).columns() != N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Matrix sizes do not match");
	}

	if ((*rhs).canAlias(this)) {
		const ResultType_t<MT> tmp(*rhs);
		subAssign(*this, tmp);
	} else {
		subAssign(*this, *rhs);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Schur product assignment operator for the multiplication of a matrix (\f$ A\circ=B \f$).
//
// \param rhs The right-hand side matrix for the Schur product.
// \return Reference to the matrix.
// \exception std::invalid_argument Matrix sizes do not match.
//
// In case the current sizes of the two matrices don't match, a \a std::invalid_argument exception
// is thrown.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side matrix
		  ,
		  bool SO> // Storage order of the right-hand side matrix
inline StaticMatrix<Type, M, N, true, AF, PF, Tag> &
StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator%=(const Matrix<MT, SO> &rhs) &
{
	using mtrc::numeric::schurAssign;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SAME_TAG(Tag, TagType_t<MT>);

	if ((*rhs).rows() != M || (*rhs).columns() != N) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Matrix sizes do not match");
	}

	if ((*rhs).canAlias(this)) {
		const ResultType_t<MT> tmp(*rhs);
		schurAssign(*this, tmp);
	} else {
		schurAssign(*this, *rhs);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(), "Invariant violation detected");

	return *this;
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the current number of rows of the matrix.
//
// \return The number of rows of the matrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr size_t StaticMatrix<Type, M, N, true, AF, PF, Tag>::rows() noexcept
{
	return M;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the current number of columns of the matrix.
//
// \return The number of columns of the matrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr size_t StaticMatrix<Type, M, N, true, AF, PF, Tag>::columns() noexcept
{
	return N;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the spacing between the beginning of two columns.
//
// \return The spacing between the beginning of two columns.
//
// This function returns the spacing between the beginning of two column, i.e. the total number
// of elements of a column.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr size_t StaticMatrix<Type, M, N, true, AF, PF, Tag>::spacing() noexcept
{
	return MM;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the maximum capacity of the matrix.
//
// \return The capacity of the matrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr size_t StaticMatrix<Type, M, N, true, AF, PF, Tag>::capacity() noexcept
{
	return MM * N;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the current capacity of the specified column.
//
// \param j The index of the column.
// \return The current capacity of column \a j.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline size_t StaticMatrix<Type, M, N, true, AF, PF, Tag>::capacity(size_t j) const noexcept
{
	MAYBE_UNUSED(j);

	METRIC_NUMERIC_USER_ASSERT(j < columns(), "Invalid column access index");

	return MM;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the total number of non-zero elements in the matrix
//
// \return The number of non-zero elements in the dense matrix.
//
// This function returns the number of non-zero elements in the matrix (i.e. the elements that
// compare unequal to their default value). Note that the number of non-zero elements is always
// less than or equal to the total number of elements in the matrix.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline size_t StaticMatrix<Type, M, N, true, AF, PF, Tag>::nonZeros() const
{
	size_t nonzeros(0UL);

	for (size_t j = 0UL; j < N; ++j)
		for (size_t i = 0UL; i < M; ++i)
			if (!isDefault<strict>(v_[i + j * MM]))
				++nonzeros;

	return nonzeros;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the number of non-zero elements in the specified column.
//
// \param j The index of the column.
// \return The number of non-zero elements of column \a j.
//
// This function returns the current number of non-zero elements in the specified column (i.e.
// the elements that compare unequal to their default value).
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline size_t StaticMatrix<Type, M, N, true, AF, PF, Tag>::nonZeros(size_t j) const
{
	METRIC_NUMERIC_USER_ASSERT(j < columns(), "Invalid column access index");

	const size_t iend(j * MM + M);
	size_t nonzeros(0UL);

	for (size_t i = j * MM; i < iend; ++i)
		if (!isDefault<strict>(v_[i]))
			++nonzeros;

	return nonzeros;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Reset to the default initial values.
//
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr void StaticMatrix<Type, M, N, true, AF, PF, Tag>::reset()
{
	using mtrc::numeric::clear;

	for (size_t j = 0UL; j < N; ++j)
		for (size_t i = 0UL; i < M; ++i)
			clear(v_[i + j * MM]);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Reset the specified column to the default initial values.
//
// \param j The index of the column.
// \return void
//
// This function reset the values in the specified column to their default value. Note that
// the capacity of the column remains unchanged.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::reset(size_t j)
{
	using mtrc::numeric::clear;

	METRIC_NUMERIC_USER_ASSERT(j < columns(), "Invalid column access index");
	for (size_t i = 0UL; i < M; ++i)
		clear(v_[i + j * MM]);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Swapping the contents of two static matrices.
//
// \param m The matrix to be swapped.
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::swap(StaticMatrix &m) noexcept
{
	using std::swap;

	for (size_t j = 0UL; j < N; ++j) {
		for (size_t i = 0UL; i < M; ++i) {
			swap(v_[i + j * MM], m(i, j));
		}
	}
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  NUMERIC FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief In-place transpose of the matrix.
//
// \return Reference to the transposed matrix.
//
// This function transposes the static matrix in-place. Note that this function can only be used
// for square static matrices, i.e. if \a M is equal to N.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline StaticMatrix<Type, M, N, true, AF, PF, Tag> &StaticMatrix<Type, M, N, true, AF, PF, Tag>::transpose()
{
	using std::swap;

	METRIC_NUMERIC_STATIC_ASSERT(M == N);

	for (size_t j = 1UL; j < N; ++j)
		for (size_t i = 0UL; i < j; ++i)
			swap(v_[i + j * MM], v_[j + i * MM]);

	return *this;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Helper function for self-transpose via the trans() function.
//
// \return void
//
// This function assists in the evaluation of self-transpose via the trans() function:

   \code
   mtrc::numeric::StaticMatrix<int,3UL,3UL,mtrc::numeric::columnMajor> A;

   A = trans( A );
   \endcode
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::transpose(TrueType)
{
	transpose();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Helper function for self-transpose via the trans() function.
//
// \return void
//
// This function assists in the evaluation of self-transpose via the trans() function:

   \code
   mtrc::numeric::StaticMatrix<int,3UL,3UL,mtrc::numeric::columnMajor> A;

   A = trans( A );
   \endcode
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::transpose(FalseType)
{
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief In-place conjugate transpose of the matrix.
//
// \return Reference to the transposed matrix.
//
// This function transposes the static matrix in-place. Note that this function can only be used
// for square static matrices, i.e. if \a M is equal to N.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline StaticMatrix<Type, M, N, true, AF, PF, Tag> &StaticMatrix<Type, M, N, true, AF, PF, Tag>::ctranspose()
{
	METRIC_NUMERIC_STATIC_ASSERT(M == N);

	for (size_t j = 0UL; j < N; ++j) {
		for (size_t i = 0UL; i < j; ++i) {
			cswap(v_[i + j * MM], v_[j + i * MM]);
		}
		conjugate(v_[j + j * MM]);
	}

	return *this;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Helper function for self-transpose via the ctrans() function.
//
// \return void
//
// This function assists in the evaluation of self-transpose via the ctrans() function:

   \code
   mtrc::numeric::StaticMatrix<int,3UL,3UL,mtrc::numeric::columnMajor> A;

   A = ctrans( A );
   \endcode
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::ctranspose(TrueType)
{
	ctranspose();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Helper function for self-transpose via the ctrans() function.
//
// \return void
//
// This function assists in the evaluation of self-transpose via the ctrans() function:

   \code
   mtrc::numeric::StaticMatrix<int,3UL,3UL,mtrc::numeric::columnMajor> A;

   A = ctrans( A );
   \endcode
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::ctranspose(FalseType)
{
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Scaling of the matrix by the scalar value \a scalar (\f$ A*=s \f$).
//
// \param scalar The scalar value for the matrix scaling.
// \return Reference to the matrix.
//
// This function scales the matrix by applying the given scalar value \a scalar to each element
// of the matrix. For built-in and \c complex data types it has the same effect as using the
// multiplication assignment operator:

   \code
   mtrc::numeric::StaticMatrix<int,2,3> A;
   // ... Initialization
   A *= 4;        // Scaling of the matrix
   A.scale( 4 );  // Same effect as above
   \endcode
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	  // Type tag
template <typename Other> // Data type of the scalar value
inline StaticMatrix<Type, M, N, true, AF, PF, Tag> &
StaticMatrix<Type, M, N, true, AF, PF, Tag>::scale(const Other &scalar)
{
	for (size_t j = 0UL; j < N; ++j)
		for (size_t i = 0UL; i < M; ++i)
			v_[i + j * MM] *= scalar;

	return *this;
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  MEMORY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Class specific implementation of operator new.
//
// \param size The total number of bytes to be allocated.
// \return Pointer to the newly allocated memory.
// \exception std::bad_alloc Allocation failed.
//
// This class-specific implementation of operator new provides the functionality to allocate
// dynamic memory based on the alignment restrictions of the StaticMatrix class template.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void *StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator new(std::size_t size)
{
	MAYBE_UNUSED(size);

	METRIC_NUMERIC_INTERNAL_ASSERT(size == sizeof(StaticMatrix), "Invalid number of bytes detected");

	return allocate<StaticMatrix>(1UL);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Class specific implementation of operator new[].
//
// \param size The total number of bytes to be allocated.
// \return Pointer to the newly allocated memory.
// \exception std::bad_alloc Allocation failed.
//
// This class-specific implementation of operator new provides the functionality to allocate
// dynamic memory based on the alignment restrictions of the StaticMatrix class template.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void *StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator new[](std::size_t size)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(size >= sizeof(StaticMatrix), "Invalid number of bytes detected");
	METRIC_NUMERIC_INTERNAL_ASSERT(size % sizeof(StaticMatrix) == 0UL, "Invalid number of bytes detected");

	return allocate<StaticMatrix>(size / sizeof(StaticMatrix));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Class specific implementation of the no-throw operator new.
//
// \param size The total number of bytes to be allocated.
// \return Pointer to the newly allocated memory.
// \exception std::bad_alloc Allocation failed.
//
// This class-specific implementation of operator new provides the functionality to allocate
// dynamic memory based on the alignment restrictions of the StaticMatrix class template.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void *StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator new(std::size_t size, const std::nothrow_t &)
{
	MAYBE_UNUSED(size);

	METRIC_NUMERIC_INTERNAL_ASSERT(size == sizeof(StaticMatrix), "Invalid number of bytes detected");

	return allocate<StaticMatrix>(1UL);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Class specific implementation of the no-throw operator new[].
//
// \param size The total number of bytes to be allocated.
// \return Pointer to the newly allocated memory.
// \exception std::bad_alloc Allocation failed.
//
// This class-specific implementation of operator new provides the functionality to allocate
// dynamic memory based on the alignment restrictions of the StaticMatrix class template.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void *StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator new[](std::size_t size, const std::nothrow_t &)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(size >= sizeof(StaticMatrix), "Invalid number of bytes detected");
	METRIC_NUMERIC_INTERNAL_ASSERT(size % sizeof(StaticMatrix) == 0UL, "Invalid number of bytes detected");

	return allocate<StaticMatrix>(size / sizeof(StaticMatrix));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Class specific implementation of operator delete.
//
// \param ptr The memory to be deallocated.
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator delete(void *ptr)
{
	deallocate(static_cast<StaticMatrix *>(ptr));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Class specific implementation of operator delete[].
//
// \param ptr The memory to be deallocated.
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator delete[](void *ptr)
{
	deallocate(static_cast<StaticMatrix *>(ptr));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Class specific implementation of no-throw operator delete.
//
// \param ptr The memory to be deallocated.
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator delete(void *ptr, const std::nothrow_t &)
{
	deallocate(static_cast<StaticMatrix *>(ptr));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Class specific implementation of no-throw operator delete[].
//
// \param ptr The memory to be deallocated.
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::operator delete[](void *ptr, const std::nothrow_t &)
{
	deallocate(static_cast<StaticMatrix *>(ptr));
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DEBUGGING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the invariants of the static matrix are intact.
//
// \return \a true in case the static matrix's invariants are intact, \a false otherwise.
//
// This function checks whether the invariants of the static matrix are intact, i.e. if its
// state is valid. In case the invariants are intact, the function returns \a true, else it
// will return \a false.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr bool StaticMatrix<Type, M, N, true, AF, PF, Tag>::isIntact() const noexcept
{
	if (IsNumeric_v<Type>) {
		for (size_t j = 0UL; j < N; ++j) {
			for (size_t i = M; i < MM; ++i) {
				if (!isDefault<strict>(v_[i + j * MM]))
					return false;
			}
		}
	}

	return true;
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  EXPRESSION TEMPLATE EVALUATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the matrix can alias with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this matrix, \a false if not.
//
// This function returns whether the given address can alias with the matrix. In contrast
// to the isAliased() function this function is allowed to use compile time expressions
// to optimize the evaluation.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	  // Type tag
template <typename Other> // Data type of the other expression
inline bool StaticMatrix<Type, M, N, true, AF, PF, Tag>::canAlias(const Other *alias) const noexcept
{
	return static_cast<const void *>(this) == static_cast<const void *>(alias);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the matrix is aliased with the given address \a alias.
//
// \param alias The alias to be checked.
// \return \a true in case the alias corresponds to this matrix, \a false if not.
//
// This function returns whether the given address is aliased with the matrix. In contrast
// to the canAlias() function this function is not allowed to use compile time expressions
// to optimize the evaluation.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag>	  // Type tag
template <typename Other> // Data type of the other expression
inline bool StaticMatrix<Type, M, N, true, AF, PF, Tag>::isAliased(const Other *alias) const noexcept
{
	return static_cast<const void *>(this) == static_cast<const void *>(alias);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the matrix is properly aligned in memory.
//
// \return \a true in case the matrix is aligned, \a false if not.
//
// This function returns whether the matrix is guaranteed to be properly aligned in memory, i.e.
// whether the beginning and the end of each column of the matrix are guaranteed to conform to
// the alignment restrictions of the element type \a Type.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
constexpr bool StaticMatrix<Type, M, N, true, AF, PF, Tag>::isAligned() noexcept
{
	return AF == aligned;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Load of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \return The loaded SIMD element.
//
// This function performs a load of a specific SIMD element of the dense matrix. The row index
// must be smaller than the number of rows and the column index must be smaller than the number
// of columns. Additionally, the row index must be a multiple of the number of values inside
// the SIMD element. This function must \b NOT be called explicitly! It is used internally
// for the performance optimized evaluation of expression templates. Calling this function
// explicitly might result in erroneous results and/or in compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::SIMDType
StaticMatrix<Type, M, N, true, AF, PF, Tag>::load(size_t i, size_t j) const noexcept
{
	if (AF == aligned)
		return loada(i, j);
	else
		return loadu(i, j);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Aligned load of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \return The loaded SIMD element.
//
// This function performs an aligned load of a specific SIMD element of the dense matrix.
// The row index must be smaller than the number of rows and the column index must be smaller
// than the number of columns. Additionally, the row index must be a multiple of the number of
// values inside the SIMD element. This function must \b NOT be called explicitly! It is used
// internally for the performance optimized evaluation of expression templates. Calling this
// function explicitly might result in erroneous results and/or in compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::SIMDType
StaticMatrix<Type, M, N, true, AF, PF, Tag>::loada(size_t i, size_t j) const noexcept
{
	using mtrc::numeric::loada;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(i + SIMDSIZE <= MM, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(PF == unpadded || i % SIMDSIZE == 0UL, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j < N, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(checkAlignment(&v_[i + j * MM]), "Invalid alignment detected");

	return loada(&v_[i + j * MM]);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Unaligned load of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \return The loaded SIMD element.
//
// This function performs an unaligned load of a specific SIMD element of the dense matrix.
// The row index must be smaller than the number of rows and the column index must be smaller
// than the number of columns. Additionally, the row index must be a multiple of the number of
// values inside the SIMD element. This function must \b NOT be called explicitly! It is used
// internally for the performance optimized evaluation of expression templates. Calling this
// function explicitly might result in erroneous results and/or in compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE typename StaticMatrix<Type, M, N, true, AF, PF, Tag>::SIMDType
StaticMatrix<Type, M, N, true, AF, PF, Tag>::loadu(size_t i, size_t j) const noexcept
{
	using mtrc::numeric::loadu;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(i + SIMDSIZE <= MM, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j < N, "Invalid column access index");

	return loadu(&v_[i + j * MM]);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Store of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \param value The SIMD element to be stored.
// \return void
//
// This function performs a store of a specific SIMD element of the dense matrix. The row index
// must be smaller than the number of rows and the column index must be smaller then the number
// of columns. Additionally, the row index must be a multiple of the number of values inside the
// SIMD element. This function must \b NOT be called explicitly! It is used internally for the
// performance optimized evaluation of expression templates. Calling this function explicitly
// might result in erroneous results and/or in compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE void StaticMatrix<Type, M, N, true, AF, PF, Tag>::store(size_t i, size_t j,
																					 const SIMDType &value) noexcept
{
	if (AF == aligned)
		storea(i, j, value);
	else
		storeu(i, j, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Aligned store of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an aligned store of a specific SIMD element of the dense matrix.
// The row index must be smaller than the number of rows and the column index must be smaller
// than the number of columns. Additionally, the row index must be a multiple of the number of
// values inside the SIMD element. This function must \b NOT be called explicitly! It is used
// internally for the performance optimized evaluation of expression templates. Calling this
// function explicitly might result in erroneous results and/or in compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE void StaticMatrix<Type, M, N, true, AF, PF, Tag>::storea(size_t i, size_t j,
																					  const SIMDType &value) noexcept
{
	using mtrc::numeric::storea;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(i + SIMDSIZE <= MM, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(PF == unpadded || i % SIMDSIZE == 0UL, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j < N, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(checkAlignment(&v_[i + j * MM]), "Invalid alignment detected");

	storea(&v_[i + j * MM], value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Unaligned store of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an unaligned store of a specific SIMD element of the dense matrix.
// The row index must be smaller than the number of rows and the column index must be smaller
// than the number of columns. Additionally, the row index must be a multiple of the number of
// values inside the SIMD element. This function must \b NOT be called explicitly! It is used
// internally for the performance optimized evaluation of expression templates. Calling this
// function explicitly might result in erroneous results and/or in compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE void StaticMatrix<Type, M, N, true, AF, PF, Tag>::storeu(size_t i, size_t j,
																					  const SIMDType &value) noexcept
{
	using mtrc::numeric::storeu;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(i + SIMDSIZE <= MM, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j < N, "Invalid column access index");

	storeu(&v_[i + j * MM], value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Aligned, non-temporal store of a SIMD element of the matrix.
//
// \param i Access index for the row. The index has to be in the range [0..M-1].
// \param j Access index for the column. The index has to be in the range [0..N-1].
// \param value The SIMD element to be stored.
// \return void
//
// This function performs an aligned, non-temporal store of a specific SIMD element of the
// dense matrix. The row index must be smaller than the number of rows and the column index
// must be smaller than the number of columns. Additionally, the row index must be a multiple
// of the number of values inside the SIMD element. This function must \b NOT be called
// explicitly! It is used internally for the performance optimized evaluation of expression
// templates. Calling this function explicitly might result in erroneous results and/or in
// compilation errors.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
METRIC_NUMERIC_ALWAYS_INLINE void StaticMatrix<Type, M, N, true, AF, PF, Tag>::stream(size_t i, size_t j,
																					  const SIMDType &value) noexcept
{
	using mtrc::numeric::stream;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT(i < M, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(i + SIMDSIZE <= MM, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(PF == unpadded || i % SIMDSIZE == 0UL, "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(j < N, "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(checkAlignment(&v_[i + j * MM]), "Invalid alignment detected");

	stream(&v_[i + j * MM], value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default implementation of the assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, true, AF, PF, Tag>::assign(const DenseMatrix<MT, SO> &rhs)
	-> DisableIf_t<VectorizedAssign_v<MT>>
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t j = 0UL; j < N; ++j) {
		for (size_t i = 0UL; i < M; ++i) {
			v_[i + j * MM] = (*rhs)(i, j);
		}
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief SIMD optimized implementation of the assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, true, AF, PF, Tag>::assign(const DenseMatrix<MT, SO> &rhs)
	-> EnableIf_t<VectorizedAssign_v<MT>>
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	constexpr bool remainder(PF == unpadded || !IsPadded_v<MT>);

	constexpr size_t ipos(remainder ? prevMultiple(M, SIMDSIZE) : M);
	METRIC_NUMERIC_INTERNAL_ASSERT(ipos <= M, "Invalid end calculation");

	for (size_t j = 0UL; j < N; ++j) {
		size_t i(0UL);

		for (; i < ipos; i += SIMDSIZE) {
			store(i, j, (*rhs).load(i, j));
		}
		for (; remainder && i < M; ++i) {
			v_[i + j * MM] = (*rhs)(i, j);
		}
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default implementation of the assignment of a column-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::assign(const SparseMatrix<MT, true> &rhs)
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t j = 0UL; j < N; ++j)
		for (auto element = (*rhs).begin(j); element != (*rhs).end(j); ++element)
			v_[element->index() + j * MM] = element->value();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default implementation of the assignment of a row-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix to be assigned.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::assign(const SparseMatrix<MT, false> &rhs)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t i = 0UL; i < M; ++i)
		for (auto element = (*rhs).begin(i); element != (*rhs).end(i); ++element)
			v_[i + element->index() * MM] = element->value();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default implementation of the addition assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, true, AF, PF, Tag>::addAssign(const DenseMatrix<MT, SO> &rhs)
	-> DisableIf_t<VectorizedAddAssign_v<MT>>
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t j = 0UL; j < N; ++j) {
		if (IsDiagonal_v<MT>) {
			v_[j + j * MM] += (*rhs)(j, j);
		} else {
			const size_t ibegin((IsLower_v<MT>) ? (IsStrictlyLower_v<MT> ? j + 1UL : j) : (0UL));
			const size_t iend((IsUpper_v<MT>) ? (IsStrictlyUpper_v<MT> ? j : j + 1UL) : (M));
			METRIC_NUMERIC_INTERNAL_ASSERT(ibegin <= iend, "Invalid loop indices detected");

			for (size_t i = ibegin; i < iend; ++i) {
				v_[i + j * MM] += (*rhs)(i, j);
			}
		}
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief SIMD optimized implementation of the addition assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, true, AF, PF, Tag>::addAssign(const DenseMatrix<MT, SO> &rhs)
	-> EnableIf_t<VectorizedAddAssign_v<MT>>
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DIAGONAL_MATRIX_TYPE(MT);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	constexpr bool remainder(PF == unpadded || !IsPadded_v<MT>);

	for (size_t j = 0UL; j < N; ++j) {
		const size_t ibegin((IsLower_v<MT>) ? (prevMultiple((IsStrictlyLower_v<MT> ? j + 1UL : j), SIMDSIZE)) : (0UL));
		const size_t iend((IsUpper_v<MT>) ? (IsStrictlyUpper_v<MT> ? j : j + 1UL) : (M));
		METRIC_NUMERIC_INTERNAL_ASSERT(ibegin <= iend, "Invalid loop indices detected");

		const size_t ipos(remainder ? prevMultiple(iend, SIMDSIZE) : iend);
		METRIC_NUMERIC_INTERNAL_ASSERT(ipos <= iend, "Invalid end calculation");

		size_t i(ibegin);

		for (; i < ipos; i += SIMDSIZE) {
			store(i, j, load(i, j) + (*rhs).load(i, j));
		}
		for (; remainder && i < iend; ++i) {
			v_[i + j * MM] += (*rhs)(i, j);
		}
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default implementation of the addition assignment of a column-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::addAssign(const SparseMatrix<MT, true> &rhs)
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t j = 0UL; j < N; ++j)
		for (auto element = (*rhs).begin(j); element != (*rhs).end(j); ++element)
			v_[element->index() + j * MM] += element->value();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default implementation of the addition assignment of a row-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix to be added.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::addAssign(const SparseMatrix<MT, false> &rhs)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t i = 0UL; i < M; ++i)
		for (auto element = (*rhs).begin(i); element != (*rhs).end(i); ++element)
			v_[i + element->index() * MM] += element->value();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default implementation of the subtraction assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, true, AF, PF, Tag>::subAssign(const DenseMatrix<MT, SO> &rhs)
	-> DisableIf_t<VectorizedSubAssign_v<MT>>
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t j = 0UL; j < N; ++j) {
		if (IsDiagonal_v<MT>) {
			v_[j + j * MM] -= (*rhs)(j, j);
		} else {
			const size_t ibegin((IsLower_v<MT>) ? (IsStrictlyLower_v<MT> ? j + 1UL : j) : (0UL));
			const size_t iend((IsUpper_v<MT>) ? (IsStrictlyUpper_v<MT> ? j : j + 1UL) : (M));
			METRIC_NUMERIC_INTERNAL_ASSERT(ibegin <= iend, "Invalid loop indices detected");

			for (size_t i = ibegin; i < iend; ++i) {
				v_[i + j * MM] -= (*rhs)(i, j);
			}
		}
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief SIMD optimized implementation of the subtraction assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, true, AF, PF, Tag>::subAssign(const DenseMatrix<MT, SO> &rhs)
	-> EnableIf_t<VectorizedSubAssign_v<MT>>
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DIAGONAL_MATRIX_TYPE(MT);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	constexpr bool remainder(PF == unpadded || !IsPadded_v<MT>);

	for (size_t j = 0UL; j < N; ++j) {
		const size_t ibegin((IsLower_v<MT>) ? (prevMultiple((IsStrictlyLower_v<MT> ? j + 1UL : j), SIMDSIZE)) : (0UL));
		const size_t iend((IsUpper_v<MT>) ? (IsStrictlyUpper_v<MT> ? j : j + 1UL) : (M));
		METRIC_NUMERIC_INTERNAL_ASSERT(ibegin <= iend, "Invalid loop indices detected");

		const size_t ipos(remainder ? prevMultiple(iend, SIMDSIZE) : iend);
		METRIC_NUMERIC_INTERNAL_ASSERT(ipos <= iend, "Invalid end calculation");

		size_t i(ibegin);

		for (; i < ipos; i += SIMDSIZE) {
			store(i, j, load(i, j) - (*rhs).load(i, j));
		}
		for (; remainder && i < iend; ++i) {
			v_[i + j * MM] -= (*rhs)(i, j);
		}
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default implementation of the subtraction assignment of a column-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::subAssign(const SparseMatrix<MT, true> &rhs)
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t j = 0UL; j < N; ++j)
		for (auto element = (*rhs).begin(j); element != (*rhs).end(j); ++element)
			v_[element->index() + j * MM] -= element->value();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default implementation of the subtraction assignment of a row-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix to be subtracted.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::subAssign(const SparseMatrix<MT, false> &rhs)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t i = 0UL; i < M; ++i)
		for (auto element = (*rhs).begin(i); element != (*rhs).end(i); ++element)
			v_[i + element->index() * MM] -= element->value();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default implementation of the Schur product assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix for the Schur product.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, true, AF, PF, Tag>::schurAssign(const DenseMatrix<MT, SO> &rhs)
	-> DisableIf_t<VectorizedSchurAssign_v<MT>>
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	for (size_t j = 0UL; j < N; ++j) {
		for (size_t i = 0UL; i < M; ++i) {
			v_[i + j * MM] *= (*rhs)(i, j);
		}
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief SIMD optimized implementation of the Schur product assignment of a dense matrix.
//
// \param rhs The right-hand side dense matrix for the Schur product.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT	// Type of the right-hand side dense matrix
		  ,
		  bool SO> // Storage order of the right-hand side dense matrix
inline auto StaticMatrix<Type, M, N, true, AF, PF, Tag>::schurAssign(const DenseMatrix<MT, SO> &rhs)
	-> EnableIf_t<VectorizedSchurAssign_v<MT>>
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTORIZABLE_TYPE(Type);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	constexpr bool remainder(PF == unpadded || !IsPadded_v<MT>);

	constexpr size_t ipos(remainder ? prevMultiple(M, SIMDSIZE) : M);
	METRIC_NUMERIC_INTERNAL_ASSERT(ipos <= M, "Invalid end calculation");

	for (size_t j = 0UL; j < N; ++j) {
		size_t i(0UL);

		for (; i < ipos; i += SIMDSIZE) {
			store(i, j, load(i, j) * (*rhs).load(i, j));
		}
		for (; remainder && i < M; ++i) {
			v_[i + j * MM] *= (*rhs)(i, j);
		}
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default implementation of the Schur product assignment of a column-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix for the Schur product.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::schurAssign(const SparseMatrix<MT, true> &rhs)
{
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	const StaticMatrix tmp(serial(*this));

	reset();

	for (size_t j = 0UL; j < N; ++j)
		for (auto element = (*rhs).begin(j); element != (*rhs).end(j); ++element)
			v_[element->index() + j * MM] = tmp.v_[element->index() + j * MM] * element->value();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default implementation of the Schur product assignment of a row-major sparse matrix.
//
// \param rhs The right-hand side sparse matrix for the Schur product.
// \return void
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
template <typename MT>	// Type of the right-hand side sparse matrix
inline void StaticMatrix<Type, M, N, true, AF, PF, Tag>::schurAssign(const SparseMatrix<MT, false> &rhs)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() == M && (*rhs).columns() == N, "Invalid matrix size");

	const StaticMatrix tmp(serial(*this));

	reset();

	for (size_t i = 0UL; i < M; ++i)
		for (auto element = (*rhs).begin(i); element != (*rhs).end(i); ++element)
			v_[i + element->index() * MM] = tmp.v_[i + element->index() * MM] * element->value();
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  STATICMATRIX OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\name StaticMatrix operators */
//@{
template <RelaxationFlag RF, typename Type, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
bool isDefault(const StaticMatrix<Type, M, N, SO, AF, PF, Tag> &m);

template <typename Type, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
bool isIntact(const StaticMatrix<Type, M, N, SO, AF, PF, Tag> &m) noexcept;

template <typename Type, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
void swap(StaticMatrix<Type, M, N, SO, AF, PF, Tag> &a, StaticMatrix<Type, M, N, SO, AF, PF, Tag> &b) noexcept;
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the given dynamic matrix is in default state.
// \ingroup dynamic_matrix
//
// \param m The matrix to be tested for its default state.
// \return \a true in case the given matrix's rows and columns are zero, \a false otherwise.
//
// This function checks whether the static matrix is in default (constructed) state. In case it
// is in default state, the function returns \a true, else it will return \a false. The following
// example demonstrates the use of the \a isDefault() function:

   \code
   mtrc::numeric::StaticMatrix<double,3,5> A;
   // ... Initialization
   if( isDefault( A ) ) { ... }
   \endcode

// Optionally, it is possible to switch between strict semantics (mtrc::numeric::strict) and relaxed
// semantics (mtrc::numeric::relaxed):

   \code
   if( isDefault<relaxed>( A ) ) { ... }
   \endcode
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline bool isDefault(const StaticMatrix<Type, M, N, SO, AF, PF, Tag> &m)
{
	if (SO == rowMajor) {
		for (size_t i = 0UL; i < M; ++i)
			for (size_t j = 0UL; j < N; ++j)
				if (!isDefault<RF>(m(i, j)))
					return false;
	} else {
		for (size_t j = 0UL; j < N; ++j)
			for (size_t i = 0UL; i < M; ++i)
				if (!isDefault<RF>(m(i, j)))
					return false;
	}

	return true;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the invariants of the given static matrix are intact.
// \ingroup static_matrix
//
// \param m The static matrix to be tested.
// \return \a true in case the given matrix's invariants are intact, \a false otherwise.
//
// This function checks whether the invariants of the static matrix are intact, i.e. if its
// state is valid. In case the invariants are intact, the function returns \a true, else it
// will return \a false. The following example demonstrates the use of the \a isIntact()
// function:

   \code
   mtrc::numeric::StaticMatrix<double,3,5> A;
   // ... Initialization
   if( isIntact( A ) ) { ... }
   \endcode
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline bool isIntact(const StaticMatrix<Type, M, N, SO, AF, PF, Tag> &m) noexcept
{
	return m.isIntact();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Swapping the contents of two static matrices.
// \ingroup static_matrix
//
// \param a The first matrix to be swapped.
// \param b The second matrix to be swapped.
// \return void
*/
template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO // Storage order
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
inline void swap(StaticMatrix<Type, M, N, SO, AF, PF, Tag> &a, StaticMatrix<Type, M, N, SO, AF, PF, Tag> &b) noexcept
{
	a.swap(b);
}
//*************************************************************************************************

//=================================================================================================
//
//  SIZE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
struct Size<StaticMatrix<T, M, N, SO, AF, PF, Tag>, 0UL> : public Ptrdiff_t<static_cast<ptrdiff_t>(M)> {};

template <typename T, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
struct Size<StaticMatrix<T, M, N, SO, AF, PF, Tag>, 1UL> : public Ptrdiff_t<static_cast<ptrdiff_t>(N)> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  MAXSIZE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
struct MaxSize<StaticMatrix<T, M, N, SO, AF, PF, Tag>, 0UL> : public Ptrdiff_t<static_cast<ptrdiff_t>(M)> {};

template <typename T, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
struct MaxSize<StaticMatrix<T, M, N, SO, AF, PF, Tag>, 1UL> : public Ptrdiff_t<static_cast<ptrdiff_t>(N)> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISSQUARE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
struct IsSquare<StaticMatrix<T, N, N, SO, AF, PF, Tag>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  HASCONSTDATAACCESS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
struct HasConstDataAccess<StaticMatrix<T, M, N, SO, AF, PF, Tag>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  HASMUTABLEDATAACCESS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
struct HasMutableDataAccess<StaticMatrix<T, M, N, SO, AF, PF, Tag>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISALIGNED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
struct IsAligned<StaticMatrix<T, M, N, SO, AF, PF, Tag>> : public BoolConstant<AF == aligned> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISCONTIGUOUS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
struct IsContiguous<StaticMatrix<T, M, N, SO, AF, PF, Tag>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISPADDED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag>
struct IsPadded<StaticMatrix<T, M, N, SO, AF, PF, Tag>> : public BoolConstant<PF == padded> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ADDTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct AddTraitEval2<T1, T2,
					 EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> &&
								(Size_v<T1, 0UL> != DefaultSize_v || Size_v<T2, 0UL> != DefaultSize_v) &&
								(Size_v<T1, 1UL> != DefaultSize_v || Size_v<T2, 1UL> != DefaultSize_v)>> {
	static constexpr bool SO1 = StorageOrder_v<T1>;
	static constexpr bool SO2 = StorageOrder_v<T2>;

	static constexpr bool SO =
		(IsDenseMatrix_v<T1> && IsDenseMatrix_v<T2>
			 ? (IsSymmetric_v<T1> ^ IsSymmetric_v<T2> ? (IsSymmetric_v<T1> ? SO2 : SO1) : SO1 && SO2)
			 : (IsDenseMatrix_v<T1> ? SO1 : SO2));

	using Type = StaticMatrix<AddTrait_t<ElementType_t<T1>, ElementType_t<T2>>, max(Size_v<T1, 0UL>, Size_v<T2, 0UL>),
							  max(Size_v<T1, 1UL>, Size_v<T2, 1UL>), SO, defaultAlignmentFlag, defaultPaddingFlag,
							  AddTrait_t<TagType_t<T1>, TagType_t<T2>>>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SUBTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct SubTraitEval2<T1, T2,
					 EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> &&
								(Size_v<T1, 0UL> != DefaultSize_v || Size_v<T2, 0UL> != DefaultSize_v) &&
								(Size_v<T1, 1UL> != DefaultSize_v || Size_v<T2, 1UL> != DefaultSize_v)>> {
	static constexpr bool SO1 = StorageOrder_v<T1>;
	static constexpr bool SO2 = StorageOrder_v<T2>;

	static constexpr bool SO =
		(IsDenseMatrix_v<T1> && IsDenseMatrix_v<T2>
			 ? (IsSymmetric_v<T1> ^ IsSymmetric_v<T2> ? (IsSymmetric_v<T1> ? SO2 : SO1) : SO1 && SO2)
			 : (IsDenseMatrix_v<T1> ? SO1 : SO2));

	using Type = StaticMatrix<SubTrait_t<ElementType_t<T1>, ElementType_t<T2>>, max(Size_v<T1, 0UL>, Size_v<T2, 0UL>),
							  max(Size_v<T1, 1UL>, Size_v<T2, 1UL>), SO, defaultAlignmentFlag, defaultPaddingFlag,
							  SubTrait_t<TagType_t<T1>, TagType_t<T2>>>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SCHURTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct SchurTraitEval2<T1, T2,
					   EnableIf_t<IsDenseMatrix_v<T1> && IsDenseMatrix_v<T2> &&
								  (Size_v<T1, 0UL> != DefaultSize_v || Size_v<T2, 0UL> != DefaultSize_v) &&
								  (Size_v<T1, 1UL> != DefaultSize_v || Size_v<T2, 1UL> != DefaultSize_v)>> {
	static constexpr bool SO1 = StorageOrder_v<T1>;
	static constexpr bool SO2 = StorageOrder_v<T2>;

	static constexpr bool SO = (IsSymmetric_v<T1> ^ IsSymmetric_v<T2> ? (IsSymmetric_v<T1> ? SO2 : SO1) : SO1 && SO2);

	using Type = StaticMatrix<MultTrait_t<ElementType_t<T1>, ElementType_t<T2>>, max(Size_v<T1, 0UL>, Size_v<T2, 0UL>),
							  max(Size_v<T1, 1UL>, Size_v<T2, 1UL>), SO, defaultAlignmentFlag, defaultPaddingFlag,
							  MultTrait_t<TagType_t<T1>, TagType_t<T2>>>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  MULTTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct MultTraitEval2<T1, T2,
					  EnableIf_t<IsMatrix_v<T1> && IsScalar_v<T2> && (Size_v<T1, 0UL> != DefaultSize_v) &&
								 (Size_v<T1, 1UL> != DefaultSize_v)>> {
	using Type = StaticMatrix<MultTrait_t<ElementType_t<T1>, T2>, Size_v<T1, 0UL>, Size_v<T1, 1UL>, StorageOrder_v<T1>,
							  defaultAlignmentFlag, defaultPaddingFlag, MultTrait_t<TagType_t<T1>, T2>>;
};

template <typename T1, typename T2>
struct MultTraitEval2<T1, T2,
					  EnableIf_t<IsScalar_v<T1> && IsMatrix_v<T2> && (Size_v<T2, 0UL> != DefaultSize_v) &&
								 (Size_v<T2, 1UL> != DefaultSize_v)>> {
	using Type = StaticMatrix<MultTrait_t<T1, ElementType_t<T2>>, Size_v<T2, 0UL>, Size_v<T2, 1UL>, StorageOrder_v<T2>,
							  defaultAlignmentFlag, defaultPaddingFlag, MultTrait_t<T1, TagType_t<T2>>>;
};

template <typename T1, typename T2>
struct MultTraitEval2<T1, T2,
					  EnableIf_t<IsColumnVector_v<T1> && IsRowVector_v<T2> && (Size_v<T1, 0UL> != DefaultSize_v) &&
								 (Size_v<T2, 0UL> != DefaultSize_v)>> {
	using Type =
		StaticMatrix<MultTrait_t<ElementType_t<T1>, ElementType_t<T2>>, Size_v<T1, 0UL>, Size_v<T2, 0UL>, false,
					 defaultAlignmentFlag, defaultPaddingFlag, MultTrait_t<TagType_t<T1>, TagType_t<T2>>>;
};

template <typename T1, typename T2>
struct MultTraitEval2<
	T1, T2,
	EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> &&
			   (Size_v<T1, 0UL> != DefaultSize_v || (IsSquare_v<T1> && Size_v<T2, 0UL> != DefaultSize_v)) &&
			   (Size_v<T2, 1UL> != DefaultSize_v || (IsSquare_v<T2> && Size_v<T1, 1UL> != DefaultSize_v))>> {
	using MultType = MultTrait_t<ElementType_t<T1>, ElementType_t<T2>>;
	using MultTag = MultTrait_t<TagType_t<T1>, TagType_t<T2>>;

	using Type = StaticMatrix<AddTrait_t<MultType, MultType>,
							  (Size_v<T1, 0UL> != DefaultSize_v ? Size_v<T1, 0UL> : Size_v<T2, 0UL>),
							  (Size_v<T2, 1UL> != DefaultSize_v ? Size_v<T2, 1UL> : Size_v<T1, 1UL>),
							  (IsSparseMatrix_v<T1> ? StorageOrder_v<T2> : StorageOrder_v<T1>), defaultAlignmentFlag,
							  defaultPaddingFlag, AddTrait_t<MultTag, MultTag>>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  KRONTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct KronTraitEval2<T1, T2,
					  EnableIf_t<IsDenseMatrix_v<T1> && IsDenseMatrix_v<T2> && (Size_v<T1, 0UL> != DefaultSize_v) &&
								 (Size_v<T2, 0UL> != DefaultSize_v) && (Size_v<T1, 1UL> != DefaultSize_v) &&
								 (Size_v<T2, 1UL> != DefaultSize_v)>> {
	using Type = StaticMatrix<MultTrait_t<ElementType_t<T1>, ElementType_t<T2>>, Size_v<T1, 0UL> * Size_v<T2, 0UL>,
							  Size_v<T1, 1UL> * Size_v<T2, 1UL>, StorageOrder_v<T2>, defaultAlignmentFlag,
							  defaultPaddingFlag, MultTrait_t<TagType_t<T1>, TagType_t<T2>>>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DIVTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct DivTraitEval2<T1, T2,
					 EnableIf_t<IsMatrix_v<T1> && IsScalar_v<T2> && (Size_v<T1, 0UL> != DefaultSize_v) &&
								(Size_v<T1, 1UL> != DefaultSize_v)>> {
	using Type = StaticMatrix<DivTrait_t<ElementType_t<T1>, T2>, Size_v<T1, 0UL>, Size_v<T1, 1UL>, StorageOrder_v<T1>,
							  defaultAlignmentFlag, defaultPaddingFlag, DivTrait_t<TagType_t<T1>, T2>>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  MAPTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T, typename OP>
struct UnaryMapTraitEval2<
	T, OP, EnableIf_t<IsMatrix_v<T> && Size_v<T, 0UL> != DefaultSize_v && Size_v<T, 1UL> != DefaultSize_v>> {
	using ElementType = decltype(std::declval<OP>()(std::declval<ElementType_t<T>>()));

	using Type = StaticMatrix<EvaluateTrait_t<ElementType>, Size_v<T, 0UL>, Size_v<T, 1UL>, StorageOrder_v<T>,
							  defaultAlignmentFlag, defaultPaddingFlag, MapTrait_t<TagType_t<T>, OP>>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2, typename OP>
struct BinaryMapTraitEval2<T1, T2, OP,
						   EnableIf_t<IsColumnVector_v<T1> && IsRowVector_v<T2> && (Size_v<T1, 0UL> != DefaultSize_v) &&
									  (Size_v<T2, 0UL> != DefaultSize_v)>> {
	using ElementType =
		decltype(std::declval<OP>()(std::declval<ElementType_t<T1>>(), std::declval<ElementType_t<T2>>()));

	using Type = StaticMatrix<EvaluateTrait_t<ElementType>, Size_v<T1, 0UL>, Size_v<T2, 0UL>, false,
							  defaultAlignmentFlag, defaultPaddingFlag, MapTrait_t<TagType_t<T1>, TagType_t<T2>, OP>>;
};

template <typename T1, typename T2, typename OP>
struct BinaryMapTraitEval2<T1, T2, OP,
						   EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> &&
									  (Size_v<T1, 0UL> != DefaultSize_v || Size_v<T2, 0UL> != DefaultSize_v) &&
									  (Size_v<T1, 1UL> != DefaultSize_v || Size_v<T2, 1UL> != DefaultSize_v)>> {
	using ElementType =
		decltype(std::declval<OP>()(std::declval<ElementType_t<T1>>(), std::declval<ElementType_t<T2>>()));

	static constexpr bool SO1 = StorageOrder_v<T1>;
	static constexpr bool SO2 = StorageOrder_v<T2>;

	static constexpr bool SO =
		(IsDenseMatrix_v<T1> && IsDenseMatrix_v<T2>
			 ? (IsSymmetric_v<T1> ^ IsSymmetric_v<T2> ? (IsSymmetric_v<T1> ? SO2 : SO1) : SO1 && SO2)
			 : (IsDenseMatrix_v<T1> ? SO1 : SO2));

	using Type = StaticMatrix<EvaluateTrait_t<ElementType>, max(Size_v<T1, 0UL>, Size_v<T2, 0UL>),
							  max(Size_v<T1, 1UL>, Size_v<T2, 1UL>), SO, defaultAlignmentFlag, defaultPaddingFlag,
							  MapTrait_t<TagType_t<T1>, TagType_t<T2>, OP>>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  EXPANDTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T, size_t E>
struct ExpandTraitEval2<T, E, EnableIf_t<IsDenseVector_v<T> && (E != inf) && (Size_v<T, 0UL> != DefaultSize_v)>> {
	using Type =
		StaticMatrix<ElementType_t<T>, (IsColumnVector_v<T> ? Size_v<T, 0UL> : E),
					 (IsColumnVector_v<T> ? E : Size_v<T, 0UL>), (IsColumnVector_v<T> ? columnMajor : rowMajor),
					 defaultAlignmentFlag, defaultPaddingFlag, TagType_t<T>>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  REPEATTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T, size_t R0, size_t R1>
struct RepeatTraitEval2<T, R0, R1, inf,
						EnableIf_t<IsDenseMatrix_v<T> && (R0 != inf) && (R1 != inf) &&
								   (Size_v<T, 0UL> != DefaultSize_v) && (Size_v<T, 1UL> != DefaultSize_v)>> {
	using Type = StaticMatrix<ElementType_t<T>, R0 * Size_v<T, 0UL>, R1 * Size_v<T, 1UL>, StorageOrder_v<T>,
							  defaultAlignmentFlag, defaultPaddingFlag, TagType_t<T>>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SOLVETRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2>
struct SolveTraitEval2<T1, T2,
					   EnableIf_t<IsDenseMatrix_v<T1> && IsDenseMatrix_v<T2> &&
								  ((Size_v<T1, 0UL> != DefaultSize_v) || (Size_v<T1, 1UL> != DefaultSize_v) ||
								   (Size_v<T2, 0UL> != DefaultSize_v)) &&
								  (Size_v<T2, 1UL> != DefaultSize_v)>> {
	using Type =
		StaticMatrix<ElementType_t<T2>, max(Size_v<T1, 0UL>, Size_v<T1, 1UL>, Size_v<T2, 0UL>), Size_v<T2, 1UL>,
					 StorageOrder_v<T2>, defaultAlignmentFlag, defaultPaddingFlag, TagType_t<T2>>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  HIGHTYPE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag, typename T2>
struct HighType<StaticMatrix<T1, M, N, SO, AF, PF, Tag>, StaticMatrix<T2, M, N, SO, AF, PF, Tag>> {
	using Type = StaticMatrix<typename HighType<T1, T2>::Type, M, N, SO, AF, PF, Tag>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  LOWTYPE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, size_t M, size_t N, bool SO, AlignmentFlag AF, PaddingFlag PF, typename Tag, typename T2>
struct LowType<StaticMatrix<T1, M, N, SO, AF, PF, Tag>, StaticMatrix<T2, M, N, SO, AF, PF, Tag>> {
	using Type = StaticMatrix<typename LowType<T1, T2>::Type, M, N, SO, AF, PF, Tag>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SUBMATRIXTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, size_t I, size_t J, size_t M, size_t N>
struct SubmatrixTraitEval2<MT, I, J, M, N,
						   EnableIf_t<I != inf && J != inf && M != inf && N != inf && IsDenseMatrix_v<MT>>> {
	using Type = StaticMatrix<RemoveConst_t<ElementType_t<MT>>, M, N, StorageOrder_v<MT>, defaultAlignmentFlag,
							  defaultPaddingFlag, TagType_t<MT>>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ROWSTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, size_t M>
struct RowsTraitEval2<MT, M, EnableIf_t<M != 0UL && IsDenseMatrix_v<MT> && Size_v<MT, 1UL> != DefaultSize_v>> {
	using Type = StaticMatrix<RemoveConst_t<ElementType_t<MT>>, M, Size_v<MT, 1UL>, false, defaultAlignmentFlag,
							  defaultPaddingFlag, TagType_t<MT>>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  COLUMNSTRAIT SPECIALIZATIONS
//
//=================================================================================================

//***********************z**************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, size_t N>
struct ColumnsTraitEval2<MT, N, EnableIf_t<N != 0UL && IsDenseMatrix_v<MT> && Size_v<MT, 0UL> != DefaultSize_v>> {
	using Type = StaticMatrix<RemoveConst_t<ElementType_t<MT>>, Size_v<MT, 0UL>, N, true, defaultAlignmentFlag,
							  defaultPaddingFlag, TagType_t<MT>>;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
