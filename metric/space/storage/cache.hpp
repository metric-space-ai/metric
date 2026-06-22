// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RUNTIME_CACHE_HPP
#define _METRIC_RUNTIME_CACHE_HPP

#include <cstddef>
#include <utility>

namespace mtrc::space::storage {

template <typename Representation> class cached_representation {
  public:
	using representation_type = Representation;

	explicit cached_representation(representation_type representation) : representation_(std::move(representation)) {}

	auto get() -> representation_type & { return representation_; }
	auto get() const -> const representation_type & { return representation_; }
	auto version() const -> std::size_t { return representation_.version(); }
	auto is_stale() const -> bool { return representation_.is_stale(); }

  private:
	representation_type representation_;
};

template <typename Representation> auto cache(Representation representation) -> cached_representation<Representation>
{
	return cached_representation<Representation>(std::move(representation));
}

} // namespace mtrc::space::storage

#endif
