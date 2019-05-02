#include "jsdimension.hpp"
#include "jsfeature.hpp"
#include "make_lambda.hpp"
#include "make_value.ipp"
#include "make_key.ipp"

// template<typename T, typename V>
// static auto make_key(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & lambda_ref) {

//   return [obj, this_ref,
//           lambda_ref](V v) -> T {
//            napi_value this_value;
//            NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
//            napi_value lambda_value;
//            NAPI_CALL(napi_get_reference_value(obj->env_, lambda_ref, &lambda_value));

//            napi_value value = convert_from<V>(obj->env_, v);
//            napi_value result;
//             napi_call_function(obj->env_, this_value, lambda_value, 1, &value,
//                                                    &result);
//            //assert(status == napi_ok);
//            return convert_to<T>(obj->env_, result);};
// }
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

template<typename K, typename V, typename D >
  static napi_value feature_sum_(napi_env env, js_function& jsf, jsdimension * obj, int key_type, int value_type) {
    napi_ref this_ref;
    NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &this_ref));
    napi_ref key_ref;
    NAPI_CALL(napi_create_reference(env, jsf.args[2], 1, &key_ref));
    napi_ref value_ref;
    NAPI_CALL(napi_create_reference(env, jsf.args[3], 1, &value_ref));
    auto & d = cast_dimension<D, cross::iterable>(obj->dim);

    jsfeature * feature = new jsfeature();
    feature->key_type = key_type;
    feature->value_type = value_type;
    feature->dim_type = obj->dim_type;
    feature->is_iterable = true;
    feature->is_group_all = false;
    using value_type_t = typename std::remove_reference<decltype(d)>::type::value_type_t;
    using key_tag_type = typename define_tag<K>::type;
    using reduce_tag_type = typename define_tag<V>::type;
    using value_tag_type = typename define_tag<value_type_t>::type;
    feature->ptr = new feature_holder<K,V,typename std::remove_reference<decltype(d)>::type, false>(std::move(d.feature_sum(make_value(env, obj, this_ref, value_ref,reduce_tag_type()),
                                                                                                                            make_key(env, obj, this_ref, key_ref, key_tag_type(), value_tag_type()))));

    // feature->ptr = new feature_holder<K,V,typename std::remove_reference<decltype(d)>::type, false>(std::move(d.feature_sum(make_value<V>(env, obj, this_ref, value_ref),
    //                                                                                                                         make_key<K,value_type_t>(env, obj, this_ref, key_ref))));
    return jsdimension::create_feature(env,feature);
  }

#define CALL_DISPATCH_DIM_4(ktype, vtype, dtype, fn, ...)  \
      return fn<ktype,vtype,js_array<dtype>>(__VA_ARGS__);  \

#define CALL_DISPATCH_DIM_3(ktype, vtype, dtype, fn, ...) \
  switch(dtype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_DIM_4(ktype, vtype, int64_t, fn, __VA_ARGS__); \
    case is_int32:                                                      \
      CALL_DISPATCH_DIM_4(ktype, vtype, int32_t,fn, __VA_ARGS__); \
    case is_bool:                                                       \
      CALL_DISPATCH_DIM_4(ktype, vtype, bool,fn, __VA_ARGS__); \
    case is_double:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, double,fn, __VA_ARGS__); \
    case is_string:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, std::string,fn, __VA_ARGS__); \
    case is_uint64:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, uint64_t, fn, __VA_ARGS__); \
  }

#define CALL_DISPATCH_DIM_2(ktype, vtype, dtype, fn, ...)  \
      switch(vtype) {                                   \
        case is_int64:                                  \
          CALL_DISPATCH_DIM_3(ktype, int64_t, dtype, fn, __VA_ARGS__); \
          break;                                                   \
        case is_int32:                                  \
          CALL_DISPATCH_DIM_3(ktype, int32_t, dtype,fn, __VA_ARGS__); \
          break;                                                   \
        case is_bool:                                   \
          CALL_DISPATCH_DIM_3(ktype, bool, dtype, fn, __VA_ARGS__); \
          break;                                               \
        case is_double:                                 \
          CALL_DISPATCH_DIM_3(ktype, double, dtype, fn, __VA_ARGS__); \
          break;                                                 \
        case is_uint64:                                                 \
          CALL_DISPATCH_DIM_3(ktype, uint64_t, dtype, fn, __VA_ARGS__); \
        break;                                                          \
      }

#define CALL_DISPATCH_DIM(ktype, vtype, dtype, fn, ...)    \
  switch(ktype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_DIM_2(int64_t, vtype, dtype,fn, __VA_ARGS__); \
      break;                                                            \
    case is_int32:                                                      \
      CALL_DISPATCH_DIM_2(int32_t, vtype, dtype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_bool:                                                       \
      CALL_DISPATCH_DIM_2(bool, vtype, dtype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_double:                                                     \
      CALL_DISPATCH_DIM_2(double, vtype, dtype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_string:                                                     \
      CALL_DISPATCH_DIM_2(std::string, vtype, dtype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_uint64:                                                     \
      CALL_DISPATCH_DIM_2(uint64_t, vtype, dtype, fn, __VA_ARGS__); \
      break;                                                            \
  }
napi_value  jsdimension::feature_sum_iterable(napi_env env, js_function & jsf, jsdimension *obj, int key_type, int value_type) {
    CALL_DISPATCH_DIM(key_type, value_type, obj->dim_type,feature_sum_,env,jsf,obj,key_type, value_type);
    return nullptr;
  }



