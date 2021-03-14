/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <stdexcept>
#include <vector>

#include "modules/space.hpp"


using namespace Catch::literals;


template <typename T, typename V=int>
struct distance {
    V operator()(const T& lhs, const T& rhs) const { return std::abs(lhs - rhs); }
};

TEST_CASE("test_insert", "[space]")
{
    std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
    metric::Tree<int, distance<int>> tree;
    for (auto t : data) {
        tree.insert(t);
        REQUIRE(tree.check_covering());
    }
}

TEST_CASE("test_insert_batch", "[space]")
{
    std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    REQUIRE(tree.check_covering());
}
TEST_CASE("test_nn", "[space]")
{
    std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    //tree.print();
    REQUIRE(tree.nn(200)->get_data() == 200);
}

TEST_CASE("test_knn", "[space]")
{
    std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    auto k1 = tree.knn(3, 15);
    REQUIRE(k1.size() == 7);
    REQUIRE(k1[0].first->get_data() == 3);
    REQUIRE(k1[1].first->get_data() == 1);
    REQUIRE(k1[2].first->get_data() == 5);
    REQUIRE(k1[3].first->get_data() == -10);
    REQUIRE(k1[4].first->get_data() == 50);
    REQUIRE(k1[5].first->get_data() == 200);
    REQUIRE(k1[6].first->get_data() == -200);
}

TEST_CASE("test_erase", "[space]")
{
    std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    auto tree_size = tree.size();
    REQUIRE(tree_size == data.size());
    REQUIRE(tree.check_covering());
    for (auto d : data) {
        tree.erase(d);
        REQUIRE(tree_size -1 == tree.size());
        tree_size = tree.size();
        REQUIRE(tree.check_covering());
    }
    REQUIRE(tree.size() == 0);
    REQUIRE(tree.empty() == true);
}

