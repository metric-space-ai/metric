//=================================================================================================
/*!
//  \file blaze/math/LAPACK.h
//  \brief Header file for the LAPACK functionality
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

#ifndef _BLAZE_MATH_LAPACK_H_
#define _BLAZE_MATH_LAPACK_H_


//*************************************************************************************************
// Includes
//*************************************************************************************************

#include "../math/lapack/geev.h"
#include "../math/lapack/gelqf.h"
#include "../math/lapack/geqlf.h"
#include "../math/lapack/geqp3.h"
#include "../math/lapack/geqrf.h"
#include "../math/lapack/gerqf.h"
#include "../math/lapack/gesdd.h"
#include "../math/lapack/gesv.h"
#include "../math/lapack/gesvd.h"
#include "../math/lapack/gesvdx.h"
#include "../math/lapack/getrf.h"
#include "../math/lapack/getri.h"
#include "../math/lapack/getrs.h"
#include "../math/lapack/heev.h"
#include "../math/lapack/heevd.h"
#include "../math/lapack/heevx.h"
#include "../math/lapack/hesv.h"
#include "../math/lapack/hetrf.h"
#include "../math/lapack/hetri.h"
#include "../math/lapack/hetrs.h"
#include "../math/lapack/orglq.h"
#include "../math/lapack/orgql.h"
#include "../math/lapack/orgqr.h"
#include "../math/lapack/orgrq.h"
#include "../math/lapack/ormlq.h"
#include "../math/lapack/ormql.h"
#include "../math/lapack/ormqr.h"
#include "../math/lapack/ormrq.h"
#include "../math/lapack/posv.h"
#include "../math/lapack/potrf.h"
#include "../math/lapack/potri.h"
#include "../math/lapack/potrs.h"
#include "../math/lapack/syev.h"
#include "../math/lapack/syevd.h"
#include "../math/lapack/syevx.h"
#include "../math/lapack/sysv.h"
#include "../math/lapack/sytrf.h"
#include "../math/lapack/sytri.h"
#include "../math/lapack/sytrs.h"
#include "../math/lapack/trsv.h"
#include "../math/lapack/trtri.h"
#include "../math/lapack/trtrs.h"
#include "../math/lapack/unglq.h"
#include "../math/lapack/ungql.h"
#include "../math/lapack/ungqr.h"
#include "../math/lapack/ungrq.h"
#include "../math/lapack/unmlq.h"
#include "../math/lapack/unmql.h"
#include "../math/lapack/unmqr.h"
#include "../math/lapack/unmrq.h"

#endif
