#include <cassert>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <metric/numeric/Math.h>

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"
#include "metric/solve/parametric/dnn.hpp"

namespace {

using Matrix = mtrc::numeric::DynamicMatrix<double>;
using record_type = std::vector<double>;

// Framing: the native DNN is a solver component under mtrc::solve::parametric::dnn,
// while the composed PHATE-AE workflow lives under mtrc::modify. These fully
// qualified paths must resolve to classes -- there is no mtrc::phate or
// mtrc::autoencoder product namespace.
static_assert(std::is_class<mtrc::solve::parametric::dnn::AutoencoderModel<double>>::value,
			  "native DNN solver must live under mtrc::solve::parametric::dnn");
static_assert(std::is_class<mtrc::modify::map::NativePhateAutoencoderMapping<double>>::value,
			  "PHATE-AE mapping must live under mtrc::modify::map");
static_assert(std::is_class<mtrc::modify::compose::NativePhateAutoencoderPipeline<double>>::value,
			  "PHATE-AE pipeline must live under mtrc::modify::compose");

auto is_finite(double value) -> bool { return std::isfinite(value); }

auto close_to(double lhs, double rhs, double tolerance = 1.0e-12) -> bool { return std::abs(lhs - rhs) <= tolerance; }

// Independent oracle: position of the nearest other 1-D record (Euclidean on a
// one-component vector is the absolute difference), with ties broken by lower
// position to match the deterministic lower-id tie-break in knn.
auto nearest_other(const std::vector<record_type> &records, std::size_t row) -> std::size_t
{
	std::size_t best = (row == 0) ? 1 : 0;
	double best_distance = std::abs(records[row][0] - records[best][0]);
	for (std::size_t index = 0; index < records.size(); ++index) {
		if (index == row) {
			continue;
		}
		const double distance = std::abs(records[row][0] - records[index][0]);
		if (distance < best_distance) {
			best_distance = distance;
			best = index;
		}
	}
	return best;
}

auto add_identity_dense_layer(mtrc::solve::parametric::dnn::Network<double> &network, std::size_t input_size, std::size_t output_size)
	-> void
{
	mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>> layer(input_size, output_size);
	layer.initConstant(0.0, 0.0);
	network.addLayer(layer);
}

auto make_autoencoder_network(double learning_rate = 0.001) -> mtrc::solve::parametric::dnn::Network<double>
{
	mtrc::solve::parametric::dnn::Network<double> network;
	add_identity_dense_layer(network, 2, 1);
	add_identity_dense_layer(network, 1, 2);
	network.setOutput(mtrc::solve::parametric::dnn::RegressionMSE<double>());
	network.setOptimizer(mtrc::solve::parametric::dnn::RMSProp<double>(learning_rate, 1.0e-8, 0.0));
	network.layers[0]->setParameters({{0.10, 0.08}, {0.00}});
	network.layers[1]->setParameters({{0.09, 0.07}, {0.00, 0.00}});
	return network;
}

auto features_from_records(const std::vector<record_type> &records) -> Matrix
{
	mtrc::solve::parametric::dnn::VectorRecordCodec<record_type, double> codec(records.front().size());
	return codec.encode_batch(records);
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

auto initial_reconstruction(const std::vector<record_type> &records) -> std::vector<record_type>
{
	mtrc::solve::parametric::dnn::AutoencoderModel<double> model(make_autoencoder_network());
	const auto features = features_from_records(records);
	const auto reconstructed = model.decode(model.encode(features));
	mtrc::solve::parametric::dnn::VectorRecordCodec<record_type, double> codec(records.front().size());
	return codec.decode_batch(reconstructed);
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

} // namespace

int main()
{
	const std::vector<record_type> records{{0.0, 0.0}, {0.5, 0.5}, {1.0, 1.0}, {1.5, 1.5}, {2.0, 2.0}};
	auto space = mtrc::make_space(records, mtrc::Euclidean<double>{});

	mtrc::modify::map::PhateGeometrySpec<double> geometry;
	geometry.dimensions = 1;
	geometry.diffusion_steps = 2;
	geometry.kernel_scale = 1.0;

	const auto targets = mtrc::modify::map::phate_geometry_targets<decltype(space), double>(space, geometry);
	assert(targets.coordinates.size() == space.size());
	assert(targets.dimensions == 1);
	assert(targets.diffusion_steps == geometry.diffusion_steps);
	assert(targets.record_count == space.size());
	assert(targets.dense_distance_evaluations == space.size() * space.size());
	assert(targets.method == "diffusion_potential_anchor_coordinates");
	assert(targets.pairwise_distances == "exact_space_distances");
	for (std::size_t index = 0; index < space.size(); ++index) {
		const auto found = targets.coordinates.find(space.id(index));
		assert(found != targets.coordinates.end());
		assert(found->second.size() == 1);
		assert(is_finite(found->second[0]));
	}
	assert(targets.coordinates.at(space.id(0))[0] < targets.coordinates.at(space.id(4))[0]);

	auto limited_geometry = geometry;
	limited_geometry.max_dense_records = 2;
	bool rejected_dense_limit = false;
	try {
		(void)mtrc::modify::map::phate_geometry_targets<decltype(space), double>(space, limited_geometry);
	} catch (const std::invalid_argument &) {
		rejected_dense_limit = true;
	}
	assert(rejected_dense_limit);

	mtrc::solve::parametric::dnn::TrainingSpec<double> training;
	training.epochs = 160;
	training.batch_size = records.size();
	training.shuffle = false;
	training.seed = 23;
	training.gradient_clip_norm = 20.0;

	const auto pipeline_builder = mtrc::modify::compose::native_phate_autoencoder_pipeline_builder(0.05, 1.0);
	auto pipeline = mtrc::modify::compose::native_phate_autoencoder(
		pipeline_builder, mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_autoencoder_network()), geometry, training);
	static_assert(mtrc::Mapping_v<decltype(pipeline), decltype(space)>);
	assert(pipeline.name() == "native_phate_autoencoder_pipeline");
	assert(pipeline.plan().name() == pipeline.name());
	assert(pipeline.component_count() == 11);
	assert(pipeline.codec() == "vector_record_codec");
	assert(pipeline.pairwise_distances() == "exact_space_distances");
	assert(pipeline.affinity_kernel() == "gaussian_affinity_kernel");
	assert(pipeline.has_component("codec", "vector_record_codec"));
	assert(pipeline.has_component("pairwise_distances", "exact_space_distances"));
	assert(pipeline.has_component("affinity_kernel", "gaussian_affinity_kernel"));
	assert(pipeline.has_component("target_generator", "diffusion_potential_anchor_coordinates"));
	assert(pipeline.has_component("trainer", "native_dnn_autoencoder_trainer"));
	assert(pipeline.has_component("loss", "reconstruction_mse_loss"));
	assert(pipeline.has_component("loss", "bottleneck_coordinate_mse_loss"));
	assert(pipeline.has_component_parameter("loss", "reconstruction_mse_loss", "weight", "0.05"));
	assert(pipeline.has_component_parameter("loss", "bottleneck_coordinate_mse_loss", "weight", "1"));

	auto cached_training = training;
	cached_training.epochs = 2;
	const auto cached_distance_builder = mtrc::modify::compose::native_phate_autoencoder_pipeline_builder(0.05, 1.0)
											 .use_distance_table_pairwise_distances()
											 .use_exponential_affinity_kernel()
											 .use_lazy_row_normalized_diffusion_operator();
	auto cached_distance_pipeline = mtrc::modify::compose::native_phate_autoencoder(
		cached_distance_builder, mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_autoencoder_network()), geometry,
		cached_training);
	static_assert(mtrc::Mapping_v<decltype(cached_distance_pipeline), decltype(space)>);
	assert(cached_distance_pipeline.pairwise_distances() == "distance_table_pairwise_distances");
	assert(cached_distance_pipeline.affinity_kernel() == "exponential_affinity_kernel");
	assert(cached_distance_pipeline.diffusion_operator() == "lazy_row_normalized_diffusion_operator");
	assert(cached_distance_pipeline.has_component("pairwise_distances", "distance_table_pairwise_distances"));
	assert(cached_distance_pipeline.has_component("affinity_kernel", "exponential_affinity_kernel"));
	assert(cached_distance_pipeline.has_component("diffusion_operator", "lazy_row_normalized_diffusion_operator"));
	auto cached_distance_model = mtrc::modify::map::fit(cached_distance_pipeline, space);
	const auto cached_distance_reduced = mtrc::modify::map::transform(cached_distance_model, space);
	assert(cached_distance_reduced.mapping == "native_phate_autoencoder");
	assert(cached_distance_reduced.strategy == "native_dnn_phate_ae");
	assert(cached_distance_reduced.space.size() == space.size());

