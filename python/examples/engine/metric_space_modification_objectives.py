"""Choose finite metric-space modification by objective, not algorithm name."""

from metric import Space
from metric.strategies import Coverage, KMedoids, PreserveDistribution, RadiusCoverage, UniformDensity


def absolute_distance(lhs, rhs):
    return abs(lhs - rhs)


def main():
    records = [0, 1, 2, 8, 9, 30]
    space = Space(records, absolute_distance)

    preserve_density = space.thin(3, strategy=PreserveDistribution())
    assert preserve_density.space.records == [0, 2, 9]
    assert preserve_density.source_record_ids == (0, 2, 4)
    print("preserve density =", preserve_density.space.records)

    flatten_density = space.equalize(radius=1)
    assert flatten_density.space.records == [0, 2, 8, 30]
    assert flatten_density.diagnostics["empirical_density_preserved"] is False
    assert flatten_density.diagnostics["local_volume_count_drift"] < 0.0
    print("flatten density =", flatten_density.space.records)

    bounded_radius = space.compress(strategy=RadiusCoverage(radius=1))
    assert bounded_radius.space.records == [0, 2, 8, 30]
    assert bounded_radius.representative_multiplicities == (2, 1, 2, 1)
    print("coverage radius representatives =", bounded_radius.space.records)

    minimize_max_error = space.compress(3, strategy=Coverage())
    assert minimize_max_error.space.records == [0, 30, 9]
    assert max(minimize_max_error.nearest_representative_distances) == 2.0
    print("minimize max error =", minimize_max_error.space.records)

    minimize_average_error = space.compress(strategy=KMedoids(groups=3))
    assert minimize_average_error.space.records == [1, 30, 8]
    assert minimize_average_error.representative_weights == (0.5, 1.0 / 6.0, 1.0 / 3.0)
    print("minimize average error =", minimize_average_error.space.records)


if __name__ == "__main__":
    main()
