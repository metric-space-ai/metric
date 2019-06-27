/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef ENCODER_HPP
#define ENCODER_HPP

#include "../../../3rdparty/blaze/Blaze.h"

//#include "utils/visualizer/visualizer.hpp" // TODO remove visualizer calls and this line




namespace linear_compressor
{
    

template <class BlazeMatrix>
blaze::DynamicMatrix<double> PCA(const BlazeMatrix In, int n_components, bool visualize=false);


// simple linear encoder based on PCA

class DirectMapping
{

private:
    blaze::DynamicMatrix<double> W_decode;
    blaze::DynamicMatrix<double> W_encode;
    bool visualize = false;
    std::default_random_engine rgen;

public:

    DirectMapping(bool visualize_ = false);

    void train(const blaze::DynamicMatrix<double> & Slices, size_t n_features = 1);


    blaze::DynamicMatrix<double> compress(
            const blaze::DynamicMatrix<double> & Slices
            );

    blaze::DynamicMatrix<double> decompress(
            const blaze::DynamicMatrix<double> & Codes
            );
};


} // namespace linear_compressor



#include "encoder.cpp"

#endif // ENCODER_HPP
