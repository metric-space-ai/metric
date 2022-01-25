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

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
// using namespace std;
//#include "Obj.h"
#include "C45.h"
#include "Classifier.h"
#include "DataSet.h"
#include "Prediction.h"

using namespace libedm;

// reimplementation of Quilan's C45 except processing of unknow values

// const int CC45::BrSubset=3;

// static const char MyName[MAX_OBJECT_NAME_LENGTH]="C45";
// const string CC45::StaticName=MyName;

static const double Log2 = 0.69314718055994530942;
//#define Log(x) ((x)<=0 ? 0.0 : log((double)x)/Log2)
template <typename T> inline T Log(T x) { return x <= 0 ? 0.0 : log((double)x) / Log2; }

inline CC45::~CC45() { DelTree(&TheRoot); }

inline void CC45::DelTree(NodeStr *Root)
{
	delete[] Root->ClassDist;

	int SubNum = Root->Forks;
	while (SubNum > 0) {
		DelTree(&(Root->SubNodes[SubNum - 1]));
		SubNum--;
	}
	if (Root->Forks > 0)
		delete[] Root->SubNodes;
	memset(Root, 0, sizeof(NodeStr));
}

// parameters- see definition
inline CC45::CC45(const CDataset &UTrainSet, int UMINOBJS, double UEpsilon, double UCF, double WillPrune)
	: TrainSet(UTrainSet)
{
	// Name=MyName;
	MINOBJS = UMINOBJS;
	Epsilon = UEpsilon;
	CF = UCF;
	// start time for training
	clock_t start = clock();

	//	int ClassNum=UTrainSet.GetInfo().ClassNum;
	//
	IsPruned = false;
	//	const MATRIX &TrainData=TrainSet.GetData();
	// info of training data
	const CASE_INFO &CaseInfo = TrainSet.GetInfo();
	// number of the training instances
	int CaseNum = CaseInfo.Height;

	// Do all discrete attributes have too many values?
	IsAllAttrMVals = true;

	// fill information of attributes
	for (int i = 0; i < CaseInfo.ValidWidth - 1; i++) {
		AttrInfoStr AttrInfo;
		AttrInfo.Bar = 0;
		AttrInfo.Gain = 0;
		AttrInfo.Info = 0;
		AttrInfo.Tested = 0;
		AttrInfo.MVals = true;
		if (CaseInfo.ValidAttrs[i].AttType == ATT_DISCRETE) {
			// Does an discrete attribute have too many values?
			if (CaseInfo.ValidAttrs[i].Disc.size() < 0.3 * (CaseInfo.Height + 1)) {
				AttrInfo.MVals = false;
				IsAllAttrMVals = false;
			}
		} else {
			AttrInfo.MVals = false;
			IsAllAttrMVals = false;
		}
		AttrInfos.push_back(AttrInfo);
	}

	for (int i = 0; i < CaseInfo.Height; i++) {
		DataInfoStr DataInfo = {1, 0, -Epsilon};
		DataInfos.push_back(DataInfo);
	}

	FormTree(0, CaseNum - 1, &TheRoot);
	//	Dump("aaa.txt");
	if (WillPrune)
		Prune(&TheRoot, 0, CaseNum - 1, true);
	//	Dump("bbb.txt");

	// time consumed
	// CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
}

