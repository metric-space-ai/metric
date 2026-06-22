// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SPARSEVECTOR_H
#define METRIC_NUMERIC_MATH_SPARSEVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Vector.h>
#include <metric/numeric/math/expressions/DVecSVecEqualExpr.h>
#include <metric/numeric/math/expressions/DVecSVecInnerExpr.h>
#include <metric/numeric/math/expressions/DVecSVecKronExpr.h>
#include <metric/numeric/math/expressions/DVecSVecMultExpr.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/expressions/SVecDVecDivExpr.h>
#include <metric/numeric/math/expressions/SVecDVecInnerExpr.h>
#include <metric/numeric/math/expressions/SVecDVecKronExpr.h>
#include <metric/numeric/math/expressions/SVecDVecMultExpr.h>
#include <metric/numeric/math/expressions/SVecEvalExpr.h>
#include <metric/numeric/math/expressions/SVecExpandExpr.h>
#include <metric/numeric/math/expressions/SVecFixExpr.h>
#include <metric/numeric/math/expressions/SVecMapExpr.h>
#include <metric/numeric/math/expressions/SVecMeanExpr.h>
#include <metric/numeric/math/expressions/SVecNoAliasExpr.h>
#include <metric/numeric/math/expressions/SVecNoSIMDExpr.h>
#include <metric/numeric/math/expressions/SVecNormExpr.h>
#include <metric/numeric/math/expressions/SVecReduceExpr.h>
#include <metric/numeric/math/expressions/SVecRepeatExpr.h>
#include <metric/numeric/math/expressions/SVecSVecAddExpr.h>
#include <metric/numeric/math/expressions/SVecSVecEqualExpr.h>
#include <metric/numeric/math/expressions/SVecSVecInnerExpr.h>
#include <metric/numeric/math/expressions/SVecSVecKronExpr.h>
#include <metric/numeric/math/expressions/SVecSVecMultExpr.h>
#include <metric/numeric/math/expressions/SVecSVecSubExpr.h>
#include <metric/numeric/math/expressions/SVecScalarDivExpr.h>
#include <metric/numeric/math/expressions/SVecScalarMultExpr.h>
#include <metric/numeric/math/expressions/SVecSerialExpr.h>
#include <metric/numeric/math/expressions/SVecStdDevExpr.h>
#include <metric/numeric/math/expressions/SVecTransExpr.h>
#include <metric/numeric/math/expressions/SVecVarExpr.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/serialization/VectorSerializer.h>
#include <metric/numeric/math/smp/DenseVector.h>
#include <metric/numeric/math/smp/SparseVector.h>
#include <metric/numeric/math/sparse/SparseVector.h>
#include <metric/numeric/math/views/Subvector.h>

#endif
