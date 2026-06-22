// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_JOIN_H
#define METRIC_NUMERIC_MATH_FUNCTORS_JOIN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsPaddingEnabled.h>
#include <metric/numeric/math/typetraits/IsSIMDEnabled.h>
#include <metric/numeric/math/typetraits/YieldsHermitian.h>
#include <metric/numeric/math/typetraits/YieldsLower.h>
#include <metric/numeric/math/typetraits/YieldsStrictlyLower.h>
#include <metric/numeric/math/typetraits/YieldsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/YieldsSymmetric.h>
#include <metric/numeric/math/typetraits/YieldsUniLower.h>
#include <metric/numeric/math/typetraits/YieldsUniUpper.h>
#include <metric/numeric/math/typetraits/YieldsUniform.h>
#include <metric/numeric/math/typetraits/YieldsUpper.h>
#include <metric/numeric/math/typetraits/YieldsZero.h>
#include <metric/numeric/system/Inline.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Generic wrapper for unpacking paired function arguments.
// \ingroup functors
*/
template <typename OP> // Type of the operation
struct Join {
  public:
	//**********************************************************************************************
	/*!\brief Constructor of the Join functor.
	//
	// \param op The wrapped operation.
	*/
	explicit constexpr Join(const OP &op) : op_(op) // The wrapped operation.
	{
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the wrapped operation for the given arguments.
	//
	// \param args The given arguments.
	// \return The result of the wrapped operation for the given arguments.
	*/
	template <typename... Args>
	METRIC_NUMERIC_ALWAYS_INLINE constexpr decltype(auto) operator()(const Args &...args) const
	{
		return op_(args...);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the wrapped operation for the given arguments.
	//
	// \param p The first two arguments in form of a pair.
	// \param args The remaining arguments.
	// \return The result of the wrapped operation for the given arguments.
	*/
	template <typename T1, typename T2, typename... Args>
	METRIC_NUMERIC_ALWAYS_INLINE constexpr decltype(auto) operator()(const std::pair<T1, T2> &p,
																	 const Args &...args) const
	{
		return (*this)(p.first, p.second, args...);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether SIMD is enabled for the specified data types \a Args.
	//
	// \return \a true in case SIMD is enabled for the data types \a Args, \a false if not.
	*/
	template <typename... Args> static constexpr bool simdEnabled() { return IsSIMDEnabled_v<OP, Args...>; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the operation supports padding, i.e. whether it can deal with zeros.
	//
	// \return \a true in case padding is supported, \a false if not.
	*/
	static constexpr bool paddingEnabled() { return IsPaddingEnabled_v<OP>; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the wrapped operation for the given SIMD vectors.
	//
	// \param args The given SIMD vectors.
	// \return The result of the wrapped operation for the given SIMD vectors.
	*/
	template <typename... Args> METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) load(const Args &...args) const
	{
		return op_.load(args...);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns the result of the wrapped operation for the given SIMD vectors.
	//
	// \param p The first two SIMD vectors in form of a pair.
	// \param args The remaining SIMD vectors.
	// \return The result of the wrapped operation for the given SIMD vectors.
	*/
	template <typename T1, typename T2, typename... Args>
	METRIC_NUMERIC_ALWAYS_INLINE decltype(auto) load(const std::pair<T1, T2> &p, const Args &...args) const
	{
		return (*this).load(p.first, p.second, args...);
	}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	OP op_; //!< The wrapped operation.
			//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Wraps the given operation to unpack paired arguments.
// \ingroup functors
//
// \param op The operation to be wrapped.
// \return The wrapped operation.
//
// The \a join() function returns a function object that represents the given operation, but
// is able to unpack arguments in the form of pairs.
*/
template <typename OP> // Type of the operation
constexpr Join<OP> join(const OP &op)
{
	return Join<OP>(op);
}
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUNIFORM SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename T, typename... Args>
struct YieldsUniform<Join<OP>, T, Args...> : public YieldsUniform<OP, T, Args...> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSYMMETRIC SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename MT, typename... MTs>
struct YieldsSymmetric<Join<OP>, MT, MTs...> : public YieldsSymmetric<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSHERMITIAN SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename MT, typename... MTs>
struct YieldsHermitian<Join<OP>, MT, MTs...> : public YieldsHermitian<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename MT, typename... MTs>
struct YieldsLower<Join<OP>, MT, MTs...> : public YieldsLower<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUNILOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename MT, typename... MTs>
struct YieldsUniLower<Join<OP>, MT, MTs...> : public YieldsUniLower<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSTRICTLYLOWER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename MT, typename... MTs>
struct YieldsStrictlyLower<Join<OP>, MT, MTs...> : public YieldsStrictlyLower<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename MT, typename... MTs>
struct YieldsUpper<Join<OP>, MT, MTs...> : public YieldsUpper<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSUNIUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename MT, typename... MTs>
struct YieldsUniUpper<Join<OP>, MT, MTs...> : public YieldsUniUpper<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSSTRICTLYUPPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename MT, typename... MTs>
struct YieldsStrictlyUpper<Join<OP>, MT, MTs...> : public YieldsStrictlyUpper<OP, MT, MTs...> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  YIELDSZERO SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename OP, typename T, typename... Args>
struct YieldsZero<Join<OP>, T, Args...> : public YieldsZero<OP, T, Args...> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
