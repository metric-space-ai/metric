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


#include <string>
#include <algorithm>
#include <iostream>
// #include <fstream>
// #include <istream>
// #include <sstream>
// #include <climits>

 #include <cstring> // uncommented by Max, Oct 21, for strcpy
 #include <assert.h> // added by Max, Oct 21, for assert
// #include <cassert>
// #include <cstdarg>
// #include <ctime>
//using namespace std;
using namespace libedm;
//#include "Obj.h"
//#include "zString.h"
//#include "DateTime.h"
#include "DataSet.h"
#include "RandSequence.h"


CDataset &CDataset::operator +=(const CDataset &b)
{
	//same object
	if(this==&b)  
		return *this;  
	//empty dat set
	if(CaseInfo.Height==0)
	{
		*this=b;
		return *this;
	}

	// if(CaseInfo.ValidWidth!=b.CaseInfo.ValidWidth)
	// 	throw("Adding different type of data!",701,0);

	CaseInfo.Height+=b.CaseInfo.Height;
	for(int i=0;i<(int)b.CaseInfo.Height;i++)
		Matrix.push_back(b.Matrix[i]);

	return *this;
}

CDataset &CDataset::operator =(const CDataset &b)
{
	if(this==&b)  
		return *this;  

	CaseInfo=b.CaseInfo;
	Matrix.assign(b.Matrix.begin(),b.Matrix.end());

	return *this;
}

CDataset::CDataset()
{
	//CreatingTime=0;

	CaseInfo.Height=0;
}


const int CDataset::LINE_OK=0;
const int CDataset::SKIP_LINE=1;
//remove all control chars and reserve only space and comma
int CDataset::FormatLine(std::string &Line) const
{
	int Dot=0;
	bool PeriodisLast=false;
	//use space as delimiter?
	bool SpaceAsDelimiter=false;
	if(Line.find(',')==std::string::npos&&Line.find(':')==std::string::npos)
		SpaceAsDelimiter=true;
	//table is transfered into space
	for(int i=0;i<(int)Line.length();i++)
		if(Line[i]=='\t')
			Line[i]=' ';

	//format
	for(int i=0;i<(int)Line.length();)
	{
		//all other non-readable chars are treated as delimiter
		if(Line[i]<' ')
		{
			Line.erase(Line.begin()+i,Line.end());
			continue;
		}

		//remove leading spaces
		if(i==0&&Line[i]==' ')
		{
			Line.erase(Line.begin());
			continue;
		}

		switch(Line[i])
		{
		case '"'://remove quotation mark
		case '\'':
			Line.erase(Line.begin()+i);
			continue;
		case '|'://remove commented
		case '%'://remove commented
			Line.erase(Line.begin()+i,Line.end());
			continue;
		case ';'://heading semicolon as comment
			if(i<=0)
			{
				Line.erase(Line.begin()+i,Line.end());
				continue;
			}
			else//otherwise as delimiter
				Line[i]=',';
			break;
		case '.'://last character in line is a dot, which means end of line
			Dot=i;
			PeriodisLast=true;
			break;
		case ','://delimiter
		case ':'://start of attribute description
			//remove preceding and trailing spaces
			while(Line[i+1]==' ')
				Line.erase(Line.begin()+i+1);
			while(i>0&&Line[i-1]==' ')
			{
				Line.erase(Line.begin()+i-1);
				i--;
			}
			break;
		case ' ':
			break;
		default://non-control chars
			//lower case
			if(Line[i]>='A'&&Line[i]<='Z')
				Line[i]-=('A'-'a');
			PeriodisLast=false;
			break;
		}

		i++;
	}

	//if last char is a period, the line is ended
	if(PeriodisLast)
		Line.erase(Line.begin()+Dot,Line.end());
	//remove trailing spaces
	for(int i=(int)Line.length()-1;i>=0;i--)
	{
		if(Line[i]==' '||Line[i]=='\t')
			Line.erase(Line.begin()+i);
		else
			break;		
	}
	//convert space into comma
	for(int i=0;i<(int)Line.length();i++)
		if(SpaceAsDelimiter&&Line[i]==' ')
			Line[i]=',';

	//empty line?
	if(Line.length()<=0)
		return SKIP_LINE;
	return LINE_OK;
}



