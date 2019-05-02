#include "crossfilter.hpp"
#include <node_api.h>
#define DEFINE_CAST_FEATURE(key_type, value_type, dim_type, is_group_all)             \
  template struct cross::feature<key_type,value_type,cross::dimension<dim_type,void*,cross::non_iterable>,is_group_all>; \
  template struct cross::impl::feature_impl<key_type,value_type,cross::dimension<dim_type,void*,cross::non_iterable>,is_group_all>; 

#include "feature_cast.ipp"
