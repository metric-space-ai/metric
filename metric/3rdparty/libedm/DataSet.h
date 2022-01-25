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

#ifndef READ_FILE_INC
#define READ_FILE_INC

#include <deque>
#include <string>
#include <vector>

namespace libedm {
// return a pseudo-random number, between 0 and Max-1
// Max: should no larger than INT_MAX
int IntRand(int Max);

// project flags
// define this if you want to log dataset when bootstrap sampling
// define WRITE_DATA
// define this if you want to log models' training info
//#define LOG_TRAINING
// define this if you want to use normal distribution in Naive Bayes
//(or value range of an continuous attribute is divided into ten equal subrange)
//#define NB_GAUSS

// value
typedef union ValueData {
	int Discr;
	double Cont;
} ValueData;
// instance
typedef std::vector<ValueData> InstanceStr;
// DataSet
typedef std::vector<InstanceStr> MATRIX;

// discrete attribute (class labels is treat as discrete values)
typedef struct DiscValueStr {
	std::string Name;
} DiscValueStr;

// types of attribute
const int ATT_IGNORED = 0;
const int ATT_DISCRETE = 1;
const int ATT_CONTINUOUS = 2;
const int ATT_CLASSLABEL = 3;
const int ATT_DATETIME = 4;
// attributes
typedef struct AttrStr {
	int AttType;
	std::string Name;
	double Max; // maximum value
	double Min; // minimum value
	bool MMSet; // Have max and min value been set?
	// corresponding position of a valid attribute:
	// if it a attribute in ValidAttrs, it's its original position in ReadAttrs
	// if it a attribute in ReadAttrs, it's its new position in ValidAttrs
	int OtherPos;

	// discrete attribute: list of all values
	std::vector<DiscValueStr> Disc;
	AttrStr &operator=(const AttrStr &a)
	{
		if (this == &a)
			return *this;

		AttType = a.AttType;
		Name = a.Name;
		Max = a.Max;
		Min = a.Min;
		MMSet = a.MMSet;
		OtherPos = a.OtherPos;
		Disc.assign(a.Disc.begin(), a.Disc.end());

		return *this;
	}
} AttrStr;

typedef struct CASE_INFO {
	int ReadWidth;	// number of attribute in each row(including label)
	int ValidWidth; // number of real attribute (ignored attributes are excluded)
	int ClassNum;
	int Height; // number of instances
	std::vector<DiscValueStr> Classes;
	std::vector<AttrStr> ReadAttrs;	 // all attributes in a row (including label)
	std::vector<AttrStr> ValidAttrs; // all attributes in a row (ignored attributes are excluded)
	CASE_INFO &operator=(const CASE_INFO &a)
	{
		if (this == &a)
			return *this;

		ReadWidth = a.ReadWidth;
		ValidWidth = a.ValidWidth;
		ClassNum = a.ClassNum;
		Height = a.Height;
		Classes.assign(a.Classes.begin(), a.Classes.end());
		ReadAttrs.assign(a.ReadAttrs.begin(), a.ReadAttrs.end());
		ValidAttrs.assign(a.ValidAttrs.begin(), a.ValidAttrs.end());

		return *this;
	}
} CASE_INFO;
inline bool operator==(const DiscValueStr &a, const DiscValueStr &b);

// class CObj;
class CDataset //: public CObj
{
  protected:
	CASE_INFO CaseInfo;
	MATRIX Matrix;

  protected:
	// virtual void ReadMatrix(ifstream &DataFile,int Number=0);

  public:
	const MATRIX &GetData() const;
	const CASE_INFO &GetInfo() const;
	// all attributes are continuous?
	bool AllContinuous() const;
	// create a new dataset, by expanding every multi-valued discrete attribute into multi boolean attributes(needed by
	// BPNN and/or SVM)
	CDataset *ExpandDiscrete() const;

	~CDataset(){};
	// copy
	CDataset(const CDataset &DataSet);
	//
	CDataset(const CASE_INFO &Info, const MATRIX &Data)
	{
		Matrix.assign(Data.begin(), Data.end());
		CaseInfo = Info;
		CaseInfo.Height = (int)Matrix.size();
	};

	CDataset(); // create a null data set

	CDataset &operator+=(const CDataset &b);
	CDataset &operator=(const CDataset &b);

	// remove single-valued attributes
	void RemoveNullAttribute();
	// A training set must not contain instances with unknown label
	void RemoveUnknownInstance();

	// get a random sub set of a data set
	bool SubSet(int DataNum, CDataset &SubSet) const;
	// prepare the training set
	// bootstrap a data set with given size
	bool BootStrap(int DataNum, CDataset &TrainSet) const;
	// weighted bootstrap
	bool BootStrap(const std::vector<double> &Weights, int DataNum, std::vector<int> &OrginalPos,
				   CDataset &TrainSet) const;
	// original data set is randomly divided into two sub set
	bool SplitData(int DataNum, CDataset &TrainSet, CDataset &TestSet) const;
	// original data set is randomly divided into SetNum datasets (DataNum instances in each) and a TestSet
	bool SplitData(int DataNum, int SetNum, std::vector<CDataset> &TrainSets, CDataset &TestSet) const;
	// data set is divided into some several parts from begging to end.
	bool DevideBySetNum(int SetNum, std::vector<CDataset> &TrainSets) const;
	bool DevideByDataNum(int DataNum, std::vector<CDataset> &TrainSets) const;

	// for a continuous attribute, get max value less than t
	double GreatestValBelow(int Att, const double &t) const;
	// switch the position of two instances
	bool SwapInstance(int a, int b);
	// insert instances
	void Insert(const InstanceStr &Instance);
	// remove
	void Remove(int Pos);
	void ClearData();

  protected:
	// is the data file in CSV format?
	bool HasHeading(std::string &Line) const;
	// transfer a data std::string into our inner format
	virtual int FormatLine(std::string &Line) const;
	// search a name in class labels, return false if it is unknown
	bool Which(ValueData &Item, const std::string &Name) const;
	// search a value in the description of an attribute, return false if it's unknown
	bool Which(ValueData &Item, int AttrNum, const std::string &Name) const;

  protected:
	static const int LINE_OK;
	static const int SKIP_LINE;
};

} // namespace libedm

#include "DataSet.cpp"

#endif
