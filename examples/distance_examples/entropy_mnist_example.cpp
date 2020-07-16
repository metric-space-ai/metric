
#include "../../modules/utils/datasets.hpp"
#include "../../modules/correlation/entropy.hpp"
#include "../../modules/mapping/PCFA.hpp"


#include <iostream>


void print_img(std::vector<double> img, size_t h, size_t w) {
    std::string s[4] = {" ", ".", "*", "&"};
    std::cout << "\n";
    for (size_t y = 0; y<h; ++y) {
        for (size_t x = 0; x<w; ++x) {
            //std::cout << (int)(img[y*w + x]*5) << " ";
            int d = img[y*w + x]*3;
            if (d < 0)
                d = 0;
            if (d > 3)
                d = 3;
            std::cout << s[d] << " ";
        }
        std::cout << "|\n";
    }
}


template <typename T>
std::vector<std::vector<T>> combine(const std::vector<std::vector<T>>& X, const std::vector<std::vector<T>>& Y)
{
    std::size_t N = X.size();
    std::size_t dx = X[0].size();
    std::size_t dy = Y[0].size();
    std::vector<std::vector<T>> XY(N);
    for (std::size_t i = 0; i < N; i++) {
        XY[i].resize(dx + dy);
        std::size_t k = 0;
        for (std::size_t j = 0; j < dx; j++, k++) {
            XY[i][k] = X[i][j];
        }
        for (std::size_t j = 0; j < dy; j++, k++) {
            XY[i][k] = Y[i][j];
        }
    }
    return XY;
}



