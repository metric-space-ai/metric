// METRIC numeric core — curated public facade for the dense matrix contracts.
//
// Small METRIC-owned entry point that groups the promoted mtrc::numeric dense
// matrix containers, the triangular/symmetric/Hermitian adaptors, the row/column/
// submatrix/subvector/diagonal/selection views, and the METRIC-authored matrix
// primitive helpers so callers can include exactly the surface they need. The
// contracts are first-party METRIC C++ (see metric/numeric/README.md for
// provenance and licensing). This facade declares no symbols of its own; it only
// re-exports promoted core headers. Dense linear-algebra factorizations live in
// metric/numeric/linear_algebra.hpp.

#ifndef METRIC_NUMERIC_MATRIX_HPP
#define METRIC_NUMERIC_MATRIX_HPP

#include <metric/numeric/math/StorageOrder.h>
#include <metric/numeric/math/AlignmentFlag.h>
#include <metric/numeric/math/PaddingFlag.h>
#include <metric/numeric/math/DynamicMatrix.h>
#include <metric/numeric/math/StaticMatrix.h>
#include <metric/numeric/math/HybridMatrix.h>
#include <metric/numeric/math/CustomMatrix.h>
#include <metric/numeric/math/UniformMatrix.h>
#include <metric/numeric/math/ZeroMatrix.h>
#include <metric/numeric/math/InitializerMatrix.h>
#include <metric/numeric/math/IdentityMatrix.h>
#include <metric/numeric/math/DiagonalMatrix.h>
#include <metric/numeric/math/SymmetricMatrix.h>
#include <metric/numeric/math/HermitianMatrix.h>
#include <metric/numeric/math/LowerMatrix.h>
#include <metric/numeric/math/UpperMatrix.h>
#include <metric/numeric/math/UniLowerMatrix.h>
#include <metric/numeric/math/UniUpperMatrix.h>
#include <metric/numeric/math/StrictlyLowerMatrix.h>
#include <metric/numeric/math/StrictlyUpperMatrix.h>
#include <metric/numeric/math/Views.h>
#include <metric/numeric/math/MatrixPrimitives.h>

#endif
