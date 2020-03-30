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

	
    // bind the types and metrics with an constructor to return a functor
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




**FUNCTION CALLS C++**



**Module**|**File**|**Class**|**Constructor**|**()-Operator**|**Encode**|**Decode**
:-----:|:-----:|:-----:|:-----:|:-----:|:-----:|:-----:
Correlation|mgc.hpp|MGC|auto functor = MGC<None, metric::Euclidean<double>, None, metric::Euclidean<double>>(metric1, metric2)|auto result = functor(dataA, dataB)|-|-
Distance|VOI.hpp|VOI\_kl|auto functor = metric::VOI\_kl(3, 2);|auto result = functor(dataA, dataB)|-|-
Distance|VOI.hpp|VOI\_normalized|auto functor = metric::VOI\_kl<double>(3, 2);|auto result = functor(dataA, dataB)|-|-
Distance|L1.hpp|sorensen|auto functor = metric::sorensen|auto result = functor(dataA, dataB)|-|-
Distance|Standards.hpp|Euclidean|auto functor = metric::Euclidean<double>()|auto result = functor(dataA, dataB)|-|-
Distance|Standards.hpp|Manhatten|auto functor = metric::Manhatten<double>()|auto result = functor(dataA, dataB)|-|-
Distance|Standards.hpp|P\_norm|auto functor = metric::P\_norm<double>(1)|auto result = functor(dataA, dataB)|-|-
Distance|Standards.hpp|Euclidean\_threshold|auto functor = metric::Euclidean\_thresholded<double>(1, 3)|auto result = functor(dataA, dataB)|-|-
Distance|Standards.hpp|Cosine|auto functor = metric::Cosine<double>()|auto result = functor(dataA, dataB)|-|-
Distance|Standards.hpp|Chebyshev|auto functor = metric::Chebyshev<double>()|auto result = functor(dataA, dataB)|-|-
Distance|EMD.hpp|EMD|auto functor = metric::EMD<double>(cost\_mat, max\_cost)|auto result = functor(dataA, dataB)|-|-
Distance|Edit.hpp|Edit|auto functor = metric::Edit<char>|auto result = functor("asdsd", "dffdf")| | 
Distance|SSIM.hpp|SSIM|auto functor = metric::SSIM<double, std::vector<double>>(100, 1)|auto result = functor(img1, img2)| | 
Distance|TWED.hpp|TWED|auto functor = metric::TWED<double>(1, 2)|auto result = functor(dataA, dataB)| | 
Distance|VOI.hpp|entropy|auto functor = metric::entropy<std::vector<std::vector<double>>, Manhatten<double>>|auto result = functor(dataA)| | 
Distance|kohonen\_distance.hpp|kohonen\_distance|auto functor = metric::kohonen\_distance<double>(train\_data, w, h)|auto result = functor(sample1, sample2)| | 
Example|myFile.hpp|myClass|auto functor = metric::myClass<data>(par1,[par\_optional1, par\_optional2]);|auto result = functor(dataA, dataB, par1, [par\_optional]);|auto result1 = functor.encode(dataA);|auto result2 = functor.decode(dataB);
Mapping|DSPCC.hpp|DSPCC| | | | 
Mapping|KOC.hpp|KOC|auto functor = mapping::KOC\_factory(w, h)(samples, num\_clusters)|-|-|-
Mapping|autoencoder.hpp|Autoencoder| | | | 
Mapping|dbscan.hpp|dbscan|auto functor = mapping::dbscan<std::vector<double>>|auto result = functor(matrix, eps, minpts)| | 
Mapping|ESN.hpp|ESN|-|-| | 
Mapping|affprop.hpp|affprop|auto functor = std::bind(mapping.affprop<std::vector<double>>, std::\_1, 1.0, 100)|auto result = functor(data)| | 
Mapping|ensembles.hpp|Boosting| | | | 
Mapping|ensembles.hpp|Bagging| | | | 
Mapping|kmeans.hpp|kmeans|auto functor = std::bind(metric::kmeans, std::\_1, std::\_2, 100, "manhatten")|auto result = functor(data, k)| | 
Mapping|kmedios.hpp|kmedios|auto functor = metric::kmedoids<std::vector<double>>|auto result = functor(data, k)| | 
Mapping|DT.hpp|DT| | | | 
Space|matrix.hpp|Matrix|auto functor = metric::Matrix<std::vector<double>>(data)|auto result = functor(i, j)| | 
Space|tree.hpp|Tree| | | | 
Transform|wavelet\_new.hpp|dwt|auto functor = std::bind(metric::dwt<std::vector<double>>, std::\_1, 3)|auto result = functor(a)|-|-
Transform|wavelet\_new.hpp|idwt|auto functor = std::bind(metric::idwt<std::vector<double>>, std::\_1, std::\_2, 1, 3)|auto result = functor(a, b)| | 
Transform|wavelet\_new.hpp|wmaxlev|auto functor = std::bind(metric::wmaxlev, std::\_1, t)|auto result = functor(size\_x)| | 
Utiils|crossfilter.hpp|dimension| | | | 
Utils|sparsify.hpp|sparsify\_effective\_resistance|auto functor = std::bind(metric::sparsify\_effective\_resistance, std::\_1, 0.1)|auto result = functor(data)| | 
Utils|sparsify.hpp|sparsify\_spanning\_tree|auto functor = std::bind(metric::sparsify\_spanning\_tree, std::\_1, false)|auto result = functor(data)| | 
 
 
 
 
**FUNCTION CALLS PYTHON**



