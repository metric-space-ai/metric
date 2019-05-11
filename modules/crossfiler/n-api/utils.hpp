#ifndef UTILS_N_API_H_GUARD
#define UTILS_N_API_H_GUARD
#include <functional>
#include <node_api.h>
#include "crossfilter.hpp"
#include "convert.hpp"

#define NAPI_CALL(x)  assert((x) == napi_ok)
#define NAPI_METHOD(name) static napi_value #name(napi_env env, napi_callback_info info)

#define DECLARE_NAPI_METHOD(name, func)         \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

struct js_function {
  napi_value jsthis;
  std::vector<napi_value> args;
};

inline js_function extract_function(napi_env env, napi_callback_info info, std::size_t argc) {
  js_function jsf;
  jsf.args.resize(argc);
  std::size_t real_argc = argc;
  NAPI_CALL(napi_get_cb_info(env, info, &real_argc, jsf.args.data(), &jsf.jsthis, nullptr));
  jsf.args.resize(real_argc);
  return jsf;
}

template<typename T>
inline T* get_object(napi_env env, napi_value jsthis) {
  T* obj;
  NAPI_CALL(napi_unwrap(env, jsthis, reinterpret_cast<void**>(&obj)));
  return obj;
}

enum {
  is_int64,
  is_double,
  is_int32,
  is_string,
  is_bool,
  is_uint64,
  is_cross,
  is_object
};

napi_value extract_value(napi_env env, const void * v, int object_type);

// extern template
// int64_t convert_to(napi_env env, napi_value v);

// extern template
// uint64_t convert_to(napi_env env, napi_value v);

// extern template
// bool convert_to(napi_env env, napi_value v);

// extern template
// int32_t convert_to(napi_env env, napi_value v);

// extern template
// double convert_to(napi_env env, napi_value v);

// extern template
// std::string convert_to(napi_env env, napi_value v);


// extern template
// napi_value convert_from(napi_env env, const int64_t & v);

// extern template
// napi_value convert_from(napi_env env, const uint64_t & v);

// extern template
// napi_value convert_from(napi_env env, const int32_t & v);

// extern template
// napi_value convert_from(napi_env env, const bool & v);

// extern template
// napi_value convert_from(napi_env env, const double & v);

// extern template
// napi_value convert_from(napi_env env, const std::string & v);

template <typename F>
inline void add_function(napi_env env, napi_value obj, F func,
                         const std::string &name) {
  napi_value fn;
  napi_create_function(env, nullptr, 0, func, nullptr, &fn);
  napi_set_named_property(env, obj, name.c_str(), fn);
}

template<typename K, typename V, typename D, bool isGroupAll>
struct feature_holder {
  cross::feature<K,V,D,isGroupAll> feature;
  explicit feature_holder(cross::feature<K,V,D,isGroupAll> && f):feature(std::move(f)) {}
};

template<typename K,  typename I>
struct dimension_holder {
  cross::dimension<K, void*, I, std::function<uint64_t(void*)>> dimension;
  explicit dimension_holder(cross::dimension<K, void*, I, std::function<uint64_t(void*)>> && d) : dimension(std::move(d)) {}
};

template<typename H>
struct filter_holder {
  cross::filter<void*,H> filter;
};
#define CALL_DISPATCH3_4(ktype, vtype, dtype, btype, fn, ...)          \
  if(btype) {                                                          \
    return fn<ktype,vtype,dtype,true>(__VA_ARGS__);                    \
  } else {                                                             \
    return fn<ktype,vtype,dtype,false>(__VA_ARGS__);                   \
  }

#define CALL_DISPATCH3_3(ktype, vtype, dtype, btype, fn, ...) \
  switch(dtype) {                                             \
    case is_int64:                                                      \
      CALL_DISPATCH3_4(ktype, vtype, int64_t, btype, fn, __VA_ARGS__);  \
    case is_int32:                                                      \
      CALL_DISPATCH3_4(ktype, vtype, int32_t, btype, fn, __VA_ARGS__);  \
    case is_bool:                                                       \
      CALL_DISPATCH3_4(ktype, vtype, bool, btype, fn, __VA_ARGS__);     \
    case is_double:                                                     \
      CALL_DISPATCH3_4(ktype, vtype, double, btype, fn, __VA_ARGS__);   \
    case is_string:                                                     \
      CALL_DISPATCH3_4(ktype, vtype, std::string, btype, fn, __VA_ARGS__); \
    case is_uint64:                                                     \
      CALL_DISPATCH3_4(ktype, vtype, uint64_t, btype, fn, __VA_ARGS__); \
  }

#define CALL_DISPATCH3_2(ktype, vtype, dtype, btype, fn, ...)     \
      switch(vtype) {                                   \
        case is_int64:                                  \
          CALL_DISPATCH3_3(ktype, int64_t, dtype, btype, fn, __VA_ARGS__);  \
          break;                                                   \
        case is_int32:                                  \
          CALL_DISPATCH3_3(ktype, int32_t, dtype, btype, fn, __VA_ARGS__);  \
          break;                                                   \
        case is_bool:                                   \
          CALL_DISPATCH3_3(ktype, bool, dtype, btype, fn, __VA_ARGS__); \
          break;                                               \
        case is_double:                                 \
          CALL_DISPATCH3_3(ktype, double, dtype, btype, fn, __VA_ARGS__); \
          break;                                                 \
        case is_uint64:                                                 \
        CALL_DISPATCH3_3(ktype, uint64_t, dtype, btype, fn, __VA_ARGS__); \
        break;                                                          \
      }

