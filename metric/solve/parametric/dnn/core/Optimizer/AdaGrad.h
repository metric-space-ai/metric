#ifndef OPTIMIZER_ADAGRAD_H_
#define OPTIMIZER_ADAGRAD_H_

#include "../Optimizer.h"

#include <unordered_map>

namespace mtrc::solve::parametric::dnn {

///
/// \ingroup Optimizers
///
/// The AdaGrad algorithm
///
template <typename Scalar> class AdaGrad : public Optimizer<Scalar> {
  private:
	using typename Optimizer<Scalar>::RowVector;
	using typename Optimizer<Scalar>::ColumnMatrix;

	using Array = mtrc::numeric::DynamicVector<Scalar>;
	using AlignedMapVec = mtrc::numeric::CustomVector<Scalar, mtrc::numeric::aligned, mtrc::numeric::unpadded>;
	using ConstAlignedMapVec =
		const mtrc::numeric::CustomVector<Scalar, mtrc::numeric::aligned, mtrc::numeric::unpadded>;

	std::unordered_map<const Scalar *, Array> m_history;
	std::unordered_map<const RowVector *, RowVector> history_vectors_;
	std::unordered_map<const ColumnMatrix *, ColumnMatrix> history_matrices_;

  public:
	Scalar m_lrate;
	Scalar m_eps;

	AdaGrad() : m_lrate(Scalar(0.01)), m_eps(Scalar(1e-7)) {}

	mtrc::core::Metadata toJson() override
	{
		mtrc::core::Metadata json;
		json["type"] = "AdaGrad";
		json["learningRate"] = m_lrate;
		json["eps"] = m_eps;
		return json;
	}

	void reset() override
	{
		m_history.clear();
		history_vectors_.clear();
		history_matrices_.clear();
	}

	void update(ConstAlignedMapVec &dvec, AlignedMapVec &vec) override
	{
		auto &grad_square = m_history[dvec.data()];

		if (grad_square.size() == 0) {
			grad_square.resize(dvec.size());
			grad_square = 0;
		}

		grad_square += mtrc::numeric::pow(dvec, 2);
		vec -= m_lrate * dvec / mtrc::numeric::sqrt(grad_square + m_eps);
	}

	void update(const RowVector &dvec, RowVector &vec) override
	{
		auto &grad_square = history_vectors_[&dvec];

		if (grad_square.size() == 0) {
			grad_square.resize(dvec.size());
			grad_square = 0;
		}

		grad_square += mtrc::numeric::pow(dvec, 2);
		vec -= m_lrate * dvec / mtrc::numeric::sqrt(grad_square + m_eps);
	}

	void update(const ColumnMatrix &dvec, ColumnMatrix &vec) override
	{
		auto &grad_square = history_matrices_[&dvec];

		if (mtrc::numeric::isDefault(grad_square)) {
			grad_square.resize(dvec.rows(), dvec.columns());
			grad_square = 0;
		}

		grad_square += mtrc::numeric::pow(dvec, 2);
		for (size_t column = 0; column < dvec.columns(); ++column) {
			mtrc::numeric::column(vec, column) -=
				m_lrate * mtrc::numeric::column(dvec, column) /
				mtrc::numeric::column(mtrc::numeric::sqrt(grad_square + m_eps), column);
		}
	}
};

} // namespace mtrc::solve::parametric::dnn

#endif /* OPTIMIZER_ADAGRAD_H_ */
