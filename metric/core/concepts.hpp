// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_CONCEPTS_HPP
#define _METRIC_CORE_CONCEPTS_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

#include "metric_traits.hpp"
#include "record_id.hpp"

namespace metric::core {

template <typename Metric, typename Record, typename = void> struct MetricCallable : std::false_type {
};

template <typename Metric, typename Record>
struct MetricCallable<Metric, Record,
					  std::void_t<decltype(std::declval<const Metric &>()(std::declval<const Record &>(),
																		   std::declval<const Record &>()))>>
	: std::true_type {
};

template <typename Metric, typename Record>
constexpr bool MetricCallable_v = MetricCallable<Metric, Record>::value;

template <typename Metric, typename Record>
using metric_result_t = decltype(std::declval<const Metric &>()(std::declval<const Record &>(),
																std::declval<const Record &>()));

template <typename Space, typename = void> struct MetricSpaceLike : std::false_type {
};

template <typename Space>
struct MetricSpaceLike<Space,
					   std::void_t<typename Space::record_type, typename Space::metric_type,
								   typename Space::distance_type, decltype(std::declval<const Space &>().size()),
								   decltype(std::declval<const Space &>().version())>> : std::true_type {
};

template <typename Space>
constexpr bool MetricSpaceLike_v = MetricSpaceLike<Space>::value;

template <typename Provider, typename = void> struct DistanceProvider : std::false_type {
};

template <typename Provider>
struct DistanceProvider<Provider,
						std::void_t<typename Provider::distance_type,
									decltype(std::declval<const Provider &>().distance(std::declval<RecordId>(),
																					   std::declval<RecordId>())),
									decltype(std::declval<const Provider &>().record_count()),
									decltype(std::declval<const Provider &>().id(std::declval<std::size_t>())),
									decltype(std::declval<const Provider &>().position_of(std::declval<RecordId>())),
									decltype(std::declval<const Provider &>().contains(std::declval<RecordId>())),
									decltype(std::declval<const Provider &>().version()),
									decltype(std::declval<const Provider &>().is_stale())>> : std::true_type {
};

template <typename Provider>
constexpr bool DistanceProvider_v = DistanceProvider<Provider>::value;

template <typename Index, typename = void> struct NeighborSearchIndex : std::false_type {
};

template <typename Index>
struct NeighborSearchIndex<Index,
						   std::void_t<typename Index::record_type, typename Index::distance_type,
									   decltype(std::declval<const Index &>().knn(
										   std::declval<const typename Index::record_type &>(), std::declval<std::size_t>())),
									   decltype(std::declval<const Index &>().record_count()),
									   decltype(std::declval<const Index &>().version()),
									   decltype(std::declval<const Index &>().is_stale())>> : std::true_type {
};

template <typename Index>
constexpr bool NeighborSearchIndex_v = NeighborSearchIndex<Index>::value;

template <typename Topology, typename = void> struct GraphTopology : std::false_type {
};

template <typename Topology>
struct GraphTopology<Topology,
					 std::void_t<typename Topology::edge_type, typename Topology::distance_type,
								 decltype(std::declval<const Topology &>().edges()),
								 decltype(std::declval<const Topology &>().record_count()),
								 decltype(std::declval<const Topology &>().version()),
								 decltype(std::declval<const Topology &>().is_stale())>> : std::true_type {
};

template <typename Topology>
constexpr bool GraphTopology_v = GraphTopology<Topology>::value;

template <typename MappingT, typename Space, typename = void> struct Mapping : std::false_type {
};

template <typename MappingT, typename Space>
struct Mapping<MappingT, Space,
			   std::void_t<decltype(std::declval<const MappingT &>().fit(std::declval<const Space &>()))>>
	: std::true_type {
};

template <typename MappingT, typename Space>
constexpr bool Mapping_v = Mapping<MappingT, Space>::value;

template <typename ModelT, typename Space, typename = void> struct MappingModel : std::false_type {
};

template <typename ModelT, typename Space>
struct MappingModel<ModelT, Space,
					std::void_t<decltype(std::declval<const ModelT &>().transform(std::declval<const Space &>()))>>
	: std::true_type {
};

template <typename ModelT, typename Space>
constexpr bool MappingModel_v = MappingModel<ModelT, Space>::value;

} // namespace metric::core

namespace metric {
using core::DistanceProvider;
using core::DistanceProvider_v;
using core::GraphTopology;
using core::GraphTopology_v;
using core::metric_result_t;
using core::Mapping;
using core::Mapping_v;
using core::MappingModel;
using core::MappingModel_v;
using core::MetricCallable;
using core::MetricCallable_v;
using core::MetricSpaceLike;
using core::MetricSpaceLike_v;
using core::NeighborSearchIndex;
using core::NeighborSearchIndex_v;
} // namespace metric

#endif
