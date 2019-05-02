#include "jsdimension.hpp"
#include "ext/jsdimension_feature_bool_ext.hpp"
#include "ext/jsdimension_feature_double_ext.hpp"
#include "ext/jsdimension_feature_string_ext.hpp"
#include "ext/jsdimension_feature_i64_ext.hpp"
#include "ext/jsdimension_feature_i32_ext.hpp"
#include "ext/jsdimension_feature_ui64_ext.hpp"



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

napi_value  jsdimension::feature(napi_env env, napi_callback_info info) {
  auto jsf = extract_function(env,info,6);
  jsdimension * obj = get_object<jsdimension>(env, jsf.jsthis);
  int32_t key_type = convert_to<int32_t>(env, jsf.args[0]);
  int32_t value_type = convert_to<int32_t>(env, jsf.args[1]);
  if(obj->is_iterable)
    return feature_iterable(env, jsf, obj, key_type, value_type);

  CALL_DISPATCH_DIM(key_type, value_type, obj->dim_type, feature_,env,jsf,obj,key_type, value_type);
  return nullptr;
}
