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

#ifndef SVM_Head_File
#define SVM_Head_File

#include "svm.h"

#define pure_fill(vt, h, w, a)                                                                                         \
	for (int i = 0; i < h; i++) {                                                                                      \
		vector<double> tmp;                                                                                            \
		for (int j = 0; j < w; j++)                                                                                    \
			tmp.push_back(a);                                                                                          \
		vt.push_back(tmp);                                                                                             \
	}

#define new_d1(type, arr, w, a)                                                                                        \
	type *arr = new type[w];                                                                                           \
	for (int i = 0; i < w; i++)                                                                                        \
		arr[i] = a;
#define fill_d1(type, arr, w, a)                                                                                       \
	{                                                                                                                  \
		arr = new type[w];                                                                                             \
		for (int i = 0; i < w; i++)                                                                                    \
			arr[i] = a;                                                                                                \
	}
#define free_d1(arr)                                                                                                   \
	if (arr != NULL)                                                                                                   \
		delete[] arr;

#define new_d2(type, arr, h, w, a)                                                                                     \
	type **arr = new type *[h];                                                                                        \
	for (int i = 0; i < h; i++)                                                                                        \
		arr[i] = new type[w];                                                                                          \
	for (int i = 0; i < h; i++)                                                                                        \
		for (int j = 0; j < w; j++)                                                                                    \
			arr[i][j] = a;
#define fill_d2(type, arr, h, w, a)                                                                                    \
	{                                                                                                                  \
		arr = new type *[h];                                                                                           \
		for (int i = 0; i < h; i++)                                                                                    \
			arr[i] = new type[w];                                                                                      \
		for (int i = 0; i < h; i++)                                                                                    \
			for (int j = 0; j < w; j++)                                                                                \
				arr[i][j] = a;                                                                                         \
	}
#define free_d2(arr, h)                                                                                                \
	if (arr != NULL) {                                                                                                 \
		for (int i = 0; i < h; i++)                                                                                    \
			if (arr[i] != NULL)                                                                                        \
				delete[] arr[i];                                                                                       \
		delete[] arr;                                                                                                  \
	}

namespace libedm {

class CClassifier;
class CDataset;
class CPrediction;

// encapsulation of libsvm
class CSVM : public CClassifier {
  public:
	virtual ~CSVM();
	virtual CPrediction *Classify(const CDataset &DataSet) const;
	virtual bool Dump(const std::string &FileName) const;
	virtual CClassifier *Clone() const
	{
		CSVM *Cls = new CSVM(*this);
		return Cls;
	}

	static std::string GetStaticName() { return StaticName; };

  public:
	CSVM(const CDataset &TrainData, int usvm_type = C_SVC, int ukernel_type = RBF, int udegree = 3, double ucoef0 = 0,
		 double ucache_size = 100, double ueps = 0.001, double uC = 1, int unr_weight = 0, int *uweight_label = NULL,
		 double *uweight = NULL, double unu = 0.5, double up = 0.1, int ushrinking = 1, int uprobability = 0);

	typedef struct svm_parameter SVMParamStr;
	static CClassifier *Create(const CDataset &TrainData, const void *Params)
	{
		if (Params == NULL)
			return new CSVM(TrainData);

		SVMParamStr *SVMParam = (SVMParamStr *)Params;
		return new CSVM(TrainData, SVMParam->svm_type, SVMParam->kernel_type, SVMParam->degree, SVMParam->coef0,
						SVMParam->cache_size, SVMParam->eps, SVMParam->C, SVMParam->nr_weight, SVMParam->weight_label,
						SVMParam->weight, SVMParam->nu, SVMParam->p, SVMParam->shrinking, SVMParam->probability);
	}

	// libsvm has sth wrong in saving models in files, so these two functions may not run well
	int Save(const std::string &Path, const std::string &FileName) const;
	static CClassifier *FileCreate(const std::string &Path, const std::string &FileName)
	{
		return new CSVM(Path, FileName);
	}
	// svm must work with the training set
	CSVM(const std::string &Path, const std::string &FileName);

  private:
	void PrepareData(const CDataset &TrainSet, struct svm_problem &DataDesc);
	// 		bool SaveDataDesc(const string &Path,const string &FileName) const;
	// 		bool LoadDataDesc(const string &Path,const string &FileName);
	CSVM(){};
	CSVM(const CSVM &a)
	{
		AttributeNum = a.AttributeNum;
		// svm model
		svm = new (struct svm_model);
		*svm = *a.svm;
		// 			svm->param=a.svm->param;
		fill_d1(double, svm->param.weight, svm->param.nr_weight, a.svm->param.weight[i]);
		fill_d1(int, svm->param.weight_label, svm->param.nr_weight, a.svm->param.weight_label[i]);

		// 			svm->nr_class=a.svm->nr_class;
		// 			svm->l=a.svm->l;
		// 			svm->free_sv=a.svm->free_sv;

		if (a.svm->rho != NULL)
			fill_d1(double, svm->rho, svm->nr_class *(svm->nr_class - 1) / 2, a.svm->rho[i]);
		if (a.svm->label != NULL)
			fill_d1(int, svm->label, svm->nr_class, a.svm->label[i]);
		if (a.svm->probA != NULL)
			fill_d1(double, svm->probA, svm->nr_class *(svm->nr_class - 1) / 2, a.svm->probA[i]);
		if (a.svm->probB != NULL)
			fill_d1(double, svm->probB, svm->nr_class *(svm->nr_class - 1) / 2, a.svm->probB[i]);
		if (a.svm->nSV != NULL)
			fill_d1(int, svm->nSV, svm->nr_class, a.svm->nSV[i]);
		if (a.svm->sv_coef != NULL)
			fill_d2(double, svm->sv_coef, svm->nr_class - 1, svm->l, a.svm->sv_coef[i][j]);
		// support vectors
		if (a.svm->SV != NULL) {
			svm->SV = new struct svm_node *[svm->l];
			for (int i = 0; i < svm->l; i++)
				svm->SV[i] = new struct svm_node[AttributeNum + 1];
			for (int i = 0; i < svm->l; i++)
				for (int j = 0; j < AttributeNum + 1; j++) {
					svm->SV[i][j] = a.svm->SV[i][j];
					if (svm->SV[i][j].index == -1)
						break;
				}
		}
	};

  private:
	const static std::string StaticName;

	struct svm_model *svm;

	int AttributeNum;
};
} // namespace libedm

#include "b-svm.cpp"

#endif
