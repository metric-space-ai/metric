#ifndef JSFEATURE_TOP_H_GUARD
#define JSFEATURE_TOP_H_GUARD
#include "jsfeature.hpp"
#include "utils.hpp"
#include "feature_cast_extern.hpp"

template<typename K, typename V>
inline napi_value convert_group_data(napi_env env, const std::vector<std::pair<K,V>> & data) {
  napi_value result;
  NAPI_CALL(napi_create_array_with_length(env, data.size(), &result));
  int i = 0;
  for(auto & v : data) {
    napi_value value = convert_from<V>(env,v.second);
    napi_value key = convert_from<K>(env,v.first);
    napi_value element;
    NAPI_CALL(napi_create_object(env,&element));
    NAPI_CALL(napi_set_named_property(env,element,"key",key));
    NAPI_CALL(napi_set_named_property(env,element,"value",value));
    NAPI_CALL(napi_set_element(env, result, i, element));
    i++;
  }
  return result;
}





template<typename Key, typename Value, typename DimType, bool is_group_all, typename I>
napi_value top_(napi_env env, js_function & jsf, jsfeature * obj) {
  auto & feature = cast_feature<Key,Value, DimType,is_group_all, I>(obj->ptr);
  napi_ref order;
  int k;
  
  if(jsf.args.size() == 2) {
    napi_valuetype vt1;
    NAPI_CALL(napi_typeof(env,jsf.args[0],&vt1));
    napi_valuetype vt2;
    NAPI_CALL(napi_typeof(env,jsf.args[1],&vt2));
    if(vt1 != napi_number && vt2 != napi_function) {
      napi_throw_error(env, nullptr, "wrong argument types, should be integer and function");
    }
    k = convert_to<int32_t>(env, jsf.args[0]);
    NAPI_CALL(napi_create_reference(env, jsf.args[1], 1, &order));
    napi_ref jsthis;
    NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &jsthis));
    auto data = feature.top(k,[env, order, jsthis](auto v) -> Value {
                                napi_value value= convert_from(env,v);
                                napi_value lambda;
                                NAPI_CALL(napi_get_reference_value(env, order, &lambda));
                                napi_value this_value;
                                NAPI_CALL(napi_get_reference_value(env, jsthis, &this_value));
                                napi_value result;
                                NAPI_CALL(napi_call_function(env, this_value, lambda, 1, &value,
                                                             &result));
                                return convert_to<Value>(env, result);
                              });
    return convert_group_data(env, data);
  } else if(jsf.args.size() == 1 || jsf.args.empty()) {
    if(!jsf.args.empty()) {
      napi_valuetype vt1;
      NAPI_CALL(napi_typeof(env,jsf.args[0],&vt1));
      if(vt1 != napi_number) {
        napi_throw_error(env, nullptr, "wrong argument types, should be integer and function");
      }
      k = convert_to<int32_t>(env, jsf.args[0]);
    } else {
      k = -1;
    }
    auto data = feature.top(k);
    return convert_group_data(env, data);
  }
  return nullptr;
}
#endif
