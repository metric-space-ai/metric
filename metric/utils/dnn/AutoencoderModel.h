#ifndef AUTOENCODER_MODEL_H_
#define AUTOENCODER_MODEL_H_

#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

#include "Loss.h"
#include "Network.h"

namespace metric::dnn {

struct AutoencoderTopology {
	std::size_t input_layer{0};
	std::size_t bottleneck_layer{0};
	std::size_t output_layer{0};
	std::vector<std::size_t> encoder_layers;
	std::vector<std::size_t> decoder_layers;
};

inline auto infer_autoencoder_topology(std::size_t layer_count) -> AutoencoderTopology
{
	if (layer_count < 2) {
		throw std::invalid_argument("autoencoder topology requires at least two layers");
	}

	AutoencoderTopology topology;
	topology.output_layer = layer_count - 1;
	topology.bottleneck_layer = layer_count % 2 == 0 ? (layer_count / 2) - 1 : layer_count / 2;

	topology.encoder_layers.reserve(topology.bottleneck_layer + 1);
	for (std::size_t layer = 0; layer <= topology.bottleneck_layer; ++layer) {
		topology.encoder_layers.push_back(layer);
	}

	for (std::size_t layer = topology.bottleneck_layer + 1; layer < layer_count; ++layer) {
		topology.decoder_layers.push_back(layer);
	}

	return topology;
}

template <class Scalar> struct EpochReport {
	std::size_t epoch{0};
	Scalar total_loss{0};
	std::vector<LossTermReport<Scalar>> terms;
	std::size_t batch_count{0};
};

template <class Scalar> struct TrainingReport {
	std::vector<EpochReport<Scalar>> epochs;
	bool stopped_early{false};
	std::string stop_reason;
};

template <class Scalar> struct TrainingSpec {
	std::size_t epochs{100};
	std::size_t batch_size{32};
	std::uint64_t seed{123};
	bool shuffle{true};
	Scalar gradient_clip_norm{0};
	Scalar early_stop_min_delta{0};
	std::size_t early_stop_patience{0};
};

template <class Scalar> class AutoencoderModel {
  public:
	using matrix_type = blaze::DynamicMatrix<Scalar>;
	using network_type = Network<Scalar>;

	explicit AutoencoderModel(network_type network)
		: network_(std::move(network))
		, topology_(infer_autoencoder_topology(network_.layers.size()))
	{
		validate_topology();
	}

	AutoencoderModel(network_type network, AutoencoderTopology topology)
		: network_(std::move(network))
		, topology_(std::move(topology))
	{
		validate_topology();
	}

	AutoencoderModel(const AutoencoderModel &other)
		: network_(clone_network(other.network_))
		, topology_(other.topology_)
		, backend_name_(other.backend_name_)
		, report_(other.report_)
	{
	}

	auto operator=(const AutoencoderModel &other) -> AutoencoderModel &
	{
		if (this != &other) {
			network_ = clone_network(other.network_);
			topology_ = other.topology_;
			backend_name_ = other.backend_name_;
			report_ = other.report_;
		}
		return *this;
	}

	AutoencoderModel(AutoencoderModel &&) noexcept = default;
	auto operator=(AutoencoderModel &&) noexcept -> AutoencoderModel & = default;

	auto network() -> network_type & { return network_; }
	auto network() const -> const network_type & { return network_; }
	auto topology() const -> const AutoencoderTopology & { return topology_; }
	auto backend_name() const -> const std::string & { return backend_name_; }
	auto training_report() const -> const TrainingReport<Scalar> & { return report_; }
	auto set_training_report(TrainingReport<Scalar> report) -> void { report_ = std::move(report); }

	auto clone() const -> AutoencoderModel { return AutoencoderModel(*this); }

	auto encode(const matrix_type &features) -> matrix_type
	{
		std::vector<matrix_type> activations;
		network_.forward_all(features, &activations);
		return activations.at(topology_.bottleneck_layer);
	}

	auto decode(const matrix_type &latent) -> matrix_type
	{
		matrix_type current(latent);
		for (const auto layer_index : topology_.decoder_layers) {
			network_.layers.at(layer_index)->forward(current);
			current = network_.layers.at(layer_index)->output();
		}
		return current;
	}

  private:
	static auto clone_network(const network_type &source) -> network_type
	{
		std::stringstream stream;
		source.save(stream);

		network_type copy;
		copy.load(stream);
		return copy;
	}

	auto validate_topology() const -> void
	{
		const auto layer_count = network_.layers.size();
		if (layer_count < 2) {
			throw std::invalid_argument("AutoencoderModel requires at least two network layers");
		}
		if (topology_.bottleneck_layer >= layer_count || topology_.output_layer >= layer_count) {
			throw std::invalid_argument("AutoencoderModel topology references an unknown layer");
		}
		if (topology_.output_layer != layer_count - 1) {
			throw std::invalid_argument("AutoencoderModel output layer must be the final network layer");
		}
		for (const auto layer : topology_.encoder_layers) {
			if (layer > topology_.bottleneck_layer || layer >= layer_count) {
				throw std::invalid_argument("AutoencoderModel encoder layer is out of range");
			}
		}
		for (const auto layer : topology_.decoder_layers) {
			if (layer <= topology_.bottleneck_layer || layer >= layer_count) {
				throw std::invalid_argument("AutoencoderModel decoder layer is out of range");
			}
		}
	}

	network_type network_;
	AutoencoderTopology topology_;
	std::string backend_name_{"native_dnn"};
	TrainingReport<Scalar> report_;
};

template <class Scalar> class NativeDnnTrainer {
  public:
	using matrix_type = blaze::DynamicMatrix<Scalar>;

	auto fit(AutoencoderModel<Scalar> &model, const EncodedDataset<Scalar> &dataset,
			 const CompositeLoss<Scalar> &objective, const TrainingSpec<Scalar> &spec) const
		-> TrainingReport<Scalar>
	{
		if (dataset.size() == 0) {
			throw std::invalid_argument("NativeDnnTrainer requires a non-empty dataset");
		}
		if (objective.size() == 0) {
			throw std::invalid_argument("NativeDnnTrainer requires at least one loss term");
		}

		model.network().reset_optimizer();
		std::mt19937 rng(static_cast<std::mt19937::result_type>(spec.seed));

		TrainingReport<Scalar> report;
		Scalar best_loss = std::numeric_limits<Scalar>::infinity();
		std::size_t stale_epochs = 0;

		for (std::size_t epoch = 0; epoch < spec.epochs; ++epoch) {
			auto batches = dataset.batches(spec.batch_size, rng, spec.shuffle);
			EpochAccumulator accumulator(epoch);

			for (auto &batch : batches) {
				std::vector<matrix_type> activations;
				model.network().forward_all(batch.x, &activations);

				auto evaluation = objective.evaluate(batch, activations);
				clip_gradients(evaluation.gradients_by_layer, spec.gradient_clip_norm);
				model.network().backprop_from_layer_gradients(batch.x, evaluation.gradients_by_layer);
				model.network().apply_optimizer();

				accumulator.add(evaluation);
			}

			report.epochs.push_back(accumulator.finish());
			const auto current_loss = report.epochs.back().total_loss;
			if (spec.early_stop_patience > 0) {
				if (current_loss + spec.early_stop_min_delta < best_loss) {
					best_loss = current_loss;
					stale_epochs = 0;
				} else {
					++stale_epochs;
					if (stale_epochs >= spec.early_stop_patience) {
						report.stopped_early = true;
						report.stop_reason = "early_stop_patience";
						break;
					}
				}
			}
		}

		model.set_training_report(report);
		return report;
	}

  private:
	class EpochAccumulator {
	  public:
		explicit EpochAccumulator(std::size_t epoch)
			: epoch_(epoch)
		{
		}

		auto add(const CompositeLossEvaluation<Scalar> &evaluation) -> void
		{
			if (batch_count_ == 0) {
				terms_ = evaluation.terms;
				for (auto &term : terms_) {
					term.value = Scalar(0);
					term.weighted_value = Scalar(0);
				}
			}
			if (evaluation.terms.size() != terms_.size()) {
				throw std::invalid_argument("Composite loss term layout changed during an epoch");
			}

			total_loss_ += evaluation.total_value;
			for (std::size_t index = 0; index < terms_.size(); ++index) {
				if (terms_[index].name != evaluation.terms[index].name) {
					throw std::invalid_argument("Composite loss term order changed during an epoch");
				}
				terms_[index].value += evaluation.terms[index].value;
				terms_[index].weighted_value += evaluation.terms[index].weighted_value;
			}
			++batch_count_;
		}

		auto finish() const -> EpochReport<Scalar>
		{
			if (batch_count_ == 0) {
				throw std::invalid_argument("NativeDnnTrainer epoch has no batches");
			}

			EpochReport<Scalar> report;
			report.epoch = epoch_;
			report.total_loss = total_loss_ / static_cast<Scalar>(batch_count_);
			report.terms = terms_;
			report.batch_count = batch_count_;
			for (auto &term : report.terms) {
				term.value /= static_cast<Scalar>(batch_count_);
				term.weighted_value /= static_cast<Scalar>(batch_count_);
			}
			return report;
		}

	  private:
		std::size_t epoch_{0};
		Scalar total_loss_{0};
		std::vector<LossTermReport<Scalar>> terms_;
		std::size_t batch_count_{0};
	};

	static auto clip_gradients(std::vector<matrix_type> &gradients_by_layer, Scalar clip_norm) -> void
	{
		if (clip_norm <= Scalar(0)) {
			return;
		}

		Scalar squared_norm{0};
		for (const auto &gradient : gradients_by_layer) {
			if (blaze::size(gradient) != 0) {
				squared_norm += blaze::sqrNorm(gradient);
			}
		}

		const auto norm = std::sqrt(squared_norm);
		if (norm == Scalar(0) || norm <= clip_norm) {
			return;
		}

		const auto scale = clip_norm / norm;
		for (auto &gradient : gradients_by_layer) {
			if (blaze::size(gradient) != 0) {
				gradient *= scale;
			}
		}
	}
};

} // namespace metric::dnn

#endif /* AUTOENCODER_MODEL_H_ */
