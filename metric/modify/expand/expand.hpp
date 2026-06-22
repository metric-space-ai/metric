// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// mtrc::modify::expand performs interpolation-like record generation: it grows a
// finite metric space with synthetic records derived from the source records,
// where the record domain supports interpolation.
//
// This is kept strictly separate from reduce (record thinning), resample (uneven
// sampling correction) and represent (representative selection): expand ADDS
// records. The only promoted contract today is metric-midpoint interpolation for
// arithmetic vector records: for each source record and its nearest neighbour a
// synthetic midpoint record is created. For record domains without a defined
// interpolation contract the operation throws StrategyUnavailableError instead of
// silently fabricating records. Synthetic records have no source preimage, so the
// mapping is not invertible and is valid only within the span of source
// neighbours.

#ifndef _METRIC_MODIFY_EXPAND_HPP
#define _METRIC_MODIFY_EXPAND_HPP

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/errors.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/record/id.hpp>

namespace mtrc::modify::expand {

// A scalar admits a meaningful numeric midpoint: arithmetic, but not a character
// type or bool. Character vectors such as std::string are sequences whose true
// metric is edit-like, not coordinate averaging, so they have no midpoint
// interpolation contract here and must route to StrategyUnavailableError.
template <typename Scalar>
struct is_interpolatable_scalar
	: std::integral_constant<bool,
							 std::is_arithmetic<Scalar>::value &&
								 !std::is_same<typename std::remove_cv<Scalar>::type, bool>::value &&
								 !std::is_same<typename std::remove_cv<Scalar>::type, char>::value &&
								 !std::is_same<typename std::remove_cv<Scalar>::type, signed char>::value &&
								 !std::is_same<typename std::remove_cv<Scalar>::type, unsigned char>::value &&
								 !std::is_same<typename std::remove_cv<Scalar>::type, wchar_t>::value &&
								 !std::is_same<typename std::remove_cv<Scalar>::type, char16_t>::value &&
								 !std::is_same<typename std::remove_cv<Scalar>::type, char32_t>::value> {};

// A record supports midpoint interpolation when it is a numeric vector: it
// exposes value_type, size() and indexing, can be rebuilt from a range, and its
// element type admits a meaningful midpoint.
template <typename Record, typename = void> struct supports_interpolation : std::false_type {};

template <typename Record>
struct supports_interpolation<
	Record, std::void_t<typename Record::value_type, decltype(std::declval<const Record &>().size()),
						 decltype(std::declval<const Record &>()[std::size_t{}]),
						 decltype(Record(std::declval<const typename Record::value_type *>(),
										 std::declval<const typename Record::value_type *>()))>>
	: is_interpolatable_scalar<typename Record::value_type> {};

template <typename Record> constexpr bool supports_interpolation_v = supports_interpolation<Record>::value;

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto expand(const Space &space)
	-> MappingResult<MetricSpace<typename Space::record_type, typename Space::metric_type>>
{
	using record_type = typename Space::record_type;
	using target_space_type = MetricSpace<record_type, typename Space::metric_type>;

	if constexpr (supports_interpolation_v<record_type>) {
		using value_type = typename record_type::value_type;

		const auto count = space.size();
		std::vector<record_type> records;
		std::vector<std::vector<RecordId>> source_records;
		std::vector<RecordId> representative_records;

		records.reserve(count);
		source_records.reserve(count);
		representative_records.reserve(count);

		// Source records are kept unchanged with a one-to-one lineage.
		for (std::size_t index = 0; index < count; ++index) {
			records.push_back(space.records()[index]);
			source_records.push_back(std::vector<RecordId>{space.id(index)});
			representative_records.push_back(space.id(index));
		}

		// One synthetic midpoint per unique unordered nearest-neighbour pair.
		std::vector<std::vector<bool>> emitted(count, std::vector<bool>(count, false));
		for (std::size_t index = 0; index < count; ++index) {
			std::size_t nearest = count;
			typename Space::distance_type best{};
			for (std::size_t other = 0; other < count; ++other) {
				if (other == index) {
					continue;
				}
				const auto distance = space.distance(space.id(index), space.id(other));
				if (nearest == count || distance < best) {
					nearest = other;
					best = distance;
				}
			}
			if (nearest == count) {
				continue; // singleton space: no neighbour to interpolate against
			}

			const auto low = index < nearest ? index : nearest;
			const auto high = index < nearest ? nearest : index;
			if (emitted[low][high]) {
				continue;
			}
			emitted[low][high] = true;

			const auto &lhs = space.records()[low];
			const auto &rhs = space.records()[high];
			if (lhs.size() != rhs.size()) {
				throw MetricInputError("modify::expand cannot interpolate records of different dimension");
			}

			std::vector<value_type> midpoint(lhs.size());
			for (std::size_t component = 0; component < lhs.size(); ++component) {
				midpoint[component] = static_cast<value_type>((lhs[component] + rhs[component]) / value_type(2));
			}

			records.push_back(record_type(midpoint.data(), midpoint.data() + midpoint.size()));
			source_records.push_back(std::vector<RecordId>{space.id(low), space.id(high)});
			representative_records.push_back(space.id(low));
		}

		target_space_type derived_space(std::move(records), space.metric());
		return core::make_mapping_result(
			std::move(derived_space), std::move(source_records), std::move(representative_records), space.size(),
			false, "expand", "nearest_neighbor_midpoint", "metric_space",
			core::metric_traits<typename Space::metric_type>::law, false,
			"interpolation-like expansion; synthetic records are metric midpoints of source nearest-neighbour "
			"pairs; valid only within the span of source neighbours; synthetic records have no source preimage");
	} else {
		throw StrategyUnavailableError(
			"modify::expand has no interpolation contract for this record domain; expansion is only promoted for "
			"arithmetic vector records");
	}
}

} // namespace mtrc::modify::expand

namespace mtrc {
using modify::expand::expand;
} // namespace mtrc

#endif