	const auto initial_restored = initial_reconstruction(records);
	const auto initial_reconstruction_error = reconstruction_mse(records, initial_restored);
	auto model = mtrc::modify::map::fit(pipeline, space);
	static_assert(mtrc::MappingModel_v<decltype(model), decltype(space)>);

	const auto &report = model.training_report();
	assert(report.epochs.size() == training.epochs);
	assert(report.epochs.front().terms.size() == 2);
	const auto initial_bottleneck = find_epoch_term(report.epochs.front(), "bottleneck_coordinate_mse");
	const auto final_bottleneck = find_epoch_term(report.epochs.back(), "bottleneck_coordinate_mse");
	assert(final_bottleneck < initial_bottleneck);

	const auto reduced = mtrc::modify::map::transform(model, space);
	assert(model.mapping_name() == "native_phate_autoencoder");
	assert(model.strategy_name() == "native_dnn_phate_ae");
	assert(reduced.mapping == "native_phate_autoencoder");
	assert(reduced.strategy == "native_dnn_phate_ae");
	assert(reduced.inverse_supported);
	assert(reduced.space.size() == space.size());
	assert(reduced.space.record(reduced.space.id(0)).size() == geometry.dimensions);
	assert(reduced.space.distance(reduced.space.id(0), reduced.space.id(1)) <
		   reduced.space.distance(reduced.space.id(0), reduced.space.id(4)));
	const auto preservation = mtrc::modify::map::neighbor_preservation(space, reduced, 1);
	assert(preservation.diagnostic == "neighbor_preservation");
	assert(preservation.source_record_count == space.size());
	assert(preservation.mapped_record_count == reduced.space.size());
	assert(preservation.evaluated_neighbor_count == 1);
	assert(preservation.source_distance_evaluations == space.size() * (space.size() - 1));
	assert(preservation.mapped_distance_evaluations == reduced.space.size() * (reduced.space.size() - 1));
	assert(preservation.recall >= 0.8);

