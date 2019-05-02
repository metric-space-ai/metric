#ifndef MAKE_LAMBDA_H_GUARD
#define MAKE_LAMBDA_H_GUARD
struct is_int64_t_tag {};
struct is_int32_t_tag {};
struct is_double_tag {};
struct is_bool_tag {};
struct is_string_tag {};
struct is_uint64_t_tag {};
using string = std::string;

template<typename T>
struct define_tag {};

template<>
struct define_tag<int64_t> {
  using type = is_int64_t_tag;
};
template<>
struct define_tag<int32_t> {
  using type = is_int32_t_tag;
};
template<>
struct define_tag<double> {
  using type = is_double_tag;
};
template<>
struct define_tag<bool> {
  using type = is_bool_tag;
};
template<>
struct define_tag<std::string> {
  using type = is_string_tag;
};
template<>
struct define_tag<uint64_t> {
  using type = is_uint64_t_tag;
};

template<typename T>
struct tag_to_type {};

template<> struct tag_to_type<is_int64_t_tag> {
  using type = int64_t;
};
template<> struct tag_to_type<is_int32_t_tag> {
  using type = int32_t;
};
template<> struct tag_to_type<is_bool_tag> {
  using type = bool;
};
template<> struct tag_to_type<is_double_tag> {
  using type = double;
};
template<> struct tag_to_type<is_string_tag> {
  using type = std::string;
};
template<> struct tag_to_type<is_uint64_t_tag> {
  using type = uint64_t;
};

#endif