TEST_CASE("test_erase_root", "[space]")
{
    std::vector<int> data = { 3, 5, -10, 50, 1, -200, 200 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    for (int i = 0; i < 7; i++) {
        auto root = tree.get_root();
        tree.erase(root->get_data());
        REQUIRE(tree.check_covering());
    }
}

TEMPLATE_TEST_CASE("test_insert_if", "[space]", float, double)
{
    metric::Tree<int, distance<int>> tree;
    tree.insert(1);
    REQUIRE_FALSE(std::get<1>(tree.insert_if(2, 10)));
    REQUIRE(std::get<1>(tree.insert_if(15, 10)));
    REQUIRE_FALSE(std::get<1>(tree.insert_if(14, 10)));
    REQUIRE(std::get<1>(tree.insert_if(26, 10)));
}

TEST_CASE("test_insert2", "[space]")
{
    std::vector<int> data = { 7, 8, 9, 10, 11, 12, 13 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    //tree.print();
}

TEST_CASE("test_to_json", "[space]")
{
    metric::Tree<int, distance<int>> tree;
    tree.insert(1);
    auto s = tree.to_json();
    std::string json1 = "{\n\"nodes\": [\n{ \"id\":0, \"values\":1}\n],\n\"edges\": [\n]}\n";
    REQUIRE(s == json1);
    tree.insert(2);
    std::string json2 = "{\n\"nodes\": [\n{ \"id\":0, \"values\":1},\n{ \"id\":1, \"values\":2}\n],\n\"edges\": [\n{ "
                        "\"source\":0, \"target\":1, \"distance\":1}\n]}\n";
    REQUIRE(tree.to_json() == json2);
}

// TEMPLATE_TEST_CASE("test_serialize_boost_text", "[space]", float, double)
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
//     REQUIRE(tree1.check_covering());
//     REQUIRE(tree1 == tree);
// }

// TEMPLATE_TEST_CASE("test_serialize_boost_binary", "[space]", float, double)
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
//     REQUIRE(tree1.check_covering());
//     REQUIRE(tree1 == tree);
// }

/*struct Record {
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
};*/

// TEMPLATE_TEST_CASE("test_serialize_boost_record_binary", "[space]", float, double)
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
//     REQUIRE(tree1.check_covering());
//     REQUIRE(tree1 == tree);
// }

// TEMPLATE_TEST_CASE("test_serialize_boost_record_text", "[space]", float, double)
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
//     REQUIRE(tree1.check_covering());
//     REQUIRE(tree1 == tree);
// }

// TEMPLATE_TEST_CASE("test_serialize_boost_record_xml", "[space]", float, double)
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
//     REQUIRE(tree1.check_covering());
//     REQUIRE(tree1 == tree);
// }

TEST_CASE("cluster1", "[space]")
{
    std::vector<int> data = { 7, 8, 9, 10, 11, 12, 13 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    //tree.print();
    std::vector<double> distribution = { 0.1, 0.2, 0.3, 0.5 };
    std::vector<std::size_t> IDS = { 1, 2, 3 };
    std::vector<int> points = { 8, 9, 10 };

    auto result = tree.clustering(distribution, IDS, data);
    auto result2 = tree.clustering(distribution, points);
    std::vector<std::vector<std::size_t>> test_result = { {}, { 1 }, { 0 }, { 2 } };
    REQUIRE(result == test_result);
    REQUIRE(result2 == test_result);
}

TEST_CASE("cluster2", "[space]")
{
    std::vector<int> data = { 7, 8, 9, 10, 11, 12, 13 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    //tree.print();
    std::vector<double> distribution = { 0.1, 0.2, 0.3, 0.5 };
    std::vector<std::size_t> IDS = { 3 };
    std::vector<int> points = { 10 };
    auto result = tree.clustering(distribution, IDS, data);
    auto result2 = tree.clustering(distribution, points);
    std::vector<std::vector<std::size_t>> test_result = { {}, { 3 }, { 4 }, { 2 } };
    REQUIRE(result == test_result);
    REQUIRE(result2 == test_result);
    std::size_t i = 0;
/*
    for (auto& v : result) {
        std::cout << distribution[i] << " = {";
        for (auto p : v) {
            std::cout << "[" << data[p] << ":" << p << "]"
                      << ", ";
        }
        std::cout << "}" << std::endl;
        i++;
    }
*/
}

TEST_CASE("cluster3", "[space]")
{
    std::vector<int> data = { 7, 8, 9, 10, 11, 12, 13 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    //tree.print();
    std::vector<double> distribution = { 0.1, 0.2, 0.5, 0.9 };
    std::vector<std::size_t> IDS = { 3 };
    std::vector<int> points = { 10 };
    auto result = tree.clustering(distribution, IDS, data);
    auto result2 = tree.clustering(distribution, points);
    std::vector<std::vector<std::size_t>> test_result = { {}, { 3 }, { 4, 2 }, { 1, 0, 5 } };
    REQUIRE(result == test_result);
    REQUIRE(result2 == test_result);
}

TEST_CASE("cluster_exception_unsorted", "[space]")
{
    std::vector<int> data = { 7, 8, 9, 10, 11, 12, 13 };
    metric::Tree<int, distance<int>> tree;
    tree.insert(data);
    std::vector<double> distribution1 = { 0.9, 0.1, 0.2, 0.5, 0.9 };
    std::vector<double> distribution2 = { 0.1, 0.2, 0.5, 0.9 };
    std::vector<double> distribution3 = { 0.1, 0.2, 0.5, 0.9, 1, 5 };
    std::vector<std::size_t> IDS = { 3 };
    std::vector<int> points = { 10 };
    REQUIRE_THROWS_AS(tree.clustering(distribution1, IDS, data), metric::unsorted_distribution_exception);
    REQUIRE_THROWS_AS(tree.clustering(distribution1, points), metric::unsorted_distribution_exception);
    REQUIRE_THROWS_AS(tree.clustering(distribution3, IDS, data), metric::bad_distribution_exception);
    REQUIRE_THROWS_AS(tree.clustering(distribution3, points), metric::bad_distribution_exception);
    REQUIRE_NOTHROW(tree.clustering(distribution2, IDS, data));
    REQUIRE_NOTHROW(tree.clustering(distribution2, points));
}

TEST_CASE("tree_element_access", "[space]")
{
    metric::Tree<int, distance<int>> tree;
    std::vector<int> data = {1,2,3,4,5,6,7,8,9,10};
    tree.insert(data);
    for(std::size_t i = 0; i < data.size(); i++) {
        REQUIRE(tree[i] == data[i]);
    }
}


TEMPLATE_TEST_CASE("tree_distance_by_id", "[space]", float, double)
{
	std::vector<TestType> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	metric::Tree<TestType, distance<TestType, TestType>> tree;
	tree.insert(data);
/*
    for(std::size_t id = 0; id < 10; id++)  {
        std::cout << id << " -> " << tree[id] << std::endl;
    }
*/
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
	for (std::size_t i = 0; i < 10; ++i) {
		REQUIRE(tree.distance_by_id(i, i) == 0);
	}
	//parent - child
	REQUIRE(tree.distance_by_id(5, 2) == 3);
	REQUIRE(tree.distance_by_id(2, 5) == 3);

	// same level, one parent
	REQUIRE(tree.distance_by_id(0, 3) == Approx((1.0 + 2.0) / 2));
	REQUIRE(tree.distance_by_id(3, 0) == Approx((1.0 + 2.0) / 2));

	// different levels, one subtree
	REQUIRE(tree.distance_by_id(1, 3) == Approx((1.0 + 2.0 + 1.0) / 3));
	REQUIRE(tree.distance_by_id(3, 1) == Approx((1.0 + 2.0 + 1.0) / 3));

	//same levels, different subtree
	REQUIRE(tree.distance_by_id(0, 7) == Approx((1. + 1. + 2. + 3.) / 4));
	REQUIRE(tree.distance_by_id(7, 0) == Approx((1. + 1. + 2. + 3.) / 4));

	//different levels, different subtree
	REQUIRE(tree.distance_by_id(1, 7) == Approx((1. + 1. + 1. + 2. + 3.) / 5));
	REQUIRE(tree.distance_by_id(7, 1) == Approx((1. + 1. + 1. + 2. + 3.) / 5));

	REQUIRE_THROWS_AS(tree.distance_by_id(100, 0), std::runtime_error);
	REQUIRE_THROWS_AS(tree.distance_by_id(1, 100), std::runtime_error);
	REQUIRE_THROWS_AS(tree.distance_by_id(100, 100), std::runtime_error);
}

TEMPLATE_TEST_CASE("tree_distance_by_value", "[space]", float, double)
{
    std::vector<TestType> data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    metric::Tree<TestType, distance<TestType, TestType>> tree;
    tree.insert(data);

    // same value
    for(std::size_t i = 0; i < 10; ++i) {
        REQUIRE(tree.distance(data[i],data[i]) == 0);
    }
    //parent - child
    REQUIRE(tree.distance(data[5], data[2]) == 3);
    REQUIRE(tree.distance(data[2], data[5]) == 3);
    REQUIRE(tree.distance(6.2, 3.3) == 3);
    REQUIRE(tree.distance(3.3, 6.2) == 3);

    // same level, one parent
    REQUIRE(tree.distance(data[0], data[3]) == Approx((1.0 + 2.0) / 2));
    REQUIRE(tree.distance(data[3], data[0]) == Approx((1.0 + 2.0)/2));
    REQUIRE(tree.distance(1.3, 4.2) == Approx((1.0 + 2.0) / 2));
    REQUIRE(tree.distance(4.2, 1.3) == Approx((1.0 + 2.0) / 2));

    // different levels, one subtree
    REQUIRE(tree.distance(2.2, 3.9) == Approx((1.0 + 2.0 + 1.0) / 3));
    REQUIRE(tree.distance(3.9, 2.2) == Approx((1.0 + 2.0 + 1.0) / 3));

    //same levels, different subtree
    REQUIRE(tree.distance(0.1, 8.2) == Approx((1. + 1. + 2. + 3.) / 4));
    REQUIRE(tree.distance(8.2, 0.7) == Approx((1. + 1. + 2. + 3.) / 4));

    //different levels, different subtree
    REQUIRE(tree.distance(1.9, 7.9) == Approx((1. + 1. + 1. + 2. + 3.)/5));
    REQUIRE(tree.distance(7.9, 1.9) == Approx((1. + 1. + 1. + 2. + 3.)/5));
}

TEMPLATE_TEST_CASE("tree_to_distance_matrix", "[space]", float, double)
{
    std::vector<TestType> data = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    metric::Tree<TestType, distance<TestType, TestType>> tree;
    tree.insert(data);
    auto m = tree.matrix();
    distance<TestType, TestType> dist;
    for(std::size_t i = 0; i < m.rows(); i++) {
        for (std::size_t j = 0; j < m.columns(); j++) {
            if( i < j) {
                REQUIRE(m(i,j) == dist(data[i], data[j]));
            } else {
                REQUIRE(m(j, i) == dist(data[i], data[j]));
            }
        }
    }
}