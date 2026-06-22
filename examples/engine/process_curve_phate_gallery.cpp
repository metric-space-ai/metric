#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/engine.hpp>
#include <metric/solve/parametric/dnn.hpp>

namespace {

struct ProcessCurve {
	std::string id;
	std::string family;
	std::vector<double> values;
};

struct AlignedCurveDistance {
	double gap_cost{2.0};

	auto operator()(const ProcessCurve &lhs, const ProcessCurve &rhs) const -> double
	{
		std::vector<double> previous(rhs.values.size() + 1, 0.0);
		for (std::size_t index = 0; index < previous.size(); ++index) {
			previous[index] = static_cast<double>(index) * gap_cost;
		}

		for (std::size_t lhs_index = 1; lhs_index <= lhs.values.size(); ++lhs_index) {
			std::vector<double> current(rhs.values.size() + 1, 0.0);
			current[0] = static_cast<double>(lhs_index) * gap_cost;
			for (std::size_t rhs_index = 1; rhs_index <= rhs.values.size(); ++rhs_index) {
				const auto substitution =
					previous[rhs_index - 1] +
					std::min(std::abs(lhs.values[lhs_index - 1] - rhs.values[rhs_index - 1]), 2 * gap_cost);
				const auto deletion = previous[rhs_index] + gap_cost;
				const auto insertion = current[rhs_index - 1] + gap_cost;
				current[rhs_index] = std::min({substitution, deletion, insertion});
			}
			previous = std::move(current);
		}

		return previous.back();
	}
};

struct PointwisePaddedCurveDistance {
	auto operator()(const ProcessCurve &lhs, const ProcessCurve &rhs) const -> double
	{
		const auto size = std::max(lhs.values.size(), rhs.values.size());
		double squared_sum = 0.0;
		for (std::size_t index = 0; index < size; ++index) {
			const auto left = index < lhs.values.size() ? lhs.values[index] : 0.0;
			const auto right = index < rhs.values.size() ? rhs.values[index] : 0.0;
			const auto delta = left - right;
			squared_sum += delta * delta;
		}
		return std::sqrt(squared_sum);
	}
};

struct CurveFeatureEncoder {
	std::size_t feature_count{0};

