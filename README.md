# PANDA | METRIC

**PANDA | METRIC** is a framework for machine learning based on the concept of metric spaces, which makes it possible to use arbitrary data instead of numeric tables of data. 

## intro

```cpp
//file.cpp
#include "metric.hpp"

int main()
{	
    // some data
    std::vector<std::vector<int>> A = {
        { 0, 1, 1, 1, 1, 1, 2, 3 },
        { 1, 1, 1, 1, 1, 2, 3, 4 },
        { 2, 2, 2, 1, 1, 2, 0, 0 },
        { 3, 3, 2, 2, 1, 1, 0, 0 },
        { 4, 3, 2, 1, 0, 0, 0, 0 },
        { 5, 3, 2, 1, 0, 0, 0, 0 },
        { 4, 6, 2, 2, 1, 1, 0, 0 },
    };
	
    // some other data
    std::deque<std::string> B = {
        "this",
        "test",
        "tests",
        "correlation",
        "of",
        "arbitrary",
        "data",
    };

	
    // bind the types and corresponding metrics with an constructor
    auto mgc_corr = metric::MGC<std::vector<int>, metric::Euclidian<int>, std::string, metric::Edit<std::string>>();

    // compute the correlation
    auto result = mgc_corr(A, B);

    std::cout << "Multiscale graph correlation: " << result << std::endl;
    // 0.0791671 (Time = 7.8e-05s)
    // Rows 2 and 3 are similar in both data sets, so that there is a minimal correlation.

    return 0;
}
```

compile with
```
clang++ -std=c++17 -lopenblas file.cpp
```

## Overview

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

**PANDA |
METRIC** is programmed in modern and template based C++, which allows a comfortable use with
optimal performance at the same time. Compared
to the approach of neural networks, the concept of metric spaces offers considerable advantages especially for industrial applications.

## Modules

**PANDA | METRIC** is organized in several submodules. 
 
**METRIC | DISTANCE** provide a extensive collection of metrics,
including factory functions for configuring complex metrics.  
They are organized into severals levels of complexity and a prio knowledge about the data. 
Basically the user give a priori information, how the data should be connected for reason, like 
a picuture is a 2d array of pixels. A metric type is basically a function, which compares 
two samples of data and gives back the numeric distance between them.
 
**METRIC | SPACE** stores and organizes distances. It provides classes to connect and relate data that are 
kind of the same observations in principle. And it include basic operations
such as the search for neighboring elements. If you can compare two entries all entries are automatically 
related through the distance-representations in a metric space. 
 
**METRIC | MAPPING** contains various algorithms that can
‘calculate’ metric spaces or map them into equivalent
metric spaces. 
In general, you can regard all of the algorithms in METRIC | MAPPING as mapper from one metric space into 
a new one, usually with the goal to reduce complexity (like clustering or classifying) or to fill missing 
information (like predicting). Also values can be bidirectionally reconstructed from
reduced space using reverse decoder. In addition, unwanted features can be removed from the space
for further evaluation. In this way, the user brings in his
a priori knowledge and understandings. on the other hand
his a priori influence is visible and not causing a loss
of information by an incidental programming of this
knowledge. 
 
**METRIC | TRANSFORM** provides deterministic algorithms that transfer data
element by element into another metric space, e.g. from
the time to the frequency domain.
This is often useful for complexity reduction as preprocessing step. A distinction can be
made between lossy compression and completely reversible
methods. In general, however, these are proven,
deterministic methods such as wavelets or physical motivated fittings.
 
**METRIC | CORRELATION** offers functions
to calculate metric entropy, which gives a measure of instrinsic local dimensionality and the correlation of two metric spaces
and thus to determine the dependency of two arbitrary
data sets. When METRIC | MAPPING is used to quantify and measure 
relations in data, METRIC | CORRELATION is used to find relations between metric spaces at all. 
 
**METRIC | UTILS** contains algorithms which are not
metric either, but which can be easily combined. On the
one hand, there is a high-performance in-memory
crossfilter. This allows a combined filtering of the patterns from the results of the
operations in quasi real time. On the other hand, the METRIC | UTILS
module also contains a nonlinear and nonparametric
signifcance test for independent features (PMQ) of a
metric space that were obtained by mapping and more.




