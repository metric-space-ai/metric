#include "jsdimension.hpp"
#include "jsfeature.hpp"


#define CALL_DISPATCH_DIM_4(ktype, dtype, itype, fn, ...)  \
  if(itype) {                                                          \
    return fn<ktype,js_array<dtype>, cross::iterable>(__VA_ARGS__);     \
  } else {                                                              \
    return fn<ktype,dtype, cross::non_iterable>(__VA_ARGS__);           \
  }                                                                     
  

#define CALL_DISPATCH_DIM_2(ktype, dtype, itype, fn, ...) \
  switch(dtype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_DIM_4(ktype, int64_t, itype,fn, __VA_ARGS__); \
    case is_int32:                                                      \
      CALL_DISPATCH_DIM_4(ktype, int32_t, itype,fn, __VA_ARGS__); \
    case is_bool:                                                       \
      CALL_DISPATCH_DIM_4(ktype, bool, itype,fn, __VA_ARGS__); \
    case is_double:                                                     \
      CALL_DISPATCH_DIM_4(ktype, double, itype,fn, __VA_ARGS__); \
    case is_string:                                                     \
      CALL_DISPATCH_DIM_4(ktype, std::string, itype,fn, __VA_ARGS__); \
    case is_uint64:                                                     \
      CALL_DISPATCH_DIM_4(ktype, uint64_t, itype,fn, __VA_ARGS__); \
  }


#define CALL_DISPATCH_DIM(ktype,dtype, itype, fn, ...)    \
  switch(ktype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_DIM_2(int64_t, dtype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_int32:                                                      \
      CALL_DISPATCH_DIM_2(int32_t, dtype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_bool:                                                       \
      CALL_DISPATCH_DIM_2(bool, dtype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_double:                                                     \
      CALL_DISPATCH_DIM_2(double, dtype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_string:                                                     \
      CALL_DISPATCH_DIM_2(std::string, dtype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_uint64:                                                     \
      CALL_DISPATCH_DIM_2(uint64_t, dtype, itype, fn, __VA_ARGS__); \
      break;                                                            \
  }

template<typename T, typename V>
static auto make_key(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & lambda_ref) {

  return [obj, this_ref,
          lambda_ref](V v) -> T {
           napi_value this_value;
           NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
           napi_value lambda_value;
           NAPI_CALL(napi_get_reference_value(obj->env_, lambda_ref, &lambda_value));

           napi_value value = convert_from<V>(obj->env_, v);
           napi_value result;
           napi_call_function(obj->env_, this_value, lambda_value, 1, &value,
                              &result);
           //assert(status == napi_ok);
           return convert_to<T>(obj->env_, result);};
}

// template<typename T>
// static auto make_value(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & value_ref) {

//   return [obj, this_ref,
//           value_ref](void * v) -> T {
//            napi_value this_value;
//            NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
//            napi_value value_value;
//            NAPI_CALL(napi_get_reference_value(obj->env_, value_ref, &value_value));

//            napi_value value = extract_value(obj->env_, v, obj->filter_type);
//            napi_value result;
//            NAPI_CALL(napi_call_function(obj->env_, this_value, value_value, 1, &value,
//                                         &result));
//            return convert_to<T>(obj->env_, result);};
// }

template<typename K, typename D, typename I>
static napi_value feature_count_(napi_env env, js_function& jsf, jsdimension * obj, int key_type, bool is_iterable) {
  napi_ref this_ref;
  NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &this_ref));
  napi_ref lambda_ref;
  NAPI_CALL(napi_create_reference(env, jsf.args[1], 1, &lambda_ref));

  auto & d = cast_dimension<D, I>(obj->dim);
  jsfeature * feature = new jsfeature();
  feature->key_type = key_type;
  feature->value_type = is_uint64;
  feature->dim_type = obj->dim_type;
  feature->is_iterable = is_iterable;
  using value_type = typename std::remove_reference<decltype(d)>::type::value_type_t;
  feature->ptr = new feature_holder<K,std::size_t, typename std::remove_reference<decltype(d)>::type, false>(d.feature_count(make_key<K,value_type>(env,obj,this_ref,lambda_ref)));
  return jsdimension::create_feature(env, feature);
}


napi_value  jsdimension::feature_count(napi_env env, napi_callback_info info) {
  auto jsf = extract_function(env,info,2);
  jsdimension * obj = get_object<jsdimension>(env, jsf.jsthis);
  int32_t key_type = convert_to<int32_t>(env, jsf.args[0]);
  CALL_DISPATCH_DIM(key_type, obj->dim_type, obj->is_iterable, feature_count_, env, jsf, obj, key_type, obj->is_iterable);
  return nullptr;
}

