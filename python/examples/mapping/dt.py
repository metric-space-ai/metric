import sys
import numpy
from time import time
from metric.mapping import DT, Dimension
from metric import distance


def extend(sequence, length):
    return sequence[:length] + [0] * (length - len(sequence))


def main():
    print("Metric Decision Tree example have started")

    img1 = numpy.float_([ # needs to be larger than blur kernel size coded intarnally as 11
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ]
    ])
    img2 = numpy.float_([
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ],
        [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ], [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ]
    ])

    selection = [
        [2, [1, 2],         [0, 1, 1, 1, 1, 1, 2, 3], img1, "", 1],
        [2, [1, 5],         [1, 1, 1, 1, 1, 2, 3, 4], img2, "A", 1],
        [1, [4, 5],         [2, 2, 2, 1, 1, 2, 0, 0], img2, "AA", 2],
        [2, [1, 2],         [3, 3, 2, 2, 1, 1, 0, 0], img1, "AAA", 1],
        [2, [5],            [4, 3, 2, 1, 0, 0, 0, 0], img1, "AAAA", 1],
        [2, [1, 4, 5],      [4, 3, 2, 1, 0, 0, 0, 0], img2, "BAAA", 1],
        [1, [1, 2, 3, 4],   [5, 3, 2, 1, 0, 0, 0, 0], img2, "BBAA", 3],
        [1, [1],            [4, 6, 2, 2, 1, 1, 0, 0], img1, "BBA", 1],
        [2, [4, 5],         [3, 7, 2, 1, 0, 0, 0, 0], img2, "BB", 1],
        [2, [1, 2, 4, 5],   [2, 5, 1, 1, 0, 0, 1, 2], img1, "B", 1]
    ]

    accessors = (
        lambda record: record[0],
        lambda record: extend(record[1], 4),
        lambda record: extend(record[2], 8),
        lambda record: record[3],
        lambda record: record[4]
    )

    # label accessor (for single record)
    response = lambda record: abs(record[5])

    dimensions = (
        Dimension(accessors[0], distance.Euclidean()),
        Dimension(accessors[1], distance.Manhatten()),
        Dimension(accessors[2], distance.P_norm()),
        Dimension(accessors[2], distance.Euclidean_thresholded()),
        Dimension(accessors[2], distance.Cosine()),
        Dimension(accessors[3], distance.SSIM()),
        Dimension(accessors[2], distance.TWED(0, 1)),
        Dimension(accessors[4], distance.Edit()),
        Dimension(accessors[2], distance.EMD(8, 8))
    )

    test_sample = [selection[0], selection[2], selection[6]]

    print("Metric Desicion Tree: ")
    start_t = time()
    model = DT()
    print("Metric Desicion Tree training... ")
    model.train(selection, dimensions, response)
    end_t = time()
    print(f"Metric Desicion Tree trained (Time = {end_t - start_t}")
    prediction = model.predict(test_sample, dimensions)
    print("Metric Desicion Tree prediction: ", list(prediction))

    prediction = model.predict(test_sample, dimensions)
    print("Metric Desicion Tree prediction2: ", list(prediction))

    print("Distances separately: ")
    # test Edit separately
    print("Edit distance: ", distance.Edit()("AAAB", "AAC"))

    # test SSIM separately
    print("SSIM distance: ", distance.SSIM()(img1, img2))


sys.exit(main())
