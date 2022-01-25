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

#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
// using namespace std;
//#include "Obj.h"
#include "Classifier.h"
#include "DataSet.h"
#include "Prediction.h"
#include "b-svm.h"
#include "svm.h"
using namespace libedm;

// const char	MyName[MAX_OBJECT_NAME_LENGTH]="SVM";
// const string CSVM::StaticName=MyName;

inline CSVM::~CSVM()
{
	if (svm != NULL)
		svm_free_and_destroy_model(&svm);
}

inline CSVM::CSVM(const CDataset &OrgSet, int usvm_type, int ukernel_type, int udegree, double ucoef0,
				  double ucache_size, double ueps, double uC, int unr_weight, int *uweight_label, double *uweight,
				  double unu, double up, int ushrinking, int uprobability)
{
	// start time for training
	// Name=MyName;
	clock_t start = clock();

	// data preprocess
	struct svm_problem DataDesc;
	PrepareData(OrgSet, DataDesc);

	// params for libsvm
	struct svm_parameter Param;
	memset(&Param, 0, sizeof(Param));
	Param.svm_type = usvm_type;
	Param.kernel_type = ukernel_type;
	Param.degree = udegree;
	Param.gamma = 1.0 / AttributeNum;
	Param.coef0 = ucoef0;
	Param.cache_size = ucache_size;
	Param.eps = ueps;
	Param.C = uC;
	Param.nr_weight = unr_weight;
	Param.weight_label = uweight_label;
	Param.weight = uweight;
	Param.nu = unu;
	Param.p = up;
	Param.shrinking = ushrinking;
	Param.probability = uprobability;

	// params checking
	const char *error_msg = svm_check_parameter(&DataDesc, &Param);
	assert(error_msg == NULL);
	// throw(CError(error_msg,100,1));

	// training
	svm = NULL;
	svm = svm_train(&DataDesc, &Param);
	svm_destroy_param(&Param);
	assert(svm != NULL);
	// {
	// 	throw(CError("fail to train svm!",100,1));
	// }

	// time consumed
	// CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

inline void CSVM::PrepareData(const CDataset &OrgSet, struct svm_problem &DataDesc)
{
	// for SVM, we need to expand all multivalued discrete attributes of the training data into multi continuous
	// attributes. expand discrete attribute
	const CDataset *TrainSet = &OrgSet;
	if (!OrgSet.AllContinuous())
		TrainSet = OrgSet.ExpandDiscrete();
	const MATRIX &TrainData = TrainSet->GetData();
	const CASE_INFO &CaseInfo = TrainSet->GetInfo();

	// number of attribute for data set
	AttributeNum = CaseInfo.ValidWidth - 1;
	// instances is formated as libsvm's requirements
	// number of instances
	DataDesc.l = CaseInfo.Height;
	// labels of instances
	DataDesc.y = new double[DataDesc.l];
	// content of instances (all attributes plus a tag for end of line, each node is initialized as end of a row)
	struct svm_node Val = {-1, 0};
	fill_d2(struct svm_node, DataDesc.x, CaseInfo.Height, CaseInfo.ValidWidth, Val);
	for (int i = 0; i < CaseInfo.Height; i++) {
		DataDesc.y[i] = (double)TrainData[i][CaseInfo.ValidWidth - 1].Discr;
		int ValidValue = 0;
		for (int j = 0; j < CaseInfo.ValidWidth - 1; j++) {
			assert(CaseInfo.ValidAttrs[j].AttType != ATT_DISCRETE);
			// {
			// 	throw(CError("SVM: discrete attribute should have been expanded!\n",100,0));
			// }
			// else//range expanding
			{
				if (TrainData[i][j].Cont == 0)
					continue;
				else if (CaseInfo.ValidAttrs[j].Max == CaseInfo.ValidAttrs[j].Min)
					continue;
				else {
					DataDesc.x[i][ValidValue].index = j + 1;
					DataDesc.x[i][ValidValue].value = (TrainData[i][j].Cont - CaseInfo.ValidAttrs[j].Min) /
													  (CaseInfo.ValidAttrs[j].Max - CaseInfo.ValidAttrs[j].Min);
					ValidValue++;
				}
			}
		}
		// tag for end of line has been set
	}

	if (!OrgSet.AllContinuous())
		delete TrainSet;
	return;
}

inline bool CSVM::Dump(const std::string &FileName) const
{
	svm_save_model(FileName.c_str(), svm);
	return true;
}

inline CPrediction *CSVM::Classify(const CDataset &OrgSet) const
{
	// start time for training
	clock_t Start = clock();

	// expand discrete attribute
	const CDataset *TrainSet = &OrgSet;
	if (!OrgSet.AllContinuous())
		TrainSet = OrgSet.ExpandDiscrete();
	const MATRIX &TrainData = TrainSet->GetData();
	const CASE_INFO &CaseInfo = TrainSet->GetInfo();

	// malloc space for result
	std::vector<double> Row(CaseInfo.ClassNum, 0);
	std::vector<std::vector<double>> Result(CaseInfo.Height, Row);

	// classifying
	struct svm_node Val = {-1, 0};
	// need an extra column for index = -1 (tag for end of line)
	new_d1(struct svm_node, data, CaseInfo.ValidWidth, Val);
	for (int i = 0; i < CaseInfo.Height; i++) {
		int ValidValue = 0;
		for (int j = 0; j < CaseInfo.ValidWidth - 1; j++) {
			assert(CaseInfo.ValidAttrs[j].AttType != ATT_DISCRETE);
			// {
			// 	throw(CError("SVM: discrete attribute should have been expanded!\n",100,0));
			// }
			// else//
			{
				if (TrainData[i][j].Cont == 0)
					continue;
				else if (CaseInfo.ValidAttrs[j].Max == CaseInfo.ValidAttrs[j].Min)
					continue;
				else {
					data[ValidValue].index = j + 1;
					data[ValidValue].value = (TrainData[i][j].Cont - CaseInfo.ValidAttrs[j].Min) /
											 (CaseInfo.ValidAttrs[j].Max - CaseInfo.ValidAttrs[j].Min);
					ValidValue++;
				}
			}
		}
		// tag of line end has been set

		// predicting
		int CaseClass = (int)svm_predict(svm, data);
		Result[i][CaseClass] = 1;
	}
	free_d1(data);

	if (!OrgSet.AllContinuous())
		delete TrainSet;
	return new CPrediction(OrgSet, Result, clock() - Start);
}

//
// int CSVM::Save(const string &Path,const string &FileName) const
// {
// 	svm_save_model((Path+FileName+"."+Name).c_str(),svm);
// 	return 0;
// }

// //must load DataDesc before use the model
// CSVM::CSVM(const string &Path,const string &FileName)
// {
// 	//start time for training
// 	Name=MyName;
// 	clock_t start=clock();

// 	svm=NULL;
// 	svm=svm_load_model((Path+FileName+"."+Name).c_str());
// 	if(svm==NULL)
// 	{
// 		throw(CError("SVM: error building from file!",100,2));
// 	}

// 	//time consumed
// 	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
// }

// bool CSVM::SaveDataDesc(const string &Path,const string &FileName) const
// {
// 	ofstream OutFile;
// 	OutFile.open((Path+FileName+"."+Name+".Datas").c_str(),ios_base::out|ios_base::binary|ios_base::trunc);
// 	if(OutFile.fail())
// 	{
// 		return false;
// 	}
//
// 	//number of attributes
// 	OutFile.write((char*)&AttributeNum,sizeof(AttributeNum));
// 	//number of instance
// 	OutFile.write((char*)&DataDesc.l,sizeof(DataDesc.l));
// 	//labels
// 	for(int i=0;i<DataDesc.l;i++)
// 	{
// 		double Tmp=DataDesc.y[i];
// 		OutFile.write((char*)&Tmp,sizeof(Tmp));
// 	}
// 	//instances
// 	for(int i=0;i<DataDesc.l;i++)
// 	{
// 		int j=-1;
// 		do
// 		{
// 			j++;
// 			struct svm_node tmp=DataDesc.x[i][j];
// 			OutFile.write((char*)&tmp,sizeof(tmp));
// 		} while(DataDesc.x[i][j].index!=-1);
// 	}
//
// 	OutFile.close();
// 	return true;
// }
//
// bool CSVM::LoadDataDesc(const string &Path,const string &FileName)
// {
// 	ifstream InFile;
// 	InFile.open((Path+FileName+"."+Name+".Datas").c_str(),ios_base::in|ios_base::binary);
// 	if(InFile.fail())
// 	{
// 		return false;
// 	}
//
// 	//number of attributes
// 	InFile.read((char*)&AttributeNum,sizeof(AttributeNum));
// 	if(InFile.fail()) return false;
// 	//number of instances
// 	InFile.read((char*)&DataDesc.l,sizeof(DataDesc.l));
// 	if(InFile.fail()) return false;
// 	DataDesc.y=new double[DataDesc.l];
// 	//labels
// 	for(int i=0;i<DataDesc.l;i++)
// 	{
// 		double Tmp;
// 		InFile.read((char*)&Tmp,sizeof(Tmp));
// 		if(InFile.fail()) return false;
// 		DataDesc.y[i]=Tmp;
// 	}
// 	//instances
// 	struct svm_node Val={-1,0};
// 	fill_d2(struct svm_node,DataDesc.x,DataDesc.l,AttributeNum+1,Val);
// 	for(int i=0;i<DataDesc.l;i++)
// 	{
// 		int j=-1;
// 		do
// 		{
// 			j++;
// 			struct svm_node Tmp;
// 			InFile.read((char*)&Tmp,sizeof(Tmp));
// 			if(InFile.fail()) return false;
// 			DataDesc.x[i][j]=Tmp;
// 		} while(DataDesc.x[i][j].index!=-1);
// 	}
//
// 	InFile.close();
// 	return true;
// }