#include <cassert>
#include <string>
#include <vector>

#include "metric/distance.hpp"
#include "metric/operators.hpp"
#include "metric/space.hpp"

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

    return 0;
}
