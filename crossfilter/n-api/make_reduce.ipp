#define DECLARE_MAKE_REDUCE(key_type)                                   \
  static auto make_reduce(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & add_ref, is_##key_type##_tag) { \
    return [obj, this_ref,                                              \
            add_ref](key_type & r, void* const & rec, bool b ) -> key_type { \
             napi_value args[3];                                        \
             args[0] = convert_from(obj->env_,r);                       \
             args[1] = extract_value(obj->env_, rec, obj->filter_type); \
             args[2] = convert_from(obj->env_, b);                      \
             napi_value this_value;                                     \
             NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value)); \
             napi_value add_;                                           \
             NAPI_CALL(napi_get_reference_value(obj->env_, add_ref, &add_)); \
             napi_value result;                                         \
             NAPI_CALL(napi_call_function(obj->env_, this_value, add_, 3, args,&result)); \
             return convert_to<key_type>(obj->env_, result);};          \
  }

DECLARE_MAKE_REDUCE(int64_t);
DECLARE_MAKE_REDUCE(int32_t);
DECLARE_MAKE_REDUCE(double);
DECLARE_MAKE_REDUCE(bool);
//DECLARE_MAKE_REDUCE(string);
DECLARE_MAKE_REDUCE(uint64_t);
