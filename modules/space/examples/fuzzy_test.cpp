/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov
*/

#include <cmath>
#include <functional>
#include <algorithm>
#include <random>
#include "assets/3dparty/serialize/archive.h"
#include "../metric_space.hpp"

// Make fuzzy testing

template<typename T>
struct distance {
  int operator()( const T &lhs,  const T &rhs) const {
    return std::abs(lhs - rhs);
  }
};
struct random_uniform_int {
  //  std::random_device rd;
  std::mt19937 gen;
  std::uniform_int_distribution<int> d;
  random_uniform_int(int min, int max)
      :gen(),d(min,max) {}
  int operator()()  {
    return d(gen);
  }
};
struct random_uniform_real {
  //  std::random_device rd;
  std::mt19937 gen;
  std::uniform_real_distribution<float> d;
  random_uniform_real(float min, float max)
      :gen(),d(min,max) {}
  float operator()()  {
    return d(gen);
  }
};
bool test(int array_size) {
  //    random_uniform_int dgen(std::numeric_limits<float>::min()/2,std::numeric_limits<float>::max()/2);
  random_uniform_real  dgen(-100000,100000);
  std::vector<float> data;
  data.reserve(array_size);
  for(int i = 0; i < array_size; i++) {
    data.push_back(dgen());
  }
  metric_space::Tree<float,distance<float>> tr;
  for(auto i : data) {
    tr.insert(i);
    if(!tr.check_covering()) {
      return false;
    }
  }
  std::ostringstream os;
  serialize::oarchive<std::ostringstream> oar(os);
  tr.serialize(oar);
  metric_space::Tree<float,distance<float>> tr1;
  std::istringstream is(os.str());
  serialize::iarchive<std::istringstream> iar(is);
  tr1.deserialize(iar,is);
  if(!tr1.check_covering())
    return false;
  if(!(tr1 == tr))
    return false;
  for(std::size_t i = 0; i < data.size(); i++) {
    auto root = tr.get_root();
    bool b = root->children.empty();
    tr.erase(root->data);
    if(!b) {
      if(!tr.check_covering()) {
        return false;
      }
    }
  }
  return true;
}

int main(int argc, char ** argv) {
    
  int iterations = 1;
  if(argc == 2)
    iterations = std::stoi(argv[1]);
  random_uniform_int len_gen(1,1000);
  for(int i = 0; i < iterations; i++) {
    int array_size = len_gen();
    bool result = test(array_size);
    if(!result) {
      std::cout << "Error!!! -- " << i << "," << array_size << std::endl;
    }
    std::cout << "Iteration #" << i << std::endl;
  }
  return 0;
}
