// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPES_H
#define METRIC_NUMERIC_UTIL_TYPES_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cstddef>
#include <cstdint>

namespace mtrc::numeric {

//=================================================================================================
//
//  TYPE DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\class mtrc::numeric::size_t
// \brief Size type of the Metric numeric library.
// \ingroup util
*/
using std::size_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::ptrdiff_t
// \brief Pointer difference type of the Metric numeric library.
// \ingroup util
*/
using std::ptrdiff_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Byte data type of the Metric numeric library.
// \ingroup util
//
// The \a byte data type is guaranteed to be an integral data type of size 1.
*/
using byte_t = unsigned char;
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::int8_t
// \brief 8-bit signed integer type of the Metric numeric library.
// \ingroup util
*/
using std::int8_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::uint8_t
// \brief 8-bit unsigned integer type of the Metric numeric library.
// \ingroup util
*/
using std::uint8_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::int16_t
// \brief 16-bit signed integer type of the Metric numeric library.
// \ingroup util
*/
using std::int16_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::uint16_t
// \brief 16-bit unsigned integer type of the Metric numeric library.
// \ingroup util
*/
using std::uint16_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::int32_t
// \brief 32-bit signed integer type of the Metric numeric library.
// \ingroup util
*/
using std::int32_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::uint32_t
// \brief 32-bit unsigned integer type of the Metric numeric library.
// \ingroup util
*/
using std::uint32_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::int64_t
// \brief 64-bit signed integer type of the Metric numeric library.
// \ingroup util
*/
using std::int64_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\class mtrc::numeric::uint64_t
// \brief 64-bit unsigned integer type of the Metric numeric library.
// \ingroup util
*/
using std::uint64_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief The largest available signed integer data type.
// \ingroup util
*/
using large_t = int64_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief The largest available unsigned integer data type.
// \ingroup util
*/
using ularge_t = uint64_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Unsigned integer data type for integral IDs.
// \ingroup util
*/
using id_t = ularge_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Type of the hidden arguments of character type within a Fortran forward declaration.
// \ingroup util
*/
#if METRIC_NUMERIC_GNU_COMPILER && METRIC_NUMERIC_GNU_MAJOR_VERSION <= 7
using fortran_charlen_t = int;
#else
using fortran_charlen_t = size_t;
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Type of single precision complex numbers within a Fortran forward declaration.
// \ingroup util
*/
struct fortran_complex8 {
	float real; //!< The real part of the complex number.
	float imag; //!< The imaginary part of the complex number.
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Type of double precision complex numbers within a Fortran forward declaration.
// \ingroup util
*/
struct fortran_complex16 {
	double real; //!< The real part of the complex number.
	double imag; //!< The imaginary part of the complex number.
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
