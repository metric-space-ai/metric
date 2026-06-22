// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_VOID_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_VOID_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type check.
// \ingroup type_traits
//
// This type trait maps an arbitrary sequence of types to the type \c void. It can for instance
// be used to determine the validity of an expression:

   \code
   template< typename T1, typename T2, typename = Void_t<> >
   struct HasAdd : public FalseType {};

   template< typename T1, typename T2
		   , Void_t< decltype( std::declval<T1>() + std::declval<T2>() ) >
   struct HasAdd : public TrueType {};
   \endcode
*/
template <typename... Ts> using Void_t = void;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
