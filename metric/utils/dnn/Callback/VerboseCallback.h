#ifndef CALLBACK_VERBOSECALLBACK_H_
#define CALLBACK_VERBOSECALLBACK_H_

#include <chrono>
#include <iostream>

#include "../Callback.h"
#include "../Network.h"

namespace metric {
namespace dnn {

///
/// \ingroup Callbacks
///
/// Callback function that prints the loss function value in each mini-batch training
///
template <typename Scalar> class VerboseCallback : public Callback<Scalar> {
  private:
	using typename Callback<Scalar>::Matrix;
	using typename Callback<Scalar>::IntegerVector;

	using Clock = std::chrono::high_resolution_clock;

	bool verboseEpoch;
	bool verboseBatch;

	Clock::time_point epochStart;
	Clock::time_point batchStart;

  public:
	VerboseCallback(bool verboseEpoch = true, bool verboseBatch = true)
		: verboseEpoch(verboseEpoch), verboseBatch(verboseBatch)
	{
	}

	void preTrainingEpoch(const Network<Scalar> *net)
	{
		if (!verboseEpoch) {
			return;
		}

		epochStart = Clock::now();
	}

	void postTrainingEpoch(const Network<Scalar> *net)
	{
		if (!verboseEpoch) {
			return;
		}

		const Scalar loss = net->get_output()->loss();

		std::cout << "[Epoch " << this->epochId << "/" << this->epochsNumber << "] Loss = " << loss;

		auto epochEnd = Clock::now();
		auto d = std::chrono::duration_cast<std::chrono::duration<double>>(epochEnd - epochStart);
		std::cout << " Training time: " << d.count() << " s" << std::endl;
	}

	void preTrainingBatch(const Network<Scalar> *net, const Matrix &x, const Matrix &y)
	{
		if (!verboseBatch) {
			return;
		}

		batchStart = Clock::now();
	}

	void postTrainingBatch(const Network<Scalar> *net, const Matrix &x, const Matrix &y)
	{
		if (!verboseBatch) {
			return;
		}

		const Scalar loss = net->get_output()->loss();

		std::cout << "[Epoch " << this->epochId << "/" << this->epochsNumber;
		std::cout << ", batch " << this->batchId << "/" << this->batchesNumber << "] Loss = " << loss;

		auto batchEnd = Clock::now();
		auto d = std::chrono::duration_cast<std::chrono::duration<double>>(batchEnd - batchStart);
		std::cout << " Training time: " << d.count() << " s" << std::endl;
	}

	/*void postTrainingBatch(const Network<Scalar>* net, const Matrix& x,
						   const IntegerVector& y)
	{
		Scalar loss = net->get_output()->loss();
		std::cout << "[Epoch " << this->epochId << "/" << this->epochsNumber;
		std::cout << ", batch " << this->batchId << "/" << this->batchesNumber << "] Loss = "
				  << loss << std::endl;
	}*/
};

} // namespace dnn
} // namespace metric

#endif /* CALLBACK_VERBOSECALLBACK_H_ */
