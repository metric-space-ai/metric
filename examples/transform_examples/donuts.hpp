#ifndef DONUTS_HPP
#define DONUTS_HPP

#include "../../3rdparty/blaze/Blaze.h"

#include <cmath>
#include <iostream>
//#include <tuple>
//#include <opencv2/opencv.hpp>




/*

blaze::DynamicMatrix<double> create_filter(double sigma) {
    std::size_t size = ceil(sigma)*3 + 1;
    blaze::DynamicMatrix<double> out (size, size, 0);
    double sz = (double)(size - 1)/2.0;
    double x, y, h;
    double max_h = 0;
    double sum_h = 0;
    for (int yi = 0; yi<size; ++yi) {
        for (int xi = 0; xi<size; ++xi) {
            x = -sz + xi;
            y = -sz + yi;
            h = exp(-(x*x + y*y)/(2*sigma*sigma));
            if (h>max_h)
                max_h = h;
            sum_h += h;
            out(yi, xi) = h;
        }
    }
    // TODO add h(h<eps*max(h(:))) = 0 for stability

    if (sum_h != 0) {
        out = out/sum_h;
    }
    return out;
}




blaze::DynamicMatrix<double> gblur3(blaze::DynamicMatrix<double> in) {
    auto im = blaze2cv(in);
    cv::Mat imb;
    cv::GaussianBlur(im, imb, cv::Size(3, 3), 0);
    return cv2blaze(imb);
}


blaze::DynamicMatrix<double> gaussianBlur(blaze::DynamicMatrix<double> in, double sigma) {
    auto im = blaze2cv(in);
    cv::Mat krn = blaze2cv(create_filter(sigma));
    cv::Mat imb;
    cv::filter2D(im, imb, cv::DataType<double>::type, krn);
    return cv2blaze(imb);
}

// */




template <typename T>
std::vector<std::vector<T>> binarize(std::vector<std::vector<T>> in, T level) {

    std::vector<std::vector<T>> out;
    for (size_t y=0; y<in.size(); ++y) {
        std::vector<T> line;
        for (size_t x=0; x<in[0].size(); ++x) {
            if (in[y][x] < level)
                line.push_back(0);
            else
                line.push_back(1);
        }
        out.push_back(line);
    }

    return out;
}


template <typename T>
std::vector<std::vector<T>> otsu_binarize(std::vector<std::vector<T>> in, float expo_corr = 0, T max = 1, int levels = 100) {

    std::vector<std::vector<T>> out;

    size_t height = in.size();
    assert(height>0);
    size_t width = in[0].size();

    std::vector<T> v_levels;
    T step = max/(float)levels;
    T curr_level = 0;
    for (int i=0; i<levels; ++i) {
        v_levels.push_back(curr_level);
        curr_level += step;
    }
    assert(abs(v_levels[levels-1] - max + step) < 1e-7); // TODO replace hardcode with type-dependent check!
    v_levels.push_back(max);

    size_t pixel_counter = 0; // TODO remove after testing
    T lum_sum = 0;
    std::vector<T> lum_hist (levels, 0);
    std::vector<int> hist (levels, 0);
    for (size_t y=0; y<height; ++y) {
        for (size_t x=0; x<width; ++x) {
            for (size_t i=1; i<levels+1; ++i) { // very inefficient!! TODO replace at least with binary search..
                if (in[y][x] > v_levels[i-1] && in[y][x] < v_levels[i]) {
                    hist[i-1]++;
                    lum_hist[i-1] += in[y][x];
                    lum_sum += in[y][x];
                    pixel_counter++;
                }
            }
        }
    }
    //assert(pixel_counter==height*width); // FIXME detector pixel loss, TODO fix!!

    // finding Otsu threshold according to https://en.wikipedia.org/wiki/Otsu%27s_method
    double omega0 = 0; //(double)hist[0] / (double)pixel_counter;
    double omega1; // = 1 - omega0;
    double weighted_lum_sum_0 = 0;
    double weighted_lum_sum_1 = lum_sum;
    double mu0;
    double mu1;
    //std::vector<double> variance;
    size_t max_variance_idx = 0;
    double max_variance = 0;
    double variance;
    double mu_diff;
    for (size_t i=0; i<levels+1; ++i) {
        omega0 += (double)hist[i] / (double)pixel_counter;
        omega1 = 1 - omega0;
        weighted_lum_sum_0 += (double)lum_hist[i];
        weighted_lum_sum_1 = lum_sum - weighted_lum_sum_0;
        mu0 = weighted_lum_sum_0 / omega0;
        mu1 = weighted_lum_sum_1 / omega1;
        mu_diff = mu0 - mu1;
        variance = omega0*omega1*mu_diff*mu_diff;
        if (variance > max_variance) {
            max_variance = variance;
            max_variance_idx = i;
        }
    }
    // here we have max_variance_idx that referce to thresold luminosity we can obtain from v_levels

    T threshold = v_levels[max_variance_idx];
    threshold += max*expo_corr; // applying manual 'exposure correction'
    for (size_t y=0; y<in.size(); ++y) {
        std::vector<T> line;
        for (size_t x=0; x<in[0].size(); ++x) {
            if (in[y][x] < threshold)
                line.push_back(0);
            else
                line.push_back(1);
        }
        out.push_back(line);
    }

    return out;
}



