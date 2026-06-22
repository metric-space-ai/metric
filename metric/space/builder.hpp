// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_BUILDER_HPP
#define _METRIC_SPACE_BUILDER_HPP

#include <cmath>
#include <cstddef>
#include <iterator>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/errors.hpp>
#include <metric/core/metadata.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/record/id.hpp>

// Safer, staged construction of a finite metric space.
//
// mtrc::make_space(records, metric) is the minimal path: it always succeeds and never validates. The
// SpaceBuilder is the opt-in safer path for callers who want construction-time guarantees:
//
//   auto space = mtrc::space::SpaceBuilder<std::string, mtrc::Edit<char>>(mtrc::Edit<char>{})
//                    .add_all(records)
//                    .require_non_empty()
//                    .require_true_metric()
//                    .build();
//
// The builder validates only what the caller opts into (emptiness, duplicate records, true-metric
// admission), then returns a plain mtrc::core::MetricSpace or throws a clear MetricInputError. It is
// header-only and adds no metric math: validation is structural plus the compile-time metric_traits
// law check. Optional name/metadata is carried with build_described() using the core Metadata type.
namespace mtrc::space {

namespace builder_detail {

template <typename T, typename = void> struct is_equality_comparable : std::false_type {};

template <typename T>
struct is_equality_comparable<
	T, std::void_t<decltype(std::declval<const T &>() == std::declval<const T &>())>> : std::true_type {};

template <typename T> constexpr bool is_equality_comparable_v = is_equality_comparable<T>::value;

// Record exposes .size() (a container record, e.g. std::vector<double>): needed for
// the optional uniform-dimension check.
template <typename T, typename = void> struct has_size : std::false_type {};
template <typename T>
struct has_size<T, std::void_t<decltype(std::declval<const T &>().size())>> : std::true_type {};
template <typename T> constexpr bool has_size_v = has_size<T>::value;

// Record is an iterable of arithmetic elements (e.g. std::vector<double>): needed for
// the optional finiteness check.
template <typename T, typename = void> struct arithmetic_element : std::false_type {};
template <typename T>
struct arithmetic_element<T, std::void_t<decltype(*std::begin(std::declval<const T &>()))>>
	: std::is_arithmetic<std::remove_cv_t<std::remove_reference_t<decltype(*std::begin(std::declval<const T &>()))>>> {
};
template <typename T> constexpr bool arithmetic_element_v = arithmetic_element<T>::value;

} // namespace builder_detail

// A built finite metric space together with the metadata attached during construction (e.g. a name).
template <typename Record, typename Metric> struct DescribedSpace {
	core::MetricSpace<Record, Metric> space;
	core::Metadata metadata;
};

template <typename Record, typename Metric> class SpaceBuilder {
  public:
	using record_type = Record;
	using metric_type = Metric;
	using space_type = core::MetricSpace<Record, Metric>;

	SpaceBuilder() : metadata_(core::Metadata::object()) {}
	explicit SpaceBuilder(Metric metric) : metric_(std::move(metric)), metadata_(core::Metadata::object()) {}

	// Set or replace the metric the built space will use.
	auto with_metric(Metric metric) -> SpaceBuilder &
	{
		metric_ = std::move(metric);
		return *this;
	}

	auto add(Record record) -> SpaceBuilder &
	{
		records_.push_back(std::move(record));
		return *this;
	}

	template <typename Container> auto add_all(const Container &records) -> SpaceBuilder &
	{
		for (const auto &record : records) {
			records_.push_back(record_type(record));
		}
		return *this;
	}

	auto reserve(std::size_t capacity) -> SpaceBuilder &
	{
		records_.reserve(capacity);
		return *this;
	}

	// Opt into validation: refuse to build an empty space.
	auto require_non_empty(bool required = true) -> SpaceBuilder &
	{
		require_non_empty_ = required;
		return *this;
	}

	// Opt into validation: refuse duplicate records (requires an equality-comparable record type;
	// building with this enabled on a non-comparable record type throws a clear MetricInputError).
	auto require_unique_records(bool required = true) -> SpaceBuilder &
	{
		require_unique_records_ = required;
		return *this;
	}

	// Opt into validation: refuse a metric that is not admitted as a true metric
	// (metric_traits<Metric>::law != metric_law::metric).
	auto require_true_metric(bool required = true) -> SpaceBuilder &
	{
		require_true_metric_ = required;
		return *this;
	}

	// Opt into validation: every (container) record must have the same dimension.
	// Reports the offending record index and sizes on failure.
	auto require_uniform_dimension(bool required = true) -> SpaceBuilder &
	{
		require_uniform_dimension_ = required;
		return *this;
	}

	// Opt into validation: every record value must be finite (no NaN/Inf). The default
	// catalog vector metrics do NOT reject non-finite input, so this guards the
	// silent-wrong-knn footgun at construction time. Reports the offending record/position.
	auto require_finite(bool required = true) -> SpaceBuilder &
	{
		require_finite_ = required;
		return *this;
	}

	// Attach a human-readable name (stored under the "name" metadata key). Surfaced by build_described().
	auto named(std::string name) -> SpaceBuilder &
	{
		metadata_["name"] = core::Metadata(std::move(name));
		return *this;
	}

	// Attach an arbitrary metadata field. Surfaced by build_described().
	auto with_metadata(const std::string &key, core::Metadata value) -> SpaceBuilder &
	{
		metadata_[key] = std::move(value);
		return *this;
	}

	auto record_count() const -> std::size_t { return records_.size(); }
	auto empty() const -> bool { return records_.empty(); }
	auto metric() const -> const Metric & { return metric_; }
	auto metadata() const -> const core::Metadata & { return metadata_; }

	// Validate the opted-in rules and construct the space. Throws MetricInputError on a violated rule.
	auto build() const -> space_type
	{
		validate();
		return space_type(records_, metric_);
	}

	// Like build(), but also returns the attached name/metadata.
	auto build_described() const -> DescribedSpace<Record, Metric>
	{
		validate();
		return DescribedSpace<Record, Metric>{space_type(records_, metric_), metadata_};
	}

  private:
	auto validate() const -> void
	{
		if (require_non_empty_ && records_.empty()) {
			throw MetricInputError("SpaceBuilder requires at least one record (require_non_empty)");
		}
		if (require_true_metric_) {
			if constexpr (core::metric_traits<Metric>::law != core::metric_law::metric) {
				throw MetricInputError(
					"SpaceBuilder requires a true metric (metric_traits<Metric>::law == metric_law::metric)");
			}
		}
		if (require_uniform_dimension_) {
			if constexpr (builder_detail::has_size_v<Record>) {
				if (!records_.empty()) {
					const std::size_t dim = records_.front().size();
					for (std::size_t i = 0; i < records_.size(); ++i) {
						if (records_[i].size() != dim) {
							throw MetricInputError(
								"SpaceBuilder requires uniform record dimension (require_uniform_dimension): record " +
								std::to_string(i) + " has size " + std::to_string(records_[i].size()) + ", expected " +
								std::to_string(dim));
						}
					}
				}
			} else {
				throw MetricInputError("SpaceBuilder require_uniform_dimension needs a record type with size()");
			}
		}
		if (require_finite_) {
			if constexpr (builder_detail::arithmetic_element_v<Record>) {
				for (std::size_t i = 0; i < records_.size(); ++i) {
					std::size_t position = 0;
					for (const auto &value : records_[i]) {
						if (!std::isfinite(static_cast<double>(value))) {
							throw MetricInputError(
								"SpaceBuilder requires finite record values (require_finite): record " +
								std::to_string(i) + ", position " + std::to_string(position));
						}
						++position;
					}
				}
			} else {
				throw MetricInputError("SpaceBuilder require_finite needs a record type of arithmetic elements");
			}
		}
		if (require_unique_records_) {
			if constexpr (builder_detail::is_equality_comparable_v<Record>) {
				for (std::size_t lhs = 0; lhs < records_.size(); ++lhs) {
					for (std::size_t rhs = lhs + 1; rhs < records_.size(); ++rhs) {
						if (records_[lhs] == records_[rhs]) {
							throw MetricInputError("SpaceBuilder requires unique records (require_unique_records)");
						}
					}
				}
			} else {
				throw MetricInputError(
					"SpaceBuilder require_unique_records needs an equality-comparable record type");
			}
		}
	}

	std::vector<Record> records_;
	Metric metric_{};
	bool require_non_empty_{false};
	bool require_unique_records_{false};
	bool require_true_metric_{false};
	bool require_uniform_dimension_{false};
	bool require_finite_{false};
	core::Metadata metadata_;
};

// Construct a SpaceBuilder, deducing the metric type. The record type is given explicitly because the
// builder accumulates records after construction:
//   auto b = mtrc::space::space_builder<std::string>(mtrc::Edit<char>{});
template <typename Record, typename Metric>
auto space_builder(Metric metric) -> SpaceBuilder<Record, typename std::decay<Metric>::type>
{
	return SpaceBuilder<Record, typename std::decay<Metric>::type>(std::move(metric));
}

// Direct user-facing construction under the Level-1 `space` namespace. This is
// the practical counterpart to mtrc::make_space: same resulting object, but
// discoverable from the finite-space API surface.
template <typename Container, typename Metric>
auto build(const Container &records, Metric metric)
	-> core::MetricSpace<typename std::decay<typename Container::value_type>::type,
						 typename std::decay<Metric>::type>
{
	using record_type = typename std::decay<typename Container::value_type>::type;
	using metric_type = typename std::decay<Metric>::type;
	return SpaceBuilder<record_type, metric_type>(std::move(metric)).add_all(records).build();
}

// Checked construction for production workflows that should fail early on the
// common safety gates: a non-empty record set and a metric admitted as a true
// metric by metric_traits.
template <typename Container, typename Metric>
auto build_checked(const Container &records, Metric metric)
	-> core::MetricSpace<typename std::decay<typename Container::value_type>::type,
						 typename std::decay<Metric>::type>
{
	using record_type = typename std::decay<typename Container::value_type>::type;
	using metric_type = typename std::decay<Metric>::type;
	return SpaceBuilder<record_type, metric_type>(std::move(metric))
		.add_all(records)
		.require_non_empty()
		.require_true_metric()
		.build();
}

} // namespace mtrc::space

namespace mtrc {
using space::build;
using space::DescribedSpace;
using space::build_checked;
using space::space_builder;
using space::SpaceBuilder;
} // namespace mtrc

#endif
