#include "jsdimension.hpp"
#include "ext/jsdimension_feature_sum_bool_ext.hpp"
#include "ext/jsdimension_feature_sum_double_ext.hpp"
#include "ext/jsdimension_feature_sum_string_ext.hpp"
#include "ext/jsdimension_feature_sum_i64_ext.hpp"
#include "ext/jsdimension_feature_sum_i32_ext.hpp"
#include "ext/jsdimension_feature_sum_ui64_ext.hpp"

#define CALL_DISPATCH_DIM_4(ktype, vtype, dtype,  itype, fn, ...) \
  if(itype) {                                                     \
    return fn<ktype,vtype,js_array<dtype>,cross::iterable>(__VA_ARGS__); \
  }  else {                                                             \
    return fn<ktype,vtype,dtype,cross::non_iterable>(__VA_ARGS__);      \
  }

#define CALL_DISPATCH_DIM_3(ktype, vtype, dtype,  itype, fn, ...)       \
  switch(dtype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_DIM_4(ktype, vtype, int64_t,  itype, fn, __VA_ARGS__); \
    case is_int32:                                                      \
      CALL_DISPATCH_DIM_4(ktype, vtype, int32_t, itype, fn, __VA_ARGS__); \
    case is_bool:                                                       \
      CALL_DISPATCH_DIM_4(ktype, vtype, bool, itype, fn, __VA_ARGS__);  \
    case is_double:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, double, itype, fn, __VA_ARGS__); \
    case is_string:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, std::string, itype, fn, __VA_ARGS__); \
    case is_uint64:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, uint64_t,  itype, fn, __VA_ARGS__); \
  }

#define CALL_DISPATCH_DIM_2(ktype, vtype, dtype,  itype, fn, ...) \
      switch(vtype) {                                   \
        case is_int64:                                  \
          CALL_DISPATCH_DIM_3(ktype, int64_t, dtype,  itype, fn, __VA_ARGS__); \
          break;                                                   \
        case is_int32:                                  \
          CALL_DISPATCH_DIM_3(ktype, int32_t, dtype, itype, fn, __VA_ARGS__); \
          break;                                                   \
        case is_bool:                                   \
          CALL_DISPATCH_DIM_3(ktype, bool, dtype,  itype, fn, __VA_ARGS__); \
          break;                                               \
        case is_double:                                 \
          CALL_DISPATCH_DIM_3(ktype, double, dtype,  itype, fn, __VA_ARGS__); \
          break;                                                 \
        case is_uint64:                                                 \
          CALL_DISPATCH_DIM_3(ktype, uint64_t, dtype,  itype, fn, __VA_ARGS__); \
        break;                                                          \
      }

#define CALL_DISPATCH_DIM(ktype, vtype, dtype, itype, fn, ...)           \
  switch(ktype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_DIM_2(int64_t, vtype, dtype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_int32:                                                      \
      CALL_DISPATCH_DIM_2(int32_t, vtype, dtype, itype,  fn, __VA_ARGS__); \
      break;                                                            \
    case is_bool:                                                       \
      CALL_DISPATCH_DIM_2(bool, vtype, dtype,  itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_double:                                                     \
      CALL_DISPATCH_DIM_2(double, vtype, dtype, itype,  fn, __VA_ARGS__); \
      break;                                                            \
    case is_string:                                                     \
      CALL_DISPATCH_DIM_2(std::string, vtype, dtype, itype,  fn, __VA_ARGS__); \
      break;                                                            \
    case is_uint64:                                                     \
      CALL_DISPATCH_DIM_2(uint64_t, vtype, dtype,  itype, fn, __VA_ARGS__); \
      break;                                                            \
  }

 napi_value  jsdimension::feature_sum(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env,info,4);
    jsdimension * obj = get_object<jsdimension>(env, jsf.jsthis);
    int32_t key_type = convert_to<int32_t>(env, jsf.args[0]);
    int32_t value_type = convert_to<int32_t>(env, jsf.args[1]);
    // if(obj->is_iterable) {
    //   return jsdimension::feature_sum_iterable(env, jsf, obj, key_type, value_type);
    // }
    CALL_DISPATCH_DIM(key_type, value_type, obj->dim_type, obj->is_iterable, feature_sum_, env,jsf,obj,key_type, value_type);
    return nullptr;
  }



