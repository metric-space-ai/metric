// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Metric-Space Mapping Pipeline -- hero application.
//
// This example frames parametric diffusion coordinate as a *finite metric-space mapping*: a finite
// metric space (records + a metric that prices every pair) is transformed into
// a derived coordinate space. It is deliberately not framed as calling a canned
// application mapping. The native parametric coordinate solver is only the *solver* of one
// interchangeable pipeline stage; the metric stays authoritative and visible
// from the source space all the way through to the diagnostics.
//
// The pipeline is composed of interchangeable, role-based components:
//
//   source space        -- the finite metric space (records + Euclidean metric)
//   metric values       -- the dense pairwise distance matrix the metric induces
//   target construction  -- diffusion-coordinate-potential anchor coordinates
//   coordinate calibration -- the C++ coordinate solver that calibrates to the target geometry
//   artifact             -- the serialized, reloadable mapping boundary
//   lineage              -- one-to-one provenance from derived back to source
//   diagnostics          -- neighbor preservation, reconstruction, OOS stability
//
// As a control we recode the *same metric values* with classical
// multidimensional scaling (a non-parametric linear distance embedding) and
// compare neighbor preservation against the parametric diffusion coordinate map on identical lineage.
// The contrast is the point: classical MDS preserves the raw (ambient) metric
// values, while the target here is built from the diffusion potential
// -log(diffused) of the metric space -- it reweights record relations by
// multi-scale connectivity (anchor-record potential coordinates) rather than raw
// distance. It is not itself a diffusion-map/MDS embedding of the potential, and
// only the parametric coordinate solver map offers an out-of-sample transform and an
// inverse. The asserted advantage below is that structural (invertible,
// out-of-sample) capability, not a claim that parametric diffusion coordinates beats MDS on neighbor recall.

#include <cassert>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>

namespace {

using record_type = std::vector<double>;

// ---------------------------------------------------------------------------
// Source space fixture: a curved 1-manifold (half-circle arc with a small,
// deterministic radial wobble) embedded in R^2. Ordered by arc angle, so the
// intrinsic structure is one-dimensional while the ambient space is planar.
// This is the canonical "unrolling" fixture that separates a diffusion map from
// a linear distance embedding.
// ---------------------------------------------------------------------------
auto make_arc_records(std::size_t count) -> std::vector<record_type>
{
	std::vector<record_type> records;
	records.reserve(count);
	const double pi = 3.14159265358979323846;
	for (std::size_t index = 0; index < count; ++index) {
		const double t = static_cast<double>(index) / static_cast<double>(count - 1);
		const double theta = pi * t;
		// Deterministic radial wobble keeps points off a perfect circle without
		// any RNG, so the fixture stays byte-stable across runs and platforms.
		// Cast to a signed type before subtracting: index % 3 is unsigned, so a
		// bare `(index % 3) - 1` would wrap 0 to SIZE_MAX.
		const double radius = 1.0 + 0.03 * static_cast<double>(static_cast<int>(index % 3) - 1);
		records.push_back({radius * std::cos(theta), radius * std::sin(theta)});
	}
	return records;
}

// Held-out query records: fresh points interpolated onto the same arc. They are
// never seen during calibration and exercise the parametric out-of-sample path.
auto make_query_records() -> std::vector<record_type>
{
	const double pi = 3.14159265358979323846;
	std::vector<record_type> records;
	for (const double t : {0.18, 0.52, 0.83}) {
		const double theta = pi * t;
		records.push_back({std::cos(theta), std::sin(theta)});
	}
	return records;
}

// ---------------------------------------------------------------------------
// Metric values: materialize the dense pairwise distance matrix induced by the
// source metric. This is the single source of truth consumed by BOTH the
// diffusion target and the classical-MDS baseline -- neither sees raw records.
// ---------------------------------------------------------------------------
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

auto mean_offdiagonal(const std::vector<std::vector<double>> &distances) -> double
{
	double total = 0.0;
	std::size_t count = 0;
	for (std::size_t row = 0; row < distances.size(); ++row) {
		for (std::size_t column = 0; column < distances.size(); ++column) {
			if (row == column) {
				continue;
			}
			total += distances[row][column];
			++count;
		}
	}
	return count == 0 ? 0.0 : total / static_cast<double>(count);
}

// ---------------------------------------------------------------------------
// Classical multidimensional scaling baseline.
//
// Double-centre the squared distances into a Gram matrix and extract the top
// eigenvectors with deterministic power iteration + deflation. This is a
// self-contained, dependency-free, distance-only linear embedding -- the naive
// control the diffusion map is measured against. It is intentionally NOT a
// product algorithm and lives only inside this example/test scope.
// ---------------------------------------------------------------------------
auto classical_mds_coordinates(const std::vector<std::vector<double>> &distances, std::size_t dimensions)
	-> std::vector<record_type>
{
	const std::size_t n = distances.size();
	// Gram matrix: B = -1/2 * J D2 J, where D2 holds squared distances and J is
	// the centring operator. By symmetry the row and column means coincide.
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
		// Deterministic, eigenvector-non-orthogonal seed vector.
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
		// Deflate so the next axis recovers the following eigenpair.
		for (std::size_t i = 0; i < n; ++i) {
			for (std::size_t j = 0; j < n; ++j) {
				gram[i][j] -= eigenvalue * vector[i] * vector[j];
			}
		}
	}
	return coordinates;
}

