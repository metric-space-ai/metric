// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SPARSEMATRIX_H
#define METRIC_NUMERIC_MATH_SPARSEMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Matrix.h>
#include <metric/numeric/math/adaptors/DiagonalMatrix.h>
#include <metric/numeric/math/adaptors/HermitianMatrix.h>
#include <metric/numeric/math/adaptors/LowerMatrix.h>
#include <metric/numeric/math/adaptors/SymmetricMatrix.h>
#include <metric/numeric/math/adaptors/UpperMatrix.h>
#include <metric/numeric/math/expressions/DMatSMatEqualExpr.h>
#include <metric/numeric/math/expressions/DMatSMatKronExpr.h>
#include <metric/numeric/math/expressions/DMatSMatSchurExpr.h>
#include <metric/numeric/math/expressions/DMatTSMatSchurExpr.h>
#include <metric/numeric/math/expressions/DVecSVecOuterExpr.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/expressions/SMatDMatKronExpr.h>
#include <metric/numeric/math/expressions/SMatDMatSchurExpr.h>
#include <metric/numeric/math/expressions/SMatDVecMultExpr.h>
#include <metric/numeric/math/expressions/SMatDeclDiagExpr.h>
#include <metric/numeric/math/expressions/SMatDeclHermExpr.h>
#include <metric/numeric/math/expressions/SMatDeclLowExpr.h>
#include <metric/numeric/math/expressions/SMatDeclStrLowExpr.h>
#include <metric/numeric/math/expressions/SMatDeclStrUppExpr.h>
#include <metric/numeric/math/expressions/SMatDeclSymExpr.h>
#include <metric/numeric/math/expressions/SMatDeclUniLowExpr.h>
#include <metric/numeric/math/expressions/SMatDeclUniUppExpr.h>
#include <metric/numeric/math/expressions/SMatDeclUppExpr.h>
#include <metric/numeric/math/expressions/SMatEvalExpr.h>
#include <metric/numeric/math/expressions/SMatFixExpr.h>
#include <metric/numeric/math/expressions/SMatMapExpr.h>
#include <metric/numeric/math/expressions/SMatMeanExpr.h>
#include <metric/numeric/math/expressions/SMatNoAliasExpr.h>
#include <metric/numeric/math/expressions/SMatNoSIMDExpr.h>
#include <metric/numeric/math/expressions/SMatNormExpr.h>
#include <metric/numeric/math/expressions/SMatReduceExpr.h>
#include <metric/numeric/math/expressions/SMatRepeatExpr.h>
#include <metric/numeric/math/expressions/SMatSMatAddExpr.h>
#include <metric/numeric/math/expressions/SMatSMatEqualExpr.h>
#include <metric/numeric/math/expressions/SMatSMatKronExpr.h>
#include <metric/numeric/math/expressions/SMatSMatMultExpr.h>
#include <metric/numeric/math/expressions/SMatSMatSchurExpr.h>
#include <metric/numeric/math/expressions/SMatSMatSubExpr.h>
#include <metric/numeric/math/expressions/SMatSVecMultExpr.h>
#include <metric/numeric/math/expressions/SMatScalarDivExpr.h>
#include <metric/numeric/math/expressions/SMatScalarMultExpr.h>
#include <metric/numeric/math/expressions/SMatSerialExpr.h>
#include <metric/numeric/math/expressions/SMatStdDevExpr.h>
#include <metric/numeric/math/expressions/SMatTSMatAddExpr.h>
#include <metric/numeric/math/expressions/SMatTSMatKronExpr.h>
#include <metric/numeric/math/expressions/SMatTSMatMultExpr.h>
#include <metric/numeric/math/expressions/SMatTSMatSchurExpr.h>
#include <metric/numeric/math/expressions/SMatTSMatSubExpr.h>
#include <metric/numeric/math/expressions/SMatTransExpr.h>
#include <metric/numeric/math/expressions/SMatVarExpr.h>
#include <metric/numeric/math/expressions/SVecDVecOuterExpr.h>
#include <metric/numeric/math/expressions/SVecSVecOuterExpr.h>
#include <metric/numeric/math/expressions/SparseMatrix.h>
#include <metric/numeric/math/expressions/TDVecSMatMultExpr.h>
#include <metric/numeric/math/expressions/TDVecTSMatMultExpr.h>
#include <metric/numeric/math/expressions/TSMatDMatSchurExpr.h>
#include <metric/numeric/math/expressions/TSMatDVecMultExpr.h>
#include <metric/numeric/math/expressions/TSMatSMatKronExpr.h>
#include <metric/numeric/math/expressions/TSMatSMatMultExpr.h>
#include <metric/numeric/math/expressions/TSMatSMatSchurExpr.h>
#include <metric/numeric/math/expressions/TSMatSMatSubExpr.h>
#include <metric/numeric/math/expressions/TSMatSVecMultExpr.h>
#include <metric/numeric/math/expressions/TSMatTSMatAddExpr.h>
#include <metric/numeric/math/expressions/TSMatTSMatKronExpr.h>
#include <metric/numeric/math/expressions/TSMatTSMatMultExpr.h>
#include <metric/numeric/math/expressions/TSMatTSMatSchurExpr.h>
#include <metric/numeric/math/expressions/TSMatTSMatSubExpr.h>
#include <metric/numeric/math/expressions/TSVecSMatMultExpr.h>
#include <metric/numeric/math/expressions/TSVecTSMatMultExpr.h>
#include <metric/numeric/math/serialization/MatrixSerializer.h>
#include <metric/numeric/math/smp/DenseMatrix.h>
#include <metric/numeric/math/smp/SparseMatrix.h>
#include <metric/numeric/math/sparse/SparseMatrix.h>
#include <metric/numeric/math/views/Column.h>
#include <metric/numeric/math/views/Row.h>
#include <metric/numeric/math/views/Submatrix.h>

#endif
