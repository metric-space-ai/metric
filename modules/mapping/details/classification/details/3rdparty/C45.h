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


#ifndef  C45_Head_File
#define  C45_Head_File

#include <fstream>

namespace libedm
{

    class   CC45 : public CClassifier
	{
	private:
		//node type
		#define BrLeaf			0//leaf
		#define BrDiscr			1//discrete
		#define ThreshContin	2//continuous attribute divided by a threshold
		#define BrSubset		3//subset test

//		static const int BrSubset;	

		//node description
		typedef  struct NodeStr
		{
			int		NodeType;	//Type: 0=leaf 1=branch 2=cut
			int		Attr; 		//Attribute used in this node (to split instances) 
			int		BestClass;	//most class of the instances in this node, for a leaf used to predict a unknown instance
			int		Forks;		//number of sub nodes
			int		ClassNum;	//number of class label(used when saving tree to file)
			double	Items;		//number of instances passed this node
			double	Cut;		//for continuous attribute: threshold of cutting
			double	Errors;		//number of prediction error (number of instances that is not in the most class)
			double	*ClassDist;	//distribution of all instances to each class labels 
			NodeStr *SubNodes;	//sub nodes

			NodeStr &operator =(const NodeStr &a)
			{
				if(this==&a)  
					return *this;  

				NodeType=a.NodeType;
				Attr=a.Attr;
				BestClass=a.BestClass;
				Forks=a.Forks;
				ClassNum=a.ClassNum;
				Items=a.Items;
				Cut=a.Cut;
				Errors=a.Errors;

				ClassDist=new double[ClassNum];
				for(int i=0;i<ClassNum;i++)
					ClassDist[i]=a.ClassDist[i];

				if(Forks!=0)
				{
					SubNodes=new NodeStr[Forks];
					for(int i=0;i<Forks;i++)
						SubNodes[i]=a.SubNodes[i];
				}
				else
					SubNodes=NULL;

				return *this;  
			};
		}NodeStr;

		//for a discrete attribute
		typedef struct ValueInfoStr
		{
			//used by C45
			std::vector<double>	ClassFreq;//appearance frequency of each value to each class label
			double			Freq;//frequency of each value
		}ValueInfoStr;
		//info of attribute
		typedef struct AttrInfoStr
		{
			int		Tested;//appearance number of this attribute in the tree
			double	Gain;//entropy gain
			double	Info;//entropy
			double	Bar;//used to split a continuous attribute

			bool	MVals;//for a discrete, dos it have too many values?
		}AttrInfoStr;
		typedef struct DataInfoStr
		{
			double	Weight;//used for unknown instances, now it's 1
			double	SplitGain;
			double	SplitInfo;
		}DataInfoStr;
		//for continuous attributes
		typedef struct ContValueStr
		{
			int		No;
			double	Value;
			int		Label;
			double	Weight;
			double	SplitGain;
			double	SplitInfo;
		}ContValueStr;
		static bool ValueAscOrder(const ContValueStr &a,const ContValueStr &b)
		{
			return (a.Value<b.Value);
		}

	public:
		typedef struct ParamStr
		{
			int		MINOBJS;//minimum instances in a node
			double	Epsilon;//minimum entropy gain
			double	CF;//upper limit of confidence level
			bool	WillPrune;
		}ParamStr;

	public:
		virtual ~CC45();
		virtual CPrediction *Classify(const CDataset &DataSet) const;
		// virtual bool Dump(const string &FileName) const;
		//virtual int Save(const string &Path,const string &FileName) const;
		virtual CClassifier *Clone() const;

		static std::string GetStaticName()
		{
			return StaticName;
		};

	public:
		CC45(const CDataset &TrainSet,int UMINOBJS=2,double UEpsilon=1e-3,double UCF=0.25,double WillPrune=true);
		static CClassifier *Create(const CDataset &TrainSet,const void* UParams)
		{
			if(UParams==NULL)
				return new CC45(TrainSet);

			ParamStr *Params=(ParamStr *)UParams;
			return new CC45(TrainSet,Params->MINOBJS,Params->Epsilon,Params->CF,Params->WillPrune);
		}

		CC45(const std::string &Path,const std::string &FileName);
		static CClassifier *FileCreate(const std::string &Path,const std::string &FileName)
		{
			return new CC45(Path,FileName);
		}

	private:
		CC45(){};
		CC45(const CC45 &a)
		{
			TheRoot=a.TheRoot;
			IsPruned=a.IsPruned;
		};

		//create a tree on partial of ordered instances set
		int FormTree(int First,int Last,NodeStr *Root);
		//prune the tree
		double Prune(NodeStr *Root,int Fp,int Lp,bool Update);
		//
		void DelTree(NodeStr *Root);

		// //load a tree from a saved file
		// void Load(ifstream &File,NodeStr *Root);
		// //record tree to a disk file
		// bool Save(ofstream &File,const NodeStr *Root) const;
		// //save file to disk
		// bool DumpTree(ofstream &File,int Layer,const NodeStr *Root) const;

		//predict a instance
		void ClassOneCase(const CDataset &DataSet,int i,const NodeStr *Root,double Weight,std::vector<double> &Prob) const;

		//get entropy
		double TotalInfo(const std::vector<double> &V);
		double TotalInfo(const std::vector<ValueInfoStr> &V);
		//find sub node's corresponding data set
		int Group(int V,int Fp,int Lp,NodeStr *TestNode);
 		//sum of instance weights
 		double CountItems(int Fp,int Lp);
		//
		void EvalContinuousAtt(int Att,int Fp,int Lp);
		void EvalDiscreteAtt(int Att,int Fp,int Lp);
		double AddErrs(double N,double e);
	private:
		//Root of the tree
		NodeStr	TheRoot;
		//be pruned?
		bool	IsPruned;

		//are all attributes multiple values?
		bool IsAllAttrMVals;
		//information of all attributes
		std::vector<AttrInfoStr> AttrInfos;
		//training set: we need to modify the instances, so we make a copy of original data set
		CDataset TrainSet;
		std::vector<DataInfoStr> DataInfos;

	private:
		const static std::string StaticName;

		int		MINOBJS;//minimum instances in a node
		double	Epsilon;//minimum entropy gain
		double	CF;//upper limit of confidence level
	};
}//namespace

#include "C45.cpp"

#endif

