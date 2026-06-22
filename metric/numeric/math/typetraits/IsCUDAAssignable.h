// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISCUDAASSIGNABLE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISCUDAASSIGNABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/Void.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> struct IsCUDAAssignable;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsCUDAAssignable type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct IsCUDAAssignableHelper : public FalseType {};

template <typename T>
struct IsCUDAAssignableHelper<T, Void_t<decltype(T::cudaAssignable)>> : public BoolConstant<T::cudaAssignable> {};

template <typename T>
struct IsCUDAAssignableHelper<T, EnableIf_t<IsExpression_v<T>>>
	: public IsCUDAAssignable<typename T::ResultType>::Type {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for data types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is an CUDA-assignable data
// type (i.e. if it is a data type that can possibly and efficiently be assigned by several
// threads). In this context, built-in data types as well as complex numbers are not considered
// CUDA-assignable, whereas several vector and matrix types (as for instance DynamicVector and
// DynamicMatrix) can be CUDA-assignable. If the type is CUDA-assignable, the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the class
// derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and
// the class derives from \a FalseType.

   \code
   using mtrc::numeric::StaticVector;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::DynamicMatrix;

   using VectorType = DynamicVector<int,columnVector>;

   VectorType a( 100UL );

   using SubvectorType = decltype( mtrc::numeric::subvector( a, 8UL, 16UL ) );

   mtrc::numeric::IsCUDAAssignable< VectorType >::value            // Evaluates to 1
   mtrc::numeric::IsCUDAAssignable< SubvectorType >::Type          // Results in TrueType
   mtrc::numeric::IsCUDAAssignable< CUDADynamicMatrix<int> >       // Is derived from TrueType
   mtrc::numeric::IsCUDAAssignable< int >::value                   // Evaluates to 0
   mtrc::numeric::IsCUDAAssignable< StaticVector<int,3UL> >::Type  // Results in FalseType
   mtrc::numeric::IsCUDAAssignable< StaticMatrix<int,4UL,5UL> >    // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsCUDAAssignable : public IsCUDAAssignableHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsCUDAAssignable type trait.
// \ingroup type_traits
//
// The IsCUDAAssignable_v variable template provides a convenient shortcut to access the nested
// \a value of the IsCUDAAssignable class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsCUDAAssignable<T>::value;
   constexpr bool value2 = mtrc::numeric::IsCUDAAssignable_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsCUDAAssignable_v = IsCUDAAssignable<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
