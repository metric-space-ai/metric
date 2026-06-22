// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_EVALUATETRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_EVALUATETRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/util/typetraits/RemoveCVRef.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Evaluation of the return type of the evaluate function.
// \ingroup math_traits
//
// Via this type trait it is possible to evaluate the resulting return type of the evaluate
// function. Given the data type \a T, the nested type \a Type corresponds to the resulting
// return type. In case \a T is a dense or sparse vector or matrix type, the resulting \a Type
// is set to the according result type. Otherwise, \a Type is set to the unqualified and
// unmodified \a T.
*/
template <typename T // Type of the operand
		  ,
		  typename = void> // Restricting condition
struct EvaluateTrait {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = RemoveCVRef_t<T>;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of EvaluateTrait for vector and matrix types.
// \ingroup math_traits
*/
template <typename T> // Type of the operand
struct EvaluateTrait<T, EnableIf_t<IsVector_v<T> || IsMatrix_v<T>>> {
	//**********************************************************************************************
	using Type = typename T::ResultType;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the EvaluateTrait type trait.
// \ingroup math_traits
//
// The EvaluateTrait_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the EvaluateTrait class template. For instance, given the type \a T the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::EvaluateTrait<T>::Type;
   using Type2 = mtrc::numeric::EvaluateTrait_t<T>;
   \endcode
*/
template <typename T> using EvaluateTrait_t = typename EvaluateTrait<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
