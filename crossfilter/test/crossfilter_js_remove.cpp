// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE CrossfilterJSRemove
#include "utils.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <utility>
#include <algorithm>
#include <cmath>
#include "crossfilter.hpp"


auto Infinity = std::numeric_limits<int>::max();
struct  Rec3 {
  double foo;
  bool operator == (const Rec3 & lhs) const {
    return foo == lhs.foo;
  }
};

struct RecIterable {
  std::vector<int> foo;
  int bar;
};


BOOST_TEST_DONT_PRINT_LOG_VALUE(Rec3)
BOOST_TEST_DONT_PRINT_LOG_VALUE(RecIterable)

BOOST_AUTO_TEST_SUITE(crossfilter_remove)
BOOST_AUTO_TEST_CASE(removing_record_works_for_a_group_with_cardinality_one) {
  
  cross::filter<Rec3> data;
  auto foo = data.dimension([](auto d) { return d.foo; });
  auto div2 = foo.feature_count([](auto value) { return std::floor(value / 2); });
  auto positive = foo.feature_count([](auto value) { return value > 0; });

  data.add(std::vector<Rec3>{{1}, {1.1}, {1.2}});
  foo.filter(1.1);
  data.remove();
  foo.filter_all();
  data.remove();
  BOOST_TEST(foo.top(Infinity).empty());
}
BOOST_AUTO_TEST_CASE(removing_record_works_for_another_group_with_cardinality_one) {

  cross::filter<Rec3> data;
  auto foo = data.dimension([](auto d) { return d.foo; });
  auto div2 = foo.feature_count([](auto value) { return std::floor(value / 2); });
  auto positive = foo.feature_count([](auto value) { return value > 0; });

  data.add(std::vector<Rec3>{{0}, {-1}});

  BOOST_TEST(positive.all() == (std::vector<std::pair<bool,std::size_t>>{{false, 2}}), boost::test_tools::per_element());
  foo.filter(0);
  data.remove();
  BOOST_TEST(positive.all() == (std::vector<std::pair<bool,std::size_t>>{{false, 1}}), boost::test_tools::per_element());
  foo.filter_all();
  auto top = foo.top(Infinity);
  BOOST_TEST(top.size() == (unsigned long)1);
  if(!top.empty()) {
    BOOST_TEST(top[0].foo == -1);
  }
  //  BOOST_TEST(foo.top(Infinity) == (std::vector<Rec3>{{-1}}), boost::test_tools::per_element());
  data.remove();
  BOOST_TEST(foo.top(Infinity).empty());
}
BOOST_AUTO_TEST_CASE(removing_record_updates_dimension) {
  cross::filter<Rec3> data;
  auto foo = data.dimension([](auto d) { return d.foo; });
  auto div2 = foo.feature_count([](auto value) { return std::floor(value / 2); });
  auto positive = foo.feature_count([](auto value) { return value > 0; });

  data.add(std::vector<Rec3>{{1}, {2}});

  foo.filter_exact(1);
  data.remove();
  foo.filter_all();
  auto top1 = foo.top(Infinity);
  BOOST_TEST(top1.size() == (unsigned long)1);
  if(!top1.empty()) {
    BOOST_TEST(top1[0].foo == 2);
  }
  
  data.remove();
  BOOST_TEST(foo.top(Infinity).empty());
}
BOOST_AUTO_TEST_CASE(removing_records_updates_group) {
  cross::filter<Rec3> data;
  auto foo = data.dimension([](auto d) { return d.foo; });
  auto div2 = foo.feature_count([](auto value) { return std::floor(value / 2); });
  auto positive = foo.feature_count([](auto value) { return value > 0; });

  data.add(std::vector<Rec3>{{1}, {2}, {3}});

  BOOST_TEST(foo.top(Infinity) == (std::vector<Rec3>{
        {3 }, {2}, {1}
      }), boost::test_tools::per_element());

  BOOST_TEST(div2.all() == (std::vector<std::pair<double,std::size_t>>{{0, 1},{1,2}}), boost::test_tools::per_element());

  foo.filter_range(1, 3);
  data.remove();
  foo.filter_all();

  BOOST_TEST(foo.top(Infinity) == (std::vector<Rec3>{
        {3 }
      }), boost::test_tools::per_element());
  BOOST_TEST(div2.all() == (std::vector<std::pair<double,std::size_t>>{{1,1}}), boost::test_tools::per_element());
  data.remove();
  BOOST_TEST(foo.top(Infinity).empty());
  BOOST_TEST(div2.all().empty());
}

