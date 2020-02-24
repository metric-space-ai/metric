//=================================================================================================
/*!
//  \file blaze/math/expressions/VecVecKronExpr.h
//  \brief Header file for the VecVecKronExpr base class
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

#ifndef _BLAZE_MATH_EXPRESSIONS_VECVECKRONEXPR_H_
#define _BLAZE_MATH_EXPRESSIONS_VECVECKRONEXPR_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../../math/expressions/KronExpr.h"


namespace blaze {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all vector/vector Kronecker expression templates.
// \ingroup math
//
// The VecVecKronExpr class serves as a tag for all expression templates that implement a
// vector/vector Kronecker product. All classes, that represent a vector Kronecker product
// and that are used within the expression template environment of the Blaze library have to
// derive publicly from this class in order to qualify as vector Kronecker product expression
// template. Only in case a class is derived publicly from the VecVecKronExpr base class, the
// IsVecVecKronExpr type trait recognizes the class as valid vector Kronecker product expression
// template.
*/
template< typename VT >  // Vector base type of the expression
struct VecVecKronExpr
   : public KronExpr<VT>
{};
//*************************************************************************************************

} // namespace blaze

#endif
