//=================================================================================================
/*!
//  \file blaze/math/SparseVector.h
//  \brief Header file for all basic SparseVector functionality
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

#ifndef _BLAZE_MATH_SPARSEVECTOR_H_
#define _BLAZE_MATH_SPARSEVECTOR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../math/expressions/DenseVector.h"
#include "../math/expressions/DVecSVecEqualExpr.h"
#include "../math/expressions/DVecSVecInnerExpr.h"
#include "../math/expressions/DVecSVecKronExpr.h"
#include "../math/expressions/DVecSVecMultExpr.h"
#include "../math/expressions/SparseVector.h"
#include "../math/expressions/SVecDVecDivExpr.h"
#include "../math/expressions/SVecDVecInnerExpr.h"
#include "../math/expressions/SVecDVecKronExpr.h"
#include "../math/expressions/SVecDVecMultExpr.h"
#include "../math/expressions/SVecEvalExpr.h"
#include "../math/expressions/SVecExpandExpr.h"
#include "../math/expressions/SVecMapExpr.h"
#include "../math/expressions/SVecMeanExpr.h"
#include "../math/expressions/SVecNoAliasExpr.h"
#include "../math/expressions/SVecNormExpr.h"
#include "../math/expressions/SVecNoSIMDExpr.h"
#include "../math/expressions/SVecReduceExpr.h"
#include "../math/expressions/SVecScalarDivExpr.h"
#include "../math/expressions/SVecScalarMultExpr.h"
#include "../math/expressions/SVecSerialExpr.h"
#include "../math/expressions/SVecStdDevExpr.h"
#include "../math/expressions/SVecSVecAddExpr.h"
#include "../math/expressions/SVecSVecEqualExpr.h"
#include "../math/expressions/SVecSVecInnerExpr.h"
#include "../math/expressions/SVecSVecKronExpr.h"
#include "../math/expressions/SVecSVecMultExpr.h"
#include "../math/expressions/SVecSVecSubExpr.h"
#include "../math/expressions/SVecTransExpr.h"
#include "../math/expressions/SVecVarExpr.h"
#include "../math/smp/DenseVector.h"
#include "../math/smp/SparseVector.h"
#include "../math/sparse/SparseVector.h"
#include "../math/Vector.h"
#include "../math/views/Subvector.h"

#endif
