// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MODIFY_RESAMPLE_THIN_HPP
#define _METRIC_MODIFY_RESAMPLE_THIN_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>
#include <metric/space/chunked.hpp>
#include <metric/space/sample_plan.hpp>
#include <metric/space/select/representatives.hpp>
#include <metric/space/storage/execution.hpp>
#include <metric/space/storage/implicit.hpp>
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

constexpr std::size_t default_approximate_resample_sample_count = 512;
constexpr std::size_t max_default_exact_resample_records = 4096;

inline auto approximate_resample_sample_count(std::size_t record_count,
											  ::mtrc::space::storage::policy runtime_policy) -> std::size_t
{
	if (record_count == 0) {
		return 0;
	}
	if (runtime_policy.graph_neighbors() != 0) {
		return std::min(runtime_policy.graph_neighbors(), record_count);
	}
	return std::min(default_approximate_resample_sample_count, record_count);
}

template <typename Provider>
auto sampled_candidate_ids(const Provider &provider, ::mtrc::space::storage::policy runtime_policy)
	-> std::vector<RecordId>
{
	const auto sample_count = approximate_resample_sample_count(provider.record_count(), runtime_policy);
	const auto candidate_plan = ::mtrc::space::regular_sample_positions(provider.record_count(), sample_count);
	return ::mtrc::space::record_ids_for_sample_plan(provider, candidate_plan);
}

template <typename Provider, typename Radius>
auto radius_coverage_from_candidates(const Provider &provider, const std::vector<RecordId> &candidate_ids,
									 Radius radius, std::string strategy, std::string representation)
	-> RepresentativeSet<typename Provider::distance_type>
{
	using distance_type = typename Provider::distance_type;

	if (radius < Radius{}) {
		throw std::invalid_argument("coverage radius must be non-negative");
	}
	if (candidate_ids.empty()) {
		return core::make_representative_set(std::vector<RecordId>{}, std::vector<distance_type>{},
											 provider.record_count(), 0, std::move(strategy),
											 std::move(representation), false);
	}

	const auto threshold = static_cast<distance_type>(radius);
	std::vector<RecordId> representatives;
	std::vector<bool> covered(candidate_ids.size(), false);
	std::size_t covered_count = 0;
	while (covered_count < candidate_ids.size()) {
		auto seed_index = std::size_t{0};
		while (seed_index < covered.size() && covered[seed_index]) {
			++seed_index;
		}
		if (seed_index == covered.size()) {
			break;
		}
		const auto seed_id = candidate_ids[seed_index];
		representatives.push_back(seed_id);
		for (std::size_t candidate_index = 0; candidate_index < candidate_ids.size(); ++candidate_index) {
			if (covered[candidate_index]) {
				continue;
			}
			if (provider.distance(seed_id, candidate_ids[candidate_index]) <= threshold) {
				covered[candidate_index] = true;
				++covered_count;
			}
		}
	}

	const auto representative_count = representatives.size();
	return core::make_representative_set(
		std::move(representatives), std::vector<distance_type>{}, provider.record_count(), representative_count,
		std::move(strategy), std::move(representation), false);
}

template <typename Distance>
auto summarize_nearest_distances(const std::vector<Distance> &distances) -> std::pair<Distance, double>
{
	if (distances.empty()) {
		return {Distance{}, 0.0};
	}
	auto coverage_radius = distances.front();
	double total = 0.0;
	for (const auto distance : distances) {
		if (coverage_radius < distance) {
			coverage_radius = distance;
		}
		total += static_cast<double>(distance);
	}
	return {coverage_radius, total / static_cast<double>(distances.size())};
}

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

