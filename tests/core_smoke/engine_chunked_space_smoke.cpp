// Chunked space view smoke: stable RecordIds per chunk, bounded local exact pair work,
// snapshot behavior after parent mutation, and strict chunk-size validation.

#include <cassert>
#include <cmath>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "metric/core/metric_space.hpp"
#include "metric/space/chunked.hpp"
#include "metric/space/storage/execution.hpp"
#include "metric/stats/properties/distribution.hpp"
#include "metric/stats/properties/local_volume.hpp"
#include "metric/stats/search/neighbors.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct CountingAbsoluteDistance {
	std::shared_ptr<std::size_t> calls;

	explicit CountingAbsoluteDistance(std::shared_ptr<std::size_t> call_counter)
		: calls(std::move(call_counter))
	{
	}

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

template <typename Callable> static auto throws(Callable &&call) -> bool
{
	try {
		call();
	} catch (...) {
		return true;
	}
	return false;
}

static auto full_pair_count(std::size_t count) -> std::size_t
{
	return count < 2 ? std::size_t{0} : (count * (count - 1)) / 2;
}

static auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool
{
	return std::abs(lhs - rhs) < tolerance;
}

static auto test_chunked_pair_work_is_local() -> void
{
	constexpr std::size_t m = 4;
	constexpr std::size_t b = 3;
	const std::vector<int> records{0, 1, 2, 10, 11, 12, 20, 21, 22, 30, 31, 32};
	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(records, CountingAbsoluteDistance(calls));

	const auto chunks = mtrc::space::chunked_view(space, b);
	assert(*calls == 0);
	assert(chunks.record_count() == m * b);
	assert(chunks.chunk_size() == b);
	assert(chunks.chunk_count() == m);

	const auto expected_local_pairs = m * b * (b - 1) / 2;
	const auto expected_representative_pairs = full_pair_count(m);
	const auto expected_dense_pairs = full_pair_count(chunks.record_count());
	assert(chunks.total_local_pair_count() == expected_local_pairs);
	assert(mtrc::space::total_local_pair_count(chunks) == expected_local_pairs);
	assert(chunks.dense_pair_count() == expected_dense_pairs);
	assert(mtrc::space::dense_pair_count(chunks) == expected_dense_pairs);
	assert(expected_local_pairs == 12);
	assert(expected_dense_pairs == 66);
	assert(chunks.total_local_pair_count() < expected_dense_pairs);

	const auto plan = chunks.plan_diagnostics();
	assert(plan.representation == "chunked_space_view");
	assert(plan.exact);
	assert(plan.record_count == chunks.record_count());
	assert(plan.chunk_size == b);
	assert(plan.chunk_count == m);
	assert(plan.max_local_record_count == b);
	assert(plan.local_pair_distance_evaluations == expected_local_pairs);
	assert(plan.dense_pair_distance_evaluations == expected_dense_pairs);
	assert(plan.representative_count == m);
	assert(plan.representative_pair_distance_evaluations == expected_representative_pairs);
	assert(plan.refinement_chunk_pair_count == 0);
	assert(plan.refinement_candidate_pair_count == 0);
	assert(plan.refinement_pair_distance_evaluations == 0);
	assert(plan.bounded_pair_distance_evaluations == expected_local_pairs + expected_representative_pairs);
	assert(plan.bounded_pair_distance_evaluations < expected_dense_pairs);
	assert(plan.exact);
	assert(plan.exactness == "exact");
	assert(plan.source_record_ids == mtrc::record_ids(space));
	assert(plan.representative_ids.size() == m);
	assert(!plan.stale);
	assert(plan.warnings.empty());
	assert(mtrc::space::chunked_plan_diagnostics(chunks).bounded_pair_distance_evaluations ==
		   plan.bounded_pair_distance_evaluations);

	std::size_t chunk_visits = 0;
	const auto returned_chunks = mtrc::space::for_each_chunk(chunks, [&](const auto &chunk) {
		assert(chunk.index() == chunk_visits);
		assert(chunk.local_record_count() == b);
		++chunk_visits;
	});
	assert(returned_chunks == m);
	assert(chunk_visits == m);

	const auto representative_ids = mtrc::space::representative_ids(chunks);
	assert(representative_ids == plan.representative_ids);
	for (std::size_t chunk_index = 0; chunk_index < chunks.chunk_count(); ++chunk_index) {
		assert(representative_ids[chunk_index] == space.id(chunk_index * b));
	}

	std::size_t representative_visits = 0;
	const auto returned_representatives = mtrc::space::for_each_representative(
		chunks, [&](std::size_t chunk_index, mtrc::RecordId id, const int &record) {
			++representative_visits;
			assert(chunk_index < chunks.chunk_count());
			assert(id == representative_ids[chunk_index]);
			assert(record == records[chunk_index * b]);
		});
	assert(returned_representatives == m);
	assert(representative_visits == m);
	assert(*calls == 0);

	std::size_t visited = 0;
	int local_distance_sum = 0;
	const auto returned = mtrc::space::for_each_local_pair(
		chunks, [&](std::size_t chunk_index, mtrc::RecordId lhs, mtrc::RecordId rhs, int distance) {
			++visited;
			assert(chunk_index < chunks.chunk_count());
			assert(chunks.chunk(chunk_index).contains(lhs));
			assert(chunks.chunk(chunk_index).contains(rhs));
			assert(distance == AbsoluteDistance{}(chunks.record(lhs), chunks.record(rhs)));
			local_distance_sum += distance;
		});
	assert(returned == expected_local_pairs);
	assert(visited == expected_local_pairs);
	assert(local_distance_sum == 16);
	assert(*calls == expected_local_pairs);

	std::size_t representative_pair_visits = 0;
	int representative_distance_sum = 0;
	const auto returned_representative_pairs = mtrc::space::for_each_representative_pair(
		chunks,
		[&](std::size_t lhs_chunk, std::size_t rhs_chunk, mtrc::RecordId lhs, mtrc::RecordId rhs, int distance) {
			++representative_pair_visits;
			assert(lhs_chunk < rhs_chunk);
			assert(lhs == representative_ids[lhs_chunk]);
			assert(rhs == representative_ids[rhs_chunk]);
			assert(distance == AbsoluteDistance{}(chunks.record(lhs), chunks.record(rhs)));
			representative_distance_sum += distance;
		});
	assert(returned_representative_pairs == expected_representative_pairs);
	assert(representative_pair_visits == expected_representative_pairs);
	assert(representative_distance_sum == 100);
	assert(*calls == expected_local_pairs + expected_representative_pairs);

	for (std::size_t chunk_index = 0; chunk_index < chunks.chunk_count(); ++chunk_index) {
		const auto chunk = chunks.chunk(chunk_index);
		assert(chunk.local_record_count() == b);
		assert(mtrc::space::local_pair_count(chunk) == b * (b - 1) / 2);
		for (std::size_t local_position = 0; local_position < b; ++local_position) {
			const auto global_position = chunk_index * b + local_position;
			const auto id = space.id(global_position);
			assert(chunk.global_position(local_position) == global_position);
			assert(chunk.id(local_position) == id);
			assert(chunks.id(global_position) == id);
			assert(chunks.global_position(id) == global_position);
			assert(chunk.position_of(id) == local_position);
			assert(chunk.global_position_of(id) == global_position);
			assert(chunk.record(local_position) == records[global_position]);
		}
		assert(chunk.representative_id() == space.id(chunk_index * b));
		assert(chunk.representative_record() == records[chunk_index * b]);
	}
}

