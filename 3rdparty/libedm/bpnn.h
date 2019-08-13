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


#ifndef  BPNN_Head_File
#define  BPNN_Head_File
#include <string>
#include <vector>

namespace libedm
{

	class CClassifier;
	class CDataset;
	class CPrediction;

	//back propagation neural network
	class CBpnn : public CClassifier
	{
	public:
		virtual ~CBpnn();
		virtual CPrediction *Classify(const CDataset &DataSet) const;
		virtual bool Dump(const string &FileName) const;
		virtual CClassifier *Clone() const;

		static string GetStaticName()
		{
			return StaticName;
		};

	public:
		typedef struct ParamStr
		{
			double Momentum;
			double LearnRate;
			double MinMSE;
			int MaxEpoch;
			int HideNode;
		}ParamStr;
		typedef struct RpropParamStr
		{
			double MinMSE;
			int MaxEpoch;
			int HideNode;
		}RpropParamStr;

	public:
		//constructing from data set by momentum method
		CBpnn(const CDataset &TrainData,double UMomentum=0.9,double ULearnRate=0.5,double UMinMSE=0.015,int UMaxEpoch=5000,int UHideNode=0);
		static CClassifier *Create(const CDataset &TrainData,const void* UParams)
		{
			if(UParams==NULL)
				return new CBpnn(TrainData,0.9,0.5,0.015,5000,0);

			ParamStr *Params=(ParamStr *)UParams;
			return new CBpnn(TrainData,Params->Momentum,Params->LearnRate,
				Params->MinMSE,Params->MaxEpoch,Params->HideNode);
		}

		//constructing from data set by RPROP method
		CBpnn(const CDataset &TrainData,double MinMSE=0.015,int MaxEpoch=3000,int UHideNode=0);
		static CClassifier *RpropCreate(const CDataset &TrainData,const void* UParams)
		{
			if(UParams==NULL)
				return new CBpnn(TrainData,0.015,3000,0);

			RpropParamStr *Params=(RpropParamStr *)UParams;
			return new CBpnn(TrainData,Params->MinMSE,Params->MaxEpoch,Params->HideNode);
		}

		//recovering from a file
		static CClassifier *FileCreate(const string &Path,const string &FileName)
		{
			return new CBpnn(Path,FileName);
		}
		CBpnn(const string &Path,const string &FileName);
		virtual int Save(const string &Path,const string &FileName) const;

		//training a NCL (negative correlation learning)
		//create a BPNN with random weights
		CBpnn(int InNode,int OutNode,int HideNode);
		//NCL: training on existed BPNN
		//training a NCL (negative correlation learning) BP-neural network by momentum method
		int MTNCLTrain(const CDataset &TrainData,const CPrediction &AveragePrediction,int M,double Gamma=0.390625,
			int MaxEpoch=5000,double MinMSE=0.015);
		//training a NCL (negative correlation learning) BP-neural network by RPROP method
		int RPNCLTrain(const CDataset &TrainData,const CPrediction &AveragePrediction,int M,double Gamma=0.390625,
			int MaxEpoch=3000,double MinMSE=0.015);

	private:
		double sigmoid(double u) const;
		//delta of gradient descent
		//�ݶ��½�: ���������ĸı���, LastDeriv���ϴε���,Deriv-���ε���,Value�������ٶȵľ���ֵ
		double delta(double &LastDeriv,double Deriv,double &Value,double &LastDelta) const;
		int sgn(double a) const;

		CBpnn(){};
		CBpnn(const CBpnn &a)
		{
			InNode=a.InNode;
			HdNode=a.HdNode;
			OutNode=a.OutNode;
			epoch=a.epoch;
			MSE=a.MSE;

			fill_d2(double,Weight_IN_HD,HdNode,InNode,a.Weight_IN_HD[i][j]);
			fill_d1(double,Bias_HD,HdNode,a.Bias_HD[i]);
			fill_d2(double,Weight_HD_OUT,OutNode,HdNode,a.Weight_HD_OUT[i][j]);
			fill_d1(double,Bias_OUT,OutNode,a.Bias_OUT[i]);
		};

	private:
		int InNode;
		int HdNode;
		int OutNode;

		double *Bias_HD;
		double *Bias_OUT;
		double **Weight_IN_HD;
		double **Weight_HD_OUT;

	private:
		const static string StaticName;
		int epoch;
		double MSE;
	};
}//namespace

#include "bpnn.cpp"

#endif

