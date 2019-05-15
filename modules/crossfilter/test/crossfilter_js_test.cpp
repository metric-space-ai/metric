#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE mtest
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

struct Fixture {
  cross::filter<Record> cr;
  cross::dimension<std::string,Record> date;
  cross::dimension<int,Record> quantity;
  cross::dimension<int,Record> total;
  cross::dimension<int,Record> tip;
  cross::dimension<std::string,Record> type;
  //  dimension<int,cross::filterImpl<Record>,true> tags;
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
      { "2011-11-14T17:29:52Z",  1,  200,  100,  "visa", {-1, 0, 10, 10}},
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
  }
};

// struct Fixture2 : public Fixture {
//   cross::feature<std::size_t,std::size_t,Record,int,true> count;
//   Fixture2()
//       :Fixture(),count(quantity.feature_all_count()) {
//   }
  
// };



BOOST_AUTO_TEST_SUITE(crossfilter)
BOOST_AUTO_TEST_CASE(up_to_32_dimension_support_test) {
  cross::filter<Record> data;
  for(int i = 0;i < 32; i++) data.dimension([](const Record &) { return 0;});
}
BOOST_AUTO_TEST_CASE(add_and_remove_32_diension_repeatedly_test) {
  cross::filter<Record> data;
  using Dim = cross::filter<Record>::dimension_t<int>; 
  
  std::list<Dim> dimensions;
  for (int j = 0; j < 10; j++) {
    for (int i = 0; i < 32; i++) dimensions.push_back(data.dimension([](const Record &) { return 0;}));
    while (!dimensions.empty())  {
      auto & d = dimensions.front();
      d.dispose();
      dimensions.pop_front();
    }
  }
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(empty_data)
BOOST_AUTO_TEST_CASE(feature_all_count_value_test) {
  cross::filter<Record> cf;
  auto g = cf.feature_count();
  BOOST_CHECK_EQUAL(g.value(),(unsigned long)0);
}

BOOST_AUTO_TEST_CASE(feature_all_value_after_removing_all_data) {
  cross::filter<Record> cf;
  auto g = cf.feature_count();
  BOOST_CHECK_EQUAL(g.value(),(unsigned long)0);
  
  cf.add({"",1,100,1,"",{}});

  BOOST_CHECK_EQUAL(g.value(),(unsigned long)1);
  cf.remove();
  BOOST_CHECK_EQUAL(g.value(),(unsigned long)0);
  
}
BOOST_AUTO_TEST_CASE(dimension_groupall_count_default_value) {
  cross::filter<Record> data;
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto count = quantity.feature_all_count();
  BOOST_CHECK_EQUAL(count.value(),(unsigned long)0);
}

BOOST_AUTO_TEST_CASE(dimension_groupall_count_value_after_removing_all_data) {
  cross::filter<Record> data;
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto count = quantity.feature_all_count();
  BOOST_CHECK_EQUAL(count.value(),(unsigned long)0);
  data.add({"",1,100,1,"",{}});
  BOOST_CHECK_EQUAL(count.value(),(unsigned long)1);
  data.remove();
  BOOST_CHECK_EQUAL(count.value(),(unsigned long)0);
}

BOOST_AUTO_TEST_CASE(dimension_groupall_sum) {
  cross::filter<Record> data;
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  auto count = quantity.feature_all_count();
  auto total = quantity.feature_all_sum([](auto r) { return r.total;});
  BOOST_CHECK_EQUAL(total.value(),0);
  data.add({"",1,190,1,"",{}});
  BOOST_CHECK_EQUAL(total.value(),190);
  data.remove();
  BOOST_CHECK_EQUAL(total.value(),0);
}
BOOST_AUTO_TEST_CASE(dimension_groupall_custom_reduce) {
  cross::filter<Record> data;
  auto quantity = data.dimension([](auto r) {return r.quantity;});
  //  auto count = quantity.feature_all_count(();
  //  auto total = quantity.feature_all_sum(([](auto r) { return r.total;});
  auto custom = quantity.feature_all([](auto v, auto, bool ) {return v + 1;},
                                     [](auto v, auto, bool ) {return v - 1;},
                                     [](){ return int(1);}
                                     );
  
  BOOST_CHECK_EQUAL(custom.value(),1);
  data.add({"",2,190,1,"",{}});
  BOOST_CHECK_EQUAL(custom.value(),2);
  //  std::cout <<"remove data" << std::endl;
  
  data.remove();
  BOOST_CHECK_EQUAL(custom.value(),1);
}

BOOST_AUTO_TEST_CASE(dimension_groupall_custom_reduce_information_lifecycle) {
  struct Rec2 {int foo; int val;};
  Rec2 vec[] = {{1,2},{2,2},{3,2},{3,2}};
  cross::filter<Rec2> data;
  data.add(vec);
  auto foo = data.dimension([](auto r) { return r.foo;});
  auto bar = data.dimension([](auto r) { return r.foo;});
  auto val = data.dimension([](auto r) { return r.val;});
  auto groupMax = bar.feature_all([](auto v, auto r, bool n) {if(n) {v+= r.val;} return v;},
                                    [](auto v, auto r, bool n) {if(n) {v-=r.val;} return v;},
                                    [](){ return int(0);});
  auto groupSum = bar.feature_all_sum([](auto r) { return r.val;});

  // on group creation
  BOOST_CHECK_EQUAL(groupMax.value(),groupSum.value());

  // on filtering
  foo.filter_range(1,3);
  BOOST_CHECK_EQUAL(groupMax.value(),8);
  BOOST_CHECK_EQUAL(groupSum.value(),4);
  foo.filter_all();

  // on adding data after group creation
  data.add({1,2});
  BOOST_CHECK_EQUAL(groupMax.value(),10);
  
  BOOST_CHECK_EQUAL(groupMax.value(),groupSum.value());

  // on adding data when a filter is in place
  foo.filter_range(1,3);
  data.add({3,1});
  BOOST_CHECK_EQUAL(groupMax.value(),11);
  BOOST_CHECK_EQUAL(groupSum.value(),6);
  foo.filter_all();

  // on removing data after group creation
  val.filter_exact(1);
  data.remove();
  BOOST_CHECK_EQUAL(groupMax.value(),10);
  BOOST_CHECK_EQUAL(groupSum.value(),0);
  val.filter_all();
  BOOST_CHECK_EQUAL(groupMax.value(),groupSum.value());
}

BOOST_AUTO_TEST_CASE(up_to_64_duimension_support) {
  cross::filter<Record> cf;
  cross::filter<Record> data;
  for(int i = 0;i < 64; i++) data.dimension([](const Record &) { return 0;});
}
BOOST_AUTO_TEST_CASE(add_and_remove_32_diension_repeatedly_test) {
  cross::filter<Record> data;
  using Dim = cross::filter<Record>::dimension_t<int>; 
  
  std::list<Dim> dimensions;
  for (int j = 0; j < 10; j++) {
    for (int i = 0; i < 64; i++) dimensions.push_back(data.dimension([](const Record &) { return 0;}));
    while (!dimensions.empty())  {
      auto & d = dimensions.front();
      d.dispose();
      dimensions.pop_front();
    }
  }
}
  

BOOST_AUTO_TEST_CASE(filtering_with_more_than_32_dimensions) {
  struct Rec2 {
    int value;
    };
  cross::filter<Rec2> data;
  using Dim = cross::filter<Rec2>::dimension_t<bool>; 
  std::vector<Dim> dims;
  for(int i = 0; i < 50; i++) {
    data.add({i});
  }
  
  //  auto d = data.dimension([]( auto val)-> bool { return val.value == 1;});
  
  for (int i = 0; i < 50; i++) dims.push_back(data.dimension([i]( auto val)-> bool { return val.value == i;}));


  for(int i = 0; i < 50; i++) {
    dims[i].filter_exact(true);
    data.remove();
    dims[i].filter_all();
    BOOST_CHECK_EQUAL(data.size(), (unsigned long)(49-i));
  }
}

BOOST_AUTO_TEST_CASE(dimension_top_test) {
  Fixture data;
  // returns the top k records by value in descending order
  {
    std::vector<Record> test = {
      { "2011-11-14T16:28:54Z",  1,  300,  200,  "visa", {2,4,5}},
      { "2011-11-14T20:49:07Z",  2,  290,  200,  "tab", {2,4,5}},
      { "2011-11-14T21:18:48Z",  4,  270,  0,  "tab", {1,2,3}}
    };
    
    auto top1 = data.total.top(3);
    BOOST_CHECK_EQUAL(top1,test);
  }
  {
    std::vector<Record> test = {
      { "2011-11-14T23:28:54Z",  2,  190,  100,  "tab", {1,2,3}},
      { "2011-11-14T23:23:29Z",  2,  190,  100,  "tab", {2,3,4}},
      { "2011-11-14T23:21:22Z",  2,  190,  100,  "tab", {2,4,5}}
    };
    auto top = data.date.top(3);
    BOOST_CHECK_EQUAL(top,test);
  }

  // returns the top k records, using offset, by value, in descending order

  {
    std::vector<Record> test = {
      { "2011-11-14T20:49:07Z",  2,  290,  200,  "tab", {2,4,5}},
      { "2011-11-14T21:18:48Z",  4,  270,  0,  "tab", {1,2,3}},
      { "2011-11-14T17:38:40Z",  2,  200,  100,  "visa", {2,4,5}}
    };
    BOOST_CHECK_EQUAL(data.total.top(3,1),test);
  }
  {
    std::vector<Record> test = {
      { "2011-11-14T22:30:22Z",  2,  89,  0,  "tab", {1,3}},
      { "2011-11-14T21:31:05Z",  2,  90,  0,  "tab", {1,2,3}},
      { "2011-11-14T21:30:55Z",  2,  190,  100,  "tab", {2,3,4}}
    };
    BOOST_CHECK_EQUAL(data.date.top(3,10), test);
  }

  //  "observes the associated dimension's filters": function(data) {
  {
    data.quantity.filter_exact(4);
    std::vector<Record> test = {
      {"2011-11-14T21:18:48Z",  4, 270, 0,  "tab", {1,2,3 }}
    };
    auto top = data.total.top(3);
    
    BOOST_CHECK_EQUAL(top, test);
    data.quantity.filter_all();
  }
  
  {
    data.date.filter_range("2011-11-14T19:00:00Z","2011-11-14T20:00:00Z");
    std::vector<Record> test = {
      { "2011-11-14T19:30:44Z",  2,  90,  0,  "tab", {1,3}},
      { "2011-11-14T19:04:22Z",  2,  90,  0,  "tab", {1,2,3}},
      { "2011-11-14T19:00:31Z",  2,  190,  100,  "tab", {2,3,4}}
    };
    BOOST_CHECK_EQUAL(data.date.top(10),test);
    
  }

  {
    std::vector<Record> test = {
      { "2011-11-14T19:00:31Z",  2,  190,  100,  "tab", {2,3,4}}
    };
    
    BOOST_CHECK_EQUAL(data.date.top(10, 2), test);
  }
  {
    std::vector<Record> test = {
      { "2011-11-14T19:30:44Z",  2,  90,  0,  "tab", {1,3}},
      { "2011-11-14T19:04:22Z",  2,  90,  0,  "tab", {1,2,3}},
      { "2011-11-14T19:00:31Z",  2,  190,  100,  "tab", {2,3,4}}
    };
    
    data.date.filter_range("2011-11-14T19:00:00Z","2011-11-14T20:00:00Z"); // also comparable
    BOOST_CHECK_EQUAL(data.date.top(10),test);
  }
  {
    std::vector<Record> test = {
      { "2011-11-14T19:00:31Z",  2,  190,  100,  "tab", {2,3,4}}
    };
    
    BOOST_CHECK_EQUAL(data.date.top(10, 2), test);
  }
  
  data.date.filter_all();

  // "observes other dimensions' filters"

  {
    data.type.filter_exact("tab");
    std::vector<Record> test = {
      { "2011-11-14T20:49:07Z",  2,  290,  200,  "tab", {2,4,5}},
      { "2011-11-14T21:18:48Z",  4,  270,  0,  "tab", {1,2,3}}
    };
    auto top = data.total.top(2);
    
    BOOST_CHECK_EQUAL(top, test);
  }
  {
    std::vector<Record> test = {
      { "2011-11-14T21:26:30Z",  2,  190,  100,  "tab", {2,4,5}},
      {"2011-11-14T23:28:54Z",  2,  190,  100,  "tab", {1,2,3}}
    };
    //    std::cout << "top15=" << data.total.top(15) << std::endl;

    BOOST_CHECK_EQUAL(data.total.top(2,8),test);
  }

  {
    std::vector<Record> test = {
      { "2011-11-14T16:28:54Z",  1,  300,  200,  "visa", {2,4,5}}
    };
    data.type.filter_exact("visa");
    BOOST_CHECK_EQUAL(data.total.top(1), test);
  }
  {
    std::vector<Record> test = {
      { "2011-11-14T17:38:40Z",  2,  200,  100,  "visa", {2,4,5}}
    };
    data.quantity.filter_exact(2);
    BOOST_CHECK_EQUAL(data.tip.top(1), test);
  }
  data.type.filter_all();
  data.quantity.filter_all();

  data.type.filter_exact("tab");
  {
    std::vector<Record> test = {{
        { "2011-11-14T23:28:54Z",  2,  190,  100,  "tab", {1,2,3}},
        { "2011-11-14T23:23:29Z",  2,  190,  100,  "tab", {2,3,4}}
      }};
    BOOST_CHECK_EQUAL(data.date.top(2),test);
  }
  {
    std::vector<Record> test ={{
        { "2011-11-14T22:30:22Z",  2,  89,  0,  "tab", {1,3}},
        { "2011-11-14T21:31:05Z",  2,  90,  0,  "tab", {1,2,3}}
      }};
        BOOST_CHECK_EQUAL(data.date.top(2, 8),test);
  }
  
  data.type.filter_exact("visa");
  {
    std::vector<Record> test ={
        { "2011-11-14T23:16:09Z",  1,  200,  100,  "visa", {2,4,5}}
      };

    BOOST_CHECK_EQUAL(data.date.top(1),test);
  }
  
  data.quantity.filter_exact(2);
  {
    std::vector<Record> test = {
      { "2011-11-14T22:58:54Z",  2,  100,  0,  "visa", {2,3,4}}
    };
    BOOST_CHECK_EQUAL(data.date.top(1), test);
  }

  data.type.filter_all();
  data.quantity.filter_all();
  // "negative or zero k returns an empty array": function(data) {
  std::vector<Record> empty ;
  
  BOOST_CHECK_EQUAL(data.quantity.top(0), empty);
  BOOST_CHECK_EQUAL(data.quantity.top(-1),empty);
  BOOST_CHECK_EQUAL(data.quantity.top(std::numeric_limits<int64_t>::min()), empty);
  BOOST_CHECK_EQUAL(data.quantity.top(0, 0), empty);
  BOOST_CHECK_EQUAL(data.quantity.top(-1, -1), empty);
  //  BOOST_CHECK_EQUAL(data.quantity.top(NaN, NaN), []);
  BOOST_CHECK_EQUAL(data.quantity.top(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::min()), empty);
  BOOST_CHECK_EQUAL(data.date.top(0), empty);
  BOOST_CHECK_EQUAL(data.date.top(-1), empty);
  //BOOST_CHECK_EQUAL(data.date.top(NaN), []);
  BOOST_CHECK_EQUAL(data.date.top(std::numeric_limits<int64_t>::min()), empty);
  BOOST_CHECK_EQUAL(data.date.top(0, 0), empty);
  BOOST_CHECK_EQUAL(data.date.top(-1, -1), empty);
  BOOST_CHECK_EQUAL(data.date.top(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::min()), empty);
}

BOOST_AUTO_TEST_CASE(dimension_bottom_test) {
  using V = std::vector<Record>;
  Fixture data;
  
  // returns the bottom k records by value, in descending order
  {
    V test = {
      { "2011-11-14T22:30:22Z",  2,  89,  0,  "tab", {1,3}},
      { "2011-11-14T16:30:43Z",  2,  90,  0,  "tab", {2,3,4}},
      { "2011-11-14T16:48:46Z",  2,  90,  0,  "tab", {1,2,3}}
    };
    BOOST_CHECK_EQUAL(data.total.bottom(3), test);
  }
  
  {
    V test =
        {
          { "2011-11-14T16:17:54Z",  2,  190,  100,  "tab", {1,2,3}},
          { "2011-11-14T16:20:19Z",  2,  190,  100,  "tab", {1,3}},
          { "2011-11-14T16:28:54Z",  1,  300,  200,  "visa", {2,4,5}}
        };
    BOOST_CHECK_EQUAL(data.date.bottom(3), test);
  }

  //  returns the bottom k records, using offset, by value, in descending order
  {
    V test =
        {
          { "2011-11-14T16:30:43Z",  2,  90,  0,  "tab", {2,3,4}},
          { "2011-11-14T16:48:46Z",  2,  90,  0,  "tab", {1,2,3}},
          { "2011-11-14T16:53:41Z",  2,  90,  0,  "tab", { 1,3}}
        };
    BOOST_CHECK_EQUAL(data.total.bottom(3, 1), test);
  }

  {
    V test =
        {
          { "2011-11-14T17:25:45Z",  2,  200,  0,  "cash", {2,4,5}},
          { "2011-11-14T17:29:52Z",  1,  200,  100,  "visa", {-1, 0, 10, 10}},
          { "2011-11-14T17:33:46Z",  2,  190,  100,  "tab", {1,2,3}}
        };
    BOOST_CHECK_EQUAL(data.date.bottom(3, 10), test);
  }

  // observes the associated dimension's filters
  data.quantity.filter_exact(4);
  {
    V test =
        {
          { "2011-11-14T21:18:48Z",  4,  270,  0,  "tab", {1,2,3}}
        };
    auto b = data.total.bottom(3);
    BOOST_CHECK_EQUAL(data.total.bottom(3), test);
  }
  data.quantity.filter_all();

  {
    V t = {
      { "2011-11-14T19:00:31Z",  2,  190,  100,  "tab", {2,3,4}},
      { "2011-11-14T19:04:22Z",  2,  90,  0,  "tab", {1,2,3}},
      { "2011-11-14T19:30:44Z",  2,  90,  0,  "tab", {1,3}}
    };
    data.date.filter_range("2011-11-14T19:00:00Z","2011-11-14T20:00:00Z");
    BOOST_CHECK_EQUAL(data.date.bottom(10),t);
  }
  BOOST_CHECK_EQUAL(data.date.bottom(10, 2), (V {{ "2011-11-14T19:30:44Z",  2,  90,  0,  "tab", {1,3}}}));
  data.date.filter_all();
  
  // observes other dimensions' filters
  data.type.filter_exact("tab");
  BOOST_CHECK_EQUAL(data.total.bottom(2), (V {
              { "2011-11-14T22:30:22Z",  2,  89,  0,  "tab", {1,3}},
              { "2011-11-14T16:30:43Z",  2,  90,  0,  "tab", {2,3,4}}
      }));
  data.type.filter_exact("tab");
  BOOST_CHECK_EQUAL(data.total.bottom(2, 8), (V {
        { "2011-11-14T17:52:02Z",  2,  90,  0,  "tab", {2,3,4}},
        { "2011-11-14T18:45:24Z",  2,  90,  0,  "tab", {2,4,5}}
      }));
  data.type.filter_exact("visa");
  BOOST_CHECK_EQUAL(data.total.bottom(1), (V {
        { "2011-11-14T22:58:54Z",  2,  100,  0,  "visa", {2,3,4}}
      }));
  data.quantity.filter_exact(2);
  BOOST_CHECK_EQUAL(data.tip.bottom(1), (V {
        { "2011-11-14T22:58:54Z",  2,  100,  0,  "visa", {2,3,4}}
      }));
  data.type.filter_all();
  data.quantity.filter_all();

  data.type.filter_exact("tab");
  BOOST_CHECK_EQUAL(data.date.bottom(2), (V {
        { "2011-11-14T16:17:54Z",  2,  190,  100,  "tab", {1,2,3}},
        { "2011-11-14T16:20:19Z",  2,  190,  100,  "tab", {1,3}}
      }));
  BOOST_CHECK_EQUAL(data.date.bottom(2, 8), (V {
        { "2011-11-14T17:33:46Z",  2,  190,  100,  "tab", {1,2,3}},
        { "2011-11-14T17:33:59Z",  2,  90,  0,  "tab", {1,3}}
      }));
  data.type.filter_exact("visa");
  BOOST_CHECK_EQUAL(data.date.bottom(1), (V {
        { "2011-11-14T16:28:54Z",  1,  300,  200,  "visa", {2,4,5}}
      }));
  data.quantity.filter_exact(2);
  BOOST_CHECK_EQUAL(data.date.bottom(1), (V {
        { "2011-11-14T17:38:40Z",  2,  200,  100,  "visa", {2,4,5}}
      }));
  data.type.filter_all();
  data.quantity.filter_all();

  // negative or zero k returns an empty array
  BOOST_CHECK_EQUAL(data.quantity.bottom(0), V());
  BOOST_CHECK_EQUAL(data.quantity.bottom(-1), V());
  //  BOOST_CHECK_EQUAL(data.quantity.bottom(NaN), V());
  BOOST_CHECK_EQUAL(data.quantity.bottom(std::numeric_limits<int64_t>::min()), V());
  BOOST_CHECK_EQUAL(data.quantity.bottom(0, 0), V());
  BOOST_CHECK_EQUAL(data.quantity.bottom(-1, -1), V());
  //  BOOST_CHECK_EQUAL(data.quantity.bottom(NaN, NaN), V());
  BOOST_CHECK_EQUAL(data.quantity.bottom(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::min()), V());
  BOOST_CHECK_EQUAL(data.date.bottom(0), V());
  BOOST_CHECK_EQUAL(data.date.bottom(-1), V());
  //  BOOST_CHECK_EQUAL(data.date.bottom(NaN), V());
  BOOST_CHECK_EQUAL(data.date.bottom(std::numeric_limits<int64_t>::min()), V());
  BOOST_CHECK_EQUAL(data.date.bottom(0, 0), V());
  BOOST_CHECK_EQUAL(data.date.bottom(-1, -1), V());
  //  BOOST_CHECK_EQUAL(data.date.bottom(NaN, NaN), V());
  BOOST_CHECK_EQUAL(data.date.bottom(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::min()), V());

}

BOOST_AUTO_TEST_CASE(dimension_filter_exact_test) {
  Fixture data;
  using V = std::vector<Record>;
  //  selects records that match the specified value exactly
  data.tip.filter_exact(100);
  BOOST_CHECK_EQUAL(data.date.top(2), (V {
          { "2011-11-14T23:28:54Z",  2,  190,  100,  "tab", {1,2,3}},
          { "2011-11-14T23:23:29Z",  2,  190,  100,  "tab", {2,3,4}}
                                          }));
  data.tip.filter_all();
  // "allows the filter value to be null
  data.tip.filter_exact(0); // equivalent to 0 by natural ordering
  BOOST_CHECK_EQUAL(data.date.top(2), (V {
              { "2011-11-14T22:58:54Z",  2,  100,  0,  "visa", {2,3,4}},
              { "2011-11-14T22:48:05Z",  2,  91,  0,  "tab", {2,4,5}}
      }));
  data.tip.filter_all();
}

BOOST_AUTO_TEST_CASE(dimension_filter_range_test) {
  Fixture data;
  //using V = std::vector<Record>;

  //  selects records greater than or equal to the inclusive lower bound
  data.total.filter_range(100, 190);
  auto top = data.date.top(std::numeric_limits<int64_t>::max());
  for(auto & v : top) {
    BOOST_CHECK(v.total >= 100);
  }
  
  data.total.filter_range(110, 190);
  auto top2 = data.date.top(std::numeric_limits<int64_t>::max());
  for(auto & v : top2) {
    BOOST_CHECK(v.total >= 110);
  }
  data.total.filter_all();
  //"selects records less than the exclusive lower bound": function(data) {
  data.total.filter_range(100, 200);
  {
    auto top = data.date.top(std::numeric_limits<int64_t>::max());
    for(auto & v : top) {
      BOOST_CHECK(v.total < 200);
    }
  }
  {
    data.total.filter_range(100, 190);
    auto top = data.date.top(std::numeric_limits<int64_t>::max());
    for(auto & v : top) {
      BOOST_CHECK(v.total < 190);
    }
  }
  data.total.filter_all();
}

BOOST_AUTO_TEST_CASE(dimension_filter_all_test) {
  Fixture data;
  
  // clears the filter
  data.total.filter_range(100, 200);
  BOOST_CHECK(data.date.top(std::numeric_limits<int64_t>::max()).size() < 43);
  data.total.filter_all();
  BOOST_CHECK_EQUAL(data.date.top(std::numeric_limits<int64_t>::max()).size(), (unsigned long)43);
}

BOOST_AUTO_TEST_CASE(dimension_filter_function1_test) {
  Fixture data;
  //  auto Infinity = std::numeric_limits<int>::max();

  // selects records according to an arbitrary function

  data.total.filter_function( [](auto d) { return d % 2; });
  {
    auto top = data.date.top(std::numeric_limits<int64_t>::max());
    for(auto & v : top) {
      BOOST_CHECK(v.total % 2);
    }
  }
  
  data.total.filter_all();
}
BOOST_AUTO_TEST_CASE(dimension_filter_function2_test) {
  Fixture data;
  auto Infinity = std::numeric_limits<int>::max();

  {
    // respects truthy values
    auto  group = data.quantity.feature_all_count();
    data.total.filter_range(0, Infinity);
    data.total.filter_function([](auto ) { return 1; });
    BOOST_CHECK_EQUAL(group.value(), (unsigned long)43);
    data.total.filter_function([](auto ) { return 0; });
    BOOST_CHECK_EQUAL(group.value(), (unsigned long)0);
    data.total.filter_all();
  }
}
BOOST_AUTO_TEST_CASE(dimension_filter_function3_test) {
  Fixture data;
  //  auto Infinity = std::numeric_limits<int>::max();

  // "groups on the first dimension are updated correctly
  {
    auto  group = data.date.feature_all_count();
    data.total.filter_function([](auto d) { return d == 90; });

    BOOST_CHECK_EQUAL(group.value(), (unsigned long)13);
    data.total.filter_function([](auto d) { return d == 91; });
    auto t = group.value();
    BOOST_CHECK_EQUAL(t, (unsigned long)1);
    data.total.filter_all();
  }
}

BOOST_AUTO_TEST_CASE(dimension_filter_function4_test) {
  Fixture data;
  auto Infinity = std::numeric_limits<int>::max();
  // followed by filter_range
  data.total.filter_function([]( auto d) { return d % 2; });
  data.total.filter_range(100, 200);
  auto t = data.date.top(Infinity);
  BOOST_CHECK_EQUAL(data.date.top(Infinity).size(), (unsigned long)19);
  data.total.filter_all();
}

BOOST_AUTO_TEST_CASE(filter_test) {
  Fixture data;

  // is equivalent to filter_range 
    data.total.filter(100, 190);
    auto top = data.date.top(Infinity);
    
    BOOST_CHECK(std::all_of(top.begin(),top.end(),[](auto d) { return d.total >= 100; }));
    data.total.filter_all();


    // is equivalent to filter_exact when passed a single value
    {
      data.total.filter(100);
      auto top = data.date.top(Infinity);
    
      BOOST_CHECK(std::all_of(top.begin(),top.end(),[](auto d) { return d.total == 100; }));
    }
    
    data.total.filter_all();
    // is equivalent to filter_function when passed a function
    {
      data.total.filter([](auto d) { return d % 2; });
      auto top = data.date.top(Infinity);
      BOOST_CHECK(std::all_of(top.begin(),top.end(),[](auto d) { return d.total % 2; }));
    }
    // is equivalent to filter_all when passed none
    data.total.filter(100, 200);
    BOOST_CHECK(data.date.top(Infinity).size() < 43);
    data.total.filter();
    BOOST_CHECK_EQUAL(data.date.top(Infinity).size(), (unsigned long)43);
}

BOOST_AUTO_TEST_CASE(group_all_count_the_default_test) {
  Fixture data;
  // reduces by add, remove, and initial

  auto count =  data.quantity.feature_all(
      [](auto p, auto v, bool) { return p + v.total; },
      [](auto p, auto v, bool) { return p - v.total; },
      []() { return std::size_t(0); });
  BOOST_CHECK_EQUAL(count.value(), (unsigned long)6660);

  {
    // reduces by count
    auto g1 = data.quantity.feature_all_sum([](auto d) { return d.total; });
    BOOST_CHECK_EQUAL(g1.value(), 6660);
    auto g2 = data.quantity.feature_all_count();
    BOOST_CHECK_EQUAL(g2.value(), (unsigned long)43);
  }

  {
    // reduces by sum of accessor function
    auto g1 = data.quantity.feature_all_sum([](auto d) { return d.total; });
    BOOST_CHECK_EQUAL(g1.value(), 6660);
    auto g2 = data.quantity.feature_all_sum([](auto) { return 1; });
    BOOST_CHECK_EQUAL(g2.value(),43);
  }
  {
    // does not observe the associated dimension's filters
    auto g = data.quantity.feature_all_count();
    data.quantity.filter_range(100, 200);
    BOOST_CHECK_EQUAL(g.value(), (unsigned long)43);
    data.quantity.filter_all();
  }
  {
    //  observes other dimensions' filters
    auto g = data.quantity.feature_all_count();
    data.type.filter_exact("tab");
    BOOST_CHECK_EQUAL(g.value(), (unsigned long)32);
    data.type.filter_exact("visa");
    BOOST_CHECK_EQUAL(g.value(), (unsigned long)7);
    data.tip.filter_exact(100);
    BOOST_CHECK_EQUAL(g.value(), (unsigned long)5);
    data.type.filter_all();
    data.tip.filter_all();
  }
}
BOOST_AUTO_TEST_CASE(feature_all_dispose_test) {
  //  detaches from reduce listeners

  {
  cross::filter<int> data(std::vector<int>{0, 1, 2});
  bool callback = false; // indicates a reduce has occurred in this group
  auto dimension = data.dimension([](auto d) { return d; });
  auto other = data.dimension([](auto d) { return d; });
  auto all = dimension.feature_all(
      [&callback](auto, auto, bool) { callback = true; return 0;},
      [&callback](auto,auto, bool) { callback = true; return 0;},
      []() {return 0;});
    all.value(); // force this group to be reduced when filters change
    callback = false;
    all.dispose();
    other.filter_range(1, 2);
    BOOST_CHECK(!callback);
  }
  {
    cross::filter<int> data(std::vector<int>{0, 1, 2});
    bool callback = false; // indicates data has been added and trigered a reduce
    auto dimension = data.dimension([](auto d) { return d; });
    auto all = dimension.feature_all(
        [&callback](auto, auto, bool) { callback = true; return 0;},
        [&callback](auto,auto, bool) { callback = true; return 0;},
        []() {return 0;});
    all.value(); // force this group to be reduced when data is added
    callback = false;
    all.dispose();
    data.add(std::vector<int>{3,4,5});
    BOOST_CHECK(!callback);
  }

}
BOOST_AUTO_TEST_CASE(group_all_sum_test) {
  Fixture data;
  
  //auto total = 

  // determines the computed reduce value
  {
  auto total = data.quantity.feature_all(
      [](auto p, auto, bool) { return p + 1; },
      [](auto p, auto, bool) { return p - 1; },
      []() { return 0; });
  BOOST_CHECK_EQUAL(total.value(), 43);
  }

  auto total = data.quantity.feature_all_sum([](auto r) {return r.total; });
  // returns the sum total of matching records
  BOOST_CHECK_EQUAL(total.value(), 6660);
  // does not observe the associated dimension's filters
  data.quantity.filter_range(100, 200);
  BOOST_CHECK_EQUAL(total.value(), 6660);
  data.quantity.filter_all();

  // observes other dimensions' filters
  data.type.filter_exact("tab");
  BOOST_CHECK_EQUAL(total.value(), 4760);
  data.type.filter_exact("visa");
  BOOST_CHECK_EQUAL(total.value(), 1400);
  data.tip.filter_exact(100);
  BOOST_CHECK_EQUAL(total.value(), 1000);
  data.type.filter_all();
  data.tip.filter_all();
}


BOOST_AUTO_TEST_CASE(group_gkey_defaults_value) {
  Fixture data;
  
  auto hours = data.date.feature_count([](auto d) {
      auto dt = d;  dt[14] = dt[15] = dt[17] = dt[18] = '0';return dt; });
  auto types = data.type.feature_count();
  auto Infinity  = std::numeric_limits<int64_t>::max();
  
  // key defaults to value
  auto top = types.top(Infinity);
  std::vector<std::pair<std::string,std::size_t>> test =  {
    {"tab",  32},
    {"visa", 7},
    {"cash", 4}
  };
 
  BOOST_CHECK(check_collections_equal(top.begin(),top.end(),test.begin(),test.end()));
}


BOOST_AUTO_TEST_CASE(group_cardinality_may_be_greater_than_256_test) {
  std::vector<int> v(256);
  std::iota(v.begin(),v.end(),0);
  v.push_back(256);
  v.push_back(256);
  cross::filter<int> data(v);
  
  auto index = data.dimension([](auto d) { return d; });
  auto indexes = index.feature_count();
  auto top1 = index.top(2);
  std::vector<int> test1 = {256, 256};
  auto top2 = indexes.top(1);
  std::vector<std::pair<int,std::size_t>> test2 = {{256, 2}};
  BOOST_CHECK(check_collections_equal(top1.begin(),top1.end(),test1.begin(),test1.end()));
  BOOST_CHECK(check_collections_equal(top2.begin(),top2.end(),test2.begin(),test2.end()));
  BOOST_CHECK_EQUAL(indexes.size(), (unsigned long)257);
}

BOOST_AUTO_TEST_CASE(group_cardinality_may_be_greater_than_65536_test) {
  std::vector<int> v(65536);
  std::iota(v.begin(),v.end(),0);
  v.push_back(65536);
  v.push_back(65536);
  cross::filter<int> data(v);
  
  auto index = data.dimension([](auto d) { return d; });
  auto indexes = index.feature_count();
  auto top1 = index.top(2);
  std::vector<int> test1 = {65536, 65536};
  auto top2 = indexes.top(1);
  std::vector<std::pair<int,std::size_t>> test2 = {{65536, 2}};
  BOOST_CHECK(check_collections_equal(top1.begin(),top1.end(),test1.begin(),test1.end()));
  BOOST_CHECK(check_collections_equal(top2.begin(),top2.end(),test2.begin(),test2.end()));
  BOOST_CHECK_EQUAL(indexes.size(), (unsigned long)65537);
}

BOOST_AUTO_TEST_CASE(group_adds_all_records_before_removing_filtered_test) {
  Fixture data;
  
  data.quantity.filter(1);
  // Group only adds
  auto addGroup = data.type.feature(
      [](auto p, auto, bool ) {
        ++p;
        return p;
      },
      [](auto p, auto,bool) {
        return p;
      },
      []() {
        return std::size_t(0);
      }
                                       );
  // Normal group
  auto stdGroup = data.type.feature_count();
  BOOST_CHECK(addGroup.top(1)[0].second > std::size_t(stdGroup.top(1)[0].second));
  data.quantity.filter_all();

}
BOOST_AUTO_TEST_CASE(group_size_test) {
  Fixture data;
  
  auto hours = data.date.feature_count([](auto d) {
      auto dt = d;  dt[14] = dt[15] = dt[17] = dt[18] = '0';return dt; });
  auto types = data.type.feature_count();

  // returns the cardinality
  BOOST_CHECK_EQUAL(hours.size(), (unsigned long)8);
  BOOST_CHECK_EQUAL(types.size(), (unsigned long)3);

  // ignores any filters
  data.type.filter_exact("tab");
  data.quantity.filter_range(100, 200);
  BOOST_CHECK_EQUAL(hours.size(), (unsigned long)8);
  BOOST_CHECK_EQUAL(types.size(), (unsigned long)3);
  data.quantity.filter_all();
  data.type.filter_all();

}

BOOST_AUTO_TEST_CASE(group_reduce_test) {
  Fixture data;
  auto hours_key = [](auto d) {
    auto dt = d;  dt[14] = dt[15] = dt[17] = dt[18] = '0';return dt; };
  
  auto hours = data.date.feature_count(hours_key);
  auto types = data.type.feature_count();
  // defaults to count
  BOOST_CHECK_EQUAL(hours.top(1)[0], (std::pair<std::string, std::size_t>("2011-11-14T17:00:00Z",9)));

  // determines the computed reduce value
  auto hours_sum = data.date.feature_sum([](auto d) 
                                            {return d.total;
                                            }, hours_key);
  BOOST_CHECK_EQUAL(hours_sum.top(1)[0], (std::pair<std::string, int>("2011-11-14T17:00:00Z",1240)));
}
BOOST_AUTO_TEST_CASE(group_reduce2_test) {
  struct Rec2 {
    int foo;
    int val;
  };
  
  cross::filter<Rec2> data;
  data.add(std::vector<Rec2>{{ 1,  2}, { 2,  2}, { 3,  2}, { 3,  2}});
  auto foo = data.dimension([](auto d) { return d.foo; });
  auto bar = data.dimension([](auto d) { return d.foo; });
  auto val = data.dimension([](auto d) { return d.val; });
  auto groupMax = bar.feature([](auto p,auto v, bool n){
      if(n) {
        p += v.val;
      }
      return p;
    },
    [](auto p,auto v,bool n) {
      if(n) {
        p -= v.val;
      }
      return p;
    },
    []() {
      return 0;
    });
  auto groupSum = bar.feature_sum([](auto d) { return d.val; });

  // gives reduce functions information on lifecycle of data element
  // on group creation
  BOOST_CHECK_EQUAL(groupMax.all(), groupSum.all());

  // on filtering
  foo.filter_range(1, 3);
  {
    auto groupMaxAll =groupMax.all();
    auto groupSumAll = groupSum.all();
    std::vector<std::pair<int,int>> t1{{ 1, 2 }, { 2, 2 }, {  3,  4 }};
    std::vector<std::pair<int,int>> t2 {{ 1,  2 }, {2, 2 }, { 3,0 }};
    
    BOOST_CHECK(check_collections_equal(groupMaxAll, t1));
    BOOST_CHECK(check_collections_equal(groupSumAll,t2));
  }
  
  foo.filter_all();

  // on adding data after group creation
  data.add({1, 2});
  BOOST_CHECK(check_collections_equal(groupMax.all(), groupSum.all()));

  auto t = groupMax.all();
  std::cout << "groupMax1=";
  boost::test_tools::tt_detail::print_log_value<std::vector<std::pair<int,int>>>()(std::cout,t);
  std::cout << std::endl;

  // on adding data when a filter is in place
  foo.filter_range(1,3);
  data.add({ 3, 1});
  t = groupMax.all();
  std::cout << "groupMax2=";
  boost::test_tools::tt_detail::print_log_value<std::vector<std::pair<int,int>>>()(std::cout,t);
  std::cout << std::endl;
  
  
  
  BOOST_TEST(t == (std::vector<std::pair<int,int>>{{1,4 }, { 2,2 }, { 3,5 }}), boost::test_tools::per_element());
  
  BOOST_TEST(groupSum.all() == (std::vector<std::pair<int,int>>{{ 1,4 }, { 2, 2 }, { 3, 0 }}), boost::test_tools::per_element()); 

  foo.filter_all();
  
  // on removing data after group creation
  val.filter(1);
  data.remove();
  BOOST_TEST(groupMax.all() == (std::vector<std::pair<int,int>>{{1,4 },{ 2,2 },{ 3,4 }}),boost::test_tools::per_element());
  BOOST_TEST(groupSum.all() ==  (std::vector<std::pair<int,int>>{{1,0 },{ 2,0 },{ 3,0 }}),boost::test_tools::per_element());

  val.filter_all();
  BOOST_TEST(groupMax.all() ==  groupSum.all(),boost::test_tools::per_element());

}

BOOST_AUTO_TEST_CASE(group_top_test) {
  Fixture data;
  auto hours_key = [](auto v) -> std::size_t {
    int y = stoi(v.substr(0,4));
    int m = stoi(v.substr(5,2));
    int d = stoi(v.substr(8,2));
    int h = stoi(v.substr(11,2));
    return y*1000000 + m*10000 + d*100 + h;
};

  auto hours = data.date.feature_count(hours_key);
  // returns the top k groups by reduce value, in descending order
  BOOST_TEST(hours.top(3) == (std::vector<std::pair<std::size_t,std::size_t>>{{2011111417,9},
        {2011111416,7},
        {2011111421,6}
      }),boost::test_tools::per_element());

  // "observes the specified order
  hours.order([](auto v) ->int64_t { return -v; });
  BOOST_TEST(hours.top(3) == (std::vector<std::pair<std::size_t,std::size_t>>{
        {2011111420,2},
        {2011111419,3},
        {2011111418,5}
      }),boost::test_tools::per_element());
      hours.order([](auto v) { return v; });

}
BOOST_AUTO_TEST_CASE(group_order_test) {
  Fixture data;
  auto hours_key = [](auto v) -> std::size_t {
    int y = stoi(v.substr(0,4));
    int m = stoi(v.substr(5,2));
    int d = stoi(v.substr(8,2));
    int h = stoi(v.substr(11,2));
    return y*1000000 + m*10000 + d*100 + h;
  };

  auto hours = data.date.feature_count(hours_key);
  
  //  defaults to the identity function
  BOOST_TEST(hours.top(1) == (std::vector<std::pair<std::size_t,std::size_t>>{
        {2011111417, 9}
      }), boost::test_tools::per_element());

  // is useful in conjunction with a compound reduce value
  
  auto hours2 = data.date.feature(
      [](CompoundReduce & p, auto v, bool) { ++p.count; p.total += v.total; return p; },
      [](CompoundReduce & p, auto v, bool) { --p.count; p.total -= v.total; return p; },
      []() { return CompoundReduce{0, 0}; }, hours_key);

  hours2.order([](auto v) { return v.total;});

  BOOST_TEST(hours2.top(1) == (std::vector<std::pair<std::size_t,CompoundReduce>>{
        {2011111417, {9, 1240}}
      }), boost::test_tools::per_element());

}

BOOST_AUTO_TEST_CASE(group_dispose_test) {

  {
    
   //   "detaches from reduce listeners"
    cross::filter<int> data(std::vector<int>{0, 1, 2});
    bool callback = false; // indicates a reduce has occurred in this group
    auto dimension = data.dimension([](auto d) { return d; });
    auto other = data.dimension([](auto d) { return d; });
    auto group = dimension.feature(
        [&callback](int&, const int&, bool) { callback = true; return 0;},
        [&callback](int&, const int&, bool) { callback = true; return 0;},
        []() {return 0;});
    group.all(); // force this group to be reduced when filters change
    callback = false;
    group.dispose();
    other.filter_range(1, 2);
    BOOST_TEST(callback == false);
  }

  {
    // detaches from add listeners
    cross::filter<int> data(std::vector<int>{0, 1, 2});
    bool callback = false; // indicates data has been added and the group has been reduced
    auto dimension = data.dimension([](auto d) { return d; });
    auto other = data.dimension([](auto d) { return d; });
    auto group = dimension.feature(
        [&callback](auto, auto, auto) { callback = true; return 0;},
        [&callback](auto, auto, auto) { callback = true; return 0;},
        []() {return 0;});
    group.all(); // force this group to be reduced when filters change
    callback = false;
    group.dispose();
    data.add(std::vector<int>{3, 4, 5});
    BOOST_TEST(callback == false);
  }
}

BOOST_AUTO_TEST_CASE(dispose_test) {

  {
  // detaches from add listeners
    cross::filter<int> data(std::vector<int>{0, 1, 2});
    bool callback = false; // indicates a reduce has occurred in this group
    auto dimension = data.dimension([&callback](auto d) { callback = true; return d; });
    callback = false;
    dimension.dispose();
    data.add(std::vector<int>{3, 4, 5});
    BOOST_TEST(callback == false);
  }
  {  
    // "detaches groups from reduce listeners": function() {
    cross::filter<int> data(std::vector<int>{0, 1, 2});
    bool callback = false;// indicates a reduce has occurred in this group
    auto dimension = data.dimension([](auto d) { return d; });
    auto other = data.dimension([](auto d) { return d; });
    auto group = dimension.feature(
        [&callback](auto, auto, auto) { callback = true; return 0;},
        [&callback](auto, auto, auto) { callback = true; return 0;},
        []() {return 0;});

    group.all(); // force this group to be reduced when filters change
    callback = false;
    dimension.dispose();
   
    other.filter_range(1, 2);
    BOOST_TEST(callback == false);
  }

  {
    // "detaches groups from add listeners": function() {
    cross::filter<int> data(std::vector<int>{0, 1, 2});
    bool callback = false; // indicates data has been added and the group has been reduced
    auto dimension = data.dimension([](auto d) { return d; });
    auto other = data.dimension([](auto d) { return d; });
    auto group = dimension.feature(
        [&callback](auto, auto, auto) { callback = true; return 0;},
        [&callback](auto, auto, auto) { callback = true; return 0;},
        []() {return 0;});
    group.all(); // force this group to be reduced when    filters change
    callback = false;
    dimension.dispose();
    data.add(std::vector<int>{3, 4, 5});
    BOOST_TEST(!callback);
  }
  {
  // "clears dimension filters from groups": function() {
    cross::filter<int> data(std::vector<int>{0, 0, 2, 2});
    auto d1 = data.dimension([](auto d) { return -d; });
    auto d2 = data.dimension([](auto d) { return +d; });
    auto g2 = d2.feature_count([](auto d) { return int(std::round( d / 2.0 ) * 2); });
    auto all = g2.all();

    d1.filter_range(-1, 1); // a filter is present when the dimension is disposed

    d1.dispose();

    BOOST_TEST(g2.all() == (std::vector<std::pair<int,std::size_t>> {
          {0,2}, {2,2} }));
  }
}

BOOST_AUTO_TEST_CASE(feature_all_determines_the_computed_reduce_value) {
  Fixture data;
  auto allGrouped = data.cr.feature_count();
  BOOST_CHECK_EQUAL(allGrouped.value(), (unsigned long)43);
}

BOOST_AUTO_TEST_CASE(feature_all_gives_reduce_functions_information_on_lifecycle_of_data_element) {
  struct Rec2{
    int foo;
    int val;
  };
  
  Rec2 vec[] = {{1,2},{2,2},{3,2},{3,2}};
  cross::filter<Rec2> data;
  data.add(vec);
  auto foo = data.dimension([](auto r) { return r.foo;});
  auto bar = data.dimension([](auto r) { return r.foo;});
  auto val = data.dimension([](auto r) { return r.val;});
  auto groupMax = bar.feature_all([](auto v, auto r, bool n) {if(n) {v+= r.val;} return v;},
                                  [](auto v, auto r, bool n) {if(n) {v-=r.val;} return v;},
                                  [](){ return int(0);});
  auto groupSum = bar.feature_all_sum([](auto r) { return r.val;});

  // on group creation
  BOOST_CHECK_EQUAL(groupMax.value(),groupSum.value());

  // on filtering
  foo.filter_range(1,3);
  BOOST_CHECK_EQUAL(groupMax.value(),8);
  BOOST_CHECK_EQUAL(groupSum.value(),4);
  foo.filter_all();

  // on adding data after group creation
  data.add({1,2});
  BOOST_CHECK_EQUAL(groupMax.value(),groupSum.value());

  // on adding data when filter is in place
  foo.filter_range(1,3);
  data.add(Rec2{3,1});
  BOOST_CHECK_EQUAL(groupMax.value(),11);
  BOOST_CHECK_EQUAL(groupSum.value(),6);
  foo.filter_all();

  // on removing data after group creation
  val.filter(1);
  data.remove();
  BOOST_CHECK_EQUAL(groupMax.value(),10);
  BOOST_CHECK_EQUAL(groupSum.value(),0);
  val.filter_all();
  BOOST_CHECK_EQUAL(groupMax.value(), groupSum.value());
}
BOOST_AUTO_TEST_CASE(feature_all_crossfilter_value_test) {
  Fixture data;
  auto allGrouped = data.cr.feature_sum([](auto d) {return d.total;} );

  // returns the sum of total of matching record
  BOOST_CHECK_EQUAL(allGrouped.value(),6660);

  // observs all dimension's filters
  data.type.filter_exact("tab");
  BOOST_CHECK_EQUAL(allGrouped.value(), 4760);
  data.type.filter_exact("visa");
  BOOST_CHECK_EQUAL(allGrouped.value(), 1400);
  data.tip.filter_exact(100);
  BOOST_CHECK_EQUAL(allGrouped.value(), 1000);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(crossfilter_feature_all_dispose);
BOOST_AUTO_TEST_CASE(detaches_from_reduce_listeners) {
  cross::filter<int> data(std::vector<int>{0, 1, 2});
  bool callback = false;// indicates a reduce has occurred in this group
  auto other = data.dimension([](auto d) { return d; });
  auto all = data.feature(
      [&callback](auto, auto, auto) { callback = true; return 0;},
      [&callback](auto, auto, auto) { callback = true; return 0;},
      []() {return 0;});
  all.value(); // force this group to be reduced when filters change
  callback = false;
  all.dispose();
  other.filter_range(1, 2);
  BOOST_CHECK_EQUAL(callback,false);
}
BOOST_AUTO_TEST_CASE(detaches_from_add_listeners) {
  cross::filter<int> data(std::vector<int>{0, 1, 2});
  bool callback = false;// indicates a reduce has occurred in this group
  auto all = data.feature(
      [&callback](auto, auto, auto) { callback = true; return 0;},
      [&callback](auto, auto, auto) { callback = true; return 0;},
      []() {return 0;});
  all.value(); // force this group to be reduced when data is added
  callback = false;
  all.dispose();
  data.add(std::vector<int>{3, 4, 5});
  BOOST_CHECK_EQUAL(callback,false);
}

BOOST_AUTO_TEST_CASE(does_not_detach_other_reduce_listeners) {
  cross::filter<int> data(std::vector<int>{0, 1, 2});
  bool callback = false;// indicates a reduce has occurred in this group
  auto other = data.dimension([](auto d) { return d; });
  auto all2 = data.feature(
      [&callback](auto, auto, auto) { callback = true; return 0;},
      [&callback](auto, auto, auto) { callback = true; return 0;},
      []() {return 0;});
  auto all = data.feature_count();
  
  all2.value(); // force this group to be reduced when filters change
  callback = false;
  all.dispose();
  other.filter_range(1, 2);
  BOOST_CHECK_EQUAL(callback,true);
}
BOOST_AUTO_TEST_CASE(does_not_detach_other_add_listeners) {
  cross::filter<int> data(std::vector<int>{0, 1, 2});
  bool callback = false;// indicates a reduce has occurred in this group

  auto all2 = data.feature(
      [&callback](auto, auto, auto) { callback = true; return 0;},
      [&callback](auto, auto, auto) { callback = true; return 0;},
      []() {return 0;});
  auto all = data.feature_count();
  
  all2.value(); // force this group to be reduced when filters change
  callback = false;
  all.dispose();
  data.add(std::vector<int>{3, 4, 5});
  BOOST_CHECK_EQUAL(callback,true);
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(crossfilter_size)
BOOST_AUTO_TEST_CASE(returns_the_total_number_of_elements) {
  Fixture data;
  BOOST_CHECK_EQUAL(data.cr.size(),(unsigned long)43);
}
BOOST_AUTO_TEST_CASE(is_not_affected_by_any_dimension_filters) {
  Fixture data;
  data.quantity.filter_exact(4);
  BOOST_CHECK_EQUAL(data.cr.size(),(unsigned long)43);
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(crossfilter_all)
BOOST_AUTO_TEST_CASE(returns_the_full_data_array) {
  Fixture data;
  auto raw = data.cr.all();
  BOOST_CHECK_EQUAL(raw.size(),(unsigned long)43);
}
BOOST_AUTO_TEST_CASE(is_not_affected_by_any_dimension_filters) {
  Fixture data;
  data.quantity.filter_exact(4);
  auto raw = data.cr.all();
  BOOST_CHECK_EQUAL(raw.size(),(unsigned long)43);
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(crossfilter_allFiltered)
BOOST_AUTO_TEST_CASE(returns_the_full_data_array_if_no_filters_applied) {
  Fixture data;
  auto raw = data.cr.all_filtered();
  BOOST_CHECK_EQUAL(raw.size(),(unsigned long)43);
}

BOOST_AUTO_TEST_CASE(is_affected_by_all_dimension_filters) {
  Fixture data;
  data.quantity.filter_exact(4);
  BOOST_CHECK_EQUAL(data.cr.all_filtered().size(), (unsigned long)1);

  data.quantity.filter_exact(2);
  BOOST_CHECK_EQUAL(data.cr.all_filtered().size(), (unsigned long)35);

  data.total.filter_range(190, 300);
  BOOST_CHECK_EQUAL(data.cr.all_filtered().size(), (unsigned long)18);
}

BOOST_AUTO_TEST_CASE(is_affected_by_all_dimensions_filters_except_those_in_ignore__dimensions) {
  Fixture data;
  data.quantity.filter_exact(2);
  auto raw = data.cr.all_filtered(data.quantity);
  BOOST_CHECK_EQUAL(raw.size(), (unsigned long)43);

  data.total.filter_range(190, 300);
  raw = data.cr.all_filtered(data.total);
  BOOST_CHECK_EQUAL(raw.size(), (unsigned long)35);

  raw = data.cr.all_filtered(data.quantity, data.total);
  BOOST_CHECK_EQUAL(raw.size(), (unsigned long)43);
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(crossfilter_add)
BOOST_AUTO_TEST_CASE(increase_the_size_of_the_crossfilter) {
  cross::filter<int> data;
  BOOST_CHECK_EQUAL(data.size(),(unsigned long)0);
  data.add(std::vector<int>{0,1,2,3,4,5,6,6,6,7});
  BOOST_CHECK_EQUAL(data.size(),(unsigned long)10);
  data.add(std::vector<int>());
  BOOST_CHECK_EQUAL(data.size(),(unsigned long)10);
}
BOOST_AUTO_TEST_CASE(existing_filters_are_consistent_with_new_records) {
  cross::filter<int> data;
  auto foo = data.dimension([](auto d) {return d;});
  auto bar = data.dimension([](auto d) {return -d;});
  BOOST_TEST(foo.top(Infinity) == (std::vector<int>()), boost::test_tools::per_element());
  foo.filter_exact(42);
  data.add(std::vector<int>{43,42,41});
  BOOST_TEST(foo.top(Infinity) == (std::vector<int>{42}), boost::test_tools::per_element());
  BOOST_TEST(bar.top(Infinity) == (std::vector<int>{42}), boost::test_tools::per_element());
  data.add(std::vector<int>{43,42});
  BOOST_TEST(foo.top(Infinity) == (std::vector<int>{42,42}), boost::test_tools::per_element());
  BOOST_TEST(bar.top(Infinity) == (std::vector<int>{42,42}), boost::test_tools::per_element());
  foo.filter_range(42,44);
  data.add(43);
  BOOST_TEST(foo.top(Infinity) == (std::vector<int>{43,43,43,42,42}), boost::test_tools::per_element());
  BOOST_TEST(bar.top(Infinity) == (std::vector<int>{42,42,43,43,43}), boost::test_tools::per_element());
  foo.filter_function([](auto d) {
      return d % 2 == 1;
    });

  data.add(std::vector<int>{44,44,45});

  BOOST_TEST(foo.top(Infinity) == (std::vector<int>{45,43,43,43,41}), boost::test_tools::per_element());
  BOOST_TEST(bar.top(Infinity) == (std::vector<int>{41,43,43,43,45}), boost::test_tools::per_element());

  bar.filter_exact(-43);
  BOOST_TEST(bar.top(Infinity) == (std::vector<int>{43,43,43}), boost::test_tools::per_element());
  data.add(43);
  BOOST_TEST(bar.top(Infinity) == (std::vector<int>{43,43,43,43}), boost::test_tools::per_element());
  bar.filter_all();
  data.add(0);
  BOOST_TEST(bar.top(Infinity) == (std::vector<int>{41, 43, 43, 43, 43, 45}), boost::test_tools::per_element());
  foo.filter_all();
  BOOST_TEST(bar.top(Infinity) == (std::vector<int>{0, 41, 42, 42, 43, 43, 43, 43, 44, 44, 45}), boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(existing_groups_are_consistent_with_new_records) {
  cross::filter<int> data;
  auto foo = data.dimension([](auto d) {return d;});
  auto bar = data.dimension([](auto d) {return -d;});
  auto foos = foo.feature_count();
  auto all = data.feature_count();

  BOOST_CHECK_EQUAL(all.value(),(unsigned long)0) ;
  BOOST_TEST(foos.all() == (std::vector<std::pair<int,std::size_t>>{}), boost::test_tools::per_element());

  foo.filter_exact(42);
  data.add(std::vector<int>{43,42,41});
  BOOST_CHECK_EQUAL(all.value(),(unsigned long)1);
  auto a = foos.all();
  
  BOOST_TEST(foos.all() == (std::vector<std::pair<int,std::size_t>>{
        {41, 1}, {42,1}, {43,1}
      }), boost::test_tools::per_element() );
  bar.filter_exact(-42);
  BOOST_CHECK_EQUAL(all.value(),(unsigned long)1);
  BOOST_TEST(foos.all() == (std::vector<std::pair<int,std::size_t>>{
        {41, 0}, {42,1}, {43,0}
      }), boost::test_tools::per_element() );

  data.add(std::vector<int>{43, 42, 41});
  BOOST_CHECK_EQUAL(all.value(), (unsigned long)2);
  BOOST_TEST(foos.all() ==  (std::vector<std::pair<int,std::size_t>>{
        {41, 0}, { 42, 2}, { 43, 0}}),boost::test_tools::per_element() );
  bar.filter_all();
  BOOST_CHECK_EQUAL(all.value(),(unsigned long) 2);
  BOOST_TEST(foos.all() ==  (std::vector<std::pair<int,std::size_t>>{
        {41, 2}, { 42, 2}, { 43, 2}}),boost::test_tools::per_element() );
  foo.filter_all();
  BOOST_CHECK_EQUAL(all.value(), (unsigned long)6);
}

BOOST_AUTO_TEST_CASE(can_add_new_groups_that_are_before_existing_groups) {
  cross::filter<int> data;
  struct reduce_t{
    int foo;
    bool operator < (const reduce_t & rhs) const {
      return foo < rhs.foo;
    }
    bool operator == (const reduce_t & rhs) const {
      return foo == rhs.foo;
    }
  };

  auto foo = data.dimension([](auto d) { return +d; });
  auto order =  [](auto p) { return p.foo; };
  auto add =   [](auto &p, auto, bool) { ++p.foo; return p; };
  auto remove = [](auto &p, auto, bool) { --p.foo; return p; };
  auto initial= []() { return reduce_t{0}; };

  auto foos = foo.feature(add, remove, initial);
  foos.order(order);
  data.add(2);
  data.add(std::vector<int>{1, 1, 1});
  auto t = foos.top(2);
  
  BOOST_TEST(t ==  (std::vector<std::pair<int,reduce_t>>{
      {1, {3}}, {2, {1}}
    }));
}

BOOST_AUTO_TEST_CASE(can_add_more_than_256_groups) {
  cross::filter<int> data;
  auto foo = data.dimension([](auto d) { return +d; });
  auto bar = data.dimension([](auto d) { return +d; });
  auto foos = foo.feature_count();
  std::vector<int> v256(256);
  std::iota(v256.begin(),v256.end(),0);
  data.add(v256);
  
  std::vector<int> keys;
  std::vector<unsigned long> values;
  for(auto i : foos.all()) {
    keys.push_back(i.first);
    values.push_back(i.second);
  }
  BOOST_TEST(keys == v256, boost::test_tools::per_element());
  std::vector<std::size_t> v(256,1);
  BOOST_TEST(values == v, boost::test_tools::per_element());
  data.add(128);
  BOOST_TEST(foos.top(1) == (std::vector<std::pair<int,std::size_t>> {
        {128, 2}
      }),boost::test_tools::per_element());
  
  bar.filter_exact(0);
  std::vector<int> vn256(256);
  std::iota(vn256.begin(), vn256.end(),-256);
  data.add(vn256);
  keys.clear();
  values.clear();
  for(auto i : foos.all()) {
    keys.push_back(i.first);
    values.push_back(i.second);
  }
  std::vector<int> vfull(foos.size());
  std::iota(vfull.begin(), vfull.end(), -256);
  BOOST_TEST(keys == vfull, boost::test_tools::per_element());
  BOOST_TEST(foos.top(1) == (std::vector<std::pair<int,std::size_t>> {
        {0, 1}
      }),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_CASE(can_add_lots_of_groups_in_reverse_order) {
  struct Rec2 {
    double foo;
    int bar;
  };
  
  cross::filter<Rec2> data;
  auto foo = data.dimension([](auto d) { return -d.foo; });
  auto bar = data.dimension([](auto d) { return d.bar; });
  auto foos = foo.feature_sum([](auto d) { return d.foo; },
                                 [](auto d) ->int{ return std::floor(d);}
                                 );
  bar.filter_exact(1);
  std::vector<Rec2> d(10);
  for (int i = 0; i < 1000; i++) {
    for(int j = 0; j < 10; j++) {
      d[j] = Rec2{i+j/10.0, i % 4};
      //      std::cout << j + i*10 << ":" << d[j].foo << ',' << d[j].bar << std::endl;
    }
    data.add(d);
  }
  foos.top(1);
  
  BOOST_TEST(foos.top(1) == (std::vector<std::pair<int,double>> {{ -998, 8977.5}}),boost::test_tools::per_element());
}
BOOST_AUTO_TEST_SUITE_END();


BOOST_AUTO_TEST_SUITE(isElementFiltered)
BOOST_AUTO_TEST_CASE(test_if_elements_are_filtered) {
  Fixture data;
  
  BOOST_TEST(data.cr.is_element_filtered(0)); // quantity = 2; total = 190;
  BOOST_TEST(data.cr.is_element_filtered(2)); // quantity = 1; total = 300;
  BOOST_TEST(data.cr.is_element_filtered(6)); // quantity = 1; total = 100;
  BOOST_TEST(data.cr.is_element_filtered(0,data.quantity)); // quantity = 2; total = 190;
  BOOST_TEST(data.cr.is_element_filtered(2,data.quantity)); // quantity = 1; total = 300;
  BOOST_TEST(data.cr.is_element_filtered(6,data.quantity)); // quantity = 1; total = 100;
  BOOST_TEST(data.cr.is_element_filtered(0,data.total)); // quantity = 2; total = 190;
  BOOST_TEST(data.cr.is_element_filtered(2,data.total)); // quantity = 1; total = 300;
  BOOST_TEST(data.cr.is_element_filtered(6,data.total)); // quantity = 1; total = 100;
  BOOST_TEST(data.cr.is_element_filtered(0,data.quantity,data.total)); // quantity = 2; total = 190;
  BOOST_TEST(data.cr.is_element_filtered(2,data.quantity,data.total)); // quantity = 1; total = 300;
  BOOST_TEST(data.cr.is_element_filtered(6,data.quantity,data.total)); // quantity = 1; total = 100;
  
  data.quantity.filter_exact(1);
  BOOST_TEST(!data.cr.is_element_filtered(0)); // quantity = 2; total = 190;
  BOOST_TEST(data.cr.is_element_filtered(2)); // quantity = 1; total = 300;
  BOOST_TEST(data.cr.is_element_filtered(6)); // quantity = 1; total = 100;
  BOOST_TEST(data.cr.is_element_filtered(0,data.quantity)); // quantity = 2; total = 190;
  BOOST_TEST(data.cr.is_element_filtered(2,data.quantity)); // quantity = 1; total = 300;
  BOOST_TEST(data.cr.is_element_filtered(6,data.quantity)); // quantity = 1; total = 100;
  BOOST_TEST(!data.cr.is_element_filtered(0,data.total)); // quantity = 2; total = 190;
  BOOST_TEST(data.cr.is_element_filtered(2,data.total)); // quantity = 1; total = 300;
  BOOST_TEST(data.cr.is_element_filtered(6,data.total)); // quantity = 1; total = 100;
  BOOST_TEST(data.cr.is_element_filtered(0,data.quantity,data.total)); // quantity = 2; total = 190;
  BOOST_TEST(data.cr.is_element_filtered(2,data.quantity,data.total)); // quantity = 1; total = 300;
  BOOST_TEST(data.cr.is_element_filtered(6,data.quantity,data.total)); // quantity = 1; total = 100;

  data.total.filter_exact(100);
  BOOST_TEST(!data.cr.is_element_filtered(0)); // quantity = 2; total = 190;
  BOOST_TEST(!data.cr.is_element_filtered(2)); // quantity = 1; total = 300;
  BOOST_TEST(data.cr.is_element_filtered(6)); // quantity = 1; total = 100;
  BOOST_TEST(!data.cr.is_element_filtered(0,data.quantity)); // quantity = 2; total = 190;
  BOOST_TEST(!data.cr.is_element_filtered(2,data.quantity)); // quantity = 1; total = 300;
  BOOST_TEST(data.cr.is_element_filtered(6,data.quantity)); // quantity = 1; total = 100;
  BOOST_TEST(!data.cr.is_element_filtered(0,data.total)); // quantity = 2; total = 190;
  BOOST_TEST(data.cr.is_element_filtered(2,data.total)); // quantity = 1; total = 300;
  BOOST_TEST(data.cr.is_element_filtered(6,data.total)); // quantity = 1; total = 100;
  BOOST_TEST(data.cr.is_element_filtered(0,data.quantity,data.total)); // quantity = 2; total = 190;
  BOOST_TEST(data.cr.is_element_filtered(2,data.quantity,data.total)); // quantity = 1; total = 300;
  BOOST_TEST(data.cr.is_element_filtered(6,data.quantity,data.total)); // quantity = 1; total = 100;
}
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(on_change)
BOOST_AUTO_TEST_CASE(sends_the_eventName_with_the_callback) {
  cross::filter<int> data;
  cross::event name;
  data.onChange([&name](auto eventName) {name = eventName;});
  data.add(1);
  BOOST_CHECK_EQUAL(name,cross::dataAdded);
}
BOOST_AUTO_TEST_CASE(callback_gets_called_when_removing_all_data) {
  Fixture data;
  bool pass = false;

  data.cr.onChange([&pass](auto) {pass = true;});
  data.cr.remove();
  BOOST_TEST(pass);
}
BOOST_AUTO_TEST_CASE(callback_gets_called_when_removing_some_data) {
  Fixture data;
  int num = 0;
  data.cr.onChange([&num](auto) {num++;});
  data.total.filter(300);
  data.cr.remove();
  
  BOOST_TEST(num == 2);
}
BOOST_AUTO_TEST_CASE(callback_gets_called_when_filtering_data_various_ways) {
  Fixture data;
  int num = 0;
  data.cr.onChange([&num](auto) {num++;});
  data.total.filter(300);
  BOOST_TEST(num == 1);
  data.total.filter(100,200);
  BOOST_TEST(num == 2);
  data.total.filter([](auto d) { return d % 2;});
  BOOST_TEST(num == 3);
  data.total.filter();
  
  BOOST_TEST(num == 4);
}
BOOST_AUTO_TEST_CASE(multiple_callbacks_gets_called_in_sequence_of_registration) {
  Fixture data;
  int num = 0;
  int pass1, pass2,pass3,pass4;
  
  data.cr.onChange([&num,&pass1](auto) {pass1 = ++num;});
  data.cr.onChange([&num,&pass2](auto) {pass2 = ++num;});
  data.cr.onChange([&num,&pass3](auto) {pass3 = ++num;});
  data.cr.onChange([&num,&pass4](auto) {pass4 = ++num;});
  data.total.filter(300);
 
  BOOST_TEST(pass1 == 1);
  BOOST_TEST(pass2 == 2);
  BOOST_TEST(pass3 == 3);
  BOOST_TEST(pass4 == 4);
}

BOOST_AUTO_TEST_CASE(callback_is_removed_when_connection_disconnected) {
  Fixture data;
  int num = 0;
  auto cb = data.cr.onChange([&num](auto) {num++;});
  data.total.filter(300);
  data.total.filter(100,200);
  cb.disconnect();
  data.total.filter([](auto d) { return d % 2;});
  data.total.filter();
  
  BOOST_TEST(num == 2);
}

BOOST_AUTO_TEST_SUITE_END();

