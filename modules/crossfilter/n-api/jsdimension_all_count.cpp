#include "jsdimension.hpp"
#include "jsfeature.hpp"
template<typename D, typename I >
static napi_value feature_all_count_(napi_env env, js_function& jsf, jsdimension * obj) {
  jsfeature * feature = new jsfeature();
  feature->key_type = is_uint64;
  feature->value_type = is_uint64;
  feature->dim_type = obj->dim_type;
  feature->is_iterable = std::is_same<I,cross::iterable>::value;
  auto & d = cast_dimension<D, I>(obj->dim);
  feature->ptr = new feature_holder<std::size_t,std::size_t,typename std::remove_reference<decltype(d)>::type, true>(d.feature_all_count());
  feature->is_group_all  = true;
  return jsdimension::create_feature(env, feature);
}

napi_value  jsdimension::feature_all_count(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env,info,0);
    jsdimension * obj = get_object<jsdimension>(env, jsf.jsthis);
    switch(obj->dim_type) {
      case is_int64:
        return (obj->is_iterable) ? feature_all_count_<js_array<int64_t>, cross::iterable>(env,jsf, obj) : feature_all_count_<int64_t, cross::non_iterable>(env,jsf, obj);
        break;
      case is_int32:
        return (obj->is_iterable) ? feature_all_count_<js_array<int32_t>, cross::iterable>(env,jsf, obj) : feature_all_count_<int32_t, cross::non_iterable>(env,jsf, obj);
        break;
      case is_bool:
        return (obj->is_iterable) ? feature_all_count_<js_array<bool>, cross::iterable>(env,jsf, obj) : feature_all_count_<bool, cross::non_iterable>(env,jsf, obj);
        break;
      case is_double:
        return (obj->is_iterable) ? feature_all_count_<js_array<double>, cross::iterable>(env,jsf, obj) : feature_all_count_<double, cross::non_iterable>(env,jsf, obj);
        break;
      case is_string:
        return (obj->is_iterable) ? feature_all_count_<js_array<std::string>, cross::iterable>(env,jsf, obj) : feature_all_count_<std::string, cross::non_iterable>(env,jsf, obj);
        break;

    }
    return nullptr;
}
