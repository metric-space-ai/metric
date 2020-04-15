#ifndef _METRIC_FRAMEWORK_CPP
#define _METRIC_FRAMEWORK_CPP

#include <cmath>
#include <vector>
//#include <map>

#include "3rdparty/blaze/Blaze.h"


namespace metric {




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
struct determine_val_t  // determines type of element both for STL containers and Blaze vectors
{
    using type = void;
};

template<typename C>
struct determine_val_t<C, 1>
{
    using type = typename C::value_type;
};

template<typename C>
struct determine_val_t<C, 2>
{
    using type = typename C::ElementType;
};

template<typename C>
struct determine_val_t<C, 3>
{
    using type = typename C::Scalar;
};



// --------
// base classes


template <typename rec_t>
class MetricBase { // all metrics to be derived from

public:

    using Rec_T = rec_t;
    using Val_T = determine_val_t<Rec_T>;

    MetricBase<rec_t>() {}

    double operator () (const rec_t & a, const rec_t & b) { // to be overridden in derived metrics
        return 0;
    }

private:

};




template <typename Metric = MetricBase<double>>
class Algorithm {

public:

    //using Rec_T = rec_t;
    //using Val_T = determine_val_t<Rec_T>;

    Algorithm<Metric>(Metric metric_) : metric(metric_) {}

    typename Metric::Rec_T process_single(typename Metric::Rec_T in) {
        return in;
    }

    template <typename set_t>
    set_t process_batch(set_t in) {
        return in;
    }



    template <typename set_t_l>
    typename std::enable_if <
     std::is_same<typename determine_val_t<set_t_l>::type, typename Metric::Rec_T>::value, // set_t matches with val_t
     set_t_l
    >::type
    sort_by_distance(set_t_l in, typename Metric::Rec_T center) {

        set_t_l out;

        auto cmp = [](std::pair<size_t, double> const & a, std::pair<size_t, double> const & b)
        {
            return a.second != b.second ? a.second < b.second : a.first < b.first;
        };

        std::vector<std::pair<size_t, double>> distances;
        for (size_t i = 0; i<in.size(); i++) {
            distances.push_back(std::make_pair(i, metric(in[i], center)));
        }
        sort(distances.begin(), distances.end(), cmp);
        for (std::vector<std::pair<size_t, double>>::iterator i = distances.begin(); i != distances.end(); i++) {
            out.push_back(in[i->first]);
        }
        return out;
    }


    /* // STL and Blaze versions apart, not needed in this case
    template <typename set_t_l>
    typename std::enable_if <
     std::is_same<typename determine_val_t<set_t_l>::type, typename Metric::Rec_T>::value
       && determine_container_type<typename determine_val_t<set_t_l>::type>::code == 1, // STL
     set_t_l
    >::type
    sort_by_distance(set_t_l in, typename Metric::Rec_T center) {

        set_t_l out;

        auto cmp = [](std::pair<size_t, double> const & a, std::pair<size_t, double> const & b)
        {
            return a.second != b.second ? a.second < b.second : a.first < b.first;
        };

        std::vector<std::pair<size_t, double>> distances;
        for (size_t i = 0; i<in.size(); i++) {
            distances.push_back(std::make_pair(i, metric(in[i], center)));
        }
        sort(distances.begin(), distances.end(), cmp);
        for (std::vector<std::pair<size_t, double>>::iterator i = distances.begin(); i != distances.end(); i++) {
            out.push_back(in[i->first]);
        }
        return out;
    }


    template <typename set_t_l>
    typename std::enable_if <
     std::is_same<typename determine_val_t<set_t_l>::type, typename Metric::Rec_T>::value
       && determine_container_type<typename determine_val_t<set_t_l>::type>::code == 2, // Blaze
     set_t_l
    >::type
    sort_by_distance(set_t_l in, typename Metric::Rec_T center) { // only STP version now

        set_t_l out (in.size());

        auto cmp = [](std::pair<size_t, double> const & a, std::pair<size_t, double> const & b)
        {
            return a.second != b.second ? a.second < b.second : a.first < b.first;
        };

        std::vector<std::pair<size_t, double>> distances;
        for (size_t i = 0; i<in.size(); i++) {
            distances.push_back(std::make_pair(i, metric(in[i], center)));
        }
        sort(distances.begin(), distances.end(), cmp);
        for (size_t i = 0; i<distances.size(); i++) {
            out[i] = in[distances[i].first];
        }
        return out;
    }
    // */


protected:

    Metric metric;
};




// --------
// derived classes


template <typename rec_t>
class EuclideanNoIter : public MetricBase<rec_t> {

public:

