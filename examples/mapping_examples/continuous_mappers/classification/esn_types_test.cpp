#include "../../../../modules/mapping.hpp"

#include <iostream>
#include "../../../../3rdparty/blaze/Blaze.h"
#include "../../../../3rdparty/Eigen/Dense"
#include <armadillo>  // linker parameter: -larmadillo




int main() {

    //using Record = std::deque<double>;
    using Record = blaze::DynamicVector<double>;
    //using Record = Eigen::Array<double, 1, Eigen::Dynamic>;
    //using Record = arma::Row<double>;

    std::vector<Record> data = { {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12} };
    std::vector<Record> target = { {0, 1}, {1, 2}, {2, 3} };
    std::vector<Record> test = { {5, 6, 7, 8}, {1, 2, 3, 4} };

    auto esn = metric::ESN<metric::EmptyMetric<Record>>();
    esn.train(data, target);
    auto prediction = esn.predict(test);

    std::cout << "ESN small test prediction, data in vector of STL resords:\n";
    for (size_t i = 0; i<prediction.size(); ++i) {
        for (size_t j = 0; j<prediction[0].size(); ++j)
            std::cout << prediction[i][j] << " ";
        std::cout << "\n";
    }


    return 0;
}


