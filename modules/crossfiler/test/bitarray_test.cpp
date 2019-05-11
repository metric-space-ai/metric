// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE BitArrayTest
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>
#include "detail/bitarray.hpp"

BOOST_AUTO_TEST_CASE (constructor_test) {
  BitArray ba;
  BOOST_CHECK_EQUAL(ba.size(),(unsigned long)1);
}

BOOST_AUTO_TEST_CASE(resize_test) {
  BitArray ba;
  ba.resize(2);
  BOOST_CHECK_EQUAL(ba.get_index_size(),(unsigned long)2);
}

BOOST_AUTO_TEST_CASE(add_row_to_empty_test) {
  BitArray ba;
  auto r = ba.add_row();

  auto newMask = ba.get_mask(std::get<0>(r));
  int bitIndex = std::get<1>(r);
  std::cout << std::get<0>(r) << ',' << std::get<1>(r) << ',' << newMask << std::endl;
  
  auto bs = std::bitset<8>(newMask);

  BOOST_CHECK_EQUAL(bs[bitIndex],1);
  
}

BOOST_AUTO_TEST_CASE(add_row_test) {
  BitArray ba;
  ba.add_row();
  BOOST_CHECK_EQUAL(std::bitset<8>(ba.get_mask(0))[0],1);
  ba.add_row();
  BOOST_CHECK_EQUAL(std::bitset<8>(ba.get_mask(0))[1],1);
}
