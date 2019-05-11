# N-API module
This directory contains N-API module for crossfilter.

## Build instructions:
```console
$ npm install
```

for test:
```console
$ npm test
```
also there is a benchmark:
```console
$ node test/benchmark.js
```

## Quick instructions:


Module exports next entity:
- crossfilter - wrapper for cross::filter object;
- ctypes - enum of constants representing C++ fundamental types.
           contains next elements : int64, int32, double, bool, string

### Crossfilter 

To start working with module, we need create object **crossfilter** first.
```javascript
var addon = require(path_to_module);
var data = new addon.crossfilter();
```

#### Methods
* add(obj, allow_duplicates = true) | add([obj...], allow_duplicates = true)
  Add one or multiple elements to the end of the container.
  If flag allow_duplicates is false, then duplicates elemtns will not be added

* push_back(obj)
  Insert element obj to the end of container

* insert(pos, elem)
  Insert element elem at position pos

* remove() 
  Removes elements that match the current filters

* remove(functor)
  remove elements satisfying the given functor
  
* erase(pos) | erase(begin,end)
  Removes element at position pos, or removes range [begin, end)
  
* at(index)
  Returns element at position index.
 
* back()
  Returns last element
* front()
  Return first element
* size
  Returns size of container
* all
  Returns all elements.
  
* dimension(type, functor, is_iterable = false)
   Constructs a new dimension using the specified value accessor function.
   if is_iterable equals true, then functor should return array of elements.

* feature_count()
     A convenience function for grouping all records and reducing to a single value,
     with predefined reduce functions to count records
     
* feature_sum( type, functor)
  A convenience function for grouping all records and reducing to a single value,
  with predefined reduce functions to sum records.
  type should be one of ctypes constant and  point to return values of the functors.
  
* feature(value_type, add_func, remove_func, init_func)
     A convenience function for grouping all records and reducing to a single value,
     with given reduce functions. value_type should be one of ctypes constant and 
     point to return values of the functors.

* all_filtered([dim1,...])
     Returns all of the raw records in the crossfilter, with filters applied.
     Can optionally ignore filters that are defined on  dimension list [dimensions]
     
* is_element_filtered(index, [dim1,...])
     Check if the data record at index is excluded by the current filter set.
     Can optionally ignore filters that are defined on  dimension list

* on_change(callback)
    set callback function, wich will be called on add, remove and filter events



### Dimension
#### Methods
* top(K, Offset)
    return K elements starting from Offset sorted by descending order
* bottom(K, Offset)
    return K elements starting from Offset sorted by acscending order
* filter() 
    clear all filters
* filter(value)
    filter all elements equal to value
* filter(min, max)
    filter all elements that greater or equal to min and less than max
* filter(functor)
    filter all elements, for wich functor returns true
* feature_count(key_type, key_functor)
    Group elements and count
* feature_sum(key_type, value_type, key_functor, value_functor)
    Group elements and summarize
* feature(key_type, value_type, key_functor, add_functor, remove_functor, init_functor)
    Group elements based on key_functor and apply reducing functors
* feature_all(vaue_type, add_functor, remove_functor, init_functor)
    Group elements to one and apply reducing functors
* feature_all_count()
    Group elements to one and count
* feature_all_sum(value_type, value_functor)
    Group elements to one and apply sum 

### Feature
#### Methods
* all()
  returns the array of all groups 
* top(k, order_functor)
     Returns a new array containing the top k groups,
     according to the order defined by order_functor of the associated reduce value.
* value()
    returns first group.
* size()
    returns number of groups
* order(order_func)
     Specifies the order value for computing the top-K groups.
     The default order is the identity function,
     which assumes that the reduction values are naturally-ordered (such as simple counts or sums).
* order_natural()
     A convenience method for using natural order for reduce values.
