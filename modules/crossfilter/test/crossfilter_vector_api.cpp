#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE CrossFilterDupTest
#include <boost/test/unit_test.hpp>
#include "crossfilter.hpp"
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <utility>
#include <cmath>
#include <limits>
//#include <cstddef>
#include "utils.hpp"

auto Infinity = std::numeric_limits<int>::max();

struct Record2 {
  int a;
  int b;
  std::string c;
  bool operator == (const Record2 & r) const {
    return a == r.a && b == r.b && c == r.c;
  }
};
std::ostream & operator <<(std::ostream & ostr, const Record2 & r) {
  return ostr << "{" << r.a << "," << r.b << "," << r.c << "}";
}
struct RecordIterable {
  int a;
  std::vector<int> v;
};
std::ostream & operator <<(std::ostream & ostr, const RecordIterable & r) {
  ostr << "{" << r.a << ", [";
  for(auto i : r.v) {
    ostr << i << ",";
  }
  return ostr << "]";
}

namespace std {
template<> struct hash<Record2> {
  using argument_type = Record2;
  using result_type = std::size_t;
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
  cross::filter<Record2, std::hash<Record2>> data;
  Fixture() {
    Record2 input[] = {
      {1,2,"aaa"},
      {2,3,"aab"},
      {3,4,"aac"},
      {4,5,"aba"},
    };
    data.add(input);
  }
};

BOOST_AUTO_TEST_CASE(push_back) {
  Fixture f;
  //  auto size = f.data.size();
  f.data.push_back(Record2{1,2,"aa"});
  BOOST_TEST(size_t(5) == f.data.size());
  const Record2 & r = Record2{1,2,"aa"};
  f.data.push_back(r);
  BOOST_TEST(size_t(6) == f.data.size());
}
BOOST_AUTO_TEST_CASE(construct_initializer_list) {
  cross::filter<Record2> data( {{1,2,"aaa"},
                                {2,3,"aab"},
                                {3,4,"aac"},
                                {4,5,"aba"}}
    );
  BOOST_TEST(size_t(4) == data.size());
}
BOOST_AUTO_TEST_CASE(assign_to_empty_with_iterators) {
  cross::filter<Record2> data;
  Record2 input[] = {
    {1,2,"aaa"},
    {2,3,"aab"},
    {3,4,"aac"},
    {4,5,"aba"},
  };
  data.assign(std::begin(input),std::end(input));
  BOOST_TEST(size_t(4) == data.size());
}
BOOST_AUTO_TEST_CASE(assign_to_non_empty_with_iterators) {
  Fixture f;
  Record2 input[] = {
    {1,2,"aaa"},
  };
  BOOST_TEST(size_t(4) == f.data.size());
  f.data.assign(std::begin(input),std::end(input));
  BOOST_TEST(size_t(1) == f.data.size());
}

BOOST_AUTO_TEST_CASE(assign_to_empty_with_value) {
  cross::filter<Record2> data;
  data.assign(10,Record2{1,2,"aaa"});
  BOOST_TEST(size_t(10) == data.size());
  data.assign(10,Record2{1,2,"aaa"},false);
  BOOST_TEST(size_t(1) == data.size());
}
BOOST_AUTO_TEST_CASE(assign_to_non_empty_value) {
  Fixture f;
  BOOST_TEST(size_t(4) == f.data.size());
  f.data.assign(10,Record2{1,2,"aaa"});
  BOOST_TEST(size_t(10) == f.data.size());
  f.data.assign(10,Record2{1,2,"aaa"},false);
  BOOST_TEST(size_t(1) == f.data.size());
}

BOOST_AUTO_TEST_CASE(assign_to_empty_with_initializer_list) {
  cross::filter<Record2> data;
  data.assign( {{1,2,"aaa"},
                {2,3,"aab"},
                {3,4,"aac"},
                {4,5,"aba"}}
    );
  BOOST_TEST(size_t(4) == data.size());
}
BOOST_AUTO_TEST_CASE(assign_to_non_empty_with_initializer_list) {
  Fixture f;
  BOOST_TEST(size_t(4) == f.data.size());
  f.data.assign( {{1,2,"aaa"},
                  {2,3,"aab"},
                  {3,4,"aac"},
                  {4,5,"aba"},
                  {5,6,"abb"}
    });
  BOOST_TEST(size_t(5) == f.data.size());
}

BOOST_AUTO_TEST_CASE(at_throw_exception) {
  Fixture f;
  BOOST_CHECK_THROW(f.data.at(10),std::out_of_range);
  BOOST_TEST((Record2{3,4,"aac"}) == f.data.at(2));
  BOOST_TEST((Record2{4,5,"aba"}) == f.data.at(3));
}

BOOST_AUTO_TEST_CASE(back) {
  Fixture f;
  BOOST_TEST((Record2{4,5,"aba"}) == f.data.back());
  f.data.push_back(Record2{5,6,"abc"});
  BOOST_TEST((Record2{5,6,"abc"}) == f.data.back());
  auto s = f.data.size();
  f.data.remove([s](auto &, int i) { return std::size_t(i) == s-1;});
  BOOST_TEST((Record2{4,5,"aba"}) == f.data.back());
}

BOOST_AUTO_TEST_CASE(iterators) {
  Fixture f;
  auto p = f.data.begin();
  std::size_t i  = 0;
  for(; p != f.data.end(); ++p,++i) {
    BOOST_TEST(*p == f.data.at(i));
  }
  for(p = f.data.begin(), i = 0; p != f.data.end(); ++p,++i) {
    BOOST_TEST(p->c == f.data.at(i).c);
  }

  BOOST_TEST(size_t(std::distance(f.data.begin(), f.data.end())) == f.data.size());
  p = std::begin(f.data);
  std::advance(p,1);
  BOOST_TEST((Record2{2,3,"aab"}) == *p);
}

BOOST_AUTO_TEST_CASE(reverse_iterators) {
  Fixture f;
  auto p = f.data.rbegin();
  std::size_t i = f.data.size() - 1;
  for(; p != f.data.rend(); ++p,--i) {
    BOOST_TEST(*p == f.data.at(i));
  }
  for(p = f.data.rbegin(), i = f.data.size()-1; p != f.data.end(); ++p,--i) {
    BOOST_TEST(p->c == f.data.at(i).c);
  }

  BOOST_TEST(size_t(std::distance(f.data.rbegin(), f.data.rend())) == f.data.size());
  p = std::rbegin(f.data);
  std::advance(p,1);
  BOOST_TEST((Record2{3,4,"aac"}) == *p);
}

BOOST_AUTO_TEST_CASE(accessors) {
  Fixture f;
  BOOST_TEST(f.data.front() == (Record2{1,2,"aaa"}));
  BOOST_TEST(f.data.back() == (Record2{4,5,"aba"}));

  f.data.insert(f.data.begin(),Record2{0,0,"ddd"});
  BOOST_TEST(f.data.front() == (Record2{0,0,"ddd"}));
  BOOST_TEST(f.data.back() == (Record2{4,5,"aba"}));

  f.data.insert(f.data.end(), Record2{9,9,"qqq"});
  BOOST_TEST(f.data.front() == (Record2{0,0,"ddd"}));
  BOOST_TEST(f.data.back() == (Record2{9,9,"qqq"}));

  auto p = f.data.begin();
  std::advance(p,3);
  f.data.insert(p,Record2{7,7,"vvv"});
  BOOST_TEST(f.data.front() == (Record2{0,0,"ddd"}));
  BOOST_TEST(f.data.back() == (Record2{9,9,"qqq"}));
}

BOOST_AUTO_TEST_SUITE(erase)
BOOST_AUTO_TEST_CASE(erase_single_element) {
  Fixture f;
  // erase first element
  f.data.erase(f.data.begin());
  BOOST_TEST(f.data.size() == size_t(3));
  BOOST_TEST(f.data[0] == (Record2{2,3,"aab"}));

  // erase element in the middle
  auto p = f.data.begin();
  std::advance(p,1);
  f.data.erase(p);
  BOOST_TEST(f.data.size() == size_t(2));
  BOOST_TEST(f.data[0] == (Record2{2,3,"aab"}));
  BOOST_TEST(f.data[1] == (Record2{4,5,"aba"}));
  // erase last element
  p = f.data.end();
  p--;
  f.data.erase(p);
  BOOST_TEST(f.data.size() == size_t(1));
  BOOST_TEST(f.data[0] == (Record2{2,3,"aab"}));

  // erase last element from container with 1 element
  f.data.erase(f.data.begin());
  BOOST_TEST(f.data.size() == size_t(0));
}

BOOST_AUTO_TEST_CASE(erase_all_elements) {
  Fixture f;
  f.data.erase(f.data.begin(),f.data.end());
  BOOST_TEST(f.data.empty());

  // try to erase from empty container
  f.data.erase(f.data.begin(),f.data.end());
  BOOST_TEST(f.data.empty());
}

BOOST_AUTO_TEST_CASE(erase_elements_from_start) {
  Fixture f;
  auto p = f.data.begin();
  std::advance(p,2);
  f.data.erase(f.data.begin(),p);
  BOOST_TEST(f.data.all() == (std::vector<Record2>{
        {3,4,"aac"},
        {4,5,"aba"}
     }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(erase_elements_from_middle_to_end) {
  Fixture f;
  auto p = f.data.begin();
  std::advance(p,2);
  f.data.erase(p,f.data.end());
  BOOST_TEST(f.data.all() == (std::vector<Record2>{
        {1,2,"aaa"},
        {2,3,"aab"}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(erase_elements_from_middle) {
  Fixture f;
  auto p = f.data.begin();
  std::advance(p,1);
  auto p1 = p;
  std::advance(p1,2);
  f.data.erase(p,p1);
  BOOST_TEST(f.data.all() == (std::vector<Record2>{
        {1,2,"aaa"},
        {4,5,"aba"}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(erase_dimension_all) {
  Fixture f;
  auto dim = f.data.dimension([](auto v) { return v.a;});
  auto top = dim.top(10);
  BOOST_TEST(top == (std::vector<Record2>{
        {4,5,"aba"},
        {3,4,"aac"},
        {2,3,"aab"},
        {1,2,"aaa"}
      }),boost::test_tools::per_element());
  f.data.erase(f.data.begin(), f.data.end());
  BOOST_TEST(dim.top(10).empty());
}
BOOST_AUTO_TEST_CASE(erase_dimension_begin_to_middle) {
  Fixture f;
  auto p = f.data.begin();
  auto dim = f.data.dimension([](auto v) { return v.a;});
  std::advance(p,2);
  f.data.erase(f.data.begin(),p);
  BOOST_TEST(dim.top(10) == (std::vector<Record2>{
        {4,5,"aba"},
        {3,4,"aac"}
      }),boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(erase_dimension_middle_to_end) {
  Fixture f;
  auto dim = f.data.dimension([](auto v) { return v.a;});
  auto p = f.data.begin();
  std::advance(p,2);
  f.data.erase(p,f.data.end());
  BOOST_TEST(dim.top(10) == (std::vector<Record2>{
        {2,3,"aab"},
        {1,2,"aaa"}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(erase_dimension_middle) {
  Fixture f;
  auto dim = f.data.dimension([](auto v) { return v.a;});
  auto p = f.data.begin();
  std::advance(p,1);
  auto p1 = p;
  std::advance(p1,2);
  f.data.erase(p,p1);
  BOOST_TEST(dim.top(10) == (std::vector<Record2>{
        {4,5,"aba"},
        {1,2,"aaa"}
      }),boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(erase_dimension_middle_filter) {
  Fixture f;
  auto dim = f.data.dimension([](auto v) { return v.a;});
  dim.filter(1);
  auto p = f.data.begin();
  std::advance(p,1);
  auto p1 = p;
  std::advance(p1,2);
  f.data.erase(p,p1);
  BOOST_TEST(dim.top(10) == (std::vector<Record2>{
        {1,2,"aaa"}
      }),boost::test_tools::per_element());
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(insert)
BOOST_AUTO_TEST_CASE(insert_single) {
  cross::filter<int> f;
  f.add(std::vector<int>{1,2,3,4,5});
  
  auto dim = f.dimension([](auto v) { return v;});
  f.insert(f.begin(),10);
  auto top = dim.top(10);
  BOOST_TEST(top[0] == 10);
  f.insert(f.end(),15);
  BOOST_TEST(dim.top(10)[0] == 15);
  BOOST_TEST(dim.bottom(15)[0] == 1);

  cross::filter<int> f1;
  auto dim1 = f1.dimension([](auto v) { return v;});
  f1.insert(f1.begin(),10);
  f1.insert(f1.begin(),15);
  BOOST_TEST(dim1.top(10)[0] == 15);
}

BOOST_AUTO_TEST_CASE(insert_range) {
  cross::filter<int> f;
  auto dim = f.dimension([](auto v) { return v;});
  auto group = dim.feature_count();

  int vv[] = {1,2,3,4,5,5};
  f.insert(f.begin(),std::begin(vv),std::end(vv));
  BOOST_TEST(dim.top(10)[0] == 5);
  BOOST_TEST(f.all() == (std::vector<int>{1,2,3,4,5,5}),boost::test_tools::per_element());
  BOOST_TEST(group.top(10) == (std::vector<std::pair<int,std::size_t>>{
      {5,2},
      {3,1},
      {4,1},
      {2,1},
      {1,1}
    }),boost::test_tools::per_element());

 
}
BOOST_AUTO_TEST_CASE(insert_range2) {
  cross::filter<int> f;
  auto dim = f.dimension([](auto v) { return v;});
  auto group = dim.feature_count();

  int vv[] = {1,2,3,4,5,5};
  f.insert(f.begin(),std::begin(vv),std::end(vv));
  BOOST_TEST(dim.top(10)[0] == 5);
  BOOST_TEST(f.all() == (std::vector<int>{1,2,3,4,5,5}),boost::test_tools::per_element());
  BOOST_TEST(group.top(10) == (std::vector<std::pair<int,std::size_t>>{
        {5,2},
        {3,1},
        {4,1},
        {2,1},
        {1,1}
      }),boost::test_tools::per_element());

  auto p = f.begin();
  std::advance(p,4);
  f.insert(p,std::begin(vv),std::end(vv));
  BOOST_TEST(dim.top(10)[0] == 5);
  BOOST_TEST(f.all() == (std::vector<int>{1,2,3,4,1,2,3,4,5,5,5,5}),boost::test_tools::per_element());
  BOOST_TEST(group.top(10) == (std::vector<std::pair<int,std::size_t>>{
        {5,4},
        {3,2},
        {4,2},
        {2,2},
        {1,2}
      }),boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(insert_iterable_dimension) {
  cross::filter<RecordIterable> f;
  auto dim = f.iterable_dimension([](auto v) { return v.v;});
  RecordIterable vv[] = {
    {1,{1,2,3}},
    {2,{3,4,5}},
    {3,{6,7}},
    {4,{9,10}},
    {5,{1,2}}
  };
  f.insert(f.begin(),std::begin(vv),std::end(vv));
  auto top = dim.top(1);
  BOOST_TEST(top[0].v == (std::vector<int>{9,10}), boost::test_tools::per_element());
  RecordIterable vv1[] = {
    {13,{3}},
    {5,{}}
  };
  f.insert(f.begin(),std::begin(vv1),std::end(vv1));
  auto top1 = dim.top(1);
  BOOST_TEST(top1[0].v == (std::vector<int>{9,10}), boost::test_tools::per_element());
  RecordIterable vv2[] = {
    {13,{15}},
    {5,{}}
  };
  auto p = f.begin();
  std::advance(p,3);
  f.insert(p,std::begin(vv2),std::end(vv2));
  auto top2 = dim.top(1);
  BOOST_TEST(top2[0].v == (std::vector<int>{15}), boost::test_tools::per_element());
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(emplace)
BOOST_AUTO_TEST_CASE(simple) {
  cross::filter<int> f;
  f.emplace_back(1);
  BOOST_TEST(f.all() == (std::vector<int>{1}));
  f.emplace_back(2);
  BOOST_TEST(f.all() == (std::vector<int>{1,2}));
}
BOOST_AUTO_TEST_CASE(check_constructors) {
  struct Rec {
    int i;
    Rec() : i(0) {}
    Rec(int k):i(k) {}
    Rec(Rec && k):i(k.i + 1) {}
    Rec(const Rec & k):i(k.i+1) {}
    Rec & operator=(const Rec&) = default;
    Rec & operator=(Rec && k) {
      i = k.i+1;
      return *this;
    }
  };
  cross::filter<Rec> f;
  f.emplace_back(10);
  BOOST_TEST(f.data()[0].i == 10);
  f.push_back(Rec{0});
  BOOST_TEST(f.data()[1].i == 1);
  Rec r{0};
  f.push_back(r);
  BOOST_TEST(f.data()[2].i == 1);
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(filter)
BOOST_AUTO_TEST_CASE(insert_before_filter) {
  Fixture f;
  auto dim = f.data.dimension([](auto v) { return v.a;});
  f.data.insert(f.data.begin(),Record2{0,0,"ddd"});
  dim.filter(2,4);
  auto filtered = f.data.all_filtered();
  BOOST_TEST(filtered == (std::vector<Record2>{
        {2,3,"aab"},
        {3,4,"aac"}
      }),boost::test_tools::per_element());
  dim.filter_all();
  BOOST_TEST(f.data.all_filtered() == (std::vector<Record2>{
        {0,0,"ddd"},
        {1,2,"aaa"},
        {2,3,"aab"},
        {3,4,"aac"},
        {4,5,"aba"}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(insert_after_filter) {
  Fixture f;
  auto dim = f.data.dimension([](auto v) { return v.a;});
  dim.filter(2,4);
  f.data.insert(f.data.begin(),Record2{0,0,"ddd"});
  BOOST_TEST(f.data.all_filtered() == (std::vector<Record2>{
        {2,3,"aab"},
        {3,4,"aac"}
      }),boost::test_tools::per_element());
  dim.filter_all();
  BOOST_TEST(f.data.all_filtered() == (std::vector<Record2>{
        {0,0,"ddd"},
        {1,2,"aaa"},
        {2,3,"aab"},
        {3,4,"aac"},
        {4,5,"aba"}
      }),boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(erase_before_filter) {
  Fixture f;
  auto dim = f.data.dimension([](auto v) { return v.a;});
  auto p = f.data.begin();
  std::advance(p,1);
  f.data.erase(p);
  BOOST_TEST(f.data.all_filtered() == (std::vector<Record2>{
        {1,2,"aaa"},
        {3,4,"aac"},
        {4,5,"aba"}
      }),boost::test_tools::per_element());

  dim.filter(2,4);
  auto filtered = f.data.all_filtered();
  BOOST_TEST(filtered == (std::vector<Record2>{
        {3,4,"aac"}
      }),boost::test_tools::per_element());
  dim.filter_all();
  BOOST_TEST(f.data.all_filtered() == (std::vector<Record2>{
        {1,2,"aaa"},
        {3,4,"aac"},
        {4,5,"aba"}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(erase_after_filter) {
  Fixture f;
  auto dim = f.data.dimension([](auto v) { return v.a;});
  dim.filter(2,4);
  BOOST_TEST(f.data.all_filtered() == (std::vector<Record2>{
        {2,3,"aab"},
        {3,4,"aac"}
      }),boost::test_tools::per_element());
  auto p = f.data.begin();
  std::advance(p,1);
  f.data.erase(p);

  auto filtered = f.data.all_filtered();
  BOOST_TEST(filtered == (std::vector<Record2>{
        {3,4,"aac"}
      }),boost::test_tools::per_element());
  dim.filter_all();
  BOOST_TEST(f.data.all_filtered() == (std::vector<Record2>{
        {1,2,"aaa"},
        {3,4,"aac"},
        {4,5,"aba"}
      }),boost::test_tools::per_element());
}

BOOST_AUTO_TEST_SUITE_END();
