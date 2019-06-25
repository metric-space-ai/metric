/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#include <node_api.h>
#include "jsdimension_feature.hpp"
#include "utils.hpp"
extern template napi_value feature_<bool,int64_t,int64_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,int64_t,int32_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,int64_t,bool>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,int64_t,double>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,int64_t,std::string>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,int64_t,uint64_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,int32_t,int64_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,int32_t,int32_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,int32_t,bool>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,int32_t,double>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,int32_t,std::string>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,int32_t,uint64_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,bool,int64_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,bool,int32_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,bool,bool>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,bool,double>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,bool,std::string>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,bool,uint64_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,double,int64_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,double,int32_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,double,bool>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,double,double>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,double,std::string>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,double,uint64_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,std::string,int64_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,std::string,int32_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,std::string,bool>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,std::string,double>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,std::string,std::string>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,std::string,uint64_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,uint64_t,int64_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,uint64_t,int32_t>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,uint64_t,bool>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,uint64_t,double>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,uint64_t,std::string>(napi_env, js_function &, jsdimension *, int, int);
extern template napi_value feature_<bool,uint64_t,uint64_t>(napi_env, js_function &, jsdimension *, int, int);
