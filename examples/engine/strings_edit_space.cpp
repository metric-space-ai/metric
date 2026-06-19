#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include <metric/distance.hpp>
#include <metric/engine.hpp>

int main()
{
    std::vector<std::string> records = {"metric", "metrics", "matrix", "tree"};
    auto space = metric::make_space(records, metric::Edit<char>{});

    metric::representations::MatrixCache<decltype(space)> matrix(space);
    assert(matrix.distance(space.id(0), space.id(1)) == 1);

    const auto neighbors =
        metric::find_neighbors(space, std::string("metricks"), metric::count{2}, metric::strategies::cover_tree{});
    assert(neighbors.operator_name == "knn");
    assert(neighbors.representation == "cover_tree_index");
    assert(neighbors.size() == 2);
    assert(neighbors[0].id == space.id(1));
    assert(neighbors[0].distance == 1);

    const auto groups = metric::find_groups(space, metric::strategies::k_medoids(2));
    assert(groups.algorithm == "kmedoids");
    assert(groups.cluster_count == 2);
    assert(groups.record_count == records.size());

    std::cout << "nearest string = " << records[neighbors[0].id.index()] << " at " << neighbors[0].distance << "\n";
    std::cout << "string groups = " << groups.cluster_count << "\n";

    return 0;
}
