# Representative Selection

Representative selection chooses existing records from a finite metric space. This is useful when the record type does not have a meaningful vector centroid, such as strings, histograms, event sequences, or mixed structured records.

The promoted C++ example [representative_selection_space.cpp](../../examples/core/representative_selection_space.cpp) selects string representatives using edit distance, deterministic farthest-first traversal, medoid selection, separated representatives, and deterministic radius coverage.

The promoted Python example [representative_selection_space.py](../../python/examples/metric_space/representative_selection_space.py) selects histogram representatives using a one-dimensional transport callable and the same four strategy rules.

C++ shape:

```cpp
#include <metric/metric/catalog.hpp>
#include <metric/space/index/operators.hpp>

#include <string>
#include <vector>

std::vector<std::string> records = {"cat", "cot", "coat", "dog"};

auto selected_ids = mtrc::space::index::representative_indices(records, mtrc::Edit<std::string>{}, 2);
auto selected_records = mtrc::space::index::representatives(records, mtrc::Edit<std::string>{}, 2);
auto center_id = mtrc::space::index::medoid_index(records, mtrc::Edit<std::string>{});
auto center_record = mtrc::space::index::medoid(records, mtrc::Edit<std::string>{});
auto separated_ids = mtrc::space::index::separated_representative_indices(records, mtrc::Edit<std::string>{}, 2);
auto separated_records = mtrc::space::index::separated_representatives(records, mtrc::Edit<std::string>{}, 2);
auto covered_ids = mtrc::space::index::coverage_representative_indices(records, mtrc::Edit<std::string>{}, 1);
auto covered_records = mtrc::space::index::coverage_representatives(records, mtrc::Edit<std::string>{}, 1);
```

Python shape:

```python
from metric import (
    coverage_representative_indices,
    coverage_representatives,
    medoid,
    medoid_index,
    representative_indices,
    representatives,
    separated_representative_indices,
    separated_representatives,
)

records = [
    (1.0, 0.0, 0.0, 0.0),
    (0.0, 1.0, 0.0, 0.0),
    (0.0, 0.0, 0.0, 1.0),
]

selected_ids = representative_indices(records, cumulative_transport_distance, k=2)
selected_records = representatives(records, cumulative_transport_distance, k=2)
center_id = medoid_index(records, cumulative_transport_distance)
center_record = medoid(records, cumulative_transport_distance)
separated_ids = separated_representative_indices(records, cumulative_transport_distance, minimum_distance=1.5)
separated_records = separated_representatives(records, cumulative_transport_distance, minimum_distance=1.5)
covered_ids = coverage_representative_indices(records, cumulative_transport_distance, radius=1)
covered_records = coverage_representatives(records, cumulative_transport_distance, radius=1)
```

The farthest-first helpers start from `seed_index=0` by default, then repeatedly choose the unselected record whose nearest selected representative is farthest away. Equal-distance ties are resolved by record order, so small fixtures can use exact expected representative IDs.

The medoid helpers choose the existing record with the smallest total distance to every record in the finite metric space. Equal total-distance ties are resolved by record order. This is a single-medoid summary, not a k-medoids optimizer.

The separated-representative helpers scan records in order and keep a candidate when it is at least `minimum_distance` from every already selected representative. This is a deterministic redundancy-threshold heuristic; it returns a maximal separated subset for that input order, not an optimal packing proof.

The coverage helpers scan records in order, choose the first uncovered record as a representative, and mark every record within `radius` as covered. This is a deterministic radius-cover heuristic. The promoted behavior is documented and covered by the core C++ and Python CI gates. Additional strategies should be promoted only after they have their own fixtures, result contracts, and release notes.
