#ifndef JS_FEATURE_H_GUARD
#define JS_FEATURE_H_GUARD
#include <node_api.h>


struct jsfeature {
  void* ptr;
  int key_type;
  int value_type;
  int dim_type;
  bool is_group_all = false;
  bool is_iterable = false;
  napi_ref wrapper;
  napi_env env_;
  static napi_ref constructor;

  static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);


  static napi_value all(napi_env env, napi_callback_info info);
  static napi_value top(napi_env env, napi_callback_info info);
  static napi_value value(napi_env env, napi_callback_info info);

  static napi_value size(napi_env env, napi_callback_info info);
  static napi_value order(napi_env env, napi_callback_info info);
  static napi_value order_natural(napi_env env, napi_callback_info info);

};
#define CALL_DISPATCH_F_4(ktype, vtype, dtype, btype, itype, fn, ...)  \
  if(btype) {                                                           \
    if(itype) {                                                         \
      return fn<ktype,vtype,js_array<dtype>,true, cross::iterable>(__VA_ARGS__);  \
    } else {                                                            \
      return fn<ktype,vtype,dtype,true, cross::non_iterable>(__VA_ARGS__); \
    }                                                                   \
  } else {                                                              \
    if(itype) {                                                         \
      return fn<ktype,vtype,js_array<dtype>,false, cross::iterable>(__VA_ARGS__); \
    } else {                                                            \
      return fn<ktype,vtype,dtype,false, cross::non_iterable>(__VA_ARGS__); \
    }                                                                   \
  }

#define CALL_DISPATCH_F_3(ktype, vtype, dtype, btype, itype, fn, ...) \
  switch(dtype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_F_4(ktype, vtype, int64_t, btype, itype,fn, __VA_ARGS__); \
      break;                                                            \
    case is_int32:                                                      \
      CALL_DISPATCH_F_4(ktype, vtype, int32_t, btype, itype,fn, __VA_ARGS__); \
      break;                                                            \
    case is_bool:                                                       \
      CALL_DISPATCH_F_4(ktype, vtype, bool, btype, itype,fn, __VA_ARGS__); \
      break;                                                            \
    case is_double:                                                     \
      CALL_DISPATCH_F_4(ktype, vtype, double, btype, itype,fn, __VA_ARGS__); \
      break;                                                            \
    case is_string:                                                     \
      CALL_DISPATCH_F_4(ktype, vtype, std::string, btype, itype,fn, __VA_ARGS__); \
      break;                                                            \
    case is_uint64:                                                     \
      CALL_DISPATCH_F_4(ktype, vtype, uint64_t, btype, itype,fn, __VA_ARGS__); \
      break;                                                            \
  }

#define CALL_DISPATCH_F_2(ktype, vtype, dtype, btype, itype,fn, ...)  \
      switch(vtype) {                                   \
        case is_int64:                                  \
          CALL_DISPATCH_F_3(ktype, int64_t, dtype, btype,itype, fn, __VA_ARGS__); \
          break;                                                   \
        case is_int32:                                  \
          CALL_DISPATCH_F_3(ktype, int32_t, dtype, btype, itype,fn, __VA_ARGS__); \
          break;                                                   \
        case is_bool:                                   \
          CALL_DISPATCH_F_3(ktype, bool, dtype, btype, itype, fn, __VA_ARGS__); \
          break;                                               \
        case is_double:                                 \
          CALL_DISPATCH_F_3(ktype, double, dtype, btype, itype, fn, __VA_ARGS__); \
          break;                                                 \
        case is_uint64:                                                 \
          CALL_DISPATCH_F_3(ktype, uint64_t, dtype, btype, itype, fn, __VA_ARGS__); \
        break;                                                          \
      }

#define CALL_DISPATCH_F(ktype, vtype, dtype, btype, itype, fn, ...)    \
  switch(ktype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_F_2(int64_t, vtype, dtype, btype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_int32:                                                      \
      CALL_DISPATCH_F_2(int32_t, vtype, dtype, btype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_bool:                                                       \
      CALL_DISPATCH_F_2(bool, vtype, dtype, btype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_double:                                                     \
      CALL_DISPATCH_F_2(double, vtype, dtype, btype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_string:                                                     \
      CALL_DISPATCH_F_2(std::string, vtype, dtype, btype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_uint64:                                                     \
      CALL_DISPATCH_F_2(uint64_t, vtype, dtype, btype, itype, fn, __VA_ARGS__); \
      break;                                                            \
  }

#endif
