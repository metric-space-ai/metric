#include "jsdimension.hpp"
#include "jsfeature.hpp"
template<typename T>
static auto make_value(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & value_ref) {

  return [obj, this_ref,
          value_ref](void * v) -> T {
           napi_value this_value;
           NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
           napi_value value_value;
           NAPI_CALL(napi_get_reference_value(obj->env_, value_ref, &value_value));

           napi_value value = extract_value(obj->env_, v, obj->filter_type);
           napi_value result;
           NAPI_CALL(napi_call_function(obj->env_, this_value, value_value, 1, &value,
                                        &result));
           return convert_to<T>(obj->env_, result);};
}

// template<typename K, typename R, typename D, bool is_group_all, typename I >
//   static std::enable_if_t<!(is_group_all && std::is_same<K,uint64_t>::value), napi_value> feature_all_sum_(napi_env env, js_function& jsf, jsdimension * obj, int key_type, int value_type) {
//     return nullptr;
//   }
template<typename R,typename D, typename I >
//  static std::enable_if_t<(is_group_all && std::is_same<K,uint64_t>::value), napi_value>
napi_value feature_all_sum_(napi_env env, js_function& jsf, jsdimension * obj,  int value_type, bool is_iterable) {
    napi_ref this_ref;
    NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &this_ref));
    napi_ref value_ref;
    NAPI_CALL(napi_create_reference(env, jsf.args[1], 1, &value_ref));
    auto & d = cast_dimension<D, I>(obj->dim);

    jsfeature * feature = new jsfeature();
    feature->key_type = is_uint64;
    feature->value_type = value_type;
    feature->dim_type = obj->dim_type;
    feature->is_iterable = is_iterable;
    feature->is_group_all = true;
    feature->ptr = new feature_holder<std::size_t,R,typename std::remove_reference<decltype(d)>::type, true>(std::move(d.feature_all_sum(make_value<R>(env, obj, this_ref, value_ref))));
    return jsdimension::create_feature(env,feature);
  }

#define CALL_DISPATCH_DIM_ALL_SUM3(vtype, dtype, itype, fn, ...)        \
  if(itype) {                                                           \
    return fn<vtype,js_array<dtype>,cross::iterable>(__VA_ARGS__);             \
  } else { \
    return fn<vtype,dtype,cross::non_iterable>(__VA_ARGS__); \
  }

#define CALL_DISPATCH_DIM_ALL_SUM2(vtype, dtype, itype, fn, ...)         \
  switch(dtype) {                                                       \
  case is_int64:                                                        \
  CALL_DISPATCH_DIM_ALL_SUM3(vtype, int64_t,itype, fn, __VA_ARGS__); \
  break;                                                                \
  case is_int32:                                                        \
  CALL_DISPATCH_DIM_ALL_SUM3(vtype, int32_t, itype, fn, __VA_ARGS__); \
  break;                                                                \
  case is_bool:                                                        \
  CALL_DISPATCH_DIM_ALL_SUM3(vtype,bool, itype, fn, __VA_ARGS__); \
  break;                                                                \
  case is_double:                                                       \
  CALL_DISPATCH_DIM_ALL_SUM3(vtype, double, itype, fn, __VA_ARGS__); \
  break;                                                                \
  case is_uint64:                                                       \
  CALL_DISPATCH_DIM_ALL_SUM3(vtype, uint64_t,itype, fn, __VA_ARGS__);   \
  break;                                                                \
  case is_string:                                                       \
  CALL_DISPATCH_DIM_ALL_SUM3(vtype, std::string,itype, fn, __VA_ARGS__);      \
  break;                                                                \
  }

#define CALL_DISPATCH_DIM_ALL_SUM(vtype, dtype, itype, fn, ...)         \
  switch(vtype) {                                                       \
  case is_int64:                                                        \
  CALL_DISPATCH_DIM_ALL_SUM2(int64_t, dtype, itype, fn, __VA_ARGS__); \
  break;                                                                \
  case is_int32:                                                        \
  CALL_DISPATCH_DIM_ALL_SUM2(int32_t, dtype, itype, fn, __VA_ARGS__); \
  break;                                                                \
  case is_double:                                                       \
  CALL_DISPATCH_DIM_ALL_SUM2(double, dtype, itype, fn, __VA_ARGS__); \
  break;                                                                \
  case is_uint64:                                                       \
  CALL_DISPATCH_DIM_ALL_SUM2(uint64_t, dtype, itype, fn, __VA_ARGS__);  \
  break;                                                                \
  }

napi_value  jsdimension::feature_all_sum(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env,info,2);
    jsdimension * obj = get_object<jsdimension>(env, jsf.jsthis);
    int32_t value_type = convert_to<int32_t>(env, jsf.args[0]);
    CALL_DISPATCH_DIM_ALL_SUM(value_type, obj->dim_type,obj->is_iterable, feature_all_sum_,env,jsf,obj, value_type, obj->is_iterable);
    std::cout << 1 << std::endl;
    return nullptr;
}

