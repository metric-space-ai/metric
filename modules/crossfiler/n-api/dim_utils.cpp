template<typename T, typename V>
static auto make_key(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & lambda_ref) {
  napi_value this_value;
  NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
  napi_value lambda_value;
  NAPI_CALL(napi_get_reference_value(obj->env_, lambda_ref, &lambda_value));

  return [obj, this_value,
          lambda_value](V v) -> T {
           napi_value value = convert_from<V>(obj->env_, v);
           napi_value result;
            napi_call_function(obj->env_, this_value, lambda_value, 1, &value,
                                                   &result);
           //assert(status == napi_ok);
           return convert_to<T>(obj->env_, result);};
}

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

template<typename T>
static auto make_add(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & add_ref) {
  napi_value this_value;
  NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
  napi_value add_;
  NAPI_CALL(napi_get_reference_value(obj->env_, add_ref, &add_));

  return [obj, this_value,
               add_](T & r, void* const & rec, bool b ) -> T {
                napi_value args[3];
                args[0] = convert_from(obj->env_,r);
                //NAPI_CALL(napi_get_reference_value(obj->env_, rec, &args[1]));
                args[1] = extract_value(obj->env_, rec, obj->filter_type);
                args[2] = convert_from(obj->env_, b);

                napi_value result;
                NAPI_CALL(napi_call_function(obj->env_, this_value, add_, 3, args,
                                             &result));
                return convert_to<T>(obj->env_, result);};
}
template<typename T>
static auto make_remove(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & remove_ref) {
  napi_value this_value;
  NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
  napi_value remove_;
  NAPI_CALL(napi_get_reference_value(obj->env_, remove_ref, &remove_));

  return [obj, this_value,
          remove_](T & r,  void* const & rec, bool b ) -> T {
           napi_value args[3];
           args[0] = convert_from(obj->env_,r);
           //NAPI_CALL(napi_get_reference_value(obj->env_, rec, &args[1]));
           args[1] = extract_value(obj->env_, rec, obj->filter_type);
           args[2] = convert_from(obj->env_, b);
           
           napi_value result;
           NAPI_CALL(napi_call_function(obj->env_, this_value, remove_, 3, args,
                                        &result));
           return convert_to<T>(obj->env_, result);};
}
template<typename T>
static auto make_init(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & init_ref) {
  napi_value this_value;
  NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
  napi_value init_;
  NAPI_CALL(napi_get_reference_value(obj->env_, init_ref, &init_));

  return [obj, this_value,
          init_]() -> T {
           napi_value result;
           NAPI_CALL(napi_call_function(obj->env_, this_value, init_, 0, nullptr,
                                        &result));
           return convert_to<T>(obj->env_, result);};

}

