// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE CrossFilterTest
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>
#include <deque>
#include <set>
#include "crossfilter.hpp"


struct Record {
  int a;
  int b;
};
bool operator < (const Record & lhs, const Record & rhs) {
  return lhs.a < rhs.a;
}

struct CrossFilterFixture {
  cross::filter<Record> crossfilter;
  CrossFilterFixture() {
    std::vector<Record> data;
    data.push_back({1,0});
    data.push_back({2,1});
    data.push_back({5,2});
    data.push_back({2,3});
    data.push_back({10,4});
    data.push_back({3,2});
    crossfilter.add(data);
  }
};

BOOST_AUTO_TEST_CASE(constructor_test) {
  cross::filter<Record> crossfilter;
  BOOST_CHECK_EQUAL(crossfilter.size(),(unsigned long)0);
}
BOOST_AUTO_TEST_CASE(constructor_from_vector_test) {
  std::vector<Record> vec(10);
  
  cross::filter<Record> crossfilter(vec);
  BOOST_CHECK_EQUAL(crossfilter.size(),(unsigned long)10);
}
BOOST_AUTO_TEST_CASE(constructor_from_deque_test) {
  std::deque<Record> vec(10);
  
  cross::filter<Record> crossfilter(vec);
  BOOST_CHECK_EQUAL(crossfilter.size(),(unsigned long)10);
}

BOOST_AUTO_TEST_CASE(constructor_from_set_test) {
  std::set<Record> vec;
  vec.insert({10,0});
  vec.insert({2,0});
  cross::filter<Record> crossfilter(vec);
  BOOST_CHECK_EQUAL(crossfilter.size(),(unsigned long)2);
}

BOOST_AUTO_TEST_CASE(constructor_from_array_test) {
  Record vec[]={{10,0},{20,0}};
  cross::filter<Record> crossfilter(vec);
  BOOST_CHECK_EQUAL(crossfilter.size(),(unsigned long)2);
}

BOOST_AUTO_TEST_CASE(add_vector_test) {
  cross::filter<Record> cf;
  std::vector<Record> data;

  data.push_back({1,0});
  data.push_back({2,0});
  
  cf.add(data);
  BOOST_CHECK_EQUAL(cf.size(),(unsigned long)2);
}

BOOST_AUTO_TEST_CASE(add_array_test) {
  Record vec[]={{10,0},{20,0}};
  cross::filter<Record> cf;
  BOOST_CHECK_EQUAL(cf.size(),(unsigned long)0);
  cf.add(vec);
  BOOST_CHECK_EQUAL(cf.size(),(unsigned long)2);
}

BOOST_AUTO_TEST_CASE(add_deque_test) {
  cross::filter<Record> cf;
  std::deque<Record> data;

  data.push_back({1,0});
  data.push_back({2,0});
  
  cf.add(data);
  BOOST_CHECK_EQUAL(cf.size(),(unsigned long)2);
}
BOOST_AUTO_TEST_CASE(add_set_test) {
  cross::filter<Record> cf;
  std::set<Record> data;
  data.insert({10,0});
  data.insert({2,0});
  
  cf.add(data);
  BOOST_CHECK_EQUAL(cf.size(),(unsigned long)2);
}
BOOST_AUTO_TEST_CASE(add_value_test) {
  cross::filter<Record> cf;
  cf.add({10,0});
  cf.add({2,0});
  
  BOOST_CHECK_EQUAL(cf.size(),(unsigned long)2);
}

BOOST_AUTO_TEST_CASE(dimension_create_test) {
  CrossFilterFixture cff;
  auto dim = cff.crossfilter.dimension([](auto  r) { return r.a;});
}
// BOOST_AUTO_TEST_CASE(dimension_create_record_without_def_constructor_test) {
//   cross::filter<Record2> cf;
  
//   auto dim = cf.dimension<int>([](auto  r) { return r.a;});
// }

BOOST_AUTO_TEST_CASE(dimension_add_test) {
  CrossFilterFixture cff;
  auto dim = cff.crossfilter.dimension([](Record r) { return r.a;});
  dim.filter_range(2,3);
  auto t1 = dim.top(10);
  
  BOOST_CHECK_EQUAL(dim.top(10).size(),(unsigned long)2);
  std::vector<Record> data;
  data.push_back({2,0});
  data.push_back({1,0});
  cff.crossfilter.add(data);
  auto top = dim.top(10);
  auto bottom = dim.bottom(10);
  
  BOOST_CHECK_EQUAL(top.size(),(unsigned long)3);
  BOOST_CHECK_EQUAL(top[0].a,2);
  BOOST_CHECK_EQUAL(top[0].b,0);
  
  BOOST_CHECK_EQUAL(bottom.size(),(unsigned long)3);
  BOOST_CHECK_EQUAL(bottom[0].a,2);
  BOOST_CHECK_EQUAL(bottom[0].b,1);


}
BOOST_AUTO_TEST_CASE(dimension_filter_top_test) {
  CrossFilterFixture cff;
  auto dim = cff.crossfilter.dimension([](Record r) { return r.a;});
  dim.filter_range(2,3);
  auto result = dim.top(10,0);
  BOOST_CHECK_EQUAL(result.size(),(unsigned long)2);
}