	auto operator()(const ProcessCurve &record) const -> std::vector<double>
	{
		if (feature_count == 0) {
			throw std::invalid_argument("curve feature encoder requires a positive feature count");
		}
		if (record.values.empty()) {
			throw std::invalid_argument("curve feature encoder requires non-empty curve values");
		}
		if (feature_count == 1 || record.values.size() == 1) {
			return std::vector<double>(feature_count, record.values.front());
		}

		std::vector<double> features;
		features.reserve(feature_count);
		for (std::size_t index = 0; index < feature_count; ++index) {
			const auto position = static_cast<double>(index) * static_cast<double>(record.values.size() - 1) /
								  static_cast<double>(feature_count - 1);
			const auto left = static_cast<std::size_t>(std::floor(position));
			const auto right = std::min(left + 1, record.values.size() - 1);
			const auto fraction = position - static_cast<double>(left);
			features.push_back(record.values[left] * (1.0 - fraction) + record.values[right] * fraction);
		}
		return features;
	}
};

auto add_identity_dense_layer(mtrc::solve::parametric::dnn::Network<double> &network, std::size_t input_size, std::size_t output_size)
	-> void
{
	mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>> layer(input_size, output_size);
	layer.initConstant(0.01, 0.0);
	network.addLayer(layer);
}

auto make_autoencoder_network(std::size_t feature_count) -> mtrc::solve::parametric::dnn::Network<double>
{
	mtrc::solve::parametric::dnn::Network<double> network;
	add_identity_dense_layer(network, feature_count, 2);
	add_identity_dense_layer(network, 2, feature_count);
	network.setOutput(mtrc::solve::parametric::dnn::RegressionMSE<double>());
	network.setOptimizer(mtrc::solve::parametric::dnn::RMSProp<double>(0.003, 1.0e-8, 0.0));
	return network;
}

auto find_epoch_term(const mtrc::solve::parametric::dnn::EpochReport<double> &epoch, const std::string &name) -> double
{
	for (const auto &term : epoch.terms) {
		if (term.name == name) {
			return term.value;
		}
	}
	assert(false);
	return 0.0;
}

auto gallery_records() -> std::vector<ProcessCurve>
{
	return {{"normal_reference", "normal", {0, 0, 1, 1, 1, 2, 3, 3}},
			{"flat_hold", "flat", {0, 0, 0, 0, 1, 1, 1, 1, 1, 1}},
			{"late_ramp", "late_ramp", {0, 0, 0, 1, 1, 1, 2, 3, 3}},
			{"spike", "spike", {0, 0, 1, 7, 1, 2, 3, 3}},
			{"early_ramp", "early_ramp", {0, 1, 1, 1, 2, 3, 3, 3}},
			{"normal_smooth", "normal", {0, 0, 1, 1, 2, 2, 3, 3}},
			{"flat_hold_long", "flat", {0, 0, 0, 0, 0, 1, 1, 1, 1, 1}},
			{"late_ramp_shifted", "late_ramp", {0, 0, 0, 0, 1, 1, 2, 3, 3}},
			{"spike_shifted", "spike", {0, 0, 0, 7, 1, 2, 3, 3}},
			{"early_ramp_short", "early_ramp", {0, 1, 1, 2, 2, 3, 3}},
			{"late_ramp_long_delay", "late_ramp", {0, 0, 0, 0, 0, 1, 2, 3, 3}},
			{"flat_hold_extra_long", "flat", {0, 0, 0, 0, 0, 0, 1, 1, 1, 1}},
			{"normal_soft_start", "normal", {0, 0, 1, 1, 1, 1, 2, 3, 3}},
			{"spike_wide", "spike", {0, 0, 0, 1, 8, 1, 2, 3, 3}},
			{"early_ramp_plateau", "early_ramp", {0, 1, 1, 1, 1, 2, 3, 3}}};
}

auto held_out_queries() -> std::vector<ProcessCurve>
{
	return {{"gallery_delay_A", "late_ramp", {0, 0, 0, 0, 1, 1, 1, 2, 3, 3}},
			{"gallery_delay_B", "late_ramp", {0, 0, 0, 0, 0, 1, 1, 2, 3, 3}},
			{"gallery_delay_C", "late_ramp", {0, 0, 0, 0, 1, 1, 2, 3}},
			{"gallery_delay_D", "late_ramp", {0, 0, 0, 0, 0, 1, 2, 3}},
			{"gallery_delay_E", "late_ramp", {0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3}},
			{"gallery_delay_F", "late_ramp", {0, 0, 0, 0, 1, 1, 1, 1, 2, 3, 3}}};
}

} // namespace