static auto test_chunked_operator_execution_is_bounded() -> void
{
	constexpr std::size_t m = 4;
	constexpr std::size_t b = 3;
	const std::vector<int> records{0, 1, 2, 10, 11, 12, 20, 21, 22, 30, 31, 32};
	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(records, CountingAbsoluteDistance(calls));
	const auto chunks = mtrc::space::chunked_view(space, b);

	const auto expected_local_pairs = m * b * (b - 1) / 2;
	const auto expected_representative_pairs = full_pair_count(m);
	const auto expected_dense_pairs = full_pair_count(records.size());
	assert(expected_local_pairs + expected_representative_pairs < expected_dense_pairs);

	const auto local_knn = chunks.local_neighbors(2);
	assert(local_knn.operation == "local_neighbors");
	assert(local_knn.scope == "local_chunks");
	assert(!local_knn.exact);
	assert(local_knn.exact_within_scope);
	assert(!local_knn.globally_exhaustive);
	assert(local_knn.requested_count == 2);
	assert(local_knn.record_count == records.size());
	assert(local_knn.chunk_size == b);
	assert(local_knn.chunk_count == m);
	assert(local_knn.local_pair_distance_evaluations == expected_local_pairs);
	assert(local_knn.representative_pair_distance_evaluations == 0);
	assert(local_knn.refinement_chunk_pair_count == 0);
	assert(local_knn.refinement_candidate_pair_count == 0);
	assert(local_knn.refinement_pair_distance_evaluations == 0);
	assert(local_knn.bounded_pair_distance_evaluations == expected_local_pairs);
	assert(local_knn.dense_pair_distance_evaluations == expected_dense_pairs);
	assert(local_knn.returned_neighbor_count == records.size() * 2);
	assert(*calls == expected_local_pairs);
	assert(*calls < expected_dense_pairs);

	for (std::size_t chunk_index = 0; chunk_index < chunks.chunk_count(); ++chunk_index) {
		const auto chunk = chunks.chunk(chunk_index);
		for (std::size_t local_position = 0; local_position < chunk.local_record_count(); ++local_position) {
			const auto id = chunk.id(local_position);
			const auto &neighbors = local_knn.neighbors(id);
			assert(neighbors.size() == 2);
			for (const auto &neighbor : neighbors) {
				assert(chunk.contains(neighbor.id));
				assert(neighbor.id != id);
			}
		}
	}

	*calls = 0;
	const auto local_range = mtrc::space::local_range(chunks, 1);
	assert(local_range.operation == "local_range");
	assert(local_range.scope == "local_chunks");
	assert(local_range.local_pair_distance_evaluations == expected_local_pairs);
	assert(local_range.representative_pair_distance_evaluations == 0);
	assert(local_range.refinement_chunk_pair_count == 0);
	assert(local_range.refinement_candidate_pair_count == 0);
	assert(local_range.refinement_pair_distance_evaluations == 0);
	assert(local_range.bounded_pair_distance_evaluations == expected_local_pairs);
	assert(local_range.dense_pair_distance_evaluations == expected_dense_pairs);
	assert(local_range.returned_neighbor_count == m * 4);
	assert(*calls == expected_local_pairs);
	assert(*calls < expected_dense_pairs);
	for (std::size_t chunk_index = 0; chunk_index < chunks.chunk_count(); ++chunk_index) {
		const auto chunk = chunks.chunk(chunk_index);
		for (std::size_t local_position = 0; local_position < chunk.local_record_count(); ++local_position) {
			const auto id = chunk.id(local_position);
			for (const auto &neighbor : local_range.neighbors(id)) {
				assert(chunk.contains(neighbor.id));
				assert(neighbor.distance <= 1);
			}
		}
	}

	*calls = 0;
	const auto representative_knn = mtrc::space::representative_neighbors(chunks, 1);
	assert(representative_knn.operation == "representative_neighbors");
	assert(representative_knn.scope == "chunk_representatives");
	assert(representative_knn.local_pair_distance_evaluations == 0);
	assert(representative_knn.representative_pair_distance_evaluations == expected_representative_pairs);
	assert(representative_knn.refinement_chunk_pair_count == 0);
	assert(representative_knn.refinement_candidate_pair_count == 0);
	assert(representative_knn.refinement_pair_distance_evaluations == 0);
	assert(representative_knn.bounded_pair_distance_evaluations == expected_representative_pairs);
	assert(representative_knn.dense_pair_distance_evaluations == expected_dense_pairs);
	assert(representative_knn.returned_neighbor_count == m);
	assert(*calls == expected_representative_pairs);
	assert(*calls < expected_dense_pairs);

	const auto representative_ids = chunks.representative_ids();
	for (std::size_t chunk_index = 0; chunk_index < chunks.chunk_count(); ++chunk_index) {
		const auto representative_id = representative_ids[chunk_index];
		const auto &neighbors = representative_knn.neighbors(representative_id);
		assert(neighbors.size() == 1);
		assert(neighbors.front().id != representative_id);
	}
	for (std::size_t position = 0; position < chunks.record_count(); ++position) {
		const auto id = chunks.id(position);
		if (position % b != 0) {
			assert(representative_knn.neighbors(id).empty());
		}
	}
}

