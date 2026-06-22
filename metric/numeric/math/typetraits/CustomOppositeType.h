// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_CUSTOMOPPOSITETYPE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_CUSTOMOPPOSITETYPE_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Evaluation of the custom opposite type.
// \ingroup math_type_traits
//
// The CustomOppositeType type trait provides a compile time indirection for the evaluation of
// the opposite type of a custom matrix. By default, the nested type alias \c Type is set to
// the \a OppositeType of the given vector or matrix \a T, but it is possible to specialize
// CustomOppositeType. Note that cv-qualifiers are not explicitly handled.
*/
template <typename T> struct CustomOppositeType {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename T::OppositeType;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the CustomOppositeType type trait.
// \ingroup math_type_traits
//
// The CustomOppositeType_t alias declaration provides a convenient shortcut to access the
// nested \a Type of the CustomOppositeType class template. For instance, given the type \a T
// the following two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::CustomOppositeType<T>::Type;
   using Type2 = mtrc::numeric::CustomOppositeType_t<T>;
   \endcode
*/
template <typename T> using CustomOppositeType_t = typename CustomOppositeType<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