/* // TODO complete if needed
blaze::SymmetricMatrix<blaze::CompressedMatrix<int>>
imgToGraph4(std::vector<std::vector<int>> in) {

    size_t h = in.size();
    size_t w = in[0].size();
    assert(h>0);

    blaze::SymmetricMatrix<blaze::CompressedMatrix<int>> out;


    size_t node_idx = w; // indexing starts from 0, but we start pricessing from 2nd row
    size_t odd = 1;
    for (size_t y = 1; y<in.size()-1; ++y) { // start from 2nd row, stop befpre last row
        //if (odd==0) odd = 1; else
        odd = (odd==1 ? 0 : 1);
        for (size_t x = 1+odd; x<in[0].size()-1-odd; x += 2) { // start from 2nd col, stop before last col
            if (in[y][x] > 0) {
                if (in[y-1][x] > 0)
                    out(node_idx, node_idx - w) = 1; //up
                if (in[y+1][x] > 0)
                    out(node_idx, node_idx + w) = 1; //down
                if (in[y][x-1] > 0)
                    out(node_idx, node_idx - 1) = 1; //left
                if (in[y][x+1] > 0)
                    out(node_idx, node_idx + 1) = 1; //right
            }
        }
        node_idx += w;
    }
    for (size_t x = 1; x<in[0].size()-1; ++x) { // first and last rows
        if (in[0][x] > 0) { // 1st row
            if (in[1][x] > 0)
                out(x, x+w) = 1; // down
            if (in[0][x-1] > 0)
                out(x, x-1) = 1; // left
            if (in[0][x+1] > 0)
                out(x, x+1) = 1; // right
        }
        size_t offset = (h-1)*w;
        if (in[h-1][x] > 0) { // last row
            if (in[h-2][x] > 0)
                out(offset+x, offset+x+w) = 1; // up
            if (in[h-1][x-1] > 0)
                out(offset+x, offset+x-1) = 1; // left
            if (in[h-1][x+1] > 0)
                out(offset+x, offset+x+1) = 1; // right
        }
    }
    for (size_t y = 1; y<in.size()-1; ++y) { // first and last columns
        // TODO vertical lines!
    }
    // TODO corners!

    return out;
}
// */



std::vector<std::vector<double>> radial_diff(std::vector<std::vector<double>> in, double step=1.5) {

    size_t height = in.size();
    assert(height>0);
    size_t width = in[0].size();

    int y0 = (int)round(height/2);
    int x0 = (int)round(width/2);

    std::vector<std::vector<double>> out;
    for (int y=0; y<(int)in.size(); ++y) {
        std::vector<double> line;
        for (int x=0; x<(int)in[0].size(); ++x) {

            double fi = atan2(x - x0, y - y0); // TODO remove trigonometry functions, components can be computed without angle!
            double xi = x - sin(fi) * step; // point for interpolation, '-' for direction to center
            double yi = y - cos(fi) * step;

            size_t bxi = (size_t)floor(xi); // base
            size_t byi = (size_t)floor(yi);

            double oxi = xi - bxi; // offset
            double oyi = yi - byi;

            double c_i = // biliniar interpolation
                    in[byi][bxi] * (1 - oxi) * (1 - oyi) +
                    in[byi + 1][bxi] * oxi * (1 - oyi) +
                    in[byi][bxi + 1] * (1 - oxi) * oyi +
                    in[byi + 1][bxi + 1] * oxi * oyi;

            line.push_back(abs(c_i - in[y][x]));
            //line.push_back(c_i);
        }
        out.push_back(line);
    }

    return out;
}



