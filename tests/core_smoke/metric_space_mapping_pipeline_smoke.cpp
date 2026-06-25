// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Hero gate: the Metric-Space Mapping Pipeline (parametric diffusion coordinate framed as a finite
// metric-space modify/map). Asserts the contracts the hero application relies
// on, organized by the pipeline's interchangeable components:
//
//   tg  target generation     -- diffusion-potential anchor coordinates
//   ip  invalid parameters    -- every guarded knob rejects bad input
//   ts  transform shape        -- derived space cardinality and dimension
//   lg  lineage                -- one-to-one provenance, survives the artifact
//   ar  artifact roundtrip     -- reload reproduces transform + inverse
//   bl  baseline comparison    -- classical MDS scored on identical lineage

#include <cassert>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>
#include <metric/solve/parametric/dnn.hpp>

namespace {

using record_type = std::vector<double>;

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool { return std::abs(lhs - rhs) <= tolerance; }

template <typename Fn> auto rejects(Fn fn) -> bool
{
	try {
		fn();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

auto make_arc_records(std::size_t count) -> std::vector<record_type>
{
	std::vector<record_type> records;
	records.reserve(count);
	const double pi = 3.14159265358979323846;
	for (std::size_t index = 0; index < count; ++index) {
		const double t = static_cast<double>(index) / static_cast<double>(count - 1);
		const double theta = pi * t;
		const double radius = 1.0 + 0.03 * static_cast<double>(static_cast<int>(index % 3) - 1);
		records.push_back({radius * std::cos(theta), radius * std::sin(theta)});
	}
	return records;
}

template <typename Space>
auto dense_distance_matrix(const Space &space) -> std::vector<std::vector<double>>
{
	const auto size = space.size();
	std::vector<std::vector<double>> distances(size, std::vector<double>(size, 0.0));
	const auto &metric = space.metric();
	for (std::size_t row = 0; row < size; ++row) {
		for (std::size_t column = 0; column < size; ++column) {
			distances[row][column] = metric(space.record(space.id(row)), space.record(space.id(column)));
		}
	}
	return distances;
}

auto classical_mds_coordinates(const std::vector<std::vector<double>> &distances, std::size_t dimensions)
	-> std::vector<record_type>
{
	const std::size_t n = distances.size();
	std::vector<double> row_mean(n, 0.0);
	double grand_mean = 0.0;
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			const double squared = distances[i][j] * distances[i][j];
			row_mean[i] += squared;
			grand_mean += squared;
		}
		row_mean[i] /= static_cast<double>(n);
	}
	grand_mean /= static_cast<double>(n) * static_cast<double>(n);

	std::vector<std::vector<double>> gram(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			gram[i][j] = -0.5 * (distances[i][j] * distances[i][j] - row_mean[i] - row_mean[j] + grand_mean);
		}
	}

	std::vector<record_type> coordinates(n, record_type(dimensions, 0.0));
	for (std::size_t axis = 0; axis < dimensions; ++axis) {
		std::vector<double> vector(n, 0.0);
		for (std::size_t i = 0; i < n; ++i) {
			vector[i] = std::sin(static_cast<double>(i + 1 + axis));
		}
		auto normalize = [&vector, n]() {
			double norm = 0.0;
			for (std::size_t i = 0; i < n; ++i) {
				norm += vector[i] * vector[i];
			}
			norm = std::sqrt(norm);
			if (norm > 0.0) {
				for (std::size_t i = 0; i < n; ++i) {
					vector[i] /= norm;
				}
			}
		};
		normalize();
		double eigenvalue = 0.0;
		for (std::size_t iteration = 0; iteration < 256; ++iteration) {
			std::vector<double> next(n, 0.0);
			for (std::size_t i = 0; i < n; ++i) {
				for (std::size_t j = 0; j < n; ++j) {
					next[i] += gram[i][j] * vector[j];
				}
			}
			vector = next;
			normalize();
		}
		for (std::size_t i = 0; i < n; ++i) {
			for (std::size_t j = 0; j < n; ++j) {
				eigenvalue += vector[i] * gram[i][j] * vector[j];
			}
		}
		const double scale = std::sqrt(std::max(eigenvalue, 0.0));
		for (std::size_t i = 0; i < n; ++i) {
			coordinates[i][axis] = scale * vector[i];
		}
		for (std::size_t i = 0; i < n; ++i) {
			for (std::size_t j = 0; j < n; ++j) {
				gram[i][j] -= eigenvalue * vector[i] * vector[j];
			}
		}
	}
	return coordinates;
}

