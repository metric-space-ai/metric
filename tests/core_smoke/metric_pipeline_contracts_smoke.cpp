// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Pipeline-domain contracts: source metric spaces, pairwise providers, coordinate
// spaces, and mapping results are distinct concepts. Distance-only operators may work
// on opaque source records; coordinate-only operators such as stats::properties::entropy
// require an explicit mapped/embedded coordinate space.

#include <array>
#include <cassert>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "metric/core/concepts.hpp"
#include "metric/core/metric_space.hpp"
#include "metric/core/result.hpp"
#include "metric/metric/catalog.hpp"
#include "metric/modify/map/map.hpp"
#include "metric/space/storage/implicit.hpp"
#include "metric/stats/search/nearest.hpp"
#include "metric/stats/properties/entropy.hpp"

namespace {

struct Item {
	int position{};
	std::string label;
};

struct ItemMetric {
	auto operator()(const Item &lhs, const Item &rhs) const -> double
	{
		const auto delta = lhs.position > rhs.position ? lhs.position - rhs.position : rhs.position - lhs.position;
		return static_cast<double>(delta) + (lhs.label == rhs.label ? 0.0 : 0.5);
	}
};

struct IntDistance {
	auto operator()(int lhs, int rhs) const -> double
	{
		return static_cast<double>(lhs > rhs ? lhs - rhs : rhs - lhs);
	}
};

using opaque_space_type = mtrc::MetricSpace<Item, ItemMetric>;
using vector_space_type = mtrc::MetricSpace<std::vector<double>, mtrc::Euclidean<double>>;
using array_space_type = mtrc::MetricSpace<std::array<double, 2>, mtrc::Euclidean<double>>;
using chebyshev_vector_space_type = mtrc::MetricSpace<std::vector<double>, mtrc::Chebyshev<double>>;
using ruzicka_vector_space_type = mtrc::MetricSpace<std::vector<double>, mtrc::Ruzicka<double>>;
using string_space_type = mtrc::MetricSpace<std::string, mtrc::Edit<std::string>>;
using coordinate_mapping_type = mtrc::MappingResult<vector_space_type>;
using opaque_mapping_type = mtrc::MappingResult<opaque_space_type>;

template <typename Space, typename = void> struct entropy_available : std::false_type {};

template <typename Space>
struct entropy_available<
	Space, std::void_t<decltype(mtrc::stats::properties::entropy(std::declval<const Space &>()))>> : std::true_type {};

template <typename Container, typename Metric, typename = void> struct records_metric_entropy_available : std::false_type {};

template <typename Container, typename Metric>
struct records_metric_entropy_available<
	Container, Metric,
	std::void_t<decltype(mtrc::stats::properties::entropy(std::declval<const Container &>(),
														  std::declval<const Metric &>()))>> : std::true_type {};

template <typename Provider, typename Query, typename = void> struct provider_knn_query_available : std::false_type {};

template <typename Provider, typename Query>
struct provider_knn_query_available<
	Provider, Query,
	std::void_t<decltype(mtrc::stats::search::knn(std::declval<const Provider &>(), std::declval<const Query &>(),
												 std::size_t{}))>> : std::true_type {};

template <typename Provider, typename Query, typename Radius, typename = void>
struct provider_range_query_available : std::false_type {};

template <typename Provider, typename Query, typename Radius>
struct provider_range_query_available<
	Provider, Query, Radius,
	std::void_t<decltype(mtrc::stats::search::range(std::declval<const Provider &>(),
													std::declval<const Query &>(), std::declval<Radius>()))>>
	: std::true_type {};

static_assert(mtrc::RecordMetricSpaceLike_v<opaque_space_type>, "MetricSpace is a full record metric space");
static_assert(mtrc::RecordMetricSpaceLike_v<vector_space_type>, "coordinate MetricSpace is still a record space");
static_assert(!mtrc::RecordMetricSpaceLike_v<int>, "plain records are not metric spaces");

static_assert(!mtrc::CoordinateRecordLike_v<Item>, "opaque domain records are not coordinates");
static_assert(mtrc::CoordinateRecordLike_v<std::vector<double>>, "numeric vectors are coordinate records");
static_assert(mtrc::CoordinateRecordLike_v<std::array<double, 2>>, "numeric arrays are coordinate records");
static_assert(!mtrc::CoordinateRecordLike_v<std::string>, "strings must not become coordinates by accident");

static_assert(mtrc::CoordinateMetricLike_v<mtrc::Euclidean<double>, std::vector<double>>,
			  "Euclidean is an admitted coordinate-neighborhood metric");
static_assert(mtrc::CoordinateMetricLike_v<mtrc::Chebyshev<double>, std::vector<double>>,
			  "Chebyshev remains an admitted coordinate-neighborhood metric for kpN entropy");
static_assert(!mtrc::CoordinateMetricLike_v<mtrc::Ruzicka<double>, std::vector<double>>,
			  "coordinate-shaped metric spaces still need an admitted coordinate-neighborhood metric");

static_assert(!mtrc::CoordinateSpaceLike_v<opaque_space_type>, "opaque source spaces are not coordinate spaces");
static_assert(mtrc::CoordinateSpaceLike_v<vector_space_type>, "numeric vector spaces are coordinate spaces");
static_assert(mtrc::CoordinateSpaceLike_v<array_space_type>, "numeric array spaces are coordinate spaces");
static_assert(mtrc::CoordinateSpaceLike_v<chebyshev_vector_space_type>,
			  "Chebyshev coordinate spaces remain entropy-consumable");
static_assert(!mtrc::CoordinateSpaceLike_v<ruzicka_vector_space_type>,
			  "metric-law alone does not make a coordinate-space operator domain");
static_assert(!mtrc::CoordinateSpaceLike_v<string_space_type>, "string spaces are source spaces, not coordinate spaces");
static_assert(entropy_available<vector_space_type>::value, "entropy accepts admitted coordinate spaces");
static_assert(!entropy_available<opaque_space_type>::value, "entropy rejects opaque source spaces");
static_assert(!entropy_available<ruzicka_vector_space_type>::value,
			  "entropy rejects coordinate-shaped spaces with unadmitted coordinate metrics");
static_assert(!records_metric_entropy_available<std::vector<std::vector<double>>, mtrc::Euclidean<double>>::value,
			  "entropy is not a records-plus-metric shortcut, even for coordinate records");
static_assert(!records_metric_entropy_available<std::vector<Item>, ItemMetric>::value,
			  "entropy is not a records-plus-metric shortcut for opaque source records");

static_assert(mtrc::MappingResultLike_v<coordinate_mapping_type>, "MappingResult has pipeline provenance fields");
static_assert(mtrc::CoordinateMappingResultLike_v<coordinate_mapping_type>,
			  "MappingResult with coordinate target is coordinate-consumable");
static_assert(!mtrc::CoordinateMappingResultLike_v<opaque_mapping_type>,
			  "opaque MappingResult targets are not coordinate-operator domains");

template <typename MappingResult> auto rejects_mapping_contract(const MappingResult &mapping) -> bool
{
	try {
		mtrc::core::require_mapping_result_contract(mapping, "broken mapping contract");
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

} // namespace

int main()
{
	const auto source = mtrc::make_space(std::vector<int>{0, 1, 2, 3}, IntDistance{});
	const auto mapped = mtrc::map(source,
								  [](int value) {
									  return std::vector<double>{
										  static_cast<double>(value),
										  static_cast<double>(value * value),
									  };
								  },
								  mtrc::Euclidean<double>{});

	static_assert(mtrc::MappingResultLike_v<decltype(mapped)>, "mapped result satisfies MappingResultLike");
	static_assert(mtrc::CoordinateMappingResultLike_v<decltype(mapped)>,
				  "mapped coordinate result satisfies CoordinateMappingResultLike");

	const mtrc::space::storage::LiveDistances<decltype(source)> provider(source);
	static_assert(mtrc::PairwiseDistanceProviderLike_v<decltype(provider)>,
				  "LiveDistances is a pairwise distance provider");
	static_assert(!mtrc::RecordMetricSpaceLike_v<decltype(provider)>,
				  "distance providers are representations, not source spaces");
	static_assert(provider_knn_query_available<decltype(provider), mtrc::RecordId>::value,
				  "pairwise providers support RecordId kNN queries");
	static_assert(!provider_knn_query_available<decltype(provider), int>::value,
				  "pairwise providers do not accept raw record-value kNN queries");
	static_assert(provider_range_query_available<decltype(provider), mtrc::RecordId, double>::value,
				  "pairwise providers support RecordId range queries");
	static_assert(!provider_range_query_available<decltype(provider), int, double>::value,
				  "pairwise providers do not accept raw record-value range queries");

	assert(mapped.source_record_count == source.size());
	assert(mapped.space.size() == source.size());
	assert(mapped.out_of_sample_supported);
	assert(mapped.mapping == "deterministic_transform");
	assert(mapped.strategy == "deterministic_transform");
	mtrc::core::require_mapping_result_contract(mapped, "pipeline mapped coordinate contract");

	auto broken = mapped;
	broken.validity.clear();
	assert(rejects_mapping_contract(broken));

	auto blank_representation = mapped;
	blank_representation.representation = " \t";
	assert(rejects_mapping_contract(blank_representation));

	auto empty_lineage = mapped;
	empty_lineage.source_records[0].clear();
	assert(rejects_mapping_contract(empty_lineage));

	auto duplicate_within_row = mapped;
	duplicate_within_row.source_records[0].push_back(duplicate_within_row.source_records[0][0]);
	assert(rejects_mapping_contract(duplicate_within_row));

	auto representative_outside_row = mapped;
	representative_outside_row.representative_records[0] = source.id(1);
	assert(rejects_mapping_contract(representative_outside_row));

	auto too_many_unique_sources = mapped;
	too_many_unique_sources.source_record_count = 1;
	assert(rejects_mapping_contract(too_many_unique_sources));

	auto duplicate_across_rows = mapped;
	duplicate_across_rows.source_records[1] = duplicate_across_rows.source_records[0];
	duplicate_across_rows.representative_records[1] = duplicate_across_rows.representative_records[0];
	mtrc::core::require_mapping_result_contract(duplicate_across_rows, "duplicate across rows remains valid");

	auto zero_source_count = mapped;
	zero_source_count.source_record_count = 0;
	assert(rejects_mapping_contract(zero_source_count));

	auto sparse_source = mtrc::make_space(std::vector<int>{0, 1, 2, 3}, IntDistance{});
	assert(sparse_source.erase(sparse_source.id(0)));
	const auto sparse_mapped = mtrc::map(sparse_source, [](int value) { return std::vector<double>{double(value)}; },
										 mtrc::Euclidean<double>{});
	assert(sparse_mapped.representative_records[0] == mtrc::RecordId::from_index(1));
	mtrc::core::require_mapping_result_contract(sparse_mapped, "sparse source ids remain valid");
	return 0;
}
