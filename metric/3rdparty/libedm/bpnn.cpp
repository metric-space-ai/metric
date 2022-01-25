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
#include <fstream>
#include <iostream>
// using namespace std;
//#include "Obj.h"
#include "Classifier.h"
#include "DataSet.h"
#include "Prediction.h"
#include "bpnn.h"
//#include "DateTime.h"
using namespace libedm;

const char MyName[MAX_OBJECT_NAME_LENGTH] = "Bpnn";
const string CBpnn::StaticName = MyName;

CBpnn::~CBpnn()
{
	free_d2(Weight_IN_HD, HdNode);
	free_d1(Bias_HD);
	free_d2(Weight_HD_OUT, OutNode);
	free_d1(Bias_OUT);
}

// create a BPNN with random weights
CBpnn::CBpnn(int UInNode, int UOutNode, int UHideNode)
{
	// start time for training
	clock_t start = clock();
	Name = MyName;

	if (UHideNode <= 0 || UInNode <= 0 || UOutNode <= 0) {
		throw(CError("BPNN: wrong structure setting!", 100, 0));
	}

	// 3-layer network
	InNode = UInNode;
	OutNode = UOutNode;
	HdNode = UHideNode;
	//����Ȩ��
	new_d2(double, Weight_IN_HD, HdNode, InNode, (rand() * 2.0 / RAND_MAX - 1) / 8);
	//�����ڵ��ڲ�ƫ��
	new_d1(double, Bias_HD, HdNode, (rand() * 2.0 / RAND_MAX - 1) / 8);
	new_d2(double, Weight_HD_OUT, OutNode, HdNode, (rand() * 2.0 / RAND_MAX - 1) / 8);
	new_d1(double, Bias_OUT, OutNode, (rand() * 2.0 / RAND_MAX - 1) / 8);

	this->Bias_HD = Bias_HD;
	this->Bias_OUT = Bias_OUT;
	this->Weight_HD_OUT = Weight_HD_OUT;
	this->Weight_IN_HD = Weight_IN_HD;

	// time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;

	return;
}