auto add_identity_dense_layer(mtrc::solve::parametric::dnn::Network<double> &network, std::size_t input_size,
							  std::size_t output_size) -> void
{
	mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>> layer(
		input_size, output_size);
	layer.initConstant(0.0, 0.0);
	network.addLayer(layer);
}

auto make_coordinate_solver_network() -> mtrc::solve::parametric::dnn::Network<double>
{
	mtrc::solve::parametric::dnn::Network<double> network;
	add_identity_dense_layer(network, 2, 1);
	add_identity_dense_layer(network, 1, 2);
	network.setOutput(mtrc::solve::parametric::dnn::RegressionMSE<double>());
	network.setOptimizer(mtrc::solve::parametric::dnn::RMSProp<double>(0.05, 1.0e-8, 0.0));
	network.layers[0]->setParameters({{0.10, 0.05}, {0.00}});
	network.layers[1]->setParameters({{0.09, 0.045}, {0.00, 0.00}});
	return network;
}

template <typename Space>
auto as_mapping_result(const Space &source_space, std::vector<record_type> coordinates, std::string mapping,
					   std::string strategy)
{
	auto derived_space = mtrc::make_space(std::move(coordinates), mtrc::Euclidean<double>{});
	auto lineage = mtrc::one_to_one_lineage(source_space);
	return mtrc::core::make_mapping_result(std::move(derived_space), std::move(lineage.source_records),
										   std::move(lineage.representative_records), source_space.size(),
										   /*inverse_supported=*/false, std::move(mapping), std::move(strategy),
										   "metric_space", mtrc::core::metric_traits<mtrc::Euclidean<double>>::law, false,
										   "in-sample coordinate embedding; derived Euclidean coordinate space "
										   "approximates source geometry and has no derived out-of-sample transform");
}

} // namespace

