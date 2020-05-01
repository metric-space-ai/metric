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

//#include "../../../../3rdparty/Eigen/Dense" // for Eigen example
//#include <armadillo>  // for Armadillo example, linker parameter: -larmadillo

#include "../../../../modules/mapping.hpp"

#include "../../../../modules/utils/visualizer.hpp"


int main()
{

    //// long double is not supported by the current BLAS!
    //blaze::DynamicMatrix<long double> ldm = {{1, 0}, {0, 1}};
    //std::cout << blaze::eigen(ldm)<< "\n"; // this fails

	std::cout << "ESN example have started" << std::endl;
	std::cout << '\n';

    bool visualize = false;

    using value_type = double;

    //*
    // run ESN on small dataset passed in the native Blaze matrix form (data points in columns)

    visualize = true;

    blaze::DynamicMatrix<value_type, blaze::rowMajor>  SlicesR {
        {1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   },
        {0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   },
        {0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   },
        {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.5 , 0.25, 0   , 0   },
        {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75},
        {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25}
    }; // first COLUMN represents zero time moment, second represents time = 1, etc

    blaze::DynamicMatrix<value_type, blaze::rowMajor> TargetR {
        {-0.45, -0.4, -0.35, -0.3, -0.25, -0.2, -0.15, -0.1, -0.05, 0   , 0.05, 0.1, 0.15 , 0.2 , 0.25 , 0.3 , 0.35 , 0.4 },
        {0.5 , 0.25 , 0  , 0.25, 0.5 , 0.25 , 0  , 0.25, 0.5 , 0.25 , 0  , 0.25, 0.5 , 0.25 , 0  , 0.25, 0.5 , 0.25},
    };
    // first line (position of peak) is easy to predict, second is much harder. ESN predicts it better in no-echo mode

    blaze::DynamicMatrix<value_type, blaze::rowMajor>  SlicesTestR {
        {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25},
        {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75},
        {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.5 , 0.25, 0   , 0   },
        {0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   },
        {0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   },
        {1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   }
    };


    if (visualize)
    {
        mat2bmp::blaze2bmp(SlicesR, "ESN_SlicesR.bmp");
        mat2bmp::blaze2bmp(TargetR, "ESN_TargetR.bmp");
        mat2bmp::blaze2bmp(SlicesTestR, "ESN_SlicesTestR.bmp");
    }

    auto esn = metric::ESN<metric::EmptyMetric<std::vector<value_type>>>(500, 4, 0.99, 0.5, 5, 0.9); // echo
    //auto esn = metric::ESN<std::vector<value_type>, void>(500, 4, 0.99, 1, 0, 0.9); // no echo (alpha=1 and no washout)
    esn.train(SlicesR, TargetR);

    auto prediction = esn.predict(SlicesTestR);

    std::cout << "ESN small test prediction, data in matrices, input vectors in COLUMNS:\n" << prediction << "\n";
    // expected horizintally mirrored Terget_R with some reverberation and delay

    if (visualize)
        mat2bmp::blaze2bmp(prediction, "ESN_prediction.bmp");
    //*/


    //*
    // run ESN for vector<RecType> dataset, data points in RecType containers (in rows)
    // data is exactly the same as in the example above

    //using RecType = std::deque<float>;
    using RecType = std::vector<double>;
    //using RecType = blaze::DynamicVector<double>; // also supported
    //using RecType = arma::Row<double>; // now fails, TODO debug!!

    std::vector<RecType> SlicesRV {
        {   1,    0,    0,    0,    0,    0}, // time = 0
        {0.75, 0.25,    0,    0,    0,    0}, // time = 1
        {0.5 ,  0.5,    0,    0,    0,    0}, // etc
        {0.25, 0.75,    0,    0,    0,    0},
        {   0,    1,    0,    0,    0,    0},
        {   0, 0.75, 0.25,    0,    0,    0},
        {   0,  0.5,  0.5,    0,    0,    0},
        {   0, 0.25, 0.75,    0,    0,    0},
        {   0,    0,    1,    0,    0,    0},
        {   0,    0, 0.75, 0.25,    0,    0},
        {   0,    0,  0.5,  0.5,    0,    0},
        {   0,    0, 0.25, 0.75,    0,    0},
        {   0,    0,    0,    1,    0,    0},
        {   0,    0,    0, 0.75, 0.25,    0},
        {   0,    0,    0,  0.5,  0.5,    0},
        {   0,    0,    0, 0.25, 0.75,    0},
        {   0,    0,    0,    0,    1,    0},
        {   0,    0,    0,    0, 0.75, 0.25}
    };

    std::vector<RecType> TargetRV {
        {-0.45,  0.5},
        {- 0.4, 0.25},
        {-0.35,    0},
        {- 0.3, 0.25},
        {-0.25,  0.5},
        {- 0.2, 0.25},
        {-0.15,    0},
        {- 0.1, 0.25},
        {-0.05,  0.5},
        {    0, 0.25},
        { 0.05,    0},
        {  0.1, 0.25},
        { 0.15,  0.5},
        {  0.2, 0.25},
        { 0.25,    0},
        {  0.3, 0.25},
        { 0.35,  0.5},
        {  0.4, 0.25}
    };
    // first line (position of peak) is easy to predict, second is much harder. ESN predicts it better in no-echo mode

    std::vector<RecType> SlicesTestRV {
        {   0,    0,    0,    0,    0,    1},
        {   0,    0,    0,    0, 0.25, 0.75},
        {   0,    0,    0,    0,  0.5,  0.5},
        {   0,    0,    0,    0, 0.75, 0.25},
        {   0,    0,    0,    0,    1,    0},
        {   0,    0,    0, 0.25, 0.75,    0},
        {   0,    0,    0,  0.5,  0.5,    0},
        {   0,    0,    0, 0.75, 0.25,    0},
        {   0,    0,    0,    1,    0,    0},
        {   0,    0, 0.25, 0.75,    0,    0},
        {   0,    0,  0.5,  0.5,    0,    0},
        {   0,    0, 0.75, 0.25,    0,    0},
        {   0,    0,    1,    0,    0,    0},
        {   0, 0.25, 0.75,    0,    0,    0},
        {   0,  0.5,  0.5,    0,    0,    0},
        {   0, 0.75, 0.25,    0,    0,    0},
        {   0,    1,    0,    0,    0,    0},
        {0.25, 0.75,    0,    0,    0,    0}
    };

    auto esnV = metric::ESN<metric::EmptyMetric<RecType>>(500, 4, 0.99, 0.5, 5, 0.9); // echo
    //auto esnV = metric::ESN<metric::EmptyMetric<RecType>>(500, 4, 0.99, 1, 0, 0.9); // no echo (alpha=1 and no washout)
    esnV.train(SlicesRV, TargetRV);

    auto predictionV = esnV.predict(SlicesTestRV);

    std::cout << "ESN small test prediction, data in vector of RecType resords (in ROWS):\n";
    for (size_t i = 0; i<predictionV.size(); ++i) {
        for (size_t j = 0; j<predictionV[0].size(); ++j)
            std::cout << predictionV[i][j] << " ";
        std::cout << "\n";
    }
    // expected horizintally mirrored Terget_R with some reverberation and delay
    std::cout << "\n\n";


    //*/




    //*
    // run ESN on sine dataset

    visualize = true; // set false in order to prevent from overwriting images from ESN internals of the previous launch

    //using value_type = float;

    size_t n_freq_steps = 10;
    size_t n_slices_per_step = 100;
    size_t waveform_length = 64; //512; //64; // 100;

    blaze::DynamicMatrix<value_type, blaze::columnMajor>  SlicesSine(waveform_length, n_freq_steps*n_slices_per_step, 0.0);
    blaze::DynamicMatrix<value_type, blaze::columnMajor>  TargetSine(1, n_freq_steps*n_slices_per_step, 0.0);
    blaze::DynamicMatrix<value_type, blaze::columnMajor>  TestSlicesSine(waveform_length, n_freq_steps, 0.0);
    blaze::DynamicMatrix<value_type, blaze::columnMajor>  TestTargetSine(1, n_freq_steps, 0.0);

    value_type frequenz; // based on original test case code
    value_type phase = 0;
    value_type delta_T = 0.05;

    // sine generator
    size_t idx = 0;
    for (size_t ii = 1; ii <= n_freq_steps; ii++) // frequency change steps
    {
        frequenz = value_type(ii)/value_type(n_freq_steps);
        for (size_t i = 0; i < n_slices_per_step; ++i) // slices with same freq and random phases (within each freq step)
        {
            phase = (value_type)rand()/RAND_MAX;
            phase = phase * 0.9 + 0.1; // never appeared in test dataset
            //std::cout << "phase = " << phase << ", freq =  " << frequenz << "\n";
            TargetSine(0, idx) = frequenz; //-0.5; // works for positive values without offset
            for (size_t t = 0; t < waveform_length; t++) // draw waveform: 100 points in each slice
            {
                SlicesSine(t, idx) = sin(2 * M_PI * (frequenz * value_type(t) * delta_T + phase));
            }
            //std::cout << idx << " " << phase << " " << frequenz << "\n";
            idx++;
        }
    }

    idx = 0;
    for (size_t i = 1; i <= n_freq_steps; i++) // frequency steps
    {
        frequenz = value_type(i)/value_type(n_freq_steps);
        phase = 0; //(double)rand()/RAND_MAX;
        TestTargetSine(0, idx) = frequenz; //-0.5;
        for (size_t t = 0; t < waveform_length; t++) // draw waveform: 100 points in each slice
        {
            TestSlicesSine(t, idx) = sin(2 * M_PI * (frequenz * value_type(t) * delta_T + phase));
        }
        //std::cout << idx << " " << phase << " " << frequenz << "\n";
        idx++;
    }


    if (visualize)
    {
        mat2bmp::blaze2bmp(SlicesSine, "SlicesSine.bmp");
        mat2bmp::blaze2bmp(TargetSine, "TargetSine.bmp");
        mat2bmp::blaze2bmp(TestSlicesSine, "TestSlicesSine.bmp");
    }


    auto esn_sine = metric::ESN<metric::EmptyMetric<std::vector<value_type>>>(500, 10, 0, 1, 0, 0.9); // reservoir disabled: w_sr=0, alpha=1, washout=0
    // ctor input: w_size=500, w_connections=10, w_sr=0.6, alpha=0.5, washout=1, beta=0.5
    esn_sine.train(SlicesSine, TargetSine);

    auto prediction_sine = esn_sine.predict(TestSlicesSine);

    if (visualize)
        mat2bmp::blaze2bmp(prediction_sine, "PredictionSine.bmp");

    std::cout << "prediction_sine:\n" << prediction_sine << "\n";
    std::cout << "test target to compare with (never used in code):\n" << TestTargetSine << "\n";


    //*/


    return 0;

}
