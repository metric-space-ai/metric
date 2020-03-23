/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/

#include <stdexcept>
//#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE space_tree_test
#include <boost/test/unit_test.hpp>

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/unordered_map.hpp>

#include <iostream>
#include <vector>
#include "modules/space.hpp"

template <typename T, typename V=int>
struct distance {
    V operator()(const T& lhs, const T& rhs) const { return std::abs(lhs - rhs); }
};

BOOST_AUTO_TEST_CASE(test_insert)
{
    std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
    metric::Tree<int, distance<int>> tree;
    for (auto t : data) {
        tree.insert(t);
        BOOST_TEST(tree.check_covering());
    }
}

BOOST_AUTO_TEST_CASE(test_insert_batch)
{
    std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    BOOST_TEST(tree.check_covering());
}
BOOST_AUTO_TEST_CASE(test_nn)
{
    std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    tree.print();
    BOOST_TEST(tree.nn(200)->get_data() == 200);
}

BOOST_AUTO_TEST_CASE(test_knn)
{
    std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    auto k1 = tree.knn(3, 15);
    BOOST_TEST(k1.size() == 7);
    BOOST_TEST(k1[0].first->get_data() == 3);
    BOOST_TEST(k1[1].first->get_data() == 1);
    BOOST_TEST(k1[2].first->get_data() == 5);
    BOOST_TEST(k1[3].first->get_data() == -10);
    BOOST_TEST(k1[4].first->get_data() == 50);
    BOOST_TEST(k1[5].first->get_data() == 200);
    BOOST_TEST(k1[6].first->get_data() == -200);
}

BOOST_AUTO_TEST_CASE(test_erase)
{
    std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    auto tree_size = tree.size();
    BOOST_TEST(tree_size == data.size());
    BOOST_TEST(tree.check_covering());
    for (auto d : data) {
        tree.erase(d);
        BOOST_TEST(tree_size -1 == tree.size());
        tree_size = tree.size();
        BOOST_TEST(tree.check_covering());
    }
    BOOST_TEST(tree.size() == 0);
    BOOST_TEST(tree.empty() == true);
}

BOOST_AUTO_TEST_CASE(test_erase_root)
{
    std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    for (int i = 0; i < 7; i++) {
        auto root = tree.get_root();
        tree.erase(root->get_data());
        BOOST_TEST(tree.check_covering());
    }
}

BOOST_AUTO_TEST_CASE(test_insert_if)
{
    metric::Tree<int, distance<int>> tree;
    tree.insert(1);
    BOOST_TEST(!std::get<1>(tree.insert_if(2, 10)));
    BOOST_TEST(std::get<1>(tree.insert_if(15, 10)));
    BOOST_TEST(!std::get<1>(tree.insert_if(14, 10)));
    BOOST_TEST(std::get<1>(tree.insert_if(26, 10)));
}

BOOST_AUTO_TEST_CASE(test_insert2)
{
    std::vector<int> data = { 7, 8, 9, 10, 11, 12, 13 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    tree.print();
}

BOOST_AUTO_TEST_CASE(test_to_json)
{
    metric::Tree<int, distance<int>> tree;
    tree.insert(1);
    auto s = tree.to_json();
    std::string json1 = "{\n\"nodes\": [\n{ \"id\":0, \"values\":1}\n],\n\"edges\": [\n]}\n";
    BOOST_TEST(s == json1);
    tree.insert(2);
    std::string json2 = "{\n\"nodes\": [\n{ \"id\":0, \"values\":1},\n{ \"id\":1, \"values\":2}\n],\n\"edges\": [\n{ "
                        "\"source\":0, \"target\":1, \"distance\":1}\n]}\n";
    BOOST_TEST(tree.to_json() == json2);
}

// BOOST_AUTO_TEST_CASE(test_serialize_boost_text)
// {
//     std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
//     metric::Tree<int, distance<int>> tree;
//     tree.insert(data);
//     std::ostringstream os;
//     boost::archive::text_oarchive oar(os);
//     tree.serialize(oar);
//     metric::Tree<int, distance<int>> tree1;
//     std::istringstream is(os.str());
//     boost::archive::text_iarchive iar(is);
//     tree1.deserialize(iar, is);
//     BOOST_TEST(tree1.check_covering());
//     BOOST_TEST(tree1 == tree);
// }

// BOOST_AUTO_TEST_CASE(test_serialize_boost_binary)
// {
//     std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
//     metric::Tree<int, distance<int>> tree;
//     tree.insert(data);
//     std::ostringstream os;
//     boost::archive::binary_oarchive oar(os);
//     tree.serialize(oar);
//     metric::Tree<int, distance<int>> tree1;
//     std::istringstream is(os.str());
//     boost::archive::binary_iarchive iar(is);
//     tree1.deserialize(iar, is);
//     BOOST_TEST(tree1.check_covering());
//     BOOST_TEST(tree1 == tree);
// }

struct Record {
    float v;
    std::vector<float> vv;
    int a;
    float operator-(const Record& r) const { return v - r.v; }
    bool operator!=(const Record rhs) const { return v != rhs.v || vv != rhs.vv || a != rhs.a; }
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        ar& BOOST_SERIALIZATION_NVP(v) & BOOST_SERIALIZATION_NVP(vv) & BOOST_SERIALIZATION_NVP(a);
    }
};

