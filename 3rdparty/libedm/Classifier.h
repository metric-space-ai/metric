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


#ifndef  CLASSIFIER_FILE
#define  CLASSIFIER_FILE

#include <string>
//#include "Obj.h"

namespace libedm
{
	//base class of all classifier
	//class CObj;
	class CDataset;
	class CPrediction;

	class CClassifier //: virtual public CObj
	{
	public:
		//predict a dataset
		virtual CPrediction *Classify(const CDataset &DataSet) const =0;
		virtual inline ~CClassifier()=0;

		//classifier is saved into file, and can be reconstructed from it
		// virtual int Save(const string &Path,const string &FileName) const =0;
		// virtual bool Dump(const string &FileName) const =0;

		virtual CClassifier* Clone() const =0;

	private:
	};
	inline CClassifier::~CClassifier(){}

	typedef CClassifier *CreateFunc(const CDataset &Dataset, const void *Params);
	typedef CClassifier *FileCreateFunc(const std::string &Path,const std::string &FileName);

}//namespace

#endif