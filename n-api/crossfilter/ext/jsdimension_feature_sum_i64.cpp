/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#include <node_api.h>
#include "jsdimension_feature_sum.hpp"
#include "utils.hpp"
template napi_value feature_sum_<int64_t,int64_t,int64_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int64_t,js_array<int64_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int64_t,int32_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int64_t,js_array<int32_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int64_t,bool, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int64_t,js_array<bool>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int64_t,double, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int64_t,js_array<double>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int64_t,std::string, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int64_t,js_array<std::string>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int64_t,uint64_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int64_t,js_array<uint64_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int32_t,int64_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int32_t,js_array<int64_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int32_t,int32_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int32_t,js_array<int32_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int32_t,bool, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int32_t,js_array<bool>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int32_t,double, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int32_t,js_array<double>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int32_t,std::string, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int32_t,js_array<std::string>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int32_t,uint64_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,int32_t,js_array<uint64_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,bool,int64_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,bool,js_array<int64_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,bool,int32_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,bool,js_array<int32_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,bool,bool, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,bool,js_array<bool>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,bool,double, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,bool,js_array<double>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,bool,std::string, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,bool,js_array<std::string>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,bool,uint64_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,bool,js_array<uint64_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,double,int64_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,double,js_array<int64_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,double,int32_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,double,js_array<int32_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,double,bool, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,double,js_array<bool>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,double,double, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,double,js_array<double>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,double,std::string, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,double,js_array<std::string>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,double,uint64_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,double,js_array<uint64_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,uint64_t,int64_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,uint64_t,js_array<int64_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,uint64_t,int32_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,uint64_t,js_array<int32_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,uint64_t,bool, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,uint64_t,js_array<bool>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,uint64_t,double, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,uint64_t,js_array<double>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,uint64_t,std::string, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,uint64_t,js_array<std::string>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,uint64_t,uint64_t, cross::non_iterable>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_sum_<int64_t,uint64_t,js_array<uint64_t>, cross::iterable>(napi_env, js_function &, jsdimension *, int, int);
