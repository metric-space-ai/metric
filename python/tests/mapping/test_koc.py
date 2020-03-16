import numpy
from metric.mapping import KOC_factory


def test_ok():
    best_w_grid_size = 5
    best_h_grid_size = 4
    anomaly_threshold = -0.5

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

    koc_factory = KOC_factory(best_w_grid_size, best_h_grid_size, 0.5, 0.0, 100)
    koc = koc_factory(dataset, 5)

    anomaly1 = koc.check_if_anomaly(samples=dataset, anomaly_threshold=anomaly_threshold)
    anomaly2 = koc.check_if_anomaly(samples=dataset, anomaly_threshold=anomaly_threshold)

    assignments1 = koc.result(dataset, anomaly_threshold)
    assignments2 = koc.result(dataset, anomaly_threshold)

    assert list(anomaly1) == list(anomaly2)
    assert list(assignments1) == list(assignments2)