BOOST_AUTO_TEST_CASE(filtering_works_correctly_after_removing_record) {
  cross::filter<Rec3> data;
  auto foo = data.dimension([](auto d) { return d.foo; });
  auto div2 = foo.feature_count([](auto value) { return std::floor(value / 2); });
  auto positive = foo.feature_count([](auto value) { return value > 0; });

  data.add(std::vector<Rec3>{{1}, {2}, {3}});
  foo.filter(2);
  data.remove();
  foo.filter_all();
  BOOST_TEST(foo.top(Infinity) == (std::vector<Rec3>{
        {3}, {1}
      }), boost::test_tools::per_element());

  data.remove();
  BOOST_TEST(foo.top(Infinity).empty());
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(remove_with_predicate)
BOOST_AUTO_TEST_CASE(removing_record_workd_with_group_cardinality_one) {
  cross::filter<Rec3> data;
  auto foo = data.dimension([](auto d) { return d.foo; });
  auto div2 = foo.feature_count([](auto value) { return std::floor(value / 2); });
  auto positive = foo.feature_count([](auto value) { return value > 0; });


  data.add(std::vector<Rec3>{{1}, {1.1}, {1.2}});
  data.remove([] (auto d, int) {
      return d.foo == 1.1;
    });
  BOOST_TEST(data.all() == (std::vector<Rec3>{
        {1}, {1.2}
      }), boost::test_tools::per_element());

  data.remove([] (auto, int) {
      return true;
    });
  BOOST_TEST(foo.top(Infinity).empty());
}

BOOST_AUTO_TEST_CASE(removing_record_works_for_another_group_with_cardinality_one) {
  cross::filter<Rec3> data;
  auto foo = data.dimension([](auto d) { return d.foo; });
  auto div2 = foo.feature_count([](auto value) { return std::floor(value / 2); });
  auto positive = foo.feature_count([](auto value) { return value > 0; });

  data.add(std::vector<Rec3>{{0}, {-1}});

  BOOST_TEST(positive.all() == (std::vector<std::pair<bool,std::size_t>>{{false, 2}}), boost::test_tools::per_element());

  data.remove([](auto d, int) { return d.foo == 0;});
  BOOST_TEST(positive.all() == (std::vector<std::pair<bool,std::size_t>>{{false, 1}}), boost::test_tools::per_element());
  BOOST_TEST(foo.top(Infinity) == (std::vector<Rec3>{
        {-1}
      }), boost::test_tools::per_element());

  data.remove([](auto, int) {return true;} );
  BOOST_TEST(foo.top(Infinity).empty());
}
BOOST_AUTO_TEST_CASE(removing_record_updates_dimension) {
  cross::filter<Rec3> data;
  auto foo = data.dimension([](auto d) { return d.foo; });
  auto div2 = foo.feature_count([](auto value) { return std::floor(value / 2); });
  auto positive = foo.feature_count([](auto value) { return value > 0; });

  data.add(std::vector<Rec3>{{1}, {2}});

  data.remove([](auto d, int) { return d.foo == 1;});
  BOOST_TEST(foo.top(Infinity) == (std::vector<Rec3>{
        {2}
      }), boost::test_tools::per_element());

  data.remove([](auto, int) { return true;});
  BOOST_TEST(foo.top(Infinity).empty());
}

BOOST_AUTO_TEST_CASE(removing_records_updates_group) {
  cross::filter<Rec3> data;
  auto foo = data.dimension([](auto d) { return d.foo; });
  auto div2 = foo.feature_count([](auto value) { return std::floor(value / 2); });
  auto positive = foo.feature_count([](auto value) { return value > 0; });

  data.add(std::vector<Rec3>{{1}, {2}, {3}});

  BOOST_TEST(foo.top(Infinity) == (std::vector<Rec3>{
        {3 }, {2}, {1}
      }), boost::test_tools::per_element());

  BOOST_TEST(div2.all() == (std::vector<std::pair<double,std::size_t>>{{0, 1},{1,2}}), boost::test_tools::per_element());

  data.remove([](auto d, int) { return d.foo < 3;});


  BOOST_TEST(foo.top(Infinity) == (std::vector<Rec3>{
        {3 }
      }), boost::test_tools::per_element());
  BOOST_TEST(div2.all() == (std::vector<std::pair<double,std::size_t>>{{1,1}}), boost::test_tools::per_element());
  data.remove([](auto,int) {return true;});
  BOOST_TEST(foo.top(Infinity).empty());
  BOOST_TEST(div2.all().empty());
}
BOOST_AUTO_TEST_CASE(can_remove_records_while_filtering) {
  cross::filter<Rec3> data;
  auto foo = data.dimension([](auto d) { return d.foo; });
  auto div2 = foo.feature_count([](auto value) { return std::floor(value / 2); });
  auto positive = foo.feature_count([](auto value) { return value > 0; });
  auto allSum = data.feature_sum([](auto d) { return d.foo; });
  data.add(std::vector<Rec3>{{1}, {2}, {3}});
  std::cout << "a1" << std::endl;
  BOOST_CHECK_EQUAL(allSum.value(),6);
  std::cout << "a2" << std::endl;
  BOOST_TEST(positive.all() == (std::vector<std::pair<bool,std::size_t>>{{true, 3}}), boost::test_tools::per_element());
  std::cout << "a3" << std::endl;
  foo.filter(2);
  std::cout << "a4" << std::endl;
  BOOST_CHECK_EQUAL(allSum.value(),2);
  data.remove([](auto d, int) { return d.foo == 3;});
  BOOST_CHECK_EQUAL(allSum.value(),2);
  BOOST_TEST(positive.all() == (std::vector<std::pair<bool,std::size_t>>{{true, 2}}), boost::test_tools::per_element());
  data.remove([](auto d, int) { return d.foo == 2;});
  BOOST_CHECK_EQUAL(allSum.value(),0);

  foo.filter_all();
  BOOST_CHECK_EQUAL(allSum.value(),1);
  data.remove([](auto,int) { return true;});
  BOOST_TEST(foo.top(Infinity).empty());
}

BOOST_AUTO_TEST_CASE(can_remove_records_while_filtering_on_iterable_dimension) {
  cross::filter<RecIterable> data;

  auto fooDimension = data.iterable_dimension([](auto d) { return d.foo;});
  auto fooGroup = fooDimension.feature_count();
  auto allBarSum = data.feature_sum([](auto d) { return d.bar;});
  auto fooBarSum = fooDimension.feature_sum([](auto d) { return d.bar;});

  data.add(std::vector<RecIterable>{
      {std::vector<int>{1,2,3},1},
      {std::vector<int>{1,2},2},
      {std::vector<int>{2,3},4}
    });
   fooGroup.all();
   allBarSum.value();

   BOOST_CHECK_EQUAL(allBarSum.value(),7);
  BOOST_TEST(fooGroup.all() == (std::vector<std::pair<int,std::size_t>> {
      {  1,  2 },
      {  2,  3 },
      {  3,  2 }
    }), boost::test_tools::per_element());
  BOOST_TEST(fooBarSum.all() == (std::vector<std::pair<int,int>> {
        {  1,  3 },
        {  2,  7 },
        {  3,  5 }
      }), boost::test_tools::per_element());

  fooDimension.filter(3);
  BOOST_CHECK_EQUAL(allBarSum.value(), 5);

  data.remove([] (auto d, int) {
      return std::find(d.foo.begin(), d.foo.end(), 1) != d.foo.end();

    });
  BOOST_CHECK_EQUAL(allBarSum.value(), 4);
  BOOST_TEST(fooGroup.all() == (std::vector<std::pair<int,std::size_t>> {
        {  2,  1 },
        {  3,  1 }
      }), boost::test_tools::per_element());
  BOOST_TEST(fooBarSum.all() == (std::vector<std::pair<int,int>> {
        {  2,  4 },
        {  3,  4 }
      }), boost::test_tools::per_element());

  fooDimension.filter_all();
  BOOST_CHECK_EQUAL(allBarSum.value(), 4);
  data.remove([] (auto, int) {
      return true;
    });
  BOOST_TEST(fooDimension.top(Infinity).empty());

 }
BOOST_AUTO_TEST_SUITE_END();
