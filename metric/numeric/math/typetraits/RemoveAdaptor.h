// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_REMOVEADAPTOR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_REMOVEADAPTOR_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Removal of top level adaptor types.
// \ingroup math_type_traits
//
// In case the given type is an adaptor type (SymmetricMatrix, LowerMatrix, UpperMatrix, ...),
// the RemoveAdaptor type trait removes the adaptor and extracts the contained general matrix
// type. Else the given type is returned as is. Note that cv-qualifiers are preserved.

   \code
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::CompressedMatrix;
   using mtrc::numeric::SymmetricMatrix;
   using mtrc::numeric::LowerMatrix;
   using mtrc::numeric::UpperMatrix;

   using SymmetricDynamic = SymmetricMatrix< DynamicMatrix<int> >;
   using LowerCompressed  = LowerMatrix< CompressedMatrix<float> >;
   using UpperDynamic     = UpperMatrix< DynamicMatrix<double> >;

   mtrc::numeric::RemoveAdaptor< SymmetricDynamic >::Type             // Results in 'DynamicMatrix<int>'
   mtrc::numeric::RemoveAdaptor< const LowerCompressed >::Type        // Results in 'const CompressedMatrix<float>'
   mtrc::numeric::RemoveAdaptor< volatile UpperDynamic >::Type        // Results in 'volatile DynamicMatrix<double>'
   mtrc::numeric::RemoveAdaptor< int >::Type                          // Results in 'int'
   mtrc::numeric::RemoveAdaptor< const DynamicVector<int> >::Type     // Results in 'const DynamicVector<int>'
   mtrc::numeric::RemoveAdaptor< volatile DynamicMatrix<int> >::Type  // Results in 'volatile DynamicMatrix<int>'
   \endcode
*/
template <typename T> struct RemoveAdaptor {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = T;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the RemoveAdaptor type trait for const types.
// \ingroup math_type_traits
*/
template <typename T> struct RemoveAdaptor<const T> {
  public:
	//**********************************************************************************************
	using Type = const typename RemoveAdaptor<T>::Type;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the RemoveAdaptor type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T> struct RemoveAdaptor<volatile T> {
  public:
	//**********************************************************************************************
	using Type = volatile typename RemoveAdaptor<T>::Type;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the RemoveAdaptor type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T> struct RemoveAdaptor<const volatile T> {
  public:
	//**********************************************************************************************
	using Type = const volatile typename RemoveAdaptor<T>::Type;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the RemoveAdaptor type trait.
// \ingroup math_type_traits
//
// The RemoveAdaptor_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the RemoveAdaptor class template. For instance, given the type \a T the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::RemoveAdaptor<T>::Type;
   using Type2 = mtrc::numeric::RemoveAdaptor_t<T>;
   \endcode
*/
template <typename T> using RemoveAdaptor_t = typename RemoveAdaptor<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