#define CALL_DISPATCH3(ktype, vtype, dtype, btype, fn,...)  \
  switch(ktype) {                                           \
    case is_int64:                                                      \
      CALL_DISPATCH3_2(int64_t, vtype, dtype, btype,fn, __VA_ARGS__);   \
      break;                                                            \
    case is_int32:                                                      \
      CALL_DISPATCH3_2(int32_t, vtype, dtype, btype,fn, __VA_ARGS__);   \
      break;                                                            \
    case is_bool:                                                       \
      CALL_DISPATCH3_2(bool, vtype, dtype, btype, fn, __VA_ARGS__);     \
      break;                                                            \
    case is_double:                                                     \
      CALL_DISPATCH3_2(double, vtype, dtype, btype, fn, __VA_ARGS__);   \
      break;                                                            \
    case is_string:                                                     \
      CALL_DISPATCH3_2(std::string, vtype, dtype, btype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_uint64:                                                     \
      CALL_DISPATCH3_2(uint64_t, vtype, dtype, btype, fn, __VA_ARGS__); \
      break;                                                            \
  }

#define CALL_DISPATCH2_2(type1, type2, fn, ...)          \
  switch(type2) {                                             \
        case is_int64:                                        \
          return fn<type1,int64_t>(__VA_ARGS__);      \
        case is_int32:                                        \
    	    return fn<type1,int32_t>(__VA_ARGS__);      \
        case is_bool:                                         \
          return fn<type1,bool>(__VA_ARGS__);         \
        case is_double:                                       \
          return fn<type1, double>(__VA_ARGS__);       \
        case is_string:                                       \
          return fn<type1,std::string>(__VA_ARGS__);  \
      }

#define CALL_DISPATCH2(type1, type2, fn,...)  \
      switch(type1) { \
        case is_int64: \
          CALL_DISPATCH2_2(int64_t, type2, fn, __VA_ARGS__);  \
          break;                                                   \
        case is_int32: \
          CALL_DISPATCH2_2(int32_t, type2, fn, __VA_ARGS__);  \
          break;                                                   \
        case is_bool: \
          CALL_DISPATCH2_2(bool, type2, fn, __VA_ARGS__); \
          break;                                               \
        case is_double: \
          CALL_DISPATCH2_2(double, type2, fn, __VA_ARGS__); \
          break;                                                 \
        case is_string: \
          CALL_DISPATCH2_2(std::string, type2, fn, __VA_ARGS__); \
          break;                                                      \
      }
template<typename> struct js_array;

template<typename T>
struct iterator {
 private:
  js_array<T>* data;
  std::size_t index;
  using iterator_category = std::input_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type*;
  using reference = value_type&;
 public:
  iterator(const js_array<T> * v, std::size_t i):data(v), index(i) {}
  iterator & operator = (iterator && i) {
    if(&i != this) {
      data = i.data;
      index = i.index;
    }
    return *this;
  }
  bool operator == (const iterator &i ) const {
    return index == i.index;
  }
  bool operator != (const iterator &i) const {
    return index != i.index;
  }
  bool operator < (const iterator & i) const {
    return index < i.index;
  }
  bool operator > (const iterator & i)  const {
    return index > i.index;
  }
  bool operator <= (const iterator & i) const {
    return index <= i.index;
  }
  bool operator >= (const iterator & i) const {
    return index >= i.index;
  }
  iterator & operator++() {
    index++;
    return *this;
  }
  iterator operator++(int) {
    auto result(*this);
    index++;
    return result;
  }
  iterator & operator--() {
    index--;
    return *this;
  }
  iterator operator--(int) {
    auto result(*this);
    index--;
    return result;
  }
  const T operator*() const {
    return data->operator[](index);
  }
};
template<typename T>
struct js_array {
  using value_type = T;
  napi_env env;
  napi_value array;
  std::size_t length;

  js_array() = default;

  js_array(napi_env env_, napi_value array_):env(env_), array(array_) {
    uint32_t sz=0;
    napi_get_array_length(env, array, &sz);
    length = sz;
  }
  bool empty() const {
    return length == 0;
  }
  std::size_t size() const {
    return length;
  }
  T operator [](std::size_t index) const {
    napi_value elem;
    napi_status status = napi_get_element(env, array, index, &elem);
    (void)status;
    return convert_to<T>(env, elem);
  }
  iterator<T> begin() const {
    return iterator<T>(this,0);
  }
  iterator<T> end() const {
    return iterator<T>(this,size);
  }
};

inline void throw_js_error(napi_env env, const std::string & msg) {
  NAPI_CALL(napi_throw_error(env, nullptr, msg.c_str()));
}
#endif

