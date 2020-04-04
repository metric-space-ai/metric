#include <cmath>
//#include <tuple>
#include <iostream>
#include <boost/gil/extension/io/jpeg.hpp>
#include "../../3rdparty/blaze/Blaze.h"
#include "../../modules/transform/distance_potential_minimization.hpp"

#include "../../modules/utils/image_processing/image_filter.hpp"

//metric::imfilter<double, 1, metric::FilterType::GAUSSIAN, metric::PadDirection::BOTH, metric::PadType::CONST> gaussianBlur_(3, 3,0.3);

std::vector<std::vector<double>> radial_diff(std::vector<std::vector<double>> in, double step = 1.5)
{

    size_t height = in.size();
    assert(height > 0);
    size_t width = in[0].size();

    int y0 = (int)round(height / 2);
    int x0 = (int)round(width / 2);

    std::vector<std::vector<double>> out;
    for (int y = 0; y < (int)in.size(); ++y) {
        std::vector<double> line;
        for (int x = 0; x < (int)in[0].size(); ++x) {

            double fi = atan2(x - x0, y - y0);
            double xi = x - sin(fi) * step;  // point for interpolation, '-' for direction to center
            double yi = y - cos(fi) * step;

            size_t bxi = (size_t)floor(xi);  // base
            size_t byi = (size_t)floor(yi);

            double oxi = xi - bxi;  // offset
            double oyi = yi - byi;

            double c_i =  // biliniar interpolation
                in[byi][bxi] * (1 - oxi) * (1 - oyi) + in[byi + 1][bxi] * oxi * (1 - oyi)
                + in[byi][bxi + 1] * (1 - oxi) * oyi + in[byi + 1][bxi + 1] * oxi * oyi;

            line.push_back(abs(c_i - in[y][x]));
            //line.push_back(c_i);
        }
        out.push_back(line);
    }

    return out;
}

//*
template <typename T>  // TODO debug!!
std::tuple<std::vector<std::vector<T>>, std::vector<std::vector<T>>> split_donut(
    std::vector<std::vector<T>> in, float crop_share = 3.0 / 5.0)
{

    assert(crop_share > 0 && crop_share < 1);
    size_t height = in.size();
    assert(height > 0);
    size_t wigth = in[0].size();

    size_t crop_middle = round(in[0].size() * crop_share);
    size_t crop_left = (wigth - crop_middle) / 2;
    size_t crop_right = crop_left + crop_middle;
    size_t crop_top = (height - crop_middle) / 2;  // inner part will be ever square
    size_t crop_bottom = crop_top + crop_middle;

    std::vector<std::vector<T>> inner;
    std::vector<std::vector<T>> outer = in;
    for (size_t y = crop_top; y < crop_bottom; ++y) {
        std::vector<T> line;
        for (size_t x = crop_left; x < crop_right; ++x) {
            line.push_back(in[y][x]);  // FIXME some indexing bug..
            outer[y][x] = 0;
        }
        inner.push_back(line);
    }
    return std::make_tuple(inner, outer);
}
// */

void eat_donut(int number)
{

    boost::gil::rgb8_image_t img;
    boost::gil::read_image("/Users/michaelwelsch/Documents/panda_github/metric/examples/transform_examples/assets/"
            + std::to_string(number) + ".jpg",
        img, boost::gil::jpeg_tag());
    auto gray = boost::gil::color_converted_view<boost::gil::gray8_pixel_t>(const_view(img));

    std::vector<std::vector<double>> p;
    for (int y = 0; y < gray.height(); ++y) {
        std::vector<double> line;
        for (int x = 0; x < gray.width(); ++x) {
            line.push_back(gray(x, y));
        }
        p.push_back(line);
    }

    blaze::DynamicMatrix<double> data(p.size(), p[0].size());

    for (size_t i = 0; i < p.size(); ++i) {
        for (size_t j = 0; j < p[0].size(); ++j) {
            data(i, j) = p[i][j];
        }
    }

    auto pr = radial_diff(p);

    // //crop
    auto [inner, outer] = split_donut(pr, 0.35);  // parameter is share of inner size

    double max_val = 0;
    blaze::DynamicMatrix<double> inner_blaze(inner.size(), inner[0].size(), 0);
    for (size_t y = 0; y < inner.size(); ++y) {
        for (size_t x = 0; x < inner[0].size(); ++x) {
            inner_blaze(y, x) = inner[y][x];
            if (inner_blaze(y, x) > max_val) {
                max_val = inner_blaze(y, x);
            }
        }
    }
    blaze::DynamicMatrix<double> outer_blaze(outer.size(), outer[0].size(), 0);
    for (size_t y = 0; y < outer.size(); ++y) {
        for (size_t x = 0; x < outer[0].size(); ++x) {
            outer_blaze(y, x) = outer[y][x];
            if (outer_blaze(y, x) > max_val) {
                max_val = outer_blaze(y, x);
            }
        }
    }
    std::cout << " _" << std::endl;
    std::cout << "inner r/c: " << inner_blaze.rows() << " " << inner_blaze.columns() << std::endl;
    std::cout << "outer r/c: " << outer_blaze.rows() << " " << outer_blaze.columns() << std::endl;

    metric::imfilter<double, 1, metric::FilterType::GAUSSIAN, metric::PadDirection::BOTH, metric::PadType::CONST> gaussianBlur_(3, 3,0.3);
    blaze::DynamicMatrix<double> outer_results = gaussianBlur_(std::move(outer_blaze));
    auto inner_results = gaussianBlur_(data);

    auto res = metric::fit_hysteresis(data);
}

int main()
{
    for (size_t j = 0; j < 100; ++j) {
        for (size_t i = 1; i <= 8; i++) {
            eat_donut(i);
        }

        return 0;
    }
