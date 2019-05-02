#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE CrossFilterJSTest

#include "utils.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <utility>
#include <cmath>
#include "crossfilter.hpp"

auto Infinity = std::numeric_limits<int>::max();

struct Fixture {
  cross::filter<Record> cr;
  cross::dimension<std::string,Record> date;
  cross::dimension<int,Record> quantity;
  cross::dimension<int,Record> total;
  cross::dimension<int,Record> tip;
  cross::dimension<std::string,Record> type;
  cross::dimension<std::vector<int>,Record, cross::iterable> tags;
  //  cross::Dimension<int,cross::filterImpl<Record>,true> tags;
  Fixture() {
    Record data[] = {
      {"2011-11-14T16:17:54Z", 2,  190,  100,  "tab",  {1,2,3}},
      { "2011-11-14T16:20:19Z",  2,  190,  100,  "tab", {1,3}},
      { "2011-11-14T16:28:54Z",  1,  300,  200,  "visa", {2,4,5}},
      { "2011-11-14T16:30:43Z",  2,  90,   0,  "tab", {2,3,4}},
      { "2011-11-14T16:48:46Z",  2,  90,   0,  "tab", {1,2,3}},
      { "2011-11-14T16:53:41Z",  2,  90,   0,  "tab", {1,3}},
      { "2011-11-14T16:54:06Z",  1,  100,  0,  "cash", {2,4,5}},
      { "2011-11-14T17:02:03Z",  2,  90,   0,  "tab", {2,3,4}},
      { "2011-11-14T17:07:21Z",  2,  90,   0,  "tab", {1,2,3}},
      { "2011-11-14T17:22:59Z",  2,  90,   0,  "tab", {}},
      { "2011-11-14T17:25:45Z",  2,  200,  0,  "cash", {2,4,5}},
      { "2011-11-14T17:29:52Z",  1,  200,  100,  "visa", {-1, 0, 5, 5}},
      { "2011-11-14T17:33:46Z",  2,  190,  100,  "tab", {1,2,3}},
      { "2011-11-14T17:33:59Z",  2,  90,   0,  "tab", {1,3}},
      { "2011-11-14T17:38:40Z",  2,  200,  100,  "visa", {2,4,5}},
      { "2011-11-14T17:52:02Z",  2,  90,   0,  "tab", {2,3,4}},
      { "2011-11-14T18:02:42Z",  2,  190,  100,  "tab", {1,2,3}},
      { "2011-11-14T18:02:51Z",  2,  190,  100,  "tab", {1,3}},
      { "2011-11-14T18:12:54Z",  1,  200,  100,  "visa", {2,4,5}},
      { "2011-11-14T18:14:53Z",  2,  100,  0,  "cash", {2,3,4}},
      { "2011-11-14T18:45:24Z",  2,  90,   0,  "tab", {2,4,5}},
      { "2011-11-14T19:00:31Z",  2,  190,  100,  "tab", {2,3,4}},
      { "2011-11-14T19:04:22Z",  2,  90,   0,  "tab", {1,2,3}},
      { "2011-11-14T19:30:44Z",  2,  90,   0,  "tab", {1,3}},
      { "2011-11-14T20:06:33Z",  1,  100,  0,  "cash", {2,4,5}},
      { "2011-11-14T20:49:07Z",  2,  290,  200,  "tab", {2,4,5}},
      { "2011-11-14T21:05:36Z",  2,  90,   0,  "tab", {2,3,4}},
      { "2011-11-14T21:18:48Z",  4,  270,  0,  "tab", {1,2,3}},
      { "2011-11-14T21:22:31Z",  1,  200,  100,  "visa", {1,3}},
      { "2011-11-14T21:26:30Z",  2,  190,  100,  "tab", {2,4,5}},
      { "2011-11-14T21:30:55Z",  2,  190,  100,  "tab", {2,3,4}},
      { "2011-11-14T21:31:05Z",  2,  90,   0,  "tab", {1,2,3}},
      { "2011-11-14T22:30:22Z",  2,  89,   0,  "tab", {1,3}},
      { "2011-11-14T22:34:28Z",  2,  190,  100,  "tab", {2,4,5}},
      { "2011-11-14T22:48:05Z",  2,  91,   0,  "tab", {2,4,5}},
      { "2011-11-14T22:51:40Z",  2,  190,  100,  "tab", {2,3,4}},
      { "2011-11-14T22:58:54Z",  2,  100,  0,  "visa", {2,3,4}},
      { "2011-11-14T23:06:25Z",  2,  190,  100,  "tab", {1,2,3}},
      { "2011-11-14T23:07:58Z",  2,  190,  100,  "tab", {1,3}},
      { "2011-11-14T23:16:09Z",  1,  200,  100,  "visa", {2,4,5}},
      { "2011-11-14T23:21:22Z",  2,  190,  100,  "tab", {2,4,5}},
      { "2011-11-14T23:23:29Z",  2,  190,  100,  "tab", {2,3,4}},
      { "2011-11-14T23:28:54Z",  2,  190,  100,  "tab", {1,2,3}}
    };
    cr.add(data);
    date = cr.dimension([](auto r) { return r.date;});
    total = cr.dimension([](auto r) { return r.total;});
    quantity = cr.dimension([](auto r) { return r.quantity;});
    type = cr.dimension([](auto r) { return r.type;});
    tip = cr.dimension([](auto r) { return r.tip;});
    tags = cr.iterable_dimension([](auto r) { return r.tags;});
    
  }
};

struct RecIterable {
  std::vector<int> foo;
  int bar;
};
struct Tags {
  std::vector<int> tags;
  bool operator == (const Tags & rhs) const { return tags == rhs.tags;}
};
struct RecIterable2 {
  int foo;
  std::vector<int> val;
  bool operator == (const RecIterable2 & rhs) const { return foo == rhs.foo && val == rhs.val;}
};

struct RecIterable3 {
  std::string name;
  int quantity;
  std::vector<int> tags;
  bool operator == (const RecIterable3 & rhs) const { return name == rhs.name && quantity == rhs.quantity && tags == rhs.tags;}

};

bool indexOf(const std::vector<int> & v, int val) {
   return std::find(v.begin(), v.end(), val) != v.end();
}
BOOST_TEST_DONT_PRINT_LOG_VALUE(RecIterable)
BOOST_TEST_DONT_PRINT_LOG_VALUE(RecIterable2)
BOOST_TEST_DONT_PRINT_LOG_VALUE(RecIterable3)
BOOST_TEST_DONT_PRINT_LOG_VALUE(Tags)
struct RecEmpty {
  std::string name;
  std::vector<std::string> labels;
  bool operator == (const RecEmpty & lhs) const {
    return name == lhs.name && labels == lhs.labels;
  }
};

struct Fixture2 {
  cross::filter<RecEmpty> cr;
  cross::dimension<std::vector<std::string>,RecEmpty,cross::iterable> labels;
  Fixture2() {
    RecEmpty data[] = {
      {"apha", {}},
      {"bravo", {}},
      {"charle", {}},
      {"delta", {}},
      {"echo",  {"courageous"}}
    };
    cr.add(data);
    labels = cr.iterable_dimension([](auto d) {return d.labels;} );
  }
};

