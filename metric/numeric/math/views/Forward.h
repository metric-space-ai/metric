// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_FORWARD_H
#define METRIC_NUMERIC_MATH_VIEWS_FORWARD_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/AlignmentFlag.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/views/band/BaseTemplate.h>
#include <metric/numeric/math/views/column/BaseTemplate.h>
#include <metric/numeric/math/views/columns/BaseTemplate.h>
#include <metric/numeric/math/views/elements/BaseTemplate.h>
#include <metric/numeric/math/views/row/BaseTemplate.h>
#include <metric/numeric/math/views/rows/BaseTemplate.h>
#include <metric/numeric/math/views/submatrix/BaseTemplate.h>
#include <metric/numeric/math/views/subvector/BaseTemplate.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  mtrc::numeric NAMESPACE FORWARD DECLARATIONS
//
//=================================================================================================

template <size_t I, size_t N, typename VT, bool TF, typename... RSAs>
decltype(auto) subvector(Vector<VT, TF> &, RSAs...);

template <size_t I, size_t N, typename VT, bool TF, typename... RSAs>
decltype(auto) subvector(const Vector<VT, TF> &, RSAs...);

template <size_t I, size_t N, typename VT, bool TF, typename... RSAs>
decltype(auto) subvector(Vector<VT, TF> &&, RSAs...);

template <AlignmentFlag AF, size_t I, size_t N, typename VT, bool TF, typename... RSAs>
decltype(auto) subvector(Vector<VT, TF> &, RSAs...);

template <AlignmentFlag AF, size_t I, size_t N, typename VT, bool TF, typename... RSAs>
decltype(auto) subvector(const Vector<VT, TF> &, RSAs...);

template <AlignmentFlag AF, size_t I, size_t N, typename VT, bool TF, typename... RSAs>
decltype(auto) subvector(Vector<VT, TF> &&, RSAs...);

template <typename VT, bool TF, typename... RSAs> decltype(auto) subvector(Vector<VT, TF> &, size_t, size_t, RSAs...);

template <typename VT, bool TF, typename... RSAs>
decltype(auto) subvector(const Vector<VT, TF> &, size_t, size_t, RSAs...);

template <typename VT, bool TF, typename... RSAs> decltype(auto) subvector(Vector<VT, TF> &&, size_t, size_t, RSAs...);

template <AlignmentFlag AF, typename VT, bool TF, typename... RSAs>
decltype(auto) subvector(Vector<VT, TF> &, size_t, size_t, RSAs...);

template <AlignmentFlag AF, typename VT, bool TF, typename... RSAs>
decltype(auto) subvector(const Vector<VT, TF> &, size_t, size_t, RSAs...);

template <AlignmentFlag AF, typename VT, bool TF, typename... RSAs>
decltype(auto) subvector(Vector<VT, TF> &&, size_t, size_t, RSAs...);

template <size_t I, size_t... Is, typename VT, bool TF, typename... REAs>
decltype(auto) elements(Vector<VT, TF> &, REAs...);

template <size_t I, size_t... Is, typename VT, bool TF, typename... REAs>
decltype(auto) elements(const Vector<VT, TF> &, REAs...);

template <size_t I, size_t... Is, typename VT, bool TF, typename... REAs>
decltype(auto) elements(Vector<VT, TF> &&, REAs...);

template <typename VT, bool TF, typename T, typename... REAs>
decltype(auto) elements(Vector<VT, TF> &, T *, size_t, REAs...);

template <typename VT, bool TF, typename T, typename... REAs>
decltype(auto) elements(const Vector<VT, TF> &, T *, size_t, REAs...);

template <typename VT, bool TF, typename T, typename... REAs>
decltype(auto) elements(Vector<VT, TF> &&, T *, size_t, REAs...);

template <typename VT, bool TF, typename P, typename... REAs>
decltype(auto) elements(Vector<VT, TF> &, P, size_t, REAs...);

template <typename VT, bool TF, typename P, typename... REAs>
decltype(auto) elements(const Vector<VT, TF> &, P, size_t, REAs...);

