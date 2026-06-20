# Python Custom Metric

Custom metrics are ordinary Python callables. They let users define geometry for
strings, rows, histograms, curves, descriptors, or domain objects.

## String Records

```python
from metric import Space


def padded_hamming(lhs, rhs):
    width = max(len(lhs), len(rhs))
    distance = 0
    for index in range(width):
        left = lhs[index] if index < len(lhs) else None
        right = rhs[index] if index < len(rhs) else None
        distance += left != right
    return distance


space = Space(["red", "reed", "road", "blue"], metric=padded_hamming)
neighbors = space.neighbors("read", count=2)

for item in neighbors.neighbors:
    print(item.record, item.distance)
```

The result uses named `Neighbor` objects, so examples can read fields instead of
unpacking tuples.

## Structured Rows

```python
from metric import Space

records = [
    {"name": "warmup", "temperature": 20.0, "status": "ok"},
    {"name": "drift", "temperature": 22.0, "status": "ok"},
    {"name": "spike", "temperature": 38.0, "status": "stop"},
]


def process_distance(lhs, rhs):
    temperature = abs(lhs["temperature"] - rhs["temperature"])
    status = 8.0 if lhs["status"] != rhs["status"] else 0.0
    return temperature + status


space = Space(records, metric=process_distance)
groups = space.groups(count=2)

print(groups.assignments)
print(groups.to_dict())
```

The metric can combine numeric, categorical, and domain-specific penalties in a
single finite-space geometry.
