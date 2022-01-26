#include "metric/utils/graph.hpp"
#include "metric/distance/k-related/Standards.hpp"
#include "metric/distance/k-structured/EMD.hpp"

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>

#include <typeindex>
#include <string>
#include <unordered_map>

namespace metric {

// define list of metric and their names
using MetricTypes = boost::mpl::vector<
    metric::Euclidean<double>
    , metric::Manhatten<double>
    , metric::Chebyshev<double>
    , metric::P_norm<double>
>;

std::unordered_map<std::type_index, std::string> typeNames = {
    {std::type_index(typeid(metric::Euclidean<double>)), "Euclidean"},
    {std::type_index(typeid(metric::Euclidean_thresholded<double>)), "Euclidean_thresholded"},
    {std::type_index(typeid(metric::Manhatten<double>)), "Manhatten"},
    {std::type_index(typeid(metric::Chebyshev<double>)), "Chebyshev"},
    {std::type_index(typeid(metric::P_norm<double>)), "Pnorm"},
    {std::type_index(typeid(metric::EMD<double>)), "EMD"},
    {std::type_index(typeid(metric::Grid4)), "Grid4"},
    {std::type_index(typeid(metric::Grid6)), "Grid6"},
    {std::type_index(typeid(metric::Grid8)), "Grid8"},
    {std::type_index(typeid(metric::Grid8)), "Grid8"}
};

template<typename Type>
std::string getTypeName() {
    auto it = typeNames.find(std::type_index(typeid(Type)));
    if (it != typeNames.end()) {
        return it->second;
    }
    const std::string name = typeid(Type).name();
    if (name.find("function") != std::string::npos) {
        return "Generic";
    }

    return name;
}

} // metric