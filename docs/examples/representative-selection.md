# Representative Selection

Representative selection chooses existing records from a finite metric space. This is useful when the record type does not have a meaningful vector centroid, such as strings, histograms, event sequences, or mixed structured records.

The promoted Python example [representative_selection_space.py](../../python/examples/metric_space/representative_selection_space.py) selects three histogram records using a one-dimensional transport callable and deterministic farthest-first traversal.

Core shape:

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

The helper starts from `seed_index=0` by default, then repeatedly chooses the unselected record whose nearest selected representative is farthest away. Equal-distance ties are resolved by record order, so small fixtures can use exact expected representative IDs.

This is a coverage-oriented heuristic, not a k-medoids optimizer. It is promoted for the Python core facade because the behavior is deterministic, documented, and covered by wheel CI. Additional strategies should be promoted only after they have their own fixtures, result contracts, and release notes.
