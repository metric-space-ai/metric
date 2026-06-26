// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MAPPINGS_CLUSTERED_SPACE_HPP
#define _METRIC_MAPPINGS_CLUSTERED_SPACE_HPP

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/record/id.hpp>
#include <metric/core/result.hpp>
#include <metric/space/storage/implicit.hpp>

namespace mtrc::modify::map {

inline constexpr std::size_t default_clustered_space_max_representatives = 4096;
inline constexpr std::size_t default_clustered_space_max_memory_bytes = 512ULL * 1024ULL * 1024ULL;
inline constexpr std::size_t default_clustered_space_max_distance_evaluations = 100'000'000;

struct ClusteredSpaceOptions {
	// Set any budget to 0 only when the caller intentionally opts into an unbounded representative table.
	std::size_t max_representatives{default_clustered_space_max_representatives};
	std::size_t max_memory_bytes{default_clustered_space_max_memory_bytes};
	std::size_t max_distance_evaluations{default_clustered_space_max_distance_evaluations};
};

namespace clustered_space_detail {

inline auto checked_product(std::size_t lhs, std::size_t rhs, const char *message) -> std::size_t
{
	if (rhs != 0 && lhs > std::numeric_limits<std::size_t>::max() / rhs) {
		throw RepresentationError(message);
	}
	return lhs * rhs;
}

inline auto checked_sum(std::size_t lhs, std::size_t rhs, const char *message) -> std::size_t
{
	if (lhs > std::numeric_limits<std::size_t>::max() - rhs) {
		throw RepresentationError(message);
	}
	return lhs + rhs;
}

template <typename Distance>
auto representative_distance_table_bytes(std::size_t representative_count) -> std::size_t
{
	const auto cells = checked_product(
		representative_count, representative_count,
		"clustered_space representative table cell estimate exceeds size_t capacity");
	const auto distance_bytes = checked_product(
		cells, sizeof(Distance), "clustered_space representative table memory estimate exceeds size_t capacity");
	const auto row_bytes = checked_product(
		representative_count, sizeof(std::vector<Distance>),
		"clustered_space representative table memory estimate exceeds size_t capacity");
	return checked_sum(distance_bytes, row_bytes,
					   "clustered_space representative table memory estimate exceeds size_t capacity");
}

template <typename Distance>
auto require_clustered_space_budget(std::size_t representative_count, ClusteredSpaceOptions options) -> void
{
	const auto distance_evaluations = checked_product(
		representative_count, representative_count,
		"clustered_space representative distance-evaluation estimate exceeds size_t capacity");
	const auto estimated_bytes = representative_distance_table_bytes<Distance>(representative_count);

	if (options.max_representatives > 0 && representative_count > options.max_representatives) {
		throw RepresentationError(
			"clustered_space refused representative distance table before metric calls: representatives=" +
			std::to_string(representative_count) + " max_representatives=" +
			std::to_string(options.max_representatives) +
			" estimated_distance_evaluations=" + std::to_string(distance_evaluations) +
			" estimated_bytes=" + std::to_string(estimated_bytes) +
			". Pass ClusteredSpaceOptions with explicit unbounded budgets only when exact clustered-space "
			"materialization is intentional.");
	}
	if (options.max_memory_bytes > 0 && estimated_bytes > options.max_memory_bytes) {
		throw RepresentationError(
			"clustered_space refused representative distance table before allocation: representatives=" +
			std::to_string(representative_count) + " estimated_bytes=" + std::to_string(estimated_bytes) +
			" max_memory_bytes=" + std::to_string(options.max_memory_bytes) +
			" estimated_distance_evaluations=" + std::to_string(distance_evaluations) +
			". Pass ClusteredSpaceOptions with explicit unbounded budgets only when exact clustered-space "
			"materialization is intentional.");
	}
	if (options.max_distance_evaluations > 0 && distance_evaluations > options.max_distance_evaluations) {
		throw RepresentationError(
			"clustered_space refused representative distance table before metric calls: representatives=" +
			std::to_string(representative_count) +
			" estimated_distance_evaluations=" + std::to_string(distance_evaluations) +
			" max_distance_evaluations=" + std::to_string(options.max_distance_evaluations) +
			" estimated_bytes=" + std::to_string(estimated_bytes) +
			". Pass ClusteredSpaceOptions with explicit unbounded budgets only when exact clustered-space "
			"materialization is intentional.");
	}
}

} // namespace clustered_space_detail

struct ClusterRecord {
	std::size_t label{};
	RecordId representative;
	std::vector<RecordId> members;
	bool unassigned{false};
};

template <typename Distance> class ClusterRepresentativeMetric {
  public:
	using distance_type = Distance;

	ClusterRepresentativeMetric() = default;

	explicit ClusterRepresentativeMetric(std::vector<std::vector<distance_type>> distances)
		: distances_(std::move(distances))
	{
	}

	auto operator()(const ClusterRecord &lhs, const ClusterRecord &rhs) const -> distance_type
	{
		return distances_.at(lhs.label).at(rhs.label);
	}

	auto distances() const -> const std::vector<std::vector<distance_type>> & { return distances_; }

  private:
	std::vector<std::vector<distance_type>> distances_;
};

template <typename Distance> class ClusteredSpaceDerivation {
  public:
	using distance_type = Distance;
	using metric_type = ClusterRepresentativeMetric<distance_type>;
	using space_type = MetricSpace<ClusterRecord, metric_type>;
	using result_type = MappingResult<space_type>;

	ClusteredSpaceDerivation(std::vector<ClusterRecord> records, std::vector<std::vector<distance_type>> distances,
							 std::vector<std::vector<RecordId>> source_records,
							 std::vector<RecordId> representative_records, std::size_t source_record_count,
							 std::string strategy, std::string representation,
							 core::metric_law source_metric_law = core::metric_law::unknown)
		: records_(std::move(records)), distances_(std::move(distances)), source_records_(std::move(source_records)),
		  representative_records_(std::move(representative_records)), source_record_count_(source_record_count),
		  strategy_(std::move(strategy)), representation_(std::move(representation)),
		  source_metric_law_(source_metric_law)
	{
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto transform(const Space &space) const -> result_type
	{
		if (space.size() != source_record_count_) {
			throw std::invalid_argument("source space size does not match clustered-space derivation");
		}
		return transform();
	}

	auto transform() const -> result_type
	{
		space_type derived_space(records_, metric_type(distances_));
		// The derived space's pairwise distances are the source metric evaluated on
		// the cluster representatives, so a true/pseudo source metric restricts to
		// the same metric law over the representatives.
		return core::make_mapping_result(
			std::move(derived_space), source_records_, representative_records_, source_record_count_, false,
			"clustered_space", strategy_, representation_, source_metric_law_, false,
			"cluster-representative space; pairwise distances are the source metric restricted to cluster "
			"representatives; in-sample only");
	}

	auto inverse_supported() const -> bool { return false; }

  private:
	std::vector<ClusterRecord> records_;
	std::vector<std::vector<distance_type>> distances_;
	std::vector<std::vector<RecordId>> source_records_;
	std::vector<RecordId> representative_records_;
	std::size_t source_record_count_{};
	std::string strategy_;
	std::string representation_;
	core::metric_law source_metric_law_{core::metric_law::unknown};
};

template <typename Distance> class ClusteredSpaceMapping {
  public:
	using distance_type = Distance;
	using clustering_type = ClusteringResult<distance_type>;

	explicit ClusteredSpaceMapping(clustering_type clustering, ClusteredSpaceOptions options = {})
		: clustering_(std::move(clustering)), options_(options)
	{
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto derive_from(const Space &space) const -> ClusteredSpaceDerivation<distance_type>
	{
		space::storage::LiveDistances<Space> provider(space);
		return build_derivation(provider, clustering_, core::metric_traits<typename Space::metric_type>::law, options_);
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto derive_from(const Space &space, ClusteredSpaceOptions options) const -> ClusteredSpaceDerivation<distance_type>
	{
		space::storage::LiveDistances<Space> provider(space);
		return build_derivation(provider, clustering_, core::metric_traits<typename Space::metric_type>::law, options);
	}

	template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
	auto derive_from_provider(const Provider &provider) const -> ClusteredSpaceDerivation<distance_type>
	{
		return build_derivation(provider, clustering_, core::metric_law::unknown, options_);
	}

	template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
	auto derive_from_provider(const Provider &provider, ClusteredSpaceOptions options) const
		-> ClusteredSpaceDerivation<distance_type>
	{
		return build_derivation(provider, clustering_, core::metric_law::unknown, options);
	}

  private:
	template <typename Provider>
	static auto build_derivation(const Provider &provider, const clustering_type &clustering,
								 core::metric_law source_metric_law, ClusteredSpaceOptions options)
		-> ClusteredSpaceDerivation<distance_type>
	{
		::mtrc::require_clustering_result_shape(clustering, provider.record_count(),
												  "clustering record count does not match source provider",
												  "clustering assignments do not match record count");
		if (clustering.cluster_count == 0) {
			throw std::invalid_argument("cannot derive a clustered space without clusters");
		}

		auto source_records =
			::mtrc::cluster_member_record_ids(provider, clustering, "clustering assignment references an unknown cluster");

		std::vector<RecordId> representative_records;
		representative_records.reserve(clustering.cluster_count);
		std::vector<ClusterRecord> records;
		records.reserve(clustering.cluster_count);

		for (std::size_t label = 0; label < clustering.cluster_count; ++label) {
			if (source_records[label].empty()) {
				throw std::invalid_argument("cannot derive a clustered space with empty clusters");
			}
			const auto representative =
				label < clustering.medoids.size() ? clustering.medoids[label] : source_records[label][0];
			representative_records.push_back(representative);
			records.push_back(ClusterRecord{label, representative, source_records[label], false});
		}

		clustered_space_detail::require_clustered_space_budget<distance_type>(representative_records.size(), options);
		auto distances = ::mtrc::distance_table_for_record_ids(provider, representative_records,
																"cluster representative id is outside provider");

		return ClusteredSpaceDerivation<distance_type>(std::move(records), std::move(distances),
													   std::move(source_records), std::move(representative_records),
													   clustering.record_count, clustering.algorithm,
													   clustering.representation, source_metric_law);
	}

	clustering_type clustering_;
	ClusteredSpaceOptions options_;
};

template <typename Distance>
auto make_clustered_space_mapping(ClusteringResult<Distance> clustering) -> ClusteredSpaceMapping<Distance>
{
	return ClusteredSpaceMapping<Distance>(std::move(clustering));
}

template <typename Distance>
auto make_clustered_space_mapping(ClusteringResult<Distance> clustering, ClusteredSpaceOptions options)
	-> ClusteredSpaceMapping<Distance>
{
	return ClusteredSpaceMapping<Distance>(std::move(clustering), options);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto clustered_space(const Space &space, ClusteringResult<typename Space::distance_type> clustering) ->
	typename ClusteredSpaceDerivation<typename Space::distance_type>::result_type
{
	return make_clustered_space_mapping(std::move(clustering)).derive_from(space).transform(space);
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto clustered_space(const Space &space, ClusteringResult<typename Space::distance_type> clustering,
					 ClusteredSpaceOptions options) ->
	typename ClusteredSpaceDerivation<typename Space::distance_type>::result_type
{
	return make_clustered_space_mapping(std::move(clustering), options).derive_from(space).transform(space);
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto clustered_space(const Provider &provider, ClusteringResult<typename Provider::distance_type> clustering) ->
	typename ClusteredSpaceDerivation<typename Provider::distance_type>::result_type
{
	auto mapping = make_clustered_space_mapping(std::move(clustering));
	auto derivation = mapping.derive_from_provider(provider);
	return derivation.transform();
}

template <typename Provider, typename std::enable_if<PairwiseDistances_v<Provider>, int>::type = 0>
auto clustered_space(const Provider &provider, ClusteringResult<typename Provider::distance_type> clustering,
					 ClusteredSpaceOptions options) ->
	typename ClusteredSpaceDerivation<typename Provider::distance_type>::result_type
{
	auto mapping = make_clustered_space_mapping(std::move(clustering), options);
	auto derivation = mapping.derive_from_provider(provider);
	return derivation.transform();
}

} // namespace mtrc::modify::map

#endif
