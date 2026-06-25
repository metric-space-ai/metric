// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MODIFY_RESAMPLE_THIN_HPP
#define _METRIC_MODIFY_RESAMPLE_THIN_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>
#include <metric/space/select/representatives.hpp>
#include <metric/stats/sample/sample.hpp>

namespace mtrc::modify::resample {

struct preserve_distribution {
	preserve_distribution() = default;
	explicit preserve_distribution(std::size_t offset) : offset(offset) {}

	std::size_t offset{};
};

template <typename Radius> struct uniform_density {
	explicit uniform_density(Radius radius) : radius(radius) {}

	Radius radius;
};

template <typename Radius> uniform_density(Radius) -> uniform_density<Radius>;

namespace detail {

template <typename Provider>
auto average_nearest_other_distance(const Provider &provider) -> double
{
	if (provider.record_count() < 2) {
		return 0.0;
	}
	double total = 0.0;
	for (std::size_t index = 0; index < provider.record_count(); ++index) {
		const auto source_id = provider.id(index);
		auto best_distance = provider.distance(source_id, provider.id(index == 0 ? 1 : 0));
		for (std::size_t other = 0; other < provider.record_count(); ++other) {
			if (index == other) {
				continue;
			}
			const auto distance = provider.distance(source_id, provider.id(other));
			if (distance < best_distance) {
				best_distance = distance;
			}
		}
		total += static_cast<double>(best_distance);
	}
	return total / static_cast<double>(provider.record_count());
}

template <typename Provider>
auto average_nearest_other_distance(const Provider &provider, const std::vector<RecordId> &record_ids) -> double
{
	if (record_ids.size() < 2) {
		return 0.0;
	}
	double total = 0.0;
	for (std::size_t index = 0; index < record_ids.size(); ++index) {
		const auto source_id = record_ids[index];
		auto best_distance = provider.distance(source_id, record_ids[index == 0 ? 1 : 0]);
		for (std::size_t other = 0; other < record_ids.size(); ++other) {
			if (index == other) {
				continue;
			}
			const auto distance = provider.distance(source_id, record_ids[other]);
			if (distance < best_distance) {
				best_distance = distance;
			}
		}
		total += static_cast<double>(best_distance);
	}
	return total / static_cast<double>(record_ids.size());
}

template <typename Provider, typename Radius>
auto average_local_volume(const Provider &provider, Radius radius) -> std::pair<double, double>
{
	using comparison_type = typename std::common_type<typename Provider::distance_type, Radius>::type;

	const auto record_count = provider.record_count();
	if (record_count == 0) {
		return {0.0, 0.0};
	}
	const auto threshold = static_cast<comparison_type>(radius);
	double total_count = 0.0;
	for (std::size_t source = 0; source < record_count; ++source) {
		const auto source_id = provider.id(source);
		std::size_t count = 0;
		for (std::size_t target = 0; target < record_count; ++target) {
			if (static_cast<comparison_type>(provider.distance(source_id, provider.id(target))) <= threshold) {
				++count;
			}
		}
		total_count += static_cast<double>(count);
	}
	const auto average_count = total_count / static_cast<double>(record_count);
	return {average_count, average_count / static_cast<double>(record_count)};
}

template <typename Provider, typename Radius>
auto average_local_volume(const Provider &provider, const std::vector<RecordId> &record_ids, Radius radius)
	-> std::pair<double, double>
{
	using comparison_type = typename std::common_type<typename Provider::distance_type, Radius>::type;

	if (record_ids.empty()) {
		return {0.0, 0.0};
	}
	const auto threshold = static_cast<comparison_type>(radius);
	double total_count = 0.0;
	for (const auto source_id : record_ids) {
		std::size_t count = 0;
		for (const auto target_id : record_ids) {
			if (static_cast<comparison_type>(provider.distance(source_id, target_id)) <= threshold) {
				++count;
			}
		}
		total_count += static_cast<double>(count);
	}
	const auto average_count = total_count / static_cast<double>(record_ids.size());
	return {average_count, average_count / static_cast<double>(record_ids.size())};
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto subset_mapping_from_record_ids(const Space &space, std::vector<RecordId> kept_ids, std::string mapping,
									std::string strategy, std::string validity,
									std::string representation = "metric_space",
									core::ModificationDiagnostics diagnostics = {},
									std::vector<std::size_t> assignments = {},
									std::vector<typename Space::distance_type> nearest_representative_distances = {},
									std::vector<std::size_t> representative_multiplicities = {},
									std::vector<double> representative_weights = {},
									typename Space::distance_type coverage_radius = typename Space::distance_type{},
									double average_assignment_distance = 0.0, bool has_assignment_summary = false)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using target_space_type = MetricSpace<typename Space::record_type, typename Space::metric_type>;

	auto records = ::mtrc::records_for_record_ids(space, kept_ids);
	auto lineage = ::mtrc::one_to_one_lineage(std::move(kept_ids));
	target_space_type thinned_space(std::move(records), space.metric());
	return core::make_mapping_result(
		std::move(thinned_space), std::move(lineage.source_records), std::move(lineage.representative_records),
		space.size(), false, std::move(mapping), std::move(strategy), std::move(representation),
		core::metric_traits<typename Space::metric_type>::law, false, std::move(validity), std::move(diagnostics),
		std::move(assignments), std::move(nearest_representative_distances), std::move(representative_multiplicities),
		std::move(representative_weights), coverage_radius, average_assignment_distance, has_assignment_summary);
}

template <typename Space, typename Radius>
auto uniform_density_mapping(const Space &space, uniform_density<Radius> strategy, std::string mapping,
							 std::string strategy_name, std::string validity)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	space::storage::LiveDistances<Space> provider(space);
	auto representatives = find_representatives(space, space::select::radius_coverage{strategy.radius});
	auto assignment = core::assign_records_to_representatives(
		provider, representatives.representatives, "uniform-density representatives must not be empty",
		"uniform-density representative id is outside provider");
	auto representative_multiplicities =
		core::compression_representative_multiplicities(assignment.assignments, representatives.representatives.size());
	auto representative_weights =
		core::compression_representative_weights(representative_multiplicities, space.size());
	core::ModificationDiagnostics diagnostics;
	diagnostics.diagnostic = "uniform_density_thinning";
	diagnostics.policy = "maximal_radius_net";
	diagnostics.source_record_count = space.size();
	diagnostics.target_record_count = representatives.representatives.size();
	diagnostics.radius = static_cast<double>(strategy.radius);
	diagnostics.coverage_radius = static_cast<double>(representatives.coverage_radius);
	diagnostics.average_assignment_distance = representatives.average_nearest_distance;
	diagnostics.source_average_nearest_neighbor_distance = detail::average_nearest_other_distance(provider);
	diagnostics.target_average_nearest_neighbor_distance =
		detail::average_nearest_other_distance(provider, representatives.representatives);
	diagnostics.local_density_drift =
		diagnostics.target_average_nearest_neighbor_distance - diagnostics.source_average_nearest_neighbor_distance;
	const auto source_local_volume = detail::average_local_volume(provider, strategy.radius);
	const auto target_local_volume =
		detail::average_local_volume(provider, representatives.representatives, strategy.radius);
	diagnostics.local_volume_radius = static_cast<double>(strategy.radius);
	diagnostics.source_average_local_volume_count = source_local_volume.first;
	diagnostics.target_average_local_volume_count = target_local_volume.first;
	diagnostics.local_volume_count_drift =
		diagnostics.target_average_local_volume_count - diagnostics.source_average_local_volume_count;
	diagnostics.source_average_local_volume_density = source_local_volume.second;
	diagnostics.target_average_local_volume_density = target_local_volume.second;
	diagnostics.local_volume_density_drift =
		diagnostics.target_average_local_volume_density - diagnostics.source_average_local_volume_density;
	diagnostics.empirical_density_preserved = false;
	diagnostics.populated = true;
	return subset_mapping_from_record_ids(
		space, std::move(representatives.representatives), std::move(mapping), std::move(strategy_name),
		std::move(validity), representatives.representation, std::move(diagnostics),
		std::move(assignment.assignments), std::move(assignment.nearest_distances),
		std::move(representative_multiplicities), std::move(representative_weights), representatives.coverage_radius,
		representatives.average_nearest_distance, true);
}

} // namespace detail

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto thin(const Space &space, std::size_t count, preserve_distribution strategy = {})
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (count > 0 && strategy.offset >= space.size()) {
		throw std::invalid_argument("thin preserve_distribution offset is outside the record set");
	}
	const auto positions = stats::sample::regular_sample_positions(space.size(), count, strategy.offset);
	std::vector<RecordId> kept_ids;
	kept_ids.reserve(positions.positions.size());
	for (const auto position : positions.positions) {
		kept_ids.push_back(space.id(position));
	}
	return detail::subset_mapping_from_record_ids(
		space, std::move(kept_ids), "thin", "preserve_distribution_regular",
		"distribution-preserving deterministic thinning; kept records are an unmodified regular sample under the "
		"source metric; retained records carry sample mass and no full-source assignment map is implied");
}

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto distribution_sample(const Space &space, std::size_t count, preserve_distribution strategy = {})
	-> decltype(thin(space, count, strategy))
{
	return thin(space, count, strategy);
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto thin(const Space &space, uniform_density<Radius> strategy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	return detail::uniform_density_mapping(
		space, strategy, "thin", "uniform_density_radius_net",
		"uniform-density deterministic thinning by maximal metric radius net; kept records are an unmodified "
		"radius-separated and radius-covering subset under the source metric; empirical density is intentionally "
		"flattened");
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto uniform_density_sample(const Space &space, uniform_density<Radius> strategy) -> decltype(thin(space, strategy))
{
	return thin(space, strategy);
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto equalize(const Space &space, uniform_density<Radius> strategy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	return detail::uniform_density_mapping(
		space, strategy, "equalize", "uniform_density_radius_net",
		"density-equalizing deterministic thinning by maximal metric radius net; kept records are an unmodified "
		"radius-separated and radius-covering subset under the source metric; empirical density is intentionally "
		"normalized toward uniform metric coverage");
}

} // namespace mtrc::modify::resample

namespace mtrc {
using modify::resample::distribution_sample;
using modify::resample::equalize;
using modify::resample::preserve_distribution;
using modify::resample::thin;
using modify::resample::uniform_density;
using modify::resample::uniform_density_sample;
} // namespace mtrc

#endif
