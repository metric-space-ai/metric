"""Cross-space dependency — adapter boundary demo.

Cross-space distance-profile correlation is a native-only METRIC algorithm. The
Python ``Space`` still adapts both record sets and exposes their explicit
distances; the comparison itself is reached through a native binding.
"""

from metric import Space
from metric.exceptions import StrategyUnavailableError
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

    # Native boundary: the dependency statistic itself requires the C++ binding.
    try:
        process_space.compare(quality_space, strategy=DistanceProfileCorrelation())
    except StrategyUnavailableError:
        print("compare: requires native C++ binding")
    else:
        raise AssertionError("compare should require a native binding")


if __name__ == "__main__":
    main()