template <typename T>
blaze::DynamicMatrix<T> radial_diff(const blaze::DynamicMatrix<T> & in, double step=1.5) {

    size_t height = in.rows();
    assert(height>0);
    size_t width = in.columns();

    int y0 = (int)round(height/2);
    int x0 = (int)round(width/2);

    blaze::DynamicMatrix<T> out(in.rows(), in.columns());
    for (int y=0; y<(int)in.rows(); ++y) {
        for (int x=0; x<(int)in.columns(); ++x) {

            //double fi = atan2(x - x0, y - y0); // components can be computed without angle!
            //double xi = x - sin(fi) * step; // point for interpolation, '-' for direction to center
            //double yi = y - cos(fi) * step;
            double d_center = sqrt(pow(x - x0, 2) + pow(y - y0, 2));
            if (d_center < 1.5) {
                out(y, x) = 0;
                continue;
            }
            double xi = x - (x - x0) * step / d_center; // point for interpolation, '-' for direction to center
            double yi = y - (y - y0) * step / d_center;

            size_t bxi = (size_t)floor(xi); // base
            size_t byi = (size_t)floor(yi);

            double oxi = xi - bxi; // offset
            double oyi = yi - byi;

            double c_i = // biliniar interpolation
                    in(byi, bxi) * (1 - oxi) * (1 - oyi) +
                    in(byi + 1, bxi) * oxi * (1 - oyi) +
                    in(byi, bxi + 1) * (1 - oxi) * oyi +
                    in(byi + 1, bxi + 1) * oxi * oyi;
            out(y, x) = abs(c_i - in(y, x));
        }
    }

    return out;
}





/*
template <typename T> // old crop version, TODO debug if needed
std::tuple<std::vector<std::vector<T>>, std::vector<std::vector<T>>>
split_donut(std::vector<std::vector<T>> in, float crop_share = 3.0/5.0) {

    assert(crop_share>0 && crop_share<1);
    size_t height = in.size();
    assert(height>0);
    size_t wigth = in[0].size();

    size_t crop_middle = round(in[0].size()*crop_share);
    size_t crop_left = (wigth - crop_middle)/2;
    size_t crop_right = crop_left + crop_middle;
    size_t crop_top = (height - crop_middle)/2; // inner part will be ever square
    size_t crop_bottom = crop_top + crop_middle;

    std::vector<std::vector<T>> inner;
    std::vector<std::vector<T>> outer = in;
    for (size_t y = crop_top; y<crop_bottom; ++y) {
        std::vector<T> line;
        for (size_t x = crop_left; x<crop_right; ++x) {
            line.push_back(in[y][x]); // FIXME some indexing bug..
            outer[y][x] = 0;
        }
        inner.push_back(line);
    }
    return std::make_tuple(inner, outer);
}
// */



template <typename T>
blaze::DynamicMatrix<T> weightingMask(size_t h, size_t w, T radius, T sigma) { // radius is where we expect the donut outline
    T center_x = T(w) / 2.0;
    T center_y = T(h) / 2.0;
    auto mask = blaze::DynamicMatrix<T>(h, w);
    T r, y_dist, x_dist, r_dist;
    for (size_t i = 0; i < h; ++i) {
        for (size_t j = 0; j < w; ++j) {
            r = sqrt(pow(i - center_y, 2) + pow(j - center_x, 2));
            //mask(i, j) = gauss(r, radius, sigma);
            mask(i, j) = exp(-1 * (pow(r - radius, 2) / pow(2 * sigma, 2))); // non-normalized gaussian
        }
    }
    return mask;
}


