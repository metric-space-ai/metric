// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STRATEGIES_REDUCTION_HPP
#define _METRIC_STRATEGIES_REDUCTION_HPP

#include <cstddef>
#include <stdexcept>

namespace metric::strategies {

struct pcfa {
	explicit pcfa(std::size_t components)
		: components(components)
	{
		if (components == 0) {
			throw std::invalid_argument("PCFA component count must be positive");
		}
	}

	std::size_t components{};
};

struct som {
	som(std::size_t width, std::size_t height)
		: width(width)
		, height(height)
	{
		if (width == 0 || height == 0) {
			throw std::invalid_argument("SOM grid dimensions must be positive");
		}
	}

	std::size_t width{};
	std::size_t height{};
};

struct koc {
	explicit koc(std::size_t clusters)
		: clusters(clusters)
	{
		if (clusters == 0) {
			throw std::invalid_argument("KOC cluster count must be positive");
		}
	}

	std::size_t clusters{};
};

struct dspcc {
	explicit dspcc(std::size_t components)
		: components(components)
	{
		if (components == 0) {
			throw std::invalid_argument("DSPCC component count must be positive");
		}
	}

	std::size_t components{};
};

} // namespace metric::strategies

#endif