// build a tree on the First to the Last instances of original training set
inline int CC45::FormTree(int Fp, int Lp, NodeStr *Root)
{
	// training data
	const MATRIX &TrainData = TrainSet.GetData();
	// info of training data
	const CASE_INFO &CaseInfo = TrainSet.GetInfo();
	// number of class labels
	int ClassNum = CaseInfo.ClassNum;
	// number of attributes (label is included)
	int Width = CaseInfo.ValidWidth;

	// set default attribute values for this node
	// Number of instances
	double CaseNum = Lp - Fp + 1;
	// distribution of training instances to each class label
	std::vector<double> ClassFreq(ClassNum, 0);
	for (int i = Fp; i <= Lp; i++) {
		// label of instance
		int Label = TrainData[i][Width - 1].Discr;
		ClassFreq[Label] += 1;
	}
	// most frequent class label is set as the label for current node
	int BestClass = 0;
	double BestFreq = 0.0;
	Root->ClassDist = new double[ClassNum];
	for (int i = 0; i < ClassNum; i++) {
		Root->ClassDist[i] = ClassFreq[i];
		if (ClassFreq[i] > BestFreq) {
			BestClass = i;
			BestFreq = ClassFreq[i];
		}
	}
	Root->NodeType = BrLeaf; // now we are a leaf
	Root->Forks = 0;
	Root->ClassNum = ClassNum;
	Root->SubNodes = NULL;
	Root->BestClass = BestClass;
	Root->Items = CaseNum;
	Root->Errors = CaseNum - BestFreq; // error is the number of instances with other labels
	Root->Attr = -1;				   // it is a leaf
	// if remaining instances are too few or all instances belong to one label, this node is a leaf
	if (CaseNum < 2 * MINOBJS || CaseNum == ClassFreq[BestClass]) {
		return 0;
	}

	// there are still enough instances, now we have to choose an unused attribute to group them
	// calculate info and gain for each attribute, and set cut values for continuous attributes
	double AverageGain = 0;
	int ValidAttr = 0;
	for (int i = 0; i < Width - 1; i++) {
		// default
		AttrInfos[i].Info = 0;
		AttrInfos[i].Gain = (-1) * Epsilon;

		if (CaseInfo.ValidAttrs[i].AttType == ATT_DISCRETE) {
			// if a discrete attribute is used in upper layer, skip it
			if (AttrInfos[i].Tested > 0)
				continue;
			EvalDiscreteAtt(i, Fp, Lp);
		}
		// continuous attributes are allowed to used several times (divided into several regions)
		else if (CaseInfo.ValidAttrs[i].AttType == ATT_CONTINUOUS || CaseInfo.ValidAttrs[i].AttType == ATT_DATETIME)
			EvalContinuousAtt(i, Fp, Lp);
		else // skipped
			continue;

		// gain sum for all attribute
		//		cout<<i<<": "<<AttrInfos[i].Gain<<endl;
		if (AttrInfos[i].Gain > -1 * Epsilon && (IsAllAttrMVals || !AttrInfos[i].MVals)) {
			AverageGain += AttrInfos[i].Gain;
			ValidAttr++;
		}
	}
	// average gain of all attributes
	AverageGain = (ValidAttr > 0 ? AverageGain / ValidAttr : 1e6);

	// attribute with max info gain ratio
	double BestVal = -1 * Epsilon;
	int BestAtt = -1;
	for (int i = 0; i < Width - 1; i++) {
		if (AttrInfos[i].Gain > -1 * Epsilon) {
			// gain/info ratio
			double Val;
			if (AttrInfos[i].Gain >= AverageGain - Epsilon && AttrInfos[i].Info > Epsilon)
				Val = AttrInfos[i].Gain / AttrInfos[i].Info;
			else
				Val = -1 * Epsilon;

			if (Val > BestVal) {
				BestAtt = i;
				BestVal = Val;
			}
		}
	}
	// none attribute is good enough
	if (BestAtt == -1)
		return 0;

	// divide this node by the best attribute value
	// create sub-nodes
	Root->Attr = BestAtt;
	{
		if (CaseInfo.ValidAttrs[BestAtt].AttType == ATT_CONTINUOUS ||
			CaseInfo.ValidAttrs[BestAtt].AttType == ATT_DATETIME) {
			Root->NodeType = ThreshContin;
			Root->Forks = 2;
			Root->Cut = TrainSet.GreatestValBelow(BestAtt, AttrInfos[BestAtt].Bar);
			Root->SubNodes = new NodeStr[2];
			memset(Root->SubNodes, 0, sizeof(NodeStr) * 2);
		} else if (CaseInfo.ValidAttrs[BestAtt].AttType == ATT_DISCRETE) {
			int ValueNum = (int)CaseInfo.ValidAttrs[BestAtt].Disc.size();
			Root->NodeType = BrDiscr;
			Root->Forks = ValueNum;
			Root->SubNodes = new NodeStr[ValueNum];
			memset(Root->SubNodes, 0, sizeof(NodeStr) * ValueNum);
		}
	}

	// flag that tells lower layers that this attribute has been used, need to recover before back to higher
	AttrInfos[BestAtt].Tested++;
	// build trees for sub nodes
	// calculate error of this node according the errors of all its sub nodes
	Root->Errors = 0;
	for (int i = 0; i < Root->Forks; i++) {
		// swap the data satisfying i-th node to the front
		int Ep = Group(i, Fp, Lp, Root);
		// at least one data in a node
		if (Ep - Fp + 1 > 0) {
			// build a sub tree
			FormTree(Fp, Ep, &(Root->SubNodes[i]));
			Root->Errors += Root->SubNodes[i].Errors;
		} else {
			Root->SubNodes[i].NodeType = BrLeaf;
			Root->SubNodes[i].Attr = -1;
			Root->SubNodes[i].Items = 0;
			Root->SubNodes[i].BestClass = BestClass;
			Root->SubNodes[i].Errors = 0;
			Root->SubNodes[i].Forks = 0;
			Root->SubNodes[i].ClassNum = ClassNum;
			Root->SubNodes[i].ClassDist = new double[ClassNum];
			for (int j = 0; j < ClassNum; j++)
				Root->SubNodes[i].ClassDist[j] = Root->ClassDist[j];
		}
	}

	// for higher layer, this attribute is not used
	AttrInfos[BestAtt].Tested--;

	// too much error, set this node as a leaf
	if (Root->Errors >= CaseNum - BestFreq - Epsilon) {
		Root->Attr = -1;
		Root->NodeType = BrLeaf;
		// only remove sub-nodes, keep others
		if (Root->Forks > 0) {
			for (int i = 0; i < Root->Forks; i++)
				DelTree(&Root->SubNodes[i]);
			delete[] Root->SubNodes;
			Root->SubNodes = NULL;
		}
		Root->Forks = 0;
	}

	return 0;
}