template <typename VT, bool TF, typename P, typename... REAs>
decltype(auto) elements(Vector<VT, TF> &&, P, size_t, REAs...);

template <size_t I, size_t J, size_t M, size_t N, typename MT, bool SO, typename... RSAs>
decltype(auto) submatrix(Matrix<MT, SO> &, RSAs...);

template <size_t I, size_t J, size_t M, size_t N, typename MT, bool SO, typename... RSAs>
decltype(auto) submatrix(const Matrix<MT, SO> &, RSAs...);

template <size_t I, size_t J, size_t M, size_t N, typename MT, bool SO, typename... RSAs>
decltype(auto) submatrix(Matrix<MT, SO> &&, RSAs...);

template <AlignmentFlag AF, size_t I, size_t J, size_t M, size_t N, typename MT, bool SO, typename... RSAs>
decltype(auto) submatrix(Matrix<MT, SO> &, RSAs...);

template <AlignmentFlag AF, size_t I, size_t J, size_t M, size_t N, typename MT, bool SO, typename... RSAs>
decltype(auto) submatrix(const Matrix<MT, SO> &, RSAs...);

template <AlignmentFlag AF, size_t I, size_t J, size_t M, size_t N, typename MT, bool SO, typename... RSAs>
decltype(auto) submatrix(Matrix<MT, SO> &&, RSAs...);

template <typename MT, bool SO, typename... RSAs>
decltype(auto) submatrix(Matrix<MT, SO> &, size_t, size_t, size_t, size_t, RSAs...);

template <typename MT, bool SO, typename... RSAs>
decltype(auto) submatrix(const Matrix<MT, SO> &, size_t, size_t, size_t, size_t, RSAs...);

template <typename MT, bool SO, typename... RSAs>
decltype(auto) submatrix(Matrix<MT, SO> &&, size_t, size_t, size_t, size_t, RSAs...);

template <AlignmentFlag AF, typename MT, bool SO, typename... RSAs>
decltype(auto) submatrix(Matrix<MT, SO> &, size_t, size_t, size_t, size_t, RSAs...);

template <AlignmentFlag AF, typename MT, bool SO, typename... RSAs>
decltype(auto) submatrix(const Matrix<MT, SO> &, size_t, size_t, size_t, size_t, RSAs...);

template <AlignmentFlag AF, typename MT, bool SO, typename... RSAs>
decltype(auto) submatrix(Matrix<MT, SO> &&, size_t, size_t, size_t, size_t, RSAs...);

template <size_t I, typename MT, bool SO, typename... RRAs> decltype(auto) row(Matrix<MT, SO> &, RRAs...);

template <size_t I, typename MT, bool SO, typename... RRAs> decltype(auto) row(const Matrix<MT, SO> &, RRAs...);

template <size_t I, typename MT, bool SO, typename... RRAs> decltype(auto) row(Matrix<MT, SO> &&, RRAs...);

template <typename MT, bool SO, typename... RRAs> decltype(auto) row(Matrix<MT, SO> &, size_t, RRAs...);

template <typename MT, bool SO, typename... RRAs> decltype(auto) row(const Matrix<MT, SO> &, size_t, RRAs...);

template <typename MT, bool SO, typename... RRAs> decltype(auto) row(Matrix<MT, SO> &&, size_t, RRAs...);

template <size_t I, size_t... Is, typename MT, bool SO, typename... RRAs>
decltype(auto) rows(Matrix<MT, SO> &, RRAs...);

template <size_t I, size_t... Is, typename MT, bool SO, typename... RRAs>
decltype(auto) rows(const Matrix<MT, SO> &, RRAs...);

template <size_t I, size_t... Is, typename MT, bool SO, typename... RRAs>
decltype(auto) rows(Matrix<MT, SO> &&, RRAs...);

template <typename MT, bool SO, typename T, typename... RRAs>
decltype(auto) rows(Matrix<MT, SO> &, T *, size_t, RRAs...);

template <typename MT, bool SO, typename T, typename... RRAs>
decltype(auto) rows(const Matrix<MT, SO> &, T *, size_t, RRAs...);

template <typename MT, bool SO, typename T, typename... RRAs>
decltype(auto) rows(Matrix<MT, SO> &&, T *, size_t, RRAs...);