//if the first word of the first line is the name of any attribute, this file has a head line
bool CDataset::HasHeading(std::string &Line) const
{
	//read a value
	char *DataLine=new char[Line.length()+1];
    std::strcpy(DataLine,Line.c_str());
	const char *Del=",";
	char *pValue=strtok(DataLine,Del);
	std::string Value=pValue;

	// is it a name for an attribute?
	int i;
	for(i=0;i<CaseInfo.ReadWidth;i++)
		if(CaseInfo.ReadAttrs[i].Name==Value)
			break;
	delete [] DataLine;
	if(i<CaseInfo.ReadWidth)
		return true;

	return false;
}


// //get training samples from * file to Matrix->data array
// //unknown class labeled is marked as -1
// //instances with unknown values is removed
// //Number=0, read all;>0 read some; <0 read none
// void CDataset::ReadMatrix(ifstream &DataFile,int Number)
// {
// 	assert(!DataFile.fail());
// 	// {
// 	// 	throw(CError("Open data file failed!",308,0));
// 	// }
// 	Matrix.clear();
// 	if(DataFile.eof()||Number<0)
// 		return;

// 	//read a instance
// 	int InstanceNum=0;
// 	int IsFinish=LINE_OK;
// 	while(!DataFile.eof())
// 	{
// 		//read a line
// 		string Line;
// 		// try
// 		// {
// 			getline(DataFile,Line);
// 		// }
// 		// catch(...)
// 		// {
// 		// 	//not enough buffer
// 		// 	throw(CError("Data file: too long a line!",309,0));
// 		// }
// 		if(DataFile.fail())
// 		{
// 			assert(!DataFile.eof());
// 			// 	continue;
// 			// throw(CError("Data file: read error!",310,0));
// 		}
// 		//format and parse the line
// 		if((IsFinish=FormatLine(Line))==SKIP_LINE)
// 			continue;
// 		//csv format has a heading line, skip it
// 		if(InstanceNum==0 && HasHeading(Line))
// 			continue;

// 		//read data from the line
// 		int ValueNum=0;
// 		InstanceStr Inst;
// 		bool HasMissedValue=false;
// 		ValueData Label;

// 		char *DataLine=new char[Line.length()+1];
// 		strcpy(DataLine,Line.c_str());
// 		const char *Del=",";
// 		char *pValue=strtok(DataLine,Del);
// 		while(pValue!=NULL)
// 		{
// 			//read a value
// 			string Value(pValue);

// 			ValueData Item;
// 			//get class label
// 			if(CaseInfo.ReadAttrs[ValueNum].AttType==ATT_CLASSLABEL)
// 			{
// 				// try
// 				// {
// 					Which(Label,Value);
// 				// }
// 				// catch(CError &Err)
// 				// {
// 				// 	delete [] DataLine;

// 				// 	basic_ostringstream<char> OutMsg;
// 				// 	OutMsg<<" in line "<<InstanceNum<<ends;
// 				// 	Err.Description+=OutMsg.str();
// 				// 	throw(Err);
// 				// }
// 				ValueNum++;
// 				//read next value
// 				pValue=strtok(NULL,Del);
// 				continue;
// 			}
// 			else if(CaseInfo.ReadAttrs[ValueNum].AttType==ATT_IGNORED)
// 			{
// 				//for ignored attribute, we just skip the value
// 				ValueNum++;
// 				//read next value
// 				pValue=strtok(NULL,Del);
// 				continue;
// 			}
// 			else if(CaseInfo.ReadAttrs[ValueNum].AttType==ATT_DISCRETE)
// 			{  
// 				//discrete attribute with int value(ItemNo)
// 				// try
// 				// {
// 					Which(Item,ValueNum,Value);
// 				// }
// 				// catch(CError &Err)
// 				// {
// 				// 	delete [] DataLine;

// 				// 	basic_ostringstream<char> OutMsg;
// 				// 	OutMsg<<" in line "<<InstanceNum<<ends;
// 				// 	Err.Description+=OutMsg.str();
// 				// 	throw(Err);
// 				// }

