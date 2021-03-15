import sys
import numpy
from metric.mapping import KOC
from metric.utils import Grid4


def main():
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
    anomaly_threshold = -0.5
    simple_dataset = numpy.float_([8, 0, 1])

    simple_koc = KOC(graph=Grid4(best_w_grid_size, best_h_grid_size),
                     anomaly_sigma=anomaly_threshold,
                     start_learn_rate=0.0,
                     finish_learn_rate=100.0)
    simple_koc.train(dataset, 5)

    print('vector anomalies: ', simple_koc.check_if_anomaly(sample=simple_dataset))

    anomalies = simple_koc.check_if_anomaly(samples=dataset)
    print('anomalies:', list(anomalies))

    assignments = simple_koc.assign_to_clusters(dataset)
    print('assignments:', list(assignments))


sys.exit(main())