/*

void eat_donut(int number) {  // old test batch procewssing function

    boost::gil::rgb8_image_t img;
    //boost::gil::read_image("assets/" + std::to_string(number) + ".jpg", img, boost::gil::jpeg_tag());
    //boost::gil::read_image("assets/new/crop_2020-07-27_16_23_01_776_donut" + std::to_string(number) + ".png", img, boost::gil::jpeg_tag());
    //boost::gil::png_read_and_convert_image("assets/new/crop_2020-07-27_16_23_01_776_donut" + std::to_string(number) + ".png", img);
    boost::gil::read_image("assets/new/crop_2020-07-27_16_23_01_776_donut" + std::to_string(number) + ".png", img, boost::gil::png_tag());
    auto gray = boost::gil::color_converted_view<boost::gil::gray8_pixel_t>(const_view(img));

    std::vector<std::vector<double>> p;
    for (int y=0; y<gray.height(); ++y) {
        std::vector<double> line;
        for (int x=1; x<gray.width(); ++x) {
            line.push_back(gray(x, y)/255.0);
        }
        p.push_back(line);
    }

    auto pr = radial_diff(p);

    //crop
    auto [inner, outer] = split_donut(pr, 0.35); // parameter is share of inner size

//    double max_val = 0;
//    blaze::DynamicMatrix<double> inner_blaze (inner.size(), inner[0].size(), 0);
//    for (size_t y=0; y<inner.size(); ++y) {
//        for (size_t x=0; x<inner[0].size(); ++x) {
//            inner_blaze(y, x) = inner[y][x]*255;
//            if (inner_blaze(y, x) > max_val) {
//                max_val = inner_blaze(y, x);
//            }
//        }
//    }
//    blaze::DynamicMatrix<double> outer_blaze (outer.size(), outer[0].size(), 0);
//    for (size_t y=0; y<outer.size(); ++y) {
//        for (size_t x=0; x<outer[0].size(); ++x) {
//            outer_blaze(y, x) = outer[y][x]*255;
//            if (outer_blaze(y, x) > max_val) {
//                max_val = outer_blaze(y, x);
//            }
//        }
//    }
    //auto inner_result = metric::fit_hysteresis(inner_blaze);
    //auto outer_result = metric::fit_hysteresis(outer_blaze);

    // TODO output!!

    std::string filenum_prefix = "donut_" + std::to_string(number) + "_";

    vector2bmp(p, filenum_prefix + "input.bmp");
    vv_to_csv(p, filenum_prefix + "input.csv");
    vector2bmp(pr, filenum_prefix + "radial_diff.bmp", 4);
    vv_to_csv(pr, filenum_prefix + "radial_diff.csv");

    vector2bmp(outer, filenum_prefix + "radial_outer.bmp", 4);
    vv_to_csv(outer, filenum_prefix + "radial_outer.csv");
    vector2bmp(inner, filenum_prefix + "radial_inner.bmp", 4);
    vv_to_csv(inner, filenum_prefix + "radial_inner.csv");

    auto outer0 = otsu_binarize(outer, 0);
    vector2bmp(outer0, filenum_prefix + "binarized_outer_0.bmp");
    vv_to_csv(outer0, filenum_prefix + "binarized_outer_0.csv");

    auto outer007 = otsu_binarize(outer, 0.07);
    vector2bmp(outer007, filenum_prefix + "binarized_outer_007.bmp");
    vv_to_csv(outer007, filenum_prefix + "binarized_outer_007.csv");

    auto outer1 = otsu_binarize(outer, 0.1);
    vector2bmp(outer1, filenum_prefix + "binarized_outer_01.bmp");
    vv_to_csv(outer1, filenum_prefix + "binarized_outer_01.csv");

    auto inner0 = otsu_binarize(inner);
    vector2bmp(inner0, filenum_prefix + "binarized_inner_0.bmp");
    vv_to_csv(inner0, filenum_prefix + "binarized_inner_0.csv");

    auto inner007 = otsu_binarize(inner, 0.07);
    vector2bmp(inner007, filenum_prefix + "binarized_inner_007.bmp");
    vv_to_csv(inner007, filenum_prefix + "binarized_inner_007.csv");

    auto inner1 = otsu_binarize(inner, 0.1);
    vector2bmp(inner1, filenum_prefix + "binarized_inner_01.bmp");
    vv_to_csv(inner1, filenum_prefix + "binarized_inner_01.csv");

}

// */


//int main() {

//    for (size_t i = 1; i<=8; i++) {
//        eat_donut(i);
//    }

//    std::cout << "\ncomplete\n";

//    return 0;
//}



#endif // DONUTS_HPP
