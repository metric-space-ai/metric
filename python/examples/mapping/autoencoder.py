import sys
import metric
import numpy


def main():
    features = numpy.int_([1, 2, 3, 4, 5, 6])
    sample = numpy.int_([10, 1, 2, 33])
    autoencoder = metric.Autoencoder(features, 15, 255)

    print(autoencoder.train(1, 256))

    prediction = autoencoder.predict(sample)
    print(prediction)

    latent_vector = autoencoder.encode(sample)
    print(latent_vector)


sys.exit(main())
