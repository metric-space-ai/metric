# PANDA | METRIC

**PANDA | METRIC** is a framework for processing arbitrary data. 
project page: https://panda.technology/de/metric

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
Data Science is no magic, it is all about Information, statistics and optimization and PANDA | METRIC provides 
all you need to generate data-driven added values. All the algorithms in data science seems like a huge loosely 
connected family. This framework provides a universal approach that makes it easy to combine these techniques. 
To do so it brings all these algorithm under one roof together and guides, how to combine them.  

**PANDA |
METRIC** is programmed in modern and template based C++, which allows a comfortable use with
optimal performance at the same time. Compared
to the approach of neural networks, the concept of
metric spaces offers significant advantages for
industrial applications.

_Check the whitepaper for more info: https://github.com/panda-official/metric/blob/master/whitepaper_PANDA_METRIC_EN_07102019.pdf_



## Modules

**PANDA | METRIC** is organized in several submodules. 
 
**METRIC | DISTANCE** provide a extensive collection of metrics,
including factory functions for configuring complex metrics.  
They are organized into severals levels of complexity and aprio knowledge about the data. 
Basically the user give a priori information, how the data should be connected for reason, like 
a picuture is a 2d array of pixels. A metric type is basically a function, which compares 
two samples of data and gives back the numeric distance between them.
 
**METRIC | SPACE** stores and organizes distances. It provides classes to connect and relate data that are 
kind of the same observations in principle. And it include basic operations
such as the search for neighboring elements. If you can compare two entries all entries are automatically 
related through the d-representations in a metric space. 
 
**METRIC | MAPPING** contains various algorithms that can
‘calculate’ metric spaces or map them into equivalent
metric spaces. 
In general, you can regard all of the algorithms in METRIC | MAPPING as mapper from one metric space into 
a new one, usually with the goal to reduce complexity (like clustering or classifying) or to fill missing 
information (like predicting). Also values can be bidirectionally reconstructed from
reduced space using the reverse decoder. In addition, unwanted features can be removed from the space
for further evaluation. In this way, the user brings in his
a priori knowledge and understandings and on the other hand
his a priori influence causes instead of causing a loss
of information by an autonomous programming of this
knowledge. 
 
**METRIC | TRANSFORM** provides deterministic algorithms that transfer data
element by element into another metric space, e.g. from
the time to the frequency domain.
This is often useful for complexity reduction as preprocessing step. A distinction can be
made between lossy compression and completely reversible
methods. In general, however, these are proven,
deterministic methods such as wavelets.
 
**METRIC | CORRELATION** offers functions
to calculate a correlation coefficient of two metric spaces
and thus to determine the dependency of two arbitrary
data sets. When METRIC | MAPPING is used to quantify and measure 
relations in data, METRIC | CORRELATION is used to find relations between metric spaces at all. 
 
**METRIC | UTILS** contains algorithms which are not
metric either, but which can be easily combined. On the
one hand, there is a high-performance in-memory
crossfilter. This allows a piecewise, UI supported and
interactive filtering of the patterns from the results of the
operations in real time. On the other hand, the METRIC | UTILS
module also contains a nonlinear and nonparametric
signifcance test for independent features (PMQ) of a
metric space that were obtained by mapping.




**Function Calls Cpp**

| Module      | Constructor                                                                                            | ()-Operator                                                | Encode                                | Decode                                |
|-------------|--------------------------------------------------------------------------------------------------------|------------------------------------------------------------|---------------------------------------|---------------------------------------|
| Example     | auto functor = metric::myClass<data>(par1,[par_optional1, par_optional2]);                             | auto result = functor(dataA, dataB, par1, [par_optional]); | auto result1 = functor.encode(dataA); | auto result2 = functor.decode(dataB); |
| Correlation | auto functor = MGC<None, metric::Euclidean<double>, None, metric::Euclidean<double>>(metric1, metric2) | auto result = functor(dataA, dataB)                        | -                                     | -                                     |
| Distance    | auto functor = metric::VOI_kl(3, 2);                                                                   | auto result = functor(dataA, dataB)                        | -                                     | -                                     |
| Distance    | auto functor = metric::VOI_kl<double>(3, 2);                                                           | auto result = functor(dataA, dataB)                        | -                                     | -                                     |
| Distance    | auto functor = metric::sorensen                                                                        | auto result = functor(dataA, dataB)                        | -                                     | -                                     |
| Distance    | auto functor = metric::Euclidean<double>()                                                             | auto result = functor(dataA, dataB)                        | -                                     | -                                     |
| Distance    | auto functor = metric::Manhatten<double>()                                                             | auto result = functor(dataA, dataB)                        | -                                     | -                                     |
| Distance    | auto functor = metric::P_norm<double>(1)                                                               | auto result = functor(dataA, dataB)                        | -                                     | -                                     |
| Distance    | auto functor = metric::Euclidean_thresholded<double>(1, 3)                                             | auto result = functor(dataA, dataB)                        | -                                     | -                                     |
| Distance    | auto functor = metric::Cosine<double>()                                                                | auto result = functor(dataA, dataB)                        | -                                     | -                                     |
| Distance    | auto functor = metric::Chebyshev<double>()                                                             | auto result = functor(dataA, dataB)                        | -                                     | -                                     |
| Distance    | auto functor = metric::EMD<double>(cost_mat, max_cost)                                                 | auto result = functor(dataA, dataB)                        | -                                     | -                                     |
| Distance    | auto functor = metric::Edit<char>                                                                      | auto result = functor("asdsd", "dffdf")                    |                                       |                                       |
| Distance    | auto functor = metric::SSIM<double, std::vector<double>>(100, 1)                                       | auto result = functor(img1, img2)                          |                                       |                                       |
| Distance    | auto functor = metric::TWED<double>(1, 2)                                                              | auto result = functor(dataA, dataB)                        |                                       |                                       |
| Distance    | auto functor = metric::entropy<std::vector<std::vector<double>>, Manhatten<double>>                    | auto result = functor(dataA)                               |                                       |                                       |

