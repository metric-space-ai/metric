#ifndef BINDING_FILTER_H_GUARD
#define BINDING_FILTER_H_GUARD
#include "crossfilter.hpp"
//#include "jsdimension.hpp"
#include "utils.hpp"

#include <node_api.h>

// struct js_hash {
//   napi_env env;
//   napi_value jsthis;
//   napi_value lambda;
//   int object_type;
//   js_hash(napi_env env_, napi_value & t, napi_value & l, int type):env(env_), lambda(l), jsthis(t), object_type(t);
//   uint64_t operator() (void * v) {
//     auto value = extract_value(env, v, object_type);
//     // napi_value lambda_value;
//     // NAPI_CALL(napi_get_reference_value(obj->env_, lambda_ref, &lambda_value));
//     napi_value result;
//     NAPI_CALL(napi_call_function(env, jsthis, lambda, 1,
//                                  &value, &result));
//     return convert_to<uint64_t>(env, result);
//   }
// };

struct crossfilter {
  napi_env env_;
  napi_ref wrapper;
  int obj_type;
  napi_value jsthis;
  napi_value hash_lambda;
  std::vector<napi_ref> listeners;

  std::function<uint64_t(void*)> hash = [this](void * v) -> uint64_t {
                                           if(hash_lambda == nullptr)
                                             return 0;
                                           auto value = extract_value(this->env_, v, this->obj_type);
                                           napi_value result;
                                           napi_value jsthis;
                                           napi_get_reference_value(this->env_,this->wrapper, &jsthis);
                                           NAPI_CALL(napi_call_function(this->env_, jsthis, this->hash_lambda, 1,
                                                                        &value, &result));
                                           return convert_to<uint64_t>(this->env_, result);
                                         };
  cross::filter<void*, decltype(hash)> filter;

  void on_change_event(cross::event event);

  static napi_ref constructor;
  
  crossfilter(napi_env env, napi_value jsthis_, napi_value hash_lambda_):env_(env), jsthis(jsthis_), hash_lambda(hash_lambda_), filter(hash) {
    filter.onChange([this](cross::event event) {on_change_event(event);});
  }

  static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);

  static void print_obj(napi_env env, napi_value &object);

  static napi_value check_function(napi_env env, napi_callback_info info);

  static napi_value dimension(napi_env env, napi_callback_info info);

  static napi_value remove(napi_env env, napi_callback_info info);
  static napi_value all(napi_env env, napi_callback_info info);
  static napi_value all_filtered(napi_env env, napi_callback_info info);
  static napi_value is_element_filtered(napi_env env, napi_callback_info info);
  static napi_value size(napi_env env, napi_callback_info info);
  static napi_value add(napi_env env, napi_callback_info info);
  static napi_value feature(napi_env env, napi_callback_info info);
  static napi_value feature_count(napi_env env, napi_callback_info info);
  static napi_value feature_sum(napi_env env, napi_callback_info info);

  static napi_value erase(napi_env env, napi_callback_info info);
  static napi_value insert(napi_env env, napi_callback_info info);
  static napi_value at(napi_env env, napi_callback_info info);
  static napi_value back(napi_env env, napi_callback_info info);
  static napi_value front(napi_env env, napi_callback_info info);
  static napi_value on_change(napi_env env, napi_callback_info info);
};

#endif
