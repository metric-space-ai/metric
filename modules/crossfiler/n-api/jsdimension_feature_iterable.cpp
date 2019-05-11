#include "jsdimension.hpp"
#include "jsfeature.hpp"
#include "make_lambda.hpp"
#include "make_key.ipp"
#include "make_reduce.ipp"
#include "make_init.ipp"

template<typename K, typename R, typename D>
static napi_value feature_(napi_env env, js_function& jsf, jsdimension * obj, int key_type, int value_type) {
    jsfeature * feature = new jsfeature();
    feature->key_type = key_type;
    feature->value_type = value_type;
    feature->dim_type = obj->dim_type;
    feature->is_iterable = true;

    napi_ref this_ref;
    NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &this_ref));
    napi_ref key_ref;
    napi_ref remove_ref;
    napi_ref add_ref;
    napi_ref init_ref;
    NAPI_CALL(napi_create_reference(env, jsf.args[2], 1, &key_ref));
    NAPI_CALL(napi_create_reference(env, jsf.args[3], 1, &add_ref));
    NAPI_CALL(napi_create_reference(env, jsf.args[4], 1, &remove_ref));
    NAPI_CALL(napi_create_reference(env, jsf.args[5], 1, &init_ref));
    auto & d = cast_dimension<js_array<D>, cross::iterable>(obj->dim);
    using value_type_t = typename std::remove_reference<decltype(d)>::type::value_type_t;
    using key_tag_type = typename define_tag<K>::type;
    using reduce_tag_type = typename define_tag<R>::type;
    using value_tag_type = typename define_tag<value_type_t>::type;
    feature->ptr = new feature_holder<K,R,typename std::remove_reference<decltype(d)>::type, false>(d.feature(make_reduce(env, obj, this_ref, add_ref, reduce_tag_type()),
                                                                                                              make_reduce(env, obj, this_ref, remove_ref, reduce_tag_type()),
                                                                                                              make_init(env, obj, this_ref, init_ref, reduce_tag_type()),
                                                                                                              make_key(env, obj, this_ref, key_ref, key_tag_type(), value_tag_type())));
    return jsdimension::create_feature(env, feature);
  }


#define CALL_DISPATCH_DIM_4(ktype, vtype, dtype, fn, ...)  \
  return fn<ktype,vtype,dtype>(__VA_ARGS__);

#define CALL_DISPATCH_DIM_3(ktype, vtype, dtype, fn, ...) \
  switch(dtype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_DIM_4(ktype, vtype, int64_t,fn, __VA_ARGS__); \
    case is_int32:                                                      \
      CALL_DISPATCH_DIM_4(ktype, vtype, int32_t,fn, __VA_ARGS__); \
    case is_bool:                                                       \
      CALL_DISPATCH_DIM_4(ktype, vtype, bool, fn, __VA_ARGS__); \
    case is_double:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, double, fn, __VA_ARGS__); \
    case is_string:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, std::string,fn, __VA_ARGS__); \
    case is_uint64:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, uint64_t,fn, __VA_ARGS__); \
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
      CALL_DISPATCH_DIM_2(int64_t, vtype, dtype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_int32:                                                      \
      CALL_DISPATCH_DIM_2(int32_t, vtype, dtype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_bool:                                                       \
      CALL_DISPATCH_DIM_2(bool, vtype, dtype,  fn, __VA_ARGS__); \
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

napi_value  jsdimension::feature_iterable(napi_env env, js_function & jsf, jsdimension * obj, int key_type, int value_type) {
  CALL_DISPATCH_DIM(key_type, value_type, obj->dim_type, feature_,env,jsf,obj,key_type, value_type);
  return nullptr;
}