inline double CC45::Prune(NodeStr *Root, int Fp, int Lp, bool Update)
{
	// training data
	const MATRIX &TrainData = TrainSet.GetData();
	// info of training data
	const CASE_INFO &CaseInfo = TrainSet.GetInfo();

	// the class label distribution
	double CaseNum = CountItems(Fp, Lp);
	std::vector<double> LocalClassDist(CaseInfo.ClassNum, 0);
	for (int i = Fp; i <= Lp; i++)
		LocalClassDist[TrainData[i][CaseInfo.ValidWidth - 1].Discr] += DataInfos[i].Weight;

	// Find the most frequent class and update the root
	int BestClass = 0;
	for (int i = 1; i < CaseInfo.ClassNum; i++)
		if (LocalClassDist[i] > LocalClassDist[BestClass])
			BestClass = i;
	double LeafErrors = CaseNum - LocalClassDist[BestClass];
	double ExtraLeafErrors = AddErrs(CaseNum, LeafErrors);
	// update root info
	if (Update) {
		Root->Items = CaseNum;
		Root->BestClass = BestClass;
		//		Root->ClassDist=new double[CaseInfo.ClassNum];
		for (int i = 0; i < CaseInfo.ClassNum; i++)
			Root->ClassDist[i] = LocalClassDist[i];
	}
	// root is a leaf
	if (Root->NodeType == BrLeaf) {
		double TreeErrors = LeafErrors + ExtraLeafErrors;
		if (Update)
			Root->Errors = TreeErrors;
		return TreeErrors;
	}

	// prune subtree recursively
	double TreeErrors = 0;
	double MaxFactor = 0;
	// sub node with most instances
	int Ep = -1;
	int MaxBr = -1;
	for (int i = 0; i < Root->Forks; i++) {
		Ep = Group(i, Fp, Lp, Root);
		if (Ep - Fp + 1 > 0) {
			double Factor = CountItems(Fp, Ep) / CaseNum;
			if (Factor >= MaxFactor) {
				MaxBr = i;
				MaxFactor = Factor;
			}

			TreeErrors += Prune(&(Root->SubNodes[i]), Fp, Ep, Update);
		}
	}
	if (!Update)
		return TreeErrors;
	assert(MaxBr >= 0);
	// throw(CError("MaxBr not found!",203,0));

	// error of the largest sub node, here we don't know whether we should update it
	double BranchErrors = Prune(&(Root->SubNodes[MaxBr]), Fp, Lp, false);
	// all subtrees are not necessary, which should be removed
	if (LeafErrors + ExtraLeafErrors <= BranchErrors + 0.1 && LeafErrors + ExtraLeafErrors <= TreeErrors + 0.1) {
		Root->NodeType = BrLeaf;
		if (Root->Forks > 0) {
			for (int i = 0; i < Root->Forks; i++)
				DelTree(&(Root->SubNodes[i]));
			delete[] Root->SubNodes;
		}
		Root->SubNodes = NULL;
		Root->Forks = 0;
		Root->Errors = LeafErrors + ExtraLeafErrors;
		IsPruned = true;
	}
	// lower error, this subtree is promoted to replace the root
	else if (BranchErrors <= TreeErrors + 0.1) {
		Prune(&(Root->SubNodes[MaxBr]), Fp, Lp, true);
		// remove other sub-nodes
		for (int i = 0; i < Root->Forks; i++) {
			if (i != MaxBr)
				DelTree(&Root->SubNodes[i]);
		}
		// promote
		NodeStr Tmp = Root->SubNodes[MaxBr];
		// remove memory for this node
		delete[] Root->SubNodes;
		delete[] Root->ClassDist;
		*Root = Tmp;
		IsPruned = true;
	} else {
		Root->Errors = TreeErrors;
	}

	return Root->Errors;
}

