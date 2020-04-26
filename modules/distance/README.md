# Distances
A distance function d(a,b) must fullfill four requriements to be metric.
1. d(a, b) ≥ 0 (non-negativity)
2. d(a, b) = 0 if and only if a = b (identity of indiscernibles)
3. d(a, b) = d(b, a) (symmetry)
4. d(a, b) ≤ d(a, c) + d(c, b) (triangle inequality)

When 2. holds only d(a, b) = 0 but not the identity of indisccernibles it is a semi-metric or pseudo-metric. When 4. is not valid then it is only a distance but not a metric. 
Only "real" metrics and pseudo-metrics are provided in the library. Pseudo-metrics are used for random data, because there is not deterministic indentity 
In principle, it is possible to use an arbitrary distance definition for some of the algorihm, but it is conceptual very invalid to do so and no garantees can be given.

# Random metrics
## The cdf family
*cdf = cummulative distribution function*
### Wasserstein (L1 norm on cdf)
This is the equivalent for the Manhatten metric for random sampled data, also know as Wasserstein distance.
### Cramér-von Mises (L2 norm on cdf)
This is the equivalent of the Euclidean metric for random sampled data.
### Kolmogorov-Smirnov (L∞ norm on cdf)
This is the equilvalent for the Chebychev metric for random sampled data.

## The entropy family
### Variation of Information
This metric uilizes the entropy to compute a pseudo-metric .

# Related metrics
## The norm family
Based on the concept of norm induced metric spaces, one can derive serveral distance functions.
### Euclidean (L2 norm, geometric difference)
In the two-dimensional plane or in three-dimensional space, the Euclidean distance d(a,b) corresponds to the distance between the points a and b, that can be computed by the law of pythagoras. In the more general case of n-dimensions, the euclidean distance is defined by the L2 Norm of the difference vector. If the points a and b are defined by the coordinates a=(a_1,...,a_n) and b=(b_1,...,b_n), then the equations leads to the distance:
d(a,b)=sqrt( (a_1 - b_1)^2 + ... + (a_n - b_n)^2 )
https://en.wikipedia.org/wiki/Euclidean_distance

Example
```cpp
std::vector<double> v0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
std::vector<double> v1 = { 1, 1, 1, 1, 1, 2, 3, 4 };

metric::Euclidean<std::vector<double>> euclideanDistance;
auto result = euclideanDistance(v0, v1);
// result = 2

```
### Manhatten (L1 norm, absolute difference)
The Manhatten metric is also known as rectilinear distance, L1 distance, L1 norm, snake distance, city block distance, taxicab distance or Manhattan length. It is the sum of the absolute difference of the vector entries.
d(a,b)= abs(a_1 - b_1) + ... + abs(a_n - b_n) )
https://en.wikipedia.org/wiki/Taxicab_geometry


