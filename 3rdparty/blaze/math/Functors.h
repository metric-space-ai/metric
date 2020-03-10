//=================================================================================================
/*!
//  \file blaze/math/Functors.h
//  \brief Header file for all functors
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

#ifndef _BLAZE_MATH_FUNCTORS_H_
#define _BLAZE_MATH_FUNCTORS_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../math/functors/Abs.h"
#include "../math/functors/Acos.h"
#include "../math/functors/Acosh.h"
#include "../math/functors/Add.h"
#include "../math/functors/AddAssign.h"
#include "../math/functors/And.h"
#include "../math/functors/AndAssign.h"
#include "../math/functors/Arg.h"
#include "../math/functors/Asin.h"
#include "../math/functors/Asinh.h"
#include "../math/functors/Assign.h"
#include "../math/functors/Atan.h"
#include "../math/functors/Atan2.h"
#include "../math/functors/Atanh.h"
#include "../math/functors/Bind1st.h"
#include "../math/functors/Bind2nd.h"
#include "../math/functors/Bind3rd.h"
#include "../math/functors/Bitand.h"
#include "../math/functors/Bitor.h"
#include "../math/functors/Bitxor.h"
#include "../math/functors/Cbrt.h"
#include "../math/functors/Ceil.h"
#include "../math/functors/Clamp.h"
#include "../math/functors/Clear.h"
#include "../math/functors/Conj.h"
#include "../math/functors/Cos.h"
#include "../math/functors/Cosh.h"
#include "../math/functors/CTrans.h"
#include "../math/functors/DeclDiag.h"
#include "../math/functors/DeclHerm.h"
#include "../math/functors/DeclId.h"
#include "../math/functors/DeclLow.h"
#include "../math/functors/DeclStrLow.h"
#include "../math/functors/DeclStrUpp.h"
#include "../math/functors/DeclSym.h"
#include "../math/functors/DeclUniLow.h"
#include "../math/functors/DeclUniUpp.h"
#include "../math/functors/DeclUpp.h"
#include "../math/functors/DeclZero.h"
#include "../math/functors/Div.h"
#include "../math/functors/DivAssign.h"
#include "../math/functors/Erf.h"
#include "../math/functors/Erfc.h"
#include "../math/functors/Eval.h"
#include "../math/functors/Exp.h"
#include "../math/functors/Exp2.h"
#include "../math/functors/Exp10.h"
#include "../math/functors/Floor.h"
#include "../math/functors/Greater.h"
#include "../math/functors/Hypot.h"
#include "../math/functors/Imag.h"
#include "../math/functors/Inv.h"
#include "../math/functors/InvAdd.h"
#include "../math/functors/InvCbrt.h"
#include "../math/functors/InvSqrt.h"
#include "../math/functors/Join.h"
#include "../math/functors/Kron.h"
#include "../math/functors/L1Norm.h"
#include "../math/functors/L2Norm.h"
#include "../math/functors/L3Norm.h"
#include "../math/functors/L4Norm.h"
#include "../math/functors/LeftShiftAssign.h"
#include "../math/functors/Less.h"
#include "../math/functors/Log.h"
#include "../math/functors/Log2.h"
#include "../math/functors/Log10.h"
#include "../math/functors/LpNorm.h"
#include "../math/functors/MAC.h"
#include "../math/functors/MakePair.h"
#include "../math/functors/Max.h"
#include "../math/functors/Min.h"
#include "../math/functors/Minmax.h"
#include "../math/functors/ModuloAssign.h"
#include "../math/functors/Mult.h"
#include "../math/functors/MultAssign.h"
#include "../math/functors/NoAlias.h"
#include "../math/functors/Noop.h"
#include "../math/functors/NoSIMD.h"
#include "../math/functors/Not.h"
#include "../math/functors/Or.h"
#include "../math/functors/OrAssign.h"
#include "../math/functors/Pow.h"
#include "../math/functors/Pow2.h"
#include "../math/functors/Pow3.h"
#include "../math/functors/Pow4.h"
#include "../math/functors/Qdrt.h"
#include "../math/functors/Real.h"
#include "../math/functors/Reset.h"
#include "../math/functors/RightShiftAssign.h"
#include "../math/functors/Round.h"
#include "../math/functors/Schur.h"
#include "../math/functors/Serial.h"
#include "../math/functors/ShiftLI.h"
#include "../math/functors/ShiftLV.h"
#include "../math/functors/ShiftRI.h"
#include "../math/functors/ShiftRV.h"
#include "../math/functors/Sign.h"
#include "../math/functors/Sin.h"
#include "../math/functors/Sinh.h"
#include "../math/functors/SqrAbs.h"
#include "../math/functors/Sqrt.h"
#include "../math/functors/Sub.h"
#include "../math/functors/SubAssign.h"
#include "../math/functors/Tan.h"
#include "../math/functors/Tanh.h"
#include "../math/functors/Trans.h"
#include "../math/functors/Trunc.h"
#include "../math/functors/XorAssign.h"

#endif
