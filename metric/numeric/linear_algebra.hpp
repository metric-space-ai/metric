// METRIC numeric core — curated public facade for the linear-algebra contracts.
//
// Small METRIC-owned entry point that groups the promoted mtrc::numeric dense
// linear-algebra routines (inversion, determinant, Cholesky/LLH, LU, QR, eigen,
// SVD, and linear-system solve) together with the inversion flags so callers can
// include exactly the surface they need. The contracts are first-party METRIC C++
// (see metric/numeric/README.md for provenance and licensing). This facade
// declares no symbols of its own; it only re-exports promoted core headers.
//
// Optional backend boundary: header inclusion is link-free. The small explicit
// kernels (e.g. invert2x2, small determinants) are pure header-only. The
// decomposition routines (eigen, svd, general inv/solve) are optional LAPACK
// execution paths — calling them requires linking an appropriate LAPACK
// implementation. BLAS acceleration stays off by default (METRIC_NUMERIC_BLAS_MODE=0).

#ifndef METRIC_NUMERIC_LINEAR_ALGEBRA_HPP
#define METRIC_NUMERIC_LINEAR_ALGEBRA_HPP

#include <metric/numeric/math/DynamicMatrix.h>
#include <metric/numeric/math/DynamicVector.h>
#include <metric/numeric/math/InversionFlag.h>
#include <metric/numeric/math/dense/Inversion.h>
#include <metric/numeric/math/dense/LLH.h>
#include <metric/numeric/math/dense/LU.h>
#include <metric/numeric/math/dense/QR.h>
#include <metric/numeric/math/dense/Eigen.h>
#include <metric/numeric/math/dense/SVD.h>
#include <metric/numeric/math/dense/LSE.h>
#include <metric/numeric/math/dense/MMM.h>

#endif
