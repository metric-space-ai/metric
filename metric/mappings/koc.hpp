// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MAPPINGS_KOC_HPP
#define _METRIC_MAPPINGS_KOC_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>

#include "../core/concepts.hpp"
#include "../core/result.hpp"

namespace metric::mappings {

namespace detail {

[[noreturn]] inline auto throw_koc_unavailable() -> void
{
	throw std::invalid_argument("KOC mapping is not promoted into the engine mapping contract yet");
}

} // namespace detail

template <typename Space> class KOCModel {
  public:
	using space_type = Space;
	using result_type = MappingResult<space_type>;

	explicit KOCModel(std::size_t clusters)
		: clusters_(clusters)
	{
	}

	auto transform(const Space &) const -> result_type
	{
		detail::throw_koc_unavailable();
	}

	auto inverse_supported() const -> bool { return false; }
	auto clusters() const -> std::size_t { return clusters_; }

  private:
	std::size_t clusters_{};
};

class KOCMapping {
  public:
	explicit KOCMapping(std::size_t clusters)
		: clusters_(clusters)
	{
		if (clusters_ == 0) {
			throw std::invalid_argument("KOC cluster count must be positive");
		}
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto fit(const Space &) const -> KOCModel<Space>
	{
		detail::throw_koc_unavailable();
	}

	auto clusters() const -> std::size_t { return clusters_; }

  private:
	std::size_t clusters_{};
};

inline auto koc(std::size_t clusters) -> KOCMapping
{
	return KOCMapping(clusters);
}

} // namespace metric::mappings

#endif
