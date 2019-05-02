#ifndef JSDIMENSION_FEATURE_H_GUARD
#define JSDIMENSION_FEATURE_H_GUARD
#include <node_api.h>

#include "jsdimension.hpp"
#include "jsfeature.hpp"
// #include "make_init.ipp"
// #include "make_key.ipp"
// #include "make_lambda.hpp"
// #include "make_reduce.ipp"
#include "utils.hpp"
template <typename key_type, typename value_type>
inline auto make_key(napi_env env, jsdimension *obj, napi_ref &this_ref,
                     napi_ref &lambda_ref) {
  return [obj, this_ref, lambda_ref](value_type v) -> key_type {
    napi_value this_value;
    NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
    napi_value lambda_value;
    NAPI_CALL(napi_get_reference_value(obj->env_, lambda_ref, &lambda_value));
    napi_value value = convert_from<value_type>(obj->env_, v);
    napi_value result;
    NAPI_CALL(napi_call_function(obj->env_, this_value, lambda_value, 1, &value,
                                 &result));
    return convert_to<key_type>(obj->env_, result);
  };
}

template <typename key_type>
inline auto make_reduce(napi_env env, jsdimension *obj, napi_ref &this_ref,
                        napi_ref &add_ref) {
  return [obj, this_ref, add_ref](key_type &r, void *const &rec,
                                  bool b) -> key_type {
    napi_value args[3];
    args[0] = convert_from(obj->env_, r);
    args[1] = extract_value(obj->env_, rec, obj->filter_type);
    args[2] = convert_from(obj->env_, b);
    napi_value this_value;
    NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
    napi_value add_;
    NAPI_CALL(napi_get_reference_value(obj->env_, add_ref, &add_));
    napi_value result;
    NAPI_CALL(
        napi_call_function(obj->env_, this_value, add_, 3, args, &result));
    return convert_to<key_type>(obj->env_, result);
  };
}
template <typename key_type>
inline auto make_init(napi_env env, jsdimension *obj, napi_ref &this_ref,
                      napi_ref &init_ref) {
  return [obj, this_ref, init_ref]() -> key_type {
    napi_value this_value;
    NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
    napi_value init_;
    NAPI_CALL(napi_get_reference_value(obj->env_, init_ref, &init_));
    napi_value result;
    NAPI_CALL(
        napi_call_function(obj->env_, this_value, init_, 0, nullptr, &result));
    return convert_to<key_type>(obj->env_, result);
  };
}

template <typename K, typename R, typename D>
napi_value feature_(napi_env env, js_function &jsf, jsdimension *obj,
                    int key_type, int value_type) {
  jsfeature *feature = new jsfeature();
  feature->key_type = key_type;
  feature->value_type = value_type;
  feature->dim_type = obj->dim_type;
  feature->is_iterable = false;

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
  auto &d = cast_dimension<D, cross::non_iterable>(obj->dim);
  using value_type_t =
      typename std::remove_reference<decltype(d)>::type::value_type_t;
  // using key_tag_type = typename define_tag<K>::type;
  // using reduce_tag_type = typename define_tag<R>::type;
  // using value_tag_type = typename define_tag<value_type_t>::type;
  feature->ptr = new feature_holder<
      K, R, typename std::remove_reference<decltype(d)>::type, false>(d.feature(
          make_reduce<R>(env, obj, this_ref, add_ref),
          make_reduce<R>(env, obj, this_ref, remove_ref),
          make_init<R>(env, obj, this_ref, init_ref),
          make_key<K,value_type_t>(env, obj, this_ref, key_ref)));
  return jsdimension::create_feature(env, feature);
}

#endif
