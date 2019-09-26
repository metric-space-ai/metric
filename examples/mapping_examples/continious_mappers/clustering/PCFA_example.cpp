/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <iostream>
#ifndef M_PI
// MSVC does not define M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../../modules/mapping.hpp"

#include "../../modules/utils/visualizer.hpp"

#include "../../modules/transform/discrete_cosine.hpp"

#include "assets/helpers.cpp"



template <typename MatrixType1, typename MatrixType2>
double mean_square_error(MatrixType1 M1, MatrixType2 M2)
{
	assert(M1.columns() == M2.columns() && M1.rows() == M2.rows());
	double overall_sum = 0;
	double column_sum;
	size_t r, c;
	for (c = 0; c < M1.columns(); c++)
	{
		column_sum = 0;
		for (r = 0; r < M1.rows(); r++)
			column_sum += pow(M1(r, c) - M2(r, c), 2);
		overall_sum += sqrt(column_sum / M1.rows());
	}
	return overall_sum / M1.columns();
}




int main()
{
	std::cout << "PCFAnet example have started" << std::endl;
	std::cout << '\n';


	// simple data
    std::cout << "\n";
    std::cout << "\n";
	std::cout << "simple data" << std::endl;
	std::cout << '\n';
	
    using recType = std::vector<float>;

    recType d0_blaze {0, 1, 2};
    recType d1_blaze {0, 1, 3};
    std::vector<recType> d_train = {d0_blaze, d1_blaze};

    auto pcfa = metric::PCFA<recType, void>(d_train, 2);

    recType d2_blaze {0, 1, 4};
    recType d3_blaze {0, 2, 2};
    std::vector<recType> d_test = {d0_blaze, d2_blaze, d3_blaze};

    auto d_compressed = pcfa.encode(d_test);

    std::cout << "compressed:\n";
    for (size_t i = 0; i<d_compressed.size(); i++) {
        for (size_t j = 0; j<d_compressed[i].size(); j++) {
            std::cout << d_compressed[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    auto d_restored = pcfa.decode(d_compressed);

    std::cout << "restored:\n";
    for (size_t i = 0; i<d_restored.size(); i++) {
        for (size_t j = 0; j<d_restored[i].size(); j++) {
            std::cout << d_restored[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    auto d_eigenmodes = pcfa.eigenmodes();

    std::cout << "eigenmodes:\n";
    for (size_t i = 0; i<d_eigenmodes.size(); i++) {
        for (size_t j = 0; j<d_eigenmodes[i].size(); j++) {
            std::cout << d_eigenmodes[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";


	//

	bool visualize = false;
	   

	// sine dataset

    std::cout << "\n";
    std::cout << "\n";
	std::cout << "sine dataset" << std::endl;
	std::cout << '\n';

	visualize = false;

	size_t n_freq_steps = 10;
	size_t n_slices_per_step = 100;
	size_t waveform_length = 64; //512; //64; // 100;

	blaze::DynamicMatrix<double, blaze::columnMajor>  SlicesSine(waveform_length, n_freq_steps*n_slices_per_step, 0.0);
	blaze::DynamicMatrix<double, blaze::columnMajor>  TargetSine(1, n_freq_steps*n_slices_per_step, 0.0);
	blaze::DynamicMatrix<double, blaze::columnMajor>  TestSlicesSine(waveform_length, n_freq_steps, 0.0);
	blaze::DynamicMatrix<double, blaze::columnMajor>  TestTargetSine(1, n_freq_steps, 0.0);

	double frequenz; // based on original test case code
	double phase = 0;
	double delta_T = 0.05;

	// sine generator
	size_t idx = 0;
	for (size_t ii = 1; ii <= n_freq_steps; ii++) // frequency change steps
	{
		frequenz = double(ii) / double(n_freq_steps);
		for (size_t i = 0; i < n_slices_per_step; ++i) // slices with same freq and random phases (within each freq step)
		{
			phase = (double)rand() / RAND_MAX * 0.9 + 0.1;
			TargetSine(0, idx) = frequenz; //-0.5; // works for positive values without offset
			for (size_t t = 0; t < waveform_length; t++) // draw waveform: 100 points in each slice
			{
				SlicesSine(t, idx) = sin(2 * M_PI * (frequenz * double(t) * delta_T + phase));
			}
			idx++;
		}
	}

	idx = 0;
	for (size_t i = 1; i <= n_freq_steps; i++) // frequency steps
	{
		frequenz = double(i) / double(n_freq_steps);
		phase = 0; //(double)rand()/RAND_MAX; // 0;
		TestTargetSine(0, idx) = frequenz; //-0.5;
		for (size_t t = 0; t < waveform_length; t++) // draw waveform: 100 points in each slice
		{
			TestSlicesSine(t, idx) = sin(2 * M_PI * (frequenz * double(t) * delta_T + phase));
		}
		idx++;
	}

	//

	// minimal direct linear mapping

	visualize = true;
	
    if (visualize) {
        mat2bmp::blaze2bmp(SlicesSine, "SlicesSine.bmp");
        mat2bmp::blaze2bmp(TestSlicesSine, "TestSlicesSine.bmp");
        blaze_dm_to_csv(blaze::DynamicMatrix<double, blaze::rowMajor>(SlicesSine), "training_dataset.csv");
        blaze_dm_to_csv(blaze::DynamicMatrix<double, blaze::rowMajor>(TestSlicesSine), "test_data.csv");
    }

    auto direct_sine = metric::PCFA_col_factory(SlicesSine, 8);  // factory deduces type

    if (visualize) {
        auto avg = direct_sine.average();
        mat2bmp::blaze2bmp(avg, "averages.bmp");
        blaze_dm_to_csv(avg, "averages.csv");
    }

    auto direct_compressed_sine = direct_sine.encode(TestSlicesSine);

    if (visualize) {
        mat2bmp::blaze2bmp_norm(direct_compressed_sine, "compressed.bmp");
        blaze_dm_to_csv(direct_compressed_sine, "compressed.csv");
    }

    auto direct_restored_sine = direct_sine.decode(direct_compressed_sine);

    if (visualize) {
        mat2bmp::blaze2bmp(direct_restored_sine, "restored.bmp");
        blaze_dm_to_csv(direct_restored_sine, "restored.csv");
    }

    std::cout << "avg error: " << mean_square_error(direct_restored_sine, TestSlicesSine) << "\n";
    std::cout << "compare visually restored.bmp to TestSliceSine.bmp\n";
    std::cout << "\n";

    if (visualize) {
        auto Eigenmodes = direct_sine.eigenmodes();
        blaze_dm_to_csv(Eigenmodes, "eigenmodes.csv");
        mat2bmp::blaze2bmp(Eigenmodes, "eigenmodes.bmp");
    }

	//

    // with DCT
	
    std::cout << "\n";
    std::cout << "\n";
	std::cout << "using DCT" << std::endl;
	std::cout << '\n';

    // turning data to frequence domain: enable to run DirectMapping in frequences

    visualize = true;

    if (visualize) {
        mat2bmp::blaze2bmp(SlicesSine, "SlicesSine_original.bmp");
        mat2bmp::blaze2bmp(TestSlicesSine, "TestSlicesSine_original.bmp");
    }

    blaze::DynamicMatrix<double> TestSlicesSineOriginal = TestSlicesSine;  // saved for computation of error

    // apply DCT to input
    metric::apply_DCT(SlicesSine);
    metric::apply_DCT(TestSlicesSine);

    blaze::DynamicMatrix<double> TestSliceSine_DCT_restored = TestSlicesSine;

    metric::apply_DCT(TestSliceSine_DCT_restored, true);
    if (visualize) {
        mat2bmp::blaze2bmp(TestSliceSine_DCT_restored, "TestSlicesSine_DCT_restored.bmp");
    }

    // direct linear mapping on spectrum

    visualize = false;

    if (visualize) {
        mat2bmp::blaze2bmp(SlicesSine, "SlicesSine_DCT.bmp");
        mat2bmp::blaze2bmp(TestSlicesSine, "TestSlicesSine_DCT.bmp");
    }

    // auto direct_sine_DCT = metric::PCFA<double>(SlicesSine, 8);
    auto direct_sine_DCT = metric::PCFA_col_factory(SlicesSine, 8);

    auto direct_compressed_sine_DCT = direct_sine_DCT.encode(TestSlicesSine);

    if (visualize)
        mat2bmp::blaze2bmp_norm(direct_compressed_sine_DCT, "compressed_DCT.bmp");

    auto direct_restored_sine_DCT = direct_sine_DCT.decode(direct_compressed_sine_DCT);

    if (visualize)
        mat2bmp::blaze2bmp(direct_restored_sine_DCT, "restored_DCT.bmp");

    // convert back to time domain: enable if DCT is applied

    visualize = true;

    metric::apply_DCT(direct_restored_sine_DCT, true);
    if (visualize) {
        mat2bmp::blaze2bmp(direct_restored_sine_DCT, "restored_unDCT.bmp");
    }

    std::cout << "with DCT: avg error: " << mean_square_error(direct_restored_sine_DCT, TestSlicesSineOriginal) << "\n";
    std::cout << "compare visually restored_unDCT.bmp to TestSliceSine_original.bmp\n";

    std::cout << "\n";

	//

	// Using PCFA_factory

	//std::cout << "using factory" << std::endl;
	//std::cout << '\n';

 //   using V = float; // double;

 //   size_t n_features = 8;

 //   auto all_data = read_csv_blaze<V>("assets/PtAll_AllGrooves_energy_5.csv", ","); // all parts  all unmixed channels
 //   blaze::DynamicMatrix<V> training_dataset = submatrix(all_data, 0, 1, all_data.rows(), all_data.columns()-2);

 //   blaze::DynamicMatrix<V> test_data = read_csv_blaze<V>("assets/test_data_input.csv", ",");

 //   mat2bmp::blaze2bmp_norm(training_dataset, "training_dataset.bmp");
 //   mat2bmp::blaze2bmp_norm(test_data, "test_data.bmp");
 //   blaze_dm_to_csv(training_dataset, "training_dataset.csv");
 //   blaze_dm_to_csv(test_data, "test_data.csv");

 //   auto model = metric::PCFA_factory(training_dataset, n_features); // dataset, compressed_code_length

 //   auto avg = model.average();
 //   mat2bmp::blaze2bmp_norm(avg, "averages.bmp");
 //   blaze_dm_to_csv(avg, "averages.csv");

 //   auto compressed = model.encode(test_data);

 //   mat2bmp::blaze2bmp_norm(compressed, "compressed.bmp");
 //   blaze_dm_to_csv(compressed, "compressed.csv");

 //   auto restored = model.decode(compressed);

 //   mat2bmp::blaze2bmp_norm(restored, "restored.bmp");
 //   blaze_dm_to_csv(restored, "restored.csv");


 //   // also making feature output for the training dataset

 //   auto all_features = model.encode(training_dataset);

 //   mat2bmp::blaze2bmp_norm(all_features, "all_features.bmp");
 //   blaze_dm_to_csv(all_features, "all_features.csv");


 //   // view contribution of each feature

 //   auto I = blaze::IdentityMatrix<V>(n_features);

 //   for (size_t feature_idx=0; feature_idx<n_features; ++feature_idx) {
 //       blaze::DynamicMatrix<V> unit_feature = submatrix(I, 0, feature_idx, I.rows(), 1);
 //       auto unit_waveform = model.decode(unit_feature, false);
 //       mat2bmp::blaze2bmp_norm(unit_waveform, "unit_waveform_" + std::to_string(feature_idx) + ".bmp");
 //       blaze_dm_to_csv(unit_waveform, "unit_waveform_" + std::to_string(feature_idx) + ".csv");
 //   }



	return 0;

}