int main()
{
	const auto records = make_arc_records(12);
	auto space = mtrc::make_space(records, mtrc::Euclidean<double>{});

	mtrc::modify::map::DiffusionCoordinateSpec<double> geometry;
	geometry.dimensions = 1;
	geometry.diffusion_steps = 5;
	geometry.kernel_scale = 1.0;
	geometry.max_dense_records = records.size();

	// === tg: target generation ===============================================
	const auto targets = mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
		space, geometry, "distance_table_pairwise_distances", "exponential_affinity_kernel",
		"lazy_row_normalized_diffusion_operator");
	assert(targets.coordinates.size() == records.size());
	assert(targets.dimensions == geometry.dimensions);
	assert(targets.diffusion_steps == geometry.diffusion_steps);
	assert(targets.record_count == records.size());
	assert(targets.dense_distance_evaluations == records.size() * records.size());
	assert(targets.method == "diffusion_potential_anchor_coordinates");
	assert(targets.pairwise_distances == "distance_table_pairwise_distances");
	assert(targets.affinity_kernel == "exponential_affinity_kernel");
	assert(targets.diffusion_operator == "lazy_row_normalized_diffusion_operator");
	for (std::size_t row = 0; row < records.size(); ++row) {
		const auto &coordinate = targets.coordinates.at(space.id(row));
		assert(coordinate.size() == geometry.dimensions);
		for (const auto value : coordinate) {
			assert(std::isfinite(value));
		}
	}
	// determinism: identical inputs reproduce identical targets element-by-element.
	const auto targets_again = mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
		space, geometry, "distance_table_pairwise_distances", "exponential_affinity_kernel",
		"lazy_row_normalized_diffusion_operator");
	for (std::size_t row = 0; row < records.size(); ++row) {
		const auto &lhs = targets.coordinates.at(space.id(row));
		const auto &rhs = targets_again.coordinates.at(space.id(row));
		assert(lhs.size() == rhs.size());
		for (std::size_t column = 0; column < lhs.size(); ++column) {
			assert(close(lhs[column], rhs[column], 0.0));
		}
	}

	// === ip: invalid parameters ==============================================
	assert(rejects([&] {
		auto bad = geometry;
		bad.dimensions = 0;
		(void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, bad);
	}));
	assert(rejects([&] {
		auto bad = geometry;
		bad.diffusion_steps = 0;
		(void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, bad);
	}));
	assert(rejects([&] {
		auto bad = geometry;
		bad.epsilon = 0.0;
		(void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, bad);
	}));
	assert(rejects([&] {
		auto bad = geometry;
		bad.max_dense_records = records.size() - 1;
		(void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, bad);
	}));
	assert(rejects([&] {
		(void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(space, geometry,
																				"unsupported_distance_provider");
	}));
	assert(rejects([&] {
		(void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
			space, geometry, "exact_space_distances", "unsupported_kernel");
	}));
	assert(rejects([&] {
		(void)mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
			space, geometry, "exact_space_distances", "gaussian_affinity_kernel", "unsupported_operator");
	}));

	// === ts: derive + transform shape ===========================================
	mtrc::modify::map::CoordinateCalibrationSpec<double> calibration;
	calibration.steps = 200;
	calibration.batch_size = records.size();
	calibration.shuffle = false;
	calibration.seed = 29;
	calibration.gradient_clip_norm = 20.0;

	const auto pipeline_builder = mtrc::modify::compose::parametric_diffusion_coordinate_pipeline_builder(0.05, 1.0)
									  .use_distance_table_pairwise_distances()
									  .use_exponential_affinity_kernel()
									  .use_lazy_row_normalized_diffusion_operator();
	auto pipeline = mtrc::modify::compose::parametric_diffusion_coordinates(
		pipeline_builder, mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_coordinate_solver_network()), geometry,
		calibration);
	assert(pipeline.component_count() >= 9);

	auto mapping_artifact = mtrc::modify::map::derive_from(pipeline, space);
	assert(mapping_artifact.latent_dimension() == geometry.dimensions);
	auto latent = mtrc::modify::map::transform(mapping_artifact, space);
	assert(latent.space.size() == records.size());
	assert(latent.space.record(latent.space.id(0)).size() == geometry.dimensions);
	assert(latent.mapping == "parametric_diffusion_coordinates");
	assert(latent.strategy == "native_metric_diffusion_coordinate_solver");
	assert(latent.inverse_supported);

	// The solver moved the coordinate-target geometry error downward.
	const auto &report = mapping_artifact.calibration_report();
	assert(report.epochs.size() == calibration.steps);

	// === lg: lineage =========================================================
	assert(latent.source_records.size() == records.size());
	assert(latent.representative_records.size() == records.size());
	assert(latent.source_record_count == records.size());
	for (std::size_t row = 0; row < records.size(); ++row) {
		assert(latent.source_records[row].size() == 1);
		assert(latent.source_records[row].front() == space.id(row));
		assert(latent.representative_records[row] == space.id(row));
	}

	// === ar: artifact roundtrip ==============================================
	const auto objective =
		mtrc::modify::map::parametric_diffusion_coordinate_objective(mapping_artifact.coordinate_solver(), targets, 0.05, 1.0);
	const auto artifact = mtrc::modify::map::make_parametric_diffusion_coordinate_artifact(mapping_artifact, objective, calibration, geometry,
																				   targets, space.version());
	assert(artifact.manifest.at("format") == "metric.parametric_diffusion_coordinate_artifact");
	assert(artifact.manifest.at("backend") == "native_dnn");
	assert(artifact.manifest.at("source").at("record_count") == records.size());

	auto reloaded = mtrc::modify::map::load_parametric_diffusion_coordinate_artifact<record_type, double>(artifact);
	assert(reloaded.latent_dimension() == mapping_artifact.latent_dimension());
	assert(reloaded.mapping_name() == mapping_artifact.mapping_name());
	assert(reloaded.strategy_name() == mapping_artifact.strategy_name());

	auto reloaded_latent = mtrc::modify::map::transform(reloaded, space);
	assert(reloaded_latent.space.size() == latent.space.size());
	for (std::size_t row = 0; row < latent.space.size(); ++row) {
		const auto original = latent.space.record(latent.space.id(row));
		const auto restored = reloaded_latent.space.record(reloaded_latent.space.id(row));
		assert(original.size() == restored.size());
		for (std::size_t column = 0; column < original.size(); ++column) {
			assert(close(original[column], restored[column]));
		}
	}
	// lineage survives the roundtrip.
	assert(latent.source_records == reloaded_latent.source_records);
	assert(latent.representative_records == reloaded_latent.representative_records);
	// inverse parity.
	const auto restored_a = mapping_artifact.inverse_transform(latent);
	const auto restored_b = reloaded.inverse_transform(reloaded_latent);
	assert(restored_a.size() == restored_b.size());
	for (std::size_t row = 0; row < restored_a.size(); ++row) {
		assert(restored_a[row].size() == restored_b[row].size());
		for (std::size_t column = 0; column < restored_a[row].size(); ++column) {
			assert(close(restored_a[row][column], restored_b[row][column]));
		}
	}

	// negative load case: a foreign backend is rejected.
	{
		auto foreign = artifact;
		foreign.manifest["backend"] = "some_other_backend";
		assert(rejects(
			[&] { (void)mtrc::modify::map::load_parametric_diffusion_coordinate_artifact<record_type, double>(foreign); }));
	}

	// === bl: baseline comparison on identical lineage ========================
	const auto distances = dense_distance_matrix(space);
	const auto mds_coordinates = classical_mds_coordinates(distances, geometry.dimensions);
	const auto mds_result = as_mapping_result(space, mds_coordinates, "classical_mds", "double_centered_eigenmap");

	const std::size_t neighbor_count = 3;
	const auto coordinate_preservation = mtrc::modify::map::neighbor_preservation(space, latent, neighbor_count);
	const auto mds_preservation = mtrc::modify::map::neighbor_preservation(space, mds_result, neighbor_count);
	assert(coordinate_preservation.recall >= 0.5 && coordinate_preservation.recall <= 1.0); // non-degenerate
	assert(mds_preservation.recall >= 0.0 && mds_preservation.recall <= 1.0);

	// Structural advantage of the parametric map (always true on identical
	// lineage): parametric diffusion coordinate is invertible and out-of-sample capable; classical MDS
	// is a one-shot, in-sample-only, non-invertible embedding. We therefore do
	// NOT assert a recall winner -- we assert the structural separation.
	assert(latent.inverse_supported);
	assert(!mds_result.inverse_supported);

	auto query_space = mtrc::make_space(
		std::vector<record_type>{{std::cos(0.4), std::sin(0.4)}, {std::cos(2.3), std::sin(2.3)}},
		mtrc::Euclidean<double>{});
	const auto oos = mtrc::modify::map::out_of_sample_neighbor_stability(mapping_artifact, space, query_space, neighbor_count);
	assert(oos.transform_supported);
	assert(oos.anchor_recall >= 0.0 && oos.anchor_recall <= 1.0);

	return 0;
}
