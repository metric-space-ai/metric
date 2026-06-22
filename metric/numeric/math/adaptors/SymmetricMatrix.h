// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ADAPTORS_SYMMETRICMATRIX_H
#define METRIC_NUMERIC_MATH_ADAPTORS_SYMMETRICMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/Forward.h>
#include <metric/numeric/math/InversionFlag.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/adaptors/symmetricmatrix/BaseTemplate.h>
#include <metric/numeric/math/adaptors/symmetricmatrix/DenseNonScalar.h>
#include <metric/numeric/math/adaptors/symmetricmatrix/DenseScalar.h>
#include <metric/numeric/math/adaptors/symmetricmatrix/SparseNonScalar.h>
#include <metric/numeric/math/adaptors/symmetricmatrix/SparseScalar.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/RequiresEvaluation.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/IsDivisor.h>
#include <metric/numeric/math/traits/AddTrait.h>
#include <metric/numeric/math/traits/DeclDiagTrait.h>
#include <metric/numeric/math/traits/DeclHermTrait.h>
#include <metric/numeric/math/traits/DeclLowTrait.h>
#include <metric/numeric/math/traits/DeclStrLowTrait.h>
#include <metric/numeric/math/traits/DeclStrUppTrait.h>
#include <metric/numeric/math/traits/DeclSymTrait.h>
#include <metric/numeric/math/traits/DeclUniLowTrait.h>
#include <metric/numeric/math/traits/DeclUniUppTrait.h>
#include <metric/numeric/math/traits/DeclUppTrait.h>
#include <metric/numeric/math/traits/DivTrait.h>
#include <metric/numeric/math/traits/KronTrait.h>
#include <metric/numeric/math/traits/MapTrait.h>
#include <metric/numeric/math/traits/MultTrait.h>
#include <metric/numeric/math/traits/RepeatTrait.h>
#include <metric/numeric/math/traits/SchurTrait.h>
#include <metric/numeric/math/traits/SubTrait.h>
#include <metric/numeric/math/traits/SubmatrixTrait.h>
#include <metric/numeric/math/typetraits/HasConstDataAccess.h>
#include <metric/numeric/math/typetraits/HighType.h>
#include <metric/numeric/math/typetraits/IsAdaptor.h>
#include <metric/numeric/math/typetraits/IsAligned.h>
#include <metric/numeric/math/typetraits/IsContiguous.h>
#include <metric/numeric/math/typetraits/IsDiagonal.h>
#include <metric/numeric/math/typetraits/IsHermitian.h>
#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsPadded.h>
#include <metric/numeric/math/typetraits/IsRestricted.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/math/typetraits/IsSquare.h>
#include <metric/numeric/math/typetraits/IsStrictlyLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/IsSymmetric.h>
#include <metric/numeric/math/typetraits/IsUniform.h>
#include <metric/numeric/math/typetraits/IsZero.h>
#include <metric/numeric/math/typetraits/LowType.h>
#include <metric/numeric/math/typetraits/MaxSize.h>
#include <metric/numeric/math/typetraits/RemoveAdaptor.h>
#include <metric/numeric/math/typetraits/Size.h>
#include <metric/numeric/math/typetraits/StorageOrder.h>
#include <metric/numeric/math/typetraits/YieldsDiagonal.h>
#include <metric/numeric/math/typetraits/YieldsHermitian.h>
#include <metric/numeric/math/typetraits/YieldsIdentity.h>
#include <metric/numeric/math/typetraits/YieldsSymmetric.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/algorithms/Min.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  SYMMETRICMATRIX OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\name SymmetricMatrix operators */
//@{
template <RelaxationFlag RF, typename MT, bool SO, bool DF, bool SF>
bool isDefault(const SymmetricMatrix<MT, SO, DF, SF> &m);

template <typename MT, bool SO, bool DF, bool SF> bool isIntact(const SymmetricMatrix<MT, SO, DF, SF> &m);