**FUNCTION CALLS**


**Calls METRIC | DISTANCE**

|Class Description                                                                                                                                                 |Language|Constructor                                                           |()-Operator                  |Default Parameters                                    |
|--------------------------------------------------------------------------------------------------------------------------------------------------------|--------|----------------------------------------------------------------------|-----------------------------|------------------------------------------------------|
|VOI normalized                                                                                                                                          |Python  |f = distance.VOI_normalized(k=3, logbase=2)                                                |result = f(dataA, dataB)     |defaults: k=3, logbase=3                              |
|                                                                                                                                                        |C++     |auto f = metric::VOI_kl<double>(3, 2);                                |auto result = f(dataA, dataB)|                                                      |
|Sorensen Distance                                                                                                                                       |Python  |f = distance.sorensen                                                 |result = f(dataA, dataB)     |                                                      |
|                                                                                                                                                        |C++     |auto f = metric::sorensen                                             |auto result = f(dataA, dataB)|                                                      |
|Euclidean Distance Metric                                                                                                                                              |Python  |f = distance.Euclidean()                                              |result = f(dataA, dataB)     |                                                      |
|                                                                                                                                                        |C++     |auto f = metric::Euclidean<double>()                                  |auto result = f(dataA, dataB)|                                                      |
|Manhatten Distance Metric                                                                                                                                               |Python  |f = distance.Manhatten()                                              |result = f(dataA, dataB)     |                                                      |
|                                                                                                                                                        |C++     |auto f = metric::Manhatten<double>()                                  |auto result = f(dataA, dataB)|                                                      |
|Minkowski (L general) Metric (P_norm)                                                                                                                                              |Python  |f = distance.P_norm(p=1)                                              |result = f(dataA, dataB)     |defaults: p=1                                         |
|                                                                                                                                                        |C++     |auto f = metric::P_norm<double>(1)                                    |auto result = f(dataA, dataB)|defaults: p=1                                         |
|Euclidean Metric Threshold                                                                                                                                     |Python  |f = distance.Euclidean_thresholded(thres=1, factor=3)                 |result = f(dataA, dataB)     |defaults: thres=1000, factor=3000                     |
|                                                                                                                                                        |C++     |auto f = metric::Euclidean_thresholded<double>(1, 3)                  |auto result = f(dataA, dataB)|    defaults: thres=1000, factor=3000  |
|Cosine Metric                                                                                                                                                 |Python  |f = distance.Cosine()                                                 |result = f(dataA, dataB)     |                                                      |
|                                                                                                                                                        |C++     |auto f = metric::Cosine<double>()                                     |auto result = f(dataA, dataB)|                                                      |
|Chebyshev Distance Metric (Maximum value distance)                                                                                                                                               |Python  |f = distance.Chebyshev()                                              |result = f(dataA, dataB)     |                                                      |
|                                                                                                                                                        |C++     |auto f = metric::Chebyshev<double>()                                  |auto result = f(dataA, dataB)|                                                      |
|Earth Mover's Distance Metric (EMD)                                                                                                                                                   |Python  |f = distance.EMD(cost_mat, extra_mass_penalty) |result = f(dataA, dataB)     |defaults: cost_mat={}, extra_mass_penalty=-1          |
|                                                                                                                                                        |C++     |auto f = metric::EMD<double>(cost_mat, max_cost)                      |auto result = f(dataA, dataB)|defaults: extra_mass_penalty=-1                       |
|Edit Distance Metric                                                                                                                                                 |Python  |f = distance.Edit()                                                   |result = f("asdsd", "dffdf") |                                                      |
|                                                                                                                                                        |C++     |auto f = metric::Edit<char>                                           |auto result = f("asdsd", "dffdf")|                                                      |
|Structural Similarity Index (SSIM)                                                                                                                                                    |Python  |f = distance.SSIM(dynamic_range=100, masking=1)                       |result = f(img1, img2)       |defaults: dynamic_range=255, masking=2                |
|                                                                                                                                                        |C++     |auto f = metric::SSIM<double, std::vector<double>>(100, 1)            |auto result = f(img1, img2)  |                                                      |
|Time Warp Edit Distance (TWED)                                                                                                                                                    |Python  |f = distance.TWED(penalty=1, elastic=2)                               |result = f(dataA, dataB)     |defaults: penalty=0, elastic=1                        |
|                                                                                                                                                        |C++     |auto f = metric::TWED<double>(1, 2)                                   |auto result = f(dataA, dataB)|                                                      |
|Kohonen Distance Metric                                                                                                                                        |Python  |f = distance.Kohonen(train_data, w, h)                                |result = f(sample1, sample2) |defaults: start_learn_rate=0.8, finish_learn_rate=0.0, iterations=20|
|                                                                                                                                                        |C++     |auto f = metric::kohonen_distance<double>(train_data, w, h)           |auto result = f(sample1, sample2)|defaults: start_learn_rate=0.8, finish_learn_rate=0.0, iterations=20|