// 				int k=CaseInfo.ReadAttrs[ValueNum].OtherPos;
// 				//value is a "?" (unknown)
// 				if(Item.Discr==-1)
// 					HasMissedValue=true;
// 				//maximum and minimum value for attribute
// 				else if(!CaseInfo.ValidAttrs[k].MMSet)
// 				{
// 					CaseInfo.ValidAttrs[k].Max=Item.Discr;
// 					CaseInfo.ValidAttrs[k].Min=Item.Discr;
// 					CaseInfo.ValidAttrs[k].MMSet=true;

// 					CaseInfo.ReadAttrs[ValueNum].Max=Item.Discr;
// 					CaseInfo.ReadAttrs[ValueNum].Min=Item.Discr;
// 					CaseInfo.ReadAttrs[ValueNum].MMSet=true;
// 				}
// 				else
// 				{
// 					if(Item.Discr>CaseInfo.ValidAttrs[k].Max)
// 					{
// 						CaseInfo.ValidAttrs[k].Max=Item.Discr;
// 						CaseInfo.ReadAttrs[ValueNum].Max=Item.Discr;
// 					}
// 					if(Item.Discr<CaseInfo.ValidAttrs[k].Min)
// 					{
// 						CaseInfo.ValidAttrs[k].Min=Item.Discr;
// 						CaseInfo.ReadAttrs[ValueNum].Min=Item.Discr;
// 					}
// 				}
// 			}
// 			//unknown continuous value?
// 			else if(Value=="?")
// 			{
// 				HasMissedValue=true;
// 			}
// 			else if(CaseInfo.ReadAttrs[ValueNum].AttType==ATT_CONTINUOUS||
// 				CaseInfo.ReadAttrs[ValueNum].AttType==ATT_DATETIME)
// 			{
// 				//continuous attribute with double value
// 				if(CaseInfo.ReadAttrs[ValueNum].AttType==ATT_CONTINUOUS)
// 				{
// 					basic_istringstream<char> FloatString(Value.c_str());
// 					FloatString>>Item.Cont;
// 					if(FloatString.fail())
// 						break;
// 				}
// 				// else//date time
// 				// {
// 				// 	CDateTime DateValue(Value);
// 				// 	Item.Cont=(double)DateValue.GetNumeric();
// 				// }

// 				//maximum and minimum value for continuous attribute
// 				int k=CaseInfo.ReadAttrs[ValueNum].OtherPos;
// 				if(!CaseInfo.ValidAttrs[k].MMSet)
// 				{
// 					CaseInfo.ValidAttrs[k].Max=Item.Cont;
// 					CaseInfo.ValidAttrs[k].Min=Item.Cont;
// 					CaseInfo.ValidAttrs[k].MMSet=true;

// 					CaseInfo.ReadAttrs[ValueNum].Max=Item.Cont;
// 					CaseInfo.ReadAttrs[ValueNum].Min=Item.Cont;
// 					CaseInfo.ReadAttrs[ValueNum].MMSet=true;
// 				}
// 				else
// 				{
// 					if(Item.Cont>CaseInfo.ValidAttrs[k].Max)
// 					{
// 						CaseInfo.ValidAttrs[k].Max=Item.Cont;
// 						CaseInfo.ReadAttrs[ValueNum].Max=Item.Cont;
// 					}
// 					if(Item.Cont<CaseInfo.ValidAttrs[k].Min)
// 					{
// 						CaseInfo.ValidAttrs[k].Min=Item.Cont;
// 						CaseInfo.ReadAttrs[ValueNum].Min=Item.Cont;
// 					}
// 				}
// 			}

