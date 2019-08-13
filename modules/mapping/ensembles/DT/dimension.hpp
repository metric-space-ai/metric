/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/

#ifndef _METRIC_MAPPING_ENSEMBLES_DT_DIMENSION_HPP
#define _METRIC_MAPPING_ENSEMBLES_DT_DIMENSION_HPP

#include <vector>
#include <variant>  // for DimensionSet
#include <type_traits>
#include <tuple>
#include <functional>

namespace metric {

/**
 * @class Dimension
 * 
 * @brief utility class for Metric decision tree constructing
 */
template <typename MetricType, typename Accessor>
class Dimension {
public:
    using InputValueType = typename MetricType::value_type;
    using ReturnValueType = typename MetricType::distance_type;
    MetricType DistanceFunctor;

    /**
 * @brief Construct a new Dimension object
 * 
 * @param accessor_ field accessor
 * @param m metric object
 */
    template <typename A, typename M>
    Dimension(A accessor_, const M& m)
        : DistanceFunctor(m)
        , accessor(accessor_)
    {
    }
    /**
     * @brief Calculate distance between fields in records
     * 
     * @param r1 data record 
     * @param r2 data record
     * @return distance between fileds in records r1 and r2
     */
    template <typename Record>
    ReturnValueType get_distance(const Record& r1, const Record& r2)
    {
        return DistanceFunctor(accessor(r1), accessor(r2));
    }

private:
    Accessor accessor;
};

/**
 * @brief function contructing Dimension boject in convenient way
 * 
 * @param m metric object
 * @param a field accessor
 * @return Dimension(a,m)
 */
template <typename Accessor, typename Metric>
inline auto make_dimension(Metric&& m, Accessor a) -> Dimension<Metric, Accessor>
{
    return Dimension<Metric, Accessor>(a, m);
}

}  // namespace metric

#endif  // DIMENSION_HPP