template <typename Space, typename Provider, typename Radius>
auto uniform_density_mapping_from_representatives(
	const Space &space, const Provider &provider, RepresentativeSet<typename Space::distance_type> representatives,
	uniform_density<Radius> strategy, std::string mapping, std::string strategy_name, std::string validity,
	const std::vector<RecordId> &diagnostic_record_ids = {})
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	core::ModificationDiagnostics diagnostics;
	diagnostics.diagnostic = "uniform_density_thinning";
	diagnostics.policy = "maximal_radius_net";
	if (!representatives.exact) {
		diagnostics.policy = representatives.representation == "chunked_space_view" ? "chunked_radius_net"
																					: "sampled_radius_net";
	}
	diagnostics.source_record_count = space.size();
	diagnostics.target_record_count = representatives.representatives.size();
	diagnostics.radius = static_cast<double>(strategy.radius);
	diagnostics.empirical_density_preserved = false;
	diagnostics.populated = true;

	if (representatives.representatives.empty()) {
		if (space.size() != 0) {
			throw std::invalid_argument("uniform-density representatives must not be empty");
		}
		return subset_mapping_from_record_ids(
			space, std::vector<RecordId>{}, std::move(mapping), std::move(strategy_name), std::move(validity),
			representatives.representation, std::move(diagnostics), {}, {}, {}, {}, typename Space::distance_type{},
			0.0, true);
	}

	auto assignment = core::assign_records_to_representatives(
		provider, representatives.representatives, "uniform-density representatives must not be empty",
		"uniform-density representative id is outside provider");
	const auto assignment_summary = summarize_nearest_distances(assignment.nearest_distances);
	auto representative_multiplicities =
		core::compression_representative_multiplicities(assignment.assignments, representatives.representatives.size());
	auto representative_weights =
		core::compression_representative_weights(representative_multiplicities, space.size());
	diagnostics.coverage_radius = static_cast<double>(assignment_summary.first);
	diagnostics.average_assignment_distance = assignment_summary.second;
	diagnostics.source_average_nearest_neighbor_distance =
		diagnostic_record_ids.empty() ? detail::average_nearest_other_distance(provider)
									  : detail::average_nearest_other_distance(provider, diagnostic_record_ids);
	diagnostics.target_average_nearest_neighbor_distance =
		detail::average_nearest_other_distance(provider, representatives.representatives);
	diagnostics.local_density_drift =
		diagnostics.target_average_nearest_neighbor_distance - diagnostics.source_average_nearest_neighbor_distance;
	const auto source_local_volume =
		diagnostic_record_ids.empty() ? detail::average_local_volume(provider, strategy.radius)
									  : detail::average_local_volume(provider, diagnostic_record_ids, strategy.radius);
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
	return subset_mapping_from_record_ids(
		space, std::move(representatives.representatives), std::move(mapping), std::move(strategy_name),
		std::move(validity), representatives.representation, std::move(diagnostics),
		std::move(assignment.assignments), std::move(assignment.nearest_distances),
		std::move(representative_multiplicities), std::move(representative_weights), assignment_summary.first,
		assignment_summary.second, true);
}

template <typename Space, typename Radius>
auto uniform_density_mapping(const Space &space, uniform_density<Radius> strategy, std::string mapping,
							 std::string strategy_name, std::string validity,
							 ::mtrc::space::select::exact_representative_work_options representative_work_options)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	::mtrc::space::storage::LiveDistances<Space> provider(space);
	auto representatives = find_representatives(
		provider, ::mtrc::space::select::radius_coverage{strategy.radius}, representative_work_options);
	representatives.representation = "metric_space";
	return uniform_density_mapping_from_representatives(
		space, provider, std::move(representatives), strategy, std::move(mapping), std::move(strategy_name),
		std::move(validity));
}

template <typename Space, typename Radius>
auto sampled_uniform_density_mapping(const Space &space, uniform_density<Radius> strategy,
									 ::mtrc::space::storage::policy runtime_policy, std::string mapping,
									 std::string strategy_name, std::string validity)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	::mtrc::space::storage::LiveDistances<Space> provider(space);
	auto candidate_ids = sampled_candidate_ids(provider, runtime_policy);
	auto representatives = radius_coverage_from_candidates(
		provider, candidate_ids, strategy.radius, "sampled_" + strategy_name, "sampled_metric_space");
	auto sampled_strategy_name = "sampled_" + strategy_name;
	return uniform_density_mapping_from_representatives(
		space, provider, std::move(representatives), strategy, std::move(mapping), std::move(sampled_strategy_name),
		std::move(validity), candidate_ids);
}

template <typename Space, typename Radius>
auto chunked_uniform_density_mapping(const Space &space, uniform_density<Radius> strategy,
									 const ::mtrc::space::storage::execution_plan &plan,
									 std::string mapping, std::string strategy_name, std::string validity)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	::mtrc::space::storage::LiveDistances<Space> provider(space);
	const auto chunk_size = plan.chunk_size == 0 ? std::size_t{1} : plan.chunk_size;
	const auto chunks = ::mtrc::space::chunked_view(space, chunk_size);
	auto candidate_ids = chunks.representative_ids();
	auto representatives = radius_coverage_from_candidates(
		provider, candidate_ids, strategy.radius, "chunked_" + strategy_name, "chunked_space_view");
	auto chunked_strategy_name = "chunked_" + strategy_name;
	return uniform_density_mapping_from_representatives(
		space, provider, std::move(representatives), strategy, std::move(mapping), std::move(chunked_strategy_name),
		std::move(validity), candidate_ids);
}

inline auto throw_resample_plan_refusal(const ::mtrc::space::storage::execution_plan &plan) -> void
{
	auto message = plan.reason.empty() ? std::string("resample runtime policy refused by resource budget")
									   : plan.reason;
	if (!plan.fallback_hint.empty()) {
		message += "; fallback: " + plan.fallback_hint;
	}
	throw ::mtrc::RepresentationError(message);
}