BOOST_AUTO_TEST_CASE(dimenstion_filter_bottom_test) {
  CrossFilterFixture cff;

  auto dim = cff.crossfilter.dimension([](Record r) {return r.a;});
  dim.filter_range(2,3);

  auto result  = dim.bottom(10,0);
  // for(auto & r : result) {
  //   std::cout << r.a << ',' << r.b << std::endl;
    
  // }
  BOOST_CHECK_EQUAL(result.size(),(unsigned long)2);
}

BOOST_AUTO_TEST_CASE(dimension_filter_range_test) {
  CrossFilterFixture cff;
  auto dim = cff.crossfilter.dimension([](Record r) {return r.a;});

  dim.filter_range(2,3);
  auto top = dim.top(10);
  // for(auto & r : top) {
  //   std::cout << r.a << ',' << r.b << std::endl;
    
  // }

  BOOST_CHECK_EQUAL(top.size(),(unsigned long)2);
  BOOST_CHECK_EQUAL(top[0].a,2);
  BOOST_CHECK_EQUAL(top[1].a,2);
  
}

BOOST_AUTO_TEST_CASE(dimension_filter_range_bottom_test) {
  CrossFilterFixture cff;
  auto dim = cff.crossfilter.dimension([](Record r) {return r.a;});

  dim.filter_range(2,3);
  auto bottom = dim.bottom(10);
  // for(auto & r : bottom) {
  //   std::cout << r.a << ',' << r.b << std::endl;
    
  // }
  // BOOST_CHECK_EQUAL(bottom.size(),3);
  // BOOST_CHECK_EQUAL(bottom[0].a,2);
  // BOOST_CHECK_EQUAL(bottom[2].a,3);
  
}

BOOST_AUTO_TEST_CASE(dimension_filter_exact_bottom_test) {
  CrossFilterFixture cff;
  auto dim = cff.crossfilter.dimension([](Record r) {return r.a;});

  dim.filter_exact(2);
  auto bottom = dim.bottom(10);
  BOOST_CHECK_EQUAL(bottom.size(),(unsigned long)2);
  BOOST_CHECK_EQUAL(bottom[0].a,2);
  BOOST_CHECK_EQUAL(bottom[0].b,1);
  BOOST_CHECK_EQUAL(bottom[1].a,2);
  BOOST_CHECK_EQUAL(bottom[1].b,3);
  
}

BOOST_AUTO_TEST_CASE(dimension_filter_all_test) {
  CrossFilterFixture cff;
  auto dim = cff.crossfilter.dimension([](Record r) {return r.a;});
  dim.filter_exact(2);
  auto bottom = dim.bottom(10);
  BOOST_CHECK_EQUAL(bottom.size(),(unsigned long)2);
  BOOST_CHECK_EQUAL(bottom[0].a,2);
  BOOST_CHECK_EQUAL(bottom[0].b,1);
  BOOST_CHECK_EQUAL(bottom[1].a,2);
  BOOST_CHECK_EQUAL(bottom[1].b,3);

  dim.filter_all();
  bottom = dim.bottom(10);
  BOOST_CHECK_EQUAL(bottom.size(),(unsigned long)6);
  BOOST_CHECK_EQUAL(bottom[0].a,1);
  BOOST_CHECK_EQUAL(bottom[0].b,0);
  BOOST_CHECK_EQUAL(bottom[1].a,2);
  BOOST_CHECK_EQUAL(bottom[1].b,1);

}

BOOST_AUTO_TEST_CASE(dimension_filter_predicate_test) {
  CrossFilterFixture cff;
  auto dim = cff.crossfilter.dimension([](Record r) {return r.a;});
  dim.filter_with_predicate([](int r) {return r % 2 == 0;});
  auto bottom = dim.bottom(10);
  BOOST_CHECK_EQUAL(bottom.size(),(unsigned long)3);
  BOOST_CHECK_EQUAL(bottom[0].a,2);
  BOOST_CHECK_EQUAL(bottom[0].b,1);
  BOOST_CHECK_EQUAL(bottom[2].a,10);
  BOOST_CHECK_EQUAL(bottom[2].b,4);
  
  
}
BOOST_AUTO_TEST_CASE(dimension_top_empty_result_test) {
  CrossFilterFixture cff;

  auto dim = cff.crossfilter.dimension([](Record r) {return r.a;});
  dim.filter_range(20,25);
  auto result  = dim.top(10,0);
  // for(auto & r : result) {
  //   std::cout << r.a << ',' << r.b << std::endl;
    
  // }
  BOOST_CHECK(result.empty());
  
}