int main()
{
    Datasets datasets;
    /* shape: [batches, rows, cols, channels] */
    auto [labels, shape, features] = datasets.getMnist("data.cereal");

    std::cout << labels.size() << "\n";
    std::cout << shape.size() << "\n";
    std::cout << features.size() << "\n";

    std::vector<std::vector<double>> images;

    int margin = 4; // set margins here

    size_t cnt = 0;
    auto img = std::vector<double>();
    for (auto el : features) {
        if (cnt >= shape[1]*shape[2]) {
            cnt = 0;
            images.push_back(img);
            img = std::vector<double>();
        }
        auto y = cnt % shape[2];
        if (
                cnt>=shape[1]*margin &&
                cnt<shape[2]*(shape[1]-margin) &&
                y>=margin &&
                y<(shape[2]-margin)
           ) // crop
            img.push_back((double)(el/255.0)); // TODO try other type
        ++cnt;
    }
    images.push_back(img); // last


    size_t ds_size = 50; //5400; // set dataset size here

    //std::vector<std::vector<int>> lbls (10); // for debug
    std::vector<std::vector<std::vector<double>>> imgs_by_digit (10);
    size_t filled = 0;
    for (size_t i = 0; i<images.size(); ++i) {
        if (imgs_by_digit[labels[i]].size() < ds_size) {
            imgs_by_digit[labels[i]].push_back(images[i]);
            //lbls[labels[i]].push_back(labels[i]); // for debug
            if (imgs_by_digit[labels[i]].size() == ds_size) {
                filled++;
            }
        }
        if (filled >= 10)
            break;
    }

    std::vector<std::vector<double>> imgs_mixed;
    size_t digit = 0;
    //std::vector<int> lbls_mixed; // for debug
    for (size_t i = 0; i<ds_size; ++i) {
        imgs_mixed.push_back(imgs_by_digit[digit][i/10U]);
        //lbls_mixed.push_back(i/10U); // for debug
        ++digit;
        if (digit >= 10)
            digit = 0;
    }

    int width = shape[2] - 2*margin; // cropped

    std::cout << "\n sizes of datasets:\n";
    for (size_t i = 0; i<10; ++i) {
        std::cout << i << ": " << imgs_by_digit[i].size() << "\n";
    }
    std::cout << "mixed: " << imgs_mixed.size() << "\n";

//    for (size_t i = 0; i<32; ++i) { // output first 32 digits
//        std::cout << "\n";
//        print_img(imgs_mixed[i], width, width);
//    }

//    for (size_t i = 0; i<100; ++i) { // output first 32 digits
//        std::cout << "\n";
//        print_img(imgs_by_digit[2][i], width, width);
//    }

    auto pcfa = metric::PCFA<std::vector<double>, void>(imgs_mixed, 20);
    //std::cout << "\neigenmodes:" << pcfa.eigenmodes_mat() << "\n";
    auto imgs_mixed_encoded = pcfa.encode(imgs_mixed);
    std::vector<std::vector<std::vector<double>>> imgs_by_digit_encoded;
    for (size_t i = 0; i<10; ++i) {
        imgs_by_digit_encoded.push_back(pcfa.encode(imgs_by_digit[i]));
    }


//    auto imgs_mixed_decoded = pcfa.decode(imgs_mixed_encoded);
//    std::cout << "\nDecoded images: \n";
//    for (size_t i = 0; i<32; ++i) { // output first 32 digits
//        std::cout << "\n";
//        print_img(imgs_mixed_decoded[i], width, width);
//    }


    //auto estimator = metric::Entropy<void, metric::Euclidean<double>>();
    auto estimator = metric::EntropySimple<void, metric::Euclidean<double>>(metric::Euclidean<double>(), 7);
//    auto estimator = metric::EntropySimple<void, metric::Chebyshev<double>>(metric::Chebyshev<double>(), 7);
//    auto estimator = metric::Entropy<std::vector<double>>();
//    auto estimator = metric::EntropySimple<std::vector<double>>();

    //std::cout << "overall entropy: " << estimator.estimate(images, 100, 0.1, 10) << std::endl;
    //std::cout << "overall entropy: " << estimator(images) << std::endl;

//    for (size_t i = 0; i<10; ++i) {
//        std::cout << "entropy for digit " << i << ": " << estimator(imgs_by_digit_encoded[i]) << std::endl;
//    }
//    std::cout << "entropy for all digits: " << estimator(imgs_mixed_encoded) << std::endl;

//    for (size_t i = 0; i<10; ++i) {
//        std::cout << "entropy for digit " << i << ": " << estimator(imgs_by_digit[i]) << std::endl;
//    }
//    std::cout << "entropy for all digits: " << estimator(imgs_mixed) << std::endl;

    //auto vm = metric::VMixing<void, metric::Euclidean<double>>(metric::Euclidean<double>(), 7, 70);
    auto vms = metric::VMixing_simple<void, metric::Euclidean<double>>(metric::Euclidean<double>(), 7);

    //std::cout << "digit1 digit2 entropy1 entropy2 | entropy_mixed entropy_mixed_half entropy_mixed_u entropy_mixed_half_u entropy_joint | diff_mixed diff_mixed_half diff_mixed_u diff_mixed_half_u VOI\n";
    //std::cout << "digit1 digit2 entropy1 entropy2 | entropy_mixed entropy_joint | diff_mixed VOI\n";
    //std::cout << "digit1 digit2 entropy1 entropy2 | entropy_mixed entropy_mixed_half entropy_joint | diff_mixed diff_mixed_half VOI\n";
    //std::cout << "digit1 digit2 entropy1 entropy2 | entropy_mixed | diff_mixed \n";
    std::cout << "digit1 digit2 entropy1 entropy2 | entropy_mixed | diff_mixed | diff_mixed \n";
    for (size_t i = 0; i<10; ++i) { // TODO set i = 0
        for (size_t j = i; j<10; ++j) {
            std::vector<std::vector<double>> concat_ds;
            concat_ds.reserve(imgs_by_digit_encoded[i].size() + imgs_by_digit_encoded[j].size());
            concat_ds.insert(concat_ds.end(), imgs_by_digit_encoded[i].begin(), imgs_by_digit_encoded[i].end());
            concat_ds.insert(concat_ds.end(), imgs_by_digit_encoded[j].begin(), imgs_by_digit_encoded[j].end());
            std::vector<std::vector<double>> joint_ds = combine(imgs_by_digit_encoded[i], imgs_by_digit_encoded[j]);
            auto H_i = estimator(imgs_by_digit_encoded[i]);
            auto H_j = estimator(imgs_by_digit_encoded[j]);
            auto H_concat = estimator(concat_ds);
//            auto H_joint = estimator(joint_ds);
//            std::vector<std::vector<double>> concat_ds_half;
//            concat_ds_half.reserve(imgs_by_digit_encoded[i].size());
//            concat_ds_half.insert(concat_ds_half.end(), imgs_by_digit_encoded[i].begin(), imgs_by_digit_encoded[i].begin() + imgs_by_digit_encoded[i].size()/2);
//            concat_ds_half.insert(concat_ds_half.end(), imgs_by_digit_encoded[j].begin(), imgs_by_digit_encoded[j].begin() + imgs_by_digit_encoded[j].size()/2);
//            auto H_concat_half = estimator(concat_ds_half);
            std::cout <<
//                         "VMixed for digits " << i << ", " << j << ": " <<
                         i << " " << j << " " << H_i << " " << H_j <<
                         " | " <<
                         H_concat << " " <<
//                         H_concat_half << " " <<
//                         H_joint <<
                         " | " <<
                         H_concat*2 - H_i - H_j << " " <<
//                         H_concat_half*2 - H_i - H_j << " " <<
//                         H_joint*2 - H_i - H_j << " " <<
                         " | " <<
                         //metric::VOI_simple(imgs_by_digit_encoded[i], imgs_by_digit_encoded[j], 7) <<
                         //metric::VOI_simple<std::vector<std::vector<double>>, metric::Euclidean<double>>(imgs_by_digit_encoded[i], imgs_by_digit_encoded[j], 7) <<
                         vms(imgs_by_digit_encoded[i], imgs_by_digit_encoded[j]) <<
                         //vm(imgs_by_digit_encoded[i], imgs_by_digit_encoded[j]) <<
                         std::endl;
        }

        std::vector<std::vector<double>> concat_ds;
        concat_ds.reserve(imgs_by_digit_encoded[i].size() + imgs_mixed_encoded.size());
        concat_ds.insert(concat_ds.end(), imgs_by_digit_encoded[i].begin(), imgs_by_digit_encoded[i].end());
        concat_ds.insert(concat_ds.end(), imgs_mixed_encoded.begin(), imgs_mixed_encoded.end());
        std::vector<std::vector<double>> joint_ds = combine(imgs_by_digit_encoded[i], imgs_mixed_encoded);
        auto H_i = estimator(imgs_by_digit_encoded[i]);
        auto H_j = estimator(imgs_mixed_encoded);
        auto H_concat = estimator(concat_ds);
//        auto H_joint = estimator(joint_ds);
//        std::vector<std::vector<double>> concat_ds_half;
//        concat_ds_half.reserve(imgs_by_digit_encoded[i].size());
//        concat_ds_half.insert(concat_ds_half.end(), imgs_by_digit_encoded[i].begin(), imgs_by_digit_encoded[i].begin() + imgs_by_digit_encoded[i].size()/2);
//        concat_ds_half.insert(concat_ds_half.end(), imgs_mixed_encoded.begin(), imgs_mixed_encoded.begin() + imgs_mixed_encoded.size()/2);
//        auto H_concat_half = estimator(concat_ds_half);
        std::cout <<
                     i << " * " << H_i << " " << H_j << " | " <<
                     H_concat << " " <<
//                     H_concat_half << " " <<
//                     H_joint <<
                     " | " <<
                     H_concat*2 - H_i - H_j << " " <<
//                     H_concat_half*2 - H_i - H_j << " " <<
//                     H_joint*2 - H_i - H_j << " " <<
                     " | " <<
//                     "VMixing for digits " << i << ", all: " <<
                     //metric::VOI_simple(imgs_by_digit_encoded[i], imgs_mixed_encoded, 7) <<
                     //metric::VOI_simple<std::vector<std::vector<double>>, metric::Euclidean<double>>(imgs_by_digit_encoded[i], imgs_mixed_encoded, 7) <<
                     vms(imgs_by_digit_encoded[i], imgs_mixed_encoded) <<
                     //vm(imgs_by_digit_encoded[i], imgs_mixed_encoded) <<
                     std::endl;
    }
    std::vector<std::vector<double>> concat_ds;
    concat_ds.reserve(imgs_mixed_encoded.size() + imgs_mixed_encoded.size());
    concat_ds.insert(concat_ds.end(), imgs_mixed_encoded.begin(), imgs_mixed_encoded.end());
    concat_ds.insert(concat_ds.end(), imgs_mixed_encoded.begin(), imgs_mixed_encoded.end());
    std::vector<std::vector<double>> joint_ds = combine(imgs_mixed_encoded, imgs_mixed_encoded);
    auto H_i = estimator(imgs_mixed_encoded);
    auto H_j = estimator(imgs_mixed_encoded);
    auto H_concat = estimator(concat_ds);
//    auto H_joint = estimator(joint_ds);
//    std::vector<std::vector<double>> concat_ds_half;
//    concat_ds_half.reserve(imgs_mixed_encoded.size());
//    concat_ds_half.insert(concat_ds_half.end(), imgs_mixed_encoded.begin(), imgs_mixed_encoded.begin() + imgs_mixed_encoded.size()/2);
//    concat_ds_half.insert(concat_ds_half.end(), imgs_mixed_encoded.begin(), imgs_mixed_encoded.begin() + imgs_mixed_encoded.size()/2);
//    auto H_concat_half = estimator(concat_ds_half);
    std::cout <<
                 "* * " << H_i << " " << H_j << " | " <<
                 H_concat << " " <<
//                 H_concat_half << " " <<
//                 H_joint <<
                 " | " <<
                 H_concat*2 - H_i - H_j << " " <<
//                 H_concat_half*2 - H_i - H_j << " " <<
//                 H_joint*2 - H_i - H_j << " " <<
                 " | " <<
//                     "VMixed for digits " << i << ", all: " <<
                 //metric::VOI_simple(imgs_by_digit_encoded[i], imgs_mixed_encoded, 7) <<
                 //metric::VOI_simple<std::vector<std::vector<double>>, metric::Euclidean<double>>(imgs_mixed_encoded, imgs_mixed_encoded, 7) <<
                 vms(imgs_mixed_encoded, imgs_mixed_encoded) <<
                 //vm(imgs_by_digit_encoded[i], imgs_mixed_encoded) <<
                 std::endl;


    return 0;
}
