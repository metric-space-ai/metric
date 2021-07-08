#ifndef _METRIC_DETERMINISTIC_SWITCH_DETECTOR_HPP
#define _METRIC_DETERMINISTIC_SWITCH_DETECTOR_HPP


#include "3rdparty/blaze/Blaze.h"


/**
 * @class SwitchDetector - non-ML deterministic detector of On-Off and Off-On switches in 3-dimensional timeseries.
 */
template <typename value_type>
class DetSwitchDetector {

public:

    DetSwitchDetector(
            const blaze::DynamicMatrix<value_type> & W1_,
            const blaze::DynamicMatrix<value_type> & Wo_,
            const size_t wnd_size_ = 15,
            const value_type update_rate_ = 0.0025
            );

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


    blaze::DynamicMatrix<value_type> encode(const blaze::DynamicMatrix<value_type> & dataset);


private:

    blaze::DynamicMatrix<value_type> W1;
    blaze::DynamicMatrix<value_type> Wo;
    size_t wnd_size;
    value_type update_rate;

};



#include "deterministic_switch_detector.cpp"

#endif // _METRIC_DETERMINISTIC_SWITCH_DETECTOR_HPP
