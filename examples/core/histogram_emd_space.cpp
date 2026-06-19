#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include <metric/distance.hpp>
#include <metric/space.hpp>

namespace {

using Histogram = std::vector<double>;

auto close_to(double lhs, double rhs) -> bool
{
    return std::fabs(lhs - rhs) < 1e-5;
}

} // namespace

int main()
{
    const std::vector<std::vector<double>> ground_cost = {
        {0.0, 1.0, 2.0, 3.0},
        {1.0, 0.0, 1.0, 2.0},
        {2.0, 1.0, 0.0, 1.0},
        {3.0, 2.0, 1.0, 0.0},
    };

    metric::EMD<double> emd(ground_cost, 3.0);

    const std::vector<Histogram> records = {
        {1.0, 0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 1.0},
        {0.5, 0.5, 0.0, 0.0},
        {0.0, 0.5, 0.5, 0.0},
    };

    metric::MatrixSpace<Histogram, metric::EMD<double>> space(records, emd);

    assert(space.size() == records.size());
    assert(close_to(space(0, 1), 1.0));
    assert(close_to(space(0, 2), 3.0));
    assert(close_to(space(3, 4), 1.0));

    const Histogram query = {0.25, 0.75, 0.0, 0.0};
    const auto nearest = space.knn(query, 2);
    assert(nearest.size() == 2);
    assert(close_to(nearest[0].second, 0.25));
    assert(close_to(nearest[1].second, 0.25));

    std::vector<std::size_t> nearest_ids = {nearest[0].first, nearest[1].first};
    std::sort(nearest_ids.begin(), nearest_ids.end());
    assert((nearest_ids == std::vector<std::size_t>{1, 3}));

    std::cout << "nearest histograms:";
    for (const auto id : nearest_ids) {
        std::cout << " " << id;
    }
    std::cout << "\n";

    return 0;
}
