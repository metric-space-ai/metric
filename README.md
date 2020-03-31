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




