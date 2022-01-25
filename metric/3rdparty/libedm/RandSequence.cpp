/*
Copyright (c) 2014, Qiangli Zhao and Yanhuang Jiang
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

* Neither the name of the copyright holders nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <vector>
// using namespace std;
//#include "Obj.h"
#include "RandSequence.h"
using namespace libedm;

// return a pseudorandom number, between 0 and Max-1
// Max: should no larger than INT_MAX
inline int libedm::IntRand(int Max)
{
	int Value = 0;

#if RAND_MAX >= INT_MAX
	Value = rand();
	if (Value >= Max)
		Value = Value % Max;
#else
	int Count = (Max + RAND_MAX) / (RAND_MAX + 1);
	for (int i = 0; i < Count; i++)
		Value += rand();
	Value = Value % Max;
#endif

	return Value;
}

inline CRandSequence::CRandSequence(int UMax) : Max(UMax) { Reset(); }

inline int CRandSequence::Poll()
{
	int Left = (int)OldIds.size();
	if (Left == 0)
		return -1;

	// select
	int Selected = IntRand(Left);
	int Ret = OldIds[Selected];
	OldIds.erase(OldIds.begin() + Selected);

	return Ret;
}

inline void CRandSequence::Reset()
{
	OldIds.clear();
	for (int i = 0; i < Max; i++)
		OldIds.push_back(i);
}