// create BPNN through momentum
// Momentum- momentum coefficient,LearnRate- learning rate
// MaxEpoch- max epoch of training
// MinMSE- minimum MSE of stopping
CBpnn::CBpnn(const CDataset &OrgSet, double Momentum, double LearnRate, double MinMSE, int MaxEpoch, int UHideNode)
{
	// start time for training
	clock_t start = clock();
	Name = MyName;

	// expand discrete attribute
	const CDataset *TrainSet = &OrgSet;
	if (!OrgSet.AllContinuous())
		TrainSet = OrgSet.ExpandDiscrete();
	const MATRIX &TrainData = TrainSet->GetData();
	const CASE_INFO &CaseInfo = TrainSet->GetInfo();

	int n_samples, target;
	// number of training instances
	n_samples = CaseInfo.Height;
	// label of an instance is at the last position
	target = CaseInfo.ValidWidth - 1;
	// 3-layers network
	InNode = CaseInfo.ValidWidth - 1;
	OutNode = CaseInfo.ClassNum;
	if (UHideNode <= 0) {
		HdNode = CaseInfo.ValidWidth - 1;
	} else
		HdNode = UHideNode;

	// Allocate space for temporary parameters
	// input layer
	new_d1(double, Output_IN, InNode, 0);
	// hiden layer
	new_d1(double, Output_HD, HdNode, 0);
	// output layer
	new_d1(double, Output_OUT, OutNode, 0);
	//�����Ȩ��ƫ��
	new_d1(double, Alpha_Weight_HD, HdNode, 0);
	//������Ȩ��ƫ��
	new_d1(double, Alpha_Weight_OUT, OutNode, 0);
	// ideal output of network
	new_d1(double, Ideal_OUT, OutNode, 0);

	//Ȩ�ظı���
	new_d2(double, Delta_Weight_HD, HdNode, InNode, 0);
	new_d2(double, Delta_Weight_OUT, OutNode, HdNode, 0);
	new_d1(double, Delta_Bias_HD, HdNode, 0);
	new_d1(double, Delta_Bias_OUT, OutNode, 0);

	//������: Allocate space for temp weight
	new_d2(double, Temp_Weight_HD, HdNode, InNode, 0);
	new_d2(double, Temp_Weight_OUT, OutNode, HdNode, 0);
	new_d1(double, Temp_Bias_HD, HdNode, 0);
	new_d1(double, Temp_Bias_OUT, OutNode, 0);

	// Initialize weight and bias randomly
	//     srand( (unsigned)time( NULL ) );
	//����Ȩ��
	new_d2(double, Weight_IN_HD, HdNode, InNode, (rand() * 2.0 / RAND_MAX - 1) / 8);
	//�����ڵ��ڲ�ƫ��
	new_d1(double, Bias_HD, HdNode, (rand() * 2.0 / RAND_MAX - 1) / 8);
	new_d2(double, Weight_HD_OUT, OutNode, HdNode, (rand() * 2.0 / RAND_MAX - 1) / 8);
	new_d1(double, Bias_OUT, OutNode, (rand() * 2.0 / RAND_MAX - 1) / 8);

	//��dataתΪ������ʽ�����٣�
	//	//vector into array: to accelerate
	//	new_d2(ValueData,Data,n_samples,target+1,TrainData[i][j]);

	//���ͳ��
	MSE = 100.0;
	//ѧϰ����
	epoch = 0;
	//��ʱ����
	double Net_HD, Net_OUT, sum;
	while (epoch < MaxEpoch && MSE > MinMSE) {
		// Initialization
		for (int i = 0; i < HdNode; i++) {
			for (int j = 0; j < InNode; j++)
				Delta_Weight_HD[i][j] = 0;
			Delta_Bias_HD[i] = 0;
		}
		for (int i = 0; i < OutNode; i++) {
			for (int j = 0; j < HdNode; j++)
				Delta_Weight_OUT[i][j] = 0;
			Delta_Bias_OUT[i] = 0;
		}

		MSE = 0;
		// using epoch training method
		for (int k = 0; k < n_samples; k++) {
			// innode��Ŀ����������Ŀ, ��������ֵ��Χ��չ
			// number of input nodes is that of attributes
			for (int i = 0; i < InNode; i++) {
				if (CaseInfo.ValidAttrs[i].Max == CaseInfo.ValidAttrs[i].Min)
					Output_IN[i] = 0;
				else
					Output_IN[i] = (TrainData[k][i].Cont - CaseInfo.ValidAttrs[i].Min) /
								   (CaseInfo.ValidAttrs[i].Max - CaseInfo.ValidAttrs[i].Min);
			}
			//��ǰ����
			// forward propagate
			// hidden layer
			for (int j = 0; j < HdNode; j++) {
				sum = 0;
				for (int i = 0; i < InNode; i++)
					sum = sum + Weight_IN_HD[j][i] * Output_IN[i];
				Net_HD = sum + Bias_HD[j];
				Output_HD[j] = sigmoid(Net_HD);
			}
			// output layer
			for (int j = 0; j < OutNode; j++) {
				sum = 0;
				for (int i = 0; i < HdNode; i++)
					sum = sum + Weight_HD_OUT[j][i] * Output_HD[i];
				Net_OUT = sum + Bias_OUT[j];
				Output_OUT[j] = sigmoid(Net_OUT);
			}
			//���������: ѵ������ʵ������Ӧ�����Ϊ1, �������Ϊ0
			// ideal output
			for (int i = 0; i < OutNode; i++)
				Ideal_OUT[i] = 0;
			// label is unknown, skip it
			if (TrainData[k][target].Discr == -1)
				continue;
			int label = TrainData[k][target].Discr;
			Ideal_OUT[label] = 1;
			// Sum of error of all output nodes
			for (int i = 0; i < OutNode; i++) {
				MSE = MSE + (Ideal_OUT[i] - Output_OUT[i]) * (Ideal_OUT[i] - Output_OUT[i]) / 2.0;
				//����ڵ�Ŀ����MSEΪ0, ��Ȩ�������MSE������ƫ��
				Alpha_Weight_OUT[i] = (Ideal_OUT[i] - Output_OUT[i]) * Output_OUT[i] * (1 - Output_OUT[i]);
			}

			//����, ��¼�����Ȩ���ݶȣ�Error backward propagation��
			for (int i = 0; i < HdNode; i++) {
				sum = 0;
				//����������Ŀ��, ���������ڵ��������Ŀ��
				for (int j = 0; j < OutNode; j++)
					sum = sum + Alpha_Weight_OUT[j] * Weight_HD_OUT[j][i];
				//�����ڵ�: ��Ȩ�������MSE������ƫ��
				Alpha_Weight_HD[i] = Output_HD[i] * (1 - Output_HD[i]) * sum;
			}
			// Add the Delta weight and bias of one class for output layer
			// Delta_weight_**��������ƫ��ֵ�����ű��ˣ���Ҫ��ȥƫ��ֵ��
			for (int i = 0; i < OutNode; i++) {
				//�޸�delta: �������Ȩ��
				for (int j = 0; j < HdNode; j++) {
					Delta_Weight_OUT[i][j] = Delta_Weight_OUT[i][j] + Alpha_Weight_OUT[i] * Output_HD[j];
				}
				//�޸�delta: ��������ڲ�ƫ��
				Delta_Bias_OUT[i] = Delta_Bias_OUT[i] + Alpha_Weight_OUT[i];
			}
			// Add the Delta weight and bias of one class for hidden layer
			for (int i = 0; i < HdNode; i++) {
				//������Ȩ���޸�delta
				for (int j = 0; j < InNode; j++) {
					Delta_Weight_HD[i][j] = Delta_Weight_HD[i][j] + Alpha_Weight_HD[i] * Output_IN[j];
				}
				Delta_Bias_HD[i] = Delta_Bias_HD[i] + Alpha_Weight_HD[i];
			}
		} // end for k

		//ѧ����������, �޸�Ȩ��
		// add to the new weight change a small proportion of the update value from the previous iteration
		//�޸�Ȩ�ص�ʱ�򣬼���һ�����ϴε�Ȩ�ظ���ֵ��Temp_**������ʹ�÷���仯��Ҫ̫ͻȻ
		for (int i = 0; i < OutNode; i++) {
			for (int j = 0; j < HdNode; j++) {
				Weight_HD_OUT[i][j] = Weight_HD_OUT[i][j] + LearnRate * Delta_Weight_OUT[i][j] / n_samples +
									  Momentum * Temp_Weight_OUT[i][j];
			}
			Bias_OUT[i] = Bias_OUT[i] + LearnRate * Delta_Bias_OUT[i] / n_samples + Momentum * Temp_Bias_OUT[i];
		}
		for (int i = 0; i < HdNode; i++) {
			for (int j = 0; j < InNode; j++) {
				Weight_IN_HD[i][j] = Weight_IN_HD[i][j] + LearnRate * Delta_Weight_HD[i][j] / n_samples +
									 Momentum * Temp_Weight_HD[i][j];
			}
			Bias_HD[i] = Bias_HD[i] + LearnRate * Delta_Bias_HD[i] / n_samples + Momentum * Temp_Bias_HD[i];
		}

		//������: save momentum value
		// momentum value: a small proportion of the update value from the previous iteration
		for (int i = 0; i < OutNode; i++) {
			for (int j = 0; j < HdNode; j++) {
				Temp_Weight_OUT[i][j] =
					LearnRate * Delta_Weight_OUT[i][j] / n_samples + Momentum * Temp_Weight_OUT[i][j];
			}
			Temp_Bias_OUT[i] = LearnRate * Delta_Bias_OUT[i] / n_samples + Momentum * Temp_Bias_OUT[i];
		}
		for (int i = 0; i < HdNode; i++) {
			for (int j = 0; j < InNode; j++) {
				Temp_Weight_HD[i][j] = LearnRate * Delta_Weight_HD[i][j] / n_samples + Momentum * Temp_Weight_HD[i][j];
			}
			Temp_Bias_HD[i] = LearnRate * Delta_Bias_HD[i] / n_samples + Momentum * Temp_Bias_HD[i];
		}

		MSE = MSE / (n_samples * OutNode);
		epoch = epoch + 1;
		if (epoch % 200 == 0)
			cout << epoch << ":" << MSE << endl;
	} // end for epoch

	free_d1(Output_IN);
	free_d1(Output_HD);
	free_d1(Output_OUT);
	free_d1(Alpha_Weight_HD);
	free_d1(Alpha_Weight_OUT);
	free_d1(Ideal_OUT);

	// Allocate space for delta weight
	free_d2(Delta_Weight_HD, HdNode);
	free_d2(Delta_Weight_OUT, OutNode);
	free_d1(Delta_Bias_HD);
	free_d1(Delta_Bias_OUT);

	// Allocate space for temp weight
	free_d2(Temp_Weight_HD, HdNode);
	free_d2(Temp_Weight_OUT, OutNode);
	free_d1(Temp_Bias_HD);
	free_d1(Temp_Bias_OUT);
	//	free_d2(Data,n_samples);

	this->Bias_HD = Bias_HD;
	this->Bias_OUT = Bias_OUT;
	this->Weight_HD_OUT = Weight_HD_OUT;
	this->Weight_IN_HD = Weight_IN_HD;

	// time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;

	if (!OrgSet.AllContinuous())
		delete TrainSet;
	return;
}

