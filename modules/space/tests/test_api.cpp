/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov
*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_api
#include <boost/test/unit_test.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/serialization/nvp.hpp>

//#include "3dparty/archive/archive.h"
//#include "3dparty/serialize/archive.h"
#include <iostream>
#include <vector>
#include "../../space.hpp"
template<typename T>
struct distance {
    int operator()( const T &lhs,  const T &rhs) const {
        return std::abs(lhs - rhs);
    }
};

BOOST_AUTO_TEST_CASE(test_insert) {
    std::vector<int> data = {3,5,-10,50,1,-200,200};
    metric::space::Tree<int,distance<int>> tree;
    for(auto t : data) {
        tree.insert(t);
        BOOST_TEST(tree.check_covering());
    }
}

BOOST_AUTO_TEST_CASE(test_insert_batch) {
    std::vector<int> data = {3,5,-10,50,1,-200,200};
    metric::space::Tree<int,distance<int>> tree;
    tree.insert(data);
    BOOST_TEST(tree.check_covering());
}
BOOST_AUTO_TEST_CASE(test_nn) {
    std::vector<int> data = {3,5,-10,50,1,-200,200};
    metric::space::Tree<int,distance<int>> tree;
    tree.insert(data);
    tree.print();
    BOOST_TEST(tree.nn(200)->data == 200);
    // for(auto d : data) {
    //     auto p = tree.nn(d);
    //     BOOST_TEST(p->data == d);
    // }
}

BOOST_AUTO_TEST_CASE(test_knn) {
    std::vector<int> data = {3,5,-10,50,1,-200,200};
    metric::space::Tree<int,distance<int>> tree;
    tree.insert(data);
    auto k1 = tree.knn(3,15);
    BOOST_TEST(k1.size() == 7);
    BOOST_TEST(k1[0].first->data == 3);
    BOOST_TEST(k1[1].first->data == 1);
    BOOST_TEST(k1[2].first->data == 5);
    BOOST_TEST(k1[3].first->data == -10);
    BOOST_TEST(k1[4].first->data == 50);
    BOOST_TEST(k1[5].first->data == 200);
    BOOST_TEST(k1[6].first->data == -200);
}

BOOST_AUTO_TEST_CASE(test_erase) {
    std::vector<int> data = {3,5,-10,50,1,-200,200};
    metric::space::Tree<int,distance<int>> tree;
    tree.insert(data);
    //    tree.print();
    for(auto d : data) {
        tree.erase(d);
        BOOST_TEST(tree.check_covering());
        //        tree.print();
    }
}

BOOST_AUTO_TEST_CASE(test_erase_root) {
    std::vector<int> data = {3,5,-10,50,1,-200,200};
    metric::space::Tree<int,distance<int>> tree;
    tree.insert(data);
    //    tree.print();
    for(int i = 0; i < 7; i++) {
        auto root = tree.get_root();
        tree.erase(root->data);
        BOOST_TEST(tree.check_covering());
        //        tree.print();
    }
}

BOOST_AUTO_TEST_CASE(test_insert_if) {
    metric::space::Tree<int,distance<int>> tree;
    tree.insert(1);
    BOOST_TEST(!tree.insert_if(2,10));
    BOOST_TEST(tree.insert_if(15,10));
    BOOST_TEST(!tree.insert_if(14,10));
    BOOST_TEST(tree.insert_if(26,10));
}

BOOST_AUTO_TEST_CASE(test_insert2) {
  std::vector<int> data = {7,8,9,10,11,12,13};
  metric::space::Tree<int,distance<int>> tree;
  tree.insert(data);
  tree.print();
}

BOOST_AUTO_TEST_CASE(test_to_json) {
    metric::space::Tree<int,distance<int>> tree;
    tree.insert(1);
    auto s = tree.to_json();
    std::string json1 = "{\n\"nodes\": [\n{ \"id\":0, \"values\":1}\n],\n\"edges\": [\n]}\n";
    BOOST_TEST(s == json1);
    tree.insert(2);
    std::string json2 = "{\n\"nodes\": [\n{ \"id\":0, \"values\":1},\n{ \"id\":1, \"values\":2}\n],\n\"edges\": [\n{ \"source\":0, \"target\":1, \"distance\":1}\n]}\n";
    BOOST_TEST(tree.to_json() == json2);
}