static auto test_chunked_refinement_uses_bounded_cross_chunk_candidates() -> void
{
	constexpr std::size_t m = 4;
	constexpr std::size_t b = 3;
	constexpr std::size_t candidate_chunks_per_chunk = 1;
	const std::vector<int> records{0, 1, 2, 10, 11, 12, 20, 21, 22, 30, 31, 32};
	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(records, CountingAbsoluteDistance(calls));
	const auto chunks = mtrc::space::chunked_view(space, b);

	const auto expected_local_pairs = m * b * (b - 1) / 2;
	const auto expected_representative_pairs = full_pair_count(m);
	const auto expected_refinement_chunk_pairs = m - 1;
	const auto expected_refinement_candidate_pairs = expected_refinement_chunk_pairs * b * b;
	const auto expected_dense_pairs = full_pair_count(records.size());
	const auto expected_bounded_pairs =
		expected_local_pairs + expected_representative_pairs + expected_refinement_candidate_pairs;
	assert(expected_bounded_pairs == 45);
	assert(expected_dense_pairs == 66);
	assert(expected_bounded_pairs < expected_dense_pairs);

	const auto candidates = chunks.representative_candidates(candidate_chunks_per_chunk);
	assert(candidates.operation == "representative_candidates");
	assert(candidates.scope == "representative_cross_chunk_candidates");
	assert(!candidates.exact);
	assert(candidates.exact_within_scope);
	assert(!candidates.globally_exhaustive);
	assert(candidates.exactness == "non_exact_bounded_cross_chunk_candidates");
	assert(candidates.requested_candidate_chunks_per_chunk == candidate_chunks_per_chunk);
	assert(candidates.record_count == records.size());
	assert(candidates.chunk_size == b);
	assert(candidates.chunk_count == m);
	assert(candidates.representative_count == m);
	assert(candidates.local_pair_distance_evaluations == 0);
	assert(candidates.representative_pair_distance_evaluations == expected_representative_pairs);
	assert(candidates.refinement_chunk_pair_count == expected_refinement_chunk_pairs);
	assert(candidates.refinement_candidate_pair_count == expected_refinement_candidate_pairs);
	assert(candidates.refinement_pair_distance_evaluations == 0);
	assert(candidates.bounded_pair_distance_evaluations == expected_representative_pairs);
	assert(candidates.dense_pair_distance_evaluations == expected_dense_pairs);
	assert(candidates.refinement_chunk_pairs.size() == expected_refinement_chunk_pairs);
	assert(*calls == expected_representative_pairs);
	for (const auto &pair : candidates.refinement_chunk_pairs) {
		assert(pair.lhs_chunk + 1 == pair.rhs_chunk);
		assert(pair.lhs_representative_id == chunks.representative_id(pair.lhs_chunk));
		assert(pair.rhs_representative_id == chunks.representative_id(pair.rhs_chunk));
		assert(pair.representative_distance == 10);
	}

	*calls = 0;
	const auto exhaustive_candidates = mtrc::space::representative_candidates(chunks, m - 1);
	assert(exhaustive_candidates.exact);
	assert(exhaustive_candidates.globally_exhaustive);
	assert(exhaustive_candidates.exactness == "exact_cross_chunk_candidates");
	assert(exhaustive_candidates.refinement_chunk_pair_count == expected_representative_pairs);
	assert(exhaustive_candidates.refinement_candidate_pair_count == expected_representative_pairs * b * b);
	assert(exhaustive_candidates.refinement_pair_distance_evaluations == 0);
	assert(*calls == expected_representative_pairs);

	*calls = 0;
	const auto refined_knn = mtrc::space::refined_neighbors(chunks, 3, candidate_chunks_per_chunk);
	assert(refined_knn.operation == "refined_neighbors");
	assert(refined_knn.scope == "local_chunks_plus_representative_refinement");
	assert(!refined_knn.exact);
	assert(refined_knn.exact_within_scope);
	assert(!refined_knn.globally_exhaustive);
	assert(refined_knn.exactness == "non_exact_bounded_refinement");
	assert(refined_knn.requested_count == 3);
	assert(refined_knn.requested_candidate_chunks_per_chunk == candidate_chunks_per_chunk);
	assert(refined_knn.local_pair_distance_evaluations == expected_local_pairs);
	assert(refined_knn.representative_pair_distance_evaluations == expected_representative_pairs);
	assert(refined_knn.refinement_chunk_pair_count == expected_refinement_chunk_pairs);
	assert(refined_knn.refinement_candidate_pair_count == expected_refinement_candidate_pairs);
	assert(refined_knn.refinement_pair_distance_evaluations == expected_refinement_candidate_pairs);
	assert(refined_knn.bounded_pair_distance_evaluations == expected_bounded_pairs);
	assert(refined_knn.dense_pair_distance_evaluations == expected_dense_pairs);
	assert(refined_knn.returned_neighbor_count == records.size() * 3);
	assert(refined_knn.refinement_chunk_pairs.size() == expected_refinement_chunk_pairs);
	assert(*calls == expected_bounded_pairs);
	assert(*calls < expected_dense_pairs);

	std::size_t rows_with_cross_chunk_neighbors = 0;
	for (std::size_t position = 0; position < chunks.record_count(); ++position) {
		const auto id = chunks.id(position);
		const auto source_chunk = position / b;
		const auto &neighbors = refined_knn.neighbors(id);
		assert(neighbors.size() == 3);
		bool has_cross_chunk_neighbor = false;
		for (const auto &neighbor : neighbors) {
			assert(neighbor.id != id);
			assert(neighbor.distance == AbsoluteDistance{}(chunks.record(id), chunks.record(neighbor.id)));
			has_cross_chunk_neighbor = has_cross_chunk_neighbor || chunks.global_position(neighbor.id) / b != source_chunk;
		}
		if (has_cross_chunk_neighbor) {
			++rows_with_cross_chunk_neighbors;
		}
	}
	assert(rows_with_cross_chunk_neighbors > 0);

	*calls = 0;
	const auto refined_range = chunks.refined_range(10, candidate_chunks_per_chunk);
	assert(refined_range.operation == "refined_range");
	assert(refined_range.scope == "local_chunks_plus_representative_refinement");
	assert(!refined_range.exact);
	assert(refined_range.exact_within_scope);
	assert(!refined_range.globally_exhaustive);
	assert(refined_range.exactness == "non_exact_bounded_refinement");
	assert(refined_range.requested_candidate_chunks_per_chunk == candidate_chunks_per_chunk);
	assert(refined_range.local_pair_distance_evaluations == expected_local_pairs);
	assert(refined_range.representative_pair_distance_evaluations == expected_representative_pairs);
	assert(refined_range.refinement_chunk_pair_count == expected_refinement_chunk_pairs);
	assert(refined_range.refinement_candidate_pair_count == expected_refinement_candidate_pairs);
	assert(refined_range.refinement_pair_distance_evaluations == expected_refinement_candidate_pairs);
	assert(refined_range.bounded_pair_distance_evaluations == expected_bounded_pairs);
	assert(refined_range.dense_pair_distance_evaluations == expected_dense_pairs);
	assert(refined_range.returned_neighbor_count == 60);
	assert(*calls == expected_bounded_pairs);
	assert(*calls < expected_dense_pairs);
	for (std::size_t position = 0; position < chunks.record_count(); ++position) {
		const auto id = chunks.id(position);
		for (const auto &neighbor : refined_range.neighbors(id)) {
			assert(neighbor.id != id);
			assert(neighbor.distance <= 10);
			assert(neighbor.distance == AbsoluteDistance{}(chunks.record(id), chunks.record(neighbor.id)));
		}
	}
}

