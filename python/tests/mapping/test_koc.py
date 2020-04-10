import numpy
from metric.mapping import KOC
from metric.utils import Grid6
from metric.distance import Euclidean


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

    koc = KOC(graph=Grid6(best_w_grid_size, best_h_grid_size),
              anomaly_sigma=anomaly_threshold,
              start_learn_rate=0.5,
              finish_learn_rate=0.0,
              iterations=100)
    koc.train(dataset, 5)

    anomaly1 = koc.check_if_anomaly(samples=dataset)
    anomaly2 = koc.check_if_anomaly(samples=dataset)

    indexes1, distances1 = koc.top_outliers(dataset)
    indexes2, distances2 = koc.top_outliers(dataset)

    assert list(anomaly1) == list(anomaly2)
    assert list(indexes1) == list(indexes2)
    assert list(distances1) == list(distances2)