bool CBpnn::Dump(const string &FileName) const
{
	ofstream OutFile;
	OutFile.open(FileName.c_str(), ios_base::out | ios_base::binary | ios_base::trunc);
	OutFile << "InNode=" << InNode << endl
			<< "HideNode=" << HdNode << endl
			<< "OutNode=" << OutNode << endl
			<< "MSE=" << MSE << endl
			<< "epoch=" << epoch << endl
			<< endl;

	for (int i = 0; i < HdNode; i++)
		for (int j = 0; j < InNode; j++)
			OutFile << "Weight_in_hide[" << i << "][" << j << "]=" << Weight_IN_HD[i][j] << endl;
	OutFile << endl;

	for (int i = 0; i < OutNode; i++)
		for (int j = 0; j < HdNode; j++)
			OutFile << "Weight_hide_out[" << i << "][" << j << "]=" << Weight_HD_OUT[i][j] << endl;
	OutFile << endl;

	for (int j = 0; j < HdNode; j++)
		OutFile << "Bias_hide[" << j << "]" << Bias_HD[j] << endl;
	OutFile << endl;

	for (int j = 0; j < OutNode; j++)
		OutFile << "Bias_out[" << j << "]" << Bias_OUT[j] << endl;
	OutFile << endl;

	OutFile.close();
	return true;
}

int CBpnn::Save(const string &Path, const string &FileName) const
{
	ofstream OutFile;
	OutFile.open((Path + FileName + "." + Name).c_str(), ios_base::out | ios_base::trunc | ios_base::binary);
	if (OutFile.fail()) {
		return 1;
	}

	OutFile.write((char *)&epoch, sizeof(epoch));
	OutFile.write((char *)&MSE, sizeof(MSE));
	OutFile.write((char *)&InNode, sizeof(InNode));
	OutFile.write((char *)&HdNode, sizeof(HdNode));
	OutFile.write((char *)&OutNode, sizeof(OutNode));

	// we save a 2d array into an 1d array
	double *temp = new double[InNode * HdNode];
	for (int i = 0; i < HdNode; i++)
		for (int j = 0; j < InNode; j++)
			temp[InNode * i + j] = Weight_IN_HD[i][j];
	OutFile.write((char *)temp, HdNode * InNode * sizeof(double));
	delete[] temp;

	temp = new double[OutNode * HdNode];
	for (int i = 0; i < OutNode; i++)
		for (int j = 0; j < HdNode; j++)
			temp[HdNode * i + j] = Weight_HD_OUT[i][j];
	OutFile.write((char *)temp, OutNode * HdNode * sizeof(double));
	delete[] temp;

	OutFile.write((char *)Bias_HD, HdNode * sizeof(double));
	OutFile.write((char *)Bias_OUT, OutNode * sizeof(double));

	OutFile.close();
	return 0;
}

