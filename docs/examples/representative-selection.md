# Representative Selection

Representative selection chooses existing records from a finite metric space. This is useful when the record type does not have a meaningful vector centroid, such as strings, histograms, event sequences, or mixed structured records.

The promoted C++ example [representative_selection_space.cpp](../../examples/core/representative_selection_space.cpp) selects string representatives using edit distance, deterministic farthest-first traversal, and deterministic radius coverage.

The promoted Python example [representative_selection_space.py](../../python/examples/metric_space/representative_selection_space.py) selects histogram representatives using a one-dimensional transport callable and the same two strategy rules.

C++ shape:

```cpp
#include <metric/distance.hpp>
#include <metric/operators.hpp>

#include <string>
#include <vector>

std::vector<std::string> records = {"cat", "cot", "coat", "dog"};

auto selected_ids = metric::operators::representative_indices(records, metric::Edit<std::string>{}, 2);
auto selected_records = metric::operators::representatives(records, metric::Edit<std::string>{}, 2);
auto covered_ids = metric::operators::coverage_representative_indices(records, metric::Edit<std::string>{}, 1);
auto covered_records = metric::operators::coverage_representatives(records, metric::Edit<std::string>{}, 1);
```

Python shape:

```python
from metric import (
    coverage_representative_indices,
    coverage_representatives,
    representative_indices,
    representatives,
)

records = [
    (1.0, 0.0, 0.0, 0.0),
    (0.0, 1.0, 0.0, 0.0),
    (0.0, 0.0, 0.0, 1.0),
]

selected_ids = representative_indices(records, cumulative_transport_distance, k=2)
selected_records = representatives(records, cumulative_transport_distance, k=2)
covered_ids = coverage_representative_indices(records, cumulative_transport_distance, radius=1)
covered_records = coverage_representatives(records, cumulative_transport_distance, radius=1)
```

The farthest-first helpers start from `seed_index=0` by default, then repeatedly choose the unselected record whose nearest selected representative is farthest away. Equal-distance ties are resolved by record order, so small fixtures can use exact expected representative IDs.

The coverage helpers scan records in order, choose the first uncovered record as a representative, and mark every record within `radius` as covered. This is a deterministic radius-cover heuristic, not a k-medoids optimizer. The promoted behavior is documented and covered by the core C++ and Python CI gates. Additional strategies should be promoted only after they have their own fixtures, result contracts, and release notes.
