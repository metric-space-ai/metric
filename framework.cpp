#ifndef _METRIC_FRAMEWORK_CPP
#define _METRIC_FRAMEWORK_CPP

#include <cmath>
#include <vector>

#include "3rdparty/blaze/Blaze.h"


namespace metric {


// global setup

using DistanceType = double;




// --------
// common type trait metafunctions



template <typename>
struct determine_container_type  // checks whether container is STL container (1) or Blaze vector (2)
{
    constexpr static int code = 0;
};

template <template <typename, typename> class Container, typename ValueType, typename Allocator>
struct determine_container_type<Container<ValueType, Allocator>>
{
    constexpr static int code = 1; // STL
};

template <template <typename, bool> class Container, typename ValueType, bool F>
struct determine_container_type<Container<ValueType, F>>
{
    constexpr static int code = 2; // blaze::DynamicVector
};

template <template <typename, int, int, int, int, int> class Container, typename ValueType, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
struct determine_container_type<Container<ValueType, _Rows, _Cols, _Options, _MaxRows, _MaxCols>>
{
    constexpr static int code = 3; // Eigen::Array
};


template<typename C, int = determine_container_type<C>::code>
struct determine_ValueType  // determines type of element both for STL containers and Blaze vectors
{
    using type = void;
};

template<typename C>
struct determine_ValueType<C, 1>
{
    using type = typename C::value_type;
};

template<typename C>
struct determine_ValueType<C, 2>
{
    using type = typename C::ElementType;
};

template<typename C>
struct determine_ValueType<C, 3>
{
    using type = typename C::Scalar;
};



// --------
// base classes


template <typename RT>
class MetricBase { // all metrics to be derived from

public:

    using RecordType = RT;
    using ValueType = determine_ValueType<RecordType>;

    double operator () (const RecordType & a, const RecordType & b) { // to be overridden in derived metrics
        double dnan = std::nan("Base class, not intended to be used directly");
        return dnan;
    }

};




template <typename MT = MetricBase<std::vector<double>>>
class Algorithm {

public:

    //using RecordType = Metric::RecordType;
    //using ValueType = determine_ValueType<RecordType>;

    //using DistanceType = typename std::invoke_result<Metric, const typename Metric::RecordType &, const typename Metric::RecordType &>::type;
    //using DistanceType = typename std::invoke_result<Metric, typename Metric::RecordType, typename Metric::RecordType>::type;

    Algorithm<MT>(MT metric_ = MT()) : metric(metric_) {}

    template <typename ST>
    typename std::enable_if <
     std::is_same<typename determine_ValueType<ST>::type, typename MT::RecordType>::value, // set_t matches with ValueType
     ST
    >::type
    sort_by_distance(ST in, typename MT::RecordType center) {

        ST out;

        auto cmp = [](std::pair<size_t, DistanceType> const & a, std::pair<size_t, DistanceType> const & b)
        {
            return a.second != b.second ? a.second < b.second : a.first < b.first;
        };

        std::vector<std::pair<size_t, DistanceType>> distances;
        for (size_t i = 0; i<in.size(); i++) {
            distances.push_back(std::make_pair(i, metric(in[i], center)));
        }
        sort(distances.begin(), distances.end(), cmp);
        for (typename std::vector<std::pair<size_t, DistanceType>>::iterator i = distances.begin(); i != distances.end(); i++) {
            out.push_back(in[i->first]);
        }
        return out;
    }

protected:

    MT metric;
};




// --------
// derived classes


template <typename RT>
class EuclideanNoIter : public MetricBase<RT> {

public:

    double operator()(const RT & a, const RT & b) const  { // siuts both Blaze and STL
        double sum = 0;
        if (a.size() != b.size()) {
            double dnan = std::nan("Input container sizes do not match");
            return dnan;
        }
        for (size_t i = 0; i < a.size(); ++i) {
            sum += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return std::sqrt(sum);
    }

};



template <typename RT>
class EuclideanTypeSpecific : public MetricBase<RT> {

public:

    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value && determine_container_type<R>::code == 1, // STL, we can use iterators
     DistanceType
    >::type
    operator()(const R & a, const R & b) const { // STL overload, TODO add Blaze overload
        if (a.size() != b.size()) {
            DistanceType dnan = std::nan("Input container sizes do not match");
            return dnan;
        }
        DistanceType sum = 0;
        for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
            sum += (*it1 - *it2) * (*it1 - *it2);
        }
        return std::sqrt(sum);
    }

    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value && determine_container_type<R>::code == 2, // Blaze, we can use Blaze matrx operations
     DistanceType
    >::type
    operator()(const R & a, const R & b) const { // STL overload, TODO add Blaze overload
        if (a.size() != b.size()) {
            DistanceType dnan = std::nan("Input container sizes do not match");
            return dnan;
        }
        auto diff = a - b;
        DistanceType sum = blaze::sum(diff*diff);
        return std::sqrt(sum);
    }

    template <typename R>
    typename std::enable_if <
     std::is_same<R, RT>::value && determine_container_type<R>::code == 3, // Eigen, [] to access elements (or we can use Eigen-specific matrix operations)
     DistanceType
    >::type
    operator()(const R & a, const R & b) const {
        DistanceType sum = 0;
        if (a.size() != b.size()) {
            DistanceType dnan = std::nan("Input container sizes do not match");
            return dnan;
        }
        for (size_t i = 0; i < a.size(); ++i) {
            sum += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return std::sqrt(sum);
    }

};



template <typename RT = std::vector<double>>
class Euclidean : public MetricBase<RT> {

public:

    DistanceType operator()(const RT & a, const RT & b) const  { // suits both Blaze and STL, but not Eigen
        DistanceType sum = 0;
        for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
            sum += (*it1 - *it2) * (*it1 - *it2);
        }
        return std::sqrt(sum);
    }

};




template <typename MT = Euclidean<std::vector<double>>>
class Norm : public Algorithm<MT> {

public:

    Norm(MT metric_) : Algorithm<MT>(metric_) {};

    template <typename R>
    typename std::enable_if <
     std::is_same<R, typename MT::RecordType>::value && determine_container_type<R>::code == 1, // STL
     double
    >::type
    operator()(const R & in) {
        typename MT::RecordType zero;
        for (size_t i = 0; i<in.size(); ++i) {
            zero.push_back(0); // STL-specific
        }
        return this->metric(in, zero);
    }


    template <typename R>
    typename std::enable_if <
     std::is_same<R, typename MT::RecordType>::value && determine_container_type<R>::code == 2, // Blaze
     double
    >::type
    operator()(const R & in) {
        typename MT::RecordType zero (in.size(), 0); // Blaze-specific
        return this->metric(in, zero);
    }


    template <typename ST>
    std::vector<double> norm_batch(ST in) {
        std::vector<double> out (in.size());
        for  (size_t i = 0; i<in.size(); ++i) {
            out[i] = this->operator()(in[i]);
        }
        return out;
    }



    typename MT::RecordType process_single(typename MT::RecordType in) {
        return in;
    }

    template <typename ST>
    ST process_batch(ST in) {
        return in;
    }


};


} // namespace metric

#endif
