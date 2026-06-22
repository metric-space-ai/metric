// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_BIND1ST_H
#define METRIC_NUMERIC_MATH_FUNCTORS_BIND1ST_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/simd/Set.h>
#include <metric/numeric/math/typetraits/IsSIMDEnabled.h>
#include <metric/numeric/math/typetraits/YieldsSymmetric.h>
#include <metric/numeric/math/typetraits/YieldsUniform.h>
#include <metric/numeric/system/Inline.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic wrapper for an operation with fixed 1st argument.
// \ingroup functors
*/
template <typename OP // Type of the operation
		  ,
		  typename A1> // Type of the bound argument
struct Bind1st {
  public:
	//**********************************************************************************************
	/*!\brief Constructor of the Bind1st functor.
	//
	// \param op The wrapped operation.
	// \param a1 The 1st argument.
	*/
	constexpr Bind1st(const OP &op, const A1 &a1)
		: op_(op) // The wrapped operation.
		  ,
		  a1_(a1) // The 1st argument
	{
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the wrapped operation for the given arguments.
	//
	// \param args The remaining arguments.
	// \return The result of the wrapped operation for the given arguments.
	*/
	template <typename... Args> METRIC_NUMERIC_ALWAYS_INLINE constexpr decltype(auto) operator()(Args &&...args) const
	{
		return op_(a1_, std::forward<Args>(args)...);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether SIMD is enabled for the specified data types \a Args.
	//
	// \return \a true in case SIMD is enabled for the data types \a Args, \a false if not.
	*/
	template <typename... Args> static constexpr bool simdEnabled() { return IsSIMDEnabled_v<OP, A1, Args...>; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the operation supports padding, i.e. whether it can deal with zeros.
	//
	// \return \a true in case padding is supported, \a false if not.
	*/
	static constexpr bool paddingEnabled() { return false; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the wrapped operation for the given SIMD vectors.
	//
	// \param args The remaining SIMD vectors.
	// \return The result of the wrapped operation for the given SIMD vectors.
	*/
	template <typename... Args> METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) load(Args &&...args) const
	{
		return op_.load(set(a1_), std::forward<Args>(args)...);
	}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	OP op_; //!< The wrapped operation.
	A1 a1_; //!< The 1st argument.
			//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Binds the given object/value to the 1st parameter of the given operation.
// \ingroup functors
//
// \param op The operation to be wrapped.
// \param a1 The argument to be bound to the second parameter of the operation.
// \return The operation with bound 1st argument.
//
// The \a bind1st() function binds the given argument \a x to the 1st parameter of the given
// operation \a op.
*/
template <typename OP // Type of the operation
		  ,
		  typename A1> // Type of the bound argument
constexpr Bind1st<OP, A1> bind1st(const OP &op, const A1 &a1)
{
	return Bind1st<OP, A1>(op, a1);
}
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUNIFORM SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename A1, typename T>
struct YieldsUniform<Bind1st<OP, A1>, T> : public YieldsUniform<OP, T> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename A1, typename MT>
struct YieldsSymmetric<Bind1st<OP, A1>, MT> : public YieldsSymmetric<OP, MT> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
