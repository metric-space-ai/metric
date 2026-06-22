#include "metric/metric/catalog/vector/Standards.hpp"
#include "metric/metric/catalog/structured/EMD.hpp"
#include "metric/utils/graph.hpp"

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>

#include <string>
#include <typeindex>
#include <unordered_map>

namespace mtrc {

// define list of metric and their names
using MetricTypes = boost::mpl::vector<mtrc::Euclidean<double>, mtrc::Manhattan<double>, mtrc::Chebyshev<double>,
									   mtrc::P_norm<double>>;

std::unordered_map<std::type_index, std::string> typeNames = {
	{std::type_index(typeid(mtrc::Euclidean<double>)), "Euclidean"},
	{std::type_index(typeid(mtrc::Euclidean_thresholded<double>)), "Euclidean_thresholded"},
	{std::type_index(typeid(mtrc::Manhattan<double>)), "Manhattan"},
	{std::type_index(typeid(mtrc::Chebyshev<double>)), "Chebyshev"},
	{std::type_index(typeid(mtrc::P_norm<double>)), "Pnorm"},
	{std::type_index(typeid(mtrc::EMD<double>)), "EMD"},
	{std::type_index(typeid(mtrc::Grid4)), "Grid4"},
	{std::type_index(typeid(mtrc::Grid6)), "Grid6"},
	{std::type_index(typeid(mtrc::Grid8)), "Grid8"},
	{std::type_index(typeid(mtrc::Grid8)), "Grid8"}};

template <typename Type> std::string getTypeName()
{
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

} // namespace mtrc