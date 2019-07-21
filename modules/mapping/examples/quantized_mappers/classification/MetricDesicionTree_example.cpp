/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Max Filippov
*/
#include <iostream>
#include <vector>
#include <deque>
#include <chrono>
#include <functional>
#include <variant>

#include "../../../details/classification/details/metric_dt_classifier.hpp"

#include "../../assets/helpers.cpp" // csv reader
#include "../../../../distance.hpp"


using namespace std;

template <typename T>
void vector_print(const std::vector<T> &vec)
{

	std::cout << "[";
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (i < vec.size() - 1)
		{
			std::cout << vec[i] << ", ";
		}
		else
		{
			std::cout << vec[i] << "]" << std::endl;
		}
	}
}

int main()
{
	std::cout << "we have started" << std::endl;
	std::cout << '\n';

    typedef std::variant<double, std::vector<double>, std::vector<std::vector<double>>, std::string> V; // field type
    typedef std::vector<V> Record;

    std::vector<std::vector<double>> img1 = { // needs to be larger than blur kernel size coded intarnally as 11
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    std::vector<std::vector<double>> img2 = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    std::vector<Record> selection = {
        {V((double)2),
         V(std::vector<double>({1, 2})),
         V(std::vector<double>({0, 1, 1, 1, 1, 1, 2, 3})),
         V(img1),
         V(""),
         V((double)1)
        },
        {V((double)2),
         V(std::vector<double>({1, 5})),
         V(std::vector<double>({1, 1, 1, 1, 1, 2, 3, 4})),
         V(img2),
         V("A"),
         V((double)1)
        },
        {V((double)1),
         V(std::vector<double>({4, 5})),
         V(std::vector<double>({2, 2, 2, 1, 1, 2, 0, 0})),
         V(img2),
         V("AA"),
         V((double)2)
        },
        {V((double)2),
         V(std::vector<double>({1, 2})),
         V(std::vector<double>({3, 3, 2, 2, 1, 1, 0, 0})),
         V(img1),
         V("AAA"),
         V((double)1)
        },
        {V((double)2),
         V(std::vector<double>({5})),
         V(std::vector<double>({4, 3, 2, 1, 0, 0, 0, 0})),
         V(img1),
         V("AAAA"),
         V((double)1)
        },
        {V((double)2),
         V(std::vector<double>({1, 4, 5})),
         V(std::vector<double>({4, 3, 2, 1, 0, 0, 0, 0})),
         V(img2),
         V("BAAA"),
         V((double)1)
        },
        {V((double)1),
         V(std::vector<double>({1, 2, 3, 4})),
         V(std::vector<double>({5, 3, 2, 1, 0, 0, 0, 0})),
         V(img2),
         V("BBAA"),
         V((double)3)
        },
        {V((double)1),
         V(std::vector<double>({1})),
         V(std::vector<double>({4, 6, 2, 2, 1, 1, 0, 0})),
         V(img1),
         V("BBA"),
         V((double)1)
        },
        {V((double)2),
         V(std::vector<double>({4, 5})),
         V(std::vector<double>({3, 7, 2, 1, 0, 0, 0, 0})),
         V(img2),
         V("BB"),
         V((double)1)
        },
        {V((double)2),
         V(std::vector<double>({1, 2, 4, 5})),
         V(std::vector<double>({2, 5, 1, 1, 0, 0, 1, 2})),
         V(img1),
         V("B"),
         V((double)1)
        }
    };
	   

    // vector of accessors for field 0
    auto field0accessors = [] (const Record & r) 
	{
        return std::get<double>(r[0]);
    };

    // vector of accessors for field 1
    auto field1accessors = [] (const Record & r) 
	{
        std::vector<double> v(std::get<std::vector<double>>(r[1]));
        v.resize(4);
        return v;
    };
	
    // vector of accessors for field 2
    auto field2accessors = [] (const Record & r) 
	{
        std::vector<double> v(std::get<std::vector<double>>(r[2]));
        v.resize(8);
        return v;
    };

    // vector of accessors for field 3
    auto field3accessors = [] (const Record & r) 
	{
        return std::get<std::vector<std::vector<double>>>(r[3]);
    };

    // vector of accessors for field 4
    auto field4accessors = [] (const Record & r) 
	{
        return std::get<std::string>(r[4]);
    };


    // label accessor (for single record)
    std::function<int(const Record&)> response = [](const Record & r)
    {
        return (int)std::abs(std::get<double>(r[5]));
    };

	   
    // build dimension and Dimension objects

	typedef double InternalType;
    namespace md = metric::distance;

	// features
    using a0_type = decltype(field0accessors);
    using a1_type = decltype(field1accessors);
    using a2_type = decltype(field2accessors);
    using a3_type = decltype(field3accessors);
    using a4_type = decltype(field4accessors);

    auto dim0 = metric::make_dimension(md::Euclidian<InternalType>(), field0accessors);
    auto dim1 = metric::make_dimension(md::Manhatten<InternalType>(), field1accessors);
    auto dim2 = metric::make_dimension(md::P_norm<InternalType>(), field2accessors);
    auto dim3 = metric::make_dimension(md::Euclidian_thresholded<InternalType>(), field2accessors);
    auto dim4 = metric::make_dimension(md::Cosine<InternalType>(), field2accessors);
    auto dim5 = metric::make_dimension(md::SSIM<std::vector<InternalType>>(), field3accessors);
    auto dim6 = metric::make_dimension(md::TWED<InternalType>(), field2accessors);
    auto dim7 = metric::make_dimension(md::Edit<char>(), field4accessors);
    auto dim10 = metric::make_dimension(md::EMD<InternalType>(8,8), field2accessors);

    typedef  std::variant<
        metric::Dimension< metric::distance::Euclidian<InternalType>, a0_type>,
        metric::Dimension< metric::distance::Manhatten<InternalType>, a1_type>,
        metric::Dimension< metric::distance::P_norm<InternalType>, a2_type>,
        metric::Dimension< metric::distance::Euclidian_thresholded<InternalType>, a2_type>,
        metric::Dimension< metric::distance::Cosine<InternalType>, a2_type>,
        metric::Dimension< metric::distance::SSIM<std::vector<InternalType>> ,a3_type>,
        metric::Dimension< metric::distance::TWED<InternalType>, a2_type>,
        metric::Dimension< metric::distance::EMD<InternalType>, a2_type>, // matrix C is temporary created inside functor
        metric::Dimension< metric::distance::Edit<std::string::value_type>, a4_type>
        > VariantType;
	
    std::vector<VariantType> dims = {dim0, dim1, dim2, dim3, dim4, dim5, dim6, dim7, dim10};

	std::vector<Record> test_sample = { selection[0], selection[2], selection[6] };

	std::vector<int> prediction;
	auto startTime = std::chrono::steady_clock::now();
	auto endTime = std::chrono::steady_clock::now();

	std::cout << "Metric Desicion Tree: " << std::endl;
	startTime = std::chrono::steady_clock::now();
	auto model = metric::classification::MetricDT<Record>();
	std::cout << "Metric Desicion Tree training... " << std::endl;
	model.train(selection, dims, response);
	endTime = std::chrono::steady_clock::now();
	std::cout << "\n";
	std::cout << "Metric Desicion Tree trained (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count()) / 1000000 << " s)" << std::endl;

	model.predict(test_sample, dims, prediction);
	std::cout << "\n";
	std::cout << "Metric Desicion Tree prediction: " << std::endl;
	vector_print(prediction);

	std::cout << "\n";

	   

	std::cout << "Distances separately: " << std::endl;
	

    // test Edit separately

    metric::distance::Edit<char> edit_functor;
    auto edit_dist = edit_functor("AAAB", "AAC");

    std::cout << "\nEdit distance: " << edit_dist << "\n";


	// test SSIM separately

    metric::distance::SSIM<std::vector<double>> SSIM_functor;
    auto SSIM_dist = SSIM_functor(img1, img2);

    std::cout << "\nSSIM distance: " << SSIM_dist << "\n";
	
	

    return 0;
}

