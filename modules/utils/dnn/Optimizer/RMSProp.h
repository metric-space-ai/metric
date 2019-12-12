#ifndef OPTIMIZER_RMSPROP_H_
#define OPTIMIZER_RMSPROP_H_

#include "../Optimizer.h"
#include "../Utils/sparsepp.h"

namespace dnn
{


///
/// \ingroup Optimizers
///
/// The RMSProp algorithm
///
template <typename Scalar>
class RMSProp: public Optimizer<Scalar>
{
    private:
		using Array = blaze::DynamicVector<Scalar>;
		using AlignedMapVec = blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;
		using ConstAlignedMapVec = const blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;

	spp::sparse_hash_map<const Scalar*, Array> m_history;

    public:
        Scalar learningRate;
        Scalar m_eps;
        Scalar m_decay;

        RMSProp() :
            learningRate(Scalar(0.01)), m_eps(Scalar(1e-6)), m_decay(Scalar(0.9))
        {}

        void reset()
        {
            m_history.clear();
        }

        void update(ConstAlignedMapVec& dvec, AlignedMapVec& vec)
        {
            // Get the accumulated squared gradient associated with this gradient
            Array& grad_square = m_history[dvec.data()];

            // If length is zero, initialize it
            if (grad_square.size() == 0)
            {
                grad_square.resize(dvec.size());
                grad_square = 0;
            }

            // Update accumulated squared gradient
            grad_square = m_decay * grad_square + (Scalar(1) - m_decay) *
                          blaze::pow(dvec, 2);
            // Update parameters
            vec -= learningRate * dvec / blaze::sqrt(grad_square + m_eps);
        }
};


} // namespace dnn


#endif /* OPTIMIZER_RMSPROP_H_ */