int main()
{
	const auto source_records = gallery_records();
	const auto queries = held_out_queries();
	auto space = mtrc::make_space(source_records, AlignedCurveDistance{});
	auto vector_baseline = mtrc::make_space(source_records, PointwisePaddedCurveDistance{});

	mtrc::modify::map::PhateGeometrySpec<double> geometry;
	geometry.dimensions = 2;
	geometry.diffusion_steps = 2;
	geometry.kernel_scale = 1.0;

	constexpr std::size_t feature_count = 8;
	auto codec = mtrc::solve::parametric::dnn::make_feature_record_codec<ProcessCurve, double>(
		feature_count, CurveFeatureEncoder{feature_count}, "process_curve_gallery_feature_codec");
	assert(codec.feature_count() == feature_count);
	assert(!codec.inverse_supported());

	mtrc::solve::parametric::dnn::TrainingSpec<double> training;
	training.epochs = 220;
	training.batch_size = source_records.size();
	training.shuffle = false;
	training.seed = 41;
	training.gradient_clip_norm = 20.0;

	const auto pipeline_plan = mtrc::modify::compose::native_phate_autoencoder_pipeline_builder(0.02, 1.0)
								   .use_feature_record_codec()
								   .use_distance_table_pairwise_distances()
								   .use_exponential_affinity_kernel()
								   .use_lazy_row_normalized_diffusion_operator()
								   .plan();
	const auto hooks = mtrc::modify::compose::resolve_native_phate_autoencoder_executable_hooks<double>(pipeline_plan);
	assert(hooks.executable());
	assert(hooks.codec == "feature_record_codec");

	const auto targets = mtrc::modify::map::phate_geometry_targets<decltype(space), double>(
		space, geometry, hooks.pairwise_distances, hooks.affinity_kernel, hooks.diffusion_operator);
	assert(targets.record_count == source_records.size());
	assert(targets.dense_distance_evaluations == source_records.size() * source_records.size());

	auto mapping = mtrc::modify::map::native_phate_autoencoder_with_codec(
		mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_autoencoder_network(feature_count)), geometry, training, codec,
		pipeline_plan);
	auto model = mtrc::modify::map::fit(mapping, space);
	assert(model.pipeline_plan().has_component("codec", "feature_record_codec"));
	assert(model.codec().to_json().at("name") == "process_curve_gallery_feature_codec");

	const auto latent = mtrc::modify::map::transform(model, space);
	assert(latent.mapping == "native_phate_autoencoder");
	assert(latent.strategy == "native_dnn_phate_ae");
	assert(!latent.inverse_supported);
	assert(latent.space.size() == source_records.size());
	assert(latent.space.record(latent.space.id(0)).size() == geometry.dimensions);

	auto query_space = mtrc::make_space(queries, AlignedCurveDistance{});
	const auto query_latent = mtrc::modify::map::transform(model, query_space);
	assert(query_latent.space.size() == queries.size());

	std::size_t metric_correct = 0;
	std::size_t vector_misses = 0;
	std::size_t latent_correct = 0;
	std::string first_metric_family;
	std::string first_vector_family;
	std::string first_latent_family;
	double metric_margin_sum = 0.0;
	std::vector<std::string> query_diagnostics;
	for (std::size_t index = 0; index < queries.size(); ++index) {
		const auto metric_neighbors = mtrc::find_neighbors(space, queries[index], mtrc::count{1});
		const auto vector_neighbors = mtrc::find_neighbors(vector_baseline, queries[index], mtrc::count{1});
		const auto latent_neighbors = mtrc::find_neighbors(
			latent.space, query_latent.space.record(query_latent.space.id(index)), mtrc::count{1});
		assert(metric_neighbors.size() == 1);
		assert(vector_neighbors.size() == 1);
		assert(latent_neighbors.size() == 1);

		const auto metric_family = source_records[metric_neighbors[0].id.index()].family;
		const auto vector_family = source_records[vector_neighbors[0].id.index()].family;
		const auto latent_family = source_records[latent_neighbors[0].id.index()].family;
		if (index == 0) {
			first_metric_family = metric_family;
			first_vector_family = vector_family;
			first_latent_family = latent_family;
		}
		if (metric_family == queries[index].family) {
			++metric_correct;
		}
		if (vector_family != queries[index].family) {
			++vector_misses;
		}
		if (latent_family == queries[index].family) {
			++latent_correct;
		}
		const auto vector_distance_under_metric =
			space.metric()(queries[index], source_records[vector_neighbors[0].id.index()]);
		const auto metric_margin = vector_distance_under_metric - metric_neighbors[0].distance;
		assert(metric_margin > 0.0);
		metric_margin_sum += metric_margin;
		query_diagnostics.push_back(queries[index].id + ":metric=" + metric_family + ",vector=" + vector_family +
									",latent=" + latent_family);
	}
	assert(metric_correct == queries.size());
	assert(vector_misses == queries.size());
	assert(latent_correct == queries.size());

	const auto preservation = mtrc::modify::map::neighbor_preservation(space, latent, 1);
	assert(preservation.source_record_count == source_records.size());
	assert(preservation.mapped_record_count == latent.space.size());
	assert(preservation.evaluated_neighbor_count == 1);

	const auto oos_stability = mtrc::modify::map::out_of_sample_neighbor_stability(model, space, query_space, 1);
	assert(oos_stability.transform_supported);
	assert(oos_stability.source_record_count == source_records.size());
	assert(oos_stability.query_record_count == queries.size());
	assert(oos_stability.evaluated_neighbor_count == 1);
	assert(oos_stability.average_mapped_best_source_rank >= 1.0);
	assert(oos_stability.maximum_mapped_best_source_rank >= 1);
	assert(oos_stability.average_best_distance_penalty >= 0.0);
	assert(oos_stability.maximum_best_distance_penalty >= 0.0);
	assert(std::abs(oos_stability.average_mapped_best_source_rank - (4.0 / 3.0)) < 1.0e-12);
	assert(oos_stability.maximum_mapped_best_source_rank == 3);
	assert(std::abs(oos_stability.average_best_distance_penalty - (1.0 / 6.0)) < 1.0e-12);
	assert(std::abs(oos_stability.maximum_best_distance_penalty - 1.0) < 1.0e-12);

	const auto &report = model.training_report();
	assert(report.epochs.size() == training.epochs);
	const auto initial_bottleneck = find_epoch_term(report.epochs.front(), "bottleneck_coordinate_mse");
	const auto final_bottleneck = find_epoch_term(report.epochs.back(), "bottleneck_coordinate_mse");
	assert(std::isfinite(initial_bottleneck));
	assert(std::isfinite(final_bottleneck));

	std::cout << "process PHATE gallery records = " << source_records.size() << "\n";
	std::cout << "process PHATE gallery queries = " << queries.size() << "\n";
	std::cout << "process PHATE gallery source metric = aligned_curve_distance\n";
	std::cout << "process PHATE gallery feature codec = "
			  << model.codec().to_json().at("name").template get<std::string>() << "\n";
	std::cout << "process PHATE gallery pipeline components = " << model.pipeline_plan().component_count() << "\n";
	std::cout << "process PHATE gallery distance provider = " << hooks.pairwise_distances << "\n";
	std::cout << "process PHATE gallery affinity kernel = " << hooks.affinity_kernel << "\n";
	std::cout << "process PHATE gallery diffusion operator = " << hooks.diffusion_operator << "\n";
	std::cout << "process PHATE gallery target evaluations = " << targets.dense_distance_evaluations << "\n";
	std::cout << "process PHATE gallery metric nearest family = " << first_metric_family << "\n";
	std::cout << "process PHATE gallery vector baseline family = " << first_vector_family << "\n";
	std::cout << "process PHATE gallery latent nearest family = " << first_latent_family << "\n";
	std::cout << "process PHATE gallery metric correct = " << metric_correct << "/" << queries.size() << "\n";
	std::cout << "process PHATE gallery vector misses = " << vector_misses << "/" << queries.size() << "\n";
	std::cout << "process PHATE gallery latent correct = " << latent_correct << "/" << queries.size() << "\n";
	std::cout << "process PHATE gallery average metric margin = "
			  << metric_margin_sum / static_cast<double>(queries.size()) << "\n";
	for (const auto &diagnostic : query_diagnostics) {
		std::cout << "process PHATE gallery query = " << diagnostic << "\n";
	}
	std::cout << "process PHATE gallery neighbor preservation = " << preservation.recall << "\n";
	std::cout << "process PHATE gallery OOS anchor recall = " << oos_stability.anchor_recall << "\n";
	std::cout << "process PHATE gallery first-anchor match rate = " << oos_stability.first_anchor_match_rate << "\n";
	std::cout << "process PHATE gallery mapped-anchor source rank = " << oos_stability.average_mapped_best_source_rank
			  << "\n";
	std::cout << "process PHATE gallery max mapped-anchor source rank = "
			  << oos_stability.maximum_mapped_best_source_rank << "\n";
	std::cout << "process PHATE gallery original-metric distance penalty = "
			  << oos_stability.average_best_distance_penalty << "\n";
	std::cout << "process PHATE gallery max original-metric distance penalty = "
			  << oos_stability.maximum_best_distance_penalty << "\n";
	std::cout << "process PHATE gallery bottleneck loss = " << initial_bottleneck << " -> " << final_bottleneck << "\n";
	std::cout << "process PHATE gallery inverse support = none_feature_codec\n";
	std::cout << "process PHATE gallery deterministic seed = " << training.seed << "\n";

	return 0;
}
