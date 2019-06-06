#ifndef _METRIC_DISTANCE_L1_HPP
#define _METRIC_DISTANCE_L1_HPP

namespace metric
{
namespace distance
{


template <typename V>
struct Sorensen
{
    using value_type = V;
    using distance_type = value_type;

    Sorensen() = default;

    template<typename Container>
    //typename std::enable_if<!std::is_same<Container, V>::value, distance_type>::type // compiled only if Container differs from V
    distance_type
    operator()(const Container &a, const Container &b) const;

//    distance_type
//    operator()(const V &a, const V &b) const;

};



} // namespace distance
} // namespace metric

#include "L1.cpp"

#endif // Header Guard
