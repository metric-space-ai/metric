#include "jsdimension.hpp"

template<typename T, typename I >
static napi_value get_top(napi_env env, jsdimension * dim, int64_t k, int64_t offset) {
  //cross::dimension<T, void*> * d = static_cast<cross::dimension<T,void*>*>(dim->dim);
  auto & d = cast_dimension<T, I>(dim->dim);
  auto data = d.top(k, offset);
  napi_value result;
  NAPI_CALL(napi_create_array_with_length(env, data.size(), &result));
  int i = 0;
  for(auto v : data) {
    napi_value value = extract_value(env, v, dim->filter_type);
    //NAPI_CALL(napi_get_reference_value(env, v, &value));
    napi_status status = napi_set_element(env, result, i, value);
    assert(status == napi_ok);
    i++;
  }
  return result;
}

template<typename T, typename I >
  static napi_value get_bottom(napi_env env, jsdimension * dim, int64_t k, int64_t offset) {
    //cross::dimension<T, void*> * d = static_cast<cross::dimension<T,void*>*>(dim->dim);
    auto & d = cast_dimension<T, I>(dim->dim);
    auto data = d.bottom(k, offset);
    napi_value result;
    NAPI_CALL(napi_create_array_with_length(env, data.size(), &result));
    int i = 0;
    for(auto & v : data) {
      napi_value value = extract_value(env, v, dim->filter_type);
      //NAPI_CALL(napi_get_reference_value(env, v, &value));
      NAPI_CALL(napi_set_element(env, result, i, value));
      i++;
    }
    return result;
  }

napi_value jsdimension::top(napi_env env, napi_callback_info info) {
  auto jsf = extract_function(env, info, 2);
  auto dim = get_object<jsdimension>(env, jsf.jsthis);
  int64_t k = 0;
  int64_t offset = 0;
  NAPI_CALL(napi_get_value_int64(env, jsf.args[0], &k));
  if (jsf.args.size() > 1) {
    NAPI_CALL(napi_get_value_int64(env, jsf.args[1], &offset));
  }
  CALL_FOR_TYPE(get_top,dim->dim_type, dim->is_iterable, env, dim, k, offset);
  return nullptr;
}
napi_value  jsdimension::bottom(napi_env env, napi_callback_info info) {
  auto jsf = extract_function(env, info, 2);
  auto dim = get_object<jsdimension>(env, jsf.jsthis);
  int64_t k = 0;
  int64_t offset = 0;
  NAPI_CALL(napi_get_value_int64(env, jsf.args[0], &k));
  if (jsf.args.size() > 1) {
    NAPI_CALL(napi_get_value_int64(env, jsf.args[1], &offset));
  }
  CALL_FOR_TYPE(get_bottom,dim->dim_type, dim->is_iterable, env, dim, k, offset);
  return nullptr;
}

