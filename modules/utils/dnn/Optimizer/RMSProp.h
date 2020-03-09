#ifndef OPTIMIZER_RMSPROP_H_
#define OPTIMIZER_RMSPROP_H_

#include "unordered_map"

#include "../Optimizer.h"


namespace metric::dnn
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
		using typename Optimizer<Scalar>::RowVector;
		using typename Optimizer<Scalar>::ColumnMatrix;

		using Array = blaze::DynamicVector<Scalar>;
		using AlignedMapVec = blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;
		using ConstAlignedMapVec = const blaze::CustomVector<Scalar, blaze::aligned, blaze::unpadded>;

		std::unordered_map<const Scalar*, Array> m_history;
		std::unordered_map<const RowVector*, RowVector> historyVectors;
		std::unordered_map<const ColumnMatrix*, ColumnMatrix> historyMatrices;

    public:
        Scalar learningRate;
        Scalar m_eps;
        Scalar m_decay;

        RMSProp(Scalar learningRate = Scalar(0.01), Scalar eps = Scalar(1e-6), Scalar decay = Scalar(0.9)) :
									learningRate(learningRate), m_eps(eps), m_decay(decay)
        {}

		nlohmann::json toJson()
		{
        	nlohmann::json json;
        	json["type"] = "RMSProp";
        	json["learningRate"] = learningRate;
			json["eps"] = m_eps;
			json["decay"] = m_decay;

			return json;
		}


		void reset()
        {
            m_history.clear();
        }

        void update(ConstAlignedMapVec& dvec, AlignedMapVec& vec)
        {
            // Get the accumulated squared gradient associated with this gradient
            auto& grad_square = m_history[dvec.data()];
            //std::cout << "history:" << m_history.size() << std::endl;

            // If length is zero, initialize it
	        if (grad_square.size() == 0) {
		        grad_square.resize(dvec.size());
		        grad_square = 0;
	        }

	        // Update accumulated squared gradient
	        grad_square = m_decay * grad_square + (Scalar(1) - m_decay) *
	                                              blaze::pow(dvec, 2);
            // Update parameters
            vec -= learningRate * dvec / blaze::sqrt(grad_square + m_eps);
        }

		void update(const RowVector& dvec, RowVector& vec)
		{
			auto& grad_square = historyVectors[&dvec];
			//std::cout << "RowVector history:" << historyVectors.size() << std::endl;

			if (grad_square.size() == 0) {
				grad_square.resize(dvec.size());
				grad_square = 0;
			}

			grad_square = m_decay * grad_square + (Scalar(1) - m_decay) *
												  blaze::pow(dvec, 2);
			vec -= learningRate * dvec / blaze::sqrt(grad_square + m_eps);
		}

		void update(const ColumnMatrix& dvec, ColumnMatrix& vec)
		{
			auto& grad_square = historyMatrices[&dvec];
			//std::cout << "ColumnMatrix history:" << historyVectors.size() << std::endl;

			if (blaze::isDefault(grad_square)) {
				grad_square.resize(dvec.rows(), dvec.columns());
				grad_square = 0;
			}

			grad_square = m_decay * grad_square + (Scalar(1) - m_decay) * blaze::pow(dvec, 2);
			for (size_t j = 0; j < dvec.columns(); ++j) {
				blaze::column(vec, j) -= learningRate * blaze::column(dvec, j) / blaze::column(blaze::sqrt(grad_square + m_eps), j);
			}
		}
};


} // namespace metric::dnn


#endif /* OPTIMIZER_RMSPROP_H_ */
