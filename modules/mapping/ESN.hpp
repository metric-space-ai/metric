/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_MAPPING_DETAILS_ESN_HPP
#define _METRIC_MAPPING_DETAILS_ESN_HPP

#include "../../3rdparty/blaze/Blaze.h"


//#include "utils/graph/graph.hpp" // for create_W only // TODO enable if considered to use graph


namespace metric {

namespace ESN_details
{
    


// ---------------------------------  math functions:

blaze::DynamicMatrix<double> get_readout_no_echo( // for ESN with disables echo
        const blaze::DynamicMatrix<double> & Slices,
        const blaze::DynamicMatrix<double> & W_in
        );

blaze::DynamicMatrix<double> get_readout( // echo mode
        const blaze::DynamicMatrix<double> & Slices,
        const blaze::DynamicMatrix<double> & W_in,
        const blaze::DynamicMatrix<double> & W, // TODO make sparse
        double alpha = 0.5,
        size_t washout = 0
        );

blaze::DynamicMatrix<double> ridge(
        const blaze::DynamicMatrix<double> & Target,
        const blaze::DynamicMatrix<double> & Readout,
        double beta = 0.5
        );


}
// -------------------------------- caller class:

class ESN {

private:
    blaze::DynamicMatrix<double> W_in;
    blaze::CompressedMatrix<double> W;
    blaze::DynamicMatrix<double> W_out = blaze::DynamicMatrix<double>(0, 0);
    bool trained = false;
    double alpha = 0.5;
    double beta = 0.5;
    size_t washout = 1;
    std::default_random_engine rgen;

    void create_W(size_t w_size, double w_connections, double w_sr);

public:

    ESN(
            size_t w_size = 500, // number of elements in reservoir
            double w_connections = 10, // number of interconnections (for each reservoir element)
            double w_sr = 0.6, // desired spectral radius of the reservoir
            double alpha_ = 0.5, // leak rate, number of slices excluded from output for washout
            size_t washout_ = 1,
            double beta_ = 0.5 // ridge solver metaparameter
            );

    void train(
            const blaze::DynamicMatrix<double> & Slices,
            const blaze::DynamicMatrix<double> & Target
            );


    blaze::DynamicMatrix<double> predict(
            const blaze::DynamicMatrix<double> & Slices
            );

}; // class ESN


} // namespace metric

#include "ESN.cpp"

#endif // _METRIC_MAPPING_DETAILS_ESN_HPP
