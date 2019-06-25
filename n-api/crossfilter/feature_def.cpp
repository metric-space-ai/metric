/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#include "crossfilter.hpp"
#include <node_api.h>
#define DEFINE_CAST_FEATURE(key_type, value_type, dim_type, is_group_all)             \
  template struct cross::feature<key_type,value_type,cross::dimension<dim_type,void*,cross::non_iterable>,is_group_all>; \
  template struct cross::impl::feature_impl<key_type,value_type,cross::dimension<dim_type,void*,cross::non_iterable>,is_group_all>; 

#include "feature_cast.ipp"
