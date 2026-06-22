// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_MAKECOMPLEX_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_MAKECOMPLEX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Complex.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Converting the given type to the matching 'complex' type.
// \ingroup type_traits
//
// The MakeComplex type trait converts the given floating point type \a T to the matching
// 'complex' type with the same cv-qualifiers. 'complex' types are preserved. For all other
// types, including integral types, no type conversion is performed.

   \code
   mtrc::numeric::MakeComplex< float                    >::Type  // Results in 'complex<float>'
   mtrc::numeric::MakeComplex< complex<double>          >::Type  // Results in 'complex<double>'
   mtrc::numeric::MakeComplex< const long double        >::Type  // Results in 'const complex<long double>'
   mtrc::numeric::MakeComplex< volatile complex<double> >::Type  // Results in 'volatile complex<double>'
   \endcode

// Note that it is possible to add support for other data types by specializing the MakeComplex
// class template.
*/
template <typename T> struct MakeComplex {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*! Specialization of the MakeComplex type trait for 'float'.
// \ingroup type_traits
*/
template <> struct MakeComplex<float> {
	//**********************************************************************************************
	using Type = complex<float>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*! Specialization of the MakeComplex type trait for 'double'.
// \ingroup type_traits
*/
template <> struct MakeComplex<double> {
	//**********************************************************************************************
	using Type = complex<double>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*! Specialization of the MakeComplex type trait for 'long double'.
// \ingroup type_traits
*/
template <> struct MakeComplex<long double> {
	//**********************************************************************************************
	using Type = complex<long double>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*! Specialization of the MakeComplex type trait for complex numbers.
// \ingroup type_traits
*/
template <typename T> struct MakeComplex<complex<T>> {
	//**********************************************************************************************
	using Type = complex<T>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*! Specialization of the MakeComplex type trait for 'const T'.
// \ingroup type_traits
*/
template <typename T> struct MakeComplex<const T> {
	//**********************************************************************************************
	using Type = const typename MakeComplex<T>::Type;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*! Specialization of the MakeComplex type trait for 'volatile T'.
// \ingroup type_traits
*/
template <typename T> struct MakeComplex<volatile T> {
	//**********************************************************************************************
	using Type = volatile typename MakeComplex<T>::Type;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*! Specialization of the MakeComplex type trait for 'const volatile T'.
// \ingroup type_traits
*/
template <typename T> struct MakeComplex<const volatile T> {
	//**********************************************************************************************
	using Type = const volatile typename MakeComplex<T>::Type;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the MakeComplex type trait.
// \ingroup type_traits
//
// The MakeComplex_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the MakeComplex class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::MakeComplex<T>::Type;
   using Type2 = mtrc::numeric::MakeComplex_t<T>;
   \endcode
*/
template <typename T> using MakeComplex_t = typename MakeComplex<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
