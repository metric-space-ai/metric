#define DECLARE_MAKE_VALUE(key_type)                                    \
  static auto make_value(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & value_ref, is_##key_type##_tag) { \
    return [obj, this_ref,                                              \
            value_ref](void * v) -> key_type {                          \
             napi_value this_value;                                     \
             NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value)); \
             napi_value value_value;                                    \
             NAPI_CALL(napi_get_reference_value(obj->env_, value_ref, &value_value)); \
             napi_value value = extract_value(obj->env_, v, obj->filter_type); \
             napi_value result;                                         \
             NAPI_CALL(napi_call_function(obj->env_, this_value, value_value, 1, &value, \
                                          &result));                    \
             return convert_to<key_type>(obj->env_, result);};          \
  }
DECLARE_MAKE_VALUE(int64_t);
DECLARE_MAKE_VALUE(int32_t);
DECLARE_MAKE_VALUE(double);
DECLARE_MAKE_VALUE(bool);
//DECLARE_MAKE_VALUE(string);
DECLARE_MAKE_VALUE(uint64_t);