CBpnn::CBpnn(const string &Path, const string &FileName)
{
	// start time for training
	clock_t start = clock();
	Name = MyName;

	ifstream InFile;
	InFile.open((Path + FileName + "." + Name).c_str(), ios_base::in | ios_base::binary);
	if (InFile.fail()) {
		throw(CError("BPNN: fail open saving file!", 101, 0));
	}

	// read structure of network
	InFile.read((char *)&epoch, sizeof(epoch));
	if (InFile.fail())
		throw(CError("BPNN: read parameter error!", 102, 0));
	InFile.read((char *)&MSE, sizeof(MSE));
	if (InFile.fail())
		throw(CError("BPNN: read parameter error!", 103, 0));
	InFile.read((char *)&InNode, sizeof(InNode));
	if (InFile.fail())
		throw(CError("BPNN: read parameter error!", 104, 0));
	InFile.read((char *)&HdNode, sizeof(HdNode));
	if (InFile.fail())
		throw(CError("BPNN: read parameter error!", 105, 0));
	InFile.read((char *)&OutNode, sizeof(OutNode));
	if (InFile.fail())
		throw(CError("BPNN: read parameter error!", 106, 0));

	// read weights for each layer
	double *temp = new double[HdNode * InNode];
	InFile.read((char *)temp, HdNode * InNode * sizeof(double));
	if (InFile.fail())
		throw(CError("BPNN: read parameter error!", 107, 0));
	// 1d array transformed into 2d
	fill_d2(double, Weight_IN_HD, HdNode, InNode, temp[InNode * i + j]);
	delete[] temp;

	temp = new double[OutNode * HdNode];
	InFile.read((char *)temp, OutNode * HdNode * sizeof(double));
	if (InFile.fail())
		throw(CError("BPNN: read parameter error!", 108, 0));
	fill_d2(double, Weight_HD_OUT, OutNode, HdNode, temp[HdNode * i + j]);
	delete[] temp;

	Bias_HD = new double[HdNode];
	InFile.read((char *)Bias_HD, HdNode * sizeof(double));
	if (InFile.fail())
		throw(CError("BPNN: read parameter error!", 109, 0));
	Bias_OUT = new double[OutNode];
	InFile.read((char *)Bias_OUT, OutNode * sizeof(double));
	if (InFile.fail())
		throw(CError("BPNN: read parameter error!", 110, 0));

	InFile.close();

	// time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

CPrediction *CBpnn::Classify(const CDataset &OrgSet) const
{
	// start time for classify
	clock_t Start = clock();

	// expand discrete attribute
	const CDataset *TrainSet = &OrgSet;
	if (!OrgSet.AllContinuous())
		TrainSet = OrgSet.ExpandDiscrete();
	const MATRIX &Data = TrainSet->GetData();
	const CASE_INFO &CaseInfo = TrainSet->GetInfo();
	if (CaseInfo.ClassNum != OutNode || CaseInfo.ValidWidth - 1 != InNode)
		throw(CError("BPNN: data set don't match this classifier!", 111, 0));

	// malloc
	vector<double> Output_IN(InNode, 0);
	vector<double> Output_HD(HdNode, 0);
	vector<double> Output_OUT(OutNode, 0);
	double Net_HD, Net_OUT, Sum;
	// malloc space for result
	vector<double> Line(CaseInfo.ClassNum, 0);
	std::vector<std::vector<double>> Result(CaseInfo.Height, Line);

	// Begin classifying
	for (int m = 0; m < CaseInfo.Height; m++) {
		// extend continuous values
		for (int i = 0; i < InNode; i++) {
			if (CaseInfo.ValidAttrs[i].Max == CaseInfo.ValidAttrs[i].Min)
				Output_IN[i] = 0;
			else
				Output_IN[i] = (Data[m][i].Cont - CaseInfo.ValidAttrs[i].Min) /
							   (CaseInfo.ValidAttrs[i].Max - CaseInfo.ValidAttrs[i].Min);
		}

		// hidden layer
		for (int j = 0; j < HdNode; j++) {
			Sum = 0;
			for (int i = 0; i < InNode; i++)
				Sum = Sum + Weight_IN_HD[j][i] * Output_IN[i];
			Net_HD = Sum + Bias_HD[j];
			Output_HD[j] = sigmoid(Net_HD);
		}

		// output layer
		for (int j = 0; j < OutNode; j++) {
			Sum = 0;
			for (int i = 0; i < HdNode; i++)
				Sum = Sum + Weight_HD_OUT[j][i] * Output_HD[i];
			Net_OUT = Sum + Bias_OUT[j];
			Result[m][j] = sigmoid(Net_OUT);
		}

		// normalize the (*result)
		Sum = 0;
		for (int i = 0; i < OutNode; i++)
			Sum += Result[m][i];
		for (int i = 0; i < OutNode; i++)
			Result[m][i] /= Sum;
	}

	if (!OrgSet.AllContinuous())
		delete TrainSet;
	return (new CPrediction(OrgSet, Result, clock() - Start));
}

double CBpnn::sigmoid(double u) const
{
	double su;
	su = 1 / (1 + exp(-u));
	return (su);
}

int CBpnn::sgn(double a) const
{
	if (a > 0)
		return 1;
	else if (a < 0)
		return -1;
	else
		return 0;
}

// return delta for weights of the network
// LastDeriv-(�ϴε���)last derivation,Deriv-(���ε���)derivation of this iteration,Value-delta of weight(absolute
// value),LastDelta-
double CBpnn::delta(double &LastDeriv, double Deriv, double &Value, double &LastDelta) const
{
	// RPROP: (Ȩ��ֵ�ı�����)ratio to change the weight
	double eta_p = 1.2;
	double eta_n = 0.5;
	// RPROP: (Ȩ��ֵ������)maximum and minimum weight
	double max_eta = 50;
	double min_eta = 1e-6;

	//�ƽ��ٶȵ���
	Deriv = (-1) * Deriv;
	double Delta = LastDeriv * Deriv;
	//��������δ�䣨�������򲻱䣩, �����ٶ�
	// signal of derivation doesn't change, so accelerate
	if (Delta > 0) {
		Value *= eta_p;
		if (Value > max_eta)
			Value = max_eta;
	}
	//�������Ÿı�䣨����������Ҫ�ı䣩, ��С�ٶ�
	// signal of derivation changed, so decelerate and the speed need not change at next step
	else if (Delta < 0) {
		Value *= eta_n;
		if (Value < min_eta)
			Value = min_eta;
		//���ŷ�ת, ��һ�β������ٶ�
		Deriv = 0;
	}
	//����Ϊ0, �ٶȲ���
	else {
	}

	//��¼�ϴε���
	LastDeriv = Deriv;

	//���Ƿ���
	// switch the direction
	if (Delta < 0) {
		Delta = (-1) * LastDelta;
	} else {
		Delta = (-1) * sgn(Deriv) * Value;
	}

	LastDelta = Delta;
	return Delta;
}

// RPROP
CBpnn::CBpnn(const CDataset &OrgSet, double MinMSE, int MaxEpoch, int UHideNode)
{
	// start time for training
	clock_t start = clock();
	Name = MyName;

	// expand discrete attribute
	const CDataset *TrainSet = &OrgSet;
	if (!OrgSet.AllContinuous())
		TrainSet = OrgSet.ExpandDiscrete();
	const MATRIX &TrainData = TrainSet->GetData();
	const CASE_INFO &CaseInfo = TrainSet->GetInfo();

	int n_samples, target;
	// number of training instances
	n_samples = CaseInfo.Height;
	// lable of an instance is at the last position
	target = CaseInfo.ValidWidth - 1;
	// 3-layers network
	InNode = CaseInfo.ValidWidth - 1;
	OutNode = CaseInfo.ClassNum;
	if (UHideNode <= 0) {
		HdNode = CaseInfo.ValidWidth - 1;
	} else
		HdNode = UHideNode;

	// Allocate space for temporary parameters
	// input layer
	new_d1(double, Output_IN, InNode, 0);
	// hiden layer
	new_d1(double, Output_HD, HdNode, 0);
	// output layer
	new_d1(double, Output_OUT, OutNode, 0);
	//�����Ȩ��ƫ��
	new_d1(double, Alpha_Weight_HD, HdNode, 0);
	//������Ȩ��ƫ��
	new_d1(double, Alpha_Weight_OUT, OutNode, 0);
	// ideal output of network
	new_d1(double, Ideal_OUT, OutNode, 0);

	//ÿ��epoch: MSE���������Ȩ�ء�bias�ĵ���
	new_d2(double, Delta_Weight_HD, HdNode, InNode, 0);
	new_d2(double, Delta_Weight_OUT, OutNode, HdNode, 0);
	new_d1(double, Delta_Bias_HD, HdNode, 0);
	new_d1(double, Delta_Bias_OUT, OutNode, 0);
	// RPROP: ��¼�ϴεĵ���
	new_d2(double, Temp_Weight_HD, HdNode, InNode, 0);
	new_d2(double, Temp_Weight_OUT, OutNode, HdNode, 0);
	new_d1(double, Temp_Bias_HD, HdNode, 0);
	new_d1(double, Temp_Bias_OUT, OutNode, 0);

	// Initialize weight and bias randomly
	// srand( (unsigned)time( NULL ) );
	//����Ȩ��
	new_d2(double, Weight_IN_HD, HdNode, InNode, (rand() * 2.0 / RAND_MAX - 1) / 8);
	//�����ڵ��ڲ�ƫ��
	new_d1(double, Bias_HD, HdNode, (rand() * 2.0 / RAND_MAX - 1) / 8);
	new_d2(double, Weight_HD_OUT, OutNode, HdNode, (rand() * 2.0 / RAND_MAX - 1) / 8);
	new_d1(double, Bias_OUT, OutNode, (rand() * 2.0 / RAND_MAX - 1) / 8);

	//��dataתΪ������ʽ�����٣�
	//	//vector into array: to accelerate
	//	new_d2(ValueData,Data,n_samples,target+1,TrainData[i][j]);

	//����ֵ�ĳ�ʼֵ
	double DeltaVal = 0.1;
	// RPROP: ����ֵ
	new_d2(double, Delta_WeightV_HD, HdNode, InNode, DeltaVal);
	new_d2(double, Delta_WeightV_OUT, OutNode, HdNode, DeltaVal);
	new_d1(double, Delta_BiasV_HD, HdNode, DeltaVal);
	new_d1(double, Delta_BiasV_OUT, OutNode, DeltaVal);
	// RPROP: �ϴε�Delta
	new_d2(double, Last_Delta_Weight_HD, HdNode, InNode, 0);
	new_d2(double, Last_Delta_Weight_OUT, OutNode, HdNode, 0);
	new_d1(double, Last_Delta_Bias_HD, HdNode, 0);
	new_d1(double, Last_Delta_Bias_OUT, OutNode, 0);

	//���ͳ��
	MSE = 100.0;
	//���������ݼ���ѧϰ����
	epoch = 0;
	//��ʱ����
	double Net_HD, Net_OUT, sum;
	while (epoch < MaxEpoch && MSE > MinMSE) {
		// Initialization
		for (int i = 0; i < HdNode; i++) {
			for (int j = 0; j < InNode; j++)
				Delta_Weight_HD[i][j] = 0;
			Delta_Bias_HD[i] = 0;
		}
		for (int i = 0; i < OutNode; i++) {
			for (int j = 0; j < HdNode; j++)
				Delta_Weight_OUT[i][j] = 0;
			Delta_Bias_OUT[i] = 0;
		}

		MSE = 0;
		// using epoch training method
		for (int k = 0; k < n_samples; k++) {
			// innode��Ŀ����������Ŀ, ��������ֵ��Χ��չ
			for (int i = 0; i < InNode; i++) {
				if (CaseInfo.ValidAttrs[i].Max == CaseInfo.ValidAttrs[i].Min)
					Output_IN[i] = 0;
				else
					Output_IN[i] = (TrainData[k][i].Cont - CaseInfo.ValidAttrs[i].Min) /
								   (CaseInfo.ValidAttrs[i].Max - CaseInfo.ValidAttrs[i].Min);
			}
			//��ǰ����
			// hidden layer
			for (int j = 0; j < HdNode; j++) {
				sum = 0;
				for (int i = 0; i < InNode; i++)
					sum = sum + Weight_IN_HD[j][i] * Output_IN[i];
				Net_HD = sum + Bias_HD[j];
				Output_HD[j] = sigmoid(Net_HD);
			}
			// output layer
			for (int j = 0; j < OutNode; j++) {
				sum = 0;
				for (int i = 0; i < HdNode; i++)
					sum = sum + Weight_HD_OUT[j][i] * Output_HD[i];
				Net_OUT = sum + Bias_OUT[j];
				Output_OUT[j] = sigmoid(Net_OUT);
			}
			//���������: ѵ������ʵ������Ӧ�����Ϊ1, �������Ϊ0
			for (int i = 0; i < OutNode; i++)
				Ideal_OUT[i] = 0;
			// label is unknown, skip it
			if (TrainData[k][target].Discr == -1)
				continue;
			int label = TrainData[k][target].Discr;
			Ideal_OUT[label] = 1;
			// Sum of error of all output nodes
			for (int i = 0; i < OutNode; i++) {
				MSE = MSE + (Ideal_OUT[i] - Output_OUT[i]) * (Ideal_OUT[i] - Output_OUT[i]) / 2.0;
				//����ڵ�Ŀ����MSEΪ0, ��Ȩ�������MSE������ƫ��
				Alpha_Weight_OUT[i] = (Ideal_OUT[i] - Output_OUT[i]) * Output_OUT[i] * (1 - Output_OUT[i]);
			}

			//����, ��¼�����Ȩ���ݶȣ�Error backword propagation��
			for (int i = 0; i < HdNode; i++) {
				sum = 0;
				//����������Ŀ��, ���������ڵ��������Ŀ��
				for (int j = 0; j < OutNode; j++)
					sum = sum + Alpha_Weight_OUT[j] * Weight_HD_OUT[j][i];
				//�����ڵ�: ��Ȩ�������MSE������ƫ��
				Alpha_Weight_HD[i] = Output_HD[i] * (1 - Output_HD[i]) * sum;
			}

			// Add the Delta weight and bias of one class for output layer
			for (int i = 0; i < OutNode; i++) {
				//�޸�delta: �������Ȩ��
				for (int j = 0; j < HdNode; j++) {
					Delta_Weight_OUT[i][j] = Delta_Weight_OUT[i][j] + Alpha_Weight_OUT[i] * Output_HD[j];
				}
				//�޸�delta: ��������ڲ�ƫ��
				Delta_Bias_OUT[i] = Delta_Bias_OUT[i] + Alpha_Weight_OUT[i];
			}
			// Add the Delta weight and bias of one class for hidden layer
			for (int i = 0; i < HdNode; i++) {
				//������Ȩ���޸�delta
				for (int j = 0; j < InNode; j++) {
					Delta_Weight_HD[i][j] = Delta_Weight_HD[i][j] + Alpha_Weight_HD[i] * Output_IN[j];
				}
				Delta_Bias_HD[i] = Delta_Bias_HD[i] + Alpha_Weight_HD[i];
			}
		} // end for k

		//ѧ����������, �޸�Ȩ��, ���м�¼ԭƫ��ֵ
		for (int i = 0; i < OutNode; i++) {
			for (int j = 0; j < HdNode; j++) {
				Weight_HD_OUT[i][j] += delta(Temp_Weight_OUT[i][j], Delta_Weight_OUT[i][j] / n_samples,
											 Delta_WeightV_OUT[i][j], Last_Delta_Weight_OUT[i][j]);
			}
			Bias_OUT[i] +=
				delta(Temp_Bias_OUT[i], Delta_Bias_OUT[i] / n_samples, Delta_BiasV_OUT[i], Last_Delta_Bias_OUT[i]);
		}
		for (int i = 0; i < HdNode; i++) {
			for (int j = 0; j < InNode; j++) {
				Weight_IN_HD[i][j] += delta(Temp_Weight_HD[i][j], Delta_Weight_HD[i][j] / n_samples,
											Delta_WeightV_HD[i][j], Last_Delta_Weight_HD[i][j]);
			}
			Bias_HD[i] +=
				delta(Temp_Bias_HD[i], Delta_Bias_HD[i] / n_samples, Delta_BiasV_HD[i], Last_Delta_Bias_HD[i]);
		}

		MSE = MSE / (n_samples * OutNode);
		epoch = epoch + 1;
		// 		if(epoch%200==0)
		// 			cout<<CDateTime::Now().FormatDateTime()<<": epoch "<<epoch<<" finished!"<<endl;
	} // end for epoch

	free_d1(Output_IN);
	free_d1(Output_HD);
	free_d1(Output_OUT);
	free_d1(Alpha_Weight_HD);
	free_d1(Alpha_Weight_OUT);
	free_d1(Ideal_OUT);

	// Allocate space for delta weight
	free_d2(Delta_Weight_HD, HdNode);
	free_d2(Delta_Weight_OUT, OutNode);
	free_d1(Delta_Bias_HD);
	free_d1(Delta_Bias_OUT);

	// Allocate space for temp weight
	free_d2(Temp_Weight_HD, HdNode);
	free_d2(Temp_Weight_OUT, OutNode);
	free_d1(Temp_Bias_HD);
	free_d1(Temp_Bias_OUT);
	free_d2(Delta_WeightV_HD, HdNode);
	free_d2(Delta_WeightV_OUT, OutNode);
	free_d1(Delta_BiasV_HD);
	free_d1(Delta_BiasV_OUT);
	free_d2(Last_Delta_Weight_HD, HdNode);
	free_d2(Last_Delta_Weight_OUT, OutNode);
	free_d1(Last_Delta_Bias_HD);
	free_d1(Last_Delta_Bias_OUT);

	//	free_d2(Data,n_samples);

	this->Bias_HD = Bias_HD;
	this->Bias_OUT = Bias_OUT;
	this->Weight_HD_OUT = Weight_HD_OUT;
	this->Weight_IN_HD = Weight_IN_HD;

	// time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;

	if (!OrgSet.AllContinuous())
		delete TrainSet;
	return;
}

// Train a NCL on an existed BPNN through momentum method
// AveragePrediction-Average predictions of all other classifiers
// M-Number of other classifiers
// Gamma- used to adjust the strength of the penalty and it is problem-dependent
int CBpnn::MTNCLTrain(const CDataset &OrgSet, const CPrediction &AveragePrediction, int M, double Gamma, int MaxEpoch,
					  double MinMSE)
{
	// start time for training
	clock_t start = clock();
	Name = MyName;

	// expand discrete attribute
	const CDataset *TrainSet = &OrgSet;
	if (!OrgSet.AllContinuous())
		TrainSet = OrgSet.ExpandDiscrete();
	const MATRIX &TrainData = TrainSet->GetData();
	const CASE_INFO &CaseInfo = TrainSet->GetInfo();

	int n_samples, target;
	// number of training instances
	n_samples = CaseInfo.Height;
	// lable of an instance is at the last position
	target = CaseInfo.ValidWidth - 1;

	// Allocate space for temporary parameters
	// input layer
	new_d1(double, Output_IN, InNode, 0);
	// hiden layer
	new_d1(double, Output_HD, HdNode, 0);
	// output layer
	new_d1(double, Output_OUT, OutNode, 0);
	//�����Ȩ��ƫ��
	new_d1(double, Alpha_Weight_HD, HdNode, 0);
	//������Ȩ��ƫ��
	new_d1(double, Alpha_Weight_OUT, OutNode, 0);
	// ideal output of network
	new_d1(double, Ideal_OUT, OutNode, 0);

	//Ȩ�ظı���
	new_d2(double, Delta_Weight_HD, HdNode, InNode, 0);
	new_d2(double, Delta_Weight_OUT, OutNode, HdNode, 0);
	new_d1(double, Delta_Bias_HD, HdNode, 0);
	new_d1(double, Delta_Bias_OUT, OutNode, 0);

	//������: Allocate space for temp weight
	new_d2(double, Temp_Weight_HD, HdNode, InNode, 0);
	new_d2(double, Temp_Weight_OUT, OutNode, HdNode, 0);
	new_d1(double, Temp_Bias_HD, HdNode, 0);
	new_d1(double, Temp_Bias_OUT, OutNode, 0);

	//��dataתΪ������ʽ�����٣�
	//	//vector into array: to accelerate
	//	new_d2(ValueData,Data,n_samples,target+1,TrainData[i][j]);

	//������ϵ��
	// momentum coefficient
	const double Momentum = 0.9;
	//ѧϰ����
	// learning rate
	const double LearnRate = 0.5;
	//���ͳ��
	MSE = 100.0;
	//ѧϰ����
	epoch = 0;
	//��ʱ����
	double Net_HD, Net_OUT, sum;
	while (epoch < MaxEpoch && MSE > MinMSE) {
		/*Initialization*/
		for (int i = 0; i < HdNode; i++) {
			for (int j = 0; j < InNode; j++)
				Delta_Weight_HD[i][j] = 0;
			Delta_Bias_HD[i] = 0;
		}
		for (int i = 0; i < OutNode; i++) {
			for (int j = 0; j < HdNode; j++)
				Delta_Weight_OUT[i][j] = 0;
			Delta_Bias_OUT[i] = 0;
		}

		MSE = 0;
		/* using epoch training method*/
		for (int k = 0; k < n_samples; k++) {
			// innode��Ŀ����������Ŀ, ��������ֵ��Χ��չ
			// number of input nodes is that of attributes
			for (int i = 0; i < InNode; i++) {
				if (CaseInfo.ValidAttrs[i].Max == CaseInfo.ValidAttrs[i].Min)
					Output_IN[i] = 0;
				else
					Output_IN[i] = (TrainData[k][i].Cont - CaseInfo.ValidAttrs[i].Min) /
								   (CaseInfo.ValidAttrs[i].Max - CaseInfo.ValidAttrs[i].Min);
			}
			//��ǰ����
			// forward propagate
			// hidden layer
			for (int j = 0; j < HdNode; j++) {
				sum = 0;
				for (int i = 0; i < InNode; i++)
					sum = sum + Weight_IN_HD[j][i] * Output_IN[i];
				Net_HD = sum + Bias_HD[j];
				Output_HD[j] = sigmoid(Net_HD);
			}
			// output layer
			for (int j = 0; j < OutNode; j++) {
				sum = 0;
				for (int i = 0; i < HdNode; i++)
					sum = sum + Weight_HD_OUT[j][i] * Output_HD[i];
				Net_OUT = sum + Bias_OUT[j];
				Output_OUT[j] = sigmoid(Net_OUT);
			}
			//���������: ѵ������ʵ������Ӧ�����Ϊ1, �������Ϊ0
			// ideal output
			for (int i = 0; i < OutNode; i++)
				Ideal_OUT[i] = 0;
			// label is unknown, skip it
			if (TrainData[k][target].Discr == -1)
				continue;
			int Label = TrainData[k][target].Discr;
			Ideal_OUT[Label] = 1;
			// Sum of error of all output nodes
			const std::vector<std::vector<double>> &Probs = AveragePrediction.GetProbs();
			for (int i = 0; i < OutNode; i++) {
				double Fn = (Probs[k][i] * M + Output_OUT[i]) / (M + 1);
				MSE = MSE + (Ideal_OUT[i] - Output_OUT[i]) * (Ideal_OUT[i] - Output_OUT[i]) / 2.0 +
					  Gamma * (Output_OUT[i] - Fn) * M * (Probs[k][i] - Fn);
				//����ڵ�Ŀ����MSEΪ0, ��Ȩ�������MSE������ƫ��
				Alpha_Weight_OUT[i] =
					(Ideal_OUT[i] - Output_OUT[i] - Gamma * 2 * (1 - 1.0 / M) * (Fn - Output_OUT[i])) * Output_OUT[i] *
					(1 - Output_OUT[i]);
			}

			//����, ��¼�����Ȩ���ݶȣ�Error backward propagation��
			for (int i = 0; i < HdNode; i++) {
				sum = 0;
				//����������Ŀ��, ���������ڵ��������Ŀ��
				for (int j = 0; j < OutNode; j++)
					sum = sum + Alpha_Weight_OUT[j] * Weight_HD_OUT[j][i];
				//�����ڵ�: ��Ȩ�������MSE������ƫ��
				Alpha_Weight_HD[i] = Output_HD[i] * (1 - Output_HD[i]) * sum;
			}
			// Add the Delta weight and bias of one class for output layer
			for (int i = 0; i < OutNode; i++) {
				//�޸�delta: �������Ȩ��
				for (int j = 0; j < HdNode; j++) {
					Delta_Weight_OUT[i][j] = Delta_Weight_OUT[i][j] + Alpha_Weight_OUT[i] * Output_HD[j];
				}
				//�޸�delta: ��������ڲ�ƫ��
				Delta_Bias_OUT[i] = Delta_Bias_OUT[i] + Alpha_Weight_OUT[i];
			}
			// Add the Delta weight and bias of one class for hidden layer
			for (int i = 0; i < HdNode; i++) {
				//������Ȩ���޸�delta
				for (int j = 0; j < InNode; j++) {
					Delta_Weight_HD[i][j] = Delta_Weight_HD[i][j] + Alpha_Weight_HD[i] * Output_IN[j];
				}
				Delta_Bias_HD[i] = Delta_Bias_HD[i] + Alpha_Weight_HD[i];
			}
		} // end for k

		// finished all instances, modify weights
		for (int i = 0; i < OutNode; i++) {
			for (int j = 0; j < HdNode; j++) {
				Weight_HD_OUT[i][j] = Weight_HD_OUT[i][j] + LearnRate * Delta_Weight_OUT[i][j] / n_samples +
									  Momentum * Temp_Weight_OUT[i][j];
			}
			Bias_OUT[i] = Bias_OUT[i] + LearnRate * Delta_Bias_OUT[i] / n_samples + Momentum * Temp_Bias_OUT[i];
		}
		for (int i = 0; i < HdNode; i++) {
			for (int j = 0; j < InNode; j++) {
				Weight_IN_HD[i][j] = Weight_IN_HD[i][j] + LearnRate * Delta_Weight_HD[i][j] / n_samples +
									 Momentum * Temp_Weight_HD[i][j];
			}
			Bias_HD[i] = Bias_HD[i] + LearnRate * Delta_Bias_HD[i] / n_samples + Momentum * Temp_Bias_HD[i];
		}

		// save momentum value
		for (int i = 0; i < OutNode; i++) {
			for (int j = 0; j < HdNode; j++) {
				Temp_Weight_OUT[i][j] =
					LearnRate * Delta_Weight_OUT[i][j] / n_samples + Momentum * Temp_Weight_OUT[i][j];
			}
			Temp_Bias_OUT[i] = LearnRate * Delta_Bias_OUT[i] / n_samples + Momentum * Temp_Bias_OUT[i];
		}
		for (int i = 0; i < HdNode; i++) {
			for (int j = 0; j < InNode; j++) {
				Temp_Weight_HD[i][j] = LearnRate * Delta_Weight_HD[i][j] / n_samples + Momentum * Temp_Weight_HD[i][j];
			}
			Temp_Bias_HD[i] = LearnRate * Delta_Bias_HD[i] / n_samples + Momentum * Temp_Bias_HD[i];
		}

		MSE = MSE / (n_samples * OutNode);
		epoch = epoch + 1;
	} // end for epoch

	free_d1(Output_IN);
	free_d1(Output_HD);
	free_d1(Output_OUT);
	free_d1(Alpha_Weight_HD);
	free_d1(Alpha_Weight_OUT);
	free_d1(Ideal_OUT);

	// Allocate space for delta weight
	free_d2(Delta_Weight_HD, HdNode);
	free_d2(Delta_Weight_OUT, OutNode);
	free_d1(Delta_Bias_HD);
	free_d1(Delta_Bias_OUT);

	// Allocate space for temp weight
	free_d2(Temp_Weight_HD, HdNode);
	free_d2(Temp_Weight_OUT, OutNode);
	free_d1(Temp_Bias_HD);
	free_d1(Temp_Bias_OUT);
	//	free_d2(Data,n_samples);

	// time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;

	if (!OrgSet.AllContinuous())
		delete TrainSet;
	return 0;
}

// Train a NCL on an existed BPNN through RPROP
// AveragePrediction-Average predictions of all other classifiers
// M-Number of other classifiers
// Gamma- used to adjust the strength of the penalty and it is problem-dependent
int CBpnn::RPNCLTrain(const CDataset &OrgSet, const CPrediction &AveragePrediction, int M, double Gamma, int MaxEpoch,
					  double MinMSE)
{
	// start time for training
	clock_t start = clock();
	Name = MyName;

	// expand discrete attribute
	const CDataset *TrainSet = &OrgSet;
	if (!OrgSet.AllContinuous())
		TrainSet = OrgSet.ExpandDiscrete();
	const MATRIX &TrainData = TrainSet->GetData();
	const CASE_INFO &CaseInfo = TrainSet->GetInfo();

	int n_samples, target;
	// number of training instances
	n_samples = CaseInfo.Height;
	// lable of an instance is at the last position
	target = CaseInfo.ValidWidth - 1;

	// Allocate space for temporary parameters
	// input layer
	new_d1(double, Output_IN, InNode, 0);
	// hiden layer
	new_d1(double, Output_HD, HdNode, 0);
	// output layer
	new_d1(double, Output_OUT, OutNode, 0);
	//�����Ȩ��ƫ��
	new_d1(double, Alpha_Weight_HD, HdNode, 0);
	//������Ȩ��ƫ��
	new_d1(double, Alpha_Weight_OUT, OutNode, 0);
	// ideal output of network
	new_d1(double, Ideal_OUT, OutNode, 0);

	//ÿ��epoch: MSE���������Ȩ�ء�bias�ĵ���
	new_d2(double, Delta_Weight_HD, HdNode, InNode, 0);
	new_d2(double, Delta_Weight_OUT, OutNode, HdNode, 0);
	new_d1(double, Delta_Bias_HD, HdNode, 0);
	new_d1(double, Delta_Bias_OUT, OutNode, 0);
	// RPROP: ��¼�ϴεĵ���
	new_d2(double, Temp_Weight_HD, HdNode, InNode, 0);
	new_d2(double, Temp_Weight_OUT, OutNode, HdNode, 0);
	new_d1(double, Temp_Bias_HD, HdNode, 0);
	new_d1(double, Temp_Bias_OUT, OutNode, 0);

	//��dataתΪ������ʽ�����٣�
	//	//vector into array: to accelerate
	//	new_d2(ValueData,Data,n_samples,target+1,TrainData[i][j]);

	//����ֵ�ĳ�ʼֵ
	double DeltaVal = 0.1;
	// RPROP: ����ֵ
	new_d2(double, Delta_WeightV_HD, HdNode, InNode, DeltaVal);
	new_d2(double, Delta_WeightV_OUT, OutNode, HdNode, DeltaVal);
	new_d1(double, Delta_BiasV_HD, HdNode, DeltaVal);
	new_d1(double, Delta_BiasV_OUT, OutNode, DeltaVal);
	// RPROP: �ϴε�Delta
	new_d2(double, Last_Delta_Weight_HD, HdNode, InNode, 0);
	new_d2(double, Last_Delta_Weight_OUT, OutNode, HdNode, 0);
	new_d1(double, Last_Delta_Bias_HD, HdNode, 0);
	new_d1(double, Last_Delta_Bias_OUT, OutNode, 0);

	//���ͳ��
	MSE = 100.0;
	//���������ݼ���ѧϰ����
	epoch = 0;
	//��ʱ����
	double Net_HD, Net_OUT, sum;
	while (epoch < MaxEpoch && MSE > MinMSE) {
		/*Initialization*/
		for (int i = 0; i < HdNode; i++) {
			for (int j = 0; j < InNode; j++)
				Delta_Weight_HD[i][j] = 0;
			Delta_Bias_HD[i] = 0;
		}
		for (int i = 0; i < OutNode; i++) {
			for (int j = 0; j < HdNode; j++)
				Delta_Weight_OUT[i][j] = 0;
			Delta_Bias_OUT[i] = 0;
		}

		MSE = 0;
		/* using epoch training method*/
		for (int k = 0; k < n_samples; k++) {
			// innode��Ŀ����������Ŀ, ��������ֵ��Χ��չ
			for (int i = 0; i < InNode; i++) {
				if (CaseInfo.ValidAttrs[i].Max == CaseInfo.ValidAttrs[i].Min)
					Output_IN[i] = 0;
				else
					Output_IN[i] = (TrainData[k][i].Cont - CaseInfo.ValidAttrs[i].Min) /
								   (CaseInfo.ValidAttrs[i].Max - CaseInfo.ValidAttrs[i].Min);
			}
			//��ǰ����
			// hidden layer
			for (int j = 0; j < HdNode; j++) {
				sum = 0;
				for (int i = 0; i < InNode; i++)
					sum = sum + Weight_IN_HD[j][i] * Output_IN[i];
				Net_HD = sum + Bias_HD[j];
				Output_HD[j] = sigmoid(Net_HD);
			}
			// output layer
			for (int j = 0; j < OutNode; j++) {
				sum = 0;
				for (int i = 0; i < HdNode; i++)
					sum = sum + Weight_HD_OUT[j][i] * Output_HD[i];
				Net_OUT = sum + Bias_OUT[j];
				Output_OUT[j] = sigmoid(Net_OUT);
			}
			//���������: ѵ������ʵ������Ӧ�����Ϊ1, �������Ϊ0
			for (int i = 0; i < OutNode; i++)
				Ideal_OUT[i] = 0;
			// label is unknown, skip it
			if (TrainData[k][target].Discr == -1)
				continue;
			int Label = TrainData[k][target].Discr;
			Ideal_OUT[Label] = 1;
			// Sum of error of all output nodes
			const std::vector<std::vector<double>> &Probs = AveragePrediction.GetProbs();
			for (int i = 0; i < OutNode; i++) {
				double Fn = (Probs[k][i] * M + Output_OUT[i]) / (M + 1);
				MSE = MSE + (Ideal_OUT[i] - Output_OUT[i]) * (Ideal_OUT[i] - Output_OUT[i]) / 2.0 +
					  Gamma * (Output_OUT[i] - Fn) * M * (Probs[k][i] - Fn);
				//����ڵ�Ŀ����MSEΪ0, ��Ȩ�������MSE������ƫ��
				Alpha_Weight_OUT[i] =
					(Ideal_OUT[i] - Output_OUT[i] - Gamma * 2 * (1 - 1.0 / M) * (Fn - Output_OUT[i])) * Output_OUT[i] *
					(1 - Output_OUT[i]);
			}

			//����, ��¼�����Ȩ���ݶȣ�Error backword propagation��
			for (int i = 0; i < HdNode; i++) {
				sum = 0;
				//����������Ŀ��, ���������ڵ��������Ŀ��
				for (int j = 0; j < OutNode; j++)
					sum = sum + Alpha_Weight_OUT[j] * Weight_HD_OUT[j][i];
				//�����ڵ�: ��Ȩ�������MSE������ƫ��
				Alpha_Weight_HD[i] = Output_HD[i] * (1 - Output_HD[i]) * sum;
			}

			// Add the Delta weight and bias of one class for output layer
			for (int i = 0; i < OutNode; i++) {
				//�޸�delta: �������Ȩ��
				for (int j = 0; j < HdNode; j++) {
					Delta_Weight_OUT[i][j] = Delta_Weight_OUT[i][j] + Alpha_Weight_OUT[i] * Output_HD[j];
				}
				//�޸�delta: ��������ڲ�ƫ��
				Delta_Bias_OUT[i] = Delta_Bias_OUT[i] + Alpha_Weight_OUT[i];
			}
			// Add the Delta weight and bias of one class for hidden layer
			for (int i = 0; i < HdNode; i++) {
				//������Ȩ���޸�delta
				for (int j = 0; j < InNode; j++) {
					Delta_Weight_HD[i][j] = Delta_Weight_HD[i][j] + Alpha_Weight_HD[i] * Output_IN[j];
				}
				Delta_Bias_HD[i] = Delta_Bias_HD[i] + Alpha_Weight_HD[i];
			}
		} // end for k

		//ѧ����������, �޸�Ȩ��, ���м�¼ԭƫ��ֵ
		for (int i = 0; i < OutNode; i++) {
			for (int j = 0; j < HdNode; j++) {
				Weight_HD_OUT[i][j] += delta(Temp_Weight_OUT[i][j], Delta_Weight_OUT[i][j] / n_samples,
											 Delta_WeightV_OUT[i][j], Last_Delta_Weight_OUT[i][j]);
			}
			Bias_OUT[i] +=
				delta(Temp_Bias_OUT[i], Delta_Bias_OUT[i] / n_samples, Delta_BiasV_OUT[i], Last_Delta_Bias_OUT[i]);
		}
		for (int i = 0; i < HdNode; i++) {
			for (int j = 0; j < InNode; j++) {
				Weight_IN_HD[i][j] += delta(Temp_Weight_HD[i][j], Delta_Weight_HD[i][j] / n_samples,
											Delta_WeightV_HD[i][j], Last_Delta_Weight_HD[i][j]);
			}
			Bias_HD[i] +=
				delta(Temp_Bias_HD[i], Delta_Bias_HD[i] / n_samples, Delta_BiasV_HD[i], Last_Delta_Bias_HD[i]);
		}

		MSE = MSE / (n_samples * OutNode);
		epoch = epoch + 1;
	} // end for epoch

	free_d1(Output_IN);
	free_d1(Output_HD);
	free_d1(Output_OUT);
	free_d1(Alpha_Weight_HD);
	free_d1(Alpha_Weight_OUT);
	free_d1(Ideal_OUT);

	// Allocate space for delta weight
	free_d2(Delta_Weight_HD, HdNode);
	free_d2(Delta_Weight_OUT, OutNode);
	free_d1(Delta_Bias_HD);
	free_d1(Delta_Bias_OUT);

	// Allocate space for temp weight
	free_d2(Temp_Weight_HD, HdNode);
	free_d2(Temp_Weight_OUT, OutNode);
	free_d1(Temp_Bias_HD);
	free_d1(Temp_Bias_OUT);
	free_d2(Delta_WeightV_HD, HdNode);
	free_d2(Delta_WeightV_OUT, OutNode);
	free_d1(Delta_BiasV_HD);
	free_d1(Delta_BiasV_OUT);
	free_d2(Last_Delta_Weight_HD, HdNode);
	free_d2(Last_Delta_Weight_OUT, OutNode);
	free_d1(Last_Delta_Bias_HD);
	free_d1(Last_Delta_Bias_OUT);

	//	free_d2(Data,n_samples);

	// time consumed
	CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;

	if (!OrgSet.AllContinuous())
		delete TrainSet;
	return 0;
}

CClassifier *CBpnn::Clone() const
{
	CBpnn *Cls = new CBpnn(*this);

	return Cls;
}
