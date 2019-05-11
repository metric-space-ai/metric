#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE CrossFilterDupTest
#include <boost/test/unit_test.hpp>
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <utility>
#include <cmath>
#include "crossfilter.hpp"

auto Infinity = std::numeric_limits<int>::max();

struct Record {
  int a;
  int b;
  std::string c;
};
namespace std {
template<> struct hash<Record> {
  typedef Record argument_type;
  typedef std::size_t result_type;
  result_type operator()(argument_type const& s) const noexcept
  {
    result_type const h1 ( std::hash<int>{}(s.a) );
    result_type const h2 ( std::hash<int>{}(s.b) );
    result_type const h3 ( std::hash<std::string>{}(s.c) );
    return (h1 ^ (h2 << 1)) ^ (h3 << 1); // or use boost::hash_combine (see Discussion)
  }
};
}
struct Fixture {
  cross::filter<Record, std::hash<Record>> data;
  Fixture() {
    Record input[] = {
      {1,2,"aaa"},
      {2,3,"aab"},
      {3,4,"aac"},
      {4,5,"aba"},
    };
    data.add(input);
  }
};
BOOST_AUTO_TEST_CASE(add_without_duplicate) {
  Fixture f;
  auto size = f.data.size();
  f.data.add(Record{1,2,"aaa"},false);
  BOOST_TEST(size == f.data.size());
}

BOOST_AUTO_TEST_CASE(add_duplicate_remove_and_add_again) {
    Fixture f;
    auto size = f.data.size();
    f.data.add(Record{1,2,"aaa"},true);
    BOOST_TEST((size + 1)== f.data.size());
    f.data.remove([](auto v,int) { return v.a == 1;});
    BOOST_TEST(f.data.size() == size_t(3));
    f.data.add(Record{1,2,"aaa"},false);
    BOOST_TEST(f.data.size() == size_t(4));
}

BOOST_AUTO_TEST_CASE(add_duplicate_filter_remove_and_add_again) {
    Fixture f;
    auto size = f.data.size();
    f.data.add(Record{1,2,"aaa"},true);
    BOOST_TEST((size + 1)== f.data.size());
    auto dim = f.data.dimension([](auto v) { return v.a;});
    dim.filter(1);
    f.data.remove();
    BOOST_TEST(f.data.size() == size_t(3));
    f.data.add(Record{1,2,"aaa"},false);
    BOOST_TEST(f.data.size() == size_t(4));
}

BOOST_AUTO_TEST_CASE(add_batch_of_data) {
    Fixture f;
    Record in2[] = {
            {1,2,"aaa"},
            {2,3,"aab"},
            {3,4,"aac"},
            {4,5,"aba"},
    };
    f.data.add(in2,false);
    BOOST_TEST( size_t(4)== f.data.size());
}

