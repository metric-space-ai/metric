#ifndef _METRIC_ESN_ON_OFF_DETECTOR_HPP
#define _METRIC_ESN_ON_OFF_DETECTOR_HPP


#include "ESN.hpp"


template <typename value_type>
class SwitchPredictor {

public:
    SwitchPredictor(const blaze::DynamicMatrix<value_type> & training_data, const blaze::DynamicMatrix<value_type> & labels) {

        wnd_size = 15;
        cmp_wnd_sz = 150;
        contrast_threshold = 0.3;

        assert(training_data.rows() == labels.rows());
        assert(training_data.columns() == 3);  // TODO relax

        auto data = preprocess(training_data);

        blaze::DynamicMatrix<value_type> target (labels.rows(), 1, 0);
        int new_label = 0;
        for (size_t i = wnd_size; i < labels.rows(); ++i) {
            if (labels(i, 0) >= 1)
                new_label = 1;
            if (labels(i, 0) <= -1)
                new_label = 0;
            target(i, 0) = new_label;
        }

        esn = metric::ESN<std::vector<value_type>, void>(500, 5, 0.99, 0.1, 2500, 0.5); // TODO pass
        esn.train(data, blaze::trans(target));
    }


    SwitchPredictor(const std::string & filename) {

        wnd_size = 15;
        cmp_wnd_sz = 150;
        contrast_threshold = 0.3;

        esn = metric::ESN<std::vector<value_type>, void>(filename);
    }


    blaze::DynamicMatrix<value_type> estimate(const blaze::DynamicMatrix<value_type> & dataset) {

        auto data = preprocess(dataset);

        auto prediction = esn.predict(data);
        blaze::DynamicMatrix<value_type, blaze::rowMajor> out = blaze::trans(prediction);

        blaze::DynamicMatrix<value_type> sl_entropy (out.rows(), 1, 0);
        for (size_t i = wnd_size; i < out.rows(); ++i) {
            blaze::DynamicMatrix<value_type> wnd_row = blaze::submatrix(out, i - wnd_size, 0, wnd_size, 1);
            blaze::DynamicVector<value_type> wnd = blaze::column(wnd_row, 0); //blaze::trans(blaze::column(wnd_row, 0));
            sl_entropy(i, 0) = class_entropy(wnd, 0.5);
        }

        blaze::DynamicMatrix<value_type> postproc (out.rows(), 1, 0);
        bool prev_l_flag = false;
        for (size_t i = cmp_wnd_sz; i < out.rows() - cmp_wnd_sz; ++i) {
            bool l_flag = false;
            if (sl_entropy(i, 0) > 0.4) {
                blaze::DynamicMatrix<value_type> wnd_past = blaze::submatrix(out, i - cmp_wnd_sz, 0, cmp_wnd_sz, 1);
                blaze::DynamicMatrix<value_type> wnd_fut  = blaze::submatrix(out, i, 0, cmp_wnd_sz, 1);
                int label = 0;
                if (blaze::mean(wnd_past) - blaze::mean(wnd_fut) < -contrast_threshold) {  // TODO determine!!
                    label = 1;
                    l_flag = true;
                }
                if (blaze::mean(wnd_past) - blaze::mean(wnd_fut) > contrast_threshold) {  // TODO determine!!
                    label = -1;
                    l_flag = true;
                }
                if (!prev_l_flag)
                    postproc(i, 0) = label;
            }
            prev_l_flag = l_flag;
        }

        return postproc;
    }


    void save(const std::string & filename) {

        esn.save(filename);
    }


private:

    metric::ESN<std::vector<value_type>, void> esn;
    size_t wnd_size;
    size_t cmp_wnd_sz;
    value_type contrast_threshold;


    blaze::DynamicMatrix<value_type, blaze::rowMajor> preprocess(const blaze::DynamicMatrix<value_type, blaze::rowMajor> & input) {

        //blaze::DynamicMatrix<value_type> target (labels.rows(), 1, 0);

        blaze::DynamicVector<value_type> feature_stddev (input.rows(), 0);
        int new_label = 0;
        for (size_t i = wnd_size; i < feature_stddev.size(); ++i) {
            auto wnd1 = blaze::submatrix(input, i - wnd_size, 0, wnd_size, 1);
            auto wnd2 = blaze::submatrix(input, i - wnd_size, 1, wnd_size, 1);
            auto wnd3 = blaze::submatrix(input, i - wnd_size, 2, wnd_size, 1);
            feature_stddev[i] = stddev(wnd1) + stddev(wnd2) + stddev(wnd3);
            //if (labels(i, 0) >= 1)
            //    new_label = 1;
            //if (labels(i, 0) <= -1)
            //    new_label = 0;
            //target(i, 0) = new_label;
        }

        blaze::DynamicMatrix<value_type> ds_all (input.rows(), 4, 0);
        blaze::submatrix(ds_all, 0, 0, input.rows(), 3) = blaze::submatrix(input, 0, 0, input.rows(), 3);
        blaze::column(ds_all, 3) = feature_stddev;
        blaze::DynamicMatrix<value_type, blaze::rowMajor> output = blaze::trans(ds_all);

        return output;
    }


    value_type class_entropy(const blaze::DynamicVector<value_type> & data, value_type threshold) {
        int sum = 0;
        value_type sz = data.size();
        for (size_t i = 0; i<sz; ++i) {
            if (data[i] > threshold)
                ++sum;
        }
        value_type p1 = sum/sz;
        if (sum == 0 || sum == sz)
            return 0;
        else
            return -p1*log2(p1) - (1 - p1)*log2(1 - p1);
    }

};



#endif // _METRIC_ESN_ON_OFF_DETECTOR_HPP
