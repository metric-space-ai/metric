# Python Errors

Python public methods raise METRIC exceptions with actionable messages. They
should not expose pybind11 overload details, generated template names, or
private `_impl` modules.

## Hierarchy

```text
MetricError
  MissingMetricError
  MetricInputError
  MetricContractError
  MetricComputationError
  AmbiguousIntentError
  IncompatibleSpaceError
  StrategyError
    StrategyUnavailableError
    StrategyParameterError
  RepresentationError
    StaleRepresentationError
  OptionalDependencyError
  NotDerivedError
  UnsupportedOperationError
```

All classes live in `metric.exceptions` and are re-exported from `metric`.

## Missing Metrics

```python
from metric import Space
from metric import MissingMetricError

try:
    Space(["cat", "cot"])
except MissingMetricError as error:
    print(error)
```

`Space(records)` without a metric raises `MissingMetricError`. Use
`Space(records, metric=...)` for arbitrary records or `Space.vectors(...)` for
the vector-specific Euclidean convenience.

## Metric Contract Errors

```python
from metric import Space
from metric import MetricContractError


def bad_distance(lhs, rhs):
    return -1.0


try:
    Space(["a", "b"], metric=bad_distance, validate="strict")
except MetricContractError as error:
    print(error)
```

Validation checks that distances are real, finite, and non-negative. It does
not prove every metric axiom unless user code asks for separate contract tests.

## Unsupported Strategies

```python
from metric import Space
from metric import StrategyUnavailableError
from metric.metrics import Edit
from metric.strategies import ParametricDiffusionCoordinates

space = Space(["cat", "cot", "coat"], metric=Edit())

try:
    space.map(strategy=ParametricDiffusionCoordinates(dimensions=2))
except StrategyUnavailableError as error:
    print(error)
```

Native adapter strategies fail explicitly instead of falling through to partial
Python behavior.

## Stale Representations

```python
from metric import Space
from metric import StaleRepresentationError
from metric.metrics import Edit

space = Space(["cat", "cot", "coat"], metric=Edit())
matrix = space.to_matrix()
space.touch()

try:
    space.groups(count=2, representation=matrix)
except StaleRepresentationError as error:
    print(error)
```

Representation errors include source and representation version information so
callers know to rebuild the representation.

## Optional Dependencies

`to_pandas()` helpers raise `OptionalDependencyError` when pandas is not
installed. Use `to_dict()` or `to_numpy()` for dependency-light code paths.

## Unsupported Inverse Reconstruction

Lossy reduction, compression, deterministic mapping, and density-filter results expose
`inverse_supported=False`. Calling `inverse_transform(...)` raises
`UnsupportedOperationError` until a strategy with a promoted inverse contract is
available.
