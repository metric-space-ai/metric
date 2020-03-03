import numpy
from metric.mapping import KOC_factory


def test_ok():
    best_w_grid_size = 5
    best_h_grid_size = 4

    dataset = numpy.float_([
        [0, 0, 0],
        [0, 1, 0],
        [0, 2, 0],
        [2, 2, 2],
        [2, 2, 2],
        [2, 2, 2],
        [0, 0, 4],
        [0, 0, 4],
        [0, 0, 4],
        [8, 0, 0],
        [8, 0, 0],
        [8, 0, 0],
    ])

    expected_anomaly = [False, False, False, True, True, True, False, False, False, False, False, False]

    simple_dataset = numpy.float_([8, 0, 1])

    koc_factory = KOC_factory(best_w_grid_size, best_h_grid_size, 0.5, 0.0, 100)
    koc = koc_factory(dataset, 5)

    assert koc.check_if_anomaly(sample=simple_dataset, anomaly_threshold=-0.5)
    assert koc.check_if_anomaly(samples=dataset, anomaly_threshold=-0.5) == expected_anomaly
    assert koc.result(dataset, -0.5) == 10
