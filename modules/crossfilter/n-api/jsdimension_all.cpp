#include "jsdimension.hpp"
#include "jsfeature.hpp"
#include "utils.hpp"

#include <cstddef>
template <typename T>
static auto make_add(napi_env env, jsdimension *obj, napi_ref &this_ref,
                     napi_ref &add_ref) {

  return [obj, this_ref, add_ref](T &r, void *const &rec, bool b) -> T {
    napi_value args[3];
    args[0] = convert_from(obj->env_, r);
    // NAPI_CALL(napi_get_reference_value(obj->env_, rec, &args[1]));
    args[1] = extract_value(obj->env_, rec, obj->filter_type);
    args[2] = convert_from(obj->env_, b);
    napi_value this_value;
    NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
    napi_value add_;
    NAPI_CALL(napi_get_reference_value(obj->env_, add_ref, &add_));

    napi_value result;
    NAPI_CALL(
        napi_call_function(obj->env_, this_value, add_, 3, args, &result));
    return convert_to<T>(obj->env_, result);
  };
}
template <typename T>
static auto make_remove(napi_env env, jsdimension *obj, napi_ref &this_ref,
                        napi_ref &remove_ref) {
  return [obj, this_ref, remove_ref](T &r, void *const &rec, bool b) -> T {
    napi_value args[3];
    args[0] = convert_from(obj->env_, r);
    // NAPI_CALL(napi_get_reference_value(obj->env_, rec, &args[1]));
    args[1] = extract_value(obj->env_, rec, obj->filter_type);
    args[2] = convert_from(obj->env_, b);
    napi_value this_value;
    NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
    napi_value remove_;
    NAPI_CALL(napi_get_reference_value(obj->env_, remove_ref, &remove_));

    napi_value result;
    NAPI_CALL(
        napi_call_function(obj->env_, this_value, remove_, 3, args, &result));
    return convert_to<T>(obj->env_, result);
  };
}
template <typename T>
static auto make_init(napi_env env, jsdimension *obj, napi_ref &this_ref,
                      napi_ref &init_ref) {
  return [obj, this_ref, init_ref]() -> T {
    napi_value this_value;
    NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
    napi_value init_;
    NAPI_CALL(napi_get_reference_value(obj->env_, init_ref, &init_));

    napi_value result;
    NAPI_CALL(
        napi_call_function(obj->env_, this_value, init_, 0, nullptr, &result));
    return convert_to<T>(obj->env_, result);
  };
}

template <typename R, typename D, typename I>
static napi_value feature_all_(napi_env env, js_function &jsf, jsdimension *obj,
                               int value_type, bool is_iterable) {
  jsfeature *feature = new jsfeature();
  feature->key_type = is_uint64;
  feature->value_type = value_type;
  feature->dim_type = obj->dim_type;
  feature->is_iterable = is_iterable;

  napi_ref this_ref;
  NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &this_ref));
  napi_ref remove_ref;
  napi_ref add_ref;
  napi_ref init_ref;
  NAPI_CALL(napi_create_reference(env, jsf.args[1], 1, &add_ref));
  NAPI_CALL(napi_create_reference(env, jsf.args[2], 1, &remove_ref));
  NAPI_CALL(napi_create_reference(env, jsf.args[3], 1, &init_ref));
  auto &d = cast_dimension<D, I>(obj->dim);
  feature->ptr = new feature_holder<
      std::size_t, R, typename std::remove_reference<decltype(d)>::type, true>(
      d.feature_all(make_add<R>(env, obj, this_ref, add_ref),
                    make_remove<R>(env, obj, this_ref, remove_ref),
                    make_init<R>(env, obj, this_ref, init_ref)));
  feature->is_group_all = true;
  return jsdimension::create_feature(env, feature);
}
#define CALL_DISPATCH_DIM_ALL3(vtype, dtype, itype, fn, ...)                   \
  if (itype) {                                                                 \
    return fn<vtype, js_array<dtype>, cross::iterable>(__VA_ARGS__);           \
  } else {                                                                     \
    return fn<vtype, dtype, cross::non_iterable>(__VA_ARGS__);                 \
  }

#define CALL_DISPATCH_DIM_ALL2(vtype, dtype, itype, fn, ...)                   \
  switch (dtype) {                                                             \
  case is_int64:                                                               \
    CALL_DISPATCH_DIM_ALL3(vtype, int64_t, itype, fn, __VA_ARGS__);            \
    break;                                                                     \
  case is_int32:                                                               \
    CALL_DISPATCH_DIM_ALL3(vtype, int32_t, itype, fn, __VA_ARGS__);            \
    break;                                                                     \
  case is_bool:                                                                \
    CALL_DISPATCH_DIM_ALL3(vtype, bool, itype, fn, __VA_ARGS__);               \
    break;                                                                     \
  case is_double:                                                              \
    CALL_DISPATCH_DIM_ALL3(vtype, double, itype, fn, __VA_ARGS__);             \
    break;                                                                     \
  case is_uint64:                                                              \
    CALL_DISPATCH_DIM_ALL3(vtype, uint64_t, itype, fn, __VA_ARGS__);           \
    break;                                                                     \
  case is_string:                                                              \
    CALL_DISPATCH_DIM_ALL3(vtype, std::string, itype, fn, __VA_ARGS__);        \
    break;                                                                     \
  }

#define CALL_DISPATCH_DIM_ALL(vtype, dtype, itype, fn, ...)                    \
  switch (vtype) {                                                             \
  case is_int64:                                                               \
    CALL_DISPATCH_DIM_ALL2(int64_t, dtype, itype, fn, __VA_ARGS__);            \
    break;                                                                     \
  case is_int32:                                                               \
    CALL_DISPATCH_DIM_ALL2(int32_t, dtype, itype, fn, __VA_ARGS__);            \
    break;                                                                     \
  case is_double:                                                              \
    CALL_DISPATCH_DIM_ALL2(double, dtype, itype, fn, __VA_ARGS__);             \
    break;                                                                     \
  case is_uint64:                                                              \
    CALL_DISPATCH_DIM_ALL2(uint64_t, dtype, itype, fn, __VA_ARGS__);           \
    break;                                                                     \
  }

napi_value jsdimension::feature_all(napi_env env, napi_callback_info info) {
  auto jsf = extract_function(env, info, 4);
  jsdimension *obj = get_object<jsdimension>(env, jsf.jsthis);
  int32_t value_type = convert_to<int32_t>(env, jsf.args[0]);
  CALL_DISPATCH_DIM_ALL(value_type, obj->dim_type, obj->is_iterable,
                        feature_all_, env, jsf, obj, value_type,
                        obj->is_iterable);
  return nullptr;
}
