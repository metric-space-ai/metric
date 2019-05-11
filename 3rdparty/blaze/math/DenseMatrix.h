//=================================================================================================
/*!
//  \file blaze/math/DenseMatrix.h
//  \brief Header file for all basic DenseMatrix functionality
//
//  Copyright (C) 2012-2018 Klaus Iglberger - All Rights Reserved
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

#ifndef _BLAZE_MATH_DENSEMATRIX_H_
#define _BLAZE_MATH_DENSEMATRIX_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../math/adaptors/DiagonalMatrix.h"
#include "../math/adaptors/HermitianMatrix.h"
#include "../math/adaptors/LowerMatrix.h"
#include "../math/adaptors/SymmetricMatrix.h"
#include "../math/adaptors/UpperMatrix.h"
#include "../math/dense/DenseMatrix.h"
#include "../math/dense/Eigen.h"
#include "../math/dense/Inversion.h"
#include "../math/dense/LLH.h"
#include "../math/dense/LQ.h"
#include "../math/dense/LU.h"
#include "../math/dense/QL.h"
#include "../math/dense/QR.h"
#include "../math/dense/RQ.h"
#include "../math/dense/SVD.h"
#include "../math/expressions/DenseMatrix.h"
#include "../math/expressions/DMatDeclDiagExpr.h"
#include "../math/expressions/DMatDeclHermExpr.h"
#include "../math/expressions/DMatDeclLowExpr.h"
#include "../math/expressions/DMatDeclSymExpr.h"
#include "../math/expressions/DMatDeclUppExpr.h"
#include "../math/expressions/DMatDetExpr.h"
#include "../math/expressions/DMatDMatAddExpr.h"
#include "../math/expressions/DMatDMatEqualExpr.h"
#include "../math/expressions/DMatDMatMapExpr.h"
#include "../math/expressions/DMatDMatMultExpr.h"
#include "../math/expressions/DMatDMatSchurExpr.h"
#include "../math/expressions/DMatDMatSubExpr.h"
#include "../math/expressions/DMatDVecMultExpr.h"
#include "../math/expressions/DMatEvalExpr.h"
#include "../math/expressions/DMatInvExpr.h"
#include "../math/expressions/DMatMapExpr.h"
#include "../math/expressions/DMatNormExpr.h"
#include "../math/expressions/DMatReduceExpr.h"
#include "../math/expressions/DMatScalarDivExpr.h"
#include "../math/expressions/DMatScalarMultExpr.h"
#include "../math/expressions/DMatSerialExpr.h"
#include "../math/expressions/DMatSMatAddExpr.h"
#include "../math/expressions/DMatSMatMultExpr.h"
#include "../math/expressions/DMatSMatSubExpr.h"
#include "../math/expressions/DMatSVecMultExpr.h"
#include "../math/expressions/DMatTDMatAddExpr.h"
#include "../math/expressions/DMatTDMatMapExpr.h"
#include "../math/expressions/DMatTDMatMultExpr.h"
#include "../math/expressions/DMatTDMatSchurExpr.h"
#include "../math/expressions/DMatTDMatSubExpr.h"
#include "../math/expressions/DMatTransExpr.h"
#include "../math/expressions/DMatTSMatAddExpr.h"
#include "../math/expressions/DMatTSMatMultExpr.h"
#include "../math/expressions/DMatTSMatSubExpr.h"
#include "../math/expressions/DVecDVecOuterExpr.h"
#include "../math/expressions/SMatDMatMultExpr.h"
#include "../math/expressions/SMatDMatSubExpr.h"
#include "../math/expressions/SMatTDMatMultExpr.h"
#include "../math/expressions/SMatTDMatSubExpr.h"
#include "../math/expressions/SparseMatrix.h"
#include "../math/expressions/TDMatDMatMultExpr.h"
#include "../math/expressions/TDMatDVecMultExpr.h"
#include "../math/expressions/TDMatSMatAddExpr.h"
#include "../math/expressions/TDMatSMatMultExpr.h"
#include "../math/expressions/TDMatSMatSubExpr.h"
#include "../math/expressions/TDMatSVecMultExpr.h"
#include "../math/expressions/TDMatTDMatMultExpr.h"
#include "../math/expressions/TDMatTSMatMultExpr.h"
#include "../math/expressions/TDVecDMatMultExpr.h"
#include "../math/expressions/TDVecTDMatMultExpr.h"
#include "../math/expressions/TSMatDMatMultExpr.h"
#include "../math/expressions/TSMatDMatSubExpr.h"
#include "../math/expressions/TSMatTDMatMultExpr.h"
#include "../math/expressions/TSVecDMatMultExpr.h"
#include "../math/expressions/TSVecTDMatMultExpr.h"
#include "../math/Matrix.h"
#include "../math/smp/DenseMatrix.h"
#include "../math/smp/SparseMatrix.h"
#include "../math/views/Column.h"
#include "../math/views/Row.h"
#include "../math/views/Submatrix.h"
#include "../math/views/Subvector.h"

#endif
