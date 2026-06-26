// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Correlate workflows: the MGC permutation/significance test and RecordId alignment.
// MGC is a DEPENDENCE statistic between two paired finite metric spaces, never a metric.
// The permutation test turns the point statistic into an upper-tail p-value with an
// explicit seed and a reproducible result contract; RecordId alignment pairs two spaces by
// shared identity and reports dropped pairs.

#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/core/metric_space.hpp"
#include "metric/space/storage/implicit.hpp"
#include "metric/stats/correlate/compare.hpp"
#include "metric/stats/correlate/correlation.hpp"
#include "metric/stats/correlate/significance.hpp"

namespace {

using Rec = std::vector<double>;
using Eucl = mtrc::Euclidean<double>;

struct NaNPairDistance {
	auto operator()(const Rec &lhs, const Rec &rhs) const -> double
	{
		if (lhs == rhs) {
			return 0.0;
		}
		return std::numeric_limits<double>::quiet_NaN();
	}
};

struct CountingPairDistance {
	std::size_t *calls{};

	auto operator()(const Rec &lhs, const Rec &rhs) const -> double
	{
		++(*calls);
		return std::abs(lhs.front() - rhs.front());
	}
};

auto close(double lhs, double rhs, double tolerance = 1.0e-9) -> bool { return std::abs(lhs - rhs) < tolerance; }

template <typename Function> auto throws_invalid_argument(Function run) -> bool
{
	try {
		run();
	} catch (const std::invalid_argument &) {
		return true;
	}
	return false;
}

template <typename Function> auto throws_representation_error(Function run) -> bool
{
	try {
		run();
	} catch (const mtrc::RepresentationError &) {
		return true;
	}
	return false;
}

void public_domain_overloads_match_mgc_core()
{
	namespace correlate = mtrc::stats::correlate;

	std::vector<Rec> x, y;
	for (int i = 0; i < 8; ++i) {
		x.push_back({static_cast<double>(i)});
		y.push_back({static_cast<double>(i * i)});
	}
	const auto sx = mtrc::make_space(x, Eucl());
	const auto sy = mtrc::make_space(y, Eucl());
	const mtrc::space::storage::LiveDistances<decltype(sx)> left_provider(sx);
	const mtrc::space::storage::LiveDistances<decltype(sy)> right_provider(sy);

	const auto records_mgc = correlate::mgc(x, Eucl(), y, Eucl());
	const auto provider_mgc = correlate::mgc(left_provider, right_provider);
	assert(provider_mgc.left_representation == "pairwise_distances");
	assert(provider_mgc.right_representation == "pairwise_distances");
	assert(close(provider_mgc.value, records_mgc.value));

	const auto records_compare = mtrc::compare(x, Eucl(), y, Eucl());
	const auto records_correlate = mtrc::correlate(x, Eucl(), y, Eucl());
	assert(records_compare.left_representation == "records");
	assert(records_correlate.right_representation == "records");
	assert(close(records_compare.value, records_mgc.value));
	assert(close(records_correlate.value, records_mgc.value));

	const auto provider_compare = mtrc::compare(left_provider, right_provider);
	const auto provider_correlate = mtrc::correlate(left_provider, right_provider);
	assert(provider_compare.left_representation == "pairwise_distances");
	assert(provider_correlate.right_representation == "pairwise_distances");
	assert(close(provider_compare.value, records_mgc.value));
	assert(close(provider_correlate.value, records_mgc.value));

	const correlate::significance_options options(0, 123ULL);
	const auto provider_significance = correlate::mgc_significance(left_provider, right_provider, options);
	assert(provider_significance.left_representation == "pairwise_distances");
	assert(close(provider_significance.statistic, records_mgc.value));
	const auto records_significance = correlate::mgc_significance(x, Eucl(), y, Eucl(), options);
	assert(records_significance.left_representation == "records");
	assert(records_significance.right_representation == "records");
	assert(close(records_significance.statistic, records_mgc.value));
}

void large_metric_space_defaults_preflight_dense_mgc()
{
	namespace correlate = mtrc::stats::correlate;

	constexpr std::size_t large_count = 4100;
	std::vector<Rec> x;
	std::vector<Rec> y;
	x.reserve(large_count);
	y.reserve(large_count);
	for (std::size_t index = 0; index < large_count; ++index) {
		x.push_back({static_cast<double>(index)});
		y.push_back({static_cast<double>((index * 17) % 251)});
	}

	std::size_t left_calls = 0;
	std::size_t right_calls = 0;
	const auto sx = mtrc::make_space(x, CountingPairDistance{&left_calls});
	const auto sy = mtrc::make_space(y, CountingPairDistance{&right_calls});
	const auto dense_all_pairs = large_count * large_count;

	bool refused_direct_space_mgc = false;
	try {
		(void)correlate::mgc(sx, sy);
	} catch (const mtrc::RepresentationError &error) {
		refused_direct_space_mgc = true;
		const std::string message = error.what();
		assert(message.find("exact MGC") != std::string::npos);
		assert(message.find("mgc_estimate") != std::string::npos);
	}
	assert(refused_direct_space_mgc);
	assert(left_calls == 0);
	assert(right_calls == 0);

	bool refused_direct_records_mgc = false;
	try {
		(void)correlate::mgc(x, CountingPairDistance{&left_calls}, y, CountingPairDistance{&right_calls});
	} catch (const mtrc::RepresentationError &) {
		refused_direct_records_mgc = true;
	}
	assert(refused_direct_records_mgc);
	assert(left_calls == 0);
	assert(right_calls == 0);

	correlate::mgc_options full_sample_options;
	full_sample_options.sample_count = large_count;
	bool refused_full_sample_estimate = false;
	try {
		(void)correlate::mgc_estimate(sx, sy, full_sample_options);
	} catch (const mtrc::RepresentationError &) {
		refused_full_sample_estimate = true;
	}
	assert(refused_full_sample_estimate);
	assert(left_calls == 0);
	assert(right_calls == 0);

	const auto compared = mtrc::compare(sx, sy);
	assert(!compared.exact);
	assert(compared.algorithm == "mgc_estimate");
	assert(compared.left_representation == "metric_space_sample");
	assert(compared.right_representation == "metric_space_sample");
	assert(compared.left_record_count == large_count);
	assert(compared.right_record_count == large_count);
	assert(compared.sample_count == mtrc::stats::correlate::mgc_options{}.sample_count);
	assert(left_calls > 0);
	assert(right_calls > 0);
	assert(left_calls < dense_all_pairs);
	assert(right_calls < dense_all_pairs);

	left_calls = 0;
	right_calls = 0;
	const auto correlated = mtrc::correlate(sx, sy);
	assert(!correlated.exact);
	assert(correlated.algorithm == "mgc_estimate");
	assert(correlated.left_representation == "metric_space_sample");
	assert(correlated.right_representation == "metric_space_sample");
	assert(left_calls > 0);
	assert(right_calls > 0);
	assert(left_calls < dense_all_pairs);
	assert(right_calls < dense_all_pairs);

	left_calls = 0;
	right_calls = 0;
	bool refused_significance = false;
	try {
		(void)correlate::mgc_significance(sx, sy, correlate::significance_options(0));
	} catch (const mtrc::RepresentationError &error) {
		refused_significance = true;
		const std::string message = error.what();
		assert(message.find("max_dense_records") != std::string::npos);
	}
	assert(refused_significance);
	assert(left_calls == 0);
	assert(right_calls == 0);
}

void permutation_test_separates_dependent_from_independent()
{
	namespace correlate = mtrc::stats::correlate;

	std::vector<Rec> x, y;
	for (int i = 0; i < 16; ++i) {
		x.push_back({static_cast<double>(i)});
		y.push_back({static_cast<double>(i) * 2.0 + 1.0}); // affine -> perfect dependence
	}
	// Fixed deterministic permutation of x's values that destroys the pairing (no RNG).
	std::vector<Rec> z = {{7}, {3}, {11}, {1}, {14}, {5}, {9}, {0},
						  {12}, {2}, {15}, {6}, {10}, {4}, {13}, {8}};

	const auto sx = mtrc::make_space(x, Eucl());
	const auto sy = mtrc::make_space(y, Eucl());
	const auto sz = mtrc::make_space(z, Eucl());

	const correlate::significance_options options(99, 0xC0FFEEULL);

	const auto dependent = correlate::mgc_significance(sx, sy, options);
	assert(dependent.algorithm == "mgc_permutation");
	assert(!dependent.exact);
	assert(dependent.permutations == 99);
	assert(dependent.seed == 0xC0FFEEULL);
	assert(dependent.record_count == 16);
	assert(close(dependent.statistic, 1.0)); // affine transform -> identity dependence
	// No relabeling can beat perfect dependence, so the p-value is the minimum 1/(P+1).
	assert(dependent.ge_count == 0);
	assert(close(dependent.p_value, 1.0 / 100.0));

	const auto independent = correlate::mgc_significance(sx, sz, options);
	// A broken pairing scores low, so many random relabelings match or beat it: its p-value
	// is strictly larger than the dependent pair's minimum.
	assert(independent.p_value > dependent.p_value);

	// permutations == 0 reports the bare statistic (== mgc()) with a neutral p-value of 1.
	const auto no_permutations = correlate::mgc_significance(sx, sy, correlate::significance_options(0));
	assert(close(no_permutations.statistic, correlate::mgc(x, Eucl(), y, Eucl()).value));
	assert(close(no_permutations.p_value, 1.0));
}

void permutation_test_is_reproducible_for_a_seed()
{
	namespace correlate = mtrc::stats::correlate;
	std::vector<Rec> x, y;
	for (int i = 0; i < 12; ++i) {
		x.push_back({static_cast<double>(i)});
		y.push_back({static_cast<double>((i * 7) % 12)});
	}
	const auto sx = mtrc::make_space(x, Eucl());
	const auto sy = mtrc::make_space(y, Eucl());

	const auto first = correlate::mgc_significance(sx, sy, correlate::significance_options(64, 12345ULL));
	const auto second = correlate::mgc_significance(sx, sy, correlate::significance_options(64, 12345ULL));
	assert(first.ge_count == second.ge_count);
	assert(close(first.p_value, second.p_value));
	assert(close(first.statistic, second.statistic));
}

void permutation_work_budget_refuses_before_metric_calls()
{
	namespace correlate = mtrc::stats::correlate;
	std::vector<Rec> x, y;
	for (int i = 0; i < 8; ++i) {
		x.push_back({static_cast<double>(i)});
		y.push_back({static_cast<double>((i * 3) % 8)});
	}

	std::size_t left_calls = 0;
	std::size_t right_calls = 0;
	const auto sx = mtrc::make_space(x, CountingPairDistance{&left_calls});
	const auto sy = mtrc::make_space(y, CountingPairDistance{&right_calls});

	auto options = correlate::significance_options(2);
	options.max_permutation_matrix_cells = x.size() * x.size() - 1;
	assert(throws_representation_error([&] { (void)correlate::mgc_significance(sx, sy, options); }));
	assert(left_calls == 0);
	assert(right_calls == 0);

	options = correlate::significance_options(10'000'000);
	assert(throws_representation_error([&] { (void)correlate::mgc_significance(sx, sy, options); }));
	assert(left_calls == 0);
	assert(right_calls == 0);
}

void permutation_test_rejects_invalid_inputs()
{
	namespace correlate = mtrc::stats::correlate;

	std::vector<Rec> small = {{0.0}, {1.0}, {2.0}};
	std::vector<Rec> mismatched = {{0.0}, {1.0}};
	const auto small_space = mtrc::make_space(small, Eucl());
	const auto mismatched_space = mtrc::make_space(mismatched, Eucl());
	assert(throws_invalid_argument([&] { (void)correlate::mgc_significance(small_space, mismatched_space); }));

	std::vector<Rec> singleton = {{0.0}};
	const auto singleton_space = mtrc::make_space(singleton, Eucl());
	assert(throws_invalid_argument([&] { (void)correlate::mgc_significance(singleton_space, singleton_space); }));

	// A non-finite distance would corrupt MGC's internal rank sort; it must be rejected.
	std::vector<Rec> records = {{0.0}, {1.0}, {2.0}, {3.0}};
	const auto nan_space = mtrc::make_space(records, NaNPairDistance{});
	const auto finite_space = mtrc::make_space(records, Eucl());
	assert(throws_invalid_argument([&] { (void)correlate::mgc_significance(nan_space, finite_space); }));
}

void record_id_alignment_reports_dropped_pairs()
{
	namespace correlate = mtrc::stats::correlate;

	std::vector<Rec> records;
	for (int i = 0; i < 6; ++i) {
		records.push_back({static_cast<double>(i)});
	}
	auto left = mtrc::make_space(records, Eucl());
	auto right = mtrc::make_space(records, Eucl());

	// Fully shared identity -> nothing dropped.
	const auto full = correlate::mgc_by_record_id(left, right);
	assert(full.matched_count == 6);
	assert(full.dropped_left == 0 && full.dropped_right == 0);
	assert(full.correlation.algorithm == "mgc");
	assert(full.correlation.left_representation == "metric_space_aligned_by_record_id");

	// Erase one record from the right space: that id is now only in the left space.
	const auto dropped_id = right.id(2);
	assert(right.erase(dropped_id));
	const auto matched = correlate::align_by_record_id(left, right);
	assert(matched.size() == 5);
	const auto partial = correlate::mgc_by_record_id(left, right);
	assert(partial.matched_count == 5);
	assert(partial.dropped_left == 1);
	assert(partial.dropped_right == 0);

	// Fewer than two shared records cannot support a dependence statistic.
	auto sparse_right = mtrc::make_space(std::vector<Rec>{{0.0}}, Eucl());
	assert(throws_invalid_argument([&] { (void)correlate::mgc_by_record_id(left, sparse_right); }));
}

} // namespace

int main()
{
	public_domain_overloads_match_mgc_core();
	large_metric_space_defaults_preflight_dense_mgc();
	permutation_test_separates_dependent_from_independent();
	permutation_test_is_reproducible_for_a_seed();
	permutation_work_budget_refuses_before_metric_calls();
	permutation_test_rejects_invalid_inputs();
	record_id_alignment_reports_dropped_pairs();
	return 0;
}
