# Python Compare Spaces

`Space.compare(...)` measures dependency between two finite metric spaces by
comparing their distance profiles.

```python
from metric import Space

sample_ids = ["a", "b", "c", "d", "e"]
process_values = [0.0, 1.0, 2.0, 3.0, 4.0]
quality_values = [0.0, 1.0, 4.0, 9.0, 16.0]


def absolute_distance(lhs, rhs):
    return abs(lhs - rhs)


process_space = Space(process_values, metric=absolute_distance, ids=sample_ids)
quality_space = Space(quality_values, metric=absolute_distance, ids=sample_ids)

comparison = process_space.compare(quality_space, align="ids")

print(comparison.statistic_name)
print(round(comparison.value, 3))
print(comparison.matched_ids)
```

Use `align="ids"` when both spaces use the same stable record IDs. Use
`align="position"` for compatibility with older positional workflows.

Raw right-hand records are accepted when an explicit `other_metric` is passed:

```python
from metric import Space

process_values = [0.0, 1.0, 2.0, 3.0]
quality_values = [0.0, 1.0, 4.0, 9.0]


def absolute_distance(lhs, rhs):
    return abs(lhs - rhs)


process_space = Space(process_values, metric=absolute_distance)
comparison = process_space.compare(
    quality_values,
    other_metric=absolute_distance,
    align="position",
)

print(comparison.to_dict())
```

The promoted strategy is exact distance-profile correlation. P-values and
permutation workflows should remain explicit strategy/runtime work until they
have promoted release gates.
