/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#define DECLARE_MAKE_INIT(key_type)\
  static auto make_init(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & init_ref, is_##key_type##_tag) { \
  return [obj, this_ref,                                                \
          init_ref]() -> key_type {                                     \
  napi_value this_value;                                                \
  NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value)); \
  napi_value init_;                                                     \
  NAPI_CALL(napi_get_reference_value(obj->env_, init_ref, &init_));     \
  napi_value result;                                                    \
  NAPI_CALL(napi_call_function(obj->env_, this_value, init_, 0, nullptr, \
                               &result));                               \
  return convert_to<key_type>(obj->env_, result);};                            \
}
DECLARE_MAKE_INIT(int64_t);
DECLARE_MAKE_INIT(int32_t);
DECLARE_MAKE_INIT(double);
DECLARE_MAKE_INIT(bool);
//DECLARE_MAKE_INIT(string);
DECLARE_MAKE_INIT(uint64_t);
