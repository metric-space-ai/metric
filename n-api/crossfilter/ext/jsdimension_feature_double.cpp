/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Dmitry Vinokurov (c) 2018.
*/
#include <node_api.h>
#include "jsdimension_feature.hpp"
#include "utils.hpp"
template napi_value feature_<double,int64_t,int64_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,int64_t,int32_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,int64_t,bool>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,int64_t,double>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,int64_t,std::string>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,int64_t,uint64_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,int32_t,int64_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,int32_t,int32_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,int32_t,bool>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,int32_t,double>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,int32_t,std::string>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,int32_t,uint64_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,bool,int64_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,bool,int32_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,bool,bool>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,bool,double>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,bool,std::string>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,bool,uint64_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,double,int64_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,double,int32_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,double,bool>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,double,double>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,double,std::string>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,double,uint64_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,std::string,int64_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,std::string,int32_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,std::string,bool>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,std::string,double>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,std::string,std::string>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,std::string,uint64_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,uint64_t,int64_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,uint64_t,int32_t>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,uint64_t,bool>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,uint64_t,double>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,uint64_t,std::string>(napi_env, js_function &, jsdimension *, int, int);
template napi_value feature_<double,uint64_t,uint64_t>(napi_env, js_function &, jsdimension *, int, int);
