/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/

#ifndef _METRIC_DISTANCE_K_STRUCTURED_EDIT_HPP
#define _METRIC_DISTANCE_K_STRUCTURED_EDIT_HPP
#include <string_view>
namespace metric {

/**
 * @class Edit
 * @breaf Edit distance(for strings)
 * @tparam
 */
template <typename V>
struct Edit {
    using value_type = V;
    using distance_type = int;

    /**
     * @brief Calculate Edit distance between two STL-like containers
     *
     * @tparam Container
     * @param str1
     * @param str2
     * @return Edit distance between str1 and str2
     */
    template <typename Container>
    distance_type operator()(const Container& str1, const Container& str2) const;

    /**
     * @brief calculate Edit distance for null terminated strings
     *
     * @param str1
     * @param str2
     * @return Edit distance between str1 and str2
     */
    distance_type operator()(const V* str1, const V* str2) const
    {
        return this->operator()(std::basic_string_view<V>(str1), std::basic_string_view<V>(str2));
    }
};

}  // namespace metric

#include "Edit.cpp"

#endif  // Header Guard
