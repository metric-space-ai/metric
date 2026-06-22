#ifndef OPTIMIZER_SGD_H_
#define OPTIMIZER_SGD_H_

#include "../Optimizer.h"

namespace mtrc::solve::parametric::dnn {

///
/// \ingroup Optimizers
///
/// The Stochastic Gradient Descent (SGD) algorithm
///
template <typename Scalar> class SGD : public Optimizer<Scalar> {
  private:
	using typename Optimizer<Scalar>::ColumnMatrix;
	using typename Optimizer<Scalar>::RowVector;
	using AlignedMapVec = mtrc::numeric::CustomVector<Scalar, mtrc::numeric::aligned, mtrc::numeric::unpadded>;
	using ConstAlignedMapVec =
		const mtrc::numeric::CustomVector<Scalar, mtrc::numeric::aligned, mtrc::numeric::unpadded>;

  public:
	Scalar m_lrate;
	Scalar m_decay;

	SGD() : m_lrate(Scalar(0.01)), m_decay(Scalar(0)) {}

	explicit SGD(Scalar learningRate, Scalar decay = Scalar(0)) : m_lrate(learningRate), m_decay(decay) {}

	mtrc::core::Metadata toJson() override
	{
		mtrc::core::Metadata json;
		json["type"] = "SGD";
		json["learningRate"] = m_lrate;
		json["decay"] = m_decay;
		return json;
	}

	void reset() override {}

	void update(ConstAlignedMapVec &dvec, AlignedMapVec &vec) override { vec -= m_lrate * (dvec + m_decay * vec); }

	void update(const RowVector &dvec, RowVector &vec) override { vec -= m_lrate * (dvec + m_decay * vec); }

	void update(const ColumnMatrix &dvec, ColumnMatrix &vec) override
	{
		for (size_t column = 0; column < dvec.columns(); ++column) {
			mtrc::numeric::column(vec, column) -=
				m_lrate * (mtrc::numeric::column(dvec, column) + m_decay * mtrc::numeric::column(vec, column));
		}
	}
};

} // namespace mtrc::solve::parametric::dnn

#endif /* OPTIMIZER_SGD_H_ */
