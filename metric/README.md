## Modules

**PANDA | METRIC** is organized in several submodules.

**[METRIC | DISTANCE](distance/README.md)** provide a extensive collection of metrics, including factory functions for
configuring complex metrics.  
They are organized into severals levels of complexity and a prio knowledge about the data. Basically the user give a
priori information, how the data should be connected for reason, like a picuture is a 2d array of pixels. A metric type
is basically a function, which compares two samples of data and gives back the numeric distance between
them. [link](distance/README.md)

**[METRIC | SPACE](space/README.md)** stores and organizes distances. It provides classes to connect and relate data
that are kind of the same observations in principle. And it include basic operations such as the search for neighboring
elements. If you can compare two entries all entries are automatically related through the distance-representations in a
metric space. [link](space/README.md)

**[METRIC | MAPPING](mapping/README.md)** contains various algorithms that can ‘calculate’ metric spaces or map them
into equivalent metric spaces. In general, you can regard all of the algorithms in METRIC | MAPPING as mapper from one
metric space into a new one, usually with the goal to reduce complexity (like clustering or classifying) or to fill
missing information (like predicting). Also values can be bidirectionally reconstructed from reduced space using reverse
decoder. In addition, unwanted features can be removed from the space for further evaluation. In this way, the user
brings in his a priori knowledge and understandings. on the other hand his a priori influence is visible and not causing
a loss of information by an incidental programming of this knowledge. [link] (mapping/README.md)

**[METRIC | TRANSFORM](transform/README.md)** provides deterministic algorithms that transfer data element by element
into another metric space, e.g. from the time to the frequency domain. This is often useful for complexity reduction as
preprocessing step. A distinction can be made between lossy compression and completely reversible methods. In general,
however, these are proven, deterministic methods such as wavelets or physical motivated
fittings.[link](transform/README.md)

**[METRIC | CORRELATION ](correlation/README.md)** offers functions to calculate metric entropy, which gives a measure
of instrinsic local dimensionality and the correlation of two metric spaces and thus to determine the dependency of two
arbitrary data sets. When METRIC | MAPPING is used to quantify and measure relations in data, METRIC | CORRELATION is
used to find relations between metric spaces at all. [link](correlation/README.md)

**[METRIC | UTILS ](utils/README.md)** contains algorithms which are not metric either, but which can be easily
combined. On the one hand, there is a high-performance in-memory crossfilter. This allows a combined filtering of the
patterns from the results of the operations in quasi real time. On the other hand, the METRIC | UTILS module also
contains a nonlinear and nonparametric signifcance test for independent features (PMQ) of a metric space that were
obtained by mapping and more. [link](utils/README.md)


