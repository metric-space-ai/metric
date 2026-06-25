#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/engine.hpp"
#include "metric/solve/parametric/dnn.hpp"

namespace {

struct ProcessCurve {
	std::string id;
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

struct CurveCoordinateEncoder {
	std::size_t coordinate_count{0};

	auto operator()(const ProcessCurve &record) const -> std::vector<double>
	{
		if (coordinate_count == 0) {
			throw std::invalid_argument("curve coordinate encoder requires a positive coordinate count");
		}
		if (record.values.empty()) {
			throw std::invalid_argument("curve coordinate encoder requires non-empty curve values");
		}
		if (coordinate_count == 1 || record.values.size() == 1) {
			return std::vector<double>(coordinate_count, record.values.front());
		}

		std::vector<double> coordinates;
		coordinates.reserve(coordinate_count);
		for (std::size_t index = 0; index < coordinate_count; ++index) {
			const auto position = static_cast<double>(index) * static_cast<double>(record.values.size() - 1) /
								  static_cast<double>(coordinate_count - 1);
			const auto left = static_cast<std::size_t>(std::floor(position));
			const auto right = std::min(left + 1, record.values.size() - 1);
			const auto fraction = position - static_cast<double>(left);
			coordinates.push_back(record.values[left] * (1.0 - fraction) + record.values[right] * fraction);
		}
		return coordinates;
	}
};

auto add_identity_dense_layer(mtrc::solve::parametric::dnn::Network<double> &network, std::size_t input_size, std::size_t output_size)
	-> void
{
	mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>> layer(input_size, output_size);
	layer.initConstant(0.01, 0.0);
	network.addLayer(layer);
}

auto make_autoencoder_network(std::size_t coordinate_count) -> mtrc::solve::parametric::dnn::Network<double>
{
	mtrc::solve::parametric::dnn::Network<double> network;
	add_identity_dense_layer(network, coordinate_count, 2);
	add_identity_dense_layer(network, 2, coordinate_count);
	network.setOutput(mtrc::solve::parametric::dnn::RegressionMSE<double>());
	network.setOptimizer(mtrc::solve::parametric::dnn::RMSProp<double>(0.001, 1.0e-8, 0.0));
	return network;
}

} // namespace

int main()
{
	const std::vector<ProcessCurve> records{{"normal_reference", {0, 0, 1, 1, 1, 2, 3, 3}},
											{"late_ramp", {0, 0, 0, 1, 1, 1, 2, 3, 3}},
											{"late_ramp_shifted", {0, 0, 0, 0, 1, 1, 2, 3, 3}},
											{"spike", {0, 0, 1, 7, 1, 2, 3, 3}},
											{"early_ramp", {0, 1, 1, 1, 2, 3, 3, 3}}};
	auto space = mtrc::make_space(records, AlignedCurveDistance{});

	mtrc::modify::map::DiffusionCoordinateSpec<double> geometry;
	geometry.dimensions = 2;
	geometry.diffusion_steps = 2;
	geometry.kernel_scale = 1.0;

	constexpr std::size_t coordinate_count = 6;
	auto codec = mtrc::solve::parametric::dnn::make_record_coordinate_codec<ProcessCurve, double>(
		coordinate_count, CurveCoordinateEncoder{coordinate_count}, "process_curve_coordinate_codec");
	assert(codec.coordinate_count() == coordinate_count);
	assert(!codec.inverse_supported());
	assert(codec.to_json().at("type") == "RecordCoordinateCodec");
	assert(codec.to_json().at("name") == "process_curve_coordinate_codec");

	mtrc::modify::map::CoordinateCalibrationSpec<double> calibration;
	calibration.steps = 32;
	calibration.batch_size = records.size();
	calibration.shuffle = false;
	calibration.seed = 17;
	calibration.gradient_clip_norm = 20.0;

	const auto pipeline_plan = mtrc::modify::compose::parametric_diffusion_coordinate_pipeline_builder(0.05, 1.0)
								   .use_record_coordinate_codec()
								   .use_distance_table_pairwise_distances()
								   .use_exponential_affinity_kernel()
								   .use_lazy_row_normalized_diffusion_operator()
								   .plan();
	const auto hooks = mtrc::modify::compose::resolve_parametric_diffusion_coordinate_executable_hooks<double>(pipeline_plan);
	assert(hooks.executable());
	assert(hooks.codec == "record_coordinate_codec");
	assert(hooks.pairwise_distances == "distance_table_pairwise_distances");
	assert(hooks.affinity_kernel == "exponential_affinity_kernel");
	assert(hooks.diffusion_operator == "lazy_row_normalized_diffusion_operator");

	auto mapping = mtrc::modify::map::parametric_diffusion_coordinates_with_codec(
		mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_autoencoder_network(coordinate_count)), geometry, calibration, codec,
		pipeline_plan);
	static_assert(mtrc::Mapping_v<decltype(mapping), decltype(space)>);

	auto mapping_artifact = mtrc::modify::map::derive_from(mapping, space);
	static_assert(mtrc::DerivedSpaceTransform_v<decltype(mapping_artifact), decltype(space)>);
	assert(mapping_artifact.mapping_name() == "parametric_diffusion_coordinates");
	assert(mapping_artifact.strategy_name() == "native_metric_diffusion_coordinate_solver");
	assert(mapping_artifact.pipeline_plan().has_component("codec", "record_coordinate_codec"));
	assert(mapping_artifact.pipeline_plan().has_component("pairwise_distances", "distance_table_pairwise_distances"));
	assert(mapping_artifact.codec().to_json().at("name") == "process_curve_coordinate_codec");
	assert(!mapping_artifact.codec().inverse_supported());
	assert(mapping_artifact.calibration_report().epochs.size() == calibration.steps);

	const auto latent = mtrc::modify::map::transform(mapping_artifact, space);
	assert(latent.mapping == "parametric_diffusion_coordinates");
	assert(latent.strategy == "native_metric_diffusion_coordinate_solver");
	assert(!latent.inverse_supported);
	assert(latent.space.size() == space.size());
	assert(latent.space.record(latent.space.id(0)).size() == geometry.dimensions);

	bool rejected_inverse = false;
	try {
		(void)mapping_artifact.inverse_transform(latent);
	} catch (const std::invalid_argument &) {
		rejected_inverse = true;
	}
	assert(rejected_inverse);

	const std::vector<ProcessCurve> queries{{"query_delayed_ramp", {0, 0, 0, 0, 1, 1, 2, 3, 3}},
											{"query_spike", {0, 0, 1, 6, 1, 2, 3, 3}}};
	auto query_space = mtrc::make_space(queries, AlignedCurveDistance{});
	const auto query_latent = mtrc::modify::map::transform(mapping_artifact, query_space);
	assert(query_latent.space.size() == query_space.size());
	assert(!query_latent.inverse_supported);

	const auto stability = mtrc::modify::map::out_of_sample_neighbor_stability(mapping_artifact, space, query_space, 1);
	assert(stability.transform_supported);
	assert(stability.source_record_count == space.size());
	assert(stability.query_record_count == query_space.size());
	assert(stability.evaluated_neighbor_count == 1);

	return 0;
}
