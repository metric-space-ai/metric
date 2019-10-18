# metric

PANDA | METRIC is a framework for processing arbitrary data 
 
https://github.com/panda-official/metric/blob/master/whitepaper_PANDA_METRIC_EN_07102019.pdf
 
 
PANDA | METRIC is located in the area of machine learning and artificial intelligence, but offers more than a loose collection of optimized and high class algorithms, because Metric combines these different algorithms seamless. 
Data Science is no magic, it is all about Information, statistics and optimization and METRIC provides all you need to generate data-driven added values. All the algorithms in data science seems like a huge loosely connected family. This framework provides a universal approach that makes it easy to combine these techniques. To do so it brings all these algorithm under one roof together and guides, how to combine them.  

PANDA | METRIC is organized in several submodules. 
 
METRIC | DISTANCE provide a huge collection of metric types. They are organized into severals levels of complexity and aprio knowledge about the data. Basically the user give a priori information, how the data should be connected for reason, like a picuture is a 2d array of pixels. A metric type is basically a function, which compares two samples of data and gives back the numeric distance between them.  
 
METRIC | SPACE stores and organizes distances. It provides classes to connect and relate data that are kind of the same observations in principle.  If you can compare two entries all entries are automatically related through the d-representations in a metric space. 
 
METRIC | MAPPING provides algorithms to operate on these distance-representations directly or indirectly. In general, you can regard all of the algorithms in METRIC | MAPPING as mapper from one metric space into a new one, usually with the goal to reduce complexity (like clustering or classifying) or to fill missing information (like predicting). Basically these functions models the dependencies for you out of the given metric space, but you can provide different levels of a priori information like labels. Also you can combine different metric spaces. 
 
METRIC | TRANSFORM provides algorithm to transform data in a pure deterministic fashion. This is often useful for complexity reduction as pre processing step. This functions are not driven by the metric space structure and map records only 1:1
 
METRIC | CORRELATION provide algorithm to compute correlations between metric spaces, to check whether it is promising to map this dependencies. When MAPPING is used to quantify and measure relations in data, CORRELATION is used to find relations between metric spaces at all. 