auto reconstruction_mse(const std::vector<record_type> &lhs, const std::vector<record_type> &rhs) -> double
{
	assert(lhs.size() == rhs.size());
	double squared_error = 0.0;
	for (std::size_t row = 0; row < lhs.size(); ++row) {
		assert(lhs[row].size() == rhs[row].size());
		for (std::size_t column = 0; column < lhs[row].size(); ++column) {
			const auto delta = lhs[row][column] - rhs[row][column];
			squared_error += delta * delta;
		}
	}
	return squared_error / static_cast<double>(lhs.size());
}

// Wrap an arbitrary derived embedding (here: classical MDS) into a MappingResult
// with one-to-one source lineage, so the SAME neighbor_preservation diagnostic
// scores both maps on identical footing.
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
	std::cout << std::fixed << std::setprecision(6);

	// -- compile-time framing: the source records are an aligned-vector metric
	//    space governed by a true metric law. -----------------------------------
	static_assert(mtrc::metric_traits<mtrc::Euclidean<double>>::law == mtrc::metric_law::metric,
				  "the source space is a true finite metric space");

	// === source space =========================================================
	const auto records = make_arc_records(16);
	auto space = mtrc::make_space(records, mtrc::Euclidean<double>{});
	assert(!space.empty());

	// === metric values ========================================================
	const auto distances = dense_distance_matrix(space);
	const auto mean_distance = mean_offdiagonal(distances);
	assert(mean_distance > 0.0);

	// === target construction ==================================================
	// Interchangeable target-construction components are declared on the plan
	// builder; here we promote the exponential affinity kernel and the lazy
	// row-normalized diffusion operator over the gaussian / strict defaults.
	mtrc::modify::map::DiffusionCoordinateSpec<double> geometry;
	geometry.dimensions = 1;
	geometry.diffusion_steps = 5;
	geometry.kernel_scale = 0.0; // 0 => auto-derive the bandwidth from the metric values
	geometry.max_dense_records = records.size();

	const auto pipeline_builder = mtrc::modify::compose::parametric_diffusion_coordinate_pipeline_builder(0.05, 1.0)
									  .use_distance_table_pairwise_distances()
									  .use_exponential_affinity_kernel()
									  .use_lazy_row_normalized_diffusion_operator();

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

	// === native coordinate calibration ========================================
	mtrc::modify::map::CoordinateCalibrationSpec<double> calibration;
	calibration.steps = 400;
	calibration.batch_size = records.size();
	calibration.shuffle = false;
	calibration.seed = 29;
	calibration.gradient_clip_norm = 20.0;

	mtrc::solve::parametric::LinearCoordinateSolverSpec<double> solver_spec;
	solver_spec.input_dimensions = records.front().size();
	solver_spec.coordinate_dimensions = geometry.dimensions;
	solver_spec.learning_rate = 0.05;
	solver_spec.encoder_weights = {0.10, 0.05};
	solver_spec.encoder_bias = {0.00};
	solver_spec.decoder_weights = {0.09, 0.045};
	solver_spec.decoder_bias = {0.00, 0.00};

	auto pipeline = mtrc::modify::compose::parametric_diffusion_coordinates(
		pipeline_builder, mtrc::solve::parametric::make_linear_coordinate_solver(solver_spec), geometry, calibration);
	assert(pipeline.name() == "parametric_diffusion_coordinate_pipeline");
	assert(pipeline.codec() == "vector_record_codec");
	assert(pipeline.pairwise_distances() == "distance_table_pairwise_distances");
	assert(pipeline.affinity_kernel() == "exponential_affinity_kernel");
	assert(pipeline.diffusion_operator() == "lazy_row_normalized_diffusion_operator");
	assert(pipeline.has_component("target_generator", "diffusion_potential_anchor_coordinates"));
	assert(pipeline.has_component("coordinate_calibration", "native_coordinate_calibration"));
	assert(pipeline.has_component("coordinate_artifact", "native_coordinate_mapping_artifact"));
	assert(pipeline.has_component("artifact", "native_coordinate_artifact"));
	assert(pipeline.has_component("loss", "reconstruction_mse_loss"));

	auto mapping_artifact = mtrc::modify::map::derive_from(pipeline, space);
	assert(mapping_artifact.latent_dimension() == geometry.dimensions);
	assert(mapping_artifact.has_pipeline_plan());

	const auto &report = mapping_artifact.calibration_report();
	const auto calibrated_steps = mtrc::solve::parametric::coordinate_calibration_step_count(report);
	assert(calibrated_steps == calibration.steps);
	const auto initial_coordinate_error = mtrc::solve::parametric::coordinate_calibration_target_error(
		report, mtrc::solve::parametric::CoordinateCalibrationPoint::first);
	const auto final_coordinate_error = mtrc::solve::parametric::coordinate_calibration_target_error(
		report, mtrc::solve::parametric::CoordinateCalibrationPoint::last);
	assert(final_coordinate_error < initial_coordinate_error);

	// === derived space + lineage ==============================================
	auto latent = mtrc::modify::map::transform(mapping_artifact, space);
	assert(latent.mapping == "parametric_diffusion_coordinates");
	assert(latent.strategy == "native_metric_diffusion_coordinate_solver");
	assert(latent.inverse_supported);
	assert(latent.space.size() == records.size());
	assert(latent.space.record(latent.space.id(0)).size() == geometry.dimensions);
	// one-to-one lineage: one source row per derived record, preserved order.
	assert(latent.source_records.size() == records.size());
	assert(latent.representative_records.size() == records.size());
	assert(latent.source_record_count == records.size());
	for (std::size_t row = 0; row < records.size(); ++row) {
		assert(latent.source_records[row].size() == 1);
		assert(latent.source_records[row].front() == space.id(row));
	}

	// === artifact + roundtrip =================================================
	const auto objective =
		mtrc::modify::map::parametric_diffusion_coordinate_objective(mapping_artifact.coordinate_solver(), targets, 0.05, 1.0);
	const auto artifact = mtrc::modify::map::make_parametric_diffusion_coordinate_artifact(mapping_artifact, objective, calibration, geometry,
																				   targets, space.version());
	assert(artifact.manifest.at("format") == "metric.parametric_diffusion_coordinate_artifact");
	assert(artifact.manifest.at("backend") == "native_dnn");
	assert(artifact.manifest.at("pipeline").at("components").size() == pipeline.component_count());

	auto reloaded = mtrc::modify::map::load_parametric_diffusion_coordinate_artifact<record_type, double>(artifact);
	auto reloaded_latent = mtrc::modify::map::transform(reloaded, space);
	// transform parity: the reloaded mapping reproduces the derived space exactly.
	assert(reloaded_latent.space.size() == latent.space.size());
	for (std::size_t row = 0; row < latent.space.size(); ++row) {
		const auto original = latent.space.record(latent.space.id(row));
		const auto restored = reloaded_latent.space.record(reloaded_latent.space.id(row));
		assert(original.size() == restored.size());
		for (std::size_t column = 0; column < original.size(); ++column) {
			assert(std::abs(original[column] - restored[column]) <= 1.0e-9);
		}
	}
	// lineage survives the artifact roundtrip.
	assert(latent.source_records == reloaded_latent.source_records);
	assert(latent.representative_records == reloaded_latent.representative_records);

	// inverse transform: decode the latent space back into source-shaped records.
	const auto restored_records = mapping_artifact.inverse_transform(latent);
	const auto reconstruction_error = reconstruction_mse(records, restored_records);
	assert(std::isfinite(reconstruction_error));

	// === diagnostics ==========================================================
	const std::size_t neighbor_count = 3;
	const auto coordinate_preservation = mtrc::modify::map::neighbor_preservation(space, latent, neighbor_count);

	auto query_space = mtrc::make_space(make_query_records(), mtrc::Euclidean<double>{});
	const auto oos_stability =
		mtrc::modify::map::out_of_sample_neighbor_stability(mapping_artifact, space, query_space, neighbor_count);
	assert(oos_stability.transform_supported);
	assert(oos_stability.anchor_recall >= 0.0 && oos_stability.anchor_recall <= 1.0);

	// === baseline comparison (classical MDS on the same metric values) ========
	const auto mds_coordinates = classical_mds_coordinates(distances, geometry.dimensions);
	const auto mds_result = as_mapping_result(space, mds_coordinates, "classical_mds", "double_centered_eigenmap");
	const auto mds_preservation = mtrc::modify::map::neighbor_preservation(space, mds_result, neighbor_count);

	const auto preservation_margin = coordinate_preservation.recall - mds_preservation.recall;
	// Honest baseline comparison. On a simple convex manifold a linear distance
	// embedding already preserves neighbors well, so we do NOT stake the demo on
	// parametric diffusion coordinate winning the in-sample recall race -- both recalls are reported. The
	// decisive, always-true advantage is structural: the parametric diffusion coordinate map is
	// parametric (out-of-sample capable) and invertible, while classical MDS is a
	// one-shot, in-sample-only, non-invertible embedding.
	assert(coordinate_preservation.recall >= 0.5);  // the derived geometry is non-degenerate
	assert(mds_preservation.recall >= 0.0 && mds_preservation.recall <= 1.0);
	assert(latent.inverse_supported);          // parametric diffusion coordinate: decoder inverse_transform
	assert(!mds_result.inverse_supported);     // classical MDS: no inverse
	assert(oos_stability.transform_supported); // parametric diffusion coordinate: out-of-sample transform
	assert(oos_stability.anchor_recall >= 0.5);// and it generalizes to held-out queries

	// === finite metric-space mapping report ===================================
	std::cout << "mapping_pipeline_source_space = finite_metric_space\n";
	std::cout << "mapping_pipeline_source_metric = euclidean\n";
	std::cout << "mapping_pipeline_source_records = " << records.size() << "\n";
	std::cout << "mapping_pipeline_source_dimension = " << records.front().size() << "\n";
	std::cout << "mapping_pipeline_metric_mean_distance = " << mean_distance << "\n";
	std::cout << "mapping_pipeline_metric_dense_evaluations = " << targets.dense_distance_evaluations << "\n";
	std::cout << "mapping_pipeline_target_method = " << targets.method << "\n";
	std::cout << "mapping_pipeline_target_dimensions = " << targets.dimensions << "\n";
	std::cout << "mapping_pipeline_target_diffusion_steps = " << targets.diffusion_steps << "\n";
	std::cout << "mapping_pipeline_target_kernel_scale = " << targets.kernel_scale << "\n";
	std::cout << "mapping_pipeline_component_codec = " << pipeline.codec() << "\n";
	std::cout << "mapping_pipeline_component_pairwise_distances = " << pipeline.pairwise_distances() << "\n";
	std::cout << "mapping_pipeline_component_affinity_kernel = " << pipeline.affinity_kernel() << "\n";
	std::cout << "mapping_pipeline_component_diffusion_operator = " << pipeline.diffusion_operator() << "\n";
	std::cout << "mapping_pipeline_component_count = " << pipeline.component_count() << "\n";
	std::cout << "mapping_pipeline_solver = native_coordinate_calibration\n";
	std::cout << "mapping_pipeline_calibration_steps = " << calibrated_steps << "\n";
	std::cout << "mapping_pipeline_calibration_seed = " << calibration.seed << "\n";
	std::cout << "mapping_pipeline_coordinate_target_error = " << initial_coordinate_error << " -> " << final_coordinate_error
			  << "\n";
	std::cout << "mapping_pipeline_artifact_format = "
			  << artifact.manifest.at("format").template get<std::string>() << "\n";
	std::cout << "mapping_pipeline_artifact_solver = native_coordinate_solver\n";
	std::cout << "mapping_pipeline_artifact_roundtrip = transform_and_lineage_parity\n";
	std::cout << "mapping_pipeline_lineage = one_to_one_source_records\n";
	std::cout << "mapping_pipeline_inverse_support = decoder_inverse_transform\n";
	std::cout << "mapping_pipeline_reconstruction_mse = " << reconstruction_error << "\n";
	std::cout << "mapping_pipeline_diffusion_coordinate_neighbor_recall = " << coordinate_preservation.recall << "\n";
	std::cout << "mapping_pipeline_diffusion_coordinate_out_of_sample_records = " << oos_stability.query_record_count << "\n";
	std::cout << "mapping_pipeline_diffusion_coordinate_out_of_sample_anchor_recall = " << oos_stability.anchor_recall << "\n";
	std::cout << "mapping_pipeline_baseline = classical_mds\n";
	std::cout << "mapping_pipeline_baseline_inverse_support = none\n";
	std::cout << "mapping_pipeline_baseline_out_of_sample_support = none\n";
	std::cout << "mapping_pipeline_baseline_neighbor_recall = " << mds_preservation.recall << "\n";
	std::cout << "mapping_pipeline_neighbor_recall_margin = " << preservation_margin << "\n";
	std::cout << "mapping_pipeline_parametric_advantage = out_of_sample_and_inverse_vs_none\n";

	return 0;
}