// entropy for a discrete attribute
inline double CC45::TotalInfo(const std::vector<ValueInfoStr> &V)
{
	double Sum = 0, TotalItems = 0;

	for (int i = 0; i < (int)V.size(); i++) {
		if (V[i].Freq <= 0)
			continue;
		Sum += V[i].Freq * Log(V[i].Freq);
		TotalItems += V[i].Freq;
	}

	return TotalItems * Log(TotalItems) - Sum;
}

// entropy for a continuous attribute
inline double CC45::TotalInfo(const std::vector<double> &V)
{
	double Sum = 0, TotalItems = 0;

	for (int i = 0; i < (int)V.size(); i++) {
		if (V[i] <= 0)
			continue;
		Sum += V[i] * Log(V[i]);
		TotalItems += V[i];
	}

	return TotalItems * Log(TotalItems) - Sum;
}

// evaluate a discrete attribute (calculate its entropy and gain)
inline void CC45::EvalDiscreteAtt(int Att, int First, int Last)
{
	// training data
	const MATRIX &TrainData = TrainSet.GetData();
	// info of training data
	const CASE_INFO &CaseInfo = TrainSet.GetInfo();
	// number of class labels
	int ClassNum = CaseInfo.ClassNum;
	// number of all training instances
	double CaseNum = Last - First + 1;
	// number of different values
	int ValueNum = (int)CaseInfo.ValidAttrs[Att].Disc.size();
	// if no training instance
	if (CaseNum <= 0)
		return;

	// for a discrete, frequency for each value
	std::vector<ValueInfoStr> ValueInfos;
	// initialize
	for (int i = 0; i < ValueNum; i++) {
		ValueInfoStr ValueInfo;
		ValueInfo.Freq = 0;
		for (int j = 0; j < ClassNum; j++)
			ValueInfo.ClassFreq.push_back(0);
		ValueInfos.push_back(ValueInfo);
	}
	// ClassFreq: appearance frequency for each value to each class label
	// Freq: frequency for each value
	for (int i = First; i <= Last; i++) {
		// for each instance
		int Class = TrainData[i][CaseInfo.ValidWidth - 1].Discr;
		int Val = TrainData[i][Att].Discr;

		ValueInfos[Val].ClassFreq[Class] += 1;
		ValueInfos[Val].Freq += 1;
	}

	// average entropy
	AttrInfos[Att].Info = TotalInfo(ValueInfos) / CaseNum;

	// sum of entropy for all class labels
	double Sum = 0;
	for (int j = 0; j < ClassNum; j++) {
		double ClassCount = 0;
		// entropy for each class label
		for (int i = 0; i < ValueNum; i++)
			ClassCount += ValueInfos[i].ClassFreq[j];
		Sum += ClassCount * Log(ClassCount);
	}
	double BaseInfo = (CaseNum * Log(CaseNum) - Sum) / CaseNum;

	// gain for values
	// number of values with instance greater than MINOBJS
	int ReasonableSubsets = 0;
	// sum of entropy for each value
	double ThisInfo = 0;
	for (int i = 0; i < ValueNum; i++) {
		if (ValueInfos[i].Freq >= MINOBJS)
			ReasonableSubsets++;
		ThisInfo += TotalInfo(ValueInfos[i].ClassFreq);
	}
	if (ReasonableSubsets < 2)
		return;

	AttrInfos[Att].Gain = BaseInfo - ThisInfo / CaseNum;
}

