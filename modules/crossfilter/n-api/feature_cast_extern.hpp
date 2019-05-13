#ifndef FEATURE_CAST_EXTERN_H_GUARD
#define FEATURE_CAST_EXTERN_H_GUARD
#include "feature_cast.hpp"

#define DEFINE_CAST_FEATURE(key_type, value_type, dim_type, is_group_all, is_iterable) \
  extern template cross::feature<key_type,value_type,cross::dimension< dim_type, void*, is_iterable>, \
                                 is_group_all> &cast_feature(void *ptr);

  //  extern template  cross::feature<key_type,value_type,cross::dimension<dim_type,void*, is_iterable>,is_group_all> & cast_feature(void *ptr);

#include "feature_cast.ipp"

extern template cross::feature<uint64_t,int64_t,cross::filter<void*>,true> & cast_feature_filter<int64_t>(void *ptr);
extern template cross::feature<uint64_t,int32_t,cross::filter<void*>,true> & cast_feature_filter<int32_t>(void *ptr);
extern template cross::feature<uint64_t,uint64_t,cross::filter<void*>,true> & cast_feature_filter<uint64_t>(void *ptr);
extern template cross::feature<uint64_t,bool,cross::filter<void*>,true> & cast_feature_filter<bool>(void *ptr);
extern template cross::feature<uint64_t,double,cross::filter<void*>,true> & cast_feature_filter<double>(void *ptr);
extern template cross::feature<uint64_t,std::string,cross::filter<void*>,true> & cast_feature_filter<std::string>(void *ptr);

#endif
