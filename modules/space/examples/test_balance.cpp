#include <cmath>
#include <functional>
#include <algorithm>
#include <iomanip>
#include "../metric_space.hpp"
template<typename T>
struct distance {
    int operator()( const T &lhs,  const T &rhs) const {
	return std::abs(lhs - rhs);
    }
};
int main() {
//    std::vector<int> data = {3,5,-10,50,1,-200,200};
//    std::vector<float> data = {-10,50,1,-200,200,250};
//    std::vector<float> data = {-10,50,1,-200,200,250,-300, 1000, -2000, std::numeric_limits<float>::max()};
    metric_space::Tree<long double,distance<long double>> tr;
    tr.insert(0);

    for(int i =0; i < 100; i++)  {
        auto root = tr.get_root();
        long double d = 0;
        long double pow = std::exp2((long double)root->level+1);
        int level = root->level;
        if(root->data > 0) {
            d = -1*(root->data + pow);
            tr.insert(d);
        } else {
            d = (root->data + pow);
            tr.insert(d);
        }
        std::cout << "insert : " << std::fixed << d << ", " << level << ", " << std::fixed << pow<< std::endl;
        tr.print();
    } 
    return 0;

}