// 			ValueNum++;
// 			Inst.push_back(Item);
// 			//read next value
// 			pValue=strtok(NULL,Del);
// 		}//line
// 		delete [] DataLine;
// 		//not enough values for all attributes
// 		assert((int)Inst.size()+1==CaseInfo.ValidWidth);
// 		// {
// 		// 	basic_ostringstream<char> OutMsg;
// 		// 	OutMsg<<"Data file: illegal instance data in line "<<InstanceNum<<ends;
// 		// 	throw(CError(string(OutMsg.str()),311,0));
// 		// }
// 		if(!HasMissedValue)//skip the instance with unknown value
// 		{
// 			//put label into instance
// 			Inst.push_back(Label);
// 			Matrix.push_back(Inst);
// 		}
// 		//number of instances has processed (include the skipped one)
// 		InstanceNum++;
// 		//we have enough instances, can now leave
// 		if(Number>0 && InstanceNum>=Number)
// 			break;
// // 		//display
// // 		if(InstanceNum%200000==0)
// // 			printf("Instance: %d\n",InstanceNum);
// 	}//end of file

// 	//number of instances
// 	CaseInfo.Height=(int)Matrix.size();
// }

//Locate value
bool libedm::operator ==(const DiscValueStr &a,const DiscValueStr &b)
{
	return (a.Name==b.Name);
}

//from string to class label
bool CDataset::Which(ValueData &Item,const std::string &Name) const
{
	//unknown label
	if(Name=="?")
	{
		Item.Discr=-1;
		return false;
	}

	//search
	DiscValueStr tmp;
	tmp.Name=Name;
	std::vector<DiscValueStr>::const_iterator it;
	it=find(CaseInfo.Classes.begin(),CaseInfo.Classes.end(),tmp);
	assert(it!=CaseInfo.Classes.end());
	// {
	// 	string Msg="unexpected class label ";
	// 	throw(CError(Msg+"'"+Name+"'",312,0));
	// }

	Item.Discr=(int)(it-CaseInfo.Classes.begin());
	return true;
}

//from string to a discrete attribute value
bool CDataset::Which(ValueData &Item,int ValueNum,const std::string &Name) const
{
	//unknown value
	if(Name=="?")
	{
		Item.Discr=-1;
		return false;
	}

	std::vector<DiscValueStr>::const_iterator it;
	DiscValueStr tmp;
	tmp.Name=Name;

	it=find(CaseInfo.ReadAttrs[ValueNum].Disc.begin(),CaseInfo.ReadAttrs[ValueNum].Disc.end(),tmp);
	assert(it!=CaseInfo.ReadAttrs[ValueNum].Disc.end());
	// {
	// 	string Msg="unexpected discrete value ";
 	// 	throw(CError(Msg+"'"+Name+"'"+" of column "+CzString::IntToStr(ValueNum),313,0));
	// }

	Item.Discr=(int)(it-CaseInfo.ReadAttrs[ValueNum].Disc.begin());
	return true;
}


//random select (no duplication)
//IN:	DataNum- size of target dataset
//OUT:	TrainSet- target dataset
bool CDataset::SubSet(int DataNum, CDataset &TrainSet) const
{
	//sub set of a data set
	int FinalSize=DataNum;
	if(DataNum>CaseInfo.Height)
		FinalSize=CaseInfo.Height;

	TrainSet.Matrix.clear();
	CRandSequence RandSequence(FinalSize);
	for(int i=0;i<FinalSize;i++)
	{
		int TrainNum=RandSequence.Poll();
		TrainSet.Matrix.push_back(Matrix[TrainNum]);
	}

	TrainSet.CaseInfo=CaseInfo;
	TrainSet.CaseInfo.Height=FinalSize;
	//TrainSet.CreatingTime=0;
	return true;
}

//caution: when we do bootstrapping, sub-setting or splitting, 
//we haven't re-calculate the max or min information of a continuous attribute

//bootstrap re-sampling
//IN:	DataNum- size of target dataset
//OUT:	TrainSet- target dataset
bool CDataset::BootStrap(int DataNum, CDataset &TrainSet) const
{
	TrainSet.Matrix.clear();
	for(int i=0;i<DataNum;i++)
	{
		int TrainNum=IntRand(CaseInfo.Height);
		TrainSet.Matrix.push_back(Matrix[TrainNum]);
	}

	TrainSet.CaseInfo=CaseInfo;
	TrainSet.CaseInfo.Height=DataNum;
	//TrainSet.CreatingTime=0;
	return true;
}

