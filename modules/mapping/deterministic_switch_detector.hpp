#ifndef _METRIC_DETERMINISTIC_SWITCH_DETECTOR_HPP
#define _METRIC_DETERMINISTIC_SWITCH_DETECTOR_HPP


#include "../../3rdparty/blaze/Blaze.h"


/**
 * @class SwitchDetector - non-ML deterministic detector of On-Off and Off-On switches in 3-dimensional timeseries.
 */
template <typename value_type>
class DetSwitchDetector {

public:

    /**
     * @brief DetSwitchDetector  - create instance using weight matrices
     * @param W1_ - hidden layer weights, 4x5 matrix:
     *   rows correspond to R, G, B stddev feature filtered signal, respective,
     *   columns correspond to R, G, B, stddev feature and bias neuron inputs
     * @param Wo_ - single output neuron mixing weights, r, g, b, stddev weights, respective, in single row
     * @param wnd_size_  - stddev computation sliding window size
     * @param update_rate_  - parameter to control off latency: weight of new signal in the cumulative output:
     *   1 - no latency, 0 - no update (eternal delay)
     */
    DetSwitchDetector(
            const blaze::DynamicMatrix<value_type> & W1_,
            const blaze::DynamicMatrix<value_type> & Wo_,
            const size_t wnd_size_ = 15,
            const value_type update_rate_ = 0.0025
            );

    /**
     * @brief DetSwitchDetector  - create instance using parameters
     * @param wnd_size_  - stddev computation sliding window size
     * @param update_rate_  - parameter to control off latency: weight of new signal in the cumulative output:
     *   1 - no latency, 0 - no update (eternal delay)
     * @param w_r  - weight for R: incerasing will cause cufoff at lover input signal
     * @param w_g  - same for G
     * @param w_b  - same for B
     * @param w_s  - same for stddev additional feature
     * @param magn - overall scale factor
     * @param rgb_offs  - single offset value for R, G and B
     * @param s_offs  - offset value for stddev feature: insreasing will lift the zero point up
     * @param rgb_wo  - output neuron mixing weights, same value for R, G and B
     * @param s_wo  - same for stddev feature
     */
    DetSwitchDetector(
            const size_t wnd_size_ = 15,
            const value_type update_rate_ = 0.0025,
            const value_type w_r = 2,
            const value_type w_g = 2,
            const value_type w_b = 2,
            const value_type w_s = 15,
            const value_type magn = 0.002,
            const value_type rgb_offs = -5,
            const value_type s_offs = 0,
            const value_type rgb_wo = 1,
            const value_type s_wo = 2
            );


    /**
     * @brief encode  - apply to GRB timeseries
     * @param dataset  - input timeseries with points in rows, row should contain at least 4 values:
     *    first is ignored (it may be timestamp), others are R, G, B input
     * @return  - single column matrix of same number of rows as input, containing found switches:
     *    -1 - off, 1 - on, 0 - no switch
     */
    blaze::DynamicMatrix<value_type> encode(const blaze::DynamicMatrix<value_type> & dataset);



private:

    blaze::DynamicMatrix<value_type> W1;  // hodden layer weights (4 rows)
    blaze::DynamicMatrix<value_type> Wo;  // output neuron weights (single row)
    size_t wnd_size;  // stddev sliding window size
    value_type update_rate;  // update rate between between 0 and 1, the greater, the lower is latency
    value_type lat = 0; // we keep latency value between calls

};



#include "deterministic_switch_detector.cpp"

#endif // _METRIC_DETERMINISTIC_SWITCH_DETECTOR_HPP
