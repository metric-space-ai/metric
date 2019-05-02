#include <cmath>
#include <functional>
#include <algorithm>
#include "../metric_space.hpp"
template<typename T>
struct distance {
    int operator()( const T &lhs,  const T &rhs) const {
	return std::abs(lhs - rhs);
    }
};
int main() {
//    std::vector<int> data = {3,5,-10,50,1,-200,200};
    std::vector<float> data = {-10,50,1,-200,200};
    metric_space::Tree<float,distance<float>> tr(data);
/*    for(auto i : data) {
	tr.insert(i);
	tr.print();
	std::cout << i << " :--------------------------" << std::endl;
    }*/
    tr.print();
    return 0;
    
}
