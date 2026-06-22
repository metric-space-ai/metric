// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRAITS_DERESTRICTTRAIT_H
#define METRIC_NUMERIC_MATH_TRAITS_DERESTRICTTRAIT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/util/InvalidType.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Evaluation of the return type of the derestrict function.
// \ingroup math_traits
//
// Via this type trait it is possible to evaluate the resulting return type of the derestrict
// function. Given the non-const vector or matrix type \a T, the nested type \a Type corresponds
// to the resulting return type. In case \a T is neither a dense or sparse vector or matrix type,
// the resulting \a Type is set to \a INVALID_TYPE.
*/
template <typename T> // Type of the vector or matrix
struct DerestrictTrait {
  private:
	//**struct Failure******************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	struct Failure {
		using Type = INVALID_TYPE;
	};
	/*! \endcond */
	//**********************************************************************************************

	//**struct Result*******************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	struct Result {
		static T &create() noexcept;
		using Type = decltype(derestrict(create()));
	};
	/*! \endcond */
	//**********************************************************************************************

  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type =
		typename If_t<IsVector_v<RemoveReference_t<T>> || IsMatrix_v<RemoveReference_t<T>>, Result, Failure>::Type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the DerestrictTrait type trait.
// \ingroup math_traits
//
// The DerestrictTrait_t alias declaration provides a convenient shortcut to access the
// nested \a Type of the DerestrictTrait class template. For instance, given the type \a T
// the following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::DerestrictTrait<T>::Type;
   using Type2 = mtrc::numeric::DerestrictTrait_t<T>;
   \endcode
*/
template <typename T> using DerestrictTrait_t = typename DerestrictTrait<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
