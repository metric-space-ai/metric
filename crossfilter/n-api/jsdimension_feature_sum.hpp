#ifndef JSDIMENSION_FEATURE_SUM_H_GUARD
#define JSDIMENSION_FEATURE_SUM_H_GUARD
#include "jsdimension.hpp"
#include "jsfeature.hpp"
// #include "make_lambda.hpp"
// #include "make_value.ipp"
// #include "make_key.ipp"
template <typename key_type>
static auto make_value(napi_env env, jsdimension *obj, napi_ref &this_ref,
                       napi_ref &value_ref) {
  return [obj, this_ref, value_ref](void *v) -> key_type {
    napi_value this_value;
    NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
    napi_value value_value;
    NAPI_CALL(napi_get_reference_value(obj->env_, value_ref, &value_value));
    napi_value value = extract_value(obj->env_, v, obj->filter_type);
    napi_value result;
    NAPI_CALL(napi_call_function(obj->env_, this_value, value_value, 1, &value,
                                 &result));
    return convert_to<key_type>(obj->env_, result);
  };
}

template <typename key_type, typename value_type>
static auto make_key(napi_env env, jsdimension *obj, napi_ref &this_ref,
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

template <typename K, typename V, typename D, typename I>
napi_value feature_sum_(napi_env env, js_function &jsf, jsdimension *obj,
                        int key_type, int value_type) {
  napi_ref this_ref;
  NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &this_ref));
  napi_ref key_ref;
  NAPI_CALL(napi_create_reference(env, jsf.args[2], 1, &key_ref));
  napi_ref value_ref;
  NAPI_CALL(napi_create_reference(env, jsf.args[3], 1, &value_ref));
  auto &d = cast_dimension<D, I>(obj->dim);

  jsfeature *feature = new jsfeature();
  feature->key_type = key_type;
  feature->value_type = value_type;
  feature->dim_type = obj->dim_type;
  feature->is_iterable = obj->is_iterable;
  using value_type_t =
      typename std::remove_reference<decltype(d)>::type::value_type_t;
  // using key_tag_type = typename define_tag<K>::type;
  // using reduce_tag_type = typename define_tag<V>::type;
  // using value_tag_type = typename define_tag<value_type_t>::type;
  feature->ptr = new feature_holder<
      K, V, typename std::remove_reference<decltype(d)>::type, false>(std::move(
      d.feature_sum(make_value<V>(env, obj, this_ref, value_ref),
                    make_key<K, value_type_t>(env, obj, this_ref, key_ref))));
  return jsdimension::create_feature(env, feature);
}

#endif
