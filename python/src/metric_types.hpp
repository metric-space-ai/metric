#include <vector>
#include <string>
#include <boost/mpl/vector.hpp>
#include "modules/correlation.hpp"
#include "modules/distance.hpp"

// define list of metric and thier names
using MetricTypes = boost::mpl::vector<
    metric::Euclidian<double>
    , metric::Manhatten<double>
    //, metric::Chebyshev<double>
    , metric::P_norm<double> // TODO: constructor argument
>;

extern std::vector<std::string> MetricTypeNames;

template<class T>
struct MPLHelpType
{
    typedef T agrument_type;
};

