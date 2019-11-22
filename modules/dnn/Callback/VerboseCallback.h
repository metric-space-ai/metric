#ifndef CALLBACK_VERBOSECALLBACK_H_
#define CALLBACK_VERBOSECALLBACK_H_

#include <iostream>
#include "../Callback.h"
#include "../Network.h"

namespace MiniDNN
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
		using Matrix = blaze::DynamicMatrix<Scalar, blaze::columnMajor>;
		using IntegerVector = blaze::DynamicVector<int, blaze::rowVector>;

    public:
        void post_training_batch(const Network<Scalar>* net, const Matrix& x, const Matrix& y)
        {
            const Scalar loss = net->get_output()->loss();
            std::cout << "[Epoch " << this->m_epoch_id << ", batch " << this->m_batch_id << "] Loss = "
                      << loss << std::endl;
        }

        void post_training_batch(const Network<Scalar>* net, const Matrix& x,
                                 const IntegerVector& y)
        {
            Scalar loss = net->get_output()->loss();
            std::cout << "[Epoch " << this->m_epoch_id << ", batch " << this->m_batch_id << "] Loss = "
                      << loss << std::endl;
        }
};


} // namespace MiniDNN


#endif /* CALLBACK_VERBOSECALLBACK_H_ */