#define CHECK_MAX_ELEM(array, value)                        \
  {                                                         \
    auto m = std::max_element(array.begin(), array.end());  \
    BOOST_TEST((m != array.end()));                         \
    BOOST_CHECK_EQUAL(*m,(value));                          \
  }

#define CHECK_MIN_ELEM(array, value)                        \
  {                                                         \
    auto m = std::min_element(array.begin(), array.end());  \
    BOOST_TEST((m != array.end()));                         \
    BOOST_CHECK_EQUAL(*m,(value));                          \
  }

BOOST_TEST_DONT_PRINT_LOG_VALUE(RecEmpty)
BOOST_AUTO_TEST_SUITE(iterablesEmptyRows)
BOOST_AUTO_TEST_CASE(returns_top_k_records_by_value_placing_non_empty_row_on_top) {
  Fixture2 data;
  auto t = data.labels.top(5);
  BOOST_TEST(t == (std::vector<RecEmpty>{
        {"echo",{"courageous" }},
        {"apha",{}},
        {"bravo",{}},
        {"charle",{}},
        {"delta",{}}
            }), boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(returns_the_top_k_records_using_offset_by_value) {
  Fixture2 data;
  auto t = data.labels.top(3,2);
  BOOST_TEST(t == (std::vector<RecEmpty>{
        {"bravo",{}},
        {"charle",{}},
        {"delta",{}}
            }), boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(returns_bottom_k_records_by_value_placing_non_empty_row_on_bottom) {
  Fixture2 data;
  auto t = data.labels.bottom(5);
  BOOST_TEST(t == (std::vector<RecEmpty>{
        {"apha",{}},
        {"bravo",{}},
        {"charle",{}},
        {"delta",{}},
        {"echo",{"courageous" }}
            }), boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(returns_the_bottom_k_records_using_offset_by_value_in_descending_order) {
  Fixture2 data;
  auto t = data.labels.bottom(3,2);
  BOOST_TEST(t == (std::vector<RecEmpty>{
        {"charle",{}},
        {"delta",{}},
        {"echo",{"courageous" }}
      }), boost::test_tools::per_element());
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterableDimension_top)
BOOST_AUTO_TEST_CASE(returns_top_k_records_by_value_in_descending_order){
  Fixture data;
  
  auto top = data.tags.top(3);
  BOOST_CHECK_EQUAL(top.size(),(unsigned long)3);
  auto m1 = std::max_element(top[0].tags.begin(), top[0].tags.end());
  BOOST_TEST((m1 != top[0].tags.end()));
  BOOST_CHECK_EQUAL(*m1,5);

  auto m2 = std::max_element(top[1].tags.begin(), top[1].tags.end());
  BOOST_TEST((m2 != top[1].tags.end()));
  BOOST_CHECK_EQUAL(*m2,5);
  auto m3 = std::max_element(top[2].tags.begin(), top[2].tags.end());
  BOOST_TEST((m3 != top[2].tags.end()));
  BOOST_CHECK_EQUAL(*m3,5);
}
BOOST_AUTO_TEST_CASE(observes_associated_dimension_filters) {
  Fixture data;
  data.tags.filter_exact(1);
  auto top = data.tags.top(3);
  BOOST_CHECK_EQUAL(top.size(),(unsigned long)3);
  BOOST_TEST(indexOf(top[0].tags,1));
  BOOST_TEST(indexOf(top[1].tags,1));
  BOOST_TEST(indexOf(top[2].tags,1));
}
BOOST_AUTO_TEST_CASE(observes_other_dimensions_filters) {
  Fixture data;
  data.quantity.filter_exact(4);
  BOOST_TEST(data.tags.top(1) == (std::vector<Record>{
        {"2011-11-14T21:18:48Z", 4, 270,  0, "tab", {1,2,3}}
      }), boost::test_tools::per_element());
  data.quantity.filter_all();
  data.type.filter_exact("visa");
  BOOST_TEST(data.tags.top(1) == (std::vector<Record>{
        {"2011-11-14T16:28:54Z", 1, 300,  200, "visa", {2,4,5}}
      }), boost::test_tools::per_element());
  data.quantity.filter_exact(2);
  BOOST_TEST(data.tags.top(1) == (std::vector<Record>{
        {"2011-11-14T17:38:40Z", 2, 200,  100, "visa", {2,4,5}}
      }), boost::test_tools::per_element());
  data.type.filter_all();
  data.quantity.filter_all();

  data.type.filter_exact("tab");
  BOOST_TEST(data.date.top(2) == (std::vector<Record>{
        {"2011-11-14T23:28:54Z", 2, 190,  100, "tab", {1,2,3}},
        {"2011-11-14T23:23:29Z", 2, 190,  100, "tab", {2,3,4}}
      }), boost::test_tools::per_element());

  data.type.filter_exact("visa");

  BOOST_TEST(data.date.top(1) == (std::vector<Record>{
        {"2011-11-14T23:16:09Z", 1, 200,  100, "visa", {2,4,5}}
      }), boost::test_tools::per_element());

  data.quantity.filter_exact(2);
  BOOST_TEST(data.date.top(1) == (std::vector<Record>{
        {"2011-11-14T22:58:54Z", 2, 100,  0, "visa", {2,3,4}}
      }), boost::test_tools::per_element());
  
}
BOOST_AUTO_TEST_CASE(negative_or_zero_k_returns_an_empty_array) {
  Fixture data;
  BOOST_TEST(data.tags.top(0).empty());
  BOOST_TEST(data.tags.top(-1).empty());
  BOOST_TEST(data.tags.top(std::numeric_limits<int64_t>::min()).empty());
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterableDimension_bottom)
BOOST_AUTO_TEST_CASE(returns_the_bottom_k_records_by_value_in_descending_order) {
  Fixture data;
  auto bottom = data.tags.bottom(3);
  BOOST_CHECK_EQUAL(bottom[0].tags.size(),(unsigned long)0);
  BOOST_CHECK_EQUAL(bottom[1].tags[0],-1);
  BOOST_CHECK_EQUAL(bottom[1].tags[1],0);
}
BOOST_AUTO_TEST_CASE(observes_the_associated_dimensions_filters) {
  Fixture data;
  data.quantity.filter_exact(4);
  BOOST_TEST(data.tags.bottom(3) == (std::vector<Record>{
        {"2011-11-14T21:18:48Z", 4, 270,  0, "tab", {1,2,3}},
        {"2011-11-14T21:18:48Z", 4, 270,  0, "tab", {1,2,3}},
        {"2011-11-14T21:18:48Z", 4, 270,  0, "tab", {1,2,3}}
      }), boost::test_tools::per_element());
  data.quantity.filter_all();

  data.date.filter_range("2011-11-14T19:00:00Z","2011-11-14T20:00:00Z");
  auto t = data.tags.bottom(10);
  
  BOOST_TEST(t  == (std::vector<Record>{
      {"2011-11-14T19:04:22Z",  2,  90,  0,  "tab",  { 1, 2, 3 }},
      { "2011-11-14T19:30:44Z", 2,  90,  0,  "tab",  { 1, 3 }},
      { "2011-11-14T19:04:22Z", 2,  90,  0,  "tab",  { 1, 2, 3 }},
      { "2011-11-14T19:00:31Z", 2,  190, 100, "tab",  { 2, 3, 4 }},
      { "2011-11-14T19:00:31Z", 2,  190, 100,  "tab", { 2, 3, 4 }},
      { "2011-11-14T19:04:22Z", 2,  90,  0,  "tab",  { 1, 2, 3 }},
      { "2011-11-14T19:30:44Z", 2,  90,  0,  "tab",  { 1, 3 }},
      { "2011-11-14T19:00:31Z", 2,  190, 100,  "tab",  { 2, 3, 4 }}
      }), boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(observers_other_dimension_filters) {
  Fixture data;

  data.type.filter_exact("tab");
  BOOST_TEST(data.tags.bottom(2)  == (std::vector<Record>{
        {"2011-11-14T17:22:59Z",  2,  90,  0,  "tab",  {}},
        {"2011-11-14T16:20:19Z",  2,  190,  100,  "tab",  {1,3}}
      }));
  data.type.filter_exact("visa");
  BOOST_TEST(data.tags.bottom(1)  == (std::vector<Record>{
        {"2011-11-14T17:29:52Z",  1,  200,  100,  "visa",  {-1,0,5,5}}
      }));
  data.quantity.filter_exact(2);
  auto t = data.tags.bottom(1);

  BOOST_TEST(t  == (std::vector<Record>{
        { "2011-11-14T22:58:54Z",  2,  100,  0,  "visa", {2,3,4}}
      }));

}

BOOST_AUTO_TEST_CASE(negative_or_zero_k_returns_an_empty_array) {
  Fixture data;
  BOOST_TEST(data.tags.bottom(0).empty());
  BOOST_TEST(data.tags.bottom(-1).empty());
  BOOST_TEST(data.tags.bottom(std::numeric_limits<int64_t>::min()).empty());
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterableDimension_filter_exact)
BOOST_AUTO_TEST_CASE(selects_records_that_match_the_specified_value_exactly) {
  Fixture data;
  data.tip.filter_exact(100);

  BOOST_TEST(data.tags.top(2)  == (std::vector<Record>{
        { "2011-11-14T22:34:28Z",  2,  190,  100,  "tab", {2,4,5}},
        { "2011-11-14T23:21:22Z",  2,  190,  100,  "tab", {2,4,5}}
      }));
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterableDimension_filter_range)
BOOST_AUTO_TEST_CASE(selects_records_greater_than_or_equal_to_the_inclusive_lower_bound) {
  Fixture data;
  data.total.filter_range(100,190);
  auto t1 = data.tags.top(Infinity);
  
  BOOST_TEST(std::all_of(t1.begin(),t1.end(),[](auto r) { return r.total >= 100;}));

  data.total.filter_range(110,190);
  auto t2 = data.tags.top(Infinity);
  
  BOOST_TEST(std::all_of(t2.begin(),t2.end(),[](auto r) { return r.total >= 110;}));
}
BOOST_AUTO_TEST_CASE(selects_records_less_than__the_exclusive_upper_bound) {
  Fixture data;
  data.total.filter_range(100,200);
  auto t1 = data.tags.top(Infinity);
  BOOST_TEST(std::all_of(t1.begin(),t1.end(),[](auto r) {  return r.total < 200;}));

  data.total.filter_range(100,190);
  auto t2 = data.tags.top(Infinity);

  BOOST_TEST(std::all_of(t2.begin(),t2.end(),[](auto r) {  return r.total < 190;}));
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterableDimension_filter_all)
BOOST_AUTO_TEST_CASE(clears_the_filter) {
  Fixture data;
  data.total.filter_range(100,200);
  BOOST_TEST(data.tags.top(Infinity).size() < (unsigned long)120);
  data.total.filter_all();
  BOOST_TEST(data.tags.top(Infinity).size() == (unsigned long)120);
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterableDimension_filter_function)
BOOST_AUTO_TEST_CASE(selects_records_according_to_an_arbitrary_function) {
  Fixture data;
  data.total.filter_function([](auto r) { return r % 2;});
  auto t = data.tags.top(Infinity);
  BOOST_TEST(std::all_of(t.begin(),t.end(),[](auto r) {  return r.total % 2;}));
}
BOOST_AUTO_TEST_CASE(groups_on_the_first_dimension_are_updated_correctly) {
  Fixture data;
  auto group = data.tags.feature_all_count();
  data.total.filter_function([](auto d) {return d == 90;});
  BOOST_CHECK_EQUAL(group.value(),(unsigned long)33);
  data.total.filter_function([](auto d) {return d == 91;});
  BOOST_CHECK_EQUAL(group.value(),(unsigned long)3);
}

BOOST_AUTO_TEST_CASE(followed_by_filter_range) {
  Fixture data;

  data.total.filter_function([](auto d) { return d % 2;});
  data.total.filter_range(100,200);
  BOOST_CHECK_EQUAL(data.tags.top(Infinity).size(), (unsigned long)54);
}

BOOST_AUTO_TEST_CASE(group_values_with_multiple_filters_on_and_off_on_standard_dimension) {
  Fixture data;
  auto group = data.tags.feature_count();
  data.total.filter_function([](auto d) {return d == 90;});
  auto g = group.all();
  BOOST_CHECK_EQUAL(g[g.size()-1].second,(unsigned long)1);
  data.total.filter_all();
  data.total.filter_function([](auto d) {return d == 91;});
  g = group.all();
  BOOST_CHECK_EQUAL(g[g.size()-1].second,(unsigned long)1);
  data.total.filter_all();
  g = group.all();
  BOOST_CHECK_EQUAL(g[g.size()-1].second,(unsigned long)15);
}

BOOST_AUTO_TEST_CASE(group_values_with_multiple_filters_on_and_off_on_iterable_dimension) {
  Fixture data;
  auto group = data.total.feature_all_count();
  BOOST_CHECK_EQUAL(group.value(),(unsigned long)43);
  
  data.tags.filter_function([](auto d) {return d == 1;});
  BOOST_CHECK_EQUAL(group.value(),(unsigned long)18);
  data.tags.filter_all();
  BOOST_CHECK_EQUAL(group.value(),(unsigned long)43);
  data.tags.filter_function([](auto d) {return d == 1;});
  BOOST_CHECK_EQUAL(group.value(),(unsigned long)18);
  data.tags.filter_all();
  BOOST_CHECK_EQUAL(group.value(),(unsigned long)43);
}
BOOST_AUTO_TEST_CASE(group_values_with_multiple_overlapped_filters) {
  Fixture data;
  auto group = data.total.feature_all_count();
  BOOST_CHECK_EQUAL(group.value(),(unsigned long)43);
  
  data.tags.filter_function([](auto d) {return d == 1;});
  BOOST_CHECK_EQUAL(group.value(),(unsigned long)18);
  data.tags.filter_function([](auto d) {return d == 2;});
  BOOST_CHECK_EQUAL(group.value(),(unsigned long)33);
  data.tags.filter_all();
  BOOST_CHECK_EQUAL(group.value(),(unsigned long)43);
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterableDimension_filter)
BOOST_AUTO_TEST_CASE(is_equivalent_to_filter_range) {
  Fixture data;
  data.total.filter(100,190);
  auto t1 = data.tags.top(Infinity);
  BOOST_TEST(std::all_of(t1.begin(),t1.end(),[](auto r) { return r.total >= 100;}));  
}
BOOST_AUTO_TEST_CASE(is_equivalent_to_filter_exact) {
  Fixture data;
  data.total.filter(100);
  auto t1 = data.tags.top(Infinity);
  BOOST_TEST(std::all_of(t1.begin(),t1.end(),[](auto r) { return r.total == 100;}));  
}
BOOST_AUTO_TEST_CASE(is_equivalent_to_filter_function) {
  Fixture data;
  data.total.filter([](auto d) { return d % 2;});
  auto t1 = data.tags.top(Infinity);
  BOOST_TEST(std::all_of(t1.begin(),t1.end(),[](auto r) { return r.total % 2;}));  
}
BOOST_AUTO_TEST_CASE(is_equivalent_to_filter_all) {
  Fixture data;
  data.total.filter(100,200);
  BOOST_TEST(data.tags.top(Infinity).size() < (unsigned long)120);
  data.total.filter();
  BOOST_CHECK_EQUAL(data.tags.top(Infinity).size(),(unsigned long)120);
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterableDimension_group)
BOOST_AUTO_TEST_CASE(key_defaults_to_value) {
  Fixture data;
  auto all = data.tags.feature_count();
  auto t = all.top(Infinity);
  
  BOOST_TEST(t == (std::vector<std::pair<int,std::size_t>>{
        {2,33},
        {3,28},
        {4,23},
        {1,18},
        {5,15},
        {0,1},
        {-1,1}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(cardinality_may_be_greater_than_65536) {
  cross::filter<Tags> data;
  std::vector<Tags> in;
  for(int i = 0; i < 65536; i++) {
    Tags t{{i,i+1,i+2}};
    in.push_back(t);
  }
  in.push_back({{65536,65537,65538} });
  in.push_back({{65536,65537,65538} });
  data.add(in);
  auto index = data.iterable_dimension([](auto d) { return d.tags;});
  auto indexes = index.feature_count();
  auto t = index.top(2);
  BOOST_TEST(t == (std::vector<Tags>{
        {{ 65536,65537,65538
                }},
        {{ 65536,65537,65538
                }},
            }),boost::test_tools::per_element());
  BOOST_TEST(indexes.top(1) == (std::vector<std::pair<int,std::size_t>>{
        {65536,4
        }
      }),boost::test_tools::per_element());
  BOOST_CHECK_EQUAL(indexes.size(),(unsigned long)65539);
}
BOOST_AUTO_TEST_CASE(adds_all_records_before_removing_filtered) {
  Fixture data;
  data.quantity.filter(1);

  auto addGroup = data.tags.feature(
      [](auto p, auto, bool) { ++p; return p;},
      [](auto p, auto, bool) { return p;},
      []() { return 0;}
                                    );
  auto stdGroup = data.tags.feature_count();
  auto t1 = addGroup.top(1);
  auto t2 = stdGroup.top(1);
  BOOST_TEST((unsigned long)t1[0].second > t2[0].second);
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterableDimension_group_size)

BOOST_AUTO_TEST_CASE(returns_the_cardinality) {
  Fixture data;
  auto all = data.tags.feature_count();
  auto hours = data.date.feature_count([](auto d) {
      auto s = d.substr(0,13);
      return s + ":00:00Z";
    });
  BOOST_CHECK_EQUAL(hours.size(),(unsigned long)8);
  BOOST_CHECK_EQUAL(all.size(),(unsigned long)7);
}
BOOST_AUTO_TEST_CASE(ignores_any_filters) {
  Fixture data;
  auto all = data.tags.feature_count();
  auto hours = data.date.feature_count([](auto d) {
      auto s = d.substr(0,13);
      return s + ":00:00Z";
    });
  data.tags.filter_exact(1);
  data.quantity.filter_range(100,200);
  BOOST_CHECK_EQUAL(hours.size(),(unsigned long)8);
  BOOST_CHECK_EQUAL(all.size(),(unsigned long)7);
}
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE(iterableDimension_group_reduce)
BOOST_AUTO_TEST_CASE(defaults_to_count) {
  Fixture data;
  auto all = data.tags.feature_count();
  BOOST_TEST(all.top(1) == (std::vector<std::pair<int,std::size_t>>{
        {2,33
        }
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(determines_the_computed_reduce_value) {
  Fixture data;
  auto all = data.tags.feature_sum([](auto d) { return d.total;});

  BOOST_TEST(all.top(Infinity) == (std::vector<std::pair<int,int>>{
        { 2, 5241},
        { 3, 4029},
        { 4, 3661},
        { 5, 2741},
        { 1, 2709},
        { 0, 200},
        { -1, 200}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE(gives_reduce_functions_information_on_lifecycle_of_data_element)
BOOST_AUTO_TEST_CASE(on_group_creation) {
  cross::filter<RecIterable2> data;
  RecIterable2 in[] = {
    { 1,{1,2}},
    { 2,{1,2}},
    { 3,{3,4,5}},
    { 3,{1,2}}
  };
  data.add(in);
  auto foo = data.dimension([](auto d) { return d.foo;});
  auto bar = data.dimension([](auto d) { return d.foo;});
  auto val = data.iterable_dimension([](auto d) { return d.val;});
  auto groupSumLength = val.feature(
      [](auto p, auto v, bool n) {
        if(n)
          p += v.val.size();
        return p;
      },
      [](auto p, auto v, bool n) {
        if(n)
          p -= v.val.size();
        return p;
      },
      []() {
        return std::size_t(0);
      }
                                  );
  auto groupSumEach = val.feature_sum([](auto d) { return d.val.size();});
  BOOST_TEST(groupSumLength.all() == groupSumEach.all());
}

BOOST_AUTO_TEST_CASE(on_filtering) {
  cross::filter<RecIterable2> data;
  RecIterable2 in[] = {
    { 1,{1,2}},
    { 2,{1,2}},
    { 3,{3,4,5}},
    { 3,{1,2}}
  };
  data.add(in);
  auto foo = data.dimension([](auto d) { return d.foo;});
  auto bar = data.dimension([](auto d) { return d.foo;});
  auto val = data.iterable_dimension([](auto d) { return d.val;});
  auto groupSumLength = val.feature(
      [](auto p, auto v, bool n) {
        if(n)
          p += v.val.size();
        return p;
      },
      [](auto p, auto v, bool n) {
        if(n)
          p -= v.val.size();
        return p;
      },
      []() {
        return std::size_t(0);
      }
                                  );
  auto groupSumEach = val.feature_sum([](auto d) { return d.val.size();});

  foo.filter(1,2);
  BOOST_TEST(groupSumLength.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,6},
        {2,6},
        {3,3},
        {4,3},
        {5,3}
      } ), boost::test_tools::per_element());
  BOOST_TEST(groupSumEach.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,2},
        {2,2},
        {3,0},
        {4,0},
        {5,0}
      } ), boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(on_adding_data_after_group_creation) {
  cross::filter<RecIterable2> data;
  RecIterable2 in[] = {
    { 1,{1,2}},
    { 2,{1,2}},
    { 3,{3,4,5}},
    { 3,{1,2}}
  };
  data.add(in);
  auto foo = data.dimension([](auto d) { return d.foo;});
  auto bar = data.dimension([](auto d) { return d.foo;});
    auto val = data.iterable_dimension([](auto d) { return d.val;});
    auto groupSumLength = val.feature(
        [](auto p, auto v, bool n) {
          if(n)
            p += v.val.size();
          return p;
        },
        [](auto p, auto v, bool n) {
          if(n)
            p -= v.val.size();
          return p;
        },
        []() {
          return std::size_t(0);
        }
                                    );
    auto groupSumEach = val.feature_sum([](auto d) { return d.val.size();});
    data.add(std::vector<RecIterable2>{
        { 1,{5,6,7}}
      });

BOOST_TEST(groupSumLength.all() == groupSumEach.all(), boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(on_adding_data_when_a_filter_is_in_place) {
  cross::filter<RecIterable2> data;
  RecIterable2 in[] = {
    { 1,{1,2}},
    { 2,{1,2}},
    { 3,{3,4,5}},
    { 3,{1,2}}
  };
  data.add(in);
  auto foo = data.dimension([](auto d) { return d.foo;});
  auto bar = data.dimension([](auto d) { return d.foo;});
  auto val = data.iterable_dimension([](auto d) { return d.val;});
  auto groupSumLength = val.feature(
      [](auto p, auto v, bool n) {
        if(n)
          p += v.val.size();
        return p;
      },
      [](auto p, auto v, bool n) {
        if(n)
          p -= v.val.size();
        return p;
      },
      []() {
        return std::size_t(0);
      }
                                  );
  auto groupSumEach = val.feature_sum([](auto d) { return d.val.size();});
  data.add(std::vector<RecIterable2>{
      { 1,{5,6,7}}
    });
  foo.filter_range(1,3);
  data.add(std::vector<RecIterable2>{
      { 3,{6}}
    });
  BOOST_TEST(groupSumLength.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,6},
        {2,6},
        {3,3},
        {4,3},
        {5,6},
        {6,4},
        {7,3}
      } ), boost::test_tools::per_element());
  BOOST_TEST(groupSumEach.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,4},
        {2,4},
        {3,0},
        {4,0},
        {5,3},
        {6,3},
        {7,3}
      } ), boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(on_removoing_data_after_group_creation) {
  cross::filter<RecIterable2> data;
  RecIterable2 in[] = {
    { 1,{1,2}},
    { 2,{1,2}},
    { 3,{3,4,5}},
    { 3,{1,2}}
  };
  data.add(in);
  auto foo = data.dimension([](auto d) { return d.foo;});
  auto bar = data.dimension([](auto d) { return d.foo;});
  auto val = data.iterable_dimension([](auto d) { return d.val;});
  auto groupSumLength = val.feature(
      [](auto p, auto v, bool n) {
        if(n)
          p += v.val.size();
        return p;
      },
      [](auto p, auto v, bool n) {
        if(n)
          p -= v.val.size();
        return p;
      },
      []() {
        return std::size_t(0);
      }
                                  );
  auto groupSumEach = val.feature_sum([](auto d) { return d.val.size();});
  data.add(std::vector<RecIterable2>{
      { 1,{5,6,7}}
    });
  data.add(std::vector<RecIterable2>{
      { 3,{6}}
    });
  val.filter(2);
  data.remove();
  auto t1 = groupSumLength.all();
  
  BOOST_TEST(t1 == (std::vector<std::pair<int,std::size_t>>{
        {3,3},
        {4,3},
        {5,6},
        {6,4},
        {7,3}
      } ), boost::test_tools::per_element());
  BOOST_TEST(groupSumEach.all() == (std::vector<std::pair<int,std::size_t>>{
        {3,3},
        {4,3},
        {5,6},
        {6,4},
        {7,3}
      } ), boost::test_tools::per_element());
  val.filter_all();
  BOOST_TEST(groupSumLength.all() == groupSumEach.all(), boost::test_tools::per_element());
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterablesDimension_top)
BOOST_AUTO_TEST_CASE(returns_the_top_k_groups_by_reduce_value_in_descending_order) {
  Fixture data;
  auto all = data.tags.feature_count();
  BOOST_TEST(all.top(3) == (std::vector<std::pair<int,std::size_t>>{
      {2,33 },
      {3,28 },
      {4,23 },
          }), boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(observes_the_specified_order) {
  Fixture data;
  auto all = data.tags.feature_count();
  all.order([](auto d) { return  -d;});
  
  BOOST_TEST(all.top(3) == (std::vector<std::pair<int,std::size_t>>{
        {0,1 },
        {-1,1 },
        {5,15 },
            }), boost::test_tools::per_element());
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterableDimension_order)
BOOST_AUTO_TEST_CASE(is_useful_in_conjuction_with_a_compound_reduce_value) {
  Fixture data;
  auto all = data.tags.feature(
      [](auto p, auto v, bool) { ++p.count; p.total += v.total; return p;},
      [](auto p, auto v, bool) { --p.count; p.total -= v.total; return p;},
      []() {return CompoundReduce{0,0};});
  
  all.order([](auto r){return r.total;});
  auto t = all.top(1);
  BOOST_TEST(t == (std::vector<std::pair<int,CompoundReduce>>{
        {
          2, {33,5241 }
        }
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(works_for_empty_arrays_in_middle_or_end) {
  cross::filter<Tags> data;
  data.add(std::vector<Tags>{
      {{1,2,3} },
      {{} },
      {{1,2,3} },
      {{3} },
      {{} }
    });
  auto dimension = data.iterable_dimension([](auto d) { return d.tags;});
  auto group = dimension.feature_count([](auto d) { return d;});
  
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterableDimension_dispose)
BOOST_AUTO_TEST_CASE(detaches_from_reduce_listener) {
  cross::filter<Tags> data;
  data.add(std::vector<Tags>{
      {{1,2,3} },
      {{1,2,3} },
      {{3} }
    });
  bool callback = false;
  auto dimension = data.iterable_dimension([](auto r) { return r.tags;});
  auto other = data.iterable_dimension([](auto r) { return r.tags;});
  
  auto group = dimension.feature(
      [&callback](int&p , const Tags&, bool) {callback = true; return p;},
      [&callback](int&p, const Tags&, bool) {callback = true; return p;},
      []() {return 0; }
                               );
  group.all();
  callback = false;
  group.dispose();
  other.filter_range(1,2);
  BOOST_TEST(callback == false);
  

}
BOOST_AUTO_TEST_CASE(detaches_from_add_listener) {
  cross::filter<Tags> data;
  data.add(std::vector<Tags>{
      {{1,2,3} },
      {{1,2,3} },
      {{3} }
    });
  bool callback = false;
  auto dimension = data.iterable_dimension([](auto r) { return r.tags;});
  auto other = data.iterable_dimension([](auto r) { return r.tags;});
  
  auto group = dimension.feature(
      [&callback](int&p , const Tags&, bool) {callback = true; return p;},
      [&callback](int&p, const Tags&, bool) {callback = true; return p;},
      []() {return 0; }
                               );
  group.all();
  callback = false;
  group.dispose();
  data.add(std::vector<Tags>{
      {{3} },
      {{4,5} },
      {{4,5,6} }
    });
  BOOST_TEST(callback == false);
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterableDimension_Add)
BOOST_AUTO_TEST_CASE(top) {
  cross::filter<RecIterable3> data;
  RecIterable3 firstSet[] = {
    {"alpha",1,{1,2}},
    {"bravo",2,{1}},
    {"charlie",1,{}}
  };
  RecIterable3 secondSet[] = {
    {"delta",0,{2}},
    {"echo",3,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(firstSet);
  data.add(secondSet);
  auto top = tags.top(7);
  BOOST_CHECK_EQUAL(top.size(),(unsigned long)6);
  auto m1 = std::max_element(top[0].tags.begin(), top[0].tags.end());
  BOOST_TEST((m1 != top[0].tags.end()));
  BOOST_CHECK_EQUAL(*m1,2);
  auto m2 = std::max_element(top[1].tags.begin(), top[1].tags.end());
  BOOST_TEST((m2 != top[1].tags.end()));
  BOOST_CHECK_EQUAL(*m2,2);
  auto m3 = std::min_element(top[2].tags.begin(), top[2].tags.end());
  BOOST_TEST((m3 != top[2].tags.end()));
  BOOST_CHECK_EQUAL(*m3,1);
  auto m4 = std::min_element(top[3].tags.begin(), top[3].tags.end());
  BOOST_TEST((m4 != top[3].tags.end()));
  BOOST_CHECK_EQUAL(*m4,1);
  BOOST_TEST(top[4].tags.empty());
  BOOST_TEST(top[5].tags.empty());

}
BOOST_AUTO_TEST_CASE(observes_the_associated_dimension_filters) {
  cross::filter<RecIterable3> data;
  RecIterable3 firstSet[] = {
    {"alpha",1,{1,2}},
    {"bravo",2,{1}},
    {"charlie",1,{}}
  };
  RecIterable3 secondSet[] = {
    {"delta",0,{2}},
    {"echo",3,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(firstSet);
  data.add(secondSet);

  tags.filter_exact(2);
  auto top = tags.top(3);
  BOOST_CHECK_EQUAL(top.size(),(unsigned long)2);
  BOOST_TEST(indexOf(top[0].tags,2));
  BOOST_TEST(indexOf(top[1].tags,2));
  
}
BOOST_AUTO_TEST_CASE(observes_other_dimensions_filters) {
  cross::filter<RecIterable3> data;
  RecIterable3 firstSet[] = {
    {"alpha",1,{1,2}},
    {"bravo",2,{1}},
    {"charlie",1,{}}
  };
  RecIterable3 secondSet[] = {
    {"delta",0,{2}},
    {"echo",3,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(firstSet);
  data.add(secondSet);

  quantity.filter_exact(1);
  auto top = tags.top(4);
  BOOST_CHECK_EQUAL(top.size(),(unsigned long)3);
  BOOST_CHECK_EQUAL(top[0].name, "alpha");
  BOOST_CHECK_EQUAL(top[1].name, "alpha");
  BOOST_CHECK_EQUAL(top[2].name, "charlie");
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE(bottom)
BOOST_AUTO_TEST_CASE(observes_other_dimensions_filters) {
  cross::filter<RecIterable3> data;
  RecIterable3 firstSet[] = {
    {"alpha",1,{1,2}},
    {"bravo",2,{1}},
    {"charlie",1,{}}
  };
  RecIterable3 secondSet[] = {
    {"delta",0,{2}},
    {"echo",3,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(firstSet);
  data.add(secondSet);
  auto bottom = tags.bottom(7);
  BOOST_CHECK_EQUAL(bottom.size(),(unsigned long)6);
  BOOST_CHECK_EQUAL(bottom[0].tags.size(),(unsigned long)0);
  BOOST_CHECK_EQUAL(bottom[1].tags.size(),(unsigned long)0);
  auto m1 = std::min_element(bottom[2].tags.begin(), bottom[2].tags.end());
  BOOST_TEST((m1 != bottom[2].tags.end()));
  BOOST_CHECK_EQUAL(*m1,1);
  auto m2 = std::min_element(bottom[3].tags.begin(), bottom[3].tags.end());
  BOOST_TEST((m2 != bottom[3].tags.end()));
  BOOST_CHECK_EQUAL(*m2,1);
  auto m3 = std::max_element(bottom[4].tags.begin(), bottom[4].tags.end());
  BOOST_TEST((m3 != bottom[4].tags.end()));
  BOOST_CHECK_EQUAL(*m3,2);
  auto m4 = std::max_element(bottom[5].tags.begin(), bottom[5].tags.end());
  BOOST_TEST((m4 != bottom[5].tags.end()));
  BOOST_CHECK_EQUAL(*m4,2);
}
  BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(force_order_when_adding)
BOOST_AUTO_TEST_CASE(others_observe_associated_dimension_filters) {
  cross::filter<RecIterable3> data;
  RecIterable3 firstSet[] = {
    {"alpha",1,{1,2}},
    {"bravo",2,{}}
  };
  RecIterable3 secondSet[] = {
    {"charlie",0,{3,4}},
    {"delta",0,{2,3}},
    {"echo",3,{4,5}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(firstSet);
  data.add(secondSet);

  tags.filter_function([](auto d) { return d == 1;});
  auto top = quantity.top(2);
  BOOST_CHECK_EQUAL(top.size(),(unsigned long)1);
  BOOST_CHECK_EQUAL(indexOf(top[0].tags,1),true);
}
BOOST_AUTO_TEST_CASE(observe_other_dimensions_filters) {
  cross::filter<RecIterable3> data;
  RecIterable3 firstSet[] = {
    {"alpha",1,{1,2}},
    {"bravo",2,{}}
  };
  RecIterable3 secondSet[] = {
    {"charlie",0,{3,4}},
    {"delta",0,{2,3}},
    {"echo",3,{4,5}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(firstSet);
  data.add(secondSet);

  quantity.filter_function([](auto d) { return d == 1;});
  auto top = tags.top(3);
  BOOST_CHECK_EQUAL(top.size(),(unsigned long)2);
  BOOST_CHECK_EQUAL(top[0].name, "alpha");
  BOOST_CHECK_EQUAL(top[1].name, "alpha");
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterable_add_group)
BOOST_AUTO_TEST_CASE(records_added_correctly) {
  cross::filter<RecIterable3> data;
  RecIterable3 firstSet[] = {
    {"alpha",1,{1,3}},
    {"bravo",0,{1,3}}
  };
  RecIterable3 secondSet[] = {
    {"charlie",0,{2}},
    {"delta",2,{2,3}},
    {"echo",2,{4}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  
  data.add(firstSet);
  data.add(secondSet);
  auto top = tagGroup.top(5);
  BOOST_CHECK_EQUAL(top.size(),(unsigned long)4);
  BOOST_CHECK_EQUAL(top[0].second, (unsigned long)3);
  BOOST_CHECK_EQUAL(top[1].second, (unsigned long)2);
  BOOST_CHECK_EQUAL(top[2].second, (unsigned long)2);
  BOOST_CHECK_EQUAL(top[3].second, (unsigned long)1);
  BOOST_CHECK_EQUAL(top[0].first, 3);
  BOOST_CHECK_EQUAL(top[3].first, 4);
}
BOOST_AUTO_TEST_CASE(observes_other_dimension_filters) {
  cross::filter<RecIterable3> data;
  RecIterable3 firstSet[] = {
    {"alpha",1,{1,3}},
    {"bravo",0,{1,3}}
  };
  RecIterable3 secondSet[] = {
    {"charlie",0,{2}},
    {"delta",2,{2,3}},
    {"echo",2,{4}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  
  data.add(firstSet);
  data.add(secondSet);
  quantity.filter_function([](auto d) { return d == 0;});
  
  auto top = tagGroup.top(5);
  BOOST_CHECK_EQUAL(top.size(),(unsigned long)4);
  BOOST_CHECK_EQUAL(top[0].second, (unsigned long)1);
  BOOST_CHECK_EQUAL(top[1].second, (unsigned long)1);
  BOOST_CHECK_EQUAL(top[2].second, (unsigned long)1);
  BOOST_CHECK_EQUAL(top[3].second, (unsigned long)0);
  BOOST_CHECK_EQUAL(top[3].first, 4);
}

BOOST_AUTO_TEST_CASE(one_tag_with_one_empty) {
  cross::filter<RecIterable3> data;
  RecIterable3 firstSet[] = {
    {"alpha",1,{1}},
    {"bravo",1,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  
  data.add(firstSet);
  BOOST_TEST(tagGroup.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,1
        }
      }),boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(one_tag_then_add_empty) {
  cross::filter<RecIterable3> data;
  RecIterable3 firstSet[] = {
    {"alpha",1,{1}}
  };
  RecIterable3 secondSet[] = {
    {"bravo",1,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  
  data.add(firstSet);
  data.add(secondSet);
  BOOST_TEST(tagGroup.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,1
              }
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(empty_tag_then_add_one_tag) {
  cross::filter<RecIterable3> data;
  RecIterable3 firstSet[] = {
    {"alpha",2,{}}
  };
  RecIterable3 secondSet[] = {
    {"bravo",1,{1}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  
  data.add(firstSet);
  data.add(secondSet);
  BOOST_TEST(tagGroup.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,1
              }
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(one_tag_then_add_one_more_tag) {
  cross::filter<RecIterable3> data;
  RecIterable3 firstSet[] = {
    {"alpha",2,{1}}
  };
  RecIterable3 secondSet[] = {
    {"bravo",1,{2}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  
  data.add(firstSet);
  data.add(secondSet);
  BOOST_TEST(tagGroup.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,1},
        {2,1}
      }),boost::test_tools::per_element());
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterable_remove_dimension)
BOOST_AUTO_TEST_CASE(other_dimension_filtered_remove) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",1,{1,3}},
    {"bravo",0,{1,3}},
    {"charlie",3,{2}},
    {"delta",3,{2,3}},
    {"echo",2,{4}},
    {"foxtrot",1,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(set);
  quantity.filter_exact(3);
  data.remove();
  quantity.filter_all();
  auto top = tags.top(7);

  BOOST_CHECK_EQUAL(top.size(),(unsigned long)6);


  CHECK_MAX_ELEM(top[0].tags,4);
  CHECK_MAX_ELEM(top[1].tags,3);
  CHECK_MIN_ELEM(top[1].tags,1);
  CHECK_MAX_ELEM(top[2].tags,3);
  CHECK_MIN_ELEM(top[2].tags,1);
  CHECK_MAX_ELEM(top[3].tags,3);
  CHECK_MIN_ELEM(top[3].tags,1);
  CHECK_MAX_ELEM(top[4].tags,3);
  CHECK_MIN_ELEM(top[4].tags,1);
  BOOST_CHECK_EQUAL(top[5].tags.size(),(unsigned long)0);
}
BOOST_AUTO_TEST_CASE(self_filter_exact_remove) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",1,{1,3}},
    {"bravo",0,{1,3}},
    {"charlie",3,{2}},
    {"delta",3,{2,3}},
    {"echo",2,{4}},
    {"foxtrot",1,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(set);
  tags.filter_exact(2);
  data.remove();
  tags.filter_all();
  auto top = tags.top(7);

  BOOST_CHECK_EQUAL(top.size(),(unsigned long)6);


  CHECK_MAX_ELEM(top[0].tags,4);
  CHECK_MAX_ELEM(top[1].tags,3);
  CHECK_MIN_ELEM(top[1].tags,1);
  CHECK_MAX_ELEM(top[2].tags,3);
  CHECK_MIN_ELEM(top[2].tags,1);
  CHECK_MAX_ELEM(top[3].tags,3);
  CHECK_MIN_ELEM(top[3].tags,1);
  CHECK_MAX_ELEM(top[4].tags,3);
  CHECK_MIN_ELEM(top[4].tags,1);
  BOOST_CHECK_EQUAL(top[5].tags.size(),(unsigned long)0);
}
BOOST_AUTO_TEST_CASE(self_filter_function_remove) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",1,{1,3}},
    {"bravo",0,{1,3}},
    {"charlie",3,{2}},
    {"delta",3,{2,3}},
    {"echo",2,{4}},
    {"foxtrot",1,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(set);
  tags.filter_function([](auto d) { return d == 2;});
  data.remove();
  tags.filter_all();
  auto top = tags.top(7);

  BOOST_CHECK_EQUAL(top.size(),(unsigned long)6);


  CHECK_MAX_ELEM(top[0].tags,4);
  CHECK_MAX_ELEM(top[1].tags,3);
  CHECK_MIN_ELEM(top[1].tags,1);
  CHECK_MAX_ELEM(top[2].tags,3);
  CHECK_MIN_ELEM(top[2].tags,1);
  CHECK_MAX_ELEM(top[3].tags,3);
  CHECK_MIN_ELEM(top[3].tags,1);
  CHECK_MAX_ELEM(top[4].tags,3);
  CHECK_MIN_ELEM(top[4].tags,1);
  BOOST_CHECK_EQUAL(top[5].tags.size(),(unsigned long)0);
}

BOOST_AUTO_TEST_CASE(other_dimension_filtered_than_self_filter_function_remove) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",1,{1,3}},
    {"bravo",0,{1,3}},
    {"charlie",3,{2}},
    {"delta",3,{2,3}},
    {"echo",2,{4}},
    {"foxtrot",1,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(set);
  quantity.filter_exact(3);
  data.remove();
  quantity.filter_all();
  tags.filter_function([](auto d) { return d == 1;});
  data.remove();
  tags.filter_all();
  auto top = tags.top(3);
  BOOST_TEST(top == (std::vector<RecIterable3>{
        {"echo",2,{4}},
        {"foxtrot",1,{}}
      }));
}
BOOST_AUTO_TEST_CASE(remove_then_add) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",1,{1,3}},
    {"bravo",0,{1,3}},
    {"charlie",3,{2}},
    {"delta",3,{2,3}},
    {"echo",2,{4}},
    {"foxtrot",1,{}}
  };
  RecIterable3 set2[] = {
    {"golf",3,{1}}
  };
  

  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(set);
  quantity.filter_exact(3);
  data.remove();
  quantity.filter_all();
  tags.filter_function([](auto d) { return d == 1;});
  data.remove();
  tags.filter_all();
  data.add(set2);
  
  auto top = tags.top(3);
  BOOST_TEST(top == (std::vector<RecIterable3>{
        {"echo",2,{4}},
        {"golf",3,{1}},
        {"foxtrot",1,{}}
      }));
}

BOOST_AUTO_TEST_CASE(filter_then_remove_empty_tag_to_only_one_tag) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",2,{1}},
    {"bravo",1,{}}
  };

  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(set);
  quantity.filter_exact(1);
  data.remove();
  quantity.filter_all();
  auto top = tags.top(3);
  BOOST_TEST(top == (std::vector<RecIterable3>{
        {"alpha",2,{1}}
      }));
}
BOOST_AUTO_TEST_CASE(filter_remove_one_tag_to_only_empty_tag) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",2,{1}},
    {"bravo",1,{}}
  };

  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(set);
  quantity.filter_exact(2);
  data.remove();
  quantity.filter_all();
  auto top = tags.top(3);
  BOOST_TEST(top == (std::vector<RecIterable3>{
        {"bravo",1,{}}
      }));
}
BOOST_AUTO_TEST_CASE(remove_multiple_tag_add_single_tag_others_observer_filter) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",2,{1}},
    {"bravo",1,{2,3}}
  };
  RecIterable3 set2[] = {
    {"charlie",3,{4}}
  };

  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  data.add(set);
  quantity.filter_exact(1);
  data.remove();
  quantity.filter_all();
  data.add(set2);
  tags.filter_exact(1);
  
  auto top = tags.top(3);
  BOOST_TEST(top == (std::vector<RecIterable3>{
        {"alpha",2,{1}}
      }));
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(iterable_remove_group)
BOOST_AUTO_TEST_CASE(other_dimension_filtered_remove) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",1,{1,3}},
    {"bravo",0,{1,3}},
    {"charlie",3,{2}},
    {"delta",3,{2,3}},
    {"echo",2,{4}},
    {"foxtrot",1,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  
  data.add(set);
  quantity.filter_exact(3);
  auto top2 = tagGroup.top(5);
  data.remove();
  quantity.filter_all();
  BOOST_TEST(tagGroup.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,2},
        {3,2},
        {4,1}
      }),boost::test_tools::per_element());


}
BOOST_AUTO_TEST_CASE(self_FilterExact_remove) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",1,{1,3}},
    {"bravo",0,{1,3}},
    {"charlie",3,{2}},
    {"delta",3,{2,3}},
    {"echo",2,{4}},
    {"foxtrot",1,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  
  data.add(set);
  tags.filter_exact(2);
  data.remove();
  tags.filter_all();
  BOOST_TEST(tagGroup.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,2},
        {3,2},
        {4,1}
      }),boost::test_tools::per_element());


}
BOOST_AUTO_TEST_CASE(self_filter_function_remove) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",1,{1,3}},
    {"bravo",0,{1,3}},
    {"charlie",3,{2}},
    {"delta",3,{2,3}},
    {"echo",2,{4}},
    {"foxtrot",1,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  
  data.add(set);
  tags.filter_function([](auto d) {return d == 2;});
  data.remove();
  tags.filter_all();
  BOOST_TEST(tagGroup.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,2},
        {3,2},
        {4,1}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(other_dimension_filtered_then_self_filter_function_remove) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",1,{1,3}},
    {"bravo",0,{1,3}},
    {"charlie",3,{2}},
    {"delta",3,{2,3}},
    {"echo",2,{4}},
    {"foxtrot",1,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  
  data.add(set);
  quantity.filter_exact(3);
  data.remove();
  quantity.filter_all();
  tags.filter_function([](auto d) {return d == 1;});
  data.remove();
  tags.filter_all();
  BOOST_TEST(tagGroup.all() == (std::vector<std::pair<int,std::size_t>>{
        {4,1}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(filter_then_remove_to_one_tag_with_one_empty) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",2,{1}},
    {"bravo",1,{}},
    {"charlie",0,{2}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  
  data.add(set);
  quantity.filter_exact(0);
  data.remove();
  quantity.filter_all();
  BOOST_TEST(tagGroup.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,1}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(filter_then_remove_empty_tag_to_only_one_tag) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",2,{1}},
    {"bravo",1,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  
  data.add(set);
  quantity.filter_exact(1);
  data.remove();
  quantity.filter_all();
  BOOST_TEST(tagGroup.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,1}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(filter_then_remove_one_tag_to_only_empty_tag) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",2,{1}},
    {"bravo",1,{}}
  };
  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();
  data.add(set);
  quantity.filter_exact(2);
  data.remove();
  quantity.filter_all();
  BOOST_TEST(tagGroup.all().empty());

}
BOOST_AUTO_TEST_CASE(remove_then_add_one_tag_back) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",2,{1}},
    {"bravo",1,{}}
  };
  RecIterable3 set2[] = {
    {"alpha",2,{1}}
  };

  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();

  data.add(set);
  quantity.filter_exact(2);
  data.remove();
  quantity.filter_all();
  data.add(set2);

  BOOST_TEST(tagGroup.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,1}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(remove_then_add_empty_tag_back) {
  cross::filter<RecIterable3> data;
  RecIterable3 set[] = {
    {"alpha",2,{1}},
    {"bravo",1,{}}
  };
  RecIterable3 set2[] = {
    {"bravo",1,{}}
  };

  auto tags = data.iterable_dimension([](auto r) { return r.tags;});
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto tagGroup = tags.feature_count();

  data.add(set);
  quantity.filter_exact(1);
  data.remove();
  quantity.filter_all();
  data.add(set2);

  BOOST_TEST(tagGroup.all() == (std::vector<std::pair<int,std::size_t>>{
        {1,1}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_SUITE_END();
