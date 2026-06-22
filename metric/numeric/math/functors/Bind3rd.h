// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_BIND3RD_H
#define METRIC_NUMERIC_MATH_FUNCTORS_BIND3RD_H
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
/*!\brief Generic wrapper for an operation with fixed 3rd argument.
// \ingroup functors
*/
template <typename OP // Type of the operation
		  ,
		  typename A3> // Type of the bound argument
struct Bind3rd {
  public:
	//**********************************************************************************************
	/*!\brief Constructor of the Bind3rd functor.
	//
	// \param op The wrapped operation.
	// \param a3 The 3rd argument.
	*/
	constexpr Bind3rd(const OP &op, const A3 &a3)
		: op_(op) // The wrapped operation.
		  ,
		  a3_(a3) // The 3rd argument
	{
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the wrapped operation for the given arguments.
	//
	// \param a1 The 1st argument.
	// \param a2 The 2nd argument.
	// \param args The remaining arguments.
	// \return The result of the wrapped operation for the given arguments.
	*/
	template <typename A1, typename A2, typename... Args>
	METRIC_NUMERIC_ALWAYS_INLINE constexpr decltype(auto) operator()(A1 &&a1, A2 &&a2, Args &&...args) const
	{
		return op_(std::forward<A1>(a1), std::forward<A2>(a2), a3_, std::forward<Args>(args)...);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether SIMD is enabled for the specified data types \a Ts.
	//
	// \return \a true in case SIMD is enabled for the data types \a Ts, \a false if not.
	*/
	template <typename A1, typename A2, typename... Args> static constexpr bool simdEnabled()
	{
		return IsSIMDEnabled_v<OP, A1, A2, A3, Args...>;
	}
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
	// \param a1 The 1st given SIMD vector.
	// \param a2 The 2nd given SIMD vector.
	// \param args The remaining SIMD vectors.
	// \return The result of the wrapped operation for the given SIMD vectors.
	*/
	template <typename A1, typename A2, typename... Args>
	METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) load(A1 &&a1, A2 &&a2, Args &&...args) const
	{
		return op_.load(std::forward<A1>(a1), std::forward<A2>(a2), set(a3_), std::forward<Args>(args)...);
	}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	OP op_; //!< The wrapped operation.
	A3 a3_; //!< The 3rd argument.
			//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Binds the given object/value to the 3rd parameter of the given operation.
// \ingroup functors
//
// \param op The operation to be wrapped.
// \param a3 The argument to be bound to the third parameter of the operation.
// \return The operation with bound 3rd argument.
//
// The \a bind3rd() function binds the given argument \a x to the 3rd parameter of the given
// operation \a op.
*/
template <typename OP // Type of the operation
		  ,
		  typename A3> // Type of the bound argument
constexpr Bind3rd<OP, A3> bind3rd(const OP &op, const A3 &a3)
{
	return Bind3rd<OP, A3>(op, a3);
}
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUNIFORM SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename A3, typename T>
struct YieldsUniform<Bind3rd<OP, A3>, T> : public YieldsUniform<OP, T> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename A3, typename MT>
struct YieldsSymmetric<Bind3rd<OP, A3>, MT> : public YieldsSymmetric<OP, MT> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