static auto test_chunked_distance_distribution_is_bounded() -> void
{
	constexpr std::size_t m = 4;
	constexpr std::size_t b = 3;
	const std::vector<int> records{0, 1, 2, 10, 11, 12, 20, 21, 22, 30, 31, 32};
	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(records, CountingAbsoluteDistance(calls));
	const auto chunks = mtrc::space::chunked_view(space, b);

	const auto expected_local_pairs = m * b * (b - 1) / 2;
	const auto expected_representative_pairs = full_pair_count(m);
	const auto expected_dense_pairs = full_pair_count(records.size());
	const auto expected_evaluated_pairs = expected_local_pairs + expected_representative_pairs;
	const auto expected_fraction =
		static_cast<double>(expected_evaluated_pairs) / static_cast<double>(expected_dense_pairs);
	assert(expected_evaluated_pairs < expected_dense_pairs);

	const auto distribution = mtrc::chunked_distance_distribution(chunks);
	assert(*calls == expected_evaluated_pairs);
	assert(distribution.record_count == records.size());
	assert(distribution.pair_count == expected_dense_pairs);
	assert(distribution.evaluated_pair_count == expected_evaluated_pairs);
	assert(distribution.sample_count == expected_evaluated_pairs);
	assert(!distribution.exact);
	assert(distribution.algorithm == "chunked_distance_distribution");
	assert(distribution.representation == "chunked_space_view");
	assert(distribution.minimum == 1);
	assert(distribution.maximum == 30);
	assert(distribution.bucket_count() == 10);
	assert(distribution.histogram_edges.size() == 11);
	assert(distribution.approximation_quality.diagnostic == "distance_distribution_approximation");
	assert(distribution.approximation_quality.candidate_policy == "local_chunks_plus_representative_pairs");
	assert(distribution.approximation_quality.candidate_count == expected_evaluated_pairs);
	assert(distribution.approximation_quality.candidate_universe == expected_dense_pairs);
	assert(distribution.approximation_quality.distance_evaluations == expected_evaluated_pairs);
	assert(distribution.approximation_quality.sample_count == expected_evaluated_pairs);
	assert(distribution.approximation_quality.sample_universe == expected_dense_pairs);
	assert(distribution.approximation_quality.candidate_fraction == expected_fraction);
	assert(distribution.approximation_quality.sample_fraction == expected_fraction);
	assert(distribution.approximation_quality.standard_error > 0.0);
	assert(distribution.approximation_quality.confidence_radius_95 >
		   distribution.approximation_quality.standard_error);

	*calls = 0;
	auto empty_space = mtrc::make_space(std::vector<int>{}, CountingAbsoluteDistance(calls));
	const auto empty_distribution =
		mtrc::stats::properties::chunked_distance_distribution(mtrc::space::chunked_view(empty_space, b));
	assert(*calls == 0);
	assert(empty_distribution.record_count == 0);
	assert(empty_distribution.pair_count == 0);
	assert(empty_distribution.evaluated_pair_count == 0);
	assert(empty_distribution.sample_count == 0);
	assert(!empty_distribution.exact);
	assert(empty_distribution.empty());
	assert(empty_distribution.approximation_quality.candidate_count == 0);
	assert(empty_distribution.approximation_quality.candidate_universe == 0);
	assert(empty_distribution.approximation_quality.sample_fraction == 1.0);

	*calls = 0;
	auto single_space = mtrc::make_space(std::vector<int>{42}, CountingAbsoluteDistance(calls));
	const auto single_distribution = mtrc::stats::chunked_distance_distribution(
		mtrc::space::chunked_view(single_space, b));
	assert(*calls == 0);
	assert(single_distribution.record_count == 1);
	assert(single_distribution.pair_count == 0);
	assert(single_distribution.evaluated_pair_count == 0);
	assert(single_distribution.sample_count == 0);
	assert(!single_distribution.exact);
	assert(single_distribution.empty());
	assert(single_distribution.approximation_quality.sample_count == 0);
	assert(single_distribution.approximation_quality.sample_universe == 0);
}

