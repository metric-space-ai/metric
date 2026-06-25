#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/engine.hpp"
#include "metric/space/cache.hpp"
#include "metric/space/storage/triangular_distance_table.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs > rhs ? lhs - rhs : rhs - lhs; }
};

struct ScaledAbsoluteDistance {
	int scale{1};
	auto operator()(int lhs, int rhs) const -> int { return scale * (lhs > rhs ? lhs - rhs : rhs - lhs); }
};

struct CountingAbsoluteDistance {
	int *calls{};

	auto operator()(int lhs, int rhs) const -> int
	{
		++(*calls);
		return lhs > rhs ? lhs - rhs : rhs - lhs;
	}
};

struct DirectedDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs <= rhs ? rhs - lhs : 100 + lhs - rhs; }
};

struct ZeroPseudoMetric {
	auto operator()(int, int) const -> int { return 0; }
};

struct DirectedProvider {
	using distance_type = int;

	std::vector<mtrc::RecordId> ids{mtrc::RecordId::from_index(10), mtrc::RecordId::from_index(20),
									  mtrc::RecordId::from_index(30)};

	auto distance(mtrc::RecordId lhs, mtrc::RecordId rhs) const -> distance_type
	{
		return 10 * static_cast<int>(position_of(lhs)) + static_cast<int>(position_of(rhs));
	}

	auto record_count() const -> std::size_t { return ids.size(); }
	auto id(std::size_t position) const -> mtrc::RecordId { return ids.at(position); }
	auto position_of(mtrc::RecordId id) const -> std::size_t
	{
		for (std::size_t index = 0; index < ids.size(); ++index) {
			if (ids[index] == id) {
				return index;
			}
		}
		throw std::out_of_range("unknown directed provider id");
	}
	auto contains(mtrc::RecordId id) const -> bool
	{
		for (const auto candidate : ids) {
			if (candidate == id) {
				return true;
			}
		}
		return false;
	}
	auto version() const -> std::size_t { return 7; }
	auto is_stale() const -> bool { return false; }
};

struct PositionCountingProvider {
	using distance_type = int;

	std::vector<mtrc::RecordId> ids{mtrc::RecordId::from_index(10), mtrc::RecordId::from_index(20),
									  mtrc::RecordId::from_index(30)};
	int *id_distance_calls{};
	int *position_distance_calls{};

	auto distance(mtrc::RecordId lhs, mtrc::RecordId rhs) const -> distance_type
	{
		++(*id_distance_calls);
		return distance_for_positions(position_of(lhs), position_of(rhs));
	}

	auto distance_at_position(std::size_t lhs_position, std::size_t rhs_position) const -> distance_type
	{
		++(*position_distance_calls);
		return distance_for_positions(lhs_position, rhs_position);
	}

	auto record_count() const -> std::size_t { return ids.size(); }
	auto id(std::size_t position) const -> mtrc::RecordId { return ids.at(position); }
	auto position_of(mtrc::RecordId id) const -> std::size_t
	{
		for (std::size_t index = 0; index < ids.size(); ++index) {
			if (ids[index] == id) {
				return index;
			}
		}
		throw std::out_of_range("unknown position-counting provider id");
	}
	auto contains(mtrc::RecordId id) const -> bool
	{
		for (const auto candidate : ids) {
			if (candidate == id) {
				return true;
			}
		}
		return false;
	}
	auto version() const -> std::size_t { return 11; }
	auto is_stale() const -> bool { return false; }

	static auto distance_for_positions(std::size_t lhs_position, std::size_t rhs_position) -> distance_type
	{
		return lhs_position > rhs_position
				   ? static_cast<distance_type>(lhs_position - rhs_position)
				   : static_cast<distance_type>(rhs_position - lhs_position);
	}
};

struct LargeCountingProvider {
	using distance_type = int;

	std::size_t count{};
	int *distance_calls{};

	auto distance(mtrc::RecordId lhs, mtrc::RecordId rhs) const -> distance_type
	{
		++(*distance_calls);
		return static_cast<distance_type>(rhs.index() - lhs.index());
	}

	auto distance_at_position(std::size_t lhs_position, std::size_t rhs_position) const -> distance_type
	{
		++(*distance_calls);
		return static_cast<distance_type>(rhs_position - lhs_position);
	}

	auto record_count() const -> std::size_t { return count; }
	auto id(std::size_t position) const -> mtrc::RecordId { return mtrc::RecordId::from_index(position); }
	auto position_of(mtrc::RecordId id) const -> std::size_t { return id.index(); }
	auto contains(mtrc::RecordId id) const -> bool { return id.index() < count; }
	auto version() const -> std::size_t { return 13; }
	auto is_stale() const -> bool { return false; }
};

