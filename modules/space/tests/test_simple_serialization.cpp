#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_boost_serialization
#include <boost/test/unit_test.hpp>
#include "../examples/assets/3dparty/serialize/archive.h"
#include <iostream>
#include <vector>
#include "../metric_space.hpp"
template<typename T>
struct distance {
  int operator()( const T &lhs,  const T &rhs) const {
    return std::abs(lhs - rhs);
  }
};
BOOST_AUTO_TEST_CASE(test_serialize_simple_fundamental) {
  std::vector<int> data = {3,5,-10,50,1,-200,200};
  metric_space::Tree<int,distance<int>> tree;
  tree.insert(data);
  std::ostringstream os;
  serialize::oarchive<std::ostringstream> oar(os);
  tree.serialize(oar);
  metric_space::Tree<int,distance<int>> tree1;
  std::istringstream is(os.str());
  serialize::iarchive<std::istringstream> iar(is);
  tree1.deserialize(iar, is);
  BOOST_TEST(tree1.check_covering());
  BOOST_TEST(tree1 == tree);
}

struct Record {
  float v;
  std::vector<float> vv;
  int a;
  float operator - (const Record & r) const {
    return v - r.v;
  }
  bool operator != (const Record rhs) {
    return v != rhs.v || vv != rhs.vv || a != rhs.a;
  }
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int) {
    ar & v & vv & a;
  }
};


BOOST_AUTO_TEST_CASE(test_serialize_record) {
  std::vector<Record> data = {
    {3.0f,{1,2,3},1},
    {5.0f,{1,6,3},2},
    {-10.0f,{1,6,3},3},
    {50.0f,{1,6,3},4},
    {1.0f,{1,6,3},5},
    {-200.0f,{1,6,3},6},
    {200.0f,{1,6,3},7}};

  metric_space::Tree<Record,distance<Record>> tree;
  tree.insert(data);
  std::ostringstream os;
  serialize::oarchive<std::ostringstream> oar(os);
  tree.serialize(oar);
  metric_space::Tree<Record,distance<Record>> tree1;
  std::istringstream is(os.str());
  serialize::iarchive<std::istringstream> iar(is);
  tree1.deserialize(iar, is);
  BOOST_TEST(tree1.check_covering());
  BOOST_TEST(tree1 == tree);
}
