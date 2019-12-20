# DNN module

Network json consist of map object. Keys is layer index (0...). Values is layer object.

## Network fully connected autoencoder example:

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
