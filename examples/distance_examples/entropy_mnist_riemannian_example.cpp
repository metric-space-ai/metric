
#include "../../modules/utils/datasets.hpp"
#include "../../modules/distance/k-related/Standards.hpp"
#include "../../modules/utils/type_traits.hpp"
//#include "../../modules/distance/d-spaced/Riemannian.hpp"
#include "../../modules/utils/wrappers/lapack.hpp"

#include <iostream>


namespace r_debug {

class RiemannianDistance {

    public:
        RiemannianDistance() = default;

        template<typename T>
        T operator()(blaze::DynamicMatrix<T> A, blaze::DynamicMatrix<T> B)
        {
            blaze::DynamicVector<T> eigenValues;
            metric::sygv(A, B, eigenValues);
            //std::cout << "\n----\n" << eigenValues << "\n----\n";

            for (T& e : eigenValues) {
                if (e <= std::numeric_limits<T>::epsilon()) {
                    e = 1;
                }
            }

            return sqrt(blaze::sum(blaze::pow(blaze::log(eigenValues), 2)));
        }
};

}


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



template <typename C, typename Metric>
double RiemannianDatasetDistance(const C& Xc, const C& Yc, Metric m = Metric()) {

    using T = metric::type_traits::underlying_type_t<C>;

    blaze::DynamicMatrix<T> distancesX (Xc.size(), Xc.size(), 0);
    blaze::DynamicMatrix<T> distancesY (Yc.size(), Yc.size(), 0);

    for (size_t i = 0; i<Xc.size(); ++i) {
        for (size_t j = 0; j<Xc.size(); ++j) {
            auto d = m(Xc[i], Xc[j]);
            if (i < j) { // upper triangular area only
                distancesX(i, j) = -d; // Laplacian matrix
                distancesX(i, i) += d;
                distancesX(j, j) += d;
            }
            //distancesX(i, j) = d; // filling blaze distance matrix for dataset Xc
            //distancesX(j, i) = d;
        }
    }
    for (size_t i = 0; i<Yc.size(); ++i) {
        for (size_t j = 0; j<Yc.size(); ++j) {
            auto d = m(Yc[i], Yc[j]);
            if (i < j) { // upper triangular area only
                distancesY(i, j) = d;
                distancesY(i, i) += d;
                distancesY(j, j) += d;
            }
            //distancesY(i, j) = d; // filling blaze distance matrix for dataset Xc
            //distancesY(j, i) = d;
        }
    }
    //std::cout << distancesX << distancesY; // TODO remove

    //auto rd = metric::RiemannianDistance();
    auto rd = r_debug::RiemannianDistance();
    return rd(distancesX, distancesY); // applying Riemannian to these distance matrices
}




int main() {

    //blaze::DynamicMatrix<double> A {{2, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    //blaze::DynamicMatrix<double> B {{1, 0, 0}, {0, 2, 0}, {0, 0, 1}};

    //blaze::DynamicMatrix<double> A {{0, 1, 1}, {0, 0, 1}, {0, 0, 0}};
    //blaze::DynamicMatrix<double> B {{0, 1, 1}, {0, 0, 2}, {0, 0, 0}};

    blaze::DynamicMatrix<double> A {{0, 1, 0}, {0, 0, 1}, {0, 0, 0}};
    blaze::DynamicMatrix<double> B {{0, 1, 0}, {0, 0, 2}, {0, 0, 0}};

    blaze::DynamicVector<double> gen_eigenv;
    metric::sygv(A, B, gen_eigenv);
    std::cout << gen_eigenv << "\n";

    auto rd = r_debug::RiemannianDistance();
    std::cout << rd(A, B) << "\n";

    //return 0;

    Datasets datasets;
    /* shape: [batches, rows, cols, channels] */
    auto [labels, shape, features] = datasets.getMnist("data.cereal");

    std::cout << labels.size() << "\n";
    std::cout << shape.size() << "\n";
    std::cout << features.size() << "\n";

    std::vector<std::vector<double>> images;

    int margin = 4; // set up here

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


    size_t ds_size = 300; //5400;

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


    for (size_t i = 0; i<10; ++i) {
        for (size_t j = i; j<10; ++j) {
            std::cout <<
                         "Riemannian distance for digits " << i << ", " << j << ": " <<
                         //RiemannianDatasetDistance<std::vector<std::vector<double>>, metric::Euclidean<double>>(imgs_by_digit[i], imgs_by_digit[j]) <<
                         RiemannianDatasetDistance<std::vector<std::vector<double>>, metric::Chebyshev<double>>(imgs_by_digit[i], imgs_by_digit[j]) <<
                         std::endl;
        }
//        std::cout <<
//                     "VOI for digits " << i << ", all: " <<
//                     RiemannianDatasetDistance<std::vector<std::vector<double>>, metric::Euclidean<double>>(imgs_by_digit[i], imgs_mixed) <<
//                     std::endl;
    }





    return 0;
}