static auto test_chunked_local_volume_is_bounded() -> void
{
	constexpr std::size_t m = 4;
	constexpr std::size_t b = 3;
	const std::vector<int> records{0, 1, 2, 10, 11, 12, 20, 21, 22, 30, 31, 32};
	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(records, CountingAbsoluteDistance(calls));
	const auto chunks = mtrc::space::chunked_view(space, b);

	const auto expected_local_pairs = m * b * (b - 1) / 2;
	const auto expected_representative_pairs = full_pair_count(m);
	const auto expected_dense_pairs = full_pair_count(records.size());
	const auto expected_evaluated_pairs = expected_local_pairs + expected_representative_pairs;
	const auto expected_fraction =
		static_cast<double>(expected_evaluated_pairs) / static_cast<double>(expected_dense_pairs);
	assert(expected_evaluated_pairs < expected_dense_pairs);

	const auto volume = mtrc::chunked_local_volume(chunks, 10);
	assert(*calls == expected_evaluated_pairs);
	assert(volume.record_count == records.size());
	assert(volume.size() == records.size());
	assert(volume.radius == 10);
	assert(volume.evaluated_distance_count == expected_evaluated_pairs);
	assert(volume.sample_count == expected_evaluated_pairs);
	assert(volume.sample_universe == expected_dense_pairs);
	assert(!volume.exact);
	assert(volume.algorithm == "chunked_local_volume");
	assert(volume.representation == "chunked_space_view");
	assert(volume.minimum_count == 3);
	assert(volume.maximum_count == 5);
	assert(close(volume.average_count, 3.5));
	assert(close(volume.minimum_density, 3.0 / 12.0));
	assert(close(volume.maximum_density, 5.0 / 12.0));
	assert(close(volume.average_density, 3.5 / 12.0));
	assert(volume.counts[0] == 4);
	assert(volume.counts[3] == 5);
	assert(volume.counts[6] == 5);
	assert(volume.counts[9] == 4);
	assert(volume.counts[1] == 3);
	assert(volume.counts[11] == 3);
	assert(volume.approximation_quality.diagnostic == "local_volume_approximation");
	assert(volume.approximation_quality.candidate_policy == "local_chunks_plus_representative_pairs");
	assert(volume.approximation_quality.candidate_count == expected_evaluated_pairs);
	assert(volume.approximation_quality.candidate_universe == expected_dense_pairs);
	assert(volume.approximation_quality.distance_evaluations == expected_evaluated_pairs);
	assert(volume.approximation_quality.sample_count == expected_evaluated_pairs);
	assert(volume.approximation_quality.sample_universe == expected_dense_pairs);
	assert(volume.approximation_quality.candidate_fraction == expected_fraction);
	assert(volume.approximation_quality.sample_fraction == expected_fraction);
	assert(volume.approximation_quality.standard_error > 0.0);
	assert(volume.approximation_quality.confidence_radius_95 > volume.approximation_quality.standard_error);
	assert(!volume.approximation_quality.reason.empty());
	assert(*calls < expected_dense_pairs);
	assert(*calls < records.size() * records.size());

	*calls = 0;
	auto empty_space = mtrc::make_space(std::vector<int>{}, CountingAbsoluteDistance(calls));
	const auto empty_volume =
		mtrc::stats::properties::chunked_local_volume(mtrc::space::chunked_view(empty_space, b), 1);
	assert(*calls == 0);
	assert(empty_volume.record_count == 0);
	assert(empty_volume.evaluated_distance_count == 0);
	assert(empty_volume.sample_count == 0);
	assert(empty_volume.sample_universe == 0);
	assert(!empty_volume.exact);
	assert(empty_volume.empty());
	assert(empty_volume.approximation_quality.candidate_count == 0);
	assert(empty_volume.approximation_quality.candidate_universe == 0);
	assert(empty_volume.approximation_quality.sample_fraction == 1.0);

	*calls = 0;
	auto single_space = mtrc::make_space(std::vector<int>{42}, CountingAbsoluteDistance(calls));
	const auto single_volume =
		mtrc::stats::chunked_local_volume(mtrc::space::chunked_view(single_space, b), 1);
	assert(*calls == 0);
	assert(single_volume.record_count == 1);
	assert(single_volume.size() == 1);
	assert(single_volume.counts.front() == 1);
	assert(single_volume.densities.front() == 1.0);
	assert(single_volume.evaluated_distance_count == 0);
	assert(single_volume.sample_count == 0);
	assert(single_volume.sample_universe == 0);
	assert(!single_volume.exact);
	assert(single_volume.approximation_quality.sample_count == 0);
	assert(single_volume.approximation_quality.sample_universe == 0);
}