//weighted bootstrap: re-sampling with considering instances' weights
//IN:	Weights- weights for instances in this data set
//		DataNum- size of target dataset
//OUT:	TrainSet- target dataset
//		OrginalPos- original position for instances of new-created dataset
bool CDataset::BootStrap(const std::vector<double> &Weights,int DataNum,std::vector<int> &OrginalPos,CDataset &TrainSet) const
{
	if((int)Weights.size()!=CaseInfo.Height)
		return false;

	int TrainNum=0;
	TrainSet.Matrix.clear();
	TrainSet.CaseInfo=CaseInfo;
	TrainSet.CaseInfo.Height=DataNum;
	//TrainSet.CreatingTime=0;

	OrginalPos.clear();

	CRoulette Roult(Weights);

	//select instances by roulette
	for(int i=0;i<DataNum;i++)
	{
		TrainNum=Roult.Poll();
		if(TrainNum>=CaseInfo.Height)
			TrainNum=CaseInfo.Height-1;
		TrainSet.Matrix.push_back(Matrix[TrainNum]);
		OrginalPos.push_back(TrainNum);
	}

	return true;
}

//sampling- the input dataset is randomly split into two parts
//IN:	DataNum- size of target dataset
//OUT:	TrainSet- target dataset
//		TestSet- rest instances
bool CDataset::SplitData(int DataNum,CDataset &TrainSet,CDataset &TestSet) const
{
	const int CaseNum=CaseInfo.Height;
	if(DataNum>=CaseNum || DataNum<=0)
		return false;
	const int TestNum=CaseNum-DataNum;

	//test set first, because it is always less than train set
	TestSet.Matrix.clear();
	TestSet.CaseInfo=CaseInfo;
	TestSet.CaseInfo.Height=TestNum;
	//TestSet.CreatingTime=0;

	//which has been selected
	std::vector<int> SelFlag(CaseNum,0);
	int WaitForSelect=CaseNum;
	//randomly select instances from original dataset(no duplicate)
	for(int i=0;i<TestNum;i++)
	{
		//number of instance wait for selecting
		int Selected=IntRand(WaitForSelect);
		//find the TrainNum-th unselected instance
		int Pos=0;
		int j;
		for(j=0;j<CaseNum;j++)
		{
			if(SelFlag[j]>0)//is a un-sampled instances?
				continue;
			if(Pos++>=Selected)
				break;
		}
		//found
		SelFlag[j]++;
		WaitForSelect--;
		TestSet.Matrix.push_back(Matrix[j]);
	}

	//the remaining instances are putted into TrainSet
	TrainSet.Matrix.clear();
	TrainSet.CaseInfo=CaseInfo;
	//TrainSet.CreatingTime=0;
	for(int j=0;j<CaseNum;j++)
		if(SelFlag[j]<=0)
			TrainSet.Matrix.push_back(Matrix[j]);
	TrainSet.CaseInfo.Height=(int)TrainSet.Matrix.size();

	return true;
}

//sampling- the input dataset is randomly split into several train-sets and one test-set
//IN:	DataNum- size of each target dataset
//		SetNum- number of target datasets
//OUT:	TrainSet- target dataset
//		TestSet- rest instances
bool CDataset::SplitData(int DataNum,int SetNum,std::vector<CDataset> &TrainSets,CDataset &TestSet) const
{
	//Test set is allowed to be null
	assert(DataNum*SetNum>=CaseInfo.Height);
		// throw(CError("Not enough instances!",501,0));

	TrainSets.clear();
	//	srand((unsigned)time(NULL));
	//flags identifying instances has been selected
	int CaseNum=CaseInfo.Height;
	std::vector<int> SelFlag(CaseInfo.Height,0);
	for(int k=0;k<SetNum;k++)
	{
		{
			CDataset TrainSet;
			TrainSet.CaseInfo=CaseInfo;
			TrainSet.CaseInfo.Height=DataNum;
			//TrainSet.CreatingTime=0;
			TrainSets.push_back(TrainSet);
		}
		//randomly select instances from original dataset(no dup)
		for(int i=0;i<DataNum;i++)
		{
			//number of instance for selecting
			int TrainNum=IntRand(CaseNum);
			//find the TrainNum-th unselected instance
			int Pos=0;
			int j;
			for(j=0;j<CaseInfo.Height;j++)
			{
				if(SelFlag[j]>0)//is a un-sampled instances?
					continue;
				if(Pos++>=TrainNum)
					break;
			}
			//found
			SelFlag[j]++;
			CaseNum--;
			TrainSets[k].Matrix.push_back(Matrix[j]);
		}
	}

	//the remaining instances are putted into TestSet
	TestSet.Matrix.clear();
	TestSet.CaseInfo=CaseInfo;
	//TestSet.CreatingTime=0;
	for(int j=0;j<CaseInfo.Height;j++)
		if(SelFlag[j]<=0)
			TestSet.Matrix.push_back(Matrix[j]);
	TestSet.CaseInfo.Height=(int)TestSet.Matrix.size();
	//
	assert(CaseNum>=0);
		//throw(CError("Not enough instances!",501,0));

	return true;
}

