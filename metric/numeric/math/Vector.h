// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VECTOR_H
#define METRIC_NUMERIC_MATH_VECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <iomanip>
#include <iosfwd>
#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/TransposeFlag.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/Vector.h>
#include <metric/numeric/math/shims/Add.h>
#include <metric/numeric/math/shims/Div.h>
#include <metric/numeric/math/shims/Mult.h>
#include <metric/numeric/math/shims/Sub.h>
#include <metric/numeric/math/views/Elements.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name Vector functions */
//@{
template <typename VT, bool TF> bool isUniform(const Vector<VT, TF> &v);

template <typename VT, bool TF> decltype(auto) pow2(const Vector<VT, TF> &v);

template <typename VT, bool TF> decltype(auto) pow3(const Vector<VT, TF> &v);

template <typename VT, bool TF> decltype(auto) pow4(const Vector<VT, TF> &v);

template <typename VT1, bool TF1, typename VT2, bool TF2>
decltype(auto) inner(const Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs);

template <typename VT1, bool TF1, typename VT2, bool TF2>
decltype(auto) dot(const Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs);

template <typename VT1, bool TF1, typename VT2, bool TF2>
decltype(auto) operator,(const Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs);

template <typename VT1, bool TF1, typename VT2, bool TF2>
decltype(auto) outer(const Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs);

template <typename VT1, typename VT2, bool TF>
decltype(auto) cross(const Vector<VT1, TF> &lhs, const Vector<VT2, TF> &rhs);

template <typename VT> decltype(auto) reverse(VT &&v);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Checks if the given vector is a uniform vector.
// \ingroup vector
//
// \param v The vector to be checked.
// \return \a true if the vector is a uniform vector, \a false if not.
//
// This function checks if the given dense or sparse vector is a uniform vector. The vector
// is considered to be uniform if all its elements are identical. The following code example
// demonstrates the use of the function:

   \code
   mtrc::numeric::DynamicVector<int,mtrc::numeric::columnVector> a, b;
   // ... Initialization
   if( isUniform( a ) ) { ... }
   \endcode

// Optionally, it is possible to switch between strict semantics (mtrc::numeric::strict) and relaxed
// semantics (mtrc::numeric::relaxed):

   \code
   if( isUniform<relaxed>( a ) ) { ... }
   \endcode

// It is also possible to check if a vector expression results in a uniform vector:

   \code
   if( isUniform( a + b ) ) { ... }
   \endcode