static auto test_chunked_local_volume_profile_reuses_bounded_pair_work() -> void
{
	constexpr std::size_t m = 4;
	constexpr std::size_t b = 3;
	const std::vector<int> records{0, 1, 2, 10, 11, 12, 20, 21, 22, 30, 31, 32};
	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(records, CountingAbsoluteDistance(calls));
	const auto chunks = mtrc::space::chunked_view(space, b);
	const std::vector<int> radii{0, 1, 10, 20};

	const auto expected_local_pairs = m * b * (b - 1) / 2;
	const auto expected_representative_pairs = full_pair_count(m);
	const auto expected_dense_pairs = full_pair_count(records.size());
	const auto expected_evaluated_pairs = expected_local_pairs + expected_representative_pairs;
	const auto expected_dense_profile_pairs = expected_dense_pairs * radii.size();
	const auto expected_fraction =
		static_cast<double>(expected_evaluated_pairs) / static_cast<double>(expected_dense_pairs);

	const auto profile = mtrc::chunked_local_volume_profile(chunks, radii);
	assert(*calls == expected_evaluated_pairs);
	assert(*calls < expected_dense_pairs);
	assert(*calls * 4 < expected_dense_profile_pairs);
	assert(profile.record_count == records.size());
	assert(profile.size() == radii.size());
	assert(profile.evaluated_distance_count == expected_evaluated_pairs);
	assert(profile.sample_count == expected_evaluated_pairs);
	assert(profile.sample_universe == expected_dense_pairs);
	assert(!profile.exact);
	assert(profile.algorithm == "chunked_local_volume_profile");
	assert(profile.representation == "chunked_space_view");
	assert(profile.approximation_quality.diagnostic == "local_volume_approximation");
	assert(profile.approximation_quality.candidate_policy == "local_chunks_plus_representative_pairs");
	assert(profile.approximation_quality.candidate_count == expected_evaluated_pairs);
	assert(profile.approximation_quality.candidate_universe == expected_dense_pairs);
	assert(profile.approximation_quality.distance_evaluations == expected_evaluated_pairs);
	assert(profile.approximation_quality.sample_count == expected_evaluated_pairs);
	assert(profile.approximation_quality.sample_universe == expected_dense_pairs);
	assert(profile.approximation_quality.candidate_fraction == expected_fraction);
	assert(profile.approximation_quality.sample_fraction == expected_fraction);
	assert(profile.approximation_quality.standard_error > 0.0);
	assert(profile.approximation_quality.confidence_radius_95 > profile.approximation_quality.standard_error);
	assert(!profile.approximation_quality.reason.empty());

	assert(profile.entries[0].radius == 0);
	assert(profile.entries[0].minimum_count == 1);
	assert(profile.entries[0].maximum_count == 1);
	assert(close(profile.entries[0].average_count, 1.0));
	assert(close(profile.entries[0].average_density, 1.0 / 12.0));

	assert(profile.entries[1].radius == 1);
	assert(profile.entries[1].minimum_count == 2);
	assert(profile.entries[1].maximum_count == 3);
	assert(close(profile.entries[1].average_count, 7.0 / 3.0));
	assert(close(profile.entries[1].average_density, 7.0 / 36.0));

	assert(profile.entries[2].radius == 10);
	assert(profile.entries[2].minimum_count == 3);
	assert(profile.entries[2].maximum_count == 5);
	assert(close(profile.entries[2].average_count, 3.5));
	assert(close(profile.entries[2].average_density, 3.5 / 12.0));

	assert(profile.entries[3].radius == 20);
	assert(profile.entries[3].minimum_count == 3);
	assert(profile.entries[3].maximum_count == 6);
	assert(close(profile.entries[3].average_count, 46.0 / 12.0));
	assert(close(profile.entries[3].average_density, 46.0 / 144.0));

	*calls = 0;
	mtrc::stats::properties::local_volume_options tight_profile_storage;
	tight_profile_storage.max_profile_cells = records.size();
	assert(throws([&] { (void)mtrc::chunked_local_volume_profile(chunks, radii, tight_profile_storage); }));
	assert(*calls == 0);
}

