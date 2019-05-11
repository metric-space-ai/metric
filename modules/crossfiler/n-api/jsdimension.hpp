#ifndef JS_DIMENSION_H_GUARD
#define JS_DIMENSION_H_GUARD
#include <type_traits>

#include "crossfilter.hpp"
#include "utils.hpp"
//#include "jsfeature.hpp"

#include <node_api.h>
template <typename T, typename I> cross::dimension<T, void*, I, std::function<uint64_t(void*)>> & cast_dimension(void * ptr);

#define CAST_DIMENSION_DEFINE(dim_type, iterables) \
  template<> \
  inline cross::dimension<dim_type, void*, iterables, std::function<uint64_t(void*)>>& cast_dimension(void * ptr) { \
    return static_cast<dimension_holder<dim_type, iterables>*>(ptr)->dimension; \
  }


CAST_DIMENSION_DEFINE(js_array<int64_t>, cross::iterable);
CAST_DIMENSION_DEFINE(int64_t, cross::non_iterable);
CAST_DIMENSION_DEFINE(js_array<int32_t>, cross::iterable);
CAST_DIMENSION_DEFINE(int32_t, cross::non_iterable);
CAST_DIMENSION_DEFINE(js_array<uint64_t>, cross::iterable);
CAST_DIMENSION_DEFINE(uint64_t, cross::non_iterable);
CAST_DIMENSION_DEFINE(js_array<bool>, cross::iterable);
CAST_DIMENSION_DEFINE(bool, cross::non_iterable);
CAST_DIMENSION_DEFINE(js_array<double>, cross::iterable);
CAST_DIMENSION_DEFINE(double, cross::non_iterable);
CAST_DIMENSION_DEFINE(js_array<std::string>, cross::iterable);
CAST_DIMENSION_DEFINE(std::string, cross::non_iterable);

// template<>
// inline cross::dimension<int32_t, void*>* cast_dimension(void * ptr) {
//   return static_cast<cross::dimension<int32_t, napi_ref>*>(ptr);
// }
// template<>
// inline cross::dimension<bool, void*>* cast_dimension(void * ptr) {
//   return static_cast<cross::dimension<bool, void*>*>(ptr);
// }
// template<>
// inline cross::dimension<double, void*>* cast_dimension(void * ptr) {
//   return static_cast<cross::dimension<double, void*>*>(ptr);
// }
// template<>
// inline cross::dimension<std::string, void*>* cast_dimension(void * ptr) {
//   return static_cast<cross::dimension<std::string, napi_ref>*>(ptr);
//}


#define CALL_FOR_TYPE(name, type, is_iterable,  ...)                       \
  switch((type)) {                                                      \
    case is_int64:                                                      \
      if(! is_iterable) return name<int64_t, cross::non_iterable>(__VA_ARGS__); \
      else return name<js_array<int64_t>, cross::iterable>(__VA_ARGS__);          \
    case is_int32:                                                      \
      if(! is_iterable) return name<int32_t, cross::non_iterable>(__VA_ARGS__); \
      else return name<js_array<int32_t>, cross::iterable>(__VA_ARGS__); \
    case is_bool:                                                       \
      if(! is_iterable) return name<bool, cross::non_iterable>(__VA_ARGS__); \
      else return name<js_array<bool>, cross::iterable>(__VA_ARGS__);   \
    case is_double:                                                     \
      if(! is_iterable) return name<double, cross::non_iterable>(__VA_ARGS__); \
      else return name<js_array<double>, cross::iterable>(__VA_ARGS__); \
    case is_string:                                                     \
      if(! is_iterable) return name<std::string, cross::non_iterable>(__VA_ARGS__); \
      else return name<js_array<std::string>, cross::iterable>(__VA_ARGS__); \
  };

// template<typename T, typename V>
// struct napi_func_wrapper {
//   napi_ref jthis;
//   napi_ref lambda;
//   napi_env env;
//   napi_func_wrapper(napi_env env_, napi_value jthis_, napi_value lambda_) {
//     NAPI_CALL(napi_create_reference(env, jsthis_, 1, &jthis));
//     NAPI_CALL(napi_create_reference(env, lambda_, 1, &lambda));
//   }
//   ~napi_func_wrapper() {
//     NAPI_CALL(napi_delete_reference(env,lambda));
//     NAPI_CALL(napi_delete_reference(env,jthis));
//   }

//   T operator() (V && v) {
//     napi_value value = convert_from(env, v);
//     napi_value this_value;
//     NAPI_CALL(napi_get_reference_value(env, jthis, &this_value));
//     napi_value lambda_value;
//     NAPI_CALL(napi_get_reference_value(env, lambda, &lambda_value));
//     napi_value result;
//     NAPI_CALL(napi_call_function(env, this_value, lambda_value, 1, &value,
//                                  &result));
//     return convert_to<T>(obj->env_, result);
//   }
// };

struct jsfeature;
struct jsdimension {
  void* dim;
  int dim_type;
  int filter_type;
  bool is_iterable;
  napi_ref wrapper;
  napi_env env_;
  int dim_offset;
  int dim_index;
  static napi_ref constructor;

  static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);


  static napi_value feature_all_sum(napi_env env, napi_callback_info info);
  static napi_value feature_all_count(napi_env env, napi_callback_info info);
  static napi_value feature(napi_env env, napi_callback_info info);
  static napi_value feature_iterable(napi_env env, js_function & jsf, jsdimension * obj, int key_type, int value_type);
  static napi_value feature_all(napi_env env, napi_callback_info info);
  static napi_value feature_count(napi_env env, napi_callback_info info);
  static napi_value feature_sum(napi_env env, napi_callback_info info);
  static napi_value feature_sum_iterable(napi_env env, js_function & jsf, jsdimension * obj, int key_type, int value_type);
  //  static napi_value create_feature(napi_env env, jsfeature * feature);
  static void filter_function(js_function & jsf, jsdimension * dim);
  static void filter_exact(js_function & jsf, jsdimension * dim);
  static void filter_range(js_function & jsf, jsdimension * dim);
  static void filter_all(js_function & jsf, jsdimension * dim);
  static napi_value filter(napi_env env, napi_callback_info info);
  static napi_value top(napi_env env, napi_callback_info info);
  static napi_value bottom(napi_env env, napi_callback_info info);
  static napi_value create_feature(napi_env env, jsfeature* info);

};


#endif
