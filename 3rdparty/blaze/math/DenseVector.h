//=================================================================================================
/*!
//  \file blaze/math/DenseVector.h
//  \brief Header file for all basic DenseVector functionality
//
//  Copyright (C) 2012-2020 Klaus Iglberger - All Rights Reserved
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

#ifndef _BLAZE_MATH_DENSEVECTOR_H_
#define _BLAZE_MATH_DENSEVECTOR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../math/dense/DenseVector.h"
#include "../math/expressions/DenseVector.h"
#include "../math/expressions/DVecDVecAddExpr.h"
#include "../math/expressions/DVecDVecCrossExpr.h"
#include "../math/expressions/DVecDVecDivExpr.h"
#include "../math/expressions/DVecDVecEqualExpr.h"
#include "../math/expressions/DVecDVecInnerExpr.h"
#include "../math/expressions/DVecDVecKronExpr.h"
#include "../math/expressions/DVecDVecMapExpr.h"
#include "../math/expressions/DVecDVecMultExpr.h"
#include "../math/expressions/DVecDVecSubExpr.h"
#include "../math/expressions/DVecEvalExpr.h"
#include "../math/expressions/DVecExpandExpr.h"
#include "../math/expressions/DVecGenExpr.h"
#include "../math/expressions/DVecMapExpr.h"
#include "../math/expressions/DVecMeanExpr.h"
#include "../math/expressions/DVecNoAliasExpr.h"
#include "../math/expressions/DVecNormExpr.h"
#include "../math/expressions/DVecNoSIMDExpr.h"
#include "../math/expressions/DVecReduceExpr.h"
#include "../math/expressions/DVecScalarDivExpr.h"
#include "../math/expressions/DVecScalarMultExpr.h"
#include "../math/expressions/DVecSerialExpr.h"
#include "../math/expressions/DVecSoftmaxExpr.h"
#include "../math/expressions/DVecStdDevExpr.h"
#include "../math/expressions/DVecSVecAddExpr.h"
#include "../math/expressions/DVecSVecCrossExpr.h"
#include "../math/expressions/DVecSVecSubExpr.h"
#include "../math/expressions/DVecTransExpr.h"
#include "../math/expressions/DVecVarExpr.h"
#include "../math/expressions/SparseVector.h"
#include "../math/expressions/SVecDVecCrossExpr.h"
#include "../math/expressions/SVecDVecSubExpr.h"
#include "../math/expressions/SVecSVecCrossExpr.h"
#include "../math/smp/DenseVector.h"
#include "../math/smp/SparseVector.h"
#include "../math/Vector.h"
#include "../math/views/Subvector.h"

#endif