static auto test_estimate_cost_reports_chunked_workflow_plan() -> void
{
	constexpr std::size_t m = 4;
	constexpr std::size_t b = 3;
	const std::vector<int> records{0, 1, 2, 10, 11, 12, 20, 21, 22, 30, 31, 32};
	auto space = mtrc::make_space(records, AbsoluteDistance{});

	const auto expected_local_pairs = m * b * (b - 1) / 2;
	const auto expected_representative_pairs = full_pair_count(m);
	const auto expected_dense_pairs = full_pair_count(records.size());
	const auto expected_bounded_pairs = expected_local_pairs + expected_representative_pairs;

	const auto dense_workflow_plan =
		mtrc::space::storage::estimate_cost(space, "workflow_context",
											mtrc::space::storage::using_distance_table());
	assert(dense_workflow_plan.representation == "distance_table");
	assert(dense_workflow_plan.estimated_distance_evaluations == records.size() * records.size());
	assert(!dense_workflow_plan.chunked_plan);

	auto materialized_policy = mtrc::space::storage::using_distance_table();
	materialized_policy = mtrc::space::storage::with_distance_table_budget(materialized_policy, b, 0);
	materialized_policy = mtrc::space::storage::allow_chunking_fallback(materialized_policy);
	const auto materialized_plan =
		mtrc::space::storage::estimate_cost(space, "workflow_context", materialized_policy);
	assert(materialized_plan.budget_exceeded);
	assert(materialized_plan.allowed);
	assert(materialized_plan.downgraded);
	assert(!materialized_plan.refused);
	assert(materialized_plan.exact);
	assert(materialized_plan.exactness == "exact_local_pairs");
	assert(materialized_plan.representation == "chunked_workflow_plan");
	assert(materialized_plan.chunked_plan);
	assert(materialized_plan.chunk_size == b);
	assert(materialized_plan.chunk_count == m);
	assert(materialized_plan.max_local_record_count == b);
	assert(materialized_plan.local_pair_distance_evaluations == expected_local_pairs);
	assert(materialized_plan.dense_pair_distance_evaluations == expected_dense_pairs);
	assert(materialized_plan.representative_count == m);
	assert(materialized_plan.representative_pair_distance_evaluations == expected_representative_pairs);
	assert(materialized_plan.bounded_pair_distance_evaluations == expected_bounded_pairs);
	assert(materialized_plan.estimated_distance_evaluations == expected_bounded_pairs);
	assert(materialized_plan.bounded_pair_distance_evaluations < materialized_plan.dense_pair_distance_evaluations);
	assert(materialized_plan.estimated_distance_evaluations < dense_workflow_plan.estimated_distance_evaluations);

	const auto diagnostics =
		mtrc::space::storage::diagnostics_for_space(space, materialized_policy, {}, "workflow_context");
	assert(diagnostics.representation == "chunked_workflow_plan");
	assert(diagnostics.chunked_plan);
	assert(diagnostics.chunk_size == b);
	assert(diagnostics.chunk_count == m);
	assert(diagnostics.local_pair_distance_evaluations == expected_local_pairs);
	assert(diagnostics.dense_pair_distance_evaluations == expected_dense_pairs);
	assert(diagnostics.bounded_pair_distance_evaluations == expected_bounded_pairs);
	assert(diagnostics.estimated_distance_evaluations == expected_bounded_pairs);

	mtrc::space::storage::resource_budget budget;
	budget.max_dense_records = b;
	budget.max_distance_evaluations = expected_bounded_pairs;
	budget.allow_chunking = true;
	const auto lazy_policy = mtrc::space::storage::with_resource_budget(mtrc::space::storage::exact(), budget);
	const auto lazy_plan = mtrc::space::storage::estimate_cost(space, "workflow_context", lazy_policy);
	assert(lazy_plan.budget_exceeded);
	assert(lazy_plan.allowed);
	assert(lazy_plan.downgraded);
	assert(lazy_plan.representation == "chunked_workflow_plan");
	assert(lazy_plan.chunked_plan);
	assert(lazy_plan.dense_pair_distance_evaluations == expected_dense_pairs);
	assert(lazy_plan.bounded_pair_distance_evaluations == expected_bounded_pairs);
	assert(lazy_plan.estimated_distance_evaluations == expected_bounded_pairs);
	assert(lazy_plan.reason.find("max_distance_evaluations") != std::string::npos);
}

