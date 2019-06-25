/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef FEATURE_EXPORT_H_GUARD
#define FEATURE_EXPORT_H_GUARD
#include "crossfilter.hpp"
#undef DEFINE_CAST_FEATURE
#define DEFINE_CAST_FEATURE(key_type, value_type, dim_type, is_group_all)             \
  extern template class cross::feature<key_type,value_type,cross::dimension<dim_type,void*,cross::non_iterable>,is_group_all>; \
  extern template class cross::impl::feature_impl<key_type,value_type,cross::dimension<dim_type,void*,cross::non_iterable>,is_group_all>;

//extern template class cross::feature<key_type,value_type,cross::dimension<dim_type,napi_ref,cross::iterable>,is_group_all>;
  //  extern template class cross::impl::feature_impl<key_type,value_type,cross::dimension<dim_type,napi_ref,cross::iterable>,is_group_all>; 
#include "feature_cast.ipp"
#undef DEFINE_CAST_FEATURE
#endif