//sampling- the input dataset is split into several new sets, from beginning to end
//IN:	SetNum- number of target datasets
//OUT:	TrainSets- target dataset
bool CDataset::DevideBySetNum(int SetNum,std::vector<CDataset> &TrainSets) const
{
	//Parameters
	const int CaseNum=CaseInfo.Height;
	assert(SetNum>=CaseNum);
		//throw(CError("CDataset::DevideBySetNum: Not enough instances!",502,0));

	TrainSets.clear();
	//in case instances can not evenly put into all sets
	for(int k=0;k<SetNum;k++)
	{
		{
			CDataset TrainSet;
			TrainSet.CaseInfo=CaseInfo;
			//TrainSet.CreatingTime=0;
			TrainSets.push_back(TrainSet);
		}
		//sometimes data can not evenly put into each set, we just make it as even as possible
		int Start=(int)(1.0*k*CaseNum/SetNum);
		int End=(int)(1.0*(k+1)*CaseNum/SetNum);
		for(int i=Start;i<End;i++)
			TrainSets[k].Matrix.push_back(Matrix[i]);

		TrainSets[k].CaseInfo.Height=(int)TrainSets[k].Matrix.size();
	}

	return true;
}

//sampling- the input dataset is split into several new sets, from beginning to end
//IN:	DataNum- number of data in each target dataset
//OUT:	TrainSets- target dataset
bool CDataset::DevideByDataNum(int DataNum,std::vector<CDataset> &TrainSets) const
{
	//Parameters
	const int CaseNum=CaseInfo.Height;

	TrainSets.clear();
	//in case instances can not evenly put into all sets
	int DataPos=0;
	while(DataPos<CaseNum)
	{
		{
			CDataset TrainSet;
			TrainSet.CaseInfo=CaseInfo;
			//TrainSet.CreatingTime=0;
			TrainSets.push_back(TrainSet);
		}
		//sometimes data can not evenly put into each set, we just make it as even as possible
		for(int i=0;DataPos<CaseNum && i<DataNum;i++,DataPos++)
			TrainSets.back().Matrix.push_back(Matrix[DataPos]);

		TrainSets.back().CaseInfo.Height=(int)TrainSets.back().Matrix.size();
	}

	return true;
}

//IN: a,b- position of two instances to be swapped
//		nothing is done for invalid input
bool CDataset::SwapInstance(int a,int b)
{
	int DataNum=(int)CaseInfo.Height;
	if(a==b)
		return true;
	if(a>=DataNum || b>=DataNum ||
		a<0 || b<0)
		return false;

	InstanceStr Hold;
	Hold=Matrix[a];
	Matrix[a]=Matrix[b];
	Matrix[b]=Hold;

	return true;
}