// evaluate a continuous attribute
// default value for info and gain has been set before enter this function
inline void CC45::EvalContinuousAtt(int Att, int Fp, int Lp)
{
	// training data
	const MATRIX &TrainData = TrainSet.GetData();
	// info of training data
	const CASE_INFO &CaseInfo = TrainSet.GetInfo();
	// number of class labels
	int ClassNum = CaseInfo.ClassNum;
	// number of attributes
	int Width = CaseInfo.ValidWidth;
	// number of all training instances
	int CaseNum = Lp - Fp + 1;
	// if too few instances
	if (CaseNum < 2 * MINOBJS)
		return;

	// sort instances by values of this attribute(we don't change the order of original data set)
	std::vector<ContValueStr> ContValues;
	for (int i = Fp; i <= Lp; i++) {
		ContValueStr ContValue;
		// instance: value for this attribute
		ContValue.Value = TrainData[i][Att].Cont;
		// label of this instance
		ContValue.Label = TrainData[i][Width - 1].Discr;
		// No. of instance
		ContValue.No = i;
		// weight of instance
		ContValue.Weight = DataInfos[i].Weight;
		//
		ContValue.SplitGain = 0;
		ContValue.SplitInfo = 0;
		// we will process instances on ContValues instead of the original data set
		ContValues.push_back(ContValue);
	}
	sort(ContValues.begin(), ContValues.end(), ValueAscOrder);

	// initializing
	// information for each value of this attribute
	std::vector<ValueInfoStr> ValueInfos;
	{
		ValueInfoStr ValueInfo;
		// appearance frequency of this value
		ValueInfo.Freq = 0;
		// appearance frequency of this value on each class label
		for (int j = 0; j < ClassNum; j++)
			ValueInfo.ClassFreq.push_back(0);
		// two
		for (int j = 0; j < 2; j++)
			ValueInfos.push_back(ValueInfo);
	}
	// Base entropy
	for (int i = 0; i < CaseNum; i++) {
		ValueInfos[1].ClassFreq[ContValues[i].Label] += ContValues[i].Weight;
		ContValues[i].SplitGain = -Epsilon;
		ContValues[i].SplitInfo = 0;
	}
	double BaseInfo = TotalInfo(ValueInfos[1].ClassFreq) / CaseNum;

	// minimum instances of a group
	double MinSplit = 0.10 * CaseNum / (ClassNum + 1);
	if (MinSplit <= MINOBJS)
		MinSplit = MINOBJS;
	else if (MinSplit > 25)
		MinSplit = 25;

	// try group the instances by a value (instances have been ordered by values of the attribute), get corresponding
	// info and gain
	double LowItems = 0;
	//	double AvGain=0;
	// number of candidate
	int Tries = 0;
	// why CaseNum-1?
	for (int i = 0; i < CaseNum - 1; i++) {
		int c = ContValues[i].Label;
		LowItems += ContValues[i].Weight;
		ValueInfos[0].ClassFreq[c] += ContValues[i].Weight;
		ValueInfos[1].ClassFreq[c] -= ContValues[i].Weight;

		if (LowItems < MinSplit)
			continue;
		else if (LowItems > CaseNum - MinSplit)
			break;

		if (ContValues[i].Value < ContValues[i + 1].Value - 1e-5) {
			Tries++;
			ValueInfos[0].Freq = LowItems;
			ValueInfos[1].Freq = CaseNum - LowItems;

			// calculate info and gain
			ContValues[i].SplitInfo = TotalInfo(ValueInfos) / CaseNum;
			// gain: reference code for discrete attribute
			int ReasonableSubsets = 0;
			double ThisInfo = 0;
			for (int j = 0; j < 2; j++) {
				if (ValueInfos[j].Freq >= MINOBJS)
					ReasonableSubsets++;
				ThisInfo += TotalInfo(ValueInfos[j].ClassFreq);
			}
			//
			if (ReasonableSubsets >= 2)
				ContValues[i].SplitGain = BaseInfo - ThisInfo / CaseNum;
			//			AvGain+=TrainData.data[i].SplitGain;
		}
	}
	double ThreshCost = Log((double)Tries) / CaseNum;

	// find the best
	double BestVal = 0;
	int BestI = -1;
	for (int i = 0; i < CaseNum - 1; i++) {
		double Val = ContValues[i].SplitGain - ThreshCost;
		if (Val > BestVal) {
			BestI = i;
			BestVal = Val;
		}
	}

	if (BestI == -1)
		return;

	AttrInfos[Att].Bar = (ContValues[BestI].Value + ContValues[BestI + 1].Value) / 2;
	AttrInfos[Att].Gain = BestVal;
	AttrInfos[Att].Info = ContValues[BestI].SplitInfo;
}

