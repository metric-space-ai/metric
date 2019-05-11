#define DECLARE_MAKE_KEY(key_type, value_type)                          \
  static auto make_key(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & lambda_ref, is_##key_type##_tag, is_##value_type##_tag) { \
  return [obj, this_ref,                                                \
          lambda_ref](value_type v) -> key_type {                       \
           napi_value this_value;                                       \
           NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value)); \
           napi_value lambda_value;                                     \
           NAPI_CALL(napi_get_reference_value(obj->env_, lambda_ref, &lambda_value)); \
           napi_value value = convert_from<value_type>(obj->env_, v);   \
           napi_value result;                                           \
           NAPI_CALL(napi_call_function(obj->env_, this_value, lambda_value, 1, &value, \
                                        &result));                      \
           return convert_to<key_type>(obj->env_, result);};            \
  }

DECLARE_MAKE_KEY(int64_t,int64_t);
DECLARE_MAKE_KEY(int64_t,int32_t);
DECLARE_MAKE_KEY(int64_t,bool);
DECLARE_MAKE_KEY(int64_t,double);
DECLARE_MAKE_KEY(int64_t,string);
DECLARE_MAKE_KEY(int64_t,uint64_t);
///-----------------------------------------------//
DECLARE_MAKE_KEY(int32_t,int64_t);
DECLARE_MAKE_KEY(int32_t,int32_t);
DECLARE_MAKE_KEY(int32_t,bool);
DECLARE_MAKE_KEY(int32_t,double);
DECLARE_MAKE_KEY(int32_t,string);
DECLARE_MAKE_KEY(int32_t,uint64_t);
///-----------------------------------------------//
DECLARE_MAKE_KEY(bool,int64_t);
DECLARE_MAKE_KEY(bool,int32_t);
DECLARE_MAKE_KEY(bool,bool);
DECLARE_MAKE_KEY(bool,double);
DECLARE_MAKE_KEY(bool,string);
DECLARE_MAKE_KEY(bool,uint64_t);
///-----------------------------------------------//
DECLARE_MAKE_KEY(double,int64_t);
DECLARE_MAKE_KEY(double,int32_t);
DECLARE_MAKE_KEY(double,bool);
DECLARE_MAKE_KEY(double,double);
DECLARE_MAKE_KEY(double,string);
DECLARE_MAKE_KEY(double,uint64_t);
///-----------------------------------------------//
DECLARE_MAKE_KEY(string,int64_t);
DECLARE_MAKE_KEY(string,int32_t);
DECLARE_MAKE_KEY(string,bool);
DECLARE_MAKE_KEY(string,double);
DECLARE_MAKE_KEY(string,string);
DECLARE_MAKE_KEY(string,uint64_t);
///-----------------------------------------------//
DECLARE_MAKE_KEY(uint64_t,int64_t);
DECLARE_MAKE_KEY(uint64_t,int32_t);
DECLARE_MAKE_KEY(uint64_t,bool);
DECLARE_MAKE_KEY(uint64_t,double);
DECLARE_MAKE_KEY(uint64_t,string);
DECLARE_MAKE_KEY(uint64_t,uint64_t);
///-----------------------------------------------//

