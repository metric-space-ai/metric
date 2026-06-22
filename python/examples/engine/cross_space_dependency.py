"""Cross-space dependency — aligned distance-profile correlation demo.

Cross-space distance-profile correlation is a native METRIC algorithm. The
Python ``Space`` adapts both record sets and exposes their explicit distances;
the comparison statistic (Pearson correlation of the two pairwise distance
profiles) is computed in native C++ and marshaled into a ``CorrelationResult``
for the promoted equal-length, ``align="position"`` path.
"""

from metric import Space
from metric.exceptions import IncompatibleSpaceError
from metric.operators import pairwise_distance_matrix
from metric.strategies import DistanceProfileCorrelation


def absolute_distance(lhs, rhs):
    return abs(lhs - rhs)


def main():
    process_records = [0, 1, 2, 3, 4, 5]
    quality_records = [0, 1, 4, 9, 16, 25]

    process_space = Space(process_records, absolute_distance)
    quality_space = Space(quality_records, absolute_distance)

    # Adapter surface: explicit distance matrices over each space.
    process_matrix = pairwise_distance_matrix(process_records, absolute_distance)
    quality_matrix = pairwise_distance_matrix(quality_records, absolute_distance)
    print("process matrix rows =", len(process_matrix))
    print("quality matrix rows =", len(quality_matrix))

    # Native boundary: the dependency statistic is computed in C++.
    comparison = process_space.compare(quality_space, strategy=DistanceProfileCorrelation())
    print("compare statistic =", comparison.statistic_name)
    print("compare pairs =", comparison.pair_count)
    print("compare value =", round(comparison.value, 6))
    print("compare defined =", comparison.diagnostics["defined"])

    # Mismatched record counts fail with a named METRIC error.
    shorter = Space(process_records[:-1], absolute_distance)
    try:
        process_space.compare(shorter)
    except IncompatibleSpaceError as exc:
        print("mismatch rejected:", exc)
    else:
        raise AssertionError("aligned compare should reject mismatched record counts")


if __name__ == "__main__":
    main()
