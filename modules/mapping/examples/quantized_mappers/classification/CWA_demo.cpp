
#include <vector>
#include <iostream>

#include "../details/classification/details/correlation_weighted_accuracy.hpp"

int main()
{

    std::vector<int> g1 = {3, 2, 2, 3, 1, 1}; // Known groups
    std::vector<int> g2 = {4, 2, 2, 2, 1, 1}; // Predicted groups

    double cwa = metric::classification::correlation_weighted_accuracy(g1,g2);

    std::cout << cwa << std::endl;

    return 0;
}
