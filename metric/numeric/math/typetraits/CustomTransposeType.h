// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_CUSTOMTRANSPOSETYPE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_CUSTOMTRANSPOSETYPE_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Evaluation of the custom transpose type.
// \ingroup math_type_traits
//
// The CustomTransposeType type trait provides a compile time indirection for the evaluation of
// the transpose type of a custom vector or custom matrix. By default, the nested type alias
// \c Type is set to the \a TransposeType of the given vector or matrix \a T, but it is possible
// to specialize CustomTransposeType. Note that cv-qualifiers are not explicitly handled.
*/
template <typename T> struct CustomTransposeType {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename T::TransposeType;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the CustomTransposeType type trait.
// \ingroup math_type_traits
//
// The CustomTransposeType_t alias declaration provides a convenient shortcut to access the
// nested \a Type of the CustomTransposeType class template. For instance, given the type \a T
// the following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::CustomTransposeType<T>::Type;
   using Type2 = mtrc::numeric::CustomTransposeType_t<T>;
   \endcode
*/
template <typename T> using CustomTransposeType_t = typename CustomTransposeType<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