//insert instances in the end
//should remove all ignored attributes, transform discrete values and labels into number (start from 0)
void CDataset::Insert(const InstanceStr &Instance)
{
	assert((int)Instance.size()==CaseInfo.ValidWidth);
		// throw(CError("Invalid data!",601,0));

	//attributes
	for(int i=0;i<CaseInfo.ValidWidth-1;i++)
	{
		switch(CaseInfo.ValidAttrs[i].AttType)
		{
		case ATT_DISCRETE:
			assert(Instance[i].Discr>=0 || Instance[i].Discr<(int)CaseInfo.ValidAttrs[i].Disc.size());
				// throw(CError("Invalid discrete data!",602,0));
			break;
		case ATT_CONTINUOUS:
		case ATT_DATETIME:
			{
				int k=CaseInfo.ValidAttrs[i].OtherPos;
				if(!CaseInfo.ValidAttrs[i].MMSet)
				{
					CaseInfo.ValidAttrs[i].MMSet=true;
					CaseInfo.ValidAttrs[i].Max=CaseInfo.ValidAttrs[i].Min=Instance[i].Cont;

					CaseInfo.ReadAttrs[k].MMSet=true;
					CaseInfo.ReadAttrs[k].Max=CaseInfo.ReadAttrs[k].Min=Instance[i].Cont;
				}
				else if(Instance[i].Cont<CaseInfo.ValidAttrs[i].Min)
				{
					CaseInfo.ValidAttrs[i].Min=Instance[i].Cont;
					CaseInfo.ReadAttrs[k].Min=Instance[i].Cont;
				}
				else if(Instance[i].Cont>CaseInfo.ValidAttrs[i].Max)
				{
					CaseInfo.ValidAttrs[i].Max=Instance[i].Cont;
					CaseInfo.ReadAttrs[k].Max=Instance[i].Cont;
				}
			}
			break;
		default:
			// throw(CError("Invalid data description!",602,0));
			break;
		}
	}
	assert(Instance[CaseInfo.ValidWidth-1].Discr>=0 || Instance[CaseInfo.ValidWidth-1].Discr<(int)CaseInfo.ClassNum);
		// throw(CError("Invalid label!",601,0));

	Matrix.push_back(Instance);
	CaseInfo.Height++;
}

//remove an instance
void CDataset::Remove(int Pos)
{
	if(CaseInfo.Height>0&&Pos>=0&&Pos<CaseInfo.Height)
		Matrix.erase(Matrix.begin()+Pos);
}

//remove all instances
void CDataset::ClearData()
{
	CaseInfo.Height=0;
	Matrix.clear();
}


//IN:	Att- number of attribute in ValidAttrs 
//		t- test value
//OUT:	the greatest value of Att-th attribute which is no larger than t
double CDataset::GreatestValBelow(int Att, const double &t) const
{
	if(t<CaseInfo.ValidAttrs[Att].Min)
		return CaseInfo.ValidAttrs[Att].Min;

	double v, Best;
	bool HasSet=false;

	for(int i=0;i<CaseInfo.Height;i++)
	{
		v=Matrix[i][Att].Cont;
		if(v<=t)
		{
			if(!HasSet)
			{
				HasSet=true;
				Best=v;
			}
			else if(v>Best)
				Best=v;
		}
	}

	return Best;
}

//copy
CDataset::CDataset(const CDataset &Dataset)
{
	//CreatingTime=0;
	*this=Dataset;
}

