# DNN module

## Network construction methods

### Load trained net
```c++
#include "metric/modules/utils/dnn.hpp"

dnn::Network<double> net;
net.load("path_to_cereal_file");
```
### Construct from json string
```c++
#include "metric/modules/utils/dnn.hpp"

# Network with one dense layer
auto json = R"({
                "0":
                    {
                        "type": "FullyConnected",
                        "inputSize": 100,
                        "outputSize": 10,
                        "activation": "ReLU"
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
            )"_json;

dnn::Network<double> net(json);

# Train
......
```

### Describe structure in program code
```c++
#include "metric/modules/utils/dnn.hpp"

dnn::Network<double> net;

net.addLayer(dnn::FullyConnected<double, dnn::ReLU<double>>(4000, 1024));
net.addLayer(dnn::FullyConnected<double, dnn::ReLU<double>>(1024, 256));
net.addLayer(dnn::FullyConnected<double, dnn::ReLU<double>>(256, 64));
net.addLayer(dnn::FullyConnected<double, dnn::ReLU<double>>(64, 256));
net.addLayer(dnn::FullyConnected<double, dnn::ReLU<double>>(256, 1024));
net.addLayer(dnn::FullyConnected<double, dnn::Sigmoid<double>>(1024, 4000));
```

##Json
Network json consist of map object. Keys is layer index (0...). Values is layer object.

### Network fully connected autoencoder example:

```javascript
{
    "0":
        {
            "type": "FullyConnected",
            "inputSize": 784,
            "outputSize": 256,
            "activation": "ReLU"
        },
    "1":
        {
            "type": "FullyConnected",
            "inputSize": 256,
            "outputSize": 64,
            "activation": "ReLU"
        },
    "2":
        {
            "type": "FullyConnected",
            "inputSize": 64,
            "outputSize": 256,
            "activation": "ReLU"
        },
    "3":
        {
            "type": "FullyConnected",
            "inputSize": 256,
            "outputSize": 784,
            "activation": "Sigmoid"
        },
    "train":
        {
            "loss": "RegressionMSE",
            "optimizer": {"type": "RMSProp",
                          "learningRate": 0.01}
        }
    }
}
```


## Layers json examples  

```javascript
{
    "type": "FullyConnected",
    "inputSize": 1024,
    "outputSize": 512,
    "activation": "ReLU"
}
```

```javascript
{
    "type": "MaxPooling",
    "inputWidth": 200,
    "inputHeight": 200,
    "inputChannels": 3,
    "windowWidth": 4,
    "windowHeight": 4,
    "activation": "ReLU"
}
```

```javascript
{
    "type": "Conv2D",
    "inputWidth": 200,
    "inputHeight": 200,
    "inputChannels": 3,
    "outputChannels": 1,
    "kernelWidth": 10,
    "kernelHeight": 10,
    "activation": "ReLU"
}
```

```javascript
{
    "type": "Conv2DTranspose",
    "inputWidth": 200,
    "inputHeight": 200,
    "inputChannels": 3,
    "outputChannels": 1,
    "kernelWidth": 10,
    "kernelHeight": 10,
    "activation": "ReLU"
}
```
