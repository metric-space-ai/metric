from metric import Space
from metric.strategies import DistanceProfileCorrelation


def absolute_distance(lhs, rhs):
    return abs(lhs - rhs)


def main():
    process_records = [0, 1, 2, 3, 4, 5]
    quality_records = [0, 1, 4, 9, 16, 25]

    process_space = Space(process_records, absolute_distance)
    quality_space = Space(quality_records, absolute_distance)

    dependency = process_space.compare(
        quality_space,
        strategy=DistanceProfileCorrelation(),
    )
    assert dependency.value > 0.8
    assert dependency.algorithm == "distance_profile_correlation"

    print("cross-space distance-profile correlation =", round(dependency.value, 3))


if __name__ == "__main__":
    main()
