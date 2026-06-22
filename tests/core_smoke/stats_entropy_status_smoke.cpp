// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Entropy result status. Entropy is a PROPERTY of a finite metric space (a kpN
// local-Gaussian differential entropy estimator), not a metric. The EntropyResult now
// carries an explicit status so callers can distinguish a valid estimate (including a
// valid NEGATIVE differential entropy) from too_few_records / degenerate failures, and
// reports the EFFECTIVE (clamped) neighbor count and approximation order. LAPACK-gated
// because the estimator uses the LAPACK-backed numeric core.

#include <cassert>
#include <cmath>
#include <vector>

#include "metric/metric/catalog.hpp"
#include "metric/core/metric_space.hpp"
#include "metric/stats/properties/entropy.hpp"

namespace {

using Rec = std::vector<double>;
using Cheb = mtrc::Chebyshev<double>;

auto close(double lhs, double rhs, double tolerance = 1.0e-6) -> bool { return std::abs(lhs - rhs) < tolerance; }

} // namespace

int main()
{
	namespace properties = mtrc::stats::properties;

	// Valid estimate. Four distinct 2-D records; the differential entropy here is NEGATIVE,
	// which is a VALID result, never a failure (regression anchor shared with the entropy
	// smokes). With requested (k=3, p=2) on n=4 the estimator clamps to (k=2, p=3).
	const std::vector<Rec> base = {{5.0, 5.0}, {2.0, 2.0}, {3.0, 3.0}, {5.0, 1.0}};
	const auto valid = properties::entropy(base, Cheb(), 3, 2);
	assert(valid.status == mtrc::entropy_status::valid);
	assert(valid.succeeded() && !valid.failed());
	assert(std::isfinite(valid.value) && valid.value < 0.0);
	assert(close(valid.value, -4.4489104772539489));
	assert(valid.neighbor_count == 3 && valid.approximation_order == 2);
	assert(valid.effective_neighbor_count == 2 && valid.effective_approximation_order == 3);

	// Too few records: n < 4 -> NaN sentinel, too_few_records, no estimation attempted.
	const std::vector<Rec> tiny = {{5.0, 5.0}, {2.0, 2.0}, {3.0, 3.0}};
	const auto too_few = properties::entropy(tiny, Cheb(), 3, 2);
	assert(too_few.status == mtrc::entropy_status::too_few_records);
	assert(too_few.failed());
	assert(std::isnan(too_few.value));
	assert(too_few.effective_neighbor_count == 0 && too_few.effective_approximation_order == 0);

	// Degenerate: a valid-sized but information-free space (all-identical records) yields the
	// NaN sentinel with status degenerate (distinct from too_few_records).
	const std::vector<Rec> identical = {{1.0, 1.0}, {1.0, 1.0}, {1.0, 1.0}, {1.0, 1.0}};
	const auto degenerate = properties::entropy(identical, Cheb(), 3, 2);
	assert(degenerate.status == mtrc::entropy_status::degenerate);
	assert(degenerate.failed());
	assert(std::isnan(degenerate.value));
	assert(degenerate.effective_neighbor_count == 2 && degenerate.effective_approximation_order == 3);

	// Space overload carries the status through and tags the representation.
	const auto space = mtrc::make_space(base, Cheb());
	const auto space_entropy = properties::entropy(space, 3, 2);
	assert(space_entropy.representation == "metric_space");
	assert(space_entropy.status == mtrc::entropy_status::valid);
	assert(close(space_entropy.value, valid.value));

	return 0;
}
