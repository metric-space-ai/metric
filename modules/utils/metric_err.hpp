
#include <cmath>




template <typename T>
std::tuple<double, double> stats(std::vector<T> v) {

    size_t n = v.size();

    double mean = 0;
    for (size_t i = 0; i<n; ++i) {
        mean += v[i];
    }
    mean = mean/(double)n;

    double dev = 0;
    for (size_t i = 0; i<n; ++i) {
        double diff = v[i] - mean;
        dev += diff*diff;
    }
    dev = sqrt(dev);
    return std::make_tuple(mean, dev);
}




template <typename Metric, typename Container>
typename Metric::distance_type norm(Container obj) { // now support only STL containers and k-related metrics
    size_t len = obj.size();
    Container zero_obj = Container(len, 0);
    Metric metric;
    return metric(obj, zero_obj);
}



template <typename Metric, typename Container>
typename Metric::distance_type distance(Container obj1, Container obj2) {
    Metric metric;
    return metric(obj1, obj2);
}


template <typename Metric, typename Container>
typename Metric::distance_type normalized_error(Container original, Container predicted) {
    return distance<Metric>(original, predicted) / norm<Metric>(original);
} // correct only for vectors of equal size!!


template <typename Metric, typename Container, template <typename, typename> class OuterContainer, typename OuterAllocator>
std::vector<typename Metric::distance_type> normalized_errors(OuterContainer<Container, OuterAllocator> original, OuterContainer<Container, OuterAllocator> predicted) {

    std::vector<typename Metric::distance_type> errors;
    for (size_t i = 0; i<predicted.size(); ++i) {
        auto err = normalized_error<Metric>(original[i], predicted[i]);
        errors.push_back(err);
    }
    return errors;
} // correct only for vectors of equal size!!


template <typename Metric, typename Container, template <typename, typename> class OuterContainer, typename OuterAllocator>
std::tuple<double, double, double, double, double, double> normalized_err_stats(OuterContainer<Container, OuterAllocator> original, OuterContainer<Container, OuterAllocator> predicted) {

    std::vector<typename Metric::distance_type> norms;
    std::vector<typename Metric::distance_type> errors;
    std::vector<typename Metric::distance_type> normalized_errors;
    for (size_t i = 0; i<predicted.size(); ++i) {
        Container cropped_original(original[i].begin(), original[i].begin() + predicted[i].size());
        //auto norm_original = norm<Metric>(original[i]);
        auto norm_original = norm<Metric>(cropped_original);
        auto err = distance<Metric>(original[i], predicted[i]);
        norms.push_back(norm_original);
        errors.push_back(err);
        normalized_errors.push_back(err/norm_original);
    }

    // mean and stddev for each vector
    auto norm_stats = stats(norms);
    auto err_stats = stats(errors);
    auto n_err_stats = stats(normalized_errors);
    return std::make_tuple(
                std::get<0>(norm_stats), std::get<1>(norm_stats),
                std::get<0>(err_stats), std::get<1>(err_stats),
                std::get<0>(n_err_stats), std::get<1>(n_err_stats)
                );
}