BOOST_AUTO_TEST_CASE(dimension_bottom_empty_result_test) {
  CrossFilterFixture cff;

  auto dim = cff.crossfilter.dimension([](Record r) {return r.a;});
  dim.filter_range(20,25);
  auto result  = dim.bottom(10,0);
  // for(auto & r : result) {
  //   std::cout << r.a << ',' << r.b << std::endl;
    
  // }
  BOOST_CHECK(result.empty());
}

BOOST_AUTO_TEST_CASE(crossfilter_remove_data_test) {
  CrossFilterFixture cff;

  cff.crossfilter.remove();
  BOOST_CHECK_EQUAL(cff.crossfilter.size(),(unsigned long)0);
  
}
BOOST_AUTO_TEST_CASE(crossfilter_remove_predicate_data_test) {
  CrossFilterFixture cff;

  cff.crossfilter.remove([](auto r,int) { return r.a == 2;});
  BOOST_CHECK_EQUAL(cff.crossfilter.size(),(unsigned long)4);
  
}

BOOST_AUTO_TEST_CASE(crossfilter_remove_data_dimension_test) {
  CrossFilterFixture cff;
  auto dim = cff.crossfilter.dimension([](Record r) {return r.a;});
  dim.filter_range(2,3);
  
  cff.crossfilter.remove([](auto r,int ) { return r.b != 2;});
  BOOST_CHECK_EQUAL(cff.crossfilter.size(),(unsigned long)2);
  auto top = dim.top(5);
  // for(auto & r : top) {
  //   std::cout << r.a << ',' << r.b << std::endl;
  // }
  BOOST_CHECK_EQUAL(top.size(),(unsigned long)0);
}


BOOST_AUTO_TEST_CASE(group_test) {
  CrossFilterFixture cff;
  auto dim = cff.crossfilter.dimension([](Record r) {return r.a;});
  auto group = dim.feature_count([](auto r) {return r; });
 
  auto top = group.top(10);
  auto all = group.all();

    BOOST_CHECK_EQUAL(all.size(),(unsigned long)5);
    BOOST_CHECK_EQUAL(top.size(),(unsigned long)5);

  // group.reduceCount();
  // all = group.all();
  // for(auto & r : all) {
  //   std::cout << r.first << ',' << r.second << std::endl;
  // }
  // std::cout <<" ----- " << std::endl;
  
   // group.reduceSum([](Record r) {return r.a;});
   // all = group.all();
   // for(auto & r : all) {
   //   std::cout << r.first << ',' << r.second << std::endl;
   // }

}

BOOST_AUTO_TEST_CASE(group_all_test) {
  CrossFilterFixture cff;
  auto dim = cff.crossfilter.dimension([](Record r) {return r.a;});
  auto group = dim.feature_all_count();
 
  auto top = group.top(10);
  auto all = group.all();

  BOOST_CHECK_EQUAL(all.size(),(unsigned long)1);
  BOOST_CHECK_EQUAL(top.size(),(unsigned long)1);
  //  group.reduceCount();
  auto v = group.value();
  
  BOOST_CHECK_EQUAL(v,(unsigned long)6);
  
  // all = group.all();
  // for(auto & r : all) {
  //   std::cout << r.first << ',' << r.second << std::endl;
  // }

}

BOOST_AUTO_TEST_CASE(group_remove_test) {
  CrossFilterFixture cff;
  auto dim = cff.crossfilter.dimension([](Record r) {return r.a;});
  auto group = dim.feature_count([](auto r) {return r; });
  auto all = group.all();
  for(auto & r : all) {
    std::cout << r.first << ',' << r.second << std::endl;
  }
  std::cout <<" ----- " << std::endl;

  cff.crossfilter.remove([](auto r, int) { return r.b == 3;});
  BOOST_CHECK_EQUAL(group.all().size(),(unsigned long)5);
  all = group.all();
  for(auto & r : all) {
    std::cout << r.first << ',' << r.second << std::endl;
  }
  std::cout <<" ----- " << std::endl;

  cff.crossfilter.remove([](auto r, int) { return r.a == 2;});

  BOOST_CHECK_EQUAL(group.all().size(),(unsigned long)4);
  all = group.all();
  for(auto & r : all) {
    std::cout << r.first << ',' << r.second << std::endl;
  }
  std::cout <<" ----- " << std::endl;

}

struct IterableRecord {
  int a;
  int b;
  std::vector<int> vec;
};

BOOST_AUTO_TEST_CASE(iterable_dimension_test) {
  cross::filter<IterableRecord> cf;
  auto dim = cf.iterable_dimension([](auto r) { return r.vec;});
  
}
