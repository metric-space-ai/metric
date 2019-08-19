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

#include "../../utils/visualizer.hpp"

#include "../../modules/transform/discrete_cosine.hpp"



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

	bool visualize = false;



	//*
	// sine dataset

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
			//std::cout << "phase = " << phase << ", freq =  " << frequenz << "\n";
			TargetSine(0, idx) = frequenz; //-0.5; // works for positive values without offset
			for (size_t t = 0; t < waveform_length; t++) // draw waveform: 100 points in each slice
			{
				SlicesSine(t, idx) = sin(2 * M_PI * (frequenz * double(t) * delta_T + phase));
			}
			//            std::cout << idx << " " << phase << " " << frequenz << "\n";
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
		//        std::cout << idx << " " << phase << " " << frequenz << "\n";
		idx++;
	}

	// */



	//* // minimal direct linear mapping

	visualize = true;

	if (visualize)
	{
		mat2bmp::blaze2bmp(SlicesSine, "SlicesSine.bmp");
		mat2bmp::blaze2bmp(TestSlicesSine, "TestSlicesSine.bmp");
	}

	auto direct_sine = metric::PCAnet(true);
	direct_sine.train(SlicesSine, 8); // dataset, compressed_code_length

	//std::cout << "trained direct mapping\n";

	auto direct_compressed_sine = direct_sine.compress(TestSlicesSine);

	if (visualize)
		mat2bmp::blaze2bmp_norm(direct_compressed_sine, "compressed.bmp");

	//std::cout << "Compressed:\n" << direct_compressed_sine << "\n";

	auto direct_restored_sine = direct_sine.decompress(direct_compressed_sine);

	if (visualize)
		mat2bmp::blaze2bmp(direct_restored_sine, "restored.bmp");

	//std::cout << "Restored:\n" << direct_restored_sine << "\n";

	std::cout << "avg error: " << mean_square_error(direct_restored_sine, TestSlicesSine) << "\n";
	std::cout << "compare visually restored.bmp to TestSliceSine.bmp\n";


	//*/




	//* // with DCT

	// turning data to frequence domain: enable to run DirectMapping in frequences

	visualize = true;

	if (visualize)
	{
		mat2bmp::blaze2bmp(SlicesSine, "SlicesSine_original.bmp");
		mat2bmp::blaze2bmp(TestSlicesSine, "TestSlicesSine_original.bmp");
	}

	blaze::DynamicMatrix<double> TestSlicesSineOriginal = TestSlicesSine; // saved for computation of error

	// apply DCT to input
	dct::apply_DCT(SlicesSine);
	dct::apply_DCT(TestSlicesSine);

	blaze::DynamicMatrix<double> TestSliceSine_DCT_restored = TestSlicesSine;
	dct::apply_DCT(TestSliceSine_DCT_restored, true);
	if (visualize)
	{
		mat2bmp::blaze2bmp(TestSliceSine_DCT_restored, "TestSlicesSine_DCT_restored.bmp");
	}



	// direct linear mapping on spectrum

	visualize = false;

	if (visualize)
	{
		mat2bmp::blaze2bmp(SlicesSine, "SlicesSine_DCT.bmp");
		mat2bmp::blaze2bmp(TestSlicesSine, "TestSlicesSine_DCT.bmp");
	}

	auto direct_sine_DCT = metric::PCAnet(visualize);
	direct_sine_DCT.train(SlicesSine, 8); // dataset, compressed_code_length

	//std::cout << "trained direct mapping after DCT applied\n";

	auto direct_compressed_sine_DCT = direct_sine_DCT.compress(TestSlicesSine);

	if (visualize)
		mat2bmp::blaze2bmp_norm(direct_compressed_sine_DCT, "compressed_DCT.bmp");

	//std::cout << "Compressed:\n" << direct_compressed_sine_DCT << "\n";

	auto direct_restored_sine_DCT = direct_sine_DCT.decompress(direct_compressed_sine_DCT);

	if (visualize)
		mat2bmp::blaze2bmp(direct_restored_sine_DCT, "restored_DCT.bmp");

	//std::cout << "Restored:\n" << direct_restored_sine_DCT << "\n";




	// convert back to time domain: enable if DCT is applied

	visualize = true;

	dct::apply_DCT(direct_restored_sine_DCT, true);
	if (visualize)
	{
		mat2bmp::blaze2bmp(direct_restored_sine_DCT, "restored_unDCT.bmp");
	}

	std::cout << "\nwith DCT: avg error: " << mean_square_error(direct_restored_sine_DCT, TestSlicesSineOriginal) << "\n";
	std::cout << "compare visually restored_unDCT.bmp to TestSliceSine_original.bmp\n";
	//*/



	return 0;

}
