#ifndef LOSS_H_
#define LOSS_H_

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <metric/numeric/Math.h>
#include <metric/core/metadata.hpp>

#include "Dataset.h"

namespace mtrc::solve::parametric::dnn {

enum class LossAnchorKind { final_output, layer_output };

struct LossAnchor {
	LossAnchorKind kind{LossAnchorKind::final_output};
	std::size_t layer_index{0};
};

inline auto loss_anchor_kind_name(LossAnchorKind kind) -> std::string
{
	return kind == LossAnchorKind::final_output ? "final_output" : "layer_output";
}

inline auto loss_anchor_to_json(LossAnchor anchor) -> mtrc::core::Metadata
{
	return {{"kind", loss_anchor_kind_name(anchor.kind)}, {"layer_index", anchor.layer_index}};
}

// Loss-term normalization contract (audited, do not "fix" the missing 1/N):
//
//   value    is normalized per observation row N: value = sum_of_squares / N.
//   gradient is the *unnormalized* derivative d(N * value) / d(activation),
//            i.e. the gradient of the summed (not averaged) objective.
//
// The averaging by N is applied downstream inside the layer backprop
// (FullyConnected divides d(L)/d(W) by `nobs`), so handing the layers the
// unnormalized gradient reproduces d(value)/d(W) for the *mean* loss. This
// mirrors the legacy RegressionMSE output layer convention. A finite-difference
// check against `value` must therefore scale the analytic `gradient` by 1/N (see
// native_dnn_loss_gradient_check_smoke).
template <class Scalar> struct LossEvaluation {
	Scalar value{0};
	mtrc::numeric::DynamicMatrix<Scalar> gradient;
};

template <class Scalar> class LossTerm {
  public:
	using matrix_type = mtrc::numeric::DynamicMatrix<Scalar>;

	virtual ~LossTerm() = default;
	virtual auto name() const -> std::string = 0;
	virtual auto anchor() const -> LossAnchor = 0;
	virtual auto evaluate(const DnnBatch<Scalar> &batch, const std::vector<matrix_type> &activations) const
		-> LossEvaluation<Scalar> = 0;
	virtual auto to_json() const -> mtrc::core::Metadata = 0;
};

template <class Scalar> struct LossTermReport {
	std::string name;
	Scalar weight{1};
	Scalar value{0};
	Scalar weighted_value{0};
	LossAnchor anchor;
};

template <class Scalar> struct CompositeLossEvaluation {
	Scalar total_value{0};
	std::vector<mtrc::numeric::DynamicMatrix<Scalar>> gradients_by_layer;
	std::vector<LossTermReport<Scalar>> terms;
};

template <class Scalar> class CompositeLoss {
  public:
	using matrix_type = mtrc::numeric::DynamicMatrix<Scalar>;

	auto add(std::shared_ptr<const LossTerm<Scalar>> term, Scalar weight = Scalar(1)) -> CompositeLoss &
	{
		if (!term) {
			throw std::invalid_argument("CompositeLoss term must not be null");
		}
		terms_.push_back({std::move(term), weight});
		return *this;
	}

	auto size() const -> std::size_t { return terms_.size(); }
	auto to_json() const -> mtrc::core::Metadata
	{
		mtrc::core::Metadata terms = mtrc::core::Metadata::array();
		for (const auto &weighted_term : terms_) {
			terms.push_back({{"weight", weighted_term.weight}, {"term", weighted_term.term->to_json()}});
		}
		return {{"type", "CompositeLoss"}, {"terms", std::move(terms)}};
	}

	auto evaluate(const DnnBatch<Scalar> &batch, const std::vector<matrix_type> &activations) const
		-> CompositeLossEvaluation<Scalar>
	{
		if (activations.empty()) {
			throw std::invalid_argument("CompositeLoss requires layer activations");
		}

		CompositeLossEvaluation<Scalar> result;
		result.gradients_by_layer.resize(activations.size());
		result.terms.reserve(terms_.size());

		for (const auto &weighted_term : terms_) {
			const auto term_evaluation = weighted_term.term->evaluate(batch, activations);
			const auto anchor = weighted_term.term->anchor();
			const auto layer_index = resolve_anchor(anchor, activations.size());
			validate_gradient_shape(term_evaluation.gradient, activations[layer_index]);

			if (mtrc::numeric::size(result.gradients_by_layer[layer_index]) == 0) {
				result.gradients_by_layer[layer_index].resize(activations[layer_index].rows(),
															  activations[layer_index].columns());
				result.gradients_by_layer[layer_index] = Scalar(0);
			}
			result.gradients_by_layer[layer_index] += weighted_term.weight * term_evaluation.gradient;

			const auto weighted_value = weighted_term.weight * term_evaluation.value;
			result.total_value += weighted_value;
			result.terms.push_back(
				{weighted_term.term->name(), weighted_term.weight, term_evaluation.value, weighted_value, anchor});
		}

		return result;
	}

  private:
	struct WeightedTerm {
		std::shared_ptr<const LossTerm<Scalar>> term;
		Scalar weight{1};
	};

	static auto resolve_anchor(LossAnchor anchor, std::size_t layer_count) -> std::size_t
	{
		if (anchor.kind == LossAnchorKind::final_output) {
			return layer_count - 1;
		}
		if (anchor.layer_index >= layer_count) {
			throw std::invalid_argument("Loss anchor layer index is out of range");
		}
		return anchor.layer_index;
	}

	static auto validate_gradient_shape(const matrix_type &gradient, const matrix_type &activation) -> void
	{
		if (gradient.rows() != activation.rows() || gradient.columns() != activation.columns()) {
			throw std::invalid_argument("Loss gradient shape does not match anchored activation");
		}
	}

	std::vector<WeightedTerm> terms_;
};

template <class Scalar> class ReconstructionMSELoss final : public LossTerm<Scalar> {
  public:
	using matrix_type = typename LossTerm<Scalar>::matrix_type;

	auto name() const -> std::string override { return "reconstruction_mse"; }
	auto anchor() const -> LossAnchor override { return {LossAnchorKind::final_output, 0}; }

	auto evaluate(const DnnBatch<Scalar> &batch, const std::vector<matrix_type> &activations) const
		-> LossEvaluation<Scalar> override
	{
		if (activations.empty()) {
			throw std::invalid_argument("ReconstructionMSELoss requires final activation");
		}
		const auto &prediction = activations.back();
		if (prediction.rows() == 0 || prediction.rows() != batch.x.rows() ||
			prediction.columns() != batch.x.columns()) {
			throw std::invalid_argument("ReconstructionMSELoss target shape does not match final activation");
		}

		LossEvaluation<Scalar> result;
		const auto delta = prediction - batch.x;
		result.gradient = Scalar(2) * delta;
		result.value = mtrc::numeric::mean_row_sqr_norm(delta);
		return result;
	}

	auto to_json() const -> mtrc::core::Metadata override
	{
		return {{"type", "ReconstructionMSELoss"}, {"anchor", loss_anchor_kind_name(anchor().kind)}};
	}
};

template <class Scalar> class BottleneckCoordinateMSELoss final : public LossTerm<Scalar> {
  public:
	using matrix_type = typename LossTerm<Scalar>::matrix_type;
	using target_table_type = std::map<SampleId, std::vector<Scalar>>;

	BottleneckCoordinateMSELoss(std::size_t layer_index, target_table_type coordinates)
		: anchor_{LossAnchorKind::layer_output, layer_index}, coordinates_(std::move(coordinates))
	{
	}

	auto name() const -> std::string override { return "bottleneck_coordinate_mse"; }
	auto anchor() const -> LossAnchor override { return anchor_; }

	auto evaluate(const DnnBatch<Scalar> &batch, const std::vector<matrix_type> &activations) const
		-> LossEvaluation<Scalar> override
	{
		if (anchor_.layer_index >= activations.size()) {
			throw std::invalid_argument("BottleneckCoordinateMSELoss anchor is out of range");
		}
		const auto &bottleneck = activations[anchor_.layer_index];
		if (bottleneck.rows() == 0 || bottleneck.rows() != batch.ids.size()) {
			throw std::invalid_argument("BottleneckCoordinateMSELoss batch IDs do not match activation rows");
		}

		LossEvaluation<Scalar> result;
		std::vector<std::vector<Scalar>> target_rows;
		target_rows.reserve(batch.ids.size());
		for (const auto &sample_id : batch.ids) {
			const auto found = coordinates_.find(sample_id);
			if (found == coordinates_.end()) {
				throw std::invalid_argument("BottleneckCoordinateMSELoss target is missing a sample ID");
			}
			if (found->second.size() != bottleneck.columns()) {
				throw std::invalid_argument("BottleneckCoordinateMSELoss target dimension does not match bottleneck");
			}
			target_rows.push_back(found->second);
		}

		const auto target_matrix = mtrc::numeric::row_vectors_to_matrix<Scalar>(target_rows, bottleneck.columns());
		const auto delta = bottleneck - target_matrix;
		result.gradient = Scalar(2) * delta;
		result.value = mtrc::numeric::mean_row_sqr_norm(delta);
		return result;
	}

	auto to_json() const -> mtrc::core::Metadata override
	{
		return {{"type", "BottleneckCoordinateMSELoss"},
				{"anchor", loss_anchor_kind_name(anchor_.kind)},
				{"layer_index", anchor_.layer_index},
				{"target_count", coordinates_.size()}};
	}

  private:
	LossAnchor anchor_;
	target_table_type coordinates_;
};

} // namespace mtrc::solve::parametric::dnn

#endif /* LOSS_H_ */
