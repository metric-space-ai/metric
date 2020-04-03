#include "modules/utils/graph.hpp"
#include "modules/distance/k-related/Standards.hpp"
#include <boost/mpl/vector.hpp>
#include <typeindex>
#include <vector>
#include <string>
#include <unordered_map>

// define list of metric and their names
using MetricTypes = boost::mpl::vector<
    metric::Euclidian<double>
    , metric::Manhatten<double>
    //, metric::Chebyshev<double>
    , metric::P_norm<double> // TODO: constructor argument
>;

std::unordered_map<std::type_index, std::string> metricTypeNames = {
    {std::type_index(typeid(metric::Euclidian<double>)), "Euclidean"},
    {std::type_index(typeid(metric::Manhatten<double>)), "Manhatten"},
    {std::type_index(typeid(metric::Chebyshev<double>)), "Chebyshev"},
    {std::type_index(typeid(metric::P_norm<double>)), "Pnorm"}
};

template<typename Metric>
std::string getMetricName() {
    return metricTypeNames[std::type_index(typeid(Metric))];
}

std::unordered_map<std::type_index, std::string> graphTypeNames = {
    {std::type_index(typeid(metric::Grid4)), "Grid4"},
    {std::type_index(typeid(metric::Grid6)), "Grid6"},
    {std::type_index(typeid(metric::Grid8)), "Grid8"}
};
template<typename Graph>
std::string getGraphName() {
    return graphTypeNames.at(std::type_index(typeid(Graph)));
}

template<typename Type>
std::string getSimpleType() {
    return typeid(Type).name();
}

template<class T>
struct MPLHelpType
{
    typedef T argument_type;
};
