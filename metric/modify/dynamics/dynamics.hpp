// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// mtrc::modify::dynamics evolves a finite metric space through a diffusion /
// Markov process built from the space's own metric.
//
// diffuse() constructs the affinity graph induced by the source metric, turns it
// into a row-stochastic (Markov) transition operator, raises it to the requested
// number of diffusion steps, and applies it to the record coordinates. The result
// is a derived finite metric space of diffusion-smoothed records under the source
// metric (so the metric law is preserved). This is the forward, native, fully
// deterministic counterpart to the legacy reverse-diffusion mapper
// (metric/mapping/Redif.hpp, quarantined): it depends only on mtrc::numeric and
// carries lineage, metric status and validity bounds.
//
// The operation is promoted only for floating-point vector records, where
// diffusion of coordinates is well defined; other record domains throw
// StrategyUnavailableError rather than producing meaningless output.

#ifndef _METRIC_INTENT_DYNAMICS_HPP
#define _METRIC_INTENT_DYNAMICS_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/numeric/Math.h>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>
#include <metric/space/storage/distance_matrix.hpp>

namespace mtrc::modify::dynamics {

// Diffusion is promoted for floating-point vector records: value_type, size(),
// indexing, range reconstruction, and a continuous coordinate domain.
template <typename Record, typename = void> struct supports_diffusion : std::false_type {};

template <typename Record>
struct supports_diffusion<
	Record, std::void_t<typename Record::value_type, decltype(std::declval<const Record &>().size()),
						 decltype(std::declval<const Record &>()[std::size_t{}]),
						 decltype(Record(std::declval<const typename Record::value_type *>(),
										 std::declval<const typename Record::value_type *>()))>>
	: std::is_floating_point<typename Record::value_type> {};

template <typename Record> constexpr bool supports_diffusion_v = supports_diffusion<Record>::value;

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto diffuse(const Space &space, std::size_t steps, double kernel_scale = 0.0, bool lazy = false)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using record_type = typename Space::record_type;
	using target_space_type = MetricSpace<record_type, typename Space::metric_type>;

	if constexpr (supports_diffusion_v<record_type>) {
		using coord_type = typename record_type::value_type;

		if (space.empty()) {
			throw MetricInputError("modify::dynamics::diffuse requires a non-empty space");
		}
		if (steps == 0) {
			throw MetricInputError("modify::dynamics::diffuse requires a positive number of diffusion steps");
		}

		const auto count = space.size();
		const auto dimension = space.records().front().size();
		for (std::size_t index = 0; index < count; ++index) {
			if (space.records()[index].size() != dimension) {
				throw MetricInputError("modify::dynamics::diffuse requires records of uniform dimension");
			}
		}

		auto distances = space::storage::metric_space_dense_distance_matrix<coord_type>(space);
		const coord_type scale =
			kernel_scale > 0.0 ? static_cast<coord_type>(kernel_scale) : numeric::positive_mean_or(distances, coord_type(1));

		numeric::DynamicMatrix<coord_type> transition = numeric::map(distances, [&scale](coord_type distance) {
			const auto scaled = distance / scale;
			return static_cast<coord_type>(std::exp(-(scaled * scaled)));
		});
		numeric::normalize_rows(transition);
		if (lazy) {
			numeric::blend_with_identity(transition, coord_type(0.5));
		}

		const auto diffused = numeric::matrix_power(transition, steps);
		const auto coordinates = numeric::row_vectors_to_matrix<coord_type>(space.records());
		const numeric::DynamicMatrix<coord_type> smoothed = diffused * coordinates;
		const auto smoothed_rows = numeric::matrix_to_row_vectors(smoothed);

		std::vector<record_type> records;
		records.reserve(count);
		for (const auto &row : smoothed_rows) {
			records.push_back(record_type(row.data(), row.data() + row.size()));
		}

		auto lineage = ::mtrc::one_to_one_lineage(space);
		target_space_type derived_space(std::move(records), space.metric());
		return core::make_mapping_result(
			std::move(derived_space), std::move(lineage.source_records), std::move(lineage.representative_records),
			space.size(), false, "diffusion", lazy ? "lazy_markov_diffusion" : "markov_diffusion", "metric_space",
			core::metric_traits<typename Space::metric_type>::law, false,
			"diffusion-evolved finite metric space; Markov diffusion over the affinity graph induced by the source "
			"metric; records remain in the source domain under the source metric; in-sample only; not invertible");
	} else {
		throw StrategyUnavailableError(
			"modify::dynamics::diffuse is only promoted for floating-point vector records; this record domain has no "
			"diffusion contract");
	}
}

} // namespace mtrc::modify::dynamics

namespace mtrc {
using modify::dynamics::diffuse;
} // namespace mtrc

#endif
