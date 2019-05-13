#ifndef JSFEATURE_ORDER_H_GUARD
#define JSFEATURE_ORDER_H_GUARD
#include "jsfeature.hpp"
#include "utils.hpp"
#include "feature_cast_extern.hpp"
template<typename Key, typename Value, typename DimType, bool is_group_all, typename I>
napi_value order_(napi_env env, js_function & jsf, jsfeature * obj) {
  auto & feature = cast_feature<Key,Value, DimType,is_group_all, I>(obj->ptr);
  napi_ref order;
  NAPI_CALL(napi_create_reference(env, jsf.args[0], 1, &order));
  napi_ref jsthis;
  NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &jsthis));
  auto lambda = [env, order, jsthis](const Value & v) -> Value {
                  napi_value value= convert_from(env,v);
                  napi_value lambda;
                  NAPI_CALL(napi_get_reference_value(env, order, &lambda));
                  napi_value this_value;
                  NAPI_CALL(napi_get_reference_value(env, jsthis, &this_value));
                  napi_value result;
                  NAPI_CALL(napi_call_function(env, this_value, lambda, 1, &value,
                                               &result));
                  return convert_to<Value>(env, result);
                };
  feature.order(lambda);
  return nullptr;
}


#endif
