"""Representative selection over a transport space — adapter boundary demo.

Representative selection (farthest-first, medoid, separated, coverage) is a
METRIC algorithm. In the adapter-only Python package these helpers stay
importable as a stable vocabulary but raise StrategyUnavailableError until the
native C++ binding is exposed. The Python layer still adapts the records and
exposes the explicit distances the native selectors consume.
"""

from metric import (
    Space,
    coverage_representative_indices,
    coverage_representatives,
    medoid,
    medoid_index,
    representative_indices,
    representatives,
    separated_representative_indices,
    separated_representatives,
)
from metric.exceptions import StrategyUnavailableError


def cumulative_transport_distance(lhs, rhs):
    """One-dimensional earth mover distance for equal-mass histograms."""
    if len(lhs) != len(rhs):
        raise ValueError("histograms must have the same number of bins")

    cumulative_delta = 0.0
    distance = 0.0
    for lhs_mass, rhs_mass in zip(lhs, rhs):
        cumulative_delta += lhs_mass - rhs_mass
        distance += abs(cumulative_delta)

    return distance


def requires_native(label, call):
    try:
        call()
    except StrategyUnavailableError:
        print(f"{label}: requires native C++ binding")
    else:
        raise AssertionError(f"{label} should require a native binding")


def main():
    names = ["left-edge", "one-step", "right-edge", "split-left", "center"]
    records = [
        (1.0, 0.0, 0.0, 0.0),
        (0.0, 1.0, 0.0, 0.0),
        (0.0, 0.0, 0.0, 1.0),
        (0.5, 0.5, 0.0, 0.0),
        (0.0, 0.5, 0.5, 0.0),
    ]

    space = Space(records, cumulative_transport_distance)

    # Adapter surface: explicit distances over a caller-provided metric.
    assert space.distance(0, 2) == 3.0
    print("records =", ", ".join(names))
    print("distance(left-edge, right-edge) =", space.distance(0, 2))

    # Native boundary: every representative selector requires the C++ binding.
    metric = cumulative_transport_distance
    requires_native("representative_indices", lambda: representative_indices(records, metric, k=3))
    requires_native("representatives", lambda: representatives(records, metric, k=3))
    requires_native("medoid_index", lambda: medoid_index(records, metric))
    requires_native("medoid", lambda: medoid(records, metric))
    requires_native("separated_representative_indices", lambda: separated_representative_indices(records, metric, 1.5))
    requires_native("separated_representatives", lambda: separated_representatives(records, metric, 1.5))
    requires_native("coverage_representative_indices", lambda: coverage_representative_indices(records, metric, 1.5))
    requires_native("coverage_representatives", lambda: coverage_representatives(records, metric, 1.5))


if __name__ == "__main__":
    main()
