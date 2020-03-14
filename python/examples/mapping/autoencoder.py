import sys
import json
import numpy
from metric.mapping import Autoencoder
from metric.utils import Datasets


def main():
    data = {
        "0":
            {
                "type": "FullyConnected",
                "inputSize": 784,
                "outputSize": 128,
                "activation": "ReLU"
            },
        "1":
            {
                "type": "FullyConnected",
                "inputSize": 128,
                "outputSize": 6,
                "activation": "ReLU"
            },
        "2":
            {
                "type": "FullyConnected",
                "inputSize": 6,
                "outputSize": 128,
                "activation": "ReLU"
            },
        "3":
            {
                "type": "FullyConnected",
                "inputSize": 128,
                "outputSize": 784,
                "activation": "Sigmoid"
            },
        "train":
            {
                "loss": "RegressionMSE",
                "optimizer": {"type": "RMSProp",
                              "learningRate": 0.01,
                              "eps": 1e-6,
                              "decay": 0.9}
            }
    }
    labels, shape, features = Datasets().get_mnist("../examples/dnn_examples/data.cereal")
    if not shape:
        raise RuntimeError("Data file is empty. Exiting.")

    # Autoencoder<uint8_t, double> autoencoder(json.dumps(data));
    autoencoder = Autoencoder(json.dumps(data))
    # autoencoder.set_callback()

    print("Train")
    autoencoder.train(features, 5, 256)

    print("Sample:")
    sample = features[:shape[1] * shape[2]]
    print(numpy.array(sample))

    print("Prediction:")
    prediction = autoencoder.predict(sample)
    print(numpy.array(prediction))

    print("latent vector")
    latent_vector = autoencoder.encode(sample)
    print(numpy.array(latent_vector))

    t = numpy.subtract(prediction, latent_vector)
    print("test:", t)


sys.exit(main())