// sum of instance weights
inline double CC45::CountItems(int Fp, int Lp)
{
	double Sum = 0;
	for (int i = Fp; i <= Lp; i++)
		Sum += DataInfos[i].Weight;
	return Sum;
}

// sort instances from fp to lp by the criterion denoted by the node
// return-last instance satisfy this node
inline int CC45::Group(int V, int Fp, int Lp, NodeStr *TestNode)
{
	// training data
	const MATRIX &TrainData = TrainSet.GetData();
	// info of training data
	//	const CASE_INFO &CaseInfo=TrainSet.GetInfo();

	int Att = TestNode->Attr;
	switch (TestNode->NodeType) {
	case BrDiscr: // instances whose value for this discrete attribute equal V are swap to the front
		for (int i = Fp; i <= Lp; i++)
			if (TrainData[i][Att].Discr == V)
				TrainSet.SwapInstance(Fp++, i);
		break;
	case ThreshContin: // divide the continuous attribute(V==0, no larger than threshold; V==1, otherwise)
		double Thresh = TestNode->Cut;
		for (int i = Fp; i <= Lp; i++)
			if ((TrainData[i][Att].Cont <= Thresh) == (V == 0))
				TrainSet.SwapInstance(Fp++, i);
		break;
	}

	return Fp - 1;
}

/*************************************************************************/
/*									 */
/*  Compute the additional errors if the error rate increases to the	 */
/*  upper limit of the confidence level.  The coefficient is the	 */
/*  square of the number of standard deviations corresponding to the	 */
/*  selected confidence level. (Taken from Document Geigy Scientific	 */
/*  Tables(Sixth Edition),p185(with modifications).)			 */
/*									 */
/*************************************************************************/
static const double Val[] = {0, 0.001, 0.005, 0.01, 0.05, 0.10, 0.20, 0.40, 1.00};
static const double Dev[] = {4.0, 3.09, 2.58, 2.33, 1.65, 1.28, 0.84, 0.25, 0.00};
inline double CC45::AddErrs(double N, double e)
{
	static double Coeff = 0;

	if (!Coeff) {
		// Compute and retain the coefficient value,interpolating from the values in Val and Dev
		int i = 0;
		while (CF > Val[i])
			i++;

		Coeff = Dev[i - 1] + (Dev[i] - Dev[i - 1]) * (CF - Val[i - 1]) / (Val[i] - Val[i - 1]);
		Coeff = Coeff * Coeff;
	}

	if (e < 1E-6)
		return N * (1 - exp(log(CF) / N));

	if (e < 0.9999) {
		double Val0 = N * (1 - exp(log(CF) / N));
		return Val0 + e * (AddErrs(N, 1.0) - Val0);
	} else if (e + 0.5 >= N) {
		return 0.67 * (N - e);
	} else {
		double Pr = (e + 0.5 + Coeff / 2 + sqrt(Coeff * ((e + 0.5) * (1 - (e + 0.5) / N) + Coeff / 4))) / (N + Coeff);
		return (N * Pr - e);
	}
}