// BOOST_AUTO_TEST_CASE(test_serialize_boost_record_binary)
// {
//     std::vector<Record> data = { { 3.0f, { 1, 2, 3 }, 1 }, { 5.0f, { 1, 6, 3 }, 2 }, { -10.0f, { 1, 6, 3 }, 3 },
//         { 50.0f, { 1, 6, 3 }, 4 }, { 1.0f, { 1, 6, 3 }, 5 }, { -200.0f, { 1, 6, 3 }, 6 }, { 200.0f, { 1, 6, 3 }, 7 } };

//     metric::Tree<Record, distance<Record, float>> tree;
//     tree.insert(data);
//     std::ostringstream os;
//     boost::archive::binary_oarchive oar(os);

//     tree.serialize(oar);
//     metric::Tree<Record, distance<Record, float>> tree1;
//     std::istringstream is(os.str());
//     boost::archive::binary_iarchive iar(is);
//     tree1.deserialize(iar, is);
//     BOOST_TEST(tree1.check_covering());
//     BOOST_TEST(tree1 == tree);
// }

// BOOST_AUTO_TEST_CASE(test_serialize_boost_record_text)
// {
//     std::vector<Record> data = { { 3.0f, { 1, 2, 3 }, 1 }, { 5.0f, { 1, 6, 3 }, 2 }, { -10.0f, { 1, 6, 3 }, 3 },
//         { 50.0f, { 1, 6, 3 }, 4 }, { 1.0f, { 1, 6, 3 }, 5 }, { -200.0f, { 1, 6, 3 }, 6 }, { 200.0f, { 1, 6, 3 }, 7 } };

//     metric::Tree<Record, distance<Record, float>> tree;
//     tree.insert(data);
//     std::ostringstream os;
//     boost::archive::text_oarchive oar(os);

//     tree.serialize(oar);
//     metric::Tree<Record, distance<Record, float>> tree1;
//     std::istringstream is(os.str());
//     boost::archive::text_iarchive iar(is);
//     tree1.deserialize(iar, is);
//     BOOST_TEST(tree1.check_covering());
//     BOOST_TEST(tree1 == tree);
// }

// BOOST_AUTO_TEST_CASE(test_serialize_boost_record_xml)
// {
//     std::vector<Record> data = { { 3.0f, { 1, 2, 3 }, 1 }, { 5.0f, { 1, 6, 3 }, 2 }, { -10.0f, { 1, 6, 3 }, 3 },
//         { 50.0f, { 1, 6, 3 }, 4 }, { 1.0f, { 1, 6, 3 }, 5 }, { -200.0f, { 1, 6, 3 }, 6 }, { 200.0f, { 1, 6, 3 }, 7 } };

//     metric::Tree<Record, distance<Record>> tree;
//     tree.insert(data);
//     std::ostringstream os;
//     boost::archive::xml_oarchive oar(os);

