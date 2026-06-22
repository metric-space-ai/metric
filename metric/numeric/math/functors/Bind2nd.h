// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_BIND2ND_H
#define METRIC_NUMERIC_MATH_FUNCTORS_BIND2ND_H
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
/*!\brief Generic wrapper for an operation with fixed 2nd argument.
// \ingroup functors
*/
template <typename OP // Type of the operation
		  ,
		  typename A2> // Type of the bound argument
struct Bind2nd {
  public:
	//**********************************************************************************************
	/*!\brief Constructor of the Bind2nd functor.
	//
	// \param op The wrapped operation.
	// \param a2 The 2nd argument.
	*/
	constexpr Bind2nd(const OP &op, const A2 &a2)
		: op_(op) // The wrapped operation.
		  ,
		  a2_(a2) // The 2nd argument
	{
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the wrapped operation for the given arguments.
	//
	// \param a1 The 1st argument.
	// \param args The remaining arguments.
	// \return The result of the wrapped operation for the given arguments.
	*/
	template <typename A1, typename... Args>
	METRIC_NUMERIC_ALWAYS_INLINE constexpr decltype(auto) operator()(A1 &&a1, Args &&...args) const
	{
		return op_(std::forward<A1>(a1), a2_, std::forward<Args>(args)...);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether SIMD is enabled for the specified data types \a Ts.
	//
	// \return \a true in case SIMD is enabled for the data types \a Ts, \a false if not.
	*/
	template <typename A1, typename... Args> static constexpr bool simdEnabled()
	{
		return IsSIMDEnabled_v<OP, A1, A2, Args...>;
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
	// \param args The remaining SIMD vectors.
	// \return The result of the wrapped operation for the given SIMD vectors.
	*/
	template <typename A1, typename... Args>
	METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) load(A1 &&a1, Args &&...args) const
	{
		return op_.load(std::forward<A1>(a1), set(a2_), std::forward<Args>(args)...);
	}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	OP op_; //!< The wrapped operation.
	A2 a2_; //!< The 2nd argument.
			//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Binds the given object/value to the 2nd parameter of the given operation.
// \ingroup functors
//
// \param op The operation to be wrapped.
// \param a2 The argument to be bound to the second parameter of the operation.
// \return The operation with bound 2nd argument.
//
// The \a bind2nd() function binds the given argument \a x to the 2nd parameter of the given
// operation \a op.
*/
template <typename OP // Type of the operation
		  ,
		  typename A2> // Type of the bound argument
constexpr Bind2nd<OP, A2> bind2nd(const OP &op, const A2 &a2)
{
	return Bind2nd<OP, A2>(op, a2);
}
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUNIFORM SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename A2, typename T>
struct YieldsUniform<Bind2nd<OP, A2>, T> : public YieldsUniform<OP, T> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename A2, typename MT>
struct YieldsSymmetric<Bind2nd<OP, A2>, MT> : public YieldsSymmetric<OP, MT> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
