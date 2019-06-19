#ifndef _METRIC_DISTANCE_L1_HPP
#define _METRIC_DISTANCE_L1_HPP

#include "blaze/Blaze.h"


namespace metric
{
namespace distance
{


// the UNFINISHED code for L1 metrics from the review papaer


/*


template <typename V>
struct Sorensen_t  // version similar to the existing code
{
    using value_type = V;
    using distance_type = value_type;

    Sorensen_t() = default;

    template<typename Container>
    //typename std::enable_if<!std::is_same<Container, V>::value, distance_type>::type // compiled only if Container differs from V
    distance_type
    operator()(const Container &a, const Container &b) const;

    //distance_type
    //operator()(const V &a, const V &b) const;

};


struct Sorensen_s // version without type members
{
    Sorensen_s() = default;

    template<typename Container>
    typename Container::value_type operator()(const Container &a, const Container &b) const;
};



template <typename V>
struct Sorensen
{
    //using value_type = V;
    //using distance_type = value_type;

    Sorensen() = default;

    template <typename Container>
    V operator()(const Container &a, const Container &b) const;

    template <typename ValueType>
    V operator()(const blaze::CompressedVector<ValueType> &a, const blaze::CompressedVector<ValueType> &b) const;
};

*/


//template <template <typename> class Container, typename Value>
//double sorensen(Container<Value> &a, Container<Value> &b);

//template <template <typename, typename> class Container, typename Value, typename Allocator>
//double sorensen(Container<Value, Allocator> &a, Container<Value, Allocator> &b);


template <typename T>
double sorensen(T &a, T &b);


template <typename Value>
double sorensen(blaze::CompressedVector<Value> &a, blaze::CompressedVector<Value> &b);

} // namespace distance
} // namespace metric

#include "L1.cpp"

#endif // Header Guard
