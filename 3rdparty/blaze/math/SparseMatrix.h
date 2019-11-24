//=================================================================================================
/*!
//  \file blaze/math/SparseMatrix.h
//  \brief Header file for all basic SparseMatrix functionality
//
//  Copyright (C) 2012-2019 Klaus Iglberger - All Rights Reserved
//
//  This file is part of the Blaze library. You can redistribute it and/or modify it under
//  the terms of the New (Revised) BSD License. Redistribution and use in source and binary
//  forms, with or without modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//     of conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//  3. Neither the names of the Blaze development group nor the names of its contributors
//     may be used to endorse or promote products derived from this software without specific
//     prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
//  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
//  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
//  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
*/
//=================================================================================================

#ifndef _BLAZE_MATH_SPARSEMATRIX_H_
#define _BLAZE_MATH_SPARSEMATRIX_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../math/adaptors/DiagonalMatrix.h"
#include "../math/adaptors/HermitianMatrix.h"
#include "../math/adaptors/LowerMatrix.h"
#include "../math/adaptors/SymmetricMatrix.h"
#include "../math/adaptors/UpperMatrix.h"
#include "../math/expressions/DenseMatrix.h"
#include "../math/expressions/DMatSMatEqualExpr.h"
#include "../math/expressions/DMatSMatKronExpr.h"
#include "../math/expressions/DMatSMatSchurExpr.h"
#include "../math/expressions/DMatTSMatSchurExpr.h"
#include "../math/expressions/DVecSVecOuterExpr.h"
#include "../math/expressions/SMatDeclDiagExpr.h"
#include "../math/expressions/SMatDeclHermExpr.h"
#include "../math/expressions/SMatDeclLowExpr.h"
#include "../math/expressions/SMatDeclSymExpr.h"
#include "../math/expressions/SMatDeclUppExpr.h"
#include "../math/expressions/SMatDMatKronExpr.h"
#include "../math/expressions/SMatDMatSchurExpr.h"
#include "../math/expressions/SMatDVecMultExpr.h"
#include "../math/expressions/SMatEvalExpr.h"
#include "../math/expressions/SMatMapExpr.h"
#include "../math/expressions/SMatMeanExpr.h"
#include "../math/expressions/SMatNormExpr.h"
#include "../math/expressions/SMatReduceExpr.h"
#include "../math/expressions/SMatScalarDivExpr.h"
#include "../math/expressions/SMatScalarMultExpr.h"
#include "../math/expressions/SMatSerialExpr.h"
#include "../math/expressions/SMatSMatAddExpr.h"
#include "../math/expressions/SMatSMatEqualExpr.h"
#include "../math/expressions/SMatSMatKronExpr.h"
#include "../math/expressions/SMatSMatMultExpr.h"
#include "../math/expressions/SMatSMatSchurExpr.h"
#include "../math/expressions/SMatSMatSubExpr.h"
#include "../math/expressions/SMatStdDevExpr.h"
#include "../math/expressions/SMatSVecMultExpr.h"
#include "../math/expressions/SMatTransExpr.h"
#include "../math/expressions/SMatTSMatAddExpr.h"
#include "../math/expressions/SMatTSMatKronExpr.h"
#include "../math/expressions/SMatTSMatMultExpr.h"
#include "../math/expressions/SMatTSMatSchurExpr.h"
#include "../math/expressions/SMatTSMatSubExpr.h"
#include "../math/expressions/SMatVarExpr.h"
#include "../math/expressions/SparseMatrix.h"
#include "../math/expressions/SVecDVecOuterExpr.h"
#include "../math/expressions/SVecSVecOuterExpr.h"
#include "../math/expressions/TDVecSMatMultExpr.h"
#include "../math/expressions/TDVecTSMatMultExpr.h"
#include "../math/expressions/TSMatDMatSchurExpr.h"
#include "../math/expressions/TSMatDVecMultExpr.h"
#include "../math/expressions/TSMatSMatKronExpr.h"
#include "../math/expressions/TSMatSMatMultExpr.h"
#include "../math/expressions/TSMatSMatSchurExpr.h"
#include "../math/expressions/TSMatSMatSubExpr.h"
#include "../math/expressions/TSMatSVecMultExpr.h"
#include "../math/expressions/TSMatTSMatAddExpr.h"
#include "../math/expressions/TSMatTSMatKronExpr.h"
#include "../math/expressions/TSMatTSMatMultExpr.h"
#include "../math/expressions/TSMatTSMatSchurExpr.h"
#include "../math/expressions/TSMatTSMatSubExpr.h"
#include "../math/expressions/TSVecSMatMultExpr.h"
#include "../math/expressions/TSVecTSMatMultExpr.h"
#include "../math/Matrix.h"
#include "../math/smp/DenseMatrix.h"
#include "../math/smp/SparseMatrix.h"
#include "../math/sparse/SparseMatrix.h"
#include "../math/views/Column.h"
#include "../math/views/Row.h"
#include "../math/views/Submatrix.h"

#endif