// //layer- depth in tree, used for display
// bool CC45::DumpTree(ofstream &File,int Layer,const NodeStr *Root) const
// {
// 	int Att=Root->Attr;

// 	if(Root->NodeType==BrLeaf)
// 	{
// 		File<<Root->BestClass;
// 		return true;
// 	}

// 	for(int i=0;i<Root->Forks;i++)
// 	{
// 		switch(Root->NodeType)
// 		{
// 			case BrDiscr:
// 				File<<endl;
// 				for(int j=0;j<=Layer;j++)
// 					File<<"|\t";
// 				File.precision(0);
// 				File<<Att<<"="<<i<<"("<<
// 					fixed<<Root->SubNodes[i].Errors<<"):";
// 				break;
// 			case ThreshContin:
// 				File<<endl;
// 				for(int j=0;j<=Layer;j++)
// 					File<<"|\t";
// 				File.precision();
// 				File<<Att<<" "<<(i<=0 ? "<=":">")<<" "<<Root->Cut;
// 				File.precision(0);
// 				File<<"("<<fixed<<Root->SubNodes[i].Errors<<"):";
// 				break;
// 			default:
// 				break;
// 		}
// 		DumpTree(File,Layer+1,&(Root->SubNodes[i]));
// 	}

// 	File<<endl;
// 	return true;
// }

// bool CC45::Dump(const string &FileName) const
// {
// 	ofstream OutFile;
// 	OutFile.open(FileName.c_str());
// 	if(OutFile.fail())
// 		return false;

// 	OutFile<<"Root";
// 	if(!DumpTree(OutFile,0,&TheRoot))
// 	{
// 		OutFile.close();
// 		return false;
// 	}

// 	OutFile.close();
// 	return true;
// }

// bool CC45::Save(ofstream &File,const NodeStr *Root) const
// {
// 	//root information
// 	File.write((char*)&Root->NodeType,sizeof(Root->NodeType));
// 	File.write((char*)&Root->Attr,sizeof(Root->Attr));
// 	File.write((char*)&Root->BestClass,sizeof(Root->BestClass));
// 	File.write((char*)&Root->Forks,sizeof(Root->Forks));
// 	File.write((char*)&Root->ClassNum,sizeof(Root->ClassNum));
// 	File.write((char*)&Root->Items,sizeof(Root->Items));
// 	File.write((char*)&Root->Cut,sizeof(Root->Cut));
// 	File.write((char*)&Root->Errors,sizeof(Root->Errors));
// 	//distribution of each class label
// 	if(Root->ClassNum>0)
// 	{
// 		File.write((char*)(Root->ClassDist),Root->ClassNum*sizeof(double));
// 	}

// 	//number of sub nodes
// 	for(int i=0;i<Root->Forks;i++)
// 		Save(File,&Root->SubNodes[i]);

// 	return true;
// }

// int CC45::Save(const string &Path,const string &FileName) const
// {
// 	ofstream OutFile;
// 	OutFile.open((Path+FileName+"."+Name).c_str(),ios_base::out|ios_base::trunc|ios_base::binary);
// 	if(OutFile.fail())
// 		return 1;

// 	if(!Save(OutFile,&TheRoot))
// 	{
// 		OutFile.close();
// 		return 2;
// 	}

// 	OutFile.close();
// 	return 0;
// }

// //get in a tree by read nodes one by one in the depth-first order
// void CC45::Load(ifstream &InFile,NodeStr *Root)
// {
// 	//read root information
// 	InFile.read((char*)&Root->NodeType,sizeof(Root->NodeType));
// 	if(InFile.fail()) throw(CError("C45: read parameter error!",202,0));
// 	InFile.read((char*)&Root->Attr,sizeof(Root->Attr));
// 	if(InFile.fail()) throw(CError("C45: read parameter error!",202,0));
// 	InFile.read((char*)&Root->BestClass,sizeof(Root->BestClass));
// 	if(InFile.fail()) throw(CError("C45: read parameter error!",202,0));
// 	InFile.read((char*)&Root->Forks,sizeof(Root->Forks));
// 	if(InFile.fail()) throw(CError("C45: read parameter error!",202,0));
// 	InFile.read((char*)&Root->ClassNum,sizeof(Root->ClassNum));
// 	if(InFile.fail()) throw(CError("C45: read parameter error!",202,0));
// 	InFile.read((char*)&Root->Items,sizeof(Root->Items));
// 	if(InFile.fail()) throw(CError("C45: read parameter error!",202,0));
// 	InFile.read((char*)&Root->Cut,sizeof(Root->Cut));
// 	if(InFile.fail()) throw(CError("C45: read parameter error!",202,0));
// 	InFile.read((char*)&Root->Errors,sizeof(Root->Errors));
// 	if(InFile.fail()) throw(CError("C45: read parameter error!",202,0));

