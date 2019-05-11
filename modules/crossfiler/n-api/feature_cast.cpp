#include "feature_cast.hpp"
#include "crossfilter.hpp"
#include <node_api.h>

// #define DEFINE_CAST_FEATURE(key_type, value_type, dim_type, is_group_all, is_iterable) 
//   template <> cross::feature<key_type,value_type,cross::dimension<      
//                                                                         typename std::conditional<std::is_same<is_iterable,cross::iterable>::value, js_array<dim_type>,dim_type>::type, 
//                                                                         void*, is_iterable>, 
//                              is_group_all> &                            
//   cast_feature(void *ptr) {  return static_cast<feature_holder<key_type,value_type,cross::dimension<typename std::conditional<std::is_same<is_iterable,cross::iterable>::value, js_array<dim_type>, dim_type>::type,void*, is_iterable>,is_group_all>*>(ptr)->feature;}

#define DEFINE_CAST_FEATURE(key_type, value_type, dim_type, is_group_all, is_iterable) \
  template <> cross::feature<key_type,value_type,cross::dimension<dim_type, void*, is_iterable>, \
                             is_group_all> & \
  cast_feature(void *ptr) {  return static_cast<feature_holder<key_type,value_type,cross::dimension<dim_type,void*, is_iterable>,is_group_all>*>(ptr)->feature;}


#include "feature_cast.ipp"

template <> cross::feature<uint64_t,int64_t,cross::filter<void*>,true> & cast_feature_filter<int64_t>(void *ptr) {  return static_cast<feature_holder<uint64_t,int64_t,cross::filter<void*>,true>*>(ptr)->feature;}
template <> cross::feature<uint64_t,int32_t,cross::filter<void*>,true> & cast_feature_filter<int32_t>(void *ptr) {  return static_cast<feature_holder<uint64_t,int32_t,cross::filter<void*>,true>*>(ptr)->feature;}
template <> cross::feature<uint64_t,uint64_t,cross::filter<void*>,true> & cast_feature_filter<uint64_t>(void *ptr) {  return static_cast<feature_holder<uint64_t,uint64_t,cross::filter<void*>,true>*>(ptr)->feature;}
template <> cross::feature<uint64_t,bool,cross::filter<void*>,true> & cast_feature_filter<bool>(void *ptr) {  return static_cast<feature_holder<uint64_t,bool,cross::filter<void*>,true>*>(ptr)->feature;}
template <> cross::feature<uint64_t,double,cross::filter<void*>,true> & cast_feature_filter<double>(void *ptr) {  return static_cast<feature_holder<uint64_t,double,cross::filter<void*>,true>*>(ptr)->feature;}
template <> cross::feature<uint64_t,std::string,cross::filter<void*>,true> & cast_feature_filter<std::string>(void *ptr) {  return static_cast<feature_holder<uint64_t,std::string,cross::filter<void*>,true>*>(ptr)->feature;}