// However, note that this might require the complete evaluation of the expression, including
// the generation of a temporary vector.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
inline bool isUniform(const Vector<VT, TF> &v)
{
	return isUniform<relaxed>(*v);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the square for each single element of the vector \a v.
// \ingroup matrix
//
// \param v The input vector.
// \return The square of each single element of \a v.
//
// The \a pow2() function computes the square for each element of the input vector \a v. The
// function returns an expression representing this operation.\n
// The following example demonstrates the use of the \a pow2() function:

   \code
   mtrc::numeric::DynamicVector<double> a, b;
   // ... Resizing and initialization
   b = pow2( a );
   \endcode
*/
template <typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) pow2(const Vector<VT, TF> &v)
{
	return (*v) * (*v);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the cube for each single element of the vector \a v.
// \ingroup matrix
//
// \param v The input vector.
// \return The cube of each single element of \a v.
//
// The \a pow3() function computes the cube for each element of the input vector \a v. The
// function returns an expression representing this operation.\n
// The following example demonstrates the use of the \a pow3() function:

   \code
   mtrc::numeric::DynamicVector<double> a, b;
   // ... Resizing and initialization
   b = pow3( a );
   \endcode
*/
template <typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) pow3(const Vector<VT, TF> &v)
{
	return (*v) * (*v) * (*v);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the quadruple for each single element of the vector \a v.
// \ingroup matrix
//
// \param v The input vector.
// \return The quadruple of each single element of \a v.
//
// The \a pow4() function computes the quadruple for each element of the input vector \a v. The
// function returns an expression representing this operation.\n
// The following example demonstrates the use of the \a pow4() function:

   \code
   mtrc::numeric::DynamicVector<double> a, b;
   // ... Resizing and initialization
   b = pow4( a );
   \endcode
*/
template <typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) pow4(const Vector<VT, TF> &v)
{
	return pow2(pow2(*v));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scalar product (dot/inner product) of two vectors (\f$ s=(\vec{a},\vec{b}) \f$).
// \ingroup vector
//
// \param lhs The left-hand side vector for the scalar product.
// \param rhs The right-hand side vector for the scalar product.
// \return The scalar product.
//
// This function represents the scalar product (inner product) of two vectors:

   \code
   mtrc::numeric::DynamicVector<double> a, b;
   mtrc::numeric::double res;
   // ... Resizing and initialization
   res = inner( a, b );
   \endcode

// The function returns a scalar value of the higher-order element type of the two involved
// vector element types \a VT1::ElementType and \a VT2::ElementType. Both vector types \a VT1
// and \a VT2 as well as the two element types \a VT1::ElementType and \a VT2::ElementType
// have to be supported by the MultTrait class template.\n
// In case the current sizes of the two given vectors don't match, a \a std::invalid_argument
// is thrown.
*/
template <typename VT1 // Type of the left-hand side vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline decltype(auto) inner(const Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs)
{
	return transTo<rowVector>(*lhs) * transTo<columnVector>(*rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scalar product (dot/inner product) of two vectors (\f$ s=(\vec{a},\vec{b}) \f$).
// \ingroup vector
//
// \param lhs The left-hand side vector for the scalar product.
// \param rhs The right-hand side vector for the scalar product.
// \return The scalar product.
//
// This function represents the scalar product (inner product) of two vectors:

   \code
   mtrc::numeric::DynamicVector<double> a, b;
   mtrc::numeric::double res;
   // ... Resizing and initialization
   res = dot( a, b );
   \endcode

// The function returns a scalar value of the higher-order element type of the two involved
// vector element types \a VT1::ElementType and \a VT2::ElementType. Both vector types \a VT1
// and \a VT2 as well as the two element types \a VT1::ElementType and \a VT2::ElementType
// have to be supported by the MultTrait class template.\n
// In case the current sizes of the two given vectors don't match, a \a std::invalid_argument
// is thrown.
*/
template <typename VT1 // Type of the left-hand side vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline decltype(auto) dot(const Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs)
{
	return inner(*lhs, *rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Scalar product (dot/inner product) of two vectors (\f$ s=(\vec{a},\vec{b}) \f$).
// \ingroup vector
//
// \param lhs The left-hand side vector for the scalar product.
// \param rhs The right-hand side vector for the scalar product.
// \return The scalar product.
//
// This function represents the scalar product (inner product) of two vectors:

   \code
   mtrc::numeric::DynamicVector<double> a, b;
   mtrc::numeric::double res;
   // ... Resizing and initialization
   res = (a,b);
   \endcode

// The function returns a scalar value of the higher-order element type of the two involved
// vector element types \a VT1::ElementType and \a VT2::ElementType. Both vector types \a VT1
// and \a VT2 as well as the two element types \a VT1::ElementType and \a VT2::ElementType
// have to be supported by the MultTrait class template.\n
// In case the current sizes of the two given vectors don't match, a \a std::invalid_argument
// is thrown.
*/
template <typename VT1 // Type of the left-hand side vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline decltype(auto) operator,(const Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs)
{
	return inner(*lhs, *rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Outer product of two vectors (\f$ A=\vec{b}*\vec{c}^T \f$).
// \ingroup vector
//
// \param lhs The left-hand side vector for the outer product.
// \param rhs The right-hand side vector for the outer product.
// \return The outer product.
//
// This function represents the outer product between two vectors:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicVector<double,columnVector> a, b;
   mtrc::numeric::DynamicMatrix<double,rowMajor> A;
   // ... Resizing and initialization
   A = outer( a, b );
   \endcode

// The operator returns an expression representing a matrix of the higher-order element type
// of the two involved element types \a VT1::ElementType and \a VT2::ElementType.
*/
template <typename VT1 // Type of the left-hand side vector
		  ,
		  bool TF1 // Transpose flag of the left-hand side vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF2> // Transpose flag of the right-hand side vector
inline decltype(auto) outer(const Vector<VT1, TF1> &lhs, const Vector<VT2, TF2> &rhs)
{
	return transTo<columnVector>(*lhs) * transTo<rowVector>(*rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Cross product of two vectors (\f$ \vec{a}=\vec{b} \times \vec{c} \f$).
// \ingroup vector
//
// \param lhs The left-hand side vector for the cross product.
// \param rhs The right-hand side vector for the cross product.
// \return The cross product of the two vectors.
// \exception std::invalid_argument Invalid vector size for cross product.
//
// This function computes the cross product of two vectors:

   \code
   mtrc::numeric::DynamicVector<double> a( 3UL ), b( 3UL );
   mtrc::numeric::StaticVector<double,3UL> c;
   // ... Resizing and initialization
   c = cross( a, b );
   \endcode

// The function returns an expression representing a dense vector of the higher-order element
// type of the two involved vector element types \a VT1::ElementType and \a VT2::ElementType.
// Both vector types \a VT1 and \a VT2 as well as the two element types \a VT1::ElementType
// and \a VT2::ElementType have to be supported by the CrossTrait class template.\n
// In case the current sizes of the two given vectors don't match, a \a std::invalid_argument
// is thrown.
*/
template <typename VT1 // Type of the left-hand side vector
		  ,
		  typename VT2 // Type of the right-hand side vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) cross(const Vector<VT1, TF> &lhs, const Vector<VT2, TF> &rhs)
{
	return (*lhs) % (*rhs);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Reverse the elements of a vector.
// \ingroup vector
//
// \param v The vector to be reversed.
// \return The reversed vector.
//
// This function reverses the elements of a dense or sparse vector. The following examples
// demonstrates this by means of a dense vector:

   \code
   mtrc::numeric::DynamicVector<int> a{ 1, 2, 3, 4, 5 };
   mtrc::numeric::DynamicVector<int> b;

   b = reverse( a );  // Results in ( 5 4 3 2 1 )
   \endcode
*/
template <typename VT> // Type of the vector
inline decltype(auto) reverse(VT &&v)
{
	return elements(*v, [max = (*v).size() - 1UL](size_t i) { return max - i; }, (*v).size());
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\name Vector operators */
//@{
template <typename VT, bool TF> std::ostream &operator<<(std::ostream &os, const Vector<VT, TF> &v);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Global output operator for dense and sparse vectors.
// \ingroup vector
//
// \param os Reference to the output stream.
// \param v Reference to a constant vector object.
// \return Reference to the output stream.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
inline std::ostream &operator<<(std::ostream &os, const Vector<VT, TF> &v)
{
	CompositeType_t<VT> tmp(*v);

	if (tmp.size() == 0UL) {
		os << "( )\n";
	} else if (TF == rowVector) {
		os << "(";
		for (size_t i = 0UL; i < tmp.size(); ++i)
			os << " " << tmp[i];
		os << " )\n";
	} else {
		for (size_t i = 0UL; i < tmp.size(); ++i)
			os << "( " << std::setw(11UL) << tmp[i] << " )\n";
	}

	return os;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
