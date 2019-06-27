/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <iostream>
#include "../details/esn.hpp"

#include "utils/visualizer/visualizer.hpp"

int main()
{

    bool visualize = false;


    //*
    // run ESN on small dataset

    visualize = true;

    blaze::DynamicMatrix<double, blaze::rowMajor>  SlicesR {
        {1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   },
        {0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   },
        {0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.5 , 0.25, 0   , 0   , 0   , 0   , 0   , 0   },
        {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.25, 0   , 0   , 0   },
        {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75},
        {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25}
    };

    blaze::DynamicMatrix<double, blaze::rowMajor> TargetR {
        {-0.45, -0.4, -0.35, -0.3, -0.25, -0.2, -0.15, -0.1, -0.05, 0   , 0.05, 0.1, 0.15 , 0.2 , 0.25 , 0.3 , 0.35 , 0.4 },
        {0.5 , 0.25 , 0  , 0.25, 0.5 , 0.25 , 0  , 0.25, 0.5 , 0.25 , 0  , 0.25, 0.5 , 0.25 , 0  , 0.25, 0.5 , 0.25},
    };
    // first line (position of peak) is easy to predict, second is much harder. ESN predicts it better in no-echo mode

    blaze::DynamicMatrix<double, blaze::rowMajor>  SlicesTestR {
        {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25},
        {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75},
        {0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0.25, 0.5 , 0.75, 1   , 0.75, 0.25, 0   , 0   , 0   },
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

//    auto esn = esn::ESN(500, 4, 0, 1, 0, 0.9); // no echo, predicts better in this case because on no dependencies along time in it
    auto esn = esn::ESN(500, 4, 0.99, 0.5, 5, 0.9); // echo
    // ctor input: w_size=500, w_connections=10, w_sr=0.6, alpha=0.5, washout=1, beta=0.5
    esn.train(SlicesR, TargetR);

    auto prediction = esn.predict(SlicesTestR);

    std::cout << "ESN small test prediction:\n" << prediction << "\n";
    // expected horizintally mirrored Terget_R with some reverberation and delay

    if (visualize)
        mat2bmp::blaze2bmp(prediction, "ESN_prediction.bmp");
    //*/




    //*
    // run ESN on sine dataset

    visualize = true; // set false in order to prevent from overwriting images from ESN internals of the previous launch

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
        frequenz = double(ii)/double(n_freq_steps);
        for (size_t i = 0; i < n_slices_per_step; ++i) // slices with same freq and random phases (within each freq step)
        {
            phase = (double)rand()/RAND_MAX;
            phase = phase * 0.9 + 0.1; // never appeared in test dataset
            //std::cout << "phase = " << phase << ", freq =  " << frequenz << "\n";
            TargetSine(0, idx) = frequenz; //-0.5; // works for positive values without offset
            for (size_t t = 0; t < waveform_length; t++) // draw waveform: 100 points in each slice
            {
                SlicesSine(t, idx) = sin(2 * M_PI * (frequenz * double(t) * delta_T + phase));
            }
            //std::cout << idx << " " << phase << " " << frequenz << "\n";
            idx++;
        }
    }

    idx = 0;
    for (size_t i = 1; i <= n_freq_steps; i++) // frequency steps
    {
        frequenz = double(i)/double(n_freq_steps);
        phase = 0; //(double)rand()/RAND_MAX;
        TestTargetSine(0, idx) = frequenz; //-0.5;
        for (size_t t = 0; t < waveform_length; t++) // draw waveform: 100 points in each slice
        {
            TestSlicesSine(t, idx) = sin(2 * M_PI * (frequenz * double(t) * delta_T + phase));
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


    auto esn_sine = esn::ESN(500, 10, 0, 1, 0, 0.9); // reservoir disabled: w_sr=0, alpha=1, washout=0
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
