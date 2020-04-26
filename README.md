# PANDA | METRIC

**PANDA | METRIC** is a framework for machine learning based on the concept of metric spaces, which makes it possible to use arbitrary data instead of single numeric values in tables. 

# Intro example

```cpp
//file.cpp
#include "metric.hpp"


int main()
{	
    using namespace metric;
    using namespace std;
    // some data
    vector<vector<int>> A = {
        { 0, 1, 1, 1, 1, 1, 2, 3 },
        { 1, 1, 1, 1, 1, 2, 3, 4 },
        { 2, 2, 2, 1, 1, 2, 0, 0 },
        { 3, 3, 2, 2, 1, 1, 0, 0 },
        { 4, 3, 2, 1, 0, 0, 0, 0 },
        { 5, 3, 2, 1, 0, 0, 0, 0 },
        { 4, 6, 2, 2, 1, 1, 0, 0 },
    };
	
    // some other data
    deque<string> B = {
        "this",
        "test",
        "tests",
        "correlation",
        "of",
        "arbitrary",
        "data",
    };

	
    // bind the types and corresponding metrics with an constructor
    auto mgc_corr = MGC<Euclidean<vector<int>>, Edit<string>>();

    // compute the correlation
    double result = mgc_corr(A, B);

    cout << "Multiscale graph correlation: " << result << endl;
    // 0.0791671 (Time = 7.8e-05s)
    // Rows 2 and 3 are similar in both data sets, so that there is a minimal correlation.

    return 0;
}
```
# Configuration and Installation

METRIC is header-only and can be used without installation. just include the metric.hpp into your file and compile with a C++17 compiler. However, you have to link a BLAS and LAPLACK library like OpenBLAS or MKL.
```
clang++ -std=c++17 -lopenblas file.cpp
```
# Build of examples
The following dependencies are required additionally for building examples and tests:

1. C++ compiler supported C++17
2. Cmake 3.10+ (https://www.cmake.org)
3. Boost libraries (https://www.boost.org)
4. BLAS and LAPACK libraries
5. Libpqxx (for SensorExample)

## Ubuntu
### Install dependencies
On Ubuntu one can install dependencies from Ubuntu repositories.

Note that CMake 3.10+ is available in default repositories since Ubuntu 18.04 LTS.
If you are using an earlier version of Ubuntu, you may need to build CMake from [source code](https://gitlab.kitware.com/cmake/cmake).

```
$ sudo apt install cmake
$ sudo apt install libboost-all-dev
$ sudo apt install libopenblas-dev
$ sudo apt install libpqxx-dev postgresql-server-dev-all
```
### Build

If you are building it from inside a virtual machine, make sure to provision at least **2 GiB of virtual memory** for the build process to succeed.

```
  $ mkdir build
  $ cd build
  $ cmake -DCMAKE_BUILD_TYPE=Release ../
  $ make
```

## macOS

### Install dependencies
Use Homebrew tool to install dependencies

```
$ brew install cmake
$ brew install boost
$ brew install openblas
$ brew install libpqxx
```

### Build
```
  $ mkdir build
  $ cd build
  $ cmake -DCMAKE_BUILD_TYPE=Release ../
  $ make
```

## Windows

### Install dependencies
On Windows one can install some dependencies using vcpkg (https://github.com/microsoft/vcpkg)

```
> vcpkg install --triplet=x64-windows-static libpqxx
> vcpkg install --triplet=x64-windows-static boost
```
Install Intel MKL Library (https://software.intel.com/en-us/mkl/choose-download/windows)

### Build
```
> mkdir build
> cd build
> cmake -DCMAKE_TOOLCHAIN_FILE=[path_to_vcpkg_root]\scripts\buildsystems\vcpkg.cmake ../
> cmake --build .
```



# Short introduction

## General Concept
A metric space is defined by a set of records (aka elements, observations, points etc.) and a corresponding metric, which is a distance function with special requirements that can compares records pair-wise about similarity. 

Almost all algorithms of this framework operate on metric spaces. However, in most situations, it is not necessary to build such a metric space explicity.

You just need to configure the algorithm implicitly with the metric for your data.

All algorithms in the METRIC framework are designed to be used in a two-phase approach. First, the algorithm must be configured with the metric and - if also required - training data or additional parameters. After the construction, the object can be used.

```cpp
using namespace metric;

using rec_t = std::vector<double>;

std::vector<rec_t> A = {
        { 0, 1, 1, 1, 1, 1, 2, 3 },
        { 1, 1, 1, 1, 1, 2, 3, 4 },
        { 2, 2, 2, 1, 1, 2, 0, 0 },
        { 3, 3, 2, 2, 1, 1, 0, 0 },
        { 4, 3, 2, 1, 0, 0, 0, 0 },
        { 5, 3, 2, 1, 0, 0, 0, 0 },
        { 4, 6, 2, 2, 1, 1, 0, 0 }}; // a set with seven records

rec_t b = { 2, 8, 2, 1, 0, 0, 0, 0 }; // a single record.

Algorithm<Euclidean<rec_t>> algo(A,2); // configure an algorihm with euclidean metric, training data and an additional parameter

auto result1 = algo(b); // apply the default method
auto result2 = algo.estimate(b); // apply a special method
```
## Explicit metric spaces
METRIC provides three classes to build explicit metric space (DistanceMatrix, KNNGraph and SearchTree). All classes provide the same methods but they have different computational advantages.

```cpp
DistanceMatrix<Manhatten<rec_t>> dMat(A); // build a full pair-wise representation
KNNGraph<Manhatten<rec_t> knnG(A); // build a local and sparse representation
SearchTree<Manhatten<rec_t>> sTree(A); // buid a hirachical organized tree representation

auto distance = dMat(0,3) // accessing the distance value by IDs.
auto nn_ID = sTree.nn(b); //searching for a best matching record of b in the metric space (aka next neighbor)
```

There are two main scenarios, where it make sense to use explict metric spaces.
1) When you want to find a best matching record or subsets
2) when you want to update your metric space in realtime and want to keep the distance computations updated incrementally.

## Distance functions

There is a hugh collection of distance function in METRIC available but you can also use your own functions. The Distance functions library is organized in three groups (random, related and structured). This framwork provides only real metric and pseudo-metric, (also known as semi-metrics), that fullfill the triangle inequality.

https://en.wikipedia.org/wiki/Metric_(mathematics)


### Random metrics 
The corresponding indices of two records are not related to each other. Also the records can have a different sizes. This is normally the case, when the values of a record represent random observations of one dimension.
```
Record 1    [1.3, 7.2, 5.8, 7.3, 2.7, 5.5 ...]
Record 2    [2.1, 6.2, 1.2, 6.2, 2.5  ...]
```

### related metrics 
The values of two records are related, when the order of the indices can be changed without effecting the result of the distance computation. This is normally the case, when each index of a record represents a dimension and the value represents one oberservation. The dimensions are regarded as independed to each other in principle. In comparison to the random metrics, different samples are provided by the records and not by the values in one record.
```
Record 1    [1.3, 7.2, 5.8, 7.3, 2.7, 5.5, ...]
              |    |    |    |    |    |   
Record 2    [2.1, 6.2, 1.2, 6.2, 2.5, 4.2, ...]
```


### k-structured metrics
When the indices in a record have a structural meaning like the bin position in a histogram or the pixel postion in a picture, you normally use a structured metric. Here the known dependency of the dimension is used for better results.
```
Record 1    [1.3, 7.2, 5.8, 7.3, 2.7, 5.5, ...]
             |  X |  X |  X |  X | |  X |
Record 2    [2.1, 6.2, 1.2, 6.2, 2.5, 4.2, ...]
```
In the most case, you will configure an algorithm with a a distance function, but of course you can construct and use a distance function manually.
```cpp
rec_t a = { 1, 2, 5, 3, 1 };
rec_t b = { 2, 2, 1, 3, 2 };

auto distance_function = P_norm<rec_t>(1.5); //configure a Minkowsky metric with p=1.5

double result = distance_function(a, b); //apply the metric
```
It's easy to provide and use your own metric. To do so, you have to define a struct with a methods that overloads the ()-operator and you have to provide the inner value type as Val_T.
```cpp
struct user_euclidean {
    using Val_T = double; // necassary template line.
    double operator()(std::vector<double> a, std::vector<double> b) // overload the ()-operator 
        {
        /* start of implementation */
        double sum=0;
        for (int i =0; i< a.size(); i++){
                sum += std::pow(a[i]-b[i],2);
        }
        return std::sqrt(sum);
        /* end of implementation*/
    }
};
```

## Computing with metric spaces

 Each data set to which a matching metric is assigned, is already a metric space by definition. So you don't have to compute metric spaces explicitly and normally you can use the algorithms with common data input. 

```cpp
std::vector<std::string> B = {
        "this",
        "is",
        "a",
        "vector",
        "of",
        "strings"}; // a set with six records
auto entropy = Entropy<Edit<std::string>>(); //configure the entropy computation

double result = entropy(B); // compute the entropy

```

The meaning of entropy for a metric spaces is different then the classical shannon entropy, that is discrete. The entropy in METRIC is based on the concept of differential entropy and has the meaning of a local degree of freedom in the dataset. It can be regarged alternativly as mean intrinsic dimensionality over the space, no matter how many real dimensions are provided.

Anyway, like correlation in the intro example the entropy gives only an information about the existance of a dependencies in the data. To model this dependency you have to apply one of the provided feature exctractors mechanisms. 

Imaging a setting, where you take a colored 400x300 photos of a scene at random time. In this scene only a door is opened and closed in some intermediate states. All pixels can change from photo to photo, but there are actually only two independed things happening, the door opening angle and the lighting setup show variations. We call this kind of intrinsic dimensionality "features". While the original record has 400*300*3 = 360000 dimension, it has only 2 intrinsic dimension.

It is important to understand, that the concept of metrics space is only applyable, when the amount of recorded dimensions are highly correlated to each other and the intrinsic dimenions are not increasing with the number of records. If this is the case, the curse of dimensionality will prevent any analysis or insights. But that is not a limit of the metric space concept. It is a limit at all.

The methods to extract features are also known as dimension reduction techniques. But this name is a little misguiding, because we are not changing the recorded dimenension, we are extracting only the underlaying features that lead to the recorded dimensions and put them in a new metric space. Or in other word, we filter out the most important information to make further processing easier and to break down complicated dependencies to a human understanding like a 2d-information.

The most aggressiv feature extractor is called clustering. Here a metric space is splitted into several similar groops and only a categorical lable is mapped as feature.

```cpp
auto [data, lables] = loadDataSet("NYC_taxi")
auto cluster = KOC<TWED<rec_t>>(data); //configure the clustering

auto lable = cluster(data[0]); // apply the clustering to first sample
```

A less aggressiv feature extraction is archieved with a continous mapper. 

```cpp
auto [data, lables] = loadDataSet("MNIST");
auto f_extractor = Autoencoder<SSIM<rec_t>>(data); //configure the clustering

auto features = f_extractor(data[0]); // apply the feature extraction to the first sample
```

When you find correlations between metric spaces, you can model them with continous mappers. For example you first extract deterministc features of a picture, then map this space to its denoised manifold and measure the distance to the manifold as outliner distance and then approximate the solution with a neural network for faster computation.

```cpp
using namespace metric;
using namespace std;
using Container = std::vector;
Container<Image> data = loadDataSet("Donuts");

auto hog_extractor = HOG(data[0].width, data[0].hight); // setup histogram of oriented gradients feature extractor

Container<vector<double>> histograms = hog_extractor(data); // compute the histograms

auto outliner_model = Rediff<Euclidean<vector<double>>>(histograms); // compute the manifold of the histogramm space

Container<double> outliners = outliner_model(histograms); // compute the outliner distance for all images

auto model = ESN<Image,double> (data, outliners); // train an approximation model

model.save("myModel.cereal")
```
then load and apply the model:
```cpp
auto model = ESN<Image,double>("myModel.cereal");
double outliner_score = model(Img1); // compute the outline score for an image
```


# Why PANDA | METRIC is awesum

**PANDA | METRIC** extends the capabilities of machine
learning algorithms for variable structured data
types and enables statements to be made about the
relationships between the data in the context of
artificial intelligence. For this purpose, the
recorded data is first modeled using a metric in a
metric space. These spaces can now be related to
each other and simplified without loss of
information. This allows essential information to be
extracted and accessible to the user. In various
modules, the framework offers a collection of
algorithms that are optimized for metric spaces and
accessible via a standardized API. 
 
**PANDA | METRIC** is located in the area of machine learning and artificial intelligence, 
but offers more than a loose collection of optimized and high class algorithms, 
because PANDA | METRIC combines these different algorithms seamless. 
Data Science is no magic, it is all about information theory, statistics and optimization methods and PANDA | METRIC provides 
all you need to generate data-driven added values. All algorithms in data science seems like a huge loosely 
connected family. This framework provides a universal approach that makes it easy to combine these techniques. 
This framework brings all these algorithm under the theory of metric spaces together and guides, how to combine them.  

**PANDA | METRIC** is programmed in modern and template based C++, which allows a comfortable use with
optimal performance at the same time. Compared
to the approach of neural networks, the concept of metric spaces offers considerable advantages especially for industrial applications.

# Full documentation
**[Modules Overview](modules/README.md)**

* **[METRIC | DISTANCE](modules/distance/README.md)** 

* **[METRIC | SPACE](modules/space/README.md)** 

* **[METRIC | CORRELATION](modules/correlation/README.md)** 

* **[METRIC | MAPPING](modules/mapping/README.md)** 

* **[METRIC | TRANSFORM](modules/transform/README.md)** 

* **[METRIC | UTILS](modules/utils/README.md)** 

