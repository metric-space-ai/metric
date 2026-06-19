# Representative Selection

Representative selection chooses existing records from a finite metric space. This is useful when the record type does not have a meaningful vector centroid, such as strings, histograms, event sequences, or mixed structured records.

The promoted C++ example [representative_selection_space.cpp](../../examples/core/representative_selection_space.cpp) selects three string records using edit distance and deterministic farthest-first traversal.

The promoted Python example [representative_selection_space.py](../../python/examples/metric_space/representative_selection_space.py) selects three histogram records using a one-dimensional transport callable and the same traversal rule.

C++ shape:

```cpp
#include <metric/distance.hpp>
#include <metric/operators.hpp>

#include <string>
#include <vector>

std::vector<std::string> records = {"cat", "cot", "coat", "dog"};

auto selected_ids = metric::operators::representative_indices(records, metric::Edit<std::string>{}, 2);
auto selected_records = metric::operators::representatives(records, metric::Edit<std::string>{}, 2);
```

Python shape:

```python
from metric import representative_indices, representatives

records = [
    (1.0, 0.0, 0.0, 0.0),
    (0.0, 1.0, 0.0, 0.0),
    (0.0, 0.0, 0.0, 1.0),
]

selected_ids = representative_indices(records, cumulative_transport_distance, k=2)
selected_records = representatives(records, cumulative_transport_distance, k=2)
```

The helpers start from `seed_index=0` by default, then repeatedly choose the unselected record whose nearest selected representative is farthest away. Equal-distance ties are resolved by record order, so small fixtures can use exact expected representative IDs.

This is a coverage-oriented heuristic, not a k-medoids optimizer. It is promoted because the behavior is deterministic, documented, and covered by the core C++ and Python CI gates. Additional strategies should be promoted only after they have their own fixtures, result contracts, and release notes.
