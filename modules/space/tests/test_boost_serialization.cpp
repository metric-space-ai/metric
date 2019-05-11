#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_boost_serialization
#include <boost/test/unit_test.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include <iostream>
#include <vector>
#include "metric_space.hpp"
template<typename T>
struct distance {
  int operator()( const T &lhs,  const T &rhs) const {
    return std::abs(lhs - rhs);
  }
};
BOOST_AUTO_TEST_CASE(test_serialize_boost_text) {
  std::vector<int> data = {3,5,-10,50,1,-200,200};
  metric_space::Tree<int,distance<int>> tree;
  tree.insert(data);
  //  tree.print();
  std::ostringstream os;
  boost::archive::text_oarchive oar(os);
  tree.serialize(oar);
  //  std::cout << os.str() << std::endl;
  metric_space::Tree<int,distance<int>> tree1;
  std::istringstream is(os.str());
  boost::archive::text_iarchive iar(is);
  tree1.deserialize(iar, is);
  //  tree1.print();
  BOOST_TEST(tree1.check_covering());
  BOOST_TEST(tree1 == tree);
}

BOOST_AUTO_TEST_CASE(test_serialize_boost_xml) {
  std::vector<int> data = {3,5,-10,50,1,-200,200};
  metric_space::Tree<int,distance<int>> tree;
  tree.insert(data);
  //  tree.print();
  std::ostringstream os;
  boost::archive::xml_oarchive oar(os);
  tree.serialize(oar);
  //  std::cout << os.str() << std::endl;
  metric_space::Tree<int,distance<int>> tree1;
  std::istringstream is(os.str());
  boost::archive::xml_iarchive iar(is);
  tree1.deserialize(iar, is);
  //  tree1.print();
  BOOST_TEST(tree1.check_covering());
  BOOST_TEST(tree1 == tree);
}

BOOST_AUTO_TEST_CASE(test_serialize_boost_binary) {
  std::vector<int> data = {3,5,-10,50,1,-200,200};
  metric_space::Tree<int,distance<int>> tree;
  tree.insert(data);
  //  tree.print();
  std::ostringstream os;
  boost::archive::binary_oarchive oar(os);
  tree.serialize(oar);
  //  std::cout << os.str() << std::endl;
  metric_space::Tree<int,distance<int>> tree1;
  std::istringstream is(os.str());
  boost::archive::binary_iarchive iar(is);
  tree1.deserialize(iar, is);
  //  tree1.print();
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
    ar & BOOST_SERIALIZATION_NVP(v) & BOOST_SERIALIZATION_NVP(vv) & BOOST_SERIALIZATION_NVP(a);
  }
};


BOOST_AUTO_TEST_CASE(test_serialize_boost_record_binary) {
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
  boost::archive::binary_oarchive oar(os);

  tree.serialize(oar);
  metric_space::Tree<Record,distance<Record>> tree1;
  std::istringstream is(os.str());
  boost::archive::binary_iarchive iar(is);
  tree1.deserialize(iar, is);
  BOOST_TEST(tree1.check_covering());
  BOOST_TEST(tree1 == tree);
}

BOOST_AUTO_TEST_CASE(test_serialize_boost_record_text) {
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
  boost::archive::text_oarchive oar(os);

  tree.serialize(oar);
  metric_space::Tree<Record,distance<Record>> tree1;
  std::istringstream is(os.str());
  boost::archive::text_iarchive iar(is);
  tree1.deserialize(iar, is);
  BOOST_TEST(tree1.check_covering());
  BOOST_TEST(tree1 == tree);
}

BOOST_AUTO_TEST_CASE(test_serialize_boost_record_xml) {
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
  boost::archive::xml_oarchive oar(os);

  tree.serialize(oar);
  std::cout << os.str() << std::endl;
  metric_space::Tree<Record,distance<Record>> tree1;
  std::istringstream is(os.str());
  boost::archive::xml_iarchive iar(is);
  tree1.deserialize(iar, is);
  BOOST_TEST(tree1.check_covering());
  BOOST_TEST(tree1 == tree);
}