**Module**|**File**|**Class**|**Constructor**|**()-Operator**|**Encode**|**Decode**
:-----:|:-----:|:-----:|:-----:|:-----:|:-----:|:-----:
Correlation|mgc.hpp|MGC|functor = partial(correlation.MGC, metric1=metric.Euclidean(), metric2=metric.Euclidean())|auto result = functor(dataA, dataB)|-|-
Distance|VOI.hpp|VOI\_kl|functor = distance.VOI(k=3, logbase=2)|result = functor(dataA, dataB)|-|-
Distance|VOI.hpp|VOI\_normalized|functor = distance.VOI(k=3, logbase=2)|result = functor(dataA, dataB)|-|-
Distance|L1.hpp|sorensen|functor = distance.sorensen|result = functor(dataA, dataB)|-|-
Distance|Standards.hpp|Euclidean|functor = distance.Euclidean()|result = functor(dataA, dataB)|-|-
Distance|Standards.hpp|Manhatten|functor = distance.Manhatten()|result = functor(dataA, dataB)|-|-
Distance|Standards.hpp|P\_norm|functor = distance.P\_norm(p=1)|result = functor(dataA, dataB)|-|-
Distance|Standards.hpp|Euclidean\_threshold|functor = distance.Euclidean\_thresholded(thres=1, factor=3)|result = functor(dataA, dataB)|-|-
Distance|Standards.hpp|Cosine|functor = distance.Cosine()|result = functor(dataA, dataB)|-|-
Distance|Standards.hpp|Chebyshev|functor = distance.Chebyshev()|result = functor(dataA, dataB)|-|-
Distance|EMD.hpp|EMD|functor = distance.EMD(cost\_mat, max\_cost)|result = functor(dataA, dataB)|-|-
Distance|Edit.hpp|Edit|functor = distance.Edit()|result = functor("asdsd", "dffdf")|-|-
Distance|SSIM.hpp|SSIM|functor = distance.SSIM(dynamic\_range=100, masking=1)|result = functor(img1, img2)|-|-
Distance|TWED.hpp|TWED|functor = distance.TWED(penalty=1, elastic=2)|result = functor(dataA, dataB)|-|-
Distance|VOI.hpp|entropy|functor = partial(distance.entropy, metric=Manhatten(), k=3, logbase=20)|result = functor(dataA)|-|-
Distance|kohonen\_distance.hpp|kohonen\_distance|functor = distance.Kohonen(train\_data, w, h)|result = functor(sample1, sample2)| | 
Example|myFile.hpp|myClass|functor = myClass(data, par1 ,[par\_optional1, par\_optional2])|result = functor(dataA, dataB, par1, [par\_optional])|result1 = functor.encode(dataA)|result2 = functor.decode(dataB)
Mapping|DSPCC.hpp|DSPCC|functor = mapping.DSPCC(dataset, n\_features=1)|-|functor.encode(data)|result = functor.decode(codes)
Mapping|KOC.hpp|KOC|functor = mapping.KOC\_factory(w, h)(samples, num\_clusters)|-|-|-
Mapping|autoencoder.hpp|Autoencoder|-|-|result = functor.encode(sample)|result = functor.decode(sample)
Mapping|dbscan.hpp|dbscan|functor = mapping.dbscan|assignments, seeds, counts = functor(matrix, eps, minpts)|-|-
Mapping|ESN.hpp|ESN|functor = partial(mapping.ESN(w\_size=400).train(slices, target).predict)|result = functor(slices)|-|-
Mapping|affprop.hpp|affprop|functor = partial(mapping.affprop, preference=1.0, maxiter=100)|result = functor(data)|-|-
Mapping|ensembles.hpp|Boosting|-|-|-|-
Mapping|ensembles.hpp|Bagging|-|-|-|-
Mapping|kmeans.hpp|kmeans|functor = partial(mapping.kmeans, maxiter=100, distance\_measure='manhatten')|result = functor(data, k)|-|-
Mapping|kmedios.hpp|kmedios|functor = mapping.kmedoids|result = functor(data, k)|-|-
Mapping|DT.hpp|DT|-|-|-|-
Space|matrix.hpp|Matrix|functor = space.Matrix(data, Euclidean())|result = functor(i, j)|-|-
Space|tree.hpp|Tree|-|-|-|-
Transform|wavelet\_new.hpp|dwt|functor = partial(transform.dwt, wavelet\_type=3)|result = functor(a)|-|-
Transform|wavelet\_new.hpp|idwt|functor = partial(transform.idwt, wavelet\_type=1, lx=3)|result = functor(a, b)|-|-
Transform|wavelet\_new.hpp|wmaxlev|functor = partial(transform.wmaxlev, wavelet\_type=t)|result = functor(size\_x)|-|-
Utils|sparsify.hpp|sparsify\_effective\_resistance|functor = partial(utils.sparsify\_effective\_resistance, ep=0.1)|result = functor(data)|-|-
Utils|sparsify.hpp|sparsify\_spanning\_tree|functor = partial(utils.sparsify\_spanning\_tree, minimum=False)|result = functor(data)|-|-
Utils|crossfilter.hpp|dimension|-|-|-|-
