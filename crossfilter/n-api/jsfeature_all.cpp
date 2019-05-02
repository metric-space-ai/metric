#include "jsfeature.hpp"
//#include "feature_export.hpp"
#include "crossfilter.hpp"
#include "utils.hpp"
#include "feature_cast_extern.hpp"
template<typename K, typename V>
static napi_value convert_group_element(napi_env env, const std::pair<K,V> & v) {
  napi_value value = convert_from<V>(env,v.second);
  napi_value key = convert_from<K>(env,v.first);
  napi_value element;
  NAPI_CALL(napi_create_object(env,&element));
  NAPI_CALL(napi_set_named_property(env,element,"key",key));
  NAPI_CALL(napi_set_named_property(env,element,"value",value));
  return element;
}

template<typename K, typename V>
static napi_value convert_group_data(napi_env env, const std::vector<std::pair<K,V>> & data) {
  napi_value result;
  NAPI_CALL(napi_create_array_with_length(env, data.size(), &result));
  int i = 0;
  for(auto & v : data) {

    NAPI_CALL(napi_set_element(env, result, i, convert_group_element(env, v)));
    i++;
  }
  return result;
}



template<typename Key, typename Value, typename DimType, bool is_group_all, typename I>
static  napi_value all_(napi_env env, napi_callback_info info) {
  napi_value jsthis;
  NAPI_CALL(napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr));

  jsfeature* obj = get_object<jsfeature>(env, jsthis);
  if(obj->dim_type == is_cross) {
    auto & feature = cast_feature_filter<Value>(obj->ptr);
    auto data = feature.all();
    return convert_group_data(env, data);
  }
  auto & feature = cast_feature<Key,Value, DimType, is_group_all, I>(obj->ptr);
  auto data = feature.all();
  return convert_group_data(env, data);
}
template<typename Value>
static  napi_value all_(napi_env env,  jsfeature * obj) {
  auto & feature = cast_feature_filter<Value>(obj->ptr);
  auto data = feature.all();
  return convert_group_data(env, data);
}

napi_value jsfeature::all(napi_env env, napi_callback_info info) {
  js_function jsf = extract_function(env, info, 0);
  jsfeature* obj = get_object<jsfeature>(env, jsf.jsthis);
  if(obj->dim_type != is_cross) {
    CALL_DISPATCH_F(obj->key_type, obj->value_type, obj->dim_type, obj->is_group_all, obj->is_iterable, all_, env, info );
  } else {
    switch(obj->value_type) {
      case is_int64:
        return all_<int64_t>(env, obj);
      case is_int32:
        return all_<int32_t>(env,obj);
      case is_bool:
        return all_<bool>(env, obj);
      case is_double:
        return all_<double>(env, obj);
      case is_uint64:
        return all_<uint64_t>(env, obj);
    }
  }
  return nullptr;
}
