// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Entropy result status. Entropy is a PROPERTY of an embedded coordinate metric space
// (a kpN local-Gaussian differential entropy estimator), not a shortcut from source
// records plus their domain metric. The EntropyResult carries an explicit status so
// callers can distinguish a valid estimate (including a valid NEGATIVE differential
// entropy) from too_few_records / degenerate failures, and reports the EFFECTIVE
// (clamped) neighbor count and approximation order. LAPACK-gated because the estimator
// uses the LAPACK-backed numeric core.

#include <cassert>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/core/errors.hpp"
#include "metric/core/metric_space.hpp"
#include "metric/stats/properties/entropy.hpp"

namespace entropy_status_smoke {

using Rec = std::vector<double>;
using Cheb = mtrc::Chebyshev<double>;

auto close(double lhs, double rhs, double tolerance = 1.0e-6) -> bool { return std::abs(lhs - rhs) < tolerance; }

struct CountingCheb {
	std::shared_ptr<std::size_t> calls{std::make_shared<std::size_t>(0)};

	auto operator()(const Rec &lhs, const Rec &rhs) const -> double
	{
		++(*calls);
		return Cheb()(lhs, rhs);
	}
};

} // namespace entropy_status_smoke

namespace mtrc::core {

template <> struct metric_traits<::entropy_status_smoke::CountingCheb> {
	static constexpr auto law = metric_law::metric;
	static constexpr auto records = record_kind::aligned_vector;
	static constexpr bool thread_safe = false;
};

namespace detail {
template <> struct coordinate_metric_family<::entropy_status_smoke::CountingCheb> : std::true_type {};
} // namespace detail

} // namespace mtrc::core

int main()
{
	namespace properties = mtrc::stats::properties;
	using namespace entropy_status_smoke;

	// Valid estimate. Four distinct 2-D records; the differential entropy here is NEGATIVE,
	// which is a VALID result, never a failure (regression anchor shared with the entropy
	// smokes). With requested (k=3, p=2) on n=4 the estimator clamps to (k=2, p=3).
	const std::vector<Rec> base = {{5.0, 5.0}, {2.0, 2.0}, {3.0, 3.0}, {5.0, 1.0}};
	const auto base_space = mtrc::make_space(base, Cheb());
	const auto valid = properties::entropy(base_space, 3, 2);
	assert(valid.status == mtrc::entropy_status::valid);
	assert(valid.succeeded() && !valid.failed());
	assert(std::isfinite(valid.value) && valid.value < 0.0);
	assert(close(valid.value, -4.4489104772539489));
	assert(valid.neighbor_count == 3 && valid.approximation_order == 2);
	assert(valid.effective_neighbor_count == 2 && valid.effective_approximation_order == 3);
	assert(mtrc::summary(valid).find("EntropyResult") != std::string::npos);
	assert(mtrc::summary(valid).find("status=valid") != std::string::npos);

	// Too few records: n < 4 -> NaN sentinel, too_few_records, no estimation attempted.
	const std::vector<Rec> tiny = {{5.0, 5.0}, {2.0, 2.0}, {3.0, 3.0}};
	const auto too_few = properties::entropy(mtrc::make_space(tiny, Cheb()), 3, 2);
	assert(too_few.status == mtrc::entropy_status::too_few_records);
	assert(too_few.failed());
	assert(std::isnan(too_few.value));
	assert(too_few.effective_neighbor_count == 0 && too_few.effective_approximation_order == 0);

	// Degenerate: a valid-sized but information-free space (all-identical records) yields the
	// NaN sentinel with status degenerate (distinct from too_few_records).
	const std::vector<Rec> identical = {{1.0, 1.0}, {1.0, 1.0}, {1.0, 1.0}, {1.0, 1.0}};
	const auto degenerate = properties::entropy(mtrc::make_space(identical, Cheb()), 3, 2);
	assert(degenerate.status == mtrc::entropy_status::degenerate);
	assert(degenerate.failed());
	assert(std::isnan(degenerate.value));
	assert(degenerate.effective_neighbor_count == 2 && degenerate.effective_approximation_order == 3);

	// Space overload carries the status through and tags the representation.
	const auto space_entropy = properties::entropy(base_space, 3, 2);
	assert(space_entropy.representation == "metric_space");
	assert(space_entropy.status == mtrc::entropy_status::valid);
	assert(close(space_entropy.value, valid.value));

	std::vector<Rec> large_records;
	large_records.reserve(64);
	for (std::size_t i = 0; i < 64; ++i) {
		large_records.push_back(Rec{static_cast<double>(i), static_cast<double>((i * i) % 17)});
	}

	CountingCheb refused_metric;
	auto refused_space = mtrc::make_space(large_records, refused_metric);
	properties::entropy_options exact_only;
	exact_only.neighbor_count = 3;
	exact_only.approximation_order = 2;
	exact_only.max_exact_records = 8;
	exact_only.allow_approximate = false;
	bool refused = false;
	try {
		(void)properties::entropy(refused_space, exact_only);
	} catch (const mtrc::RepresentationError &error) {
		refused = std::string(error.what()).find("entropy refused exact default work") != std::string::npos;
	}
	assert(refused);
	assert(*refused_metric.calls == 0);

	CountingCheb sampled_metric;
	auto sampled_space = mtrc::make_space(large_records, sampled_metric);
	properties::entropy_options sampled;
	sampled.neighbor_count = 3;
	sampled.approximation_order = 2;
	sampled.max_exact_records = 8;
	sampled.sample_count = 12;
	sampled.sample_seed = 5;
	const auto sampled_entropy = properties::entropy(sampled_space, sampled);
	assert(sampled_entropy.status == mtrc::entropy_status::valid);
	assert(!sampled_entropy.exact);
	assert(sampled_entropy.representation == "sampled_metric_space");
	assert(sampled_entropy.record_count == sampled_space.size());
	assert(sampled_entropy.sample_count == sampled.sample_count);
	assert(sampled_entropy.sample_seed == sampled.sample_seed);
	assert(!sampled_entropy.approximation_reason.empty());
	assert(*sampled_metric.calls > 0);
	assert(*sampled_metric.calls < sampled_space.size() * sampled_space.size());
	assert(mtrc::summary(sampled_entropy).find("sample_count=12") != std::string::npos);

	return 0;
}
