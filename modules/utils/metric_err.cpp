


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
}


template <typename Metric, typename Container, template <typename, typename> class OuterContainer, typename OuterAllocator>
std::vector<typename Metric::distance_type> normalized_errors(OuterContainer<Container, OuterAllocator> original, OuterContainer<Container, OuterAllocator> predicted) {

    std::vector<typename Metric::distance_type> errors;
    for (size_t i = 0; i<predicted.size(); ++i) {
        auto err = normalized_error<Metric>(original[i], predicted[i]);
        errors.push_back(err);
    }
    return errors;
}


//template <typename Metric, typename Container, template <typename, typename> class OuterContainer, typename OuterAllocator>
//std::tuple<double, double, double, double, double, double> normalized_err_stats(OuterContainer<Container, OuterAllocator> original, OuterContainer<Container, OuterAllocator> predicted) {

//    std::vector<typename Metric::distance_type> norms;
//    std::vector<typename Metric::distance_type> errors;
//    std::vector<typename Metric::distance_type> normalized_errors;
//    for (size_t i = 0; i<predicted.size(); ++i) {
//        auto norm_original = norma<Metric>(original[i]);
//        auto err = distance<Metric>(original[i], predicted[i]);
//        norms.push_back(norm_original);
//        errors.push_back(err);
//        normalized_errors.push_back(err/norm_original);
//    }

//    // TODO compute and return mean and stddev for each vector;
//}




