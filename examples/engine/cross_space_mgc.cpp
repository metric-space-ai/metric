#include <array>
#include <cassert>
#include <cmath>
#include <deque>
#include <iostream>
#include <vector>

#include <metric/distance.hpp>
#include <metric/engine.hpp>

int main()
{
    std::vector<std::vector<double>> process_space_records;
    std::deque<std::array<float, 1>> quality_space_records;

    for (int index = 0; index < 12; ++index) {
        const auto process_value = static_cast<double>(index);
        process_space_records.push_back({process_value});
        quality_space_records.push_back({static_cast<float>((process_value * process_value) / 10.0)});
    }

    auto process_space = metric::make_space(process_space_records, metric::Euclidean<double>());
    auto quality_space = metric::make_space(quality_space_records, metric::Manhattan<float>());

    const auto compared = metric::compare(process_space, quality_space, metric::strategies::mgc{});
    assert(compared.algorithm == "mgc");
    assert(compared.left_representation == "metric_space");
    assert(compared.right_representation == "metric_space");
    assert(compared.left_record_count == process_space.size());
    assert(compared.right_record_count == quality_space.size());
    assert(compared.exact);
    assert(std::isfinite(compared.value));
    assert(compared.value >= 0.0);

    std::cout << "cross-space MGC = " << compared.value << "\n";

    return 0;
}