**Calls METRIC | SPACE**


|Class Description                                                                                                                                             |Language|Constructor                                                           |()-Operator                  |Parameters                            |
|--------------------------------------------------------------------------------------------------------------------------------------------------------|--------|----------------------------------------------------------------------|-----------------------------|--------------------------------------|
|Distance matrix                                                                                                                                         |Python  |f = space.Matrix(data, Euclidean())                                   |result = f(i, j)             |Default Parameters: data = {}, metric=Euclidean()|
|Distance matrix                                                                                                                                         |C++     |auto f = metric::Matrix<std::vector<double>>(data)                    |auto result = f(i, j)        |constructor defaults: d = Metric() /*template argument*/|
|A Search Tree works like a std-container to store data of some structure. Basically a Metric Search Tree has the same principle as a binary search tree or a kd-tree, but it works for arbitrary data structures. This Metric Search Tree is basically a Cover Tree Implementation. Additionally to the distiance (or similarity) between the data, a covering distance from level to level decides how the tree grows.|Python  |f = space.Tree()                          |auto result = f(i, j)        |                                      |
|                                                                                                                                                        |C++     |auto f = metric::Tree<std::vector<double>>()                                                           |auto result = f(i, j)        |                                      |

**Calls METRIC | MAPPING**


|Class Description                                                                                                                                                 |Language|Constructor                                                           |()-Operator                  |Encode                                |Decode                   |Default Parameters                                                                                                                                                                |
|--------------------------------------------------------------------------------------------------------------------------------------------------------|--------|----------------------------------------------------------------------|-----------------------------|--------------------------------------|-------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|Domain split principle components compressor                                                                                                            |Python  |f = mapping.DSPCC(dataset, n_features=1)                              |-                            |f.encode(data)                        |result = f.decode(codes) |defaults: n_features=1, n_subbands=4, time_freq_balance=0.5, n_top_features=16                                                                                                    |
|                                                                                                                                                        |C++     |auto f = metric::DSPCC<vector<double>, void>(dataset, 1)              |                             |                                      |                         |defaults: n_features=1, n_subbands=4, time_freq_balance=0.5, n_top_features=16                                                                                                    |
|Kohonen Distance Clustering is one of the Neural Network unsupervised learning algorithms. This algorithm is used in solving problems in various areas, especially in clustering complex data sets.|Python  |f = mapping.KOC_factory(w, h)                                         |koc = f(samples, num_clusters) |-                                     |-                        |defaults: nodes_width=5, nodes_height=4, anomaly_sigma=1.0, start_learn_rate=0.8, finish_learn_rate=0, iterations=20, distribution_min=-1, distribution_max=1,  min_cluster_size=1, metric=distance.Euclidean()|
|                                                                                                                                                        |C++     |auto f = mapping::KOC_factory<std::vector<double>, metric::Grid6, metric::Euclidian<double>>(w, h)                                   |auto koc = f(samples, num_clusters)|-                                     |-                        |                                                                                      defaults: nodes_width=5, nodes_height=4, anomaly_sigma=1.0, start_learn_rate=0.8, finish_learn_rate=0, iterations=20, distribution_min=-1, distribution_max=1,  min_cluster_size=1                                                                                            |
|Autoencoder  is an unsupervised artificial neural network that learns how to efficiently compress and encode data then learns how to reconstruct the data back from the reduced encoded representation to a representation that is as close to the original input as possible. It reduces data dimensions by learning how to ignore the noise in the data.|Python  |f  = mapping.Autoencoder()                                            |-                            |result = f.encode(sample)             |result = f.decode(sample)|                                                                                                                                                                                  |
|                                                                                                                                                        |C++     |auto f = metric::Autoencoder<uint8_t, double>()                       |  -                           |auto result = f.encode(sample) |result = f.decode(sample)  |                                                                                                                                                                                  |
|dbscan (Density-based spatial clustering of applications with noise) is a data clustering  non-parametric algorithm. Given a set of points in some space, it groups together points that are closely packed together (points with many nearby neighbors), marking as outliers points that lie alone in low-density regions (whose nearest neighbors are too far away). DBSCAN is one of the most common clustering algorithms and also most cited in scientific literature.|Python  |f = mapping.dbscan                                                    |assignments, seeds, counts = f(matrix, eps, minpts)|-                                     |-                        |                                                                                                                                                                                  |
|dbscan                                                                                                                                                  |C++     |auto f = mapping::dbscan<std::vector<double>>                         |auto result = f(matrix, eps, minpts)|                                      |                         |                                                                                                                                                                                  |
|ESN  (echo state network) is a recurrent neural network with a sparsely connected hidden layer (with typically 1% connectivity). The connectivity and weights of hidden neurons are fixed and randomly assigned|Python  |f = partial(mapping.ESN(w_size=400).train(slices, target).predict)    |result = f(slices)           |-                                     |-                        |defaults: w_size=500, w_connections=10, w_sr=0.6, alpha=0.5, washout=1, beta=0.5                                                                                                  |
|ESN                                                                                                                                                     |C++     |auto f = metric::ESN<std::vector<double>, Euclidean<double>>()        |-                            |-                                     |                         | defaults: w_size=500, w_connections=10, w_sr=0.6, alpha=0.5, washout=1, beta=0.5 |
|affprop (Affinity propagation) is a clustering algorithm based on message passing between data points. Similar to K-medoids, it looks at the (dis)similarities in the data, picks one exemplar data point for each cluster, and assigns every point in the data set to the cluster with the closest exemplar.|Python  |f = partial(mapping.affprop, preference=1.0, maxiter=100);            |result = f(data)             |-                                     |-                        |defaults: preference=0.5, maxiter=200, tol=1.0e-6, damp=0.5                                                                                                                       |
|affprop                                                                                                                                                 |C++     |auto f = mapping.affprop<std::vector<double>>|auto result = f(data)        |                                      |                         |defaults: preference=0.5, maxiter=200, tol=1.0e-6, damp=0.5                                                                                                                                                                   |
|kmeans  clustering is a method of vector quantization, that aims to partition n observations into k clusters in which each observation belongs to the cluster with the nearest mean (cluster centers or cluster centroid), serving as a prototype of the cluster.|Python  |f = partial(mapping.kmeans, maxiter=100, distance_measure='manhatten') |result = f(data, k)          |-                                     |-                        |defaults: maxiter=200, distance_measure='euclidean', random_seed=-1                                                                                                               |
|kmeans                                                                                                                                                  |C++     |auto f = metric::kmeans                                               |auto result = f(data, k)     |                                      |                         |defaults: maxiter=200, distance_measure='euclidean', random_seed=-1|
|kmedoids is a classical partitioning technique of clustering, which clusters the data set of n objects into k clusters, with the number k of clusters assumed known a priori (which implies that the programmer must specify k before the execution of the algorithm).|Python  |f = mapping.kmedoids                                                  |result = f(data, k)          |-                                     |-                        |                                                                                                                                                                                  |
|kmedoids                                                                                                                                                |C++     |auto f = metric::kmedoids<std::vector<double>>                        |auto result = f(data, k)     |                                      |                         |                                                                                                                                                                                  |


