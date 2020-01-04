#ifndef CALLBACK_VERBOSECALLBACK_H_
#define CALLBACK_VERBOSECALLBACK_H_

#include <iostream>
#include "../Callback.h"
#include "../Network.h"

namespace metric
{
namespace dnn
{


///
/// \ingroup Callbacks
///
/// Callback function that prints the loss function value in each mini-batch training
///
template <typename Scalar>
class VerboseCallback: public Callback<Scalar>
{
	private:
		using typename Callback<Scalar>::Matrix;
		using typename Callback<Scalar>::IntegerVector;

    public:
        void postTrainingBatch(const Network<Scalar>* net, const Matrix& x, const Matrix& y)
        {
            const Scalar loss = net->get_output()->loss();
            std::cout << "[Epoch " << this->epochId << "/" << this->epochsNumber;
            std::cout << ", batch " << this->batchId << "/" << this->batchesNumber << "] Loss = "
                      << loss << std::endl;
        }

        void postTrainingBatch(const Network<Scalar>* net, const Matrix& x,
                               const IntegerVector& y)
        {
            Scalar loss = net->get_output()->loss();
            std::cout << "[Epoch " << this->epochId << "/" << this->epochsNumber;
            std::cout << ", batch " << this->batchId << "/" << this->batchesNumber << "] Loss = "
                      << loss << std::endl;
        }
};


} // namespace dnn
} // namespace metric


#endif /* CALLBACK_VERBOSECALLBACK_H_ */