	const std::vector<record_type> new_records{{0.25, 0.25}, {1.75, 1.75}};
	auto new_space = mtrc::make_space(new_records, mtrc::Euclidean<double>{});
	const auto oos_stability = mtrc::modify::map::out_of_sample_neighbor_stability(model, space, new_space, 1);
	assert(oos_stability.diagnostic == "out_of_sample_neighbor_stability");
	assert(oos_stability.mapping == "native_phate_autoencoder");
	assert(oos_stability.strategy == "native_dnn_phate_ae");
	assert(oos_stability.source_record_count == space.size());
	assert(oos_stability.query_record_count == new_space.size());
	assert(oos_stability.evaluated_neighbor_count == 1);
	assert(oos_stability.source_distance_evaluations == space.size() * new_space.size());
	assert(oos_stability.mapped_distance_evaluations == space.size() * new_space.size());
	assert(oos_stability.transform_supported);
	assert(oos_stability.anchor_recall >= 0.5);
	assert(oos_stability.first_anchor_matches <= new_space.size());
	assert(oos_stability.first_anchor_match_rate >= 0.0);
	assert(oos_stability.first_anchor_match_rate <= 1.0);
	assert(oos_stability.average_mapped_best_source_rank >= 1.0);
	assert(oos_stability.maximum_mapped_best_source_rank <= space.size());
	assert(oos_stability.average_best_distance_penalty >= 0.0);