//create a new dataset, by extending every multi-valued discrete attribute into multi boolean attributes(needed by BPNN and/or SVM)
//notice: new data set don't correspond to any file
CDataset *CDataset::ExpandDiscrete() const
{
	clock_t start=clock();
	//prepare
	CDataset *newDataSet=new CDataset(*this);

	//only process discrete attributes
	{
		int j;
		//skip the label
		for(j=CaseInfo.ValidWidth-2;j>=0;j--)
			if(CaseInfo.ValidAttrs[j].AttType==ATT_DISCRETE)
				break;
		//just copy this one if no discrete attribute
		if(j<0)
			return newDataSet;
	}


	//need to modify instances and their description
	//remove all single-value attributes to run faster
	newDataSet->CaseInfo.ValidAttrs.clear();
	newDataSet->Matrix.clear();
	//including attributes and label
	for(int j=0;j<CaseInfo.ValidWidth;j++)
	{
		if(CaseInfo.ValidAttrs[j].AttType==ATT_DISCRETE)
		{
			//transform into continuous attributes
			AttrStr Attr;
			Attr.AttType=ATT_CONTINUOUS;
			Attr.Max=1;
			Attr.Min=0;
			Attr.MMSet=true;

			int ValueNum=(int)CaseInfo.ValidAttrs[j].Disc.size();
			//transformed into continuous attribute directly
			if(ValueNum<=2)
			{
				Attr.Name=CaseInfo.ValidAttrs[j].Name;
				newDataSet->CaseInfo.ValidAttrs.push_back(Attr);
			}
			else
			{
				for(int k=0;k<ValueNum;k++)
				{
					Attr.Name=CaseInfo.ValidAttrs[j].Name+"_"+CaseInfo.ValidAttrs[j].Disc[k].Name;
					newDataSet->CaseInfo.ValidAttrs.push_back(Attr);
				}
			}
		}
		//keep non discrete attribute
		else
		{
			newDataSet->CaseInfo.ValidAttrs.push_back(CaseInfo.ValidAttrs[j]);
		}
	}//attributes
	newDataSet->CaseInfo.ValidWidth=(int)newDataSet->CaseInfo.ValidAttrs.size();


	//instances
	for(int i=0;i<CaseInfo.Height;i++)
	{
		//single instance
		InstanceStr Inst;

		//process all attributes and the label
		for(int j=0;j<CaseInfo.ValidWidth;j++)
		{
			if(CaseInfo.ValidAttrs[j].AttType==ATT_DISCRETE)
			{
				ValueData Value=Matrix[i][j];

				int ValueNum=(int)CaseInfo.ValidAttrs[j].Disc.size();
				//transformed into continuous attribute directly
				if(ValueNum<=2)
				{
					Value.Cont=(double)Value.Discr;
					Inst.push_back(Value);
				}
				else
				{
					for(int k=0;k<ValueNum;k++)
					{
						Value.Cont=0;
						if(Matrix[i][j].Discr==k)
							Value.Cont=1;
						Inst.push_back(Value);
					}
				}
			}
			else
			{
				Inst.push_back(Matrix[i][j]);
			}
		}//attributes

		//insert this new instance
		newDataSet->Matrix.push_back(Inst);
	}//instances

	//newDataSet->CreatingTime=(double)(clock()-start)/CLOCKS_PER_SEC;
	return newDataSet;
}

//A training set must not contain instances with unknown label
void CDataset::RemoveUnknownInstance()
{
	for(int i=CaseInfo.Height-1;i>=0;i--)
	{
		//unknown label?
		if(Matrix[i][CaseInfo.ValidWidth-1].Discr==-1)
			Matrix.erase(Matrix.begin()+i);
	}

	CaseInfo.Height=(int)Matrix.size();
}

//remove the attribute which has only a value
//don't use it on a expanded data set
void CDataset::RemoveNullAttribute()
{
	//skip the labels
	bool Changed=false;
	for(int i=CaseInfo.ValidWidth-2;i>=0;i--)
	{
		if(CaseInfo.ValidAttrs[i].Max==CaseInfo.ValidAttrs[i].Min)
		{
			for(int j=0;j<CaseInfo.Height;j++)
				Matrix[j].erase(Matrix[j].begin()+i);
			CaseInfo.ValidAttrs.erase(CaseInfo.ValidAttrs.begin()+i);
			Changed=true;
		}
	}
	if(Changed)
	{
		CaseInfo.ValidWidth=(int)CaseInfo.ValidAttrs.size();
		if(CaseInfo.ValidWidth==0)
		{
			Matrix.clear();
			CaseInfo.Height=0;
		}
		//the data set can't be used to read from file any more
		CaseInfo.ReadAttrs.clear();
		CaseInfo.ReadWidth=0;
	}
}

const MATRIX &CDataset::GetData() const
{
	return Matrix;
}
const CASE_INFO &CDataset::GetInfo() const
{
	return CaseInfo;
}

bool CDataset::AllContinuous() const
{
	for(int i=0;i<CaseInfo.ValidWidth-1;i++)
		if(CaseInfo.ValidAttrs[i].AttType==ATT_DISCRETE)
			return false;

	return true;
}

