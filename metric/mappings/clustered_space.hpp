// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MAPPINGS_CLUSTERED_SPACE_HPP
#define _METRIC_MAPPINGS_CLUSTERED_SPACE_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../core/concepts.hpp"
#include "../core/metric_space.hpp"
#include "../core/record_id.hpp"
#include "../core/result.hpp"
#include "../representations/implicit.hpp"

namespace metric::mappings {

struct ClusterRecord {
	std::size_t label{};
	RecordId representative;
	std::vector<RecordId> members;
	bool noise{false};
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

template <typename Distance> class ClusteredSpaceModel {
  public:
	using distance_type = Distance;
	using metric_type = ClusterRepresentativeMetric<distance_type>;
	using space_type = MetricSpace<ClusterRecord, metric_type>;
	using result_type = MappingResult<space_type>;

	ClusteredSpaceModel(std::vector<ClusterRecord> records, std::vector<std::vector<distance_type>> distances,
						std::vector<std::vector<RecordId>> source_records,
						std::vector<RecordId> representative_records, std::size_t source_record_count,
						std::string strategy, std::string representation)
		: records_(std::move(records))
		, distances_(std::move(distances))
		, source_records_(std::move(source_records))
		, representative_records_(std::move(representative_records))
		, source_record_count_(source_record_count)
		, strategy_(std::move(strategy))
		, representation_(std::move(representation))
	{
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto transform(const Space &space) const -> result_type
	{
		if (space.size() != source_record_count_) {
			throw std::invalid_argument("source space size does not match fitted clustered-space mapping");
		}
		return transform();
	}

	auto transform() const -> result_type
	{
		space_type derived_space(records_, metric_type(distances_));
		result_type result{std::move(derived_space), source_records_, representative_records_, source_record_count_,
						   false, "clustered_space", strategy_, representation_};
		return result;
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
};

template <typename Distance> class ClusteredSpaceMapping {
  public:
	using distance_type = Distance;
	using clustering_type = ClusteringResult<distance_type>;

	explicit ClusteredSpaceMapping(clustering_type clustering)
		: clustering_(std::move(clustering))
	{
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto fit(const Space &space) const -> ClusteredSpaceModel<distance_type>
	{
		representations::ImplicitDistanceProvider<Space> provider(space);
		return fit_provider(provider);
	}

	template <typename Provider, typename std::enable_if<DistanceProvider_v<Provider>, int>::type = 0>
	auto fit_provider(const Provider &provider) const -> ClusteredSpaceModel<distance_type>
	{
		return build_model(provider, clustering_);
	}

  private:
	template <typename Provider>
	static auto build_model(const Provider &provider, const clustering_type &clustering)
		-> ClusteredSpaceModel<distance_type>
	{
		const auto noise_label = clustering_type::noise_label;
		if (clustering.record_count != provider.record_count()) {
			throw std::invalid_argument("clustering record count does not match source provider");
		}
		if (clustering.assignments.size() != clustering.record_count) {
			throw std::invalid_argument("clustering assignments do not match record count");
		}
		if (clustering.cluster_count == 0) {
			throw std::invalid_argument("cannot derive a clustered space without clusters");
		}

		std::vector<std::vector<RecordId>> source_records(clustering.cluster_count);
		for (std::size_t index = 0; index < clustering.assignments.size(); ++index) {
			const auto label = clustering.assignments[index];
			if (label == noise_label) {
				continue;
			}
			if (label >= clustering.cluster_count) {
				throw std::invalid_argument("clustering assignment references an unknown cluster");
			}
			source_records[label].push_back(provider.id(index));
		}

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

		std::vector<std::vector<distance_type>> distances(clustering.cluster_count,
														 std::vector<distance_type>(clustering.cluster_count));
		for (std::size_t lhs = 0; lhs < clustering.cluster_count; ++lhs) {
			for (std::size_t rhs = 0; rhs < clustering.cluster_count; ++rhs) {
				distances[lhs][rhs] = provider.distance(representative_records[lhs], representative_records[rhs]);
			}
		}

		return ClusteredSpaceModel<distance_type>(std::move(records), std::move(distances), std::move(source_records),
												 std::move(representative_records), clustering.record_count,
												 clustering.algorithm, clustering.representation);
	}

	clustering_type clustering_;
};

template <typename Distance>
auto make_clustered_space_mapping(ClusteringResult<Distance> clustering) -> ClusteredSpaceMapping<Distance>
{
	return ClusteredSpaceMapping<Distance>(std::move(clustering));
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto clustered_space(const Space &space, ClusteringResult<typename Space::distance_type> clustering)
	-> typename ClusteredSpaceModel<typename Space::distance_type>::result_type
{
	return make_clustered_space_mapping(std::move(clustering)).fit(space).transform(space);
}

template <typename Provider, typename std::enable_if<DistanceProvider_v<Provider>, int>::type = 0>
auto clustered_space(const Provider &provider, ClusteringResult<typename Provider::distance_type> clustering)
	-> typename ClusteredSpaceModel<typename Provider::distance_type>::result_type
{
	auto mapping = make_clustered_space_mapping(std::move(clustering));
	auto model = mapping.fit_provider(provider);
	return model.transform();
}

} // namespace metric::mappings

#endif
