#ifndef OPTIMIZER_SGD_H_
#define OPTIMIZER_SGD_H_

#include "../Optimizer.h"


namespace metric
{
namespace dnn
{


///
/// \ingroup Optimizers
///
/// The Stochastic Gradient Descent (SGD) algorithm
///
template <typename Scalar>
class SGD: public Optimizer<Scalar>
{
    private:
		using Vector = blaze::DynamicVector<Scalar>;
		using AlignedMapVec = blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;
		using ConstAlignedMapVec = const blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;

	public:
        Scalar m_lrate;
        Scalar m_decay;

        SGD() :
            m_lrate(Scalar(0.01)), m_decay(Scalar(0))
        {}

        void update(ConstAlignedMapVec& dvec, AlignedMapVec& vec)
        {
            vec -= m_lrate * (dvec + m_decay * vec);
        }
};


} // namespace dnn
} // namespace metric


#endif /* OPTIMIZER_SGD_H_ */