    double operator()(const rec_t & a, const rec_t & b) const  { // siuts both Blaze and STL
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



template <typename rec_t>
class EuclideanTypeSpecific : public MetricBase<rec_t> {

public:

    template <typename rec_t_l>
    typename std::enable_if <
     std::is_same<rec_t_l, rec_t>::value && determine_container_type<rec_t_l>::code == 1, // STL, we can use iterators
     double
    >::type
    operator()(const rec_t_l & a, const rec_t_l & b) const { // STL overload, TODO add Blaze overload
        if (a.size() != b.size()) {
            double dnan = std::nan("Input container sizes do not match");
            return dnan;
        }
        double sum = 0;
        for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
            sum += (*it1 - *it2) * (*it1 - *it2);
        }
        return std::sqrt(sum);
    }

    template <typename rec_t_l>
    typename std::enable_if <
     std::is_same<rec_t_l, rec_t>::value && determine_container_type<rec_t_l>::code == 2, // Blaze, we can use Blaze matrx operations
     double
    >::type
    operator()(const rec_t_l & a, const rec_t_l & b) const { // STL overload, TODO add Blaze overload
        if (a.size() != b.size()) {
            double dnan = std::nan("Input container sizes do not match");
            return dnan;
        }
        auto diff = a - b;
        double sum = blaze::sum(diff*diff);
        return std::sqrt(sum);
    }

    template <typename rec_t_l>
    typename std::enable_if <
     std::is_same<rec_t_l, rec_t>::value && determine_container_type<rec_t_l>::code == 3, // Eigen, [] to access elements (or we can use Eigen-specific matrix operations)
     double
    >::type
    operator()(const rec_t_l & a, const rec_t_l & b) const {
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


//template <typename rec_t> class Euclidean {};


template <typename rec_t>
class Euclidean : public MetricBase<rec_t> {

public:

    // STL-specific, not needed in this case
//    template <typename rec_t_l>
//    typename std::enable_if <
//     std::is_same<rec_t_l, rec_t>::value && determine_container_type<rec_t_l>::code == 1, // STL
//     double
//    >::type
//    operator()(const rec_t_l & a, const rec_t_l & b) const { // STL overload, TODO add Blaze overload
//        double sum = 0;
//        for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
//            sum += (*it1 - *it2) * (*it1 - *it2);
//        }
//        return std::sqrt(sum);
//    }


    // Blaze-specific code, not needed in thios case
//    template <typename rec_t_l>
//    typename std::enable_if <
//     std::is_same<rec_t_l, rec_t>::value && determine_container_type<rec_t_l>::code == 2, // Blaze
//     double
//    >::type
//    operator()(const rec_t_l & a, const rec_t_l & b) const { // STL overload, TODO add Blaze overload
//        auto diff = a - b;
//        double sum = blaze::sum(diff*diff);
//        return std::sqrt(sum);
//    }


    double operator()(const rec_t & a, const rec_t & b) const  { // siuts both Blaze and STL
        double sum = 0;
        for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
            sum += (*it1 - *it2) * (*it1 - *it2);
        }
        return std::sqrt(sum);
    }


};






template <typename Metric = Euclidean<std::vector<double>>>
class Norm : public Algorithm<Metric> {

public:

    Norm(Metric metric_) : Algorithm<Metric>(metric_) {};

    template <typename rec_t_l>
    typename std::enable_if <
     std::is_same<rec_t_l, typename Metric::Rec_T>::value && determine_container_type<rec_t_l>::code == 1, // STL
     double
    >::type
    operator()(const rec_t_l & in) {
        typename Metric::Rec_T zero;
        for (size_t i = 0; i<in.size(); ++i) {
            zero.push_back(0); // STL-specific
        }
        return this->metric(in, zero);
    }


    template <typename rec_t_l>
    typename std::enable_if <
     std::is_same<rec_t_l, typename Metric::Rec_T>::value && determine_container_type<rec_t_l>::code == 2, // Blaze
     double
    >::type
    operator()(const rec_t_l & in) {
        typename Metric::Rec_T zero (in.size(), 0); // Blaze-specific
        return this->metric(in, zero);
    }


    template <typename set_t_l>
    std::vector<double> norm_batch(set_t_l in) {
        std::vector<double> out (in.size());
        for  (size_t i = 0; i<in.size(); ++i) {
            out[i] = this->operator()(in[i]);
        }
        return out;
    }


//    template <typename set_t>
//    typename std::enable_if <
//     determine_container_type<set_t>::code == 1, // STL
//     std::vector<double>
//    >::type
//    norm_batch(set_t & in) {
//        std::vector<double> out;
//        for (size_t i = 0; i<in.size(); ++i) {
//            out.push_back(this->operator()(in[i]));
//        }
//        return out;
//    }


};


} // namespace metric

#endif
