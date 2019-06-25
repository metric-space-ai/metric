/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#include "jsfeature.hpp"
#include "ext/jsfeature_top_bool_ext.hpp"
#include "ext/jsfeature_top_double_ext.hpp"
#include "ext/jsfeature_top_string_ext.hpp"
#include "ext/jsfeature_top_i64_ext.hpp"
#include "ext/jsfeature_top_i32_ext.hpp"
#include "ext/jsfeature_top_ui64_ext.hpp"

napi_value jsfeature::top(napi_env env, napi_callback_info info) {
  js_function jsf = extract_function(env, info, 2);
  jsfeature* obj = get_object<jsfeature>(env, jsf.jsthis);
  if(obj->dim_type != is_cross) {
    CALL_DISPATCH_F(obj->key_type, obj->value_type, obj->dim_type, obj->is_group_all, obj->is_iterable, top_, env, jsf, obj);
  } else {
    return value(env,info);
  }
  return nullptr;
}
