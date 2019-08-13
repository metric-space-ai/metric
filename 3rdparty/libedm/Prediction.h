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


#ifndef CRESULT_INC
#define CRESULT_INC

#include <vector>
#include <ctime>

namespace libedm
{
	//predicted result for each instance
	//class CDataset;
	class CPrediction //: public CObj
	{
	private:
		int ClassNum;
		int CaseNum;

		std::vector<std::vector<double>> Probs;//probability that each instance belongs to each class type
		double Accuracy;
		std::vector<int> PredLabelIndices;//predicted class label for each instance
		std::vector<bool> IsCorrect;//is the prediction correct?
	public:
		CPrediction(const CDataset &Dataset,const std::vector<std::vector<double>> &Probabilities,clock_t PredictTime);
		~CPrediction();

		const std::vector<std::vector<double>>& GetProbs() const;
		const std::vector<int> &GetPredictedLabelIndices() const;
		const std::vector<bool>& GetCorrectness() const;
		double GetAccuracy() const;

		int	GetClassNum()
		{
			return ClassNum;
		};
		int	GetCaseNum()
		{
			return CaseNum;
		};
			
	};
}

#include "Prediction.cpp"

#endif