template <typename MT, bool SO, bool DF, bool SF>
void swap(SymmetricMatrix<MT, SO, DF, SF> &a, SymmetricMatrix<MT, SO, DF, SF> &b) noexcept;
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the given symmetric matrix is in default state.
// \ingroup symmetric_matrix
//
// \param m The symmetric matrix to be tested for its default state.
// \return \a true in case the given matrix is component-wise zero, \a false otherwise.
//
// This function checks whether the matrix is in default state. For instance, in case the
// matrix is instantiated for a built-in integral or floating point data type, the function
// returns \a true in case all matrix elements are 0 and \a false in case any matrix element
// is not 0. The following example demonstrates the use of the \a isDefault function:

   \code
   mtrc::numeric::SymmetricMatrix<int> A;
   // ... Resizing and initialization
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
		  typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  bool SF> // Scalar flag
inline bool isDefault(const SymmetricMatrix<MT, SO, DF, SF> &m)
{
	if (Size_v<MT, 0UL> == DefaultSize_v)
		return m.rows() == 0UL;
	else
		return isStrictlyLower<RF>(m);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the invariants of the given symmetric matrix are intact.
// \ingroup symmetric_matrix
//
// \param m The symmetric matrix to be tested.
// \return \a true in case the given matrix's invariants are intact, \a false otherwise.
//
// This function checks whether the invariants of the symmetric matrix are intact, i.e. if its
// state is valid. In case the invariants are intact, the function returns \a true, else it
// will return \a false. The following example demonstrates the use of the \a isIntact()
// function:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::SymmetricMatrix;

   SymmetricMatrix< DynamicMatrix<int> > A;
   // ... Resizing and initialization
   if( isIntact( A ) ) { ... }
   \endcode
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  bool SF> // Scalar flag
inline bool isIntact(const SymmetricMatrix<MT, SO, DF, SF> &m)
{
	return m.isIntact();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Swapping the contents of two matrices.
// \ingroup symmetric_matrix
//
// \param a The first matrix to be swapped.
// \param b The second matrix to be swapped.
// \return void
*/
template <typename MT // Type of the adapted matrix
		  ,
		  bool SO // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  bool SF> // Scalar flag
inline void swap(SymmetricMatrix<MT, SO, DF, SF> &a, SymmetricMatrix<MT, SO, DF, SF> &b) noexcept
{
	a.swap(b);
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief In-place inversion of the given symmetric dense matrix.
// \ingroup symmetric_matrix
//
// \param m The symmetric dense matrix to be inverted.
// \return void
// \exception std::invalid_argument Inversion of singular matrix failed.
//
// This function inverts the given symmetric dense matrix by means of the specified matrix
// inversion algorithm \c IF. The The inversion fails if the given matrix is singular and not
// invertible. In this case a \a std::invalid_argument exception is thrown.
//
// \note The matrix inversion can only be used for dense matrices with \c float, \c double,
// \c complex<float> or \c complex<double> element type. The attempt to call the function with
// matrices of any other element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a linker error will be created.
//
// \note This function does only provide the basic exception safety guarantee, i.e. in case of an
// exception \a m may already have been modified.
*/
template <InversionFlag IF // Inversion algorithm
		  ,
		  typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order of the dense matrix
inline void invert(SymmetricMatrix<MT, SO, true, true> &m)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	if (IF == asUniLower || IF == asUniUpper) {
		METRIC_NUMERIC_INTERNAL_ASSERT(isIdentity(m), "Violation of preconditions detected");
		return;
	}

	constexpr InversionFlag flag(
		(IF == byLU || IF == byLDLT || IF == byLDLH || IF == asGeneral || IF == asSymmetric || IF == asHermitian)
			? (byLDLT)
			: ((IF == byLLH) ? (byLLH) : (asDiagonal)));

	MT tmp(m.matrix_);
	invert<flag>(tmp);
	m.matrix_ = std::move(tmp);

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(m), "Broken invariant detected");
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a matrix to a symmetric matrix.
// \ingroup symmetric_matrix
//
// \param lhs The target left-hand side symmetric matrix.
// \param rhs The right-hand side matrix to be assigned.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the adapted matrix
		  ,
		  bool SO1 // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  bool SF // Scalar flag
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline bool tryAssign(const SymmetricMatrix<MT1, SO1, DF, SF> &lhs, const Matrix<MT2, SO2> &rhs, size_t row,
					  size_t column)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(MT2);

	METRIC_NUMERIC_INTERNAL_ASSERT(row <= lhs.rows(), "Invalid row access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(column <= lhs.columns(), "Invalid column access index");
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).rows() <= lhs.rows() - row, "Invalid number of rows");
	METRIC_NUMERIC_INTERNAL_ASSERT((*rhs).columns() <= lhs.columns() - column, "Invalid number of columns");

	MAYBE_UNUSED(lhs);

	const size_t M((*rhs).rows());
	const size_t N((*rhs).columns());

	if ((row + M <= column) || (column + N <= row))
		return true;

	const bool lower(row > column);
	const size_t size(min(row + M, column + N) - (lower ? row : column));

	if (size < 2UL)
		return true;

	const size_t subrow(lower ? 0UL : column - row);
	const size_t subcol(lower ? row - column : 0UL);

	return isSymmetric(submatrix(*rhs, subrow, subcol, size, size));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the addition assignment of a matrix to a symmetric matrix.
// \ingroup symmetric_matrix
//
// \param lhs The target left-hand side symmetric matrix.
// \param rhs The right-hand side matrix to be added.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the adapted matrix
		  ,
		  bool SO1 // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  bool SF // Scalar flag
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline bool tryAddAssign(const SymmetricMatrix<MT1, SO1, DF, SF> &lhs, const Matrix<MT2, SO2> &rhs, size_t row,
						 size_t column)
{
	return tryAssign(lhs, *rhs, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the subtraction assignment of a matrix to a symmetric
//        matrix.
// \ingroup symmetric_matrix
//
// \param lhs The target left-hand side symmetric matrix.
// \param rhs The right-hand side matrix to be subtracted.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the adapted matrix
		  ,
		  bool SO1 // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  bool SF // Scalar flag
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline bool trySubAssign(const SymmetricMatrix<MT1, SO1, DF, SF> &lhs, const Matrix<MT2, SO2> &rhs, size_t row,
						 size_t column)
{
	return tryAssign(lhs, *rhs, row, column);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the Schur product assignment of a matrix to a symmetric
//        matrix.
// \ingroup symmetric_matrix
//
// \param lhs The target left-hand side symmetric matrix.
// \param rhs The right-hand side matrix for the Schur product.
// \param row The row index of the first element to be modified.
// \param column The column index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename MT1 // Type of the adapted matrix
		  ,
		  bool SO1 // Storage order of the adapted matrix
		  ,
		  bool DF // Density flag
		  ,
		  bool SF // Scalar flag
		  ,
		  typename MT2 // Type of the right-hand side matrix
		  ,
		  bool SO2> // Storage order of the right-hand side matrix
inline bool trySchurAssign(const SymmetricMatrix<MT1, SO1, DF, SF> &lhs, const Matrix<MT2, SO2> &rhs, size_t row,
						   size_t column)
{
	return tryAssign(lhs, *rhs, row, column);
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SIZE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF>
struct Size<SymmetricMatrix<MT, SO, DF, SF>, 0UL> : public Size<MT, 0UL> {};

template <typename MT, bool SO, bool DF, bool SF>
struct Size<SymmetricMatrix<MT, SO, DF, SF>, 1UL> : public Size<MT, 1UL> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  MAXSIZE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF>
struct MaxSize<SymmetricMatrix<MT, SO, DF, SF>, 0UL> : public MaxSize<MT, 0UL> {};

template <typename MT, bool SO, bool DF, bool SF>
struct MaxSize<SymmetricMatrix<MT, SO, DF, SF>, 1UL> : public MaxSize<MT, 1UL> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISSQUARE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF> struct IsSquare<SymmetricMatrix<MT, SO, DF, SF>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISUNIFORM SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF>
struct IsUniform<SymmetricMatrix<MT, SO, DF, SF>> : public IsUniform<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF>
struct IsSymmetric<SymmetricMatrix<MT, SO, DF, SF>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISHERMITIAN SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF>
struct IsHermitian<SymmetricMatrix<MT, SO, DF, SF>> : public IsBuiltin<ElementType_t<MT>> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISSTRICTLYLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF>
struct IsStrictlyLower<SymmetricMatrix<MT, SO, DF, SF>> : public IsZero<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISSTRICTLYUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF>
struct IsStrictlyUpper<SymmetricMatrix<MT, SO, DF, SF>> : public IsZero<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISADAPTOR SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF>
struct IsAdaptor<SymmetricMatrix<MT, SO, DF, SF>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISRESTRICTED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF>
struct IsRestricted<SymmetricMatrix<MT, SO, DF, SF>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  HASCONSTDATAACCESS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool SF>
struct HasConstDataAccess<SymmetricMatrix<MT, SO, true, SF>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISALIGNED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF>
struct IsAligned<SymmetricMatrix<MT, SO, DF, SF>> : public IsAligned<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISCONTIGUOUS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF>
struct IsContiguous<SymmetricMatrix<MT, SO, DF, SF>> : public IsContiguous<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISPADDED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF>
struct IsPadded<SymmetricMatrix<MT, SO, DF, SF>> : public IsPadded<MT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  REMOVEADAPTOR SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF> struct RemoveAdaptor<SymmetricMatrix<MT, SO, DF, SF>> {
	using Type = MT;
};
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
struct AddTraitEval1<T1, T2,
					 EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> &&
								((IsSymmetric_v<T1> && IsSymmetric_v<T2>) || (IsSymmetric_v<T1> && IsDiagonal_v<T2>) ||
								 (IsDiagonal_v<T1> && IsSymmetric_v<T2>)) &&
								!(IsDiagonal_v<T1> && IsDiagonal_v<T2>) && !(IsUniform_v<T1> && IsUniform_v<T2>) &&
								!(IsZero_v<T1> || IsZero_v<T2>)>> {
	using Type = SymmetricMatrix<typename AddTraitEval2<T1, T2>::Type>;
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
struct SubTraitEval1<T1, T2,
					 EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> &&
								((IsSymmetric_v<T1> && IsSymmetric_v<T2>) || (IsSymmetric_v<T1> && IsDiagonal_v<T2>) ||
								 (IsDiagonal_v<T1> && IsSymmetric_v<T2>)) &&
								!(IsDiagonal_v<T1> && IsDiagonal_v<T2>) && !(IsUniform_v<T1> && IsUniform_v<T2>) &&
								!(IsZero_v<T1> || IsZero_v<T2>)>> {
	using Type = SymmetricMatrix<typename SubTraitEval2<T1, T2>::Type>;
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
struct SchurTraitEval1<T1, T2,
					   EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> && (IsSymmetric_v<T1> && IsSymmetric_v<T2>) &&
								  !(IsDiagonal_v<T1> || IsDiagonal_v<T2>) && !(IsUniform_v<T1> && IsUniform_v<T2>) &&
								  !(IsZero_v<T1> || IsZero_v<T2>)>> {
	using Type = SymmetricMatrix<typename SchurTraitEval2<T1, T2>::Type>;
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
struct MultTraitEval1<
	T1, T2,
	EnableIf_t<IsMatrix_v<T1> && IsScalar_v<T2> && (IsSymmetric_v<T1> && !IsDiagonal_v<T1> && !IsUniform_v<T1>)>> {
	using Type = SymmetricMatrix<typename MultTraitEval2<T1, T2>::Type>;
};

template <typename T1, typename T2>
struct MultTraitEval1<
	T1, T2,
	EnableIf_t<IsScalar_v<T1> && IsMatrix_v<T2> && (IsSymmetric_v<T2> && !IsDiagonal_v<T2> && !IsUniform_v<T2>)>> {
	using Type = SymmetricMatrix<typename MultTraitEval2<T1, T2>::Type>;
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
struct KronTraitEval1<T1, T2,
					  EnableIf_t<IsMatrix_v<T1> && IsMatrix_v<T2> && (IsSymmetric_v<T1> && IsSymmetric_v<T2>) &&
								 !(IsDiagonal_v<T1> && IsDiagonal_v<T2>) && !(IsUniform_v<T1> && IsUniform_v<T2>) &&
								 !(IsZero_v<T1> || IsZero_v<T2>)>> {
	using Type = SymmetricMatrix<typename KronTraitEval2<T1, T2>::Type>;
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
struct DivTraitEval1<T1, T2, EnableIf_t<IsSymmetric_v<T1> && !IsDiagonal_v<T1> && IsScalar_v<T2>>> {
	using Type = SymmetricMatrix<typename DivTraitEval2<T1, T2>::Type>;
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
struct UnaryMapTraitEval1<T, OP,
						  EnableIf_t<YieldsSymmetric_v<OP, T> && !YieldsHermitian_v<OP, T> &&
									 !YieldsDiagonal_v<OP, T> && !YieldsIdentity_v<OP, T>>> {
	using Type = SymmetricMatrix<typename UnaryMapTraitEval2<T, OP>::Type, StorageOrder_v<T>>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T1, typename T2, typename OP>
struct BinaryMapTraitEval1<T1, T2, OP,
						   EnableIf_t<YieldsSymmetric_v<OP, T1, T2> && !YieldsHermitian_v<OP, T1, T2> &&
									  !YieldsDiagonal_v<OP, T1, T2> && !YieldsIdentity_v<OP, T1, T2>>> {
	using Type = SymmetricMatrix<typename BinaryMapTraitEval2<T1, T2, OP>::Type>;
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
struct RepeatTraitEval1<T, R0, R1, inf, EnableIf_t<R0 != inf && R1 != inf && R0 == R1 && IsSymmetric_v<T>>> {
	using Type = SymmetricMatrix<typename RepeatTraitEval2<T, R0, R1, inf>::Type>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLSYMTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF> struct DeclSymTrait<SymmetricMatrix<MT, SO, DF, SF>> {
	using Type = SymmetricMatrix<MT, SO, DF, SF>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLHERMTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF> struct DeclHermTrait<SymmetricMatrix<MT, SO, DF, SF>> {
	using Type = HermitianMatrix<MT, SO, DF>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLLOWTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF> struct DeclLowTrait<SymmetricMatrix<MT, SO, DF, SF>> {
	using Type = DiagonalMatrix<MT, SO, DF>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLUNILOWTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF> struct DeclUniLowTrait<SymmetricMatrix<MT, SO, DF, SF>> {
	using Type = IdentityMatrix<ElementType_t<MT>, SO>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLSTRLOWTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF> struct DeclStrLowTrait<SymmetricMatrix<MT, SO, DF, SF>> {
	using Type = ZeroMatrix<ElementType_t<MT>, SO>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLUPPTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF> struct DeclUppTrait<SymmetricMatrix<MT, SO, DF, SF>> {
	using Type = DiagonalMatrix<MT, SO, DF>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLUNIUPPTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF> struct DeclUniUppTrait<SymmetricMatrix<MT, SO, DF, SF>> {
	using Type = IdentityMatrix<ElementType_t<MT>, SO>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLSTRUPPTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF> struct DeclStrUppTrait<SymmetricMatrix<MT, SO, DF, SF>> {
	using Type = ZeroMatrix<ElementType_t<MT>, SO>;
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  DECLDIAGTRAIT SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT, bool SO, bool DF, bool SF> struct DeclDiagTrait<SymmetricMatrix<MT, SO, DF, SF>> {
	using Type = DiagonalMatrix<MT, SO, DF>;
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
template <typename MT1, bool SO1, bool DF1, bool SF1, typename MT2, bool SO2, bool DF2, bool SF2>
struct HighType<SymmetricMatrix<MT1, SO1, DF1, SF1>, SymmetricMatrix<MT2, SO2, DF2, SF2>> {
	using Type = SymmetricMatrix<typename HighType<MT1, MT2>::Type>;
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
template <typename MT1, bool SO1, bool DF1, bool SF1, typename MT2, bool SO2, bool DF2, bool SF2>
struct LowType<SymmetricMatrix<MT1, SO1, DF1, SF1>, SymmetricMatrix<MT2, SO2, DF2, SF2>> {
	using Type = SymmetricMatrix<typename LowType<MT1, MT2>::Type>;
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
template <typename MT, size_t I, size_t N>
struct SubmatrixTraitEval1<
	MT, I, I, N, N,
	EnableIf_t<I != inf && N != inf && IsSymmetric_v<MT> && !IsDiagonal_v<MT> && !IsUniform_v<MT> && !IsZero_v<MT>>> {
	using Type = SymmetricMatrix<typename SubmatrixTraitEval2<MT, I, I, N, N>::Type>;
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
