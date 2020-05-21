#pragma once

#include <functional>
#include <optional>

namespace metric {


template <typename Container, typename Value = double>
class PythonMetric : public std::function<Value(const Container&, const Container&)> {
public:
    using Callable = std::function<Value(const Container&, const Container&)>;
    using value_type = Value;
    using distance_type = value_type;

    using Callable::Callable;
};

// this was a good idea, but not worth the efforts and not transparently convertible
//template <typename Container, typename V = double>
//class CustomMetric {
//public:
//    using value_type = V;
//    using distance_type = value_type;
//
//    using ScalarFunc = std::function<distance_type(const V&, const V&)>;
//    using VectorFunc = std::function<distance_type(const Container&, const Container&)>;
//
//    CustomMetric(const VectorFunc& vectorDistance, std::optional<ScalarFunc> vectorDistance)
//        : scalarDistance(scalarDistance), vectorDistance(vectorDistance.value_or(nullptr))
//    {
//    }
    // one can always express scalar distance as distance between vectors with single item
//    distance_type operator()(const V& a, const V& b) const
//    {
//        return this->scalarDistance(a, b);
//    }
//
//    distance_type operator()(const Container& a, const Container& b) const
//    {
//        return this->vectorDistance(a, b);
//    }
//
//private:
//    ScalarFunc scalarDistance;
//    VectorFunc vectorDistance;
//};

} // metric