**Calls METRIC | TRANSFORM**


|Class Description                                                                                                                                       |Language|Constructor                                                           |()-Operator                  |
|--------------------------------------------------------------------------------------------------------------------------------------------------------|--------|----------------------------------------------------------------------|-----------------------------|
|Discrete wavelet transform(DWT) is any wavelet transform for which the wavelets are discretely sampled. As with other wavelet transforms, a key advantage it has over Fourier transforms is temporal resolution: it captures both frequency and location information.|Python  |f = partial(transform.dwt, wavelet_type=3)                            |result = f(a)                |
|                                                                                                                                                        |C++     |auto f = metric::dwt<std::vector<double>>                             |auto result = f(a)           |
|The idwt command performs a single-level one-dimensional wavelet reconstruction.                                                                        |Python  |f = partial(transform.idwt, wavelet_type=1, lx=3)                     |result = f(a, b)             |
|                                                                                                                                                        |C++     |auto f =metric::idwt<std::vector<double>>                             |auto result = f(a, b)        |
|wmaxlev returns the maximum level L possible for a wavelet decomposition of a signal or image of size size_x. The maximum level is the last level for which at least one coefficient is correct.|Python  |f = partial(transform.wmaxlev, wavelet_type=t)                        |result = f(size_x)           |
|                                                                                                                                                        |C++     |auto f = metric::wmaxlev                                              |auto result = f(size_x)      |