template <typename Space, typename Radius>
auto uniform_density_mapping(const Space &space, uniform_density<Radius> strategy,
							 ::mtrc::space::storage::policy runtime_policy, std::string mapping,
							 std::string strategy_name, std::string validity)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	::mtrc::space::storage::require_parallel_metric<typename Space::metric_type>(runtime_policy);
	auto plan = ::mtrc::space::storage::estimate_cost(space, "resample", runtime_policy);
	if (!plan.refused && runtime_policy.uses_materialization() &&
		!::mtrc::space::storage::uses_distance_table_materialization(runtime_policy)) {
		plan = ::mtrc::space::storage::estimate_cost(
			space, "resample", ::mtrc::space::storage::using_distance_table(runtime_policy));
	}
	if (plan.refused) {
		throw_resample_plan_refusal(plan);
	}
	if (runtime_policy.is_approximate() ||
		(plan.allowed && plan.downgraded && !plan.exact && plan.representation == "sampled_metric_space")) {
		return sampled_uniform_density_mapping(
			space, strategy, runtime_policy, std::move(mapping), std::move(strategy_name), std::move(validity));
	}
	if (plan.allowed && plan.downgraded && !plan.exact && plan.representation == "chunked_space_view") {
		return chunked_uniform_density_mapping(
			space, strategy, plan, std::move(mapping), std::move(strategy_name), std::move(validity));
	}

	if (runtime_policy.uses_materialization()) {
		return ::mtrc::space::storage::with_materialized_distance_provider(
			space, runtime_policy, "resample", [&](const auto &provider, const auto &materialized_plan) {
				auto representatives = find_representatives(
					provider, ::mtrc::space::select::radius_coverage{strategy.radius},
					::mtrc::space::select::representative_detail::unbounded_exact_representative_work());
				representatives.representation =
					::mtrc::space::storage::materialized_operator_representation(materialized_plan);
				return uniform_density_mapping_from_representatives(
					space, provider, std::move(representatives), strategy, std::move(mapping), std::move(strategy_name),
					std::move(validity));
			});
	}

	auto result = uniform_density_mapping(
		space, strategy, std::move(mapping), std::move(strategy_name), std::move(validity),
		::mtrc::space::select::representative_detail::exact_representative_work_for_policy(runtime_policy));
	result.representation = ::mtrc::space::storage::execution_representation(runtime_policy);
	return result;
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
	if (space.size() > detail::max_default_exact_resample_records) {
		return detail::sampled_uniform_density_mapping(
			space, strategy, ::mtrc::space::storage::approximate(), "thin", "uniform_density_radius_net",
			"bounded approximate uniform-density thinning by sampled metric radius net; kept records are an "
			"unmodified subset under the source metric; full-source assignment is exact only to sampled representatives");
	}
	return detail::uniform_density_mapping(
		space, strategy, "thin", "uniform_density_radius_net",
		"uniform-density deterministic thinning by maximal metric radius net; kept records are an unmodified "
		"radius-separated and radius-covering subset under the source metric; empirical density is intentionally "
		"flattened",
		::mtrc::space::select::representative_detail::unbounded_exact_representative_work());
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto thin(const Space &space, uniform_density<Radius> strategy, ::mtrc::space::storage::policy runtime_policy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	return detail::uniform_density_mapping(
		space, strategy, runtime_policy, "thin", "uniform_density_radius_net",
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
auto uniform_density_sample(const Space &space, uniform_density<Radius> strategy,
							::mtrc::space::storage::policy runtime_policy)
	-> decltype(thin(space, strategy, runtime_policy))
{
	return thin(space, strategy, runtime_policy);
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto equalize(const Space &space, uniform_density<Radius> strategy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	if (space.size() > detail::max_default_exact_resample_records) {
		return detail::sampled_uniform_density_mapping(
			space, strategy, ::mtrc::space::storage::approximate(), "equalize", "uniform_density_radius_net",
			"bounded approximate density-equalizing thinning by sampled metric radius net; kept records are an "
			"unmodified subset under the source metric; full-source assignment is exact only to sampled representatives");
	}
	return detail::uniform_density_mapping(
		space, strategy, "equalize", "uniform_density_radius_net",
		"density-equalizing deterministic thinning by maximal metric radius net; kept records are an unmodified "
		"radius-separated and radius-covering subset under the source metric; empirical density is intentionally "
		"normalized toward uniform metric coverage",
		::mtrc::space::select::representative_detail::unbounded_exact_representative_work());
}

template <typename Space, typename Radius, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto equalize(const Space &space, uniform_density<Radius> strategy,
			  ::mtrc::space::storage::policy runtime_policy)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	return detail::uniform_density_mapping(
		space, strategy, runtime_policy, "equalize", "uniform_density_radius_net",
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