namespace mtrc::core {
template <> struct metric_traits<AbsoluteDistance> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};

template <> struct metric_traits<ScaledAbsoluteDistance> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;

	static auto cache_key(const ScaledAbsoluteDistance &metric) -> std::string
	{
		return "scaled_absolute:scale=" + std::to_string(metric.scale);
	}
};

template <> struct metric_traits<CountingAbsoluteDistance> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};

template <> struct metric_traits<DirectedDistance> {
	static constexpr auto law = metric_law::distance;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};

template <> struct metric_traits<ZeroPseudoMetric> {
	static constexpr auto law = metric_law::pseudo_metric;
	static constexpr auto records = record_kind::custom;
	static constexpr bool thread_safe = true;
};
} // namespace mtrc::core

int main()
{
	auto space = mtrc::make_space(std::vector<int>{0, 2, 5, 9}, AbsoluteDistance{});
	static_assert(mtrc::MetricSpaceLike_v<decltype(space)>);

	const auto id0 = space.id(0);
	const auto id1 = space.id(1);
	const auto id2 = space.id(2);
	const auto id3 = space.id(3);
	const std::vector<mtrc::RecordId> initial_ids{id0, id1, id2, id3};
	const std::vector<mtrc::RecordId> sparse_ids{id0, id2};
	assert(mtrc::space::storage::representation_cache_key("demo_index", "metric:v1", 42, sparse_ids) ==
		   "demo_index|metric=metric:v1|version=42|ids=0,2");
	assert(mtrc::space::storage::representation_cache_key("demo_index", "metric:v1", 42, sparse_ids,
															 {{"k", "3"}, {"mode", "lazy"}}) ==
		   "demo_index|metric=metric:v1|version=42|k=3|mode=lazy|ids=0,2");

	auto implicit = mtrc::space::storage::implicit(space);
	static_assert(mtrc::PairwiseDistances_v<decltype(implicit)>);
	assert(implicit.record_count() == space.size());
	assert(implicit.id(0) == id0);
	assert(implicit.contains(id2));
	assert(implicit.position_of(id2) == 2);
	assert(implicit.distance(id0, id2) == space.distance(id0, id2));
	const auto implicit_diagnostics = implicit.diagnostics();
	assert(implicit_diagnostics.kind == mtrc::space::storage::representation_kind::live_distances);
	assert(implicit_diagnostics.exact == mtrc::space::storage::exactness::exact);
	assert(implicit_diagnostics.materialized == mtrc::space::storage::materialization::lazy);
	assert(implicit_diagnostics.updates == mtrc::space::storage::update_mode::live);
	assert(!implicit_diagnostics.stale);
	auto strategy_implicit = mtrc::space::storage::make(space, mtrc::stats::search::brute_force{});
	static_assert(mtrc::PairwiseDistances_v<decltype(strategy_implicit)>);
	assert(strategy_implicit.distance(id0, id2) == implicit.distance(id0, id2));
	auto exact_scan = mtrc::space::storage::exact_scan(space);
	static_assert(mtrc::NeighborSearchIndex_v<decltype(exact_scan)>);
	assert(exact_scan.id(2) == id2);
	assert(exact_scan.contains(id3));
	const auto exact_scan_neighbors = exact_scan.knn(4, 2);
	assert(exact_scan_neighbors.size() == 2);
	assert(exact_scan_neighbors[0].id == id2);
	assert(exact_scan_neighbors[0].distance == 1);
	assert(exact_scan_neighbors[1].id == id1);
	assert(exact_scan_neighbors[1].distance == 2);
	const auto exact_scan_diagnostics = exact_scan.diagnostics();
	assert(exact_scan_diagnostics.kind == mtrc::space::storage::representation_kind::live_distances);
	assert(exact_scan_diagnostics.materialized == mtrc::space::storage::materialization::materialized);
	assert(exact_scan_diagnostics.updates == mtrc::space::storage::update_mode::snapshot);
	assert(exact_scan_diagnostics.records == space.size());
	assert(exact_scan_diagnostics.built_for_version == space.version());
	assert(exact_scan_diagnostics.metric_key == mtrc::metric_cache_key(space.metric()));
	assert(exact_scan_diagnostics.cache_key == exact_scan.cache_key());
	assert(exact_scan_diagnostics.cache_key.find("exact_scan_index") != std::string::npos);
	assert(exact_scan_diagnostics.source_record_ids == initial_ids);
	assert(!exact_scan.is_stale());
	auto strategy_exact_scan = mtrc::space::storage::make(space, mtrc::stats::search::exact_scan{});
	static_assert(mtrc::NeighborSearchIndex_v<decltype(strategy_exact_scan)>);
	assert(strategy_exact_scan.knn(4, 2)[0].id == exact_scan_neighbors[0].id);
	const auto implicit_distances = mtrc::space::storage::provider_dense_distance_matrix(implicit);
	assert(implicit_distances.rows() == space.size());
	assert(implicit_distances.columns() == space.size());
	assert(implicit_distances(0, 0) == 0);
	assert(implicit_distances(0, 2) == 5);
	assert(implicit_distances(2, 0) == 5);
	const auto space_distances = mtrc::space::storage::metric_space_dense_distance_matrix<double>(space);
	assert(space_distances(1, 3) == 7.0);
	const auto implicit_symmetric_distances =
		mtrc::space::storage::provider_symmetric_distance_matrix<double>(implicit);
	assert(implicit_symmetric_distances(0, 2) == 5.0);
	assert(implicit_symmetric_distances(2, 0) == 5.0);

	const DirectedProvider directed_provider;
	static_assert(mtrc::PairwiseDistances_v<DirectedProvider>);
	const auto directed_distances = mtrc::space::storage::provider_dense_distance_matrix(directed_provider);
	assert(directed_distances.rows() == directed_provider.record_count());
	assert(directed_distances.columns() == directed_provider.record_count());
	assert(directed_distances(0, 1) == 1);
	assert(directed_distances(1, 0) == 10);
	assert(directed_distances(2, 1) == 21);

	int dense_id_distance_calls = 0;
	int dense_position_distance_calls = 0;
	const PositionCountingProvider dense_counting_provider{
		{mtrc::RecordId::from_index(10), mtrc::RecordId::from_index(20), mtrc::RecordId::from_index(30)},
		&dense_id_distance_calls, &dense_position_distance_calls};
	const auto dense_counted_distances =
		mtrc::space::storage::provider_dense_distance_matrix(dense_counting_provider);
	assert(dense_counted_distances(0, 2) == 2);
	assert(dense_counted_distances(2, 0) == 2);
	assert(dense_id_distance_calls == 0);
	assert(dense_position_distance_calls == 9);

	int symmetric_id_distance_calls = 0;
	int symmetric_position_distance_calls = 0;
	const PositionCountingProvider symmetric_counting_provider{
		{mtrc::RecordId::from_index(10), mtrc::RecordId::from_index(20), mtrc::RecordId::from_index(30)},
		&symmetric_id_distance_calls, &symmetric_position_distance_calls};
	const auto symmetric_counted_distances =
		mtrc::space::storage::provider_symmetric_distance_matrix(symmetric_counting_provider);
	assert(symmetric_counted_distances(0, 2) == 2);
	assert(symmetric_counted_distances(2, 0) == 2);
	assert(symmetric_id_distance_calls == 0);
	assert(symmetric_position_distance_calls == 3);

	int guarded_dense_distance_calls = 0;
	const LargeCountingProvider large_dense_provider{
		mtrc::space::storage::default_dense_distance_matrix_max_records + 1, &guarded_dense_distance_calls};
	bool rejected_large_dense_matrix = false;
	try {
		(void)mtrc::space::storage::provider_dense_distance_matrix(large_dense_provider);
	} catch (const mtrc::RepresentationError &) {
		rejected_large_dense_matrix = true;
	}
	assert(rejected_large_dense_matrix);
	assert(guarded_dense_distance_calls == 0);

	bool rejected_custom_symmetric_budget = false;
	try {
		(void)mtrc::space::storage::provider_symmetric_distance_matrix(
			symmetric_counting_provider, mtrc::space::storage::dense_distance_matrix_options{8});
	} catch (const mtrc::RepresentationError &) {
		rejected_custom_symmetric_budget = true;
	}
	assert(rejected_custom_symmetric_budget);
	assert(symmetric_position_distance_calls == 3);
	const auto unbounded_symmetric_distances = mtrc::space::storage::provider_symmetric_distance_matrix(
		symmetric_counting_provider, mtrc::space::storage::dense_distance_matrix_options{0});
	assert(unbounded_symmetric_distances(0, 2) == 2);
	assert(symmetric_position_distance_calls == 6);

	auto matrix = mtrc::space::storage::matrix(space);
	static_assert(mtrc::PairwiseDistances_v<decltype(matrix)>);
	assert(matrix.record_count() == space.size());
	assert(matrix.id(2) == id2);
	assert(matrix.contains(id1));
	assert(matrix.distance(id0, id2) == 5);
	assert(matrix.distance(id1, id3) == space.distance(id1, id3));
	const auto matrix_diagnostics = matrix.diagnostics();
	assert(matrix_diagnostics.kind == mtrc::space::storage::representation_kind::distance_table);
	assert(matrix_diagnostics.cached_distances == space.size() * space.size());
	assert(matrix_diagnostics.distance_evaluations == space.size() * space.size());
	assert(matrix_diagnostics.dense_distance_slots == space.size() * space.size());
	assert(matrix_diagnostics.max_dense_records == 0);
	assert(matrix_diagnostics.max_memory_bytes == 0);
	assert(matrix_diagnostics.memory_bytes_estimate ==
		   mtrc::space::storage::estimate_distance_table_memory_bytes<int>(space.size()));
	assert(matrix_diagnostics.built_for_version == space.version());
	assert(matrix_diagnostics.metric_key == mtrc::metric_cache_key(space.metric()));
	assert(!matrix_diagnostics.cache_key.empty());
	assert(matrix_diagnostics.cache_key == matrix.cache_key());
	assert(matrix_diagnostics.source_record_ids == initial_ids);
	assert(!matrix_diagnostics.stale);
	assert(matrix.stats().fill_ratio == 1.0);
	assert(matrix.stats().hits == 2);
	assert(matrix.dense_distance_slots() == space.size() * space.size());
	assert(matrix.source_record_ids() == initial_ids);

	int symmetric_metric_calls = 0;
	auto counted_space =
		mtrc::make_space(std::vector<int>{0, 2, 5, 9}, CountingAbsoluteDistance{&symmetric_metric_calls});
	const auto counted_id0 = counted_space.id(0);
	const auto counted_id1 = counted_space.id(1);
	const auto counted_id2 = counted_space.id(2);
	const auto counted_id3 = counted_space.id(3);
	auto symmetric_table = mtrc::space::storage::symmetric_distance_table(counted_space);
	static_assert(mtrc::PairwiseDistances_v<decltype(symmetric_table)>);
	assert(symmetric_table.record_count() == counted_space.size());
	assert(symmetric_table.off_diagonal_slots() == counted_space.size() * (counted_space.size() - 1) / 2);
	assert(symmetric_table.dense_distance_slots() == symmetric_table.off_diagonal_slots());
	assert(symmetric_table.cached_distances() == symmetric_table.off_diagonal_slots());
	assert(symmetric_metric_calls == static_cast<int>(symmetric_table.off_diagonal_slots()));
	assert(symmetric_table.distance(counted_id0, counted_id0) == 0);
	assert(symmetric_table.distance_at_position(3, 3) == 0);
	assert(symmetric_metric_calls == static_cast<int>(symmetric_table.off_diagonal_slots()));
	assert(symmetric_table.distance(counted_id0, counted_id3) == 9);
	assert(symmetric_table.distance(counted_id3, counted_id0) == 9);
	assert(symmetric_table.distance_at_position(1, 3) == 7);
	assert(symmetric_table.distance_at_position(3, 1) == 7);
	assert(symmetric_table.contains(counted_id2));
	assert(symmetric_table.position_of(counted_id3) == 3);
	assert(symmetric_table.memory_bytes_estimate() ==
		   mtrc::space::storage::estimate_triangular_distance_table_memory_bytes<int>(counted_space.size()));
	assert(mtrc::space::storage::triangular_distance_slot_count(counted_space.size()) ==
		   symmetric_table.off_diagonal_slots());
	const auto symmetric_diagnostics = symmetric_table.diagnostics();
	assert(symmetric_diagnostics.kind == mtrc::space::storage::representation_kind::distance_table);
	assert(symmetric_diagnostics.exact == mtrc::space::storage::exactness::exact);
	assert(symmetric_diagnostics.materialized == mtrc::space::storage::materialization::materialized);
	assert(symmetric_diagnostics.updates == mtrc::space::storage::update_mode::snapshot);
	assert(symmetric_diagnostics.cached_distances == symmetric_table.off_diagonal_slots());
	assert(symmetric_diagnostics.distance_evaluations == symmetric_table.off_diagonal_slots());
	assert(symmetric_diagnostics.dense_distance_slots == symmetric_table.off_diagonal_slots());
	assert(symmetric_diagnostics.source_record_ids == symmetric_table.source_record_ids());
	assert(!symmetric_diagnostics.stale);
	assert(symmetric_table.cache_key().find("symmetric_distance_table") != std::string::npos);
	counted_space.insert(12);
	assert(symmetric_table.is_stale());
	assert(symmetric_table.diagnostics().stale);
	assert(!symmetric_table.diagnostics().warnings.empty());

	auto pseudo_space = mtrc::make_space(std::vector<int>{1, 1, 2}, ZeroPseudoMetric{});
	auto pseudo_symmetric_table = mtrc::space::storage::symmetric_distance_table(pseudo_space);
	assert(pseudo_symmetric_table.record_count() == pseudo_space.size());
	assert(pseudo_symmetric_table.distance(pseudo_space.id(0), pseudo_space.id(2)) == 0);

	auto directed_space = mtrc::make_space(std::vector<int>{0, 2, 5}, DirectedDistance{});
	bool rejected_directed_symmetric_table = false;
	try {
		(void)mtrc::space::storage::symmetric_distance_table(directed_space);
	} catch (const mtrc::RepresentationError &) {
		rejected_directed_symmetric_table = true;
	}
	assert(rejected_directed_symmetric_table);

	auto strategy_matrix = mtrc::space::storage::make(space, mtrc::stats::search::distance_table{});
	static_assert(mtrc::PairwiseDistances_v<decltype(strategy_matrix)>);
	assert(strategy_matrix.distance(id0, id2) == matrix.distance(id0, id2));
	auto strategy_entry_matrix = mtrc::space::storage::make_representation(space, mtrc::stats::search::distance_table{});
	static_assert(mtrc::PairwiseDistances_v<decltype(strategy_entry_matrix)>);
	assert(strategy_entry_matrix.distance(id0, id2) == matrix.distance(id0, id2));

	mtrc::space::storage::distance_table_options limited_matrix_options;
	limited_matrix_options.mode = mtrc::space::storage::distance_table_mode::eager;
	limited_matrix_options.max_dense_records = space.size();
	limited_matrix_options.max_memory_bytes =
		mtrc::space::storage::estimate_distance_table_memory_bytes<int>(space.size());
	auto limited_matrix = mtrc::space::storage::matrix(space, limited_matrix_options);
	assert(limited_matrix.max_dense_records() == space.size());
	assert(limited_matrix.max_memory_bytes() == limited_matrix_options.max_memory_bytes);
	assert(limited_matrix.memory_bytes_estimate() == limited_matrix_options.max_memory_bytes);
	assert(limited_matrix.diagnostics().max_dense_records == space.size());
	assert(limited_matrix.diagnostics().max_memory_bytes == limited_matrix_options.max_memory_bytes);
	assert(limited_matrix.diagnostics().dense_distance_slots == space.size() * space.size());
	auto rebuilt_limited_matrix = mtrc::space::cache::rebuild(limited_matrix, space);
	assert(rebuilt_limited_matrix.max_dense_records() == limited_matrix_options.max_dense_records);
	assert(rebuilt_limited_matrix.max_memory_bytes() == limited_matrix_options.max_memory_bytes);
	assert(rebuilt_limited_matrix.diagnostics().max_memory_bytes == limited_matrix_options.max_memory_bytes);
	bool rejected_dense_limit = false;
	limited_matrix_options.max_dense_records = space.size() - 1;
	try {
		(void)mtrc::space::storage::matrix(space, limited_matrix_options);
	} catch (const mtrc::RepresentationError &) {
		rejected_dense_limit = true;
	}
	assert(rejected_dense_limit);
	limited_matrix_options.max_dense_records = space.size();
	limited_matrix_options.max_memory_bytes = limited_matrix.memory_bytes_estimate() - 1;
	bool rejected_memory_limit = false;
	try {
		(void)mtrc::space::storage::matrix(space, limited_matrix_options);
	} catch (const mtrc::RepresentationError &) {
		rejected_memory_limit = true;
	}
	assert(rejected_memory_limit);

		auto lazy_matrix = mtrc::space::storage::matrix(space, mtrc::space::storage::distance_table_mode::lazy);
		assert(lazy_matrix.cached_distances() == 0);
		const auto lazy_initial_diagnostics = lazy_matrix.diagnostics();
		assert(lazy_initial_diagnostics.materialized == mtrc::space::storage::materialization::lazy);
		assert(lazy_initial_diagnostics.cached_distances == 0);
		assert(lazy_initial_diagnostics.dense_distance_slots == space.size() * space.size());
		assert(lazy_initial_diagnostics.memory_bytes_estimate == lazy_matrix.memory_bytes_estimate());
		assert(lazy_initial_diagnostics.memory_bytes_estimate < matrix_diagnostics.memory_bytes_estimate);
		assert(lazy_initial_diagnostics.cache_key == lazy_matrix.cache_key());
		mtrc::space::storage::distance_table_options limited_lazy_options;
		limited_lazy_options.mode = mtrc::space::storage::distance_table_mode::lazy;
		limited_lazy_options.max_dense_records = space.size() - 1;
		auto limited_lazy_matrix = mtrc::space::storage::matrix(space, limited_lazy_options);
		assert(limited_lazy_matrix.max_dense_records() == space.size() - 1);
		assert(limited_lazy_matrix.cached_distances() == 0);
		assert(limited_lazy_matrix.memory_bytes_estimate() < matrix_diagnostics.memory_bytes_estimate);
		assert(lazy_initial_diagnostics.memory_bytes_estimate > 0);
		limited_lazy_options.max_memory_bytes = lazy_initial_diagnostics.memory_bytes_estimate - 1;
		bool rejected_lazy_memory_limit = false;
		try {
			(void)mtrc::space::storage::matrix(space, limited_lazy_options);
		} catch (const mtrc::RepresentationError &) {
			rejected_lazy_memory_limit = true;
		}
		assert(rejected_lazy_memory_limit);
		mtrc::space::storage::distance_table_options growth_limited_lazy_options;
		growth_limited_lazy_options.mode = mtrc::space::storage::distance_table_mode::lazy;
		growth_limited_lazy_options.max_memory_bytes = lazy_initial_diagnostics.memory_bytes_estimate;
		auto growth_limited_lazy = mtrc::space::storage::matrix(space, growth_limited_lazy_options);
		bool rejected_lazy_cache_growth = false;
		try {
			(void)growth_limited_lazy.distance(id0, id2);
		} catch (const mtrc::RepresentationError &) {
			rejected_lazy_cache_growth = true;
		}
		assert(rejected_lazy_cache_growth);
		assert(growth_limited_lazy.cached_distances() == 0);
		assert(growth_limited_lazy.stats().misses == 0);
		assert(lazy_matrix.distance(id0, id2) == 5);
		assert(lazy_matrix.cached_distances() == 1);
	assert(lazy_matrix.distance(id0, id2) == 5);
	assert(lazy_matrix.distance(id1, id3) == space.distance(id1, id3));
	const auto lazy_stats = lazy_matrix.stats();
	assert(lazy_stats.hits == 1);
	assert(lazy_stats.misses == 2);
	assert(lazy_stats.fill_ratio == 2.0 / static_cast<double>(space.size() * space.size()));
	const auto lazy_diagnostics = lazy_matrix.diagnostics();
	assert(lazy_diagnostics.cached_distances == 2);
	assert(lazy_diagnostics.distance_evaluations == 2);

	auto tree = mtrc::space::storage::cover_tree(space);
	static_assert(mtrc::NeighborSearchIndex_v<decltype(tree)>);
	assert(tree.id(3) == id3);
	assert(tree.contains(id0));
	const auto tree_neighbors = tree.knn(4, 2);
	assert(tree_neighbors.size() == 2);
	assert(tree_neighbors[0].id == id2);
	assert(tree_neighbors[0].distance == 1);
	assert(tree_neighbors[1].id == id1);
	assert(tree_neighbors[1].distance == 2);
	const auto tree_diagnostics = tree.diagnostics();
	assert(tree_diagnostics.kind == mtrc::space::storage::representation_kind::cover_tree_index);
	assert(tree_diagnostics.records == space.size());
	assert(tree_diagnostics.built_for_version == space.version());
	assert(tree_diagnostics.metric_key == mtrc::metric_cache_key(space.metric()));
	assert(tree_diagnostics.cache_key == tree.cache_key());
	assert(tree_diagnostics.source_record_ids == initial_ids);
	assert(tree.source_record_ids() == initial_ids);
	assert(!tree.cache_key().empty());
	assert(tree.stats().nodes == space.size());
	auto strategy_tree = mtrc::space::storage::make(space, mtrc::stats::search::cover_tree{});
	static_assert(mtrc::NeighborSearchIndex_v<decltype(strategy_tree)>);
	assert(strategy_tree.knn(4, 2)[0].id == tree_neighbors[0].id);

	auto graph = mtrc::space::storage::knn_graph(space, 1);
	static_assert(mtrc::NeighborSearchIndex_v<decltype(graph)>);
	assert(graph.k() == 1);
	assert(graph.id(0) == id0);
	assert(graph.contains(id3));
	assert(graph.neighbors(id0).size() == 1);
	assert(graph.neighbors(id0)[0].id == id1);
	assert(graph.neighbors(id3).size() == 1);
	assert(graph.neighbors(id3)[0].id == id2);
	const auto graph_diagnostics = graph.diagnostics();
	assert(graph_diagnostics.kind == mtrc::space::storage::representation_kind::knn_graph_index);
	assert(graph_diagnostics.exact == mtrc::space::storage::exactness::approximate);
	assert(graph_diagnostics.built_for_version == space.version());
	assert(graph_diagnostics.metric_key == mtrc::metric_cache_key(space.metric()));
	assert(graph_diagnostics.cache_key == graph.cache_key());
	assert(graph_diagnostics.source_record_ids == initial_ids);
	assert(graph.source_record_ids() == initial_ids);
	assert(!graph.cache_key().empty());
	assert(graph.cache_key().find("k=1") != std::string::npos);
	assert(graph.stats().edges == space.size());
	const auto graph_recall_stats = graph.stats_against(matrix);
	assert(graph_recall_stats.recall_validated);
	assert(graph_recall_stats.sampled_recall == 1.0);
	assert(graph.sampled_recall(matrix, 2) == 1.0);
	const auto graph_quality = graph.quality_against(matrix);
	assert(graph_quality.diagnostic == "knn_graph_quality");
	assert(graph_quality.representation == "knn_graph_index");
	assert(graph_quality.reference_representation == "distance_table");
	assert(graph_quality.record_count == space.size());
	assert(graph_quality.edge_count == space.size());
	assert(graph_quality.requested_neighbors == 1);
	assert(graph_quality.evaluated_queries == space.size());
	assert(graph_quality.evaluated_neighbor_count == 1);
	assert(graph_quality.matched_neighbors == space.size());
	assert(graph_quality.possible_neighbors == space.size());
	assert(graph_quality.exact_distance_evaluations == space.size() * (space.size() - 1));
	assert(graph_quality.graph_edge_evaluations == space.size());
	assert(graph_quality.recall == 1.0);
	assert(graph_quality.average_row_recall == 1.0);
	assert(graph_quality.minimum_row_recall == 1.0);
	assert(graph_quality.maximum_row_recall == 1.0);
	assert(graph_quality.average_best_distance_inflation == 1.0);
	assert(graph_quality.maximum_best_distance_inflation == 1.0);
	assert(graph_quality.exact_reference);
	assert(graph_quality.approximate_candidate);
	assert(!graph_quality.stale);
	const auto sampled_graph_quality = graph.quality_against(matrix, 2);
	assert(sampled_graph_quality.evaluated_queries == 2);
	assert(sampled_graph_quality.exact_distance_evaluations == 2 * (space.size() - 1));
	assert(sampled_graph_quality.graph_edge_evaluations == 2);
	const auto graph_alias = mtrc::space::storage::graph(space, 1);
	assert(graph_alias.k() == graph.k());
	assert(graph_alias.neighbors(id0)[0].id == graph.neighbors(id0)[0].id);
	auto strategy_graph = mtrc::space::storage::make(space, mtrc::stats::search::knn_graph(1));
	static_assert(mtrc::NeighborSearchIndex_v<decltype(strategy_graph)>);
	assert(strategy_graph.k() == graph.k());
	assert(strategy_graph.neighbors(id0)[0].id == graph.neighbors(id0)[0].id);
	bool rejected_missing_graph_neighbors = false;
	try {
		(void)mtrc::space::storage::make(space, mtrc::stats::search::knn_graph{});
	} catch (const std::invalid_argument &) {
		rejected_missing_graph_neighbors = true;
	}
	assert(rejected_missing_graph_neighbors);

	auto topology = mtrc::space::storage::topology(space);
	static_assert(mtrc::GraphTopology_v<decltype(topology)>);
	assert(topology.id(1) == id1);
	assert(topology.contains(id2));
	topology.add_edge(id0, id1, matrix.distance(id0, id1));
	assert(topology.record_count() == space.size());
	assert(topology.edge_count() == 1);
	assert(topology.edges()[0].source == id0);
	assert(topology.edges()[0].target == id1);
	assert(topology.edges()[0].distance == 2);
	const auto topology_diagnostics = topology.diagnostics();
	assert(topology_diagnostics.kind == mtrc::space::storage::representation_kind::graph_topology);
	assert(topology_diagnostics.materialized == mtrc::space::storage::materialization::topology);

	assert(!implicit.is_stale());
	assert(!exact_scan.is_stale());
	assert(!matrix.is_stale());
	assert(!lazy_matrix.is_stale());
	assert(!tree.is_stale());
	assert(!graph.is_stale());
	assert(!topology.is_stale());

	const auto inserted_id = space.insert(12);
	assert(!implicit.is_stale());
	assert(implicit.contains(inserted_id));
	assert(implicit.position_of(inserted_id) == 4);
	assert(exact_scan.is_stale());
	assert(matrix.is_stale());
	assert(lazy_matrix.is_stale());
	assert(tree.is_stale());
	assert(graph.is_stale());
	assert(topology.is_stale());
	assert(exact_scan.diagnostics().stale);
	assert(matrix.diagnostics().stale);
	assert(lazy_matrix.diagnostics().stale);
	assert(tree.diagnostics().stale);
	assert(graph.diagnostics().stale);
	assert(!exact_scan.diagnostics().warnings.empty());
	assert(!matrix.diagnostics().warnings.empty());
	assert(!lazy_matrix.diagnostics().warnings.empty());
	assert(!tree.diagnostics().warnings.empty());
	assert(!graph.diagnostics().warnings.empty());

	assert(matrix.distance(id1, id3) == 7);
	assert(space.erase(id1));
	assert(!space.contains(id1));
	assert(space.position_of(id2) == 1);
	assert(matrix.contains(id1));
	assert(matrix.position_of(id3) == 3);
	assert(matrix.distance(id1, id3) == 7);

	auto refreshed_matrix = mtrc::space::storage::matrix(space);
	assert(refreshed_matrix.record_count() == space.size());
	assert(!refreshed_matrix.contains(id1));
	assert(refreshed_matrix.id(1) == id2);
	assert(refreshed_matrix.contains(inserted_id));
	assert(refreshed_matrix.distance(id0, id2) == space.distance(id0, id2));
	assert(refreshed_matrix.cache_key() != matrix.cache_key());
	const std::vector<mtrc::RecordId> refreshed_ids{id0, id2, id3, inserted_id};
	assert(refreshed_matrix.diagnostics().source_record_ids == refreshed_ids);
	auto refreshed_exact_scan = mtrc::space::storage::exact_scan(space);
	assert(!refreshed_exact_scan.is_stale());
	assert(!refreshed_exact_scan.contains(id1));
	assert(refreshed_exact_scan.source_record_ids() == refreshed_ids);
	assert(refreshed_exact_scan.cache_key() != exact_scan.cache_key());
	auto refreshed_tree = mtrc::space::storage::cover_tree(space);
	assert(!refreshed_tree.is_stale());
	assert(!refreshed_tree.contains(id1));
	assert(refreshed_tree.source_record_ids() == refreshed_ids);
	assert(refreshed_tree.cache_key() != tree.cache_key());

	auto scaled_space = mtrc::make_space(std::vector<int>{0, 2, 5}, ScaledAbsoluteDistance{3});
	auto scaled_matrix = mtrc::space::storage::matrix(scaled_space);
	assert(scaled_matrix.metric_key() == "scaled_absolute:scale=3");
	assert(scaled_matrix.cache_key().find("scaled_absolute:scale=3") != std::string::npos);
	assert(scaled_matrix.cache_key().find("version=0") != std::string::npos);
	assert(scaled_matrix.cache_key().find("ids=0,1,2") != std::string::npos);
	assert(scaled_matrix.diagnostics().metric_key == scaled_matrix.metric_key());
	auto scaled_exact_scan = mtrc::space::storage::exact_scan(scaled_space);
	assert(scaled_exact_scan.metric_key() == "scaled_absolute:scale=3");
	assert(scaled_exact_scan.cache_key().find("exact_scan_index") != std::string::npos);
	assert(scaled_exact_scan.cache_key().find("scaled_absolute:scale=3") != std::string::npos);
	assert(scaled_exact_scan.cache_key().find("version=0") != std::string::npos);
	assert(scaled_exact_scan.cache_key().find("ids=0,1,2") != std::string::npos);
	auto scaled_tree = mtrc::space::storage::cover_tree(scaled_space);
	assert(scaled_tree.metric_key() == "scaled_absolute:scale=3");
	assert(scaled_tree.cache_key().find("cover_tree_index") != std::string::npos);
	assert(scaled_tree.cache_key().find("scaled_absolute:scale=3") != std::string::npos);
	assert(scaled_tree.cache_key().find("version=0") != std::string::npos);
	assert(scaled_tree.cache_key().find("ids=0,1,2") != std::string::npos);
	assert(scaled_tree.diagnostics().source_record_ids == scaled_tree.source_record_ids());
	auto scaled_graph = mtrc::space::storage::knn_graph(scaled_space, 2);
	assert(scaled_graph.metric_key() == "scaled_absolute:scale=3");
	assert(scaled_graph.cache_key().find("knn_graph_index") != std::string::npos);
	assert(scaled_graph.cache_key().find("scaled_absolute:scale=3") != std::string::npos);
	assert(scaled_graph.cache_key().find("version=0") != std::string::npos);
	assert(scaled_graph.cache_key().find("k=2") != std::string::npos);
	assert(scaled_graph.cache_key().find("ids=0,1,2") != std::string::npos);
	assert(scaled_graph.diagnostics().source_record_ids == scaled_graph.source_record_ids());

	return 0;
}
