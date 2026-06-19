#include <cassert>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

#include "metric/distance.hpp"
#include "metric/operators.hpp"
#include "metric/space.hpp"

struct AbsoluteDistance {
    auto operator()(int lhs, int rhs) const -> int
    {
        return lhs > rhs ? lhs - rhs : rhs - lhs;
    }
};

int main()
{
    std::vector<std::string> records = {"cat", "cot", "coat", "dog"};

    auto space = metric::Space::from_records(records, metric::Edit<std::string>{});

    assert(space.size() == records.size());
    assert(space[0] == "cat");
    assert(space(0, 0) == 0);
    assert(space(0, 1) == 1);
    assert(space.distance(0, 2) == 1);
    assert(space(0, 1) == space(1, 0));

    const auto distances = space.pairwise_distances();
    assert(distances.size() == records.size());
    assert(distances[0][1] == 1);

    const auto operator_distances = metric::operators::pairwise_distance_matrix(records, metric::Edit<std::string>{});
    assert(operator_distances.size() == records.size());
    assert(operator_distances[0][1] == 1);

    const auto nearest = space.neighbors(std::string("cut"), 2);
    assert(nearest.size() == 2);
    assert(nearest[0].second == 1);
    assert(space.nearest(std::string("cut")).second == 1);
    assert(space.within_radius(std::string("cut"), 1).size() == 2);

    const auto operator_nearest =
        metric::operators::nearest_neighbors(records, metric::Edit<std::string>{}, std::string("cut"), 2);
    assert(operator_nearest.size() == 2);
    assert(operator_nearest[0].second == 1);

    const auto operator_range =
        metric::operators::range_neighbors(records, metric::Edit<std::string>{}, std::string("cut"), 1);
    assert(operator_range.size() == 2);

    const auto representative_ids = metric::operators::representative_indices(records, metric::Edit<std::string>{}, 3);
    assert((representative_ids == std::vector<std::size_t>{0, 3, 1}));

    const auto representative_records = metric::operators::representatives(records, metric::Edit<std::string>{}, 3);
    assert((representative_records == std::vector<std::string>{"cat", "dog", "cot"}));

    assert(metric::operators::medoid_index(records, metric::Edit<std::string>{}) == 1);
    assert(metric::operators::medoid(records, metric::Edit<std::string>{}) == "cot");

    const auto coverage_ids = metric::operators::coverage_representative_indices(records, metric::Edit<std::string>{}, 1);
    assert((coverage_ids == std::vector<std::size_t>{0, 3}));

    const auto coverage_records = metric::operators::coverage_representatives(records, metric::Edit<std::string>{}, 1);
    assert((coverage_records == std::vector<std::string>{"cat", "dog"}));

    bool rejected_edit_negative_radius = false;
    try {
        metric::operators::coverage_representative_indices(records, metric::Edit<std::string>{}, -1);
    } catch (const std::invalid_argument &) {
        rejected_edit_negative_radius = true;
    }
    assert(rejected_edit_negative_radius);

    bool rejected_empty_records = false;
    try {
        metric::operators::representative_indices(std::vector<std::string>{}, metric::Edit<std::string>{}, 1);
    } catch (const std::invalid_argument &) {
        rejected_empty_records = true;
    }
    assert(rejected_empty_records);

    bool rejected_empty_medoid = false;
    try {
        metric::operators::medoid_index(std::vector<std::string>{}, metric::Edit<std::string>{});
    } catch (const std::invalid_argument &) {
        rejected_empty_medoid = true;
    }
    assert(rejected_empty_medoid);

    bool rejected_large_k = false;
    try {
        metric::operators::representative_indices(records, metric::Edit<std::string>{}, records.size() + 1);
    } catch (const std::invalid_argument &) {
        rejected_large_k = true;
    }
    assert(rejected_large_k);

    bool rejected_seed = false;
    try {
        metric::operators::representative_indices(records, metric::Edit<std::string>{}, 1, records.size());
    } catch (const std::out_of_range &) {
        rejected_seed = true;
    }
    assert(rejected_seed);

    const std::vector<int> line = {0, 1, 2, 3, 4};
    assert((metric::operators::coverage_representative_indices(line, AbsoluteDistance{}, 2) ==
            std::vector<std::size_t>{0, 3}));
    assert((metric::operators::coverage_representatives(line, AbsoluteDistance{}, 2) == std::vector<int>{0, 3}));
    assert(metric::operators::medoid_index(line, AbsoluteDistance{}) == 2);
    assert(metric::operators::medoid(line, AbsoluteDistance{}) == 2);

    bool rejected_negative_radius = false;
    try {
        metric::operators::coverage_representative_indices(line, AbsoluteDistance{}, -1);
    } catch (const std::invalid_argument &) {
        rejected_negative_radius = true;
    }
    assert(rejected_negative_radius);

    const double dimension = metric::operators::intrinsic_dimension(line, AbsoluteDistance{});
    assert(std::abs(dimension - (std::log(5.0 / 3.0) / std::log(2.0))) < 1e-12);

    return 0;
}