**Function Calls Python**

| Module      | Constructor                                                                                | ()-Operator                                               | Encode                          | Decode                          |
|-------------|--------------------------------------------------------------------------------------------|-----------------------------------------------------------|---------------------------------|---------------------------------|
| Example     | functor = myClass(data, par1 ,[par_optional1, par_optional2])                              | result = functor(dataA, dataB, par1, [par_optional])      | result1 = functor.encode(dataA) | result2 = functor.decode(dataB) |
| Correlation | functor = partial(correlation.MGC, metric1=metric.Euclidean(), metric2=metric.Euclidean()) | auto result = functor(dataA, dataB)                       | -                               | -                               |
| Distance    | functor = distance.VOI(k=3, logbase=2)                                                     | result = functor(dataA, dataB)                            | -                               | -                               |
| Distance    | functor = distance.VOI(k=3, logbase=2)                                                     | result = functor(dataA, dataB)                            | -                               | -                               |
| Distance    | functor = distance.sorensen                                                                | result = functor(dataA, dataB)                            | -                               | -                               |
| Distance    | functor = distance.Euclidean()                                                             | result = functor(dataA, dataB)                            | -                               | -                               |
| Distance    | functor = distance.Manhatten()                                                             | result = functor(dataA, dataB)                            | -                               | -                               |
| Distance    | functor = distance.P_norm(p=1)                                                             | result = functor(dataA, dataB)                            | -                               | -                               |
| Distance    | functor = distance.Euclidean_thresholded(thres=1, factor=3)                                | result = functor(dataA, dataB)                            | -                               | -                               |
| Distance    | functor = distance.Cosine()                                                                | result = functor(dataA, dataB)                            | -                               | -                               |
| Distance    | functor = distance.Chebyshev()                                                             | result = functor(dataA, dataB)                            | -                               | -                               |
| Distance    | functor = distance.EMD(cost_mat, max_cost)                                                 | result = functor(dataA, dataB)                            | -                               | -                               |
| Distance    | functor = distance.Edit()                                                                  | result = functor("asdsd", "dffdf")                        | -                               | -                               |
| Distance    | functor = distance.SSIM(dynamic_range=100, masking=1)                                      | result = functor(img1, img2)                              | -                               | -                               |
| Distance    | functor = distance.TWED(penalty=1, elastic=2)                                              | result = functor(dataA, dataB)                            | -                               | -                               |
| Distance    | functor = partial(distance.entropy, metric=Manhatten(), k=3, logbase=20)                   | result = functor(dataA)                                   | -                               | -                               |
| Distance    | functor = distance.Kohonen(train_data, w, h)                                               | result = functor(sample1, sample2)                        |                                 |                                 |
| Mapping     | functor = mapping.DSPCC(dataset, n_features=1)                                             | -                                                         | functor.encode(data)            | result = functor.decode(codes)  |
| Mapping     | functor = mapping.KOC_factory(w, h)                                                        | koc = functor(samples, num_clusters)                      |                                 |                                 |
| Mapping     | -                                                                                          | -                                                         | result = functor.encode(sample) | result = functor.decode(sample) |
| Mapping     | functor = mapping.dbscan                                                                   | assignments, seeds, counts = functor(matrix, eps, minpts) | -                               | -                               |
| Space       | functor = space.Matrix(data, Euclidean())                                                  | result = functor(i, j)                                    | -                               | -                               |
| Space       | -                                                                                          | -                                                         | -                               | -                               |
| Transform   | functor = partial(transform.dwt, wavelet_type=3)                                           | result = functor(a)                                       | -                               | -                               |
| Transform   | functor = partial(transform.idwt, wavelet_type=1, lx=3)                                    | result = functor(a, b)                                    | -                               | -                               |
| Transform   | functor = partial(transform.wmaxlev, wavelet_type=t)                                       | result = functor(size_x)                                  | -                               | -                               |
| Utils       | functor = partial(utils.sparsify_effective_resistance, ep=0.1)                             | result = functor(data)                                    | -                               | -                               |
| Utils       | functor = partial(utils.sparsify_spanning_tree, minimum=False)                             | result = functor(data)                                    | -                               | -                               |
| Utils       |                                                                                            |                                                           |                                 |                                 |
| Mapping     | functor = partial(mapping.ESN(w_size=400).train(slices, target).predict)                   | result = functor(slices)                                  | -                               | -                               |
| Mapping     | functor = partial(mapping.affprop, preference=1.0, maxiter=100)                            | result = functor(data)                                    | -                               | -                               |
| Mapping     | -                                                                                          | -                                                         | -                               | -                               |
| Mapping     | -                                                                                          | -                                                         | -                               | -                               |
| Mapping     | functor = partial(mapping.kmeans, maxiter=100, distance_measure='manhatten')               | result = functor(data, k)                                 | -                               | -                               |
| Mapping     | functor = mapping.kmedoids                                                                 | result = functor(data, k)                                 | -                               | -                               |
| Utils       | -                                                                                          | -                                                         | -                               | -                               |
| Mapping     | -                                                                                          | -                                                         | -                               | -                               |
