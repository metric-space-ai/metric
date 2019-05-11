//=================================================================================================
/*!
//  \file blaze/util/TypeTraits.h
//  \brief Header file for all type traits
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

#ifndef _BLAZE_UTIL_TYPETRAITS_H_
#define _BLAZE_UTIL_TYPETRAITS_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../util/typetraits/AddConst.h"
#include "../util/typetraits/AddCV.h"
#include "../util/typetraits/AddPointer.h"
#include "../util/typetraits/AddReference.h"
#include "../util/typetraits/AddVolatile.h"
#include "../util/typetraits/AlignmentOf.h"
#include "../util/typetraits/All.h"
#include "../util/typetraits/Any.h"
#include "../util/typetraits/CommonType.h"
#include "../util/typetraits/Decay.h"
#include "../util/typetraits/Extent.h"
#include "../util/typetraits/GetMemberType.h"
#include "../util/typetraits/HasMember.h"
#include "../util/typetraits/HasSize.h"
#include "../util/typetraits/HaveSameSize.h"
#include "../util/typetraits/IsArithmetic.h"
#include "../util/typetraits/IsArray.h"
#include "../util/typetraits/IsAssignable.h"
#include "../util/typetraits/IsBaseOf.h"
#include "../util/typetraits/IsBoolean.h"
#include "../util/typetraits/IsBuiltin.h"
#include "../util/typetraits/IsCharacter.h"
#include "../util/typetraits/IsClass.h"
#include "../util/typetraits/IsComplex.h"
#include "../util/typetraits/IsComplexDouble.h"
#include "../util/typetraits/IsComplexFloat.h"
#include "../util/typetraits/IsConst.h"
#include "../util/typetraits/IsConstructible.h"
#include "../util/typetraits/IsConvertible.h"
#include "../util/typetraits/IsDestructible.h"
#include "../util/typetraits/IsDouble.h"
#include "../util/typetraits/IsEmpty.h"
#include "../util/typetraits/IsEnum.h"
#include "../util/typetraits/IsFloat.h"
#include "../util/typetraits/IsFloatingPoint.h"
#include "../util/typetraits/IsInteger.h"
#include "../util/typetraits/IsIntegral.h"
#include "../util/typetraits/IsLong.h"
#include "../util/typetraits/IsLongDouble.h"
#include "../util/typetraits/IsLValueReference.h"
#include "../util/typetraits/IsNumeric.h"
#include "../util/typetraits/IsObject.h"
#include "../util/typetraits/IsPod.h"
#include "../util/typetraits/IsPointer.h"
#include "../util/typetraits/IsReference.h"
#include "../util/typetraits/IsRValueReference.h"
#include "../util/typetraits/IsSame.h"
#include "../util/typetraits/IsShort.h"
#include "../util/typetraits/IsSigned.h"
#include "../util/typetraits/IsUnion.h"
#include "../util/typetraits/IsUnsigned.h"
#include "../util/typetraits/IsValid.h"
#include "../util/typetraits/IsVectorizable.h"
#include "../util/typetraits/IsVoid.h"
#include "../util/typetraits/IsVolatile.h"
#include "../util/typetraits/MakeSigned.h"
#include "../util/typetraits/MakeUnsigned.h"
#include "../util/typetraits/Rank.h"
#include "../util/typetraits/RemoveAllExtents.h"
#include "../util/typetraits/RemoveConst.h"
#include "../util/typetraits/RemoveCV.h"
#include "../util/typetraits/RemoveExtent.h"
#include "../util/typetraits/RemovePointer.h"
#include "../util/typetraits/RemoveReference.h"
#include "../util/typetraits/RemoveVolatile.h"
#include "../util/typetraits/Void.h"

#endif
