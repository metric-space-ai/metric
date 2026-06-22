// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SERIALIZATION_TYPEVALUEMAPPING_H
#define METRIC_NUMERIC_MATH_SERIALIZATION_TYPEVALUEMAPPING_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsComplex.h>
#include <metric/numeric/util/typetraits/IsFloatingPoint.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>
#include <metric/numeric/util/typetraits/IsSigned.h>
#include <metric/numeric/util/typetraits/IsUnsigned.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the TypeValueMapping class template.
// \ingroup math_serialization
*/
template <bool IsSignedIntegral, bool IsUnsignedIntegral, bool IsFloatingPoint, bool IsComplex>
struct TypeValueMappingHelper;
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the TypeValueMappingHelper for compound data types.
// \ingroup math_serialization
*/
template <> struct TypeValueMappingHelper<false, false, false, false> {
  public:
	//**********************************************************************************************
	enum { value = 0 };
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the TypeValueMappingHelper for signed integral data types.
// \ingroup math_serialization
*/
template <> struct TypeValueMappingHelper<true, false, false, false> {
  public:
	//**********************************************************************************************
	enum { value = 1 };
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the TypeValueMappingHelper for unsigned integral data types.
// \ingroup math_serialization
*/
template <> struct TypeValueMappingHelper<false, true, false, false> {
  public:
	//**********************************************************************************************
	enum { value = 2 };
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the TypeValueMappingHelper for floating-point data types.
// \ingroup math_serialization
*/
template <> struct TypeValueMappingHelper<false, false, true, false> {
  public:
	//**********************************************************************************************
	enum { value = 3 };
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the TypeValueMappingHelper for complex data types.
// \ingroup math_serialization
*/
template <> struct TypeValueMappingHelper<false, false, false, true> {
  public:
	//**********************************************************************************************
	enum { value = 4 };
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Conversion from a data type to a serial representation.
// \ingroup math_serialization
//
// This class template converts the given data type into an integral representation suited for
// serialization. Depending on the given data type, the \a value member enumeration is set to
// the according serial representation.
*/
template <typename T> struct TypeValueMapping {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	enum {
		value = TypeValueMappingHelper < IsIntegral_v<T> && IsSigned_v<T>,
		IsIntegral_v<T> &&IsUnsigned_v<T>,
		IsFloatingPoint_v<T>,
		IsComplex_v<T> > ::value
	};
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