	// Framing: the native solver self-identifies as the native_dnn backend.
	assert(mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_autoencoder_network()).backend_name() ==
		   "native_dnn");

	// lc1: the PHATE-AE mapping result carries an exact one-to-one lineage aligned
	// to the source space id order.
	assert(reduced.source_records.size() == space.size());
	assert(reduced.representative_records.size() == space.size());
	assert(reduced.source_record_count == space.size());
	for (std::size_t index = 0; index < space.size(); ++index) {
		assert(reduced.source_records[index].size() == 1);
		assert(reduced.source_records[index][0] == space.id(index));
		assert(reduced.representative_records[index] == space.id(index));
	}

	// lc2: lineage follows the SOURCE space ids, not the latent positional ids.
	// Erasing the first record leaves a non-default-id space {1,2,3,4}; the latent
	// space still receives fresh 0..N-1 ids while representative_records keep the
	// original source ids. This also pins the count contract: the result count
	// tracks the transformed space while the model keeps the fitted count.
	{
		auto shifted_space = mtrc::make_space(records, mtrc::Euclidean<double>{});
		assert(shifted_space.erase(shifted_space.id(0)));
		const auto shifted_reduced = mtrc::modify::map::transform(model, shifted_space);
		assert(shifted_reduced.space.size() == shifted_space.size());
		assert(shifted_reduced.representative_records == mtrc::record_ids(shifted_space));
		assert(shifted_reduced.representative_records[0] == mtrc::RecordId::from_index(1));
		assert(shifted_reduced.space.id(0) == mtrc::RecordId::from_index(0));
		for (std::size_t index = 0; index < shifted_space.size(); ++index) {
			assert(shifted_reduced.source_records[index].size() == 1);
			assert(shifted_reduced.source_records[index][0] == shifted_space.id(index));
		}
		assert(shifted_reduced.source_record_count == shifted_space.size());
		assert(model.source_record_count() == space.size());
	}

	// lc4: neighbor-preservation rejects tampered lineage on a real PHATE-AE
	// result -- both a source id outside the space and duplicated source ids.
	{
		auto outside_lineage = reduced;
		outside_lineage.source_records[0][0] = mtrc::RecordId::from_index(999);
		bool rejected_outside = false;
		try {
			(void)mtrc::modify::map::neighbor_preservation(space, outside_lineage, 1);
		} catch (const std::invalid_argument &) {
			rejected_outside = true;
		}
		assert(rejected_outside);

		auto duplicate_lineage = reduced;
		duplicate_lineage.source_records[1] = duplicate_lineage.source_records[0];
		bool rejected_duplicate = false;
		try {
			(void)mtrc::modify::map::neighbor_preservation(space, duplicate_lineage, 1);
		} catch (const std::invalid_argument &) {
			rejected_duplicate = true;
		}
		assert(rejected_duplicate);
	}

	// np1: neighbor preservation evaluated on the actual deterministic
	// diffusion-potential anchor coordinates of a 1-D chain. The kernel scale is
	// wide enough that no potential saturates against the epsilon clamp, so the
	// potential to anchor 0 grows monotonically with chain position. The diagnostic
	// recall is then pinned to an independent brute-force oracle over the generated
	// coordinates (no loose inequality).
	{
		const std::vector<record_type> chain_records{{0.0}, {1.0}, {2.0}, {3.0}, {4.0}};
		auto chain_space = mtrc::make_space(chain_records, mtrc::Euclidean<double>{});
		mtrc::modify::map::PhateGeometrySpec<double> chain_spec;
		chain_spec.dimensions = 1;
		chain_spec.diffusion_steps = 1;
		chain_spec.kernel_scale = 2.0;
		const auto chain_targets =
			mtrc::modify::map::phate_geometry_targets<decltype(chain_space), double>(chain_space, chain_spec);

		std::vector<record_type> coordinate_records;
		for (std::size_t index = 0; index < chain_space.size(); ++index) {
			coordinate_records.push_back(chain_targets.coordinates.at(chain_space.id(index)));
		}
		// Monotonicity invariant (robust to floating-point ties). The diffusion
		// potential to anchor 0 increases with chain position, so the generated
		// coordinates must preserve the chain order. A strict `>` between adjacent
		// values is fragile: two potentials can round to a tie and the sign of their
		// difference then depends on codegen. Instead require the sequence to be
		// non-decreasing up to a tolerance well above the FP noise floor (~1e-12 for
		// this 5x5 diffusion + log + centering pipeline) yet far below a genuine
		// inversion (the real per-step gaps here are O(0.1)), and to make genuine,
		// non-degenerate progress across the whole chain. The span check is not a
		// finiteness check: coordinates are centered and scaled to unit max-abs, so a
		// truly order-preserving chain spans >= ~1.0 (front < 0 < back, max|coord| ==
		// 1), while a collapsed/constant mapping would span ~0 and fail.
		const double coordinate_tolerance = 1.0e-9;
		for (std::size_t index = 1; index < coordinate_records.size(); ++index) {
			assert(coordinate_records[index][0] >= coordinate_records[index - 1][0] - coordinate_tolerance);
		}
		const double chain_span = coordinate_records.back()[0] - coordinate_records.front()[0];
		assert(chain_span > 0.5);

		mtrc::MetricSpace<record_type, mtrc::Euclidean<double>> coordinate_space(coordinate_records,
																				mtrc::Euclidean<double>{});
		auto chain_lineage = mtrc::one_to_one_lineage(chain_space);
		auto chain_mapping = mtrc::make_mapping_result(
			std::move(coordinate_space), std::move(chain_lineage.source_records),
			std::move(chain_lineage.representative_records), chain_space.size(), false, "native_phate_autoencoder",
			"diffusion_potential_anchor_coordinates", "metric_space");

		const auto chain_preservation = mtrc::modify::map::neighbor_preservation(chain_space, chain_mapping, 1);
		assert(chain_preservation.exact);
		assert(chain_preservation.evaluated_neighbor_count == 1);
		assert(chain_preservation.possible_neighbors == chain_space.size());

		std::size_t oracle_matches = 0;
		for (std::size_t row = 0; row < chain_space.size(); ++row) {
			if (nearest_other(chain_records, row) == nearest_other(coordinate_records, row)) {
				++oracle_matches;
			}
		}
		assert(chain_preservation.matched_neighbors == oracle_matches);
		assert(close_to(chain_preservation.recall,
						static_cast<double>(oracle_matches) / static_cast<double>(chain_space.size())));

		const auto repeated_preservation = mtrc::modify::map::neighbor_preservation(chain_space, chain_mapping, 1);
		assert(repeated_preservation.matched_neighbors == chain_preservation.matched_neighbors);
		assert(repeated_preservation.recall == chain_preservation.recall);
	}

	const auto restored = model.inverse_transform(reduced);
	const auto final_reconstruction_error = reconstruction_mse(records, restored);
	assert(final_reconstruction_error < initial_reconstruction_error);

	return 0;
}