template <typename MT, bool SO, typename P, typename... RRAs> decltype(auto) rows(Matrix<MT, SO> &, P, size_t, RRAs...);

template <typename MT, bool SO, typename P, typename... RRAs>
decltype(auto) rows(const Matrix<MT, SO> &, P, size_t, RRAs...);

template <typename MT, bool SO, typename P, typename... RRAs>
decltype(auto) rows(Matrix<MT, SO> &&, P, size_t, RRAs...);

template <size_t I, typename MT, bool SO, typename... RCAs> decltype(auto) column(Matrix<MT, SO> &, RCAs...);

template <size_t I, typename MT, bool SO, typename... RCAs> decltype(auto) column(const Matrix<MT, SO> &, RCAs...);

template <size_t I, typename MT, bool SO, typename... RCAs> decltype(auto) column(Matrix<MT, SO> &&, RCAs...);

template <typename MT, bool SO, typename... RCAs> decltype(auto) column(Matrix<MT, SO> &, size_t, RCAs...);

template <typename MT, bool SO, typename... RCAs> decltype(auto) column(const Matrix<MT, SO> &, size_t, RCAs...);

template <typename MT, bool SO, typename... RCAs> decltype(auto) column(Matrix<MT, SO> &&, size_t, RCAs...);

template <size_t I, size_t... Is, typename MT, bool SO, typename... RCAs>
decltype(auto) columns(Matrix<MT, SO> &, RCAs...);

template <size_t I, size_t... Is, typename MT, bool SO, typename... RCAs>
decltype(auto) columns(const Matrix<MT, SO> &, RCAs...);

template <size_t I, size_t... Is, typename MT, bool SO, typename... RCAs>
decltype(auto) columns(Matrix<MT, SO> &&, RCAs...);

template <typename MT, bool SO, typename T, typename... RCAs>
decltype(auto) columns(Matrix<MT, SO> &, T *, size_t, RCAs...);

template <typename MT, bool SO, typename T, typename... RCAs>
decltype(auto) columns(const Matrix<MT, SO> &, T *, size_t, RCAs...);

template <typename MT, bool SO, typename T, typename... RCAs>
decltype(auto) columns(Matrix<MT, SO> &&, T *, size_t, RCAs...);

template <typename MT, bool SO, typename P, typename... RCAs>
decltype(auto) columns(Matrix<MT, SO> &, P, size_t, RCAs...);

template <typename MT, bool SO, typename P, typename... RCAs>
decltype(auto) columns(const Matrix<MT, SO> &, P, size_t, RCAs...);

template <typename MT, bool SO, typename P, typename... RCAs>
decltype(auto) columns(Matrix<MT, SO> &&, P, size_t, RCAs...);

template <ptrdiff_t I, typename MT, bool SO, typename... RBAs> decltype(auto) band(Matrix<MT, SO> &, RBAs...);

template <ptrdiff_t I, typename MT, bool SO, typename... RBAs> decltype(auto) band(const Matrix<MT, SO> &, RBAs...);

template <ptrdiff_t I, typename MT, bool SO, typename... RBAs> decltype(auto) band(Matrix<MT, SO> &&, RBAs...);

template <typename MT, bool SO, typename... RBAs> decltype(auto) band(Matrix<MT, SO> &, ptrdiff_t, RBAs...);

template <typename MT, bool SO, typename... RBAs> decltype(auto) band(const Matrix<MT, SO> &, ptrdiff_t, RBAs...);

template <typename MT, bool SO, typename... RBAs> decltype(auto) band(Matrix<MT, SO> &&, ptrdiff_t, RBAs...);

template <typename MT, bool SO, typename... RDAs> decltype(auto) diagonal(Matrix<MT, SO> &, RDAs...);

template <typename MT, bool SO, typename... RDAs> decltype(auto) diagonal(const Matrix<MT, SO> &, RDAs...);

template <typename MT, bool SO, typename... RDAs> decltype(auto) diagonal(Matrix<MT, SO> &&, RDAs...);

} // namespace mtrc::numeric

#endif
