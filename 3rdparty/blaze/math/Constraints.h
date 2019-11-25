//=================================================================================================
/*!
//  \file blaze/math/Constraints.h
//  \brief Header file for all mathematical constraints
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

#ifndef _BLAZE_MATH_CONSTRAINTS_H_
#define _BLAZE_MATH_CONSTRAINTS_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../math/constraints/Adaptor.h"
#include "../math/constraints/AddExpr.h"
#include "../math/constraints/Aligned.h"
#include "../math/constraints/Band.h"
#include "../math/constraints/BinaryMapExpr.h"
#include "../math/constraints/BLASCompatible.h"
#include "../math/constraints/Column.h"
#include "../math/constraints/ColumnMajorMatrix.h"
#include "../math/constraints/Columns.h"
#include "../math/constraints/ColumnVector.h"
#include "../math/constraints/Computation.h"
#include "../math/constraints/ConstDataAccess.h"
#include "../math/constraints/Contiguous.h"
#include "../math/constraints/CrossExpr.h"
#include "../math/constraints/Custom.h"
#include "../math/constraints/Declaration.h"
#include "../math/constraints/DeclDiagExpr.h"
#include "../math/constraints/DeclExpr.h"
#include "../math/constraints/DeclHermExpr.h"
#include "../math/constraints/DeclLowExpr.h"
#include "../math/constraints/DeclSymExpr.h"
#include "../math/constraints/DeclUppExpr.h"
#include "../math/constraints/DenseMatrix.h"
#include "../math/constraints/DenseVector.h"
#include "../math/constraints/Diagonal.h"
#include "../math/constraints/DivExpr.h"
#include "../math/constraints/Elements.h"
#include "../math/constraints/EvalExpr.h"
#include "../math/constraints/Expression.h"
#include "../math/constraints/General.h"
#include "../math/constraints/Hermitian.h"
#include "../math/constraints/Identity.h"
#include "../math/constraints/Initializer.h"
#include "../math/constraints/Invertible.h"
#include "../math/constraints/Lower.h"
#include "../math/constraints/MatEvalExpr.h"
#include "../math/constraints/MatInvExpr.h"
#include "../math/constraints/MatMapExpr.h"
#include "../math/constraints/MatMatAddExpr.h"
#include "../math/constraints/MatMatMapExpr.h"
#include "../math/constraints/MatMatMultExpr.h"
#include "../math/constraints/MatMatSubExpr.h"
#include "../math/constraints/MatReduceExpr.h"
#include "../math/constraints/Matrix.h"
#include "../math/constraints/MatScalarDivExpr.h"
#include "../math/constraints/MatScalarMultExpr.h"
#include "../math/constraints/MatSerialExpr.h"
#include "../math/constraints/MatTransExpr.h"
#include "../math/constraints/MatVecMultExpr.h"
#include "../math/constraints/MultExpr.h"
#include "../math/constraints/MutableDataAccess.h"
#include "../math/constraints/NumericMatrix.h"
#include "../math/constraints/NumericVector.h"
#include "../math/constraints/Operation.h"
#include "../math/constraints/OpposedView.h"
#include "../math/constraints/Padded.h"
#include "../math/constraints/Proxy.h"
#include "../math/constraints/ReduceExpr.h"
#include "../math/constraints/RequiresEvaluation.h"
#include "../math/constraints/Resizable.h"
#include "../math/constraints/Restricted.h"
#include "../math/constraints/Row.h"
#include "../math/constraints/RowMajorMatrix.h"
#include "../math/constraints/Rows.h"
#include "../math/constraints/RowVector.h"
#include "../math/constraints/SchurExpr.h"
#include "../math/constraints/SerialExpr.h"
#include "../math/constraints/Shrinkable.h"
#include "../math/constraints/SIMDCombinable.h"
#include "../math/constraints/SIMDEnabled.h"
#include "../math/constraints/SIMDPack.h"
#include "../math/constraints/SMPAssignable.h"
#include "../math/constraints/SparseElement.h"
#include "../math/constraints/SparseMatrix.h"
#include "../math/constraints/SparseVector.h"
#include "../math/constraints/Square.h"
#include "../math/constraints/Static.h"
#include "../math/constraints/StorageOrder.h"
#include "../math/constraints/StrictlyLower.h"
#include "../math/constraints/StrictlyTriangular.h"
#include "../math/constraints/StrictlyUpper.h"
#include "../math/constraints/SubExpr.h"
#include "../math/constraints/Submatrix.h"
#include "../math/constraints/Subvector.h"
#include "../math/constraints/Symmetric.h"
#include "../math/constraints/TransExpr.h"
#include "../math/constraints/Transformation.h"
#include "../math/constraints/TransposeFlag.h"
#include "../math/constraints/Triangular.h"
#include "../math/constraints/TVecMatMultExpr.h"
#include "../math/constraints/UnaryMapExpr.h"
#include "../math/constraints/Uniform.h"
#include "../math/constraints/UniLower.h"
#include "../math/constraints/UniTriangular.h"
#include "../math/constraints/UniUpper.h"
#include "../math/constraints/Upper.h"
#include "../math/constraints/VecEvalExpr.h"
#include "../math/constraints/VecMapExpr.h"
#include "../math/constraints/VecScalarDivExpr.h"
#include "../math/constraints/VecScalarMultExpr.h"
#include "../math/constraints/VecSerialExpr.h"
#include "../math/constraints/Vector.h"
#include "../math/constraints/VecTransExpr.h"
#include "../math/constraints/VecTVecMultExpr.h"
#include "../math/constraints/VecVecAddExpr.h"
#include "../math/constraints/VecVecDivExpr.h"
#include "../math/constraints/VecVecMapExpr.h"
#include "../math/constraints/VecVecMultExpr.h"
#include "../math/constraints/VecVecSubExpr.h"
#include "../math/constraints/View.h"

#endif
