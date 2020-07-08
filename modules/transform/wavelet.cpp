/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019  Michael Welsch
*/

#ifndef _METRIC_TRANSFORM_WAVELET_CPP
#define _METRIC_TRANSFORM_WAVELET_CPP
#include "wavelet.hpp"
#include <type_traits>
#include <memory>
#include <algorithm>



namespace wavelet {

//template <typename T, bool SO>
//void shrinkToFit(blaze::CompressedVector<T, SO>& mat)
//{
//    blaze::CompressedVector<T>(~mat).swap(~mat);
//} // moved to helper_functions.cpp



// valid convolution
template <typename Container>
Container conv_valid(Container const& f, Container const& g)
{
    using El = types::index_value_type_t<Container>;

    int const nf = f.size();
    int const ng = g.size();
    Container const& min_v = (nf < ng) ? f : g;
    Container const& max_v = (nf < ng) ? g : f;
    int const n = std::max(nf, ng) - std::min(nf, ng) + 1;
    //Container out(n, typename Container::value_type());
    Container out(n, El());
    for (auto i(0); i < n; ++i) {
        for (int j(min_v.size() - 1), k(i); j >= 0; --j) {
            out[i] += min_v[j] * max_v[k];
            ++k;
        }
    }
    return out;
}


// full convolution
template <typename Container>
Container conv(Container const& f, Container const& g)
{
    using El = types::index_value_type_t<Container>;

    int const nf = f.size();
    int const ng = g.size();
    int const n = nf + ng - 1;
    //Container out(n, typename Container::value_type());
    Container out(n, El());
    for (auto i(0); i < n; ++i) {
        int const jmn = (i >= ng - 1) ? i - (ng - 1) : 0;
        int const jmx = (i < nf - 1) ? i : nf - 1;
        for (auto j(jmn); j <= jmx; ++j) {
            out[i] += (f[j] * g[i - j]);
        }
    }
    return out;
}


// linspace (erzeugt einen linearen Datenvektor)
template <typename Container>
Container linspace(typename Container::value_type a, typename Container::value_type b, int n)
{
    Container array;
    if (n > 1) {
        typename Container::value_type step = (b - a) / typename Container::value_type(n - 1);
        int count = 0;
        while (count < n) {
            array.push_back(a + count * step);
            ++count;
        }
    } else {
        array.push_back(b);
    }
    return array;
}



// upsconv
template <typename Container>
Container upsconv(Container const& x, Container const& f, int len)
{

    //initialize std::vector dyay
    //typename Container::value_type tmp[x.size() * 2];
    //Container dyay(x.size() * 2);
    Container dyay(x.size() * 2 - 1);

    for (int i = 0, j = 0; i < x.size(); ++i, j = j + 2) {
        dyay[j] = x[i];
        if (j + 1 < dyay.size())
            dyay[j + 1] = 0.0;
    }
    //dyay.pop_back();

    Container cnv = conv(dyay, f);

    int d = (cnv.size() - len) / 2;
    int first = 1 + (cnv.size() - len) / 2;  //floor inclucded
    int last = first + len;

    //cnv.erase(cnv.begin() + last - 1, cnv.end());
    //cnv.erase(cnv.begin(), cnv.begin() + first - 1);
    Container out (len);
    for (int i = first - 1; i < last; ++i)
        out[i - first + 1] = cnv[i];
    //return cnv;
    return out;
}



template <typename Container>
//Container dbwavf(int const wnum, typename Container::value_type returnTypeExample)
Container dbwavf(int const wnum, types::index_value_type_t<Container> returnTypeExample)
{
    static const std::vector<std::function<Container()>> F { []() {
                                                                     Container F0 = {};
                                                                     return F0;
                                                                 },
        []() {
            Container F1 = { 0.50000000000000, 0.50000000000000 };
            return F1;
        },
        []() {
            Container F2 = { 0.34150635094622, 0.59150635094587, 0.15849364905378, -0.09150635094587 };
            return F2;
        },
        []() {
            Container F3 = { 0.23523360389270, 0.57055845791731, 0.32518250026371, -0.09546720778426,
                -0.06041610415535, 0.02490874986589 };
            return F3;
        },
        []() {
            Container F4 = { 0.16290171402562, 0.50547285754565, 0.44610006912319, -0.01978751311791,
                -0.13225358368437, 0.02180815023739, 0.02325180053556, -0.00749349466513 };
            return F4;
        },
        []() {
            Container F5
                = { 0.11320949129173, 0.42697177135271, 0.51216347213016, 0.09788348067375, -0.17132835769133,
                      -0.02280056594205, 0.05485132932108, -0.00441340005433, -0.00889593505093, 0.00235871396920 };
            return F5;
        },
        []() {
            Container F6 = { 0.07887121600143, 0.34975190703757, 0.53113187994121, 0.22291566146505,
                -0.02233187416548, 0.00039162557603, 0.00337803118151, -0.00076176690258, -0.15999329944587,
                -0.09175903203003, 0.06894404648720, 0.01946160485396 };
            return F6;
        },
        []() {
            Container F7 = { 0.05504971537285, 0.28039564181304, 0.51557424581833, 0.33218624110566,
                -0.10175691123173, -0.15841750564054, 0.05042323250485, 0.05700172257986, -0.02689122629486,
                -0.01171997078235, 0.00887489618962, 0.00030375749776, -0.00127395235906, 0.00025011342658 };
            return F7;
        },
        []() {
            Container F8 = { 0.03847781105406, 0.22123362357624, 0.47774307521438, 0.41390826621166,
                -0.01119286766665, -0.20082931639111, 0.00033409704628, 0.09103817842345, -0.01228195052300,
                -0.03117510332533, 0.00988607964808, 0.00618442240954, -0.00344385962813, -0.00027700227421,
                0.00047761485533, -0.00008306863060 };
            return F8;
        },
        []() {
            Container F9 = { 0.02692517479416, 0.17241715192471, 0.42767453217028, 0.46477285717278,
                0.09418477475112, -0.20737588089628, -0.06847677451090, 0.10503417113714, 0.02172633772990,
                -0.04782363205882, 0.00017744640673, 0.01581208292614, -0.00333981011324, -0.00302748028715,
                0.00130648364018, 0.00016290733601, -0.00017816487955, 0.00002782275679 };
            return F9;
        },
        []() {
            Container F10
                = { 0.01885857879640, 0.13306109139687, 0.37278753574266, 0.48681405536610, 0.19881887088440,
                      -0.17666810089647, -0.13855493935993, 0.09006372426666, 0.06580149355070, -0.05048328559801,
                      -0.02082962404385, 0.02348490704841, 0.00255021848393, -0.00758950116768, 0.00098666268244,
                      0.00140884329496, -0.00048497391996, -0.00008235450295, 0.00006617718320, -0.00000937920789 };
            return F10;
        } };

    return F[wnum]();
}



// orthfilt
template <typename Container>
std::tuple<Container, Container, Container, Container> orthfilt(Container const& W_in)
{
    using El = types::index_value_type_t<Container>;

    auto qmf = [](Container const& x) {
        //Container y(x.rbegin(), x.rend());
        Container y(x.size());
        for (size_t i = 0; i<x.size(); ++i)
            y[i] = x[x.size()-1-i];

        auto isEven = [](int n) {
            if (n % 2 == 0)
                return true;
            else
                return false;
        };
        int first;
        if (isEven(y.size())) {
            first = 1;
        } else {
            first = 2;
        }
        for (int i = first; i < y.size(); i = i + 2) {
            y[i] = -y[i];
        }
        return y;
    };
    auto sqrt = [](Container const& x) {
        //Container out;
        //out.reserve(x.size());
        Container out(x.size());
        for (int i = 0; i < x.size(); ++i) {
            //out.push_back(std::sqrt(2) * (x[i]));
            out[i] = std::sqrt(2) * (x[i]);
        }

        return out;
    };

    //typename Container::value_type W_in_sum = std::accumulate(W_in.begin(), W_in.end(), 0);
    El W_in_sum = 0;
    for (size_t i = 0; i<W_in.size(); ++i)
        W_in_sum += W_in[i];

    Container Lo_R = sqrt(W_in);
    Container Hi_R = qmf(Lo_R);
    //Container Hi_D(Hi_R.rbegin(), Hi_R.rend());
    Container Hi_D(Hi_R.size());
    for (size_t i = 0; i<Hi_R.size(); ++i)
        Hi_D[i] = Hi_R[Hi_R.size()-1-i];
    //Container Lo_D(Lo_R.rbegin(), Lo_R.rend());
    Container Lo_D(Lo_R.size());
    for (size_t i = 0; i<Lo_R.size(); ++i)
        Lo_D[i] = Lo_R[Lo_R.size()-1-i];


    return { Lo_D, Hi_D, Lo_R, Hi_R };
}




// dwt
template <typename Container>
std::tuple<Container, Container> dwt(Container const& x, int waveletType)
{
    using El = types::index_value_type_t<Container>;

    //Container F = dbwavf<Container>(waveletType, typename Container::value_type(1.0));
    Container F = dbwavf<Container>(waveletType, El(1.0));

    auto [Lo_D, Hi_D, Lo_R, Hi_R] = orthfilt(F);

    int lf = Lo_D.size();
    int lx = x.size();

    int first = 2;
    int lenEXT = lf - 1;
    int last = lx + lf - 1;

    //Container x_ext;
    //x_ext.reserve(lx + 2 * lenEXT);  // preallocate memory
    Container x_ext(lx + 2 * lenEXT);
    //x_ext.insert(x_ext.end(), x.rbegin() + (lx - lenEXT), x.rend());
    for (int i = 0; i < lenEXT; ++i)
        x_ext[i] = x[lenEXT - 1 - i];
    //x_ext.insert(x_ext.end(), x.begin(), x.end());
    for (int i = 0; i < lx; ++i)
        x_ext[lenEXT + i] = x[i];
    //x_ext.insert(x_ext.end(), x.rbegin(), x.rend() - (lx - lenEXT));
    for (int i = 0; i < lenEXT; ++i)
        x_ext[lenEXT + lx + i] = x[lx - 1 - i];

    Container z1 = conv_valid(x_ext, Lo_D);
    Container z2 = conv_valid(x_ext, Hi_D);
    //Container a;
    //Container d;
    //a.reserve(last);
    //d.reserve(last);
    size_t len = (last - first + 2)/2;
    Container a(len);
    Container d(len);

    size_t cnt = 0;
    for (int i = first - 1; i < last; i = i + 2) {
        //a.push_back(z1[i]);
        //d.push_back(z2[i]);
        a[cnt] = z1[i];
        d[cnt] = z2[i];
        ++cnt;
    }

    return { a, d };
}




template <typename Container>
Container idwt(Container a, Container d, int waveletType, int lx)
{
    using El = types::index_value_type_t<Container>;

    //Container F = dbwavf<Container>(waveletType, typename Container::value_type(1.0));
    Container F = dbwavf<Container>(waveletType, El(1.0));
    auto [Lo_D, Hi_D, Lo_R, Hi_R] = orthfilt(F);

    Container out1 = upsconv(a, Lo_R, lx);
    Container out2 = upsconv(d, Hi_R, lx);
    Container result(out1.size());
    //std::transform(out1.begin(), out1.end(), result.begin(), std::bind(std::plus<typename Container::value_type>(), out2));
    for (size_t i = 0; i<out1.size(); ++i)
        result[i] = out1[i] + out2[i];
    return result;
}


static int wmaxlev(int sizeX, int waveletType)
{
    std::vector<double> F = dbwavf<std::vector<double>>(waveletType, double(1.0));
    auto [Lo_D, Hi_D, Lo_R, Hi_R] = orthfilt(F);

    int lev = (int)(std::log2((double)sizeX / ((double)Lo_D.size() - 1.0)));
    if (lev >= 1) {
        return lev;
    } else {
        return 0;
    }
}




template <typename Container>
std::deque<Container> wavedec(Container const& x, int order, int waveletType)
{
    std::deque<Container> subBands;
    Container d;
    Container x_tmp = x;

    Container zeros(x.size(), 0);

    subBands.push_front(zeros);

    for (int k = 0; k < order; ++k) {
        auto [x_tmp2, d] = dwt(x_tmp, waveletType);
        x_tmp = x_tmp2;
        subBands.push_front(d);
    }
    subBands.push_front(x_tmp);
    return subBands;
}




template <typename Container>
Container waverec(std::deque<Container> const& subBands, int waveletType)
{
    std::deque<Container> a;
    a.push_back(subBands[0]);

    std::vector<int> l_devide;

    for (int i = 0; i < subBands.size(); ++i) {
        l_devide.push_back(subBands[i].size());
    }

    for (int p = 1; p < l_devide.size() - 1; p++) {
        a.push_back(idwt(a[p - 1], subBands[p], waveletType, l_devide[p + 1]));
    }

    return a[a.size() - 1];
}






// 2d functions



template <typename Container>
typename std::enable_if<
 !blaze::IsMatrix<
  Container>::value,
  std::tuple<std::vector<Container>, std::vector<Container>, std::vector<Container>, std::vector<Container>>
 >::type
dwt2(std::vector<Container> const & x, int waveletType)
//template <typename Container>
//std::tuple<std::vector<Container>, std::vector<Container>, std::vector<Container>, std::vector<Container>>
//dwt2(std::vector<Container> const & x, int waveletType)
{
    std::vector<Container> ll, lh, hl, hh, l, h;

    l = std::vector<Container>(x.size());
    h = std::vector<Container>(x.size());
    for (size_t row_idx = 0; row_idx<x.size(); ++row_idx) { // top-level split, by rows
        auto row_split = dwt(x[row_idx], waveletType);
        l[row_idx] = std::get<0>(row_split);
        h[row_idx] = std::get<1>(row_split);
    }
    assert(l[0].size() == h[0].size()); // TODO remove after testing

    //bool vector_empty = true;
    for (size_t col_idx = 0; col_idx<l[0].size(); col_idx++) { // 2 lower level splits, by colmns
        Container l_col (l.size());
        Container h_col (h.size());
        for (size_t row_idx = 0; row_idx<l.size(); ++row_idx) { // we assume sizes of l and r are equal
            l_col[row_idx] = l[row_idx][col_idx];
            h_col[row_idx] = h[row_idx][col_idx];
        }
        {
            auto col_split_l = dwt(l_col, waveletType);
            assert(std::get<0>(col_split_l).size()==std::get<1>(col_split_l).size()); // TODO remove after testing
            //if (vector_empty) {
            if (col_idx < 1) { // first iteration only
                // init
                size_t r_sz = std::get<0>(col_split_l).size();
                ll = std::vector<Container>(r_sz, Container(l[0].size(), 0));
                lh = std::vector<Container>(r_sz, Container(l[0].size(), 0));
                hl = std::vector<Container>(r_sz, Container(l[0].size(), 0));
                hh = std::vector<Container>(r_sz, Container(l[0].size(), 0));
                //vector_empty = false;
            }
            //std::vector<Container> ll_col, lh_col;
            //ll_col.push_back(std::get<0>(col_split_l));
            //lh_col.push_back(std::get<1>(col_split_l));
            for (size_t row_idx = 0; row_idx<std::get<0>(col_split_l).size(); ++row_idx) {
                ll[row_idx][col_idx] = std::get<0>(col_split_l)[row_idx];
                lh[row_idx][col_idx] = std::get<1>(col_split_l)[row_idx];
            }
        } // remove col_split_l from memory
        {
            auto col_split_h = dwt(h_col, waveletType);
            //std::vector<Container> hl_col, hh_col;
            //hl_col.push_back(std::get<0>(col_split_h));
            //hh_col.push_back(std::get<1>(col_split_h));
            assert(std::get<0>(col_split_h).size()==std::get<1>(col_split_h).size()); // TODO remove after testing
            for (size_t row_idx = 0; row_idx<std::get<0>(col_split_h).size(); ++row_idx) {
                hl[row_idx][col_idx] = std::get<0>(col_split_h)[row_idx];
                hh[row_idx][col_idx] = std::get<1>(col_split_h)[row_idx];
            }
        }
    }

    return std::make_tuple(ll, lh, hl, hh);
}


template <typename Container2d>
//std::tuple<Container2d, Container2d, Container2d, Container2d>
typename std::enable_if<
 blaze::IsMatrix<Container2d>::value,
 std::tuple<Container2d, Container2d, Container2d, Container2d>
>::type
dwt2(Container2d const & x, int waveletType) {

    using El = typename Container2d::ElementType; // now we support only Blaze matrices, TODO add type traits, generalize!!
    Container2d ll, lh, hl, hh, l, h; // TODO use sparsed if input is sparsed

    for (size_t row_idx = 0; row_idx<x.rows(); ++row_idx) { // top-level split, by rows
        blaze::DynamicVector<El, blaze::rowVector> curr_row = blaze::row(x, row_idx);
        //auto row_split = dwt(blaze::row(x, row_idx), waveletType); // TODO check if it's possible!!
        auto row_split = dwt(curr_row, waveletType);
        if (row_idx < 1) { // first iteration only
            l = Container2d(x.rows(), std::get<0>(row_split).size());
            h = Container2d(x.rows(), std::get<1>(row_split).size());
        }
        blaze::row(l, row_idx) = std::get<0>(row_split);
        blaze::row(h, row_idx) = std::get<1>(row_split);
    }

    for (size_t col_idx = 0; col_idx<l.columns(); col_idx++) { // 2 lower level splits, by colmns
        blaze::DynamicVector<El> l_col = blaze::column(l, col_idx);
        blaze::DynamicVector<El> h_col = blaze::column(h, col_idx);;
        {
            auto col_split_l = dwt(l_col, waveletType);
            if (col_idx < 1) { // first iteration only
                size_t r_sz = std::get<0>(col_split_l).size();
                ll = Container2d (r_sz, l.columns());
                lh = Container2d (r_sz, l.columns());
                hl = Container2d (r_sz, l.columns());
                hh = Container2d (r_sz, l.columns());
            }
            blaze::column(ll, col_idx) = std::get<0>(col_split_l);
            blaze::column(lh, col_idx) = std::get<1>(col_split_l);
        } // remove col_split_l from memory
        {
            auto col_split_h = dwt(h_col, waveletType);
            blaze::column(hl, col_idx) = std::get<0>(col_split_h);
            blaze::column(hh, col_idx) = std::get<1>(col_split_h);
        }
    }

    return std::make_tuple(ll, lh, hl, hh);

}




template <typename Container>
std::vector<Container> idwt2(
            std::vector<Container> const & ll,
            std::vector<Container> const & lh,
            std::vector<Container> const & hl,
            std::vector<Container> const & hh,
            int waveletType,
            int hx,
            int wx)
{

    assert(ll.size()==lh.size()); // TODO remove after testing and add exception
    assert(ll.size()==hl.size());
    assert(ll.size()==hh.size());
    assert(ll[0].size()==lh[0].size());
    assert(ll[0].size()==hl[0].size());
    assert(ll[0].size()==hh[0].size());

    std::vector<Container> l_colmajor (ll[0].size());
    std::vector<Container> h_colmajor (ll[0].size());
    for (size_t col_idx = 0; col_idx<ll[0].size(); col_idx++) {
        Container col_split_l, col_split_h;
        Container col_ll (ll.size());
        Container col_lh (ll.size());
        Container col_hl (ll.size());
        Container col_hh (ll.size());
        for (size_t row_idx = 0; row_idx<ll.size(); ++row_idx) {
            col_ll[row_idx] = ll[row_idx][col_idx];
            col_lh[row_idx] = lh[row_idx][col_idx];
            col_hl[row_idx] = hl[row_idx][col_idx];
            col_hh[row_idx] = hh[row_idx][col_idx];
        }
        col_split_l = wavelet::idwt(col_ll, col_lh, waveletType, hx);
        l_colmajor[col_idx] = col_split_l;
        col_split_h = wavelet::idwt(col_hl, col_hh, waveletType, hx);
        h_colmajor[col_idx] = col_split_h;
    }

    assert(l_colmajor[0].size()==h_colmajor[0].size());

    // transpose and apply second idwt
    std::vector<Container> out (l_colmajor[0].size());
    for (size_t row_idx = 0; row_idx<l_colmajor[0].size(); ++row_idx) {
        Container row_split_l (l_colmajor.size());
        Container row_split_h (l_colmajor.size());
        for (size_t col_idx = 0; col_idx<l_colmajor.size(); col_idx++) {
            row_split_l[col_idx] = l_colmajor[col_idx][row_idx];
            row_split_h[col_idx] = h_colmajor[col_idx][row_idx];
        }
        //Container row = idwt(row_split_l, row_split_h, waveletType, wx);
        out[row_idx] = idwt(row_split_l, row_split_h, waveletType, wx);
    }

    return out;
}


template <typename Container>
std::vector<Container> idwt2(
            std::tuple<std::vector<Container>, std::vector<Container>, std::vector<Container>, std::vector<Container>> in,
            int waveletType,
            int hx,
            int wx)
{
    return idwt2(std::get<0>(in), std::get<1>(in), std::get<2>(in), std::get<3>(in), waveletType, hx, wx);
}



/* // sequence 1

template <typename Container2d>
Container2d idwt2(
            Container2d const & ll,
            Container2d const & lh,
            Container2d const & hl,
            Container2d const & hh,
            int waveletType,
            int hx,
            int wx)
{
    using El = typename Container2d::ElementType; // now we support only Blaze matrices, TODO add type traits, generalize!!
    // TODO use sparsed if input is sparsed


    assert(ll.rows()==lh.rows()); // TODO replace with exception of nan return
    assert(ll.rows()==hl.rows());
    assert(ll.rows()==hh.rows());
    assert(ll.columns()==lh.columns());
    assert(ll.columns()==hl.columns());
    assert(ll.columns()==hh.columns());

    Container2d l;
    Container2d h;
    for (size_t row_idx = 0; row_idx<ll.rows(); row_idx++) {

        blaze::DynamicVector<El, blaze::rowVector> row_ll = blaze::row(ll, row_idx);
        blaze::DynamicVector<El, blaze::rowVector> row_lh = blaze::row(lh, row_idx);
        blaze::DynamicVector<El, blaze::rowVector> row_hl = blaze::row(hl, row_idx);
        blaze::DynamicVector<El, blaze::rowVector> row_hh = blaze::row(hh, row_idx);

        auto row_split_l = wavelet::idwt(row_ll, row_lh, waveletType, wx);
        auto row_split_h = wavelet::idwt(row_hl, row_hh, waveletType, wx);
        if (row_idx < 1) {
            l = blaze::DynamicMatrix<El>(ll.rows(), row_split_l.size());
            h = blaze::DynamicMatrix<El>(ll.rows(), row_split_h.size());
        }
        blaze::row(l, row_idx) = row_split_l;
        blaze::row(h, row_idx) = row_split_h;
    }

    // second idwt
    blaze::DynamicMatrix<El, blaze::columnMajor> out_col_major; // temporary, TODO replace using type trait
    for (size_t col_idx = 0; col_idx<l.columns(); ++col_idx) {
        blaze::DynamicVector<El> col_split_l (l.rows()); // column vector
        blaze::DynamicVector<El> col_split_h (l.rows());
        for (size_t row_idx = 0; row_idx<l.rows(); row_idx++) { // row-major to column-major, TODO optimize
            col_split_l[row_idx] = l(row_idx, col_idx);
            col_split_h[row_idx] = h(row_idx, col_idx);
        }
        auto curr_column = idwt(col_split_l, col_split_h, waveletType, hx);
        if (col_idx < 1) {
            out_col_major = blaze::DynamicMatrix<El, blaze::columnMajor> (curr_column.size(), l.columns());
        }
        blaze::column(out_col_major, col_idx) = curr_column;
    }
    Container2d out = out_col_major; // col-major to row-major

    return out;
}

// */



template <typename Container2d>
//Container2d idwt2(
typename std::enable_if<blaze::IsMatrix<Container2d>::value, Container2d>::type idwt2(
            Container2d const & ll,
            Container2d const & lh,
            Container2d const & hl,
            Container2d const & hh,
            int waveletType,
            int hx,
            int wx)
{
    using El = typename Container2d::ElementType; // now we support only Blaze matrices, TODO add type traits, generalize!!
    // TODO use sparsed if input is sparsed

    assert(ll.rows()==lh.rows()); // TODO replace with exception of nan return
    assert(ll.rows()==hl.rows());
    assert(ll.rows()==hh.rows());
    assert(ll.columns()==lh.columns());
    assert(ll.columns()==hl.columns());
    assert(ll.columns()==hh.columns());

    blaze::DynamicMatrix<El, blaze::columnMajor> ll_cm = ll; // row-major to col-major // type is temporary, TODO add type trait
    blaze::DynamicMatrix<El, blaze::columnMajor> lh_cm = lh;
    blaze::DynamicMatrix<El, blaze::columnMajor> hl_cm = hl;
    blaze::DynamicMatrix<El, blaze::columnMajor> hh_cm = hh;

    blaze::DynamicMatrix<El, blaze::columnMajor> l_cm;
    blaze::DynamicMatrix<El, blaze::columnMajor> h_cm;
    for (size_t col_idx = 0; col_idx<ll_cm.columns(); col_idx++) {

        blaze::DynamicVector<El> col_ll = blaze::column(ll_cm, col_idx);
        blaze::DynamicVector<El> col_lh = blaze::column(lh_cm, col_idx);
        blaze::DynamicVector<El> col_hl = blaze::column(hl_cm, col_idx);
        blaze::DynamicVector<El> col_hh = blaze::column(hh_cm, col_idx);

        auto col_split_l = wavelet::idwt(col_ll, col_lh, waveletType, hx);
        auto col_split_h = wavelet::idwt(col_hl, col_hh, waveletType, hx);
        if (col_idx < 1) {
            l_cm = blaze::DynamicMatrix<El, blaze::columnMajor>(col_split_l.size(), ll_cm.columns());
            h_cm = blaze::DynamicMatrix<El, blaze::columnMajor>(col_split_h.size(), ll_cm.columns());
        }
        blaze::column(l_cm, col_idx) = col_split_l;
        blaze::column(h_cm, col_idx) = col_split_h;
    }

    Container2d l = l_cm; // col-major to row-major
    Container2d h = h_cm;

    // second idwt
    Container2d out;
    for (size_t row_idx = 0; row_idx<l.rows(); ++row_idx) {
        blaze::DynamicVector<El, blaze::rowVector> row_split_l = blaze::row(l, row_idx);
        blaze::DynamicVector<El, blaze::rowVector> row_split_h = blaze::row(h, row_idx);
        auto curr_row = idwt(row_split_l, row_split_h, waveletType, wx);
        if (row_idx < 1) {
            out = Container2d (l.rows(), curr_row.size());
        }
        blaze::row(out, row_idx) = curr_row;
    }

    return out;
}



// ------------------------------ DWT based on matrix multiplication

template <typename T>
blaze::DynamicMatrix<T> DaubechiesMat(size_t size, int degree = 4) {

    std::vector<T> c (degree);
    c[0] = (1+sqrt(3))/(4*sqrt(2)); // TODO replace hardcode with coeff computation procedure
    c[1] = (3+sqrt(3))/(4*sqrt(2));
    c[2] = (3-sqrt(3))/(4*sqrt(2));
    c[3] = (1-sqrt(3))/(4*sqrt(2));

    auto mat = blaze::DynamicMatrix<T>(size, size, 0);
    size_t split_size = size/2;
    for (size_t i = 0; i < split_size; ++i) {
        int sign = 1;
        for (size_t ci = 0; ci < c.size(); ++ci) {
            mat(i, (i*2 + ci) % size) = c[ci];
            mat(i + split_size, (i*2 + ci) % size) = c[degree - 1 - ci]*sign;
            sign *= -1;
        }
    }

    return mat;
}



template <typename Container2d>
typename std::enable_if<
 blaze::IsMatrix<Container2d>::value,
 std::tuple<Container2d, Container2d, Container2d, Container2d>
>::type
dwt2(Container2d const & x, Container2d const & dmat_w, Container2d const & dmat_h) {

    assert(dmat_w.columns() == dmat_w.rows());
    assert(dmat_h.columns() == dmat_h.rows());
    assert(dmat_w.rows() == x.columns());
    assert(dmat_h.rows() == x.rows());

    using El = typename Container2d::ElementType; // now we support only Blaze matrices, TODO add type traits, generalize!!
    Container2d ll, lh, hl, hh, l, h;
    size_t split_sz_w = dmat_w.columns()/2;
    size_t split_sz_h = dmat_h.columns()/2;
    l = Container2d(x.rows(), split_sz_w);
    h = Container2d(x.rows(), split_sz_w);
    ll = Container2d(split_sz_h, split_sz_w);
    lh = Container2d(split_sz_h, split_sz_w);
    hl = Container2d(split_sz_h, split_sz_w);
    hh = Container2d(split_sz_h, split_sz_w);

    for (size_t row_idx = 0; row_idx<x.rows(); ++row_idx) { // top-level split, by rows
        blaze::DynamicVector<El, blaze::rowVector> curr_row = blaze::row(x, row_idx);
        blaze::DynamicVector<El> row_split = dmat_w*blaze::trans(curr_row);
        blaze::row(l, row_idx) = blaze::trans(blaze::subvector(row_split, 0, split_sz_w));
        blaze::row(h, row_idx) = blaze::trans(blaze::subvector(row_split, split_sz_w, split_sz_w));
    }

    for (size_t col_idx = 0; col_idx<l.columns(); col_idx++) { // 2 lower level splits, by columns
        blaze::DynamicVector<El> l_col = blaze::column(l, col_idx);
        blaze::DynamicVector<El> h_col = blaze::column(h, col_idx);;
        {
            blaze::DynamicVector<El> col_split_l = dmat_h*l_col;
            blaze::column(ll, col_idx) = blaze::subvector(col_split_l, 0, split_sz_h);
            blaze::column(lh, col_idx) = blaze::subvector(col_split_l, split_sz_h, split_sz_h);
        } // remove col_split_l from memory
        {
            blaze::DynamicVector<El> col_split_h = dmat_h*h_col;
            blaze::column(hl, col_idx) = blaze::subvector(col_split_h, 0, split_sz_h);
            blaze::column(hh, col_idx) = blaze::subvector(col_split_h, split_sz_h, split_sz_h);
        }
    }

    return std::make_tuple(ll, lh, hl, hh);
}





//template <typename Container2d>
//typename std::enable_if<blaze::IsMatrix<Container2d>::value, Container2d>::type dwt2_reordered(
//            Container2d const & ll,
//            Container2d const & lh,
//            Container2d const & hl,
//            Container2d const & hh,
//            Container2d const & dmat_w,
//            Container2d const & dmat_h)
//{
//    using El = typename Container2d::ElementType; // now we support only Blaze matrices

//    assert(ll.rows()==lh.rows());
//    assert(ll.rows()==hl.rows());
//    assert(ll.rows()==hh.rows());
//    assert(ll.columns()==lh.columns());
//    assert(ll.columns()==hl.columns());
//    assert(ll.columns()==hh.columns());

//    blaze::DynamicMatrix<El, blaze::columnMajor> ll_cm = ll; // row-major to col-major
//    blaze::DynamicMatrix<El, blaze::columnMajor> lh_cm = lh;
//    blaze::DynamicMatrix<El, blaze::columnMajor> hl_cm = hl;
//    blaze::DynamicMatrix<El, blaze::columnMajor> hh_cm = hh;

//    blaze::DynamicMatrix<El, blaze::columnMajor> col_composed_cm (ll.rows() + lh(rows), ll.columns() + lh.columns());
//    //blaze::DynamicMatrix<El, blaze::columnMajor> h_cm;
//    for (size_t col_idx = 0; col_idx < ll.columns() + lh.columns(); col_idx++) {

//        //blaze::DynamicVector<El> col_ll = blaze::column(ll_cm, col_idx);
//        //blaze::DynamicVector<El> col_lh = blaze::column(lh_cm, col_idx);
//        //blaze::DynamicVector<El> col_hl = blaze::column(hl_cm, col_idx);
//        //blaze::DynamicVector<El> col_hh = blaze::column(hh_cm, col_idx);
//        blaze::DynamicVector<El> col_concat (ll.rows() + lh(rows));
//        blaze::subvector(col_concat, 0, ll.rows()) = ll_cm;
//        blaze::subvector(col_concat, ll.rows(), lh.rows()) = lh_cm;

//        //auto col_split_l = wavelet::idwt(col_ll, col_lh, waveletType, hx);
//        //auto col_split_h = wavelet::idwt(col_hl, col_hh, waveletType, hx);
//        blaze::DynamicVector<El> col_composed_v = dmat_h_t*blaze::trans(col_concat);

//        // till here

//        //if (col_idx < 1) {
//            //col_composed_cm = blaze::DynamicMatrix<El, blaze::columnMajor>(col_composed.size(), ll_cm.columns());
//            //h_cm = blaze::DynamicMatrix<El, blaze::columnMajor>(col_split_h.size(), ll_cm.columns());
//        //}
//        blaze::column(col_composed_cm, col_idx) = col_composed_v;
//        //blaze::column(h_cm, col_idx) = col_split_h;
//    }

//    Container2d col_composed = col_composed_cm; // col-major to row-major
//    //Container2d h = h_cm;

//    // second idwt
//    Container2d out (ll.rows() + lh(rows), ll.columns() + lh.columns());
//    for (size_t row_idx = 0; row_idx<col_composed.rows(); ++row_idx) {
//        blaze::DynamicVector<El, blaze::rowVector> row_split = blaze::row(col_composed, row_idx);
//        //blaze::DynamicVector<El, blaze::rowVector> row_split_h = blaze::row(h, row_idx);
//        //auto curr_row = idwt(row_split, row_split_h, waveletType, wx);
//        blaze::DynamicVector<El> curr_row = dmat_w_t*blaze::trans(row_split);
//        //if (row_idx < 1) {
//            //out = Container2d (col_composed.rows(), curr_row.size());
//        //}
//        blaze::row(out, row_idx) = blaze::trans(curr_row);
//    }

//    return out;
//}




}  // end namespace
#endif