// 	//sub node number
//  	if(Root->ClassNum>0)
// 	{
// 		Root->ClassDist=new double[Root->ClassNum];
// 		InFile.read((char*)Root->ClassDist,Root->ClassNum*sizeof(double));
// 		if(InFile.fail()) throw(CError("C45: read parameter error!",202,0));
// 	}

// 	//read sub node recursively
// 	if(Root->Forks>0)
// 	{
// 		Root->SubNodes=new NodeStr[Root->Forks];
// 		for(int i=0;i<Root->Forks;i++)
// 			Load(InFile,&Root->SubNodes[i]);
// 	}
// }

// CC45::CC45(const string &Path,const string &FileName)
// {
// 	//Name=MyName;
// 	//start time for training
// 	clock_t start=clock();

// 	ifstream InFile;
// 	InFile.open((Path+FileName+"."+Name).c_str(),ios_base::in|ios_base::binary);
// 	if(InFile.fail())
// 		throw(CError("C45: fail open saved file!",201,0));

// 	Load(InFile,&TheRoot);
// 	InFile.close();

// 	//time consumed
// 	//CreatingTime = (double)(clock() - start) / CLOCKS_PER_SEC;
// }

// classify an instance, output the probabilities that this instance belongs to each class label
inline void CC45::ClassOneCase(const CDataset &DataSet, int i, const NodeStr *Root, double Weight,
							   std::vector<double> &Prob) const
{
	// training data
	const MATRIX &TrainData = DataSet.GetData();
	// info of training data
	const CASE_INFO &CaseInfo = DataSet.GetInfo();
	// number of class label
	int ClassNum = CaseInfo.ClassNum;

	switch (Root->NodeType) {
	case BrLeaf: // in a leaf,there may contain instances with different labels
		if (Root->Items > 0) {
			for (int i = 0; i < ClassNum; i++)
				if (Root->ClassDist[i] > 0) // probability is computed from the frequency got when training
					Prob[i] += (Weight * Root->ClassDist[i] / Root->Items);
		} else
			Prob[Root->BestClass] += Weight;
		break;
	case BrDiscr:
		// assume all value of this attribute are known
		{
			int DVal = TrainData[i][Root->Attr].Discr;
			ClassOneCase(DataSet, i, &Root->SubNodes[DVal], Weight, Prob);
		}
		break;
	case ThreshContin: // enter the branch determined by threshold
	{
		double Cv = TrainData[i][Root->Attr].Cont;
		int DVal = (Cv <= Root->Cut ? 0 : 1);
		ClassOneCase(DataSet, i, &Root->SubNodes[DVal], Weight, Prob);
	} break;
	}

	return;
}

// classify a data set
inline CPrediction *CC45::Classify(const CDataset &TestSet) const
{
	// start time for training
	clock_t Start = clock();

	//	const MATRIX &TrainData=TestSet.GetData();
	const CASE_INFO &CaseInfo = TestSet.GetInfo();
	int ClassNum = CaseInfo.ClassNum;
	int CaseNum = CaseInfo.Height;
	// 2d array to save probabilities of each instance on each class label
	std::vector<std::vector<double>> Probs;

	// classifying on each instance
	for (int i = 0; i < CaseNum; i++) {
		std::vector<double> Prob(ClassNum, 0);
		ClassOneCase(TestSet, i, &TheRoot, 1.0, Prob);
		Probs.push_back(Prob);
	}

	// create a CPrediction object
	return (new CPrediction(TestSet, Probs, clock() - Start));
}

inline CClassifier *CC45::Clone() const
{
	CC45 *Cls = new CC45(*this);

	return Cls;
}
