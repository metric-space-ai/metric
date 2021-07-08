
#include "esn_switch_detector.hpp"





template <typename value_type>
DetSwitchDetector<value_type>::DetSwitchDetector(
        const blaze::DynamicMatrix<value_type> & W1_,
        const blaze::DynamicMatrix<value_type> & Wo_,
        const size_t wnd_size_,
        const value_type update_rate_
    )
    : wnd_size(wnd_size_)
    , update_rate(update_rate_)
    , W1(W1_)
    , Wo(Wo_)
{}



template <typename value_type>
DetSwitchDetector<value_type>::DetSwitchDetector(
        const size_t wnd_size_,
        const value_type update_rate_,
        const value_type w_r,
        const value_type w_g,
        const value_type w_b,
        const value_type w_s,
        const value_type magn,
        const value_type rgb_offs,
        const value_type s_offs,
        const value_type rgb_wo,
        const value_type s_wo
    )
    : wnd_size(wnd_size_)
    , update_rate(update_rate_)
{
    W1 = {
            {magn*w_r, 0, 0, 0, magn*rgb_offs},
            {0, magn*w_g, 0, 0, magn*rgb_offs},
            {0, 0, magn*w_b, 0, magn*rgb_offs},
            {0, 0, 0, magn*w_s, magn*  s_offs},
    };

    Wo = {
            {rgb_wo, rgb_wo, rgb_wo, s_wo}
    };
}


template <typename value_type>
blaze::DynamicMatrix<value_type>
DetSwitchDetector<value_type>::encode(const blaze::DynamicMatrix<value_type> & dataset) {


    // additional feature: sliding window stddev

    blaze::DynamicVector<value_type> feature_stddev (dataset.rows(), 0);  // TODO remove saving, stddev can be computed on the fly
    for (size_t i = wnd_size; i < feature_stddev.size(); ++i) {
        auto wnd1 = blaze::submatrix(dataset, i - wnd_size, 1, wnd_size, 1);
        auto wnd2 = blaze::submatrix(dataset, i - wnd_size, 2, wnd_size, 1);
        auto wnd3 = blaze::submatrix(dataset, i - wnd_size, 3, wnd_size, 1);
        feature_stddev[i] = stddev(wnd1) + stddev(wnd2) + stddev(wnd3);
    }

    blaze::DynamicMatrix<value_type> ds_all (dataset.rows(), 4, 0);
    //blaze::submatrix(ds_all, 0, 0, dataset.rows(), 3) = blaze::submatrix(dataset, 0, 1, dataset.rows(), 3);  // fails on datasets larger than 100000
    blaze::column(ds_all, 0) = blaze::column(dataset, 1);
    blaze::column(ds_all, 1) = blaze::column(dataset, 2);
    blaze::column(ds_all, 2) = blaze::column(dataset, 3);
    blaze::column(ds_all, 3) = feature_stddev;


    // filtering

    value_type sliding_prev = 0;
    unsigned int bin_out;
    unsigned int bin_prev = 0;
    blaze::DynamicMatrix<value_type> out (ds_all.rows(), 1, 0);

    for (size_t i = wnd_size; i < dataset.rows(); ++i) {

        blaze::DynamicVector<value_type> in1 (ds_all.columns() + 1);
        blaze::subvector(in1, 0, ds_all.columns()) = blaze::trans(blaze::row(ds_all, i));
        in1[ds_all.columns()] = 1;  // offset element
        auto layer1 = blaze::tanh(W1 * in1);

        value_type mixed_out = blaze::sum(Wo*layer1) / blaze::sum(Wo);
        value_type latency_out;
        if (mixed_out > 0.5) { // switching on immediately
            latency_out = 1;
        } else { // switch off latency
            value_type upd = ( mixed_out*update_rate + sliding_prev*(1 - update_rate) );
            latency_out = mixed_out > upd ? mixed_out : upd;  // reset latency
        }
        sliding_prev = latency_out;

        int bin_out = 0;
        if (latency_out > 0.5) { // binarize
            bin_out = 1;
        }
        if (bin_out == 1 && bin_prev == 0)
            out(i, 0) = 1;
        if (bin_out == 0 && bin_prev == 1)
            out(i, 0) = -1;

        bin_prev = bin_out;
    }

    return out;
}