//     tree.serialize(oar);
//     std::cout << os.str() << std::endl;
//     metric::Tree<Record, distance<Record>> tree1;
//     std::istringstream is(os.str());
//     boost::archive::xml_iarchive iar(is);
//     tree1.deserialize(iar, is);
//     BOOST_TEST(tree1.check_covering());
//     BOOST_TEST(tree1 == tree);
// }
namespace std {
std::ostream& operator<<(std::ostream& ostr, const std::vector<std::size_t>& v)
{
    ostr << "[ ";
    for (auto i : v) {
        ostr << i << ", ";
    }
    ostr << "]";
    return ostr;
}
}
BOOST_AUTO_TEST_CASE(cluster1)
{
    std::vector<int> data = { 7, 8, 9, 10, 11, 12, 13 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    tree.print();
    std::vector<double> distribution = { 0.1, 0.2, 0.3, 0.5 };
    std::vector<std::size_t> IDS = { 1, 2, 3 };
    std::vector<int> points = { 8, 9, 10 };

    auto result = tree.clustering(distribution, IDS, data);
    auto result2 = tree.clustering(distribution, points);
    std::vector<std::vector<std::size_t>> test_result = { {}, { 1 }, { 0 }, { 2 } };
    BOOST_TEST(result == test_result, boost::test_tools::per_element());
    BOOST_TEST(result2 == test_result, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(cluster2)
{
    std::vector<int> data = { 7, 8, 9, 10, 11, 12, 13 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    tree.print();
    std::vector<double> distribution = { 0.1, 0.2, 0.3, 0.5 };
    std::vector<std::size_t> IDS = { 3 };
    std::vector<int> points = { 10 };
    auto result = tree.clustering(distribution, IDS, data);
    auto result2 = tree.clustering(distribution, points);
    std::vector<std::vector<std::size_t>> test_result = { {}, { 3 }, { 4 }, { 2 } };
    BOOST_TEST(result == test_result, boost::test_tools::per_element());
    BOOST_TEST(result2 == test_result, boost::test_tools::per_element());
    std::size_t i = 0;
    for (auto& v : result) {
        std::cout << distribution[i] << " = {";
        for (auto p : v) {
            std::cout << "[" << data[p] << ":" << p << "]"
                      << ", ";
        }
        std::cout << "}" << std::endl;
        i++;
    }
}

BOOST_AUTO_TEST_CASE(cluster3)
{
    std::vector<int> data = { 7, 8, 9, 10, 11, 12, 13 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    tree.print();
    std::vector<double> distribution = { 0.1, 0.2, 0.5, 0.9 };
    std::vector<std::size_t> IDS = { 3 };
    std::vector<int> points = { 10 };
    auto result = tree.clustering(distribution, IDS, data);
    auto result2 = tree.clustering(distribution, points);
    std::vector<std::vector<std::size_t>> test_result = { {}, { 3 }, { 4, 2 }, { 1, 0, 5 } };
    BOOST_TEST(result == test_result, boost::test_tools::per_element());
    BOOST_TEST(result2 == test_result, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(cluster_exception_unsorted)
{
    std::vector<int> data = { 7, 8, 9, 10, 11, 12, 13 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    std::vector<double> distribution1 = { 0.9, 0.1, 0.2, 0.5, 0.9 };
    std::vector<double> distribution2 = { 0.1, 0.2, 0.5, 0.9 };
    std::vector<double> distribution3 = { 0.1, 0.2, 0.5, 0.9, 1, 5 };
    std::vector<std::size_t> IDS = { 3 };
    std::vector<int> points = { 10 };
    BOOST_CHECK_THROW(tree.clustering(distribution1, IDS, data), metric::unsorted_distribution_exception);
    BOOST_CHECK_THROW(tree.clustering(distribution1, points), metric::unsorted_distribution_exception);
    BOOST_CHECK_THROW(tree.clustering(distribution3, IDS, data), metric::bad_distribution_exception);
    BOOST_CHECK_THROW(tree.clustering(distribution3, points), metric::bad_distribution_exception);
    BOOST_REQUIRE_NO_THROW(tree.clustering(distribution2, IDS, data));
    BOOST_REQUIRE_NO_THROW(tree.clustering(distribution2, points));
}

BOOST_AUTO_TEST_CASE(tree_element_access)
{
    metric::Tree<int, distance<int>> tree;
    std::vector<int> data = {1,2,3,4,5,6,7,8,9,10};
    tree.insert(data);
    for(std::size_t i = 0; i < data.size(); i++) {
        BOOST_TEST(tree[i] == data[i]);
    }
}


BOOST_AUTO_TEST_CASE(tree_distance_by_id) {
    std::vector<float> data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    metric::Tree<float, distance<float, float>> tree;
    tree.insert(data);
    for(std::size_t id = 0; id < 10; id++)  {
        std::cout << id << " -> " << tree[id] << std::endl;
    }
    // clang-format off
    /*
    (5)
    ├──(2)
    |   ├──(0)
    |   |   └──(1)
    |   └──(3)
    |       └──(4)
    ├──(6)
    |   └──(7)
    |       └──(8)
    └──(9)
    */
    // clang-format on

    // same ID
    for(std::size_t i = 0; i < 10; ++i) {
        BOOST_TEST(tree.distance_by_id(i,i) == 0);
    }
    //parent - child
    BOOST_TEST(tree.distance_by_id(5, 2) == 3);
    BOOST_TEST(tree.distance_by_id(2, 5) == 3);

    // same level, one parent
    BOOST_TEST(tree.distance_by_id(0, 3) == (1.0 + 2.0) / 2);
    BOOST_TEST(tree.distance_by_id(3, 0) == (1.0 + 2.0)/2);

    // different levels, one subtree
    BOOST_TEST(tree.distance_by_id(1, 3) == (1.0f + 2.0f + 1.0f) / 3);
    BOOST_TEST(tree.distance_by_id(3, 1) == (1.0f + 2.0f + 1.0f) / 3);

    //same levels, different subtree
    BOOST_TEST(tree.distance_by_id(0, 7) == (1.f + 1.f + 2.f + 3.f) / 4);
    BOOST_TEST(tree.distance_by_id(7, 0) == (1.f + 1.f + 2.f + 3.f) / 4);

    //different levels, different subtree
    BOOST_TEST(tree.distance_by_id(1, 7) == (1.f + 1.f + 1.f + 2.f + 3.f)/5);
    BOOST_TEST(tree.distance_by_id(7, 1) == (1.f + 1.f + 1.f + 2.f + 3.f)/5);

    BOOST_CHECK_THROW(tree.distance_by_id(100, 0), std::runtime_error);
    BOOST_CHECK_THROW(tree.distance_by_id(1, 100), std::runtime_error);
    BOOST_CHECK_THROW(tree.distance_by_id(100, 100), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(tree_distance_by_value)
{
    std::vector<float> data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    metric::Tree<float, distance<float, float>> tree;
    tree.insert(data);

    // same value
    for(std::size_t i = 0; i < 10; ++i) {
        BOOST_TEST(tree.distance(data[i],data[i]) == 0);
    }
    //parent - child
    BOOST_TEST(tree.distance(data[5], data[2]) == 3);
    BOOST_TEST(tree.distance(data[2], data[5]) == 3);
    BOOST_TEST(tree.distance(6.2, 3.3) == 3);
    BOOST_TEST(tree.distance(3.3, 6.2) == 3);

    // same level, one parent
    BOOST_TEST(tree.distance(data[0], data[3]) == (1.0 + 2.0) / 2);
    BOOST_TEST(tree.distance(data[3], data[0]) == (1.0 + 2.0)/2);
    BOOST_TEST(tree.distance(1.3, 4.2) == (1.0 + 2.0) / 2);
    BOOST_TEST(tree.distance(4.2, 1.3) == (1.0 + 2.0) / 2);

    // different levels, one subtree
    BOOST_TEST(tree.distance(2.2, 3.9) == (1.0f + 2.0f + 1.0f) / 3);
    BOOST_TEST(tree.distance(3.9, 2.2) == (1.0f + 2.0f + 1.0f) / 3);

    //same levels, different subtree
    BOOST_TEST(tree.distance(0.1, 8.2) == (1.f + 1.f + 2.f + 3.f) / 4);
    BOOST_TEST(tree.distance(8.2, 0.7) == (1.f + 1.f + 2.f + 3.f) / 4);

    //different levels, different subtree
    BOOST_TEST(tree.distance(1.9, 7.9) == (1.f + 1.f + 1.f + 2.f + 3.f)/5);
    BOOST_TEST(tree.distance(7.9, 1.9) == (1.f + 1.f + 1.f + 2.f + 3.f)/5);
}


BOOST_AUTO_TEST_CASE(tree_to_distance_matrix) {
    std::vector<float> data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    metric::Tree<float, distance<float, float>> tree;
    tree.insert(data);
    auto m = tree.matrix();
    distance<float, float> dist;
    for(std::size_t i = 0; i < m.rows(); i++) {
        for (std::size_t j = 0; j < m.columns(); j++) {
            if( i < j) {
                BOOST_TEST(m(i,j) == dist(data[i], data[j]));
            } else {
                BOOST_TEST(m(j, i) == dist(data[i], data[j]));
            }
        }
    }
}
