/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <iostream>
#include <chrono>

#include "../../3rdparty/blaze/Blaze.h"
#include "../../3rdparty/Eigen/Dense"
//#if __has_include(<armadillo>)
//    #include <armadillo>
//    #define ARMA_EXISTS
//#endif // linker parameter: -larmadillo

#include "../../modules/distance.hpp"

int main()
{
	std::cout << "Standart Distances example have started" << std::endl;
	std::cout << "" << std::endl;

	/*** here are some data records ***/

    std::vector<double> stlv0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
    std::vector<double> stlv1 = { 1, 1, 1, 1, 1, 2, 3, 4 };

    blaze::DynamicVector<double> blazev0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
    blaze::DynamicVector<double> blazev1 = { 1, 1, 1, 1, 1, 2, 3, 4 };

    blaze::CompressedMatrix<double> blazem0 = { {0, 1, 1, 1}, {1, 1, 2, 3} };
    blaze::CompressedMatrix<double> blazem1 = { {1, 1, 1, 1}, {1, 2, 3, 4} };

    auto eigenv0 = Eigen::Array<double, 1, Eigen::Dynamic>(8);
    auto eigenv1 = Eigen::Array<double, 1, Eigen::Dynamic>(8);
    eigenv0 << 0, 1, 1, 1, 1, 1, 2, 3;
    eigenv1 << 1, 1, 1, 1, 1, 2, 3, 4;

#ifdef ARMA_EXISTS
    auto armav0 = arma::Row<double>();
    auto armav1 = arma::Row<double>();
    armav0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
    armav1 = { 1, 1, 1, 1, 1, 2, 3, 4 };
#endif


	/******************** examples for Euclidean (L2) Metric **************************/
    {
        std::cout << "Euclidean (L2) Metric in STL vectors" << std::endl;
        metric::Euclidean<std::vector<double>> EuclideanL2Distance;
        auto startTime = std::chrono::steady_clock::now();
        auto result = EuclideanL2Distance(stlv0, stlv1);
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000 << " ms)" << std::endl;
        std::cout << "" << std::endl;
        // out:
        // Euclidean (L2) Metric
        // result: 2 (Time = 0.018 ms)
    }
    {
        std::cout << "Euclidean (L2) Metric in Blaze vectors" << std::endl;
        metric::Euclidean<blaze::DynamicVector<double>> EuclideanL2Distance;
        auto startTime = std::chrono::steady_clock::now();
        auto result = EuclideanL2Distance(blazev0, blazev1);
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000 << " ms)" << std::endl;
        std::cout << "" << std::endl;
        // out:
        // Euclidean (L2) Metric
        // result: 2 (Time = 0.003 ms)
    }
    {
        std::cout << "Euclidean (L2) Metric in Blaze matrices" << std::endl;
        metric::Euclidean<blaze::CompressedMatrix<double>> EuclideanL2Distance;
        auto startTime = std::chrono::steady_clock::now();
        auto result = EuclideanL2Distance(blazem0, blazem1);
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000 << " ms)" << std::endl;
        std::cout << "" << std::endl;
        // out:
        // Euclidean (L2) Metric
        // result: 2 (Time = 0.003 ms)
    }
    {
        std::cout << "Euclidean (L2) Metric in Eigen vectors" << std::endl;
        metric::Euclidean<Eigen::Array<double, 1, Eigen::Dynamic>> EuclideanL2Distance;
        auto startTime = std::chrono::steady_clock::now();
        auto result = EuclideanL2Distance(eigenv0, eigenv1);
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000 << " ms)" << std::endl;
        std::cout << "" << std::endl;
        // out:
        // Euclidean (L2) Metric
        // result: 2 (Time = 0.009 ms)
    }
#ifdef ARMA_EXISTS
    {
        std::cout << "Euclidean (L2) Metric in Armadillo vectors" << std::endl;
        metric::Euclidean<arma::Row<double>> EuclideanL2Distance;
        auto startTime = std::chrono::steady_clock::now();
        auto result = EuclideanL2Distance(armav0, armav1);
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000 << " ms)" << std::endl;
        std::cout << "" << std::endl;
        // out:
        // Euclidean (L2) Metric
        // result: 2 (Time = 0.002 ms)
    }
#endif

    /******************** examples for Euclidean thresholded Metric **************************/
    {
        std::cout << "Euclidean Thresholded Metric in STL vectors" << std::endl;
        auto metric = metric::Euclidean_thresholded<std::vector<double>>(1.5, 10);
        auto startTime = std::chrono::steady_clock::now();
        auto result = metric(stlv0, stlv1);
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000 << " ms)" << std::endl;
        std::cout << "" << std::endl;
        // out:
        // Euclidean (L2) Metric
        // result: 2 (Time = 0.018 ms)
    }
    {
        std::cout << "Euclidean Thresholded Metric in Blaze vectors" << std::endl;
        auto metric = metric::Euclidean_thresholded<blaze::DynamicVector<double>>(1.5, 10);
        auto startTime = std::chrono::steady_clock::now();
        auto result = metric(blazev0, blazev1);
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000 << " ms)" << std::endl;
        std::cout << "" << std::endl;
        // out:
        // Euclidean (L2) Metric
        // result: 2 (Time = 0.003 ms)
    }
    {
        std::cout << "Euclidean Thresholded Metric in Blaze matrices" << std::endl;
        auto metric = metric::Euclidean_thresholded<blaze::CompressedMatrix<double>>(1.5, 10);
        auto startTime = std::chrono::steady_clock::now();
        auto result = metric(blazem0, blazem1);
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000 << " ms)" << std::endl;
        std::cout << "" << std::endl;
        // out:
        // Euclidean (L2) Metric
        // result: 2 (Time = 0.003 ms)
    }
    {
        std::cout << "Euclidean Thresholded Metric in Eigen vectors" << std::endl;
        auto metric = metric::Euclidean_thresholded<Eigen::Array<double, 1, Eigen::Dynamic>>(1.5, 10);
        auto startTime = std::chrono::steady_clock::now();
        auto result = metric(eigenv0, eigenv1);
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000 << " ms)" << std::endl;
        std::cout << "" << std::endl;
        // out:
        // Euclidean (L2) Metric
        // result: 2 (Time = 0.009 ms)
    }
#ifdef ARMA_EXISTS
    {
        std::cout << "Euclidean Thresholded Metric in Armadillo vectors" << std::endl;
        auto metric = metric::Euclidean_thresholded<arma::Row<double>>(1.5, 10);
        auto startTime = std::chrono::steady_clock::now();
        auto result = metric(armav0, armav1);
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "result: " << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000 << " ms)" << std::endl;
        std::cout << "" << std::endl;
        // out:
        // Euclidean (L2) Metric
        // result: 2 (Time = 0.002 ms)
    }
#endif


    // test code to show how type detection works, TODO remove
    {
        auto metric = metric::TestMetric<Eigen::Array<double, 1, Eigen::Dynamic>>();
        //auto metric = metric::TestMetric<blaze::CompressedMatrix<double>>();
        //auto metric = metric::TestMetric<blaze::DynamicVector<double>>();
        auto result = metric(eigenv0, eigenv1);
        //auto result = metric(blazem0, blazem1);
        //auto result = metric(blazev0, blazev1);
        std::cout << "\n\ntest result: " << result << "\n";
    }


	return 0;
}
