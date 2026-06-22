// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_DENSEVECTOR_H
#define METRIC_NUMERIC_MATH_DENSEVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Vector.h>
#include <metric/numeric/math/dense/DenseVector.h>
#include <metric/numeric/math/expressions/DVecDVecAddExpr.h>
#include <metric/numeric/math/expressions/DVecDVecCrossExpr.h>
#include <metric/numeric/math/expressions/DVecDVecDivExpr.h>
#include <metric/numeric/math/expressions/DVecDVecEqualExpr.h>
#include <metric/numeric/math/expressions/DVecDVecInnerExpr.h>
#include <metric/numeric/math/expressions/DVecDVecKronExpr.h>
#include <metric/numeric/math/expressions/DVecDVecMapExpr.h>
#include <metric/numeric/math/expressions/DVecDVecMultExpr.h>
#include <metric/numeric/math/expressions/DVecDVecSubExpr.h>
#include <metric/numeric/math/expressions/DVecEvalExpr.h>
#include <metric/numeric/math/expressions/DVecExpandExpr.h>
#include <metric/numeric/math/expressions/DVecFixExpr.h>
#include <metric/numeric/math/expressions/DVecGenExpr.h>
#include <metric/numeric/math/expressions/DVecMapExpr.h>
#include <metric/numeric/math/expressions/DVecMeanExpr.h>
#include <metric/numeric/math/expressions/DVecNoAliasExpr.h>
#include <metric/numeric/math/expressions/DVecNoSIMDExpr.h>
#include <metric/numeric/math/expressions/DVecNormExpr.h>
#include <metric/numeric/math/expressions/DVecReduceExpr.h>
#include <metric/numeric/math/expressions/DVecRepeatExpr.h>
#include <metric/numeric/math/expressions/DVecSVecAddExpr.h>
#include <metric/numeric/math/expressions/DVecSVecCrossExpr.h>
#include <metric/numeric/math/expressions/DVecSVecSubExpr.h>
#include <metric/numeric/math/expressions/DVecScalarDivExpr.h>
#include <metric/numeric/math/expressions/DVecScalarMultExpr.h>
#include <metric/numeric/math/expressions/DVecSerialExpr.h>
#include <metric/numeric/math/expressions/DVecSoftmaxExpr.h>
#include <metric/numeric/math/expressions/DVecStdDevExpr.h>
#include <metric/numeric/math/expressions/DVecTransExpr.h>
#include <metric/numeric/math/expressions/DVecVarExpr.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/expressions/SVecDVecCrossExpr.h>
#include <metric/numeric/math/expressions/SVecDVecSubExpr.h>
#include <metric/numeric/math/expressions/SVecSVecCrossExpr.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/serialization/VectorSerializer.h>
#include <metric/numeric/math/smp/DenseVector.h>
#include <metric/numeric/math/smp/SparseVector.h>
#include <metric/numeric/math/views/Subvector.h>

#endif