**Calls METRIC | CORRELATION**

|Class Description                                                                                                                                       |Language|Constructor                                                           |()-Operator                  |Estimate                              |Parameters                                                                                                                                                   |
|--------------------------------------------------------------------------------------------------------------------------------------------------------|--------|----------------------------------------------------------------------|-----------------------------|--------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------|
|Use MGC (Multiscale Graph Correlation) as correlation coefficient to find nonlinear dependencies in data sets. It is optimized for small data set sizes.|C++     |auto f = metric::MGC<void, Euclidian<double>,void, Manhatten<float>>()|auto result = f(dataA, dataB)|auto result= f.estimate(dataA, dataB) |Default parameters constructor: metric1=Euclidean(), metric2=Euclidean() Default parameters estimate: b_sample_size=250, threshold=0.05, max_iterations=1000|
|                                                                                                                                                        |Python  |f = correlation.MGC()                                                 |auto result = f(dataA, dataB)|auto result= f.estimate(dataA, dataB) | Default parameters constructor: metric1=Euclidean(), metric2=Euclidean();   Default parameters estimate: b_sample_size=250, threshold=0.05, max_iterations=1000                                                                                                                                                         |
|Calculate metric entropy, which gives a measure of instrinsic local dimensionality                                                                      |C++     |auto f = metric::Entropy<void, Manhatten<double>>(metric, k, p, exp)  |auto result = f(dataA)       |auto result= f.estimate(data)         |Default parameters constructor: metric=Euclidean(), k=7, p=70, exp=False Default parameters estimate: samples_size=250, threshold=0.05, max_iterations=100     |
|                                                                                                                                                       |Python  |f = distance.Entropy(metric=Manhatten(), k=3, p=30)                   |result = f(dataA)            |result = f.estimate(data)             |  Default parameters constructor: metric=Euclidean(), k=7, p=70, exp=False Default parameters estimate: samples_size=250, threshold=0.05, max_iterations=100                                                                                                                                                            |


**Calls METRIC | UTILS**


|Class Description                                                                                                                                                  |Language|Constructor                                                           |()-Operator                  |Parameters                                            |
|--------------------------------------------------------------------------------------------------------------------------------------------------------|--------|----------------------------------------------------------------------|-----------------------------|------------------------------------------------------|
|The goal of resistance sparsification of graphs  is to find a sparse subgraph (with reweighted edges) that approximately preserves the effective resistances between every pair of nodes. |C++     |auto f = metric::sparsify_effective_resistance                        |auto result = f(data)        |Default params: ep=0.3, max_conc_const=4.0, jl_fac=4.0|
|                                                                                                                                                        |Python  |f = partial(utils.sparsify_effective_resistance, ep=0.1)              |result = f(data)             |Default params: ep=0.3, max_conc_const=4.0, jl_fac=4.0|
|A minimum spanning tree is a graph consisting of the subset of edges which together connect all connected nodes, while minimizing the total sum of weights on the edges. This is computed using the Kruskal algorithm.|C++     |auto f = metric::sparsify_spanning_tree                               |auto result = f(data)        |Default params: minimum=true                          |
|                                                                                                                                                        |Python  |f = partial(utils.sparsify_spanning_tree, minimum=False)              |result = f(data)             |Default params: minimum=True                          |