### Chebychev (L∞ norm, maximum difference)
The Chebyshev distance (also known as Tchebychev distance, maximum metric, or L∞ metric is defined as the distance between two vectors, that is the greatest of their differences along any coordinate dimension.
d(a,b)= max(abs(a_1 - b_1), ..., abs(a_n - b_n) )
https://en.wikipedia.org/wiki/Chebyshev_distance

### Minkowski (Lp norm)

The Minkowski distance can be considered as a generalization of the Euclidean, Manhattan and Chebychev distance. The parameter p defines the metric. p=Inf is Chebychev metric, p=1.0 in the Manhatten metric and p=2.0 in the Euclidean metric. The parameter could be freely choosen between [0.0 ... Inf]

d(a,b)=( (a_1 - b_1)^p + ... + (a_n - b_n)^p )^(1/p)

https://en.wikipedia.org/wiki/Minkowski_distance

## The absolute difference family
Several distance in the literatue are based on the Manhatten metric, more precisely on the idea of the absolute difference vector, but utilizes some kind of build-in normalization instead of using pre-scaled values per standardization or after scaled distances by thresholding.

### Canberra (metric)
https://en.wikipedia.org/wiki/Canberra_distance
similiar to Sørensen (Bray-Curtis) distance, but it's known to be very sensitive to small changes near zero. Instead of Sørensen distance, it's a metric.

### Hassanat (metric!)
optimized version of the Wave Hedges metric.
Hassanat  B.  A. Dimensionality  Invariant  Similarity  Measure. J  Am  Sci 2014;10(8):221-226].  (ISSN:  1545-1003
### Ružička (metric!)
The  one-complement of the Ružička index, that is also known as Jaccardized Czekanowsk index

## The Inner Product Family

### Inner Product (metric!)

### Cosine (metric!)
Cosine distance uses the inner product to measures the cosine of the angle between vectors. It is implemented as the metric version, also known as angular distance, and not the cosine similiarty measure
https://en.wikipedia.org/wiki/Cosine_similarity

### Weierstrass (metric!)
### Jaccard (metric!)
https://en.wikipedia.org/wiki/Jaccard_index

### Hellinger (metric!)




# Structured metrics
## The Edit family
The idea of an edit distance is to transform one record to another by basic edit operations. Each basic operation is assign to a cost, for example to shift one entry to the next index. The distance is the minimum possible sum of costs and therefor realted to the optimal transforming path. Here, an optimization problem is solved, for example by dynamical programming.

### Levenshtein
Also known as edit distance, can be used for character arrays aka strings.
### Longest Common Subsequence (LCS)
### Dynamic Time Warping (DTW)
### Time Warp Edit Distance (TWED)
Costs are defined for an y-value and x-value edit, often the time. Therefor it can be used to compare two curves that are not in phase.


## The Earth Mover's Distance
The Earth Mover's distance is some kind of universial metric. the entries of the records are modelled as nodes of a graph. the distance is the minimal cost to flow mass through the edges of the graph. The costs are configured by a ground distance matrix, which represents the basic costs between the nodes, the edge values of the graph.

ground_distance_matrix_for_1dgrid
ground_distance_matrix_for_2dgrid(w,h)

ground_distance_matrix_for_Image(c,w,h)
ground_distance_matrix_for_hog(w,...)


# metric for metric spaces
## Riemannian
A metric space can be represented through a pairwise distance matrix. This matrix is symmetric and positive definite. One can measure the distance between two metric spaces by the Riemannian metric.

coming soon

## Graph Edit
coming soon

## Hausdorf 
coming soon

## Tree Edit
comming soon


# Standardization of a metric (pre-processing)
Often, some form of standardization of the input values can help to balance out the contributions of different scales in the dimensions of the record. The conventional way to do this is to transform the values so they all have the same variance of 1 and at the same time to centre the values at their means.

To etimate the mean and the standard deviation the metric and build with a referenz data set, which provides samples of the expected data. The mean and standard deviation are computed from that set and stored inside the distance function. The transformation of the value it done on the fly, when the metric is called.
standardized value = (original value – mean) / standard deviation

The Standardization works as wrapper for an abritrary metric.

Example
```cpp
std::vector<double> reference_sample = {
        { 0, 1, 1, 0.01, 1, 1, 200, 3 },
        { 1, 1, 1, 0.01, 1, 2, 300, 4 },
        { 2, 2, 2, 0.01, 1, 2, 0, 0 },
        { 3, 3, 2, 0.02, 1, 1, 0, 0 },
        { 4, 3, 2, 0.01, 0, 0, 0, 0 },
        { 5, 3, 2, 0.01, 0, 0, 0, 0 },
        { 4, 6, 2, 0.02, 1, 1, 0, 0 }};

metric::standardized<Euclidean<double>> s_euclidean_distance(reference_sample);

double result = s_euclidean_distance(A[0], A[4]);
```

However, the results are not necessary better by standardization, because outliners could have a huge impact and lead to a collapse of the variance.


# Thresholdding of a metric (post-processing)
For high dimensional vectors, it could make sense to non-linear deform the resulting distance of a metric by hard or soft clipping the distance value. Additionally the distance value can be scaled below the threhshold to clipp faster or slower againt the threshold. Clipping leads to better computation times, when used for the EMD ground distance for example.

Example
```cpp
metric::hard_clipped<metric::Euclidean<double>> euclidean_thresholded(8.0/*,1.0*/);
auto result1 = euclidean_thresholded(v0, v1);
```
Instead of hard clipping the values, you can use a continous saturation, that is modeled with an exponential function.
```cpp
metric::soft_clipped<metric::Euclidean<double>> euclidean_scaled_and_limited(8.0 ,0.8, 2.0);
auto result2 = euclidean_scaled_and_limited(v0, v1);
```
The threshold [0.01..0.99] is realated to the max_distance value and defines the percentage level where the saturation starts. Below that point, the value is scaled linearly with the scal factor. The scal factor must be positve. values greater one will shorten the range until saturation takes place and a value less one will longer the range until the limitting starts.

|  standard     |    hard_clipped (max_distance = 8)  | soft_clipped (max_distance = 8, thresh = 0.8, scal = 2)|
| --- | --- | --- |
|0,00	|0,00	|0,00|
|1,00	|1,00	|2,00|
|2,00	|2,00	|4,00|
|3,00	|3,00	|6,00|
|4,00	|4,00	|7,41|
|5,00	|5,00	|7,83|
|6,00	|6,00	|7,95|
|7,00	|7,00	|7,99|
|8,00	|8,00	|8,00|
|9,00	|8,00	|8,00|
|10,00	|8,00	|8,00|
