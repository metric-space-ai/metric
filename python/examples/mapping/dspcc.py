import sys
import numpy
from metric.mapping import DSPCC


def main():
    # small dataset
    d = numpy.float_([
        [0, 1, 2, 3, 4, 5, 6, 100, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23],
        [0, 1, 2, 3, 4, 5, 6, 7,   8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 100]
    ])

    # try values from 0 to 1 (e g 0, 0.5, 1) to get the following portions of freq-domain: 0, 4 / 9, 8 / 9
    freq_time_balance = 0.5
    bundle = DSPCC(d, 4, 2, freq_time_balance, 4)
    encoded = bundle.encode(d)
    print(encoded)
    decoded = bundle.decode(encoded)
    print(decoded)


sys.exit(main())
