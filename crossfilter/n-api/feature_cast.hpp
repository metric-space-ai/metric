#ifndef FEATURE_CAST_H_GUARD
#define FEATURE_CAST_H_GUARD
#include "crossfilter.hpp"
#include "utils.hpp"
#include <type_traits>

template <typename K, typename V, typename D, bool is_group_all, typename I>
cross::feature<K,V,cross::dimension<D, void*, I>,is_group_all> &
cast_feature(void *ptr) {
  return static_cast<feature_holder<K,V,cross::dimension<D,void*, I>,is_group_all>*>(ptr)->feature;
}

// cross::feature<K, V, cross::dimension<D, void*>, is_group_all>&  cast_feature(void * ptr) {
//   return static_cast<feature_holder<K, V, cross::dimension<D, void*>, is_group_all>*>(ptr)->feature;
// }

template <typename V>
cross::feature<uint64_t, V, cross::filter<void*>, true>&  cast_feature_filter(void * ptr) {
  return static_cast<feature_holder<uint64_t, V, cross::filter<void*>, true>*>(ptr)->feature;
}

// #define DEFINE_CAST_FEATURE(key_type, value_type, dim_type)               
//   template <> inline cross::feature<key_type,value_type,cross::dimension<dim_type,napi_ref>,false>* cast_feature(void *ptr) { return static_cast<cross::feature<key_type,value_type,cross::dimension<dim_type,napi_ref>,false>*>(ptr) ;}

//#define DEFINE_CAST_FEATURE(key_type, value_type, dim_type, is_group_all)             
//     template <> inline cross::feature<key_type,value_type,cross::dimension<dim_type,napi_ref>,is_group_all> & cast_feature(void *ptr) { return static_cast<feature_holder<key_type,value_type,cross::dimension<dim_type,napi_ref>,is_group_all>*>(ptr)->feature ;}

// #define DEFINE_CAST_FEATURE(key_type, value_type, dim_type, is_group_all)             
//   extern template  cross::feature<key_type,value_type,cross::dimension<dim_type,napi_ref>,is_group_all> & cast_feature(void *ptr);

//#include "feature_cast.ipp"
#endif
