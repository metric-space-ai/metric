#include "../../../../modules/mapping.hpp"

#include <iostream>
#include "../../../../3rdparty/blaze/Blaze.h"
#include "../../../../3rdparty/Eigen/Dense"
#include <armadillo>  // linker parameter: -larmadillo




int main() {

    using Record = std::deque<double>;
    //using Record = blaze::DynamicVector<double>;
    //using Record = Eigen::Array<double, 1, Eigen::Dynamic>; // TODO add support
    //using Record = arma::Row<double>; // maybe it's better to CHANGE the whole type detection scheme..

    //* // Non-Eigen init
    std::vector<Record> data = { {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12} };
    std::vector<Record> target = { {0, 1}, {1, 2}, {2, 3} };
    std::vector<Record> test = { {5, 6, 7, 8}, {1, 2, 3, 4} };
    // */

    /* // Eigen init
    Record r1 (4), r2 (4), r3 (4), t1 (2), t2 (2), t3 (2);
    r1 << 1, 2, 3, 4;
    r2 << 5, 6, 7, 8;
    r3 << 9, 10, 11, 12;
    t1 << 0, 1;
    t2 << 1, 2;
    t3 << 2, 3;
    std::vector<Record> data {r1, r2, r3};
    std::vector<Record> target {t1, t2, t3};
    std::vector<Record> test {r2, r1};
    // */

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


