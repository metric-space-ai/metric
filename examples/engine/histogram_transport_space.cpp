#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <metric/engine.hpp>

namespace {

using Histogram = std::vector<double>;

struct CumulativeTransportDistance {
    auto operator()(const Histogram &lhs, const Histogram &rhs) const -> double
    {
        if (lhs.size() != rhs.size()) {
            throw std::invalid_argument("histograms must have the same number of bins");
        }

        double cumulative_delta = 0.0;
        double distance = 0.0;
        for (std::size_t index = 0; index < lhs.size(); ++index) {
            cumulative_delta += lhs[index] - rhs[index];
            distance += std::abs(cumulative_delta);
        }
        return distance;
    }
};

auto close_to(double lhs, double rhs) -> bool
{
    return std::abs(lhs - rhs) < 1e-12;
}

} // namespace

int main()
{
    const std::vector<Histogram> records = {
        {1.0, 0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 1.0},
        {0.5, 0.5, 0.0, 0.0},
        {0.0, 0.5, 0.5, 0.0},
    };

    auto space = metric::make_space(records, CumulativeTransportDistance{});

    metric::representations::MatrixCache<decltype(space)> matrix(space);
    assert(close_to(matrix.distance(space.id(0), space.id(1)), 1.0));
    assert(close_to(matrix.distance(space.id(0), space.id(2)), 3.0));
    assert(close_to(matrix.distance(space.id(3), space.id(4)), 1.0));

    const Histogram query = {0.25, 0.75, 0.0, 0.0};
    const auto neighbors = metric::find_neighbors(space, query, 2);
    assert(neighbors.size() == 2);
    assert(neighbors[0].id == space.id(1));
    assert(close_to(neighbors[0].distance, 0.25));
    assert(neighbors[1].id == space.id(3));
    assert(close_to(neighbors[1].distance, 0.25));

    const auto materialized_policy = metric::runtime::materialized(metric::runtime::exact());
    const auto materialized_neighbors = metric::find_neighbors(space, space.id(3), 2, materialized_policy);
    assert(materialized_neighbors.representation == "matrix_cache");
    assert(materialized_neighbors.size() == 2);

    const auto groups = metric::find_groups(space, metric::strategies::k_medoids(2));
    assert(groups.algorithm == "kmedoids");
    assert(groups.cluster_count == 2);

    std::cout << "nearest histograms = " << neighbors[0].id.index() << ", " << neighbors[1].id.index() << "\n";
    std::cout << "histogram runtime representation = " << materialized_neighbors.representation << "\n";

    return 0;
}
