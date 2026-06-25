#include <cassert>
#include <cstddef>
#include <cmath>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct CountingAbsoluteDistance {
	std::shared_ptr<std::size_t> calls;

	auto operator()(int lhs, int rhs) const -> int
	{
		++*calls;
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

struct HistogramWasserstein1D {
	auto operator()(const std::vector<int> &lhs, const std::vector<int> &rhs) const -> int
	{
		assert(lhs.size() == rhs.size());
		int cumulative = 0;
		int total = 0;
		for (std::size_t index = 0; index < lhs.size(); ++index) {
			cumulative += lhs[index] - rhs[index];
			total += cumulative < 0 ? -cumulative : cumulative;
		}
		return total;
	}
};

auto close(double actual, double expected, double tolerance = 1e-12) -> bool
{
	return std::abs(actual - expected) < tolerance;
}

auto assert_weights_close(const std::vector<double> &actual, const std::vector<double> &expected) -> void
{
	assert(actual.size() == expected.size());
	for (std::size_t index = 0; index < expected.size(); ++index) {
		assert(close(actual[index], expected[index]));
	}
}

auto assert_weight_mass(const std::vector<std::size_t> &multiplicities, const std::vector<double> &weights,
						std::size_t source_record_count) -> void
{
	assert(multiplicities.size() == weights.size());
	assert(std::accumulate(multiplicities.begin(), multiplicities.end(), std::size_t{0}) == source_record_count);
	const auto weight_sum = std::accumulate(weights.begin(), weights.end(), 0.0);
	assert(source_record_count == 0 ? close(weight_sum, 0.0) : close(weight_sum, 1.0));
}

int main()
{
	const auto line = mtrc::make_space(std::vector<int>{0, 1, 2, 3, 4}, AbsoluteDistance{});

	auto manual_space = mtrc::make_space(std::vector<int>{0, 4}, AbsoluteDistance{});
	const auto manual = mtrc::core::make_compression_result(
		std::move(manual_space), std::vector<mtrc::RecordId>{line.id(0), line.id(4)},
		std::vector<std::size_t>{0, 1, 1}, std::vector<int>{0, 2, 1}, 3, "manual_compression",
		"manual_strategy", "manual_provider", false, true, false);
	assert(manual.source_record_count == 3);
	assert(manual.compressed_record_count == 2);
	assert(manual.size() == 2);
	assert((manual.source_record_ids == std::vector<mtrc::RecordId>{line.id(0), line.id(4)}));
	assert((manual.assignments == std::vector<std::size_t>{0, 1, 1}));
	assert((manual.nearest_representative_distances == std::vector<int>{0, 2, 1}));
	assert((manual.representative_multiplicities == std::vector<std::size_t>{1, 2}));
	assert_weights_close(manual.representative_weights, std::vector<double>{1.0 / 3.0, 2.0 / 3.0});
	assert_weight_mass(manual.representative_multiplicities, manual.representative_weights, manual.source_record_count);
	assert(close(manual.compression_ratio, 2.0 / 3.0));
	assert(!manual.exact);
	assert(manual.operator_name == "compress");
	assert(manual.compression == "manual_compression");
	assert(manual.strategy == "manual_strategy");
	assert(manual.representation == "manual_provider");
	assert(manual.lossy);
	assert(!manual.inverse_supported);

	auto empty_manual_space = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
	const auto empty_manual = mtrc::core::make_compression_result(
		std::move(empty_manual_space), std::vector<mtrc::RecordId>{}, std::vector<std::size_t>{},
		std::vector<int>{}, 0, "empty_compression", "empty_strategy", "empty_provider");
	assert(empty_manual.empty());
	assert(empty_manual.source_record_count == 0);
	assert(empty_manual.compressed_record_count == 0);
	assert(empty_manual.compression_ratio == 0.0);
	assert(empty_manual.representative_multiplicities.empty());
	assert(empty_manual.representative_weights.empty());
	assert(empty_manual.exact);
	assert(empty_manual.lossy);
	assert(!empty_manual.inverse_supported);

	const auto compressed = mtrc::compress(line, 3);
	assert(compressed.source_record_count == 5);
	assert(compressed.compressed_record_count == 3);
	assert(compressed.size() == 3);
	assert((compressed.source_record_ids == std::vector<mtrc::RecordId>{line.id(0), line.id(4), line.id(2)}));
	assert((compressed.assignments == std::vector<std::size_t>{0, 0, 2, 2, 1}));
	assert((compressed.nearest_representative_distances == std::vector<int>{0, 1, 0, 1, 0}));
	assert((compressed.representative_multiplicities == std::vector<std::size_t>{2, 1, 2}));
	assert_weights_close(compressed.representative_weights, std::vector<double>{0.4, 0.2, 0.4});
	assert_weight_mass(compressed.representative_multiplicities, compressed.representative_weights,
					   compressed.source_record_count);
	assert(compressed.space.record(compressed.space.id(0)) == 0);
	assert(compressed.space.record(compressed.space.id(1)) == 4);
	assert(compressed.space.record(compressed.space.id(2)) == 2);
	assert(close(compressed.compression_ratio, 0.6));
	assert(compressed.exact);
	assert(compressed.operator_name == "compress");
	assert(compressed.compression == "representatives");
	assert(compressed.strategy == "farthest_first");
	assert(compressed.representation == "metric_space");
	assert(compressed.lossy);
	assert(!compressed.inverse_supported);

	const auto identity = mtrc::compress(line, line.size());
	assert(identity.exact);
	assert(identity.strategy == "identity");
	assert(identity.representation == "metric_space");
	assert(!identity.lossy);
	assert(identity.inverse_supported);
	assert(identity.source_record_count == line.size());
	assert(identity.compressed_record_count == line.size());
	assert(identity.source_record_ids.size() == line.size());
	assert(identity.assignments.size() == line.size());
	assert(identity.nearest_representative_distances.size() == line.size());
	assert(identity.representative_multiplicities.size() == line.size());
	assert(identity.representative_weights.size() == line.size());
	assert_weight_mass(identity.representative_multiplicities, identity.representative_weights,
					   identity.source_record_count);
	for (std::size_t index = 0; index < line.size(); ++index) {
		assert(identity.source_record_ids[index] == line.id(index));
		assert(identity.assignments[index] == index);
		assert(identity.nearest_representative_distances[index] == 0);
		assert(identity.representative_multiplicities[index] == 1);
		assert(close(identity.representative_weights[index], 1.0 / static_cast<double>(line.size())));
	}

	const auto materialized_policy = mtrc::space::storage::materialized(mtrc::space::storage::exact());
	const auto materialized = mtrc::compress(line, 3, materialized_policy);
	assert(materialized.representation == "distance_table");
	assert(materialized.source_record_ids == compressed.source_record_ids);
	assert(materialized.assignments == compressed.assignments);
	assert(materialized.nearest_representative_distances == compressed.nearest_representative_distances);
	assert(materialized.representative_multiplicities == compressed.representative_multiplicities);
	assert_weights_close(materialized.representative_weights, compressed.representative_weights);
	assert(materialized.space.record(materialized.space.id(1)) == compressed.space.record(compressed.space.id(1)));

	const auto coverage = mtrc::compress(line, 3, mtrc::space::select::coverage{});
	assert(coverage.strategy == "coverage");
	assert(coverage.source_record_ids == compressed.source_record_ids);
	assert(coverage.assignments == compressed.assignments);
	assert(coverage.representative_multiplicities == compressed.representative_multiplicities);
	assert_weights_close(coverage.representative_weights, compressed.representative_weights);

	const auto k_center = mtrc::compress(line, 3, mtrc::space::select::k_center{});
	assert(k_center.strategy == "k_center");
	assert(k_center.source_record_ids == compressed.source_record_ids);
	assert(k_center.assignments == compressed.assignments);
	assert(k_center.representative_multiplicities == compressed.representative_multiplicities);
	assert_weights_close(k_center.representative_weights, compressed.representative_weights);

	const auto radius_coverage = mtrc::compress(line, mtrc::space::select::radius_coverage{1});
	assert(radius_coverage.strategy == "radius_coverage");
	assert(radius_coverage.representation == "metric_space");
	assert(radius_coverage.source_record_count == line.size());
	assert(radius_coverage.compressed_record_count == 3);
	assert((radius_coverage.source_record_ids == std::vector<mtrc::RecordId>{line.id(0), line.id(2), line.id(4)}));
	assert((radius_coverage.assignments == std::vector<std::size_t>{0, 0, 1, 1, 2}));
	assert((radius_coverage.nearest_representative_distances == std::vector<int>{0, 1, 0, 1, 0}));
	assert((radius_coverage.representative_multiplicities == std::vector<std::size_t>{2, 2, 1}));
	assert_weights_close(radius_coverage.representative_weights, std::vector<double>{0.4, 0.4, 0.2});
	assert_weight_mass(radius_coverage.representative_multiplicities, radius_coverage.representative_weights,
					   radius_coverage.source_record_count);

	const auto radius_coverage_two = mtrc::compress(line, mtrc::space::select::radius_coverage{2});
	assert(radius_coverage_two.strategy == "radius_coverage");
	assert(radius_coverage_two.compressed_record_count == 2);
	assert((radius_coverage_two.source_record_ids == std::vector<mtrc::RecordId>{line.id(0), line.id(3)}));
	assert((radius_coverage_two.assignments == std::vector<std::size_t>{0, 0, 1, 1, 1}));
	assert((radius_coverage_two.representative_multiplicities == std::vector<std::size_t>{2, 3}));
	assert_weights_close(radius_coverage_two.representative_weights, std::vector<double>{0.4, 0.6});

	const auto materialized_radius =
		mtrc::compress(line, mtrc::space::select::radius_coverage{1}, materialized_policy);
	assert(materialized_radius.representation == "distance_table");
	assert(materialized_radius.source_record_ids == radius_coverage.source_record_ids);
	assert(materialized_radius.assignments == radius_coverage.assignments);
	assert(materialized_radius.representative_multiplicities == radius_coverage.representative_multiplicities);
	assert_weights_close(materialized_radius.representative_weights, radius_coverage.representative_weights);

	const auto medoid_compressed = mtrc::compress(line, mtrc::k_medoids_options(2));
	assert(medoid_compressed.strategy == "k_medoids");
	assert(medoid_compressed.representation == "metric_space");
	assert(medoid_compressed.compressed_record_count == 2);
	assert((medoid_compressed.source_record_ids == std::vector<mtrc::RecordId>{line.id(0), line.id(3)}));
	assert((medoid_compressed.assignments == std::vector<std::size_t>{0, 0, 1, 1, 1}));
	assert((medoid_compressed.nearest_representative_distances == std::vector<int>{0, 1, 1, 0, 1}));
	assert((medoid_compressed.representative_multiplicities == std::vector<std::size_t>{2, 3}));
	assert_weights_close(medoid_compressed.representative_weights, std::vector<double>{0.4, 0.6});

	const auto materialized_medoids =
		mtrc::compress(line, mtrc::k_medoids_options(2), materialized_policy);
	assert(materialized_medoids.representation == "distance_table");
	assert(materialized_medoids.source_record_ids == medoid_compressed.source_record_ids);
	assert(materialized_medoids.assignments == medoid_compressed.assignments);
	assert(materialized_medoids.representative_multiplicities == medoid_compressed.representative_multiplicities);
	assert_weights_close(materialized_medoids.representative_weights, medoid_compressed.representative_weights);

	const auto seeded = mtrc::compress(line, 2, mtrc::space::select::farthest_first(2));
	assert((seeded.source_record_ids == std::vector<mtrc::RecordId>{line.id(2), line.id(0)}));
	assert((seeded.assignments == std::vector<std::size_t>{1, 1, 0, 0, 0}));
	assert((seeded.representative_multiplicities == std::vector<std::size_t>{3, 2}));
	assert_weights_close(seeded.representative_weights, std::vector<double>{0.6, 0.4});

	const auto materialized_seeded =
		mtrc::compress(line, 2, mtrc::space::select::farthest_first(2), materialized_policy);
	assert(materialized_seeded.representation == "distance_table");
	assert(materialized_seeded.source_record_ids == seeded.source_record_ids);
	assert(materialized_seeded.assignments == seeded.assignments);
	assert(materialized_seeded.representative_multiplicities == seeded.representative_multiplicities);
	assert_weights_close(materialized_seeded.representative_weights, seeded.representative_weights);

	bool rejected_zero = false;
	try {
		(void)mtrc::compress(line, 0);
	} catch (const std::invalid_argument &) {
		rejected_zero = true;
	}
	assert(rejected_zero);

	bool rejected_empty = false;
	try {
		const auto empty = mtrc::make_space(std::vector<int>{}, AbsoluteDistance{});
		(void)mtrc::compress(empty, 1);
	} catch (const std::invalid_argument &) {
		rejected_empty = true;
	}
	assert(rejected_empty);

	bool rejected_too_many = false;
	try {
		(void)mtrc::compress(line, line.size() + 1);
	} catch (const std::invalid_argument &) {
		rejected_too_many = true;
	}
	assert(rejected_too_many);

	bool rejected_negative_radius = false;
	try {
		(void)mtrc::compress(line, mtrc::space::select::radius_coverage{-1});
	} catch (const std::invalid_argument &) {
		rejected_negative_radius = true;
	}
	assert(rejected_negative_radius);

	const auto approximate = mtrc::compress(line, 2, mtrc::space::storage::approximate());
	assert(!approximate.exact);
	assert(approximate.representation == "sampled_metric_space");
	assert(approximate.strategy == "sampled_farthest_first");
	assert(approximate.source_record_count == line.size());
	assert(approximate.compressed_record_count == 2);
	assert((approximate.source_record_ids == std::vector<mtrc::RecordId>{line.id(0), line.id(4)}));
	assert((approximate.assignments == std::vector<std::size_t>{0, 0, 0, 1, 1}));
	assert((approximate.nearest_representative_distances == std::vector<int>{0, 1, 2, 1, 0}));
	assert((approximate.representative_multiplicities == std::vector<std::size_t>{3, 2}));
	assert_weights_close(approximate.representative_weights, std::vector<double>{0.6, 0.4});

	const auto words = mtrc::make_space(std::vector<std::string>{"cat", "cot", "coat", "dog"}, mtrc::Edit<char>{});
	const auto word_compressed = mtrc::compress(words, 2);
	assert(word_compressed.strategy == "farthest_first");
	assert((word_compressed.source_record_ids == std::vector<mtrc::RecordId>{words.id(0), words.id(3)}));
	assert((word_compressed.assignments == std::vector<std::size_t>{0, 0, 0, 1}));
	assert((word_compressed.representative_multiplicities == std::vector<std::size_t>{3, 1}));
	assert_weights_close(word_compressed.representative_weights, std::vector<double>{0.75, 0.25});
	assert(word_compressed.space.record(word_compressed.space.id(0)) == "cat");
	assert(word_compressed.space.record(word_compressed.space.id(1)) == "dog");

	const auto word_radius = mtrc::compress(words, mtrc::space::select::radius_coverage{1});
	assert(word_radius.strategy == "radius_coverage");
	assert((word_radius.source_record_ids == std::vector<mtrc::RecordId>{words.id(0), words.id(3)}));
	assert((word_radius.assignments == std::vector<std::size_t>{0, 0, 0, 1}));

	const auto histograms = mtrc::make_space(
		std::vector<std::vector<int>>{{2, 0, 0}, {1, 1, 0}, {0, 2, 0}, {0, 1, 1}, {0, 0, 2}},
		HistogramWasserstein1D{});
	const auto histogram_compressed = mtrc::compress(histograms, 3);
	assert(histogram_compressed.strategy == "farthest_first");
	assert((histogram_compressed.source_record_ids ==
			std::vector<mtrc::RecordId>{histograms.id(0), histograms.id(4), histograms.id(2)}));
	assert((histogram_compressed.assignments == std::vector<std::size_t>{0, 0, 2, 2, 1}));
	assert((histogram_compressed.representative_multiplicities == std::vector<std::size_t>{2, 1, 2}));
	assert_weights_close(histogram_compressed.representative_weights, std::vector<double>{0.4, 0.2, 0.4});
	assert_weight_mass(histogram_compressed.representative_multiplicities, histogram_compressed.representative_weights,
					   histogram_compressed.source_record_count);

	std::vector<int> large_records;
	large_records.reserve(4100);
	for (int value = 0; value < 4100; ++value) {
		large_records.push_back(value);
	}
	const auto large_line = mtrc::make_space(large_records, AbsoluteDistance{});
	const auto default_large_compress = mtrc::compress(large_line, 8);
	assert(!default_large_compress.exact);
	assert(default_large_compress.representation == "sampled_metric_space");
	assert(default_large_compress.strategy == "sampled_farthest_first");
	assert(default_large_compress.source_record_count == large_line.size());
	assert(default_large_compress.compressed_record_count == 8);
	assert(default_large_compress.assignments.size() == large_line.size());
	assert(default_large_compress.nearest_representative_distances.size() == large_line.size());
	assert(default_large_compress.representative_multiplicities.size() ==
		   default_large_compress.compressed_record_count);
	assert(default_large_compress.representative_weights.size() == default_large_compress.compressed_record_count);
	assert_weight_mass(default_large_compress.representative_multiplicities,
					   default_large_compress.representative_weights, default_large_compress.source_record_count);

	const auto bounded_approximate =
		mtrc::compress(large_line, 8, mtrc::space::select::farthest_first{},
					   mtrc::space::storage::using_knn_graph(32, mtrc::space::storage::approximate()));
	assert(!bounded_approximate.exact);
	assert(bounded_approximate.representation == "sampled_metric_space");
	assert(bounded_approximate.strategy == "sampled_farthest_first");
	assert(bounded_approximate.source_record_count == large_line.size());
	assert(bounded_approximate.compressed_record_count == 8);
	assert(bounded_approximate.assignments.size() == large_line.size());
	assert(bounded_approximate.nearest_representative_distances.size() == large_line.size());
	assert(bounded_approximate.representative_multiplicities.size() == bounded_approximate.compressed_record_count);
	assert(bounded_approximate.representative_weights.size() == bounded_approximate.compressed_record_count);
	assert_weight_mass(bounded_approximate.representative_multiplicities,
					   bounded_approximate.representative_weights, bounded_approximate.source_record_count);

	auto high_target_calls = std::make_shared<std::size_t>(0);
	const auto high_target_line = mtrc::make_space(large_records, CountingAbsoluteDistance{high_target_calls});
	const auto high_target_compress = mtrc::compress(high_target_line, 3000);
	assert(!high_target_compress.exact);
	assert(high_target_compress.representation == "sampled_metric_space");
	assert(high_target_compress.strategy == "sampled_regular");
	assert(high_target_compress.source_record_count == high_target_line.size());
	assert(high_target_compress.compressed_record_count == 3000);
	assert(high_target_compress.assignments.size() == high_target_line.size());
	assert(high_target_compress.nearest_representative_distances.size() == high_target_line.size());
	assert(high_target_compress.representative_multiplicities.size() ==
		   high_target_compress.compressed_record_count);
	assert(high_target_compress.representative_weights.size() == high_target_compress.compressed_record_count);
	assert_weight_mass(high_target_compress.representative_multiplicities,
					   high_target_compress.representative_weights, high_target_compress.source_record_count);
	assert(*high_target_calls <= high_target_line.size() * 512);
	assert(*high_target_calls < high_target_line.size() * high_target_compress.compressed_record_count / 2);

	return 0;
}
