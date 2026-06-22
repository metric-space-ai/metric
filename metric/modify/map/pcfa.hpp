// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MAPPINGS_PCFA_HPP
#define _METRIC_MAPPINGS_PCFA_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/record/id.hpp>
#include <metric/core/result.hpp>
#include <metric/metric/catalog/vector/Standards.hpp>
#include <metric/mapping/PCFA.hpp>

namespace mtrc::modify::map {

template <typename Record> using default_pcfa_metric_t = Euclidean<typename Record::value_type>;

template <typename Record, typename CodeMetric = default_pcfa_metric_t<Record>> class PCFAModel {
  public:
	using record_type = Record;
	using code_metric_type = CodeMetric;
	using legacy_model_type = mtrc::PCFA<record_type, void>;
	using space_type = MetricSpace<record_type, code_metric_type>;
	using result_type = MappingResult<space_type>;

	PCFAModel(legacy_model_type model, std::size_t component_count, code_metric_type code_metric = code_metric_type{})
		: model_(std::move(model)), component_count_(component_count), code_metric_(std::move(code_metric))
	{
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto transform(const Space &space) const -> result_type
	{
		static_assert(std::is_same<typename Space::record_type, record_type>::value,
					  "PCFAModel can only transform spaces with the fitted record type");

		auto encoded_records = model_.encode(space.records());
		space_type derived_space(encoded_records, code_metric_);
		auto lineage = mtrc::one_to_one_lineage(space);

		// PCFA is coordinate (dimension) reduction: it produces a derived
		// Euclidean coordinate space that approximates the source geometry. It is
		// not record-set compression and not record thinning. The code metric is
		// a true metric; out-of-sample transform is supported by the fitted model.
		return core::make_mapping_result(
			std::move(derived_space), std::move(lineage.source_records),
			std::move(lineage.representative_records), space.size(), true, "pcfa", "pcfa", "metric_space",
			core::metric_traits<code_metric_type>::law, true,
			"coordinate (dimension) reduction; derived Euclidean coordinate space approximating source geometry; "
			"not record compression");
	}

	auto inverse_transform(const space_type &derived_space, bool unshift = true) const -> std::vector<record_type>
	{
		return model_.decode(derived_space.records(), unshift);
	}

	auto inverse_transform(const result_type &result, bool unshift = true) const -> std::vector<record_type>
	{
		return inverse_transform(result.space, unshift);
	}

	auto component_count() const -> std::size_t { return component_count_; }
	auto legacy_model() const -> const legacy_model_type & { return model_; }

  private:
	mutable legacy_model_type model_;
	std::size_t component_count_{};
	code_metric_type code_metric_;
};

class PCFAMapping {
  public:
	explicit PCFAMapping(std::size_t component_count) : component_count_(component_count)
	{
		if (component_count_ == 0) {
			throw std::invalid_argument("PCFA component count must be positive");
		}
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto fit(const Space &space) const
		-> PCFAModel<typename Space::record_type, default_pcfa_metric_t<typename Space::record_type>>
	{
		if (space.empty()) {
			throw std::invalid_argument("cannot fit PCFA mapping on an empty space");
		}

		using record_type = typename Space::record_type;
		using model_type = PCFAModel<record_type, default_pcfa_metric_t<record_type>>;
		using legacy_model_type = typename model_type::legacy_model_type;

		legacy_model_type legacy_model(space.records(), component_count_);
		return model_type(std::move(legacy_model), component_count_);
	}

	auto component_count() const -> std::size_t { return component_count_; }

  private:
	std::size_t component_count_{};
};

inline auto pcfa(std::size_t component_count) -> PCFAMapping { return PCFAMapping(component_count); }

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto pcfa_space(const Space &space, std::size_t component_count) ->
	typename PCFAModel<typename Space::record_type, default_pcfa_metric_t<typename Space::record_type>>::result_type
{
	auto mapping = pcfa(component_count);
	auto model = mapping.fit(space);
	return model.transform(space);
}

} // namespace mtrc::modify::map

#endif
