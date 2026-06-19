#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include <metric/engine.hpp>

namespace {

struct AlignedCurveDistance {
    double gap_cost{2.0};

    auto operator()(const std::vector<double> &lhs, const std::vector<double> &rhs) const -> double
    {
        std::vector<double> previous(rhs.size() + 1, 0.0);
        for (std::size_t index = 0; index < previous.size(); ++index) {
            previous[index] = static_cast<double>(index) * gap_cost;
        }

        for (std::size_t lhs_index = 1; lhs_index <= lhs.size(); ++lhs_index) {
            std::vector<double> current(rhs.size() + 1, 0.0);
            current[0] = static_cast<double>(lhs_index) * gap_cost;
            for (std::size_t rhs_index = 1; rhs_index <= rhs.size(); ++rhs_index) {
                const auto substitution =
                    previous[rhs_index - 1] + std::min(std::abs(lhs[lhs_index - 1] - rhs[rhs_index - 1]), 2 * gap_cost);
                const auto deletion = previous[rhs_index] + gap_cost;
                const auto insertion = current[rhs_index - 1] + gap_cost;
                current[rhs_index] = std::min({substitution, deletion, insertion});
            }
            previous = std::move(current);
        }

        return previous.back();
    }
};

} // namespace

int main()
{
    const std::vector<std::string> names = {"baseline", "shifted", "flat", "spike"};
    const std::vector<std::vector<double>> records = {
        {0, 1, 1, 1, 2, 3},
        {0, 0, 1, 1, 1, 2, 3},
        {2, 2, 2, 2, 2, 2},
        {0, 1, 6, 1, 2, 3},
    };

    auto space = metric::make_space(records, AlignedCurveDistance{});
    const std::vector<double> query = {0, 1, 1, 1, 2, 4};

    const auto neighbors = metric::find_neighbors(space, query, 2);
    assert(neighbors.representation == "metric_space");
    assert(neighbors.size() == 2);
    assert(neighbors[0].id == space.id(0));
    assert(neighbors[0].distance == 1.0);

    const auto representatives = metric::find_representatives(space, 2, metric::strategies::farthest_first{});
    assert(representatives.size() == 2);
    assert(representatives.strategy == "farthest_first");

    metric::representations::GraphTopology<decltype(space)> graph(space);
    for (std::size_t source = 0; source < space.size(); ++source) {
        for (std::size_t target = 0; target < space.size(); ++target) {
            if (source == target) {
                continue;
            }
            const auto distance = space.distance(space.id(source), space.id(target));
            if (distance <= 2.0) {
                graph.add_edge(space.id(source), space.id(target), distance);
            }
        }
    }
    assert(graph.record_count() == space.size());
    assert(graph.edge_count() > 0);

    std::cout << "nearest process curve = " << names[neighbors[0].id.index()] << " at " << neighbors[0].distance
              << "\n";
    std::cout << "process curve graph edges = " << graph.edge_count() << "\n";

    return 0;
}
