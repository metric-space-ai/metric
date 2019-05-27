# N-API module
This directory contains N-API module for metric_search.

## Build instructions:
```console
$ npm install
```

for test:
```console
$ npm test
```

## Quick instructions:


Module exports next entity:
- metric_search - wrapper for metric_search::Tree object;
- metrics - enum of constants representing metrics from metric_distance library.
           contains next elements : euclidian, euclidian_thresholded, manhatten, cosine, p_norm, twed, emd, ssim ( metric Edit is not supported yet)

### Crossfilter 

To start working with module, we need create object **crossfilter** first.
```javascript
var addon = require(path_to_module);
var data = new addon.metric_search();
```

#### Methods
* insert(scalar) | insert([...]) | insert([[...], ..., [...]]
  insert element into the tree. Element may be a scalar (it internaly convert to vector with one coordinate), vector, and matrix

* insert_if(scalar, threshold) | insert_if([...], threshold) | insert_if([[...], ..., [...]], threshold)
  insert element into the tree if distance between new point and root is greater than threshold.  
  Element may be a scalar (it internaly convert to vector with one coordinate), vector, and matrix.

* erase(scalar) | erase([...]) | erase([[...], ..., [...]])
  Erase element from tree

* get(id) 
  get element by it's ID (wrapper for C++'s operator[] )

* nn(element)
  find nearest neighbourgh for the element

* knn(element, K)
  find K nearest neighbourghs for the element

* rnn(element, distance)
  find all nearest neighbourghs for the element within circle with radius 'distance'
  
* size()
  Return size of the tree
  
* traverse(lambda)
  Traverse the tree and call back lambda function for each node
  
* level_size()
  Return level of the root node
  
* print()
  Return string with ASCII representation of the tree

* print_levels()
  print information about tree levels

* empty()
  return true if tree is empty, false otherwise
  
* check_covering()
  Return true if tree satisfy covering invariant
  
* to_vector()
  Return array of tree elements
  
* to_json() 
  Return string with JSON representation of the tree 
  
* clustering(distribution, IDS, points)
  Return array of arrays with clusters points