static auto test_planner_dispatches_chunked_search_when_approximation_is_allowed() -> void
{
	constexpr std::size_t m = 4;
	constexpr std::size_t b = 3;
	constexpr std::size_t candidate_chunks_per_chunk = 1;
	const std::vector<int> records{0, 1, 2, 10, 11, 12, 20, 21, 22, 30, 31, 32};
	const auto calls = std::make_shared<std::size_t>(0);
	auto space = mtrc::make_space(records, CountingAbsoluteDistance(calls));

	const auto expected_local_pairs = m * b * (b - 1) / 2;
	const auto expected_representative_pairs = full_pair_count(m);
	const auto expected_refinement_chunk_pairs = m - 1;
	const auto expected_refinement_candidate_pairs = expected_refinement_chunk_pairs * b * b;
	const auto expected_dense_pairs = full_pair_count(records.size());
	const auto expected_bounded_pairs =
		expected_local_pairs + expected_representative_pairs + expected_refinement_candidate_pairs;
	const auto expected_query_chunk_evaluations = m + b - 1;
	assert(expected_bounded_pairs < expected_dense_pairs);

	auto search_policy = mtrc::space::storage::using_distance_table();
	search_policy = mtrc::space::storage::with_distance_table_budget(search_policy, b, 0);
	search_policy = mtrc::space::storage::allow_approximate_fallback(search_policy);
	search_policy = mtrc::space::storage::allow_chunking_fallback(search_policy);
	assert(mtrc::space::storage::chunked_search_candidate_chunks_per_chunk(m, search_policy) ==
		   candidate_chunks_per_chunk);

	const auto search_plan = mtrc::space::storage::estimate_cost(space, "neighbors", search_policy);
	assert(*calls == 0);
	assert(search_plan.budget_exceeded);
	assert(search_plan.allowed);
	assert(search_plan.downgraded);
	assert(!search_plan.refused);
	assert(!search_plan.exact);
	assert(search_plan.representation == "chunked_space_view");
	assert(search_plan.chunked_plan);
	assert(search_plan.chunk_size == b);
	assert(search_plan.chunk_count == m);
	assert(search_plan.local_pair_distance_evaluations == expected_local_pairs);
	assert(search_plan.representative_pair_distance_evaluations == expected_representative_pairs);
	assert(search_plan.dense_pair_distance_evaluations == expected_dense_pairs);
	assert(search_plan.bounded_pair_distance_evaluations < expected_dense_pairs);

	const auto diagnostics =
		mtrc::space::storage::diagnostics_for_space(space, search_policy, {}, "neighbors");
	assert(diagnostics.representation == "chunked_space_view");
	assert(diagnostics.chunked_plan);
	assert(!diagnostics.exact);
	assert(diagnostics.bounded_pair_distance_evaluations == search_plan.bounded_pair_distance_evaluations);

	*calls = 0;
	const auto neighbors = mtrc::find_neighbors(space, space.id(0), 3, search_policy);
	assert(neighbors.representation == "chunked_space_view");
	assert(!neighbors.exact);
	assert(neighbors.operator_name == "knn");
	assert(neighbors.requested_count == 3);
	assert(neighbors.size() == 3);
	assert(neighbors.provenance.route == "chunked_search_refinement");
	assert(neighbors.approximation_quality.diagnostic == "chunked_search_refinement");
	assert(neighbors.approximation_quality.candidate_policy == "local_chunks_plus_representative_refinement");
	assert(neighbors.approximation_quality.distance_evaluations == expected_bounded_pairs);
	assert(neighbors.approximation_quality.sample_count == expected_refinement_chunk_pairs);
	assert(!neighbors.approximation_quality.recall_measured);
	assert(*calls == expected_bounded_pairs);
	assert(*calls < expected_dense_pairs);
	for (const auto &neighbor : neighbors) {
		assert(neighbor.id != space.id(0));
		assert(neighbor.distance == AbsoluteDistance{}(space.record(space.id(0)), space.record(neighbor.id)));
	}

	*calls = 0;
	const auto nearby = mtrc::stats::search::range(space, space.id(0), 10, search_policy);
	assert(nearby.representation == "chunked_space_view");
	assert(!nearby.exact);
	assert(nearby.operator_name == "range");
	assert(nearby.size() == 3);
	assert(nearby.approximation_quality.diagnostic == "chunked_search_refinement");
	assert(nearby.approximation_quality.distance_evaluations == expected_bounded_pairs);
	assert(*calls == expected_bounded_pairs);
	assert(*calls < expected_dense_pairs);
	for (const auto &neighbor : nearby) {
		assert(neighbor.id != space.id(0));
		assert(neighbor.distance <= 10);
		assert(neighbor.distance == AbsoluteDistance{}(space.record(space.id(0)), space.record(neighbor.id)));
	}

	*calls = 0;
	const auto query_neighbors = mtrc::find_neighbors(space, 0, 3, search_policy);
	assert(query_neighbors.representation == "chunked_space_view");
	assert(!query_neighbors.exact);
	assert(query_neighbors.operator_name == "knn");
	assert(query_neighbors.size() == 3);
	assert(query_neighbors[0].id == space.id(0));
	assert(query_neighbors.approximation_quality.diagnostic == "chunked_search_refinement");
	assert(query_neighbors.approximation_quality.candidate_policy == "query_representative_chunk_refinement");
	assert(query_neighbors.approximation_quality.candidate_count == b);
	assert(query_neighbors.approximation_quality.distance_evaluations == expected_query_chunk_evaluations);
	assert(*calls == expected_query_chunk_evaluations);
	assert(*calls < expected_dense_pairs);

	*calls = 0;
	const auto query_range = mtrc::stats::search::range(space, 0, 2, search_policy);
	assert(query_range.representation == "chunked_space_view");
	assert(!query_range.exact);
	assert(query_range.operator_name == "range");
	assert(query_range.size() == 3);
	assert(query_range.approximation_quality.diagnostic == "chunked_search_refinement");
	assert(query_range.approximation_quality.candidate_policy == "query_representative_chunk_refinement");
	assert(query_range.approximation_quality.distance_evaluations == expected_query_chunk_evaluations);
	assert(*calls == expected_query_chunk_evaluations);
	assert(*calls < expected_dense_pairs);
	for (const auto &neighbor : query_range) {
		assert(neighbor.distance <= 2);
		assert(neighbor.distance == AbsoluteDistance{}(0, space.record(neighbor.id)));
	}

	auto exact_chunking_policy = mtrc::space::storage::using_distance_table();
	exact_chunking_policy = mtrc::space::storage::with_distance_table_budget(exact_chunking_policy, b, 0);
	exact_chunking_policy = mtrc::space::storage::allow_chunking_fallback(exact_chunking_policy);
	const auto exact_chunking_plan =
		mtrc::space::storage::estimate_cost(space, "neighbors", exact_chunking_policy);
	assert(exact_chunking_plan.exact);
	assert(exact_chunking_plan.representation == "chunked_distance_table");
	assert(mtrc::space::storage::uses_blocked_exact_fallback(exact_chunking_plan));
}

static auto test_chunked_view_is_snapshot_with_stale_version() -> void
{
	auto space = mtrc::make_space(std::vector<int>{0, 1, 2, 10, 11, 12}, AbsoluteDistance{});
	const auto id0 = space.id(0);
	const auto id4 = space.id(4);
	const auto version = space.version();
	const auto chunks = mtrc::space::chunked_view(space, 3);

	assert(chunks.version() == version);
	assert(chunks.built_for_version() == version);
	assert(!chunks.is_stale());
	assert(chunks.contains(id0));
	assert(chunks.position_of(id4) == 4);
	assert(chunks.record(id4) == 11);

	assert(space.erase(id0));
	space.replace(id4, 111);

	assert(chunks.is_stale());
	assert(chunks.source_version() == space.version());
	const auto stale_plan = chunks.plan_diagnostics();
	assert(stale_plan.stale);
	assert(stale_plan.space_version == space.version());
	assert(stale_plan.built_for_version == version);
	assert(stale_plan.source_record_ids.front() == id0);
	assert(!stale_plan.warnings.empty());
	assert(chunks.contains(id0));
	assert(chunks.position_of(id0) == 0);
	assert(chunks.position_of(id4) == 4);
	assert(chunks.record(id4) == 11);
	assert(chunks.chunk(0).contains(id0));
	assert(chunks.chunk(1).contains(id4));
	assert(chunks.distance(id0, id4) == 11);
}

static auto test_invalid_chunk_size() -> void
{
	auto space = mtrc::make_space(std::vector<int>{1, 2}, AbsoluteDistance{});
	assert(throws([&] { (void)mtrc::space::chunked_view(space, 0); }));
}

int main()
{
	test_chunked_pair_work_is_local();
	test_chunked_operator_execution_is_bounded();
	test_chunked_refinement_uses_bounded_cross_chunk_candidates();
	test_chunked_distance_distribution_is_bounded();
	test_chunked_local_volume_is_bounded();
	test_chunked_local_volume_profile_reuses_bounded_pair_work();
	test_estimate_cost_reports_chunked_workflow_plan();
	test_planner_dispatches_chunked_search_when_approximation_is_allowed();
	test_chunked_view_is_snapshot_with_stale_version();
	test_invalid_chunk_size();
	return 0;
}
