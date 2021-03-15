/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019  Michael Welsch
*/

#ifndef _METRIC_HELPER_FUNCTIONS_CPP
#define _METRIC_HELPER_FUNCTIONS_CPP

#include <algorithm>

namespace helper_functions {



template <typename T, bool SO>
void shrinkToFit(blaze::CompressedVector<T, SO>& mat)
{
    blaze::CompressedVector<T>(~mat).swap(~mat);
}


template <typename T>
T Lerp(T v0, T v1, T t)
{
    return (1 - t) * v0 + t * v1;
}


template <typename Container>
Container quantile(Container data, const Container & probs)
{

    using T = typename Container::value_type;

    if (data.empty()) {
        return Container();
    }

    if (1 == data.size()) {
        return Container(1, data[0]);
    }

    std::sort(data.begin(), data.end());
    Container quantiles;

    for (int i = 0; i < probs.size(); ++i) {
        T poi = Lerp<T>(-0.5, data.size() - 0.5, probs[i]);

        int left = std::max(int(std::floor(poi)), int(0));
        int right = std::min(int(std::ceil(poi)), int(data.size() - 1));

        T datLeft = data[left];
        T datRight = data[right];

        T quantile = Lerp(datLeft, datRight, poi - left);

        quantiles.push_back(quantile);
    }

    return quantiles;
}



// linear interpolation
template <typename Container>
Container lininterp(const Container& x, const Container& y, Container xi, bool extrapolate = true)
{
    using T = typename Container::value_type;

    int n = x.size();
    Container yi(xi.size());
    for (int ii = 0; ii < xi.size(); ++ii) {
        int i = 0;  // find left end of interval for interpolation
        if (xi[ii] >= x[n - 2])  // special case: beyond right end
        {
            i = n - 2;
        } else {
            while (xi[ii] > x[i + 1])
                i++;
        }
        T xL = x[i], yL = y[i], xR = x[i + 1], yR = y[i + 1];  // points on either side (unless beyond ends)
        if (!extrapolate)  // if beyond ends of array and not extrapolating
        {
            if (xi[ii] < xL)
                yR = yL;
            if (xi[ii] > xR)
                yL = yR;
        }

        T dydx = (yR - yL) / (xR - xL);  // gradient
        yi.push_back(yL + dydx * (xi[ii] - xL));
    }
    return yi;  // linear interpolation
}




// akima interpolation
/*
Ref. : Hiroshi Akima, Journal of the ACM, Vol. 17, No. 4, October 1970,
      pages 589-602.
*/
template <typename Container>
Container akima(Container const& x, Container const& y, Container const& xi, bool save_Mode = true)
{
    using T = typename Container::value_type;

    // check inputs
    if (save_Mode) {
        auto diff = [](Container const& x) {
            Container v(x.size() - 1);
            for (int i = 1; i < x.size(); ++i) {
                v[i - 1] = x[i] - x[i - 1];
            }
            return v;
        };
        Container d = diff(x);

        if (x.size() != y.size())
            std::cout << "Error in rts::akima ==> input vectors must have the same length" << std::endl;
        if (!std::is_sorted(xi.begin(), xi.end()))
            std::cout << "Error in rts::akima ==> xi values do not have ascending order" << std::endl;
        if (!std::is_sorted(x.begin(), x.end()))
            std::cout << "Error in rts::akima ==> x values do not have ascending order" << std::endl;
        if (std::find_if(d.begin(), d.end(), [](T m) { return (m <= T(0)); }) != d.end())
            std::cout << "Error in rts::akima ==> x values contain distinct values" << std::endl;
    }

    //calculate u vector
    auto uVec = [](Container const& x, Container const& y) {
        int n = x.size();
        Container u((n + 3));
        for (int i = 1; i < n; ++i) {
            u[i + 1] = (y[i] - y[i - 1]) / (x[i] - x[i - 1]);  // Shift i to i+2
        }

        auto akima_end = [](const T& u1, const T& u2) { return 2.0 * u1 - u2; };

        u[1] = akima_end(u[2], u[3]);
        u[0] = akima_end(u[1], u[2]);
        u[n + 1] = akima_end(u[n], u[n - 1]);
        u[n + 2] = akima_end(u[n + 1], u[n]);

        return u;
    };
    Container u = uVec(x, y);

    // calculate yp vector
    Container yp(x.size());
    for (int i = 0; i < x.size(); ++i) {
        auto a = std::abs(u[i + 3] - u[i + 2]);
        auto b = std::abs(u[i + 1] - u[i]);
        if ((a + b) != 0) {
            yp[i] = (a * u[i + 1] + b * u[i + 2]) / (a + b);
        } else {
            yp[i] = (u[i + 2] + u[i + 1]) / 2.0;
        }
    }

    // calculte interpolated yi values
    auto kFind = [](const T& xii, const Container& x, int start, int end) {
        int klo = start;
        int khi = end;
        // // Find subinterval by bisection
        while (khi - klo > 1) {
            int k = (khi + klo) / 2;
            x[k] > xii ? khi = k : klo = k;
        }
        return klo;
    };

    Container yi(xi.size());
    for (int i = 0; i < xi.size(); ++i) {
        // Find the right place in the table by means of a bisection.
        int k = kFind(xi[i], x, int(0), x.size() - 1);

        // Evaluate Akima polynomial
        T b = x[k + 1] - x[k];
        T a = xi[i] - x[k];
        yi[i] = y[k] + yp[k] * a + (3.0 * u[k + 2] - 2.0 * yp[k] - yp[k + 1]) * a * a / b
            + (yp[k] + yp[k + 1] - 2.0 * u[k + 2]) * a * a * a / (b * b);
    }
    return yi;
}




// pchip interpolation
/*!
  // Reference:
  // ==========
  //
  //    F.N. Fritsch, R.E. Carlson:
  //    Monotone Piecewise Cubic Interpolation,
  //    SIAM J. Numer. Anal. Vol 17, No. 2, April 1980
  //
  //    F.N. Fritsch and J. Butland:
  //    A method for constructing local monotone piecewise cubic interpolants,
  //    SIAM Journal on Scientific and Statistical Computing 5, 2 (June 1984), pp. 300-304.
  */
template <typename Container>
Container pchip(Container const& x, Container const& y, Container const& xi, bool save_Mode = false)
{
    using T = typename Container::value_type;


    // check inputs
    if (save_Mode) {
        auto diff = [](Container const& x) {
            Container v(x.size() - 1);
            for (int i = 1; i < x.size(); ++i) {
                v[i - 1] = x[i] - x[i - 1];
            }
            return v;
        };
        Container d = diff(x);

        if (x.size() != y.size())
            std::cout << "Error in rts::pchip ==> input vectors must have the same length" << std::endl;
        if (!std::is_sorted(xi.begin(), xi.end()))
            std::cout << "Error in rts::pchip ==> xi values do not have ascending order" << std::endl;
        if (!std::is_sorted(x.begin(), x.end()))
            std::cout << "Error in rts::pchip ==> x values do not have ascending order" << std::endl;
        if (std::find_if(d.begin(), d.end(), [](T m) { return (m <= T(0)); }) != d.end())
            std::cout << "Error in rts::pchip ==> x values contain distinct values" << std::endl;
    }

    //int n = xi.size();

    auto diff = [](Container const& x) {
        Container v(x.size() - 1);
        for (int i = 1; i < x.size(); ++i) {
            v[i - 1] = x[i] - x[i - 1];
        }
        return v;
    };

    // First derivatives
    Container h = diff(x);

    Container delta(xi.size() - 1);
    for (int i = 1; i < xi.size(); ++i) {
        delta[i - 1] = (y[i] - y[i - 1]) / (x[i] - x[i - 1]);  // Shift i to i+2
    }

    auto pchip_slopes = [](Container const& h, Container const& delta) {
        // Slopes at interior points
        int n = h.size() + 1;
        Container d(n, 0);

        auto sign = [](T val) { return (T(0) < val) - (val < T(0)); };

        std::vector<int> k;
        for (int i = 0; i < n - 1; ++i) {
            if ((sign(delta[i]) * sign(delta[i + 1])) > 0.0) {
                k.push_back(i);
            }
        }

        for (int i = 0; i < k.size(); ++i) {
            T hs = h[k[i]] + h[k[i] + 1];
            T w1 = (h[k[i]] + hs) / (3 * hs);
            T w2 = (hs + h[k[i] + 1]) / (3 * hs);
            T dmax = std::max(std::abs(delta[k[i]]), std::abs(delta[k[i] + 1]));
            T dmin = std::min(std::abs(delta[k[i]]), std::abs(delta[k[i] + 1]));
            d[k[i] + 1] = dmin / (w1 * (delta[k[i]] / dmax) + w2 * (delta[k[i] + 1] / dmax));
        }

        auto pchip_end = [](const T& h1, const T& h2, const T& del1, const T& del2) {
            auto sign = [](T val) { return (T(0) < val) - (val < T(0)); };
            // Noncentered, shape-preserving, three-point formula.
            T d;
            if (sign(d) != sign(del1)) {
                d = 0;
            } else if ((sign(del1) != sign(del2)) && (std::abs(d) > std::abs(3 * del1))) {
                d = 3.0 * del1;
            } else {
                d = ((2.0 * h1 + h2) * del1 - h1 * del2) / (h1 + h2);
            }

            return d;
        };

        // Slopes at endpoints
        d[0] = pchip_end(h[0], h[1], delta[0], delta[1]);
        d[n - 1] = pchip_end(h[n - 2], h[n - 3], delta[n - 2], delta[n - 3]);

        return d;
    };

    // Derivative values for shape-preserving Piecewise Cubic Hermite Interpolation
    Container d = pchip_slopes(h, delta);

    // Piecewise polynomial coefficients
    Container a(h.size() - 1);
    Container b(h.size() - 1);
    for (int i = 0; i < h.size() - 1; ++i) {
        a[i] = (3.0 * delta[i] - 2.0 * d[i] - d[i + 1]) / h[i];
        b[i] = (d[i] - 2.0 * delta[i] + d[i + 1]) / (h[i] * h[i]);
    }

    // calculte interpolated yi values
    auto kFind = [](const T& xii, const Container& x, int start, int end) {
        int klo = start;
        int khi = end;
        // // Find subinterval by bisection
        while (khi - klo > 1) {
            int k = (khi + klo) / 2;
            x[k] > xii ? khi = k : klo = k;
        }
        return klo;
    };

    Container yi(xi.size());
    for (int i = 0; i < xi.size(); ++i) {
        int k = kFind(xi[i], x, int(1), x.size() - 2);
        T s = xi[i] - x[k];
        yi[i] = y[k] + s * (d[k] + s * (a[k] + s * b[k]));
    }

    return yi;
}





template <typename Container>
blaze::CompressedVector<typename Container::value_type> smoothDenoise(Container const& data, typename Container::value_type const& tresh)
{
    //smooth reduces of noise by threshold and gives back a sparse vector.
    // initialize

    using T = typename Container::value_type;

    blaze::CompressedVector<T> svector(data.size());
    svector.reserve(data.size());

    bool lastEqualsZero;
    bool keepNext;

    lastEqualsZero = false;
    keepNext = false;
    for (int i = 0; i < data.size(); i++) {
        if (data[i] != T(0)) {
            if (std::abs(data[i]) > tresh) {  //größer als Schwellwert
                if (lastEqualsZero == true) {

                    svector.set(i - 1,
                        data[i - 1]);  // letzten Wert doch nicht zu Null setzen, wenn der aktuelle nicht null ist
                    lastEqualsZero = false;
                }

                svector.append(i, data[i]);
                keepNext = true;
            } else {  // unterhalb des Schwellwertes
                if (keepNext == true) {

                    svector.append(i, data[i]);
                }

                lastEqualsZero = true;
                keepNext = false;
            }
        }
    }

    shrinkToFit(svector);

    return svector;
}





template <typename T>
std::vector<T> sparseToVector(blaze::CompressedVector<T> const& data)
{

    std::vector<T> values_zeropadded;

    T value;
    bool addZeroFront;
    bool addZeroLastBack;
    int index;
    int index_last = -1;

    for (blaze::CompressedVector<double>::ConstIterator it = data.cbegin(); it != data.cend(); ++it) {
        index = it->index();  // Read access to the index of the non-zero element.
        value = it->value();  // Read access to the value of the non-zero element.

        if (index == index_last + 1) {
            addZeroFront = false;
        } else {
            addZeroFront = true;
        }

        if (index > index_last + 1 && index != 1 && index != index_last + 2) {
            addZeroLastBack = true;
        } else {
            addZeroLastBack = false;
        }

        if (addZeroLastBack == true) {
            values_zeropadded.push_back(0);
        }
        if (addZeroFront == true) {
            values_zeropadded.push_back(0);
        }

        values_zeropadded.push_back(value);

        index_last = index;
    }

    if (index_last < data.size() - 2)  // vorletzter nicht vorhanden
    {
        values_zeropadded.push_back(0);
    }
    if (index_last < data.size() - 1) {
        values_zeropadded.push_back(0);
    }

    return values_zeropadded;
}






template <typename Container>
Container sparseToContainer(blaze::CompressedVector<typename Container::value_type> const& data)
{
    using T = typename Container::value_type;

    Container values_zeropadded;

    T value;
    bool addZeroFront;
    bool addZeroLastBack;
    int index;
    int index_last = -1;

    for (blaze::CompressedVector<double>::ConstIterator it = data.cbegin(); it != data.cend(); ++it) {
        index = it->index();  // Read access to the index of the non-zero element.
        value = it->value();  // Read access to the value of the non-zero element.

        if (index == index_last + 1) {
            addZeroFront = false;
        } else {
            addZeroFront = true;
        }

        if (index > index_last + 1 && index != 1 && index != index_last + 2) {
            addZeroLastBack = true;
        } else {
            addZeroLastBack = false;
        }

        if (addZeroLastBack == true) {
            values_zeropadded.push_back(0);
        }
        if (addZeroFront == true) {
            values_zeropadded.push_back(0);
        }

        values_zeropadded.push_back(value);

        index_last = index;
    }

    if (index_last < data.size() - 2)  // vorletzter nicht vorhanden
    {
        values_zeropadded.push_back(0);
    }
    if (index_last < data.size() - 1) {
        values_zeropadded.push_back(0);
    }

    return values_zeropadded;
}





template <typename T>
blaze::CompressedVector<T> zeroPad(blaze::CompressedVector<T> const& data)
{
    // adds zero pads to blaze::sparsevector (for preparing sed)
    blaze::CompressedVector<T> data_zeropadded(data.size());
    data_zeropadded.reserve(2 + data.nonZeros() * 2);
    T value;
    bool addZeroFront;
    bool addZeroLastBack;
    int index;
    int index_last = -1;

    if (data.nonZeros() == 0) {
        data_zeropadded.set(0, T(0));
        data_zeropadded.set(data.size() - 1, T(0));
    } else {

        for (blaze::CompressedVector<double>::ConstIterator it = data.cbegin(); it != data.cend(); ++it) {
            index = it->index();  // Read access to the index of the non-zero element.
            value = it->value();  // Read access to the value of the non-zero element.

            if (index == index_last + 1)
                addZeroFront = false;
            else
                addZeroFront = true;

            if (index > index_last + 1 && index != 1 && index != index_last + 2)
                addZeroLastBack = true;
            else
                addZeroLastBack = false;

            if (addZeroLastBack == true)
                data_zeropadded.append(index_last + 1, T(0));

            if (addZeroFront == true)
                data_zeropadded.append(index - 1, T(0));

            data_zeropadded.append(index, value);
            index_last = index;
        }

        if (index_last < data.size() - 2)  // vorletzter nicht vorhanden
        {
            data_zeropadded.append(index_last + 1, T(0));
        }
        if (index_last < data.size() - 1) {
            data_zeropadded.append(data.size() - 1, T(0));
        }
    }

    shrinkToFit(data_zeropadded);
    return data_zeropadded;
}







// distance measure by time elastic cost matrix.
template <typename T>
T TWED(blaze::CompressedVector<T> const& As, blaze::CompressedVector<T> const& Bs, T const& penalty, T const& elastic)
{
    // calculates the Time Warp Edit Distance (TWED) for the sparse vectors A(time) und B(time)
    //
    // A      := values of timeseries A (e.g. [ 10 2 30 4])
    // B      := values of timeseries B
    // time   := time values

    // initialize

    //build zero padded vectors
    std::vector<T> A;
    A.reserve(As.nonZeros());
    std::vector<T> timeA;
    timeA.reserve(As.nonZeros());

    std::vector<T> B;
    B.reserve(Bs.nonZeros());

    std::vector<T> timeB;
    timeB.reserve(Bs.nonZeros());

    for (blaze::CompressedVector<double>::ConstIterator it = As.cbegin(); it != As.cend(); ++it) {
        timeA.push_back(it->index());  // Read access to the index of the non-zero element.
        A.push_back(it->value());  // Read access to the value of the non-zero element.
    }

    for (blaze::CompressedVector<double>::ConstIterator it = Bs.cbegin(); it != Bs.cend(); ++it) {
        timeB.push_back(it->index());  // Read access to the index of the non-zero element.
        B.push_back(it->value());  // Read access to the value of the non-zero element.
    }

    T C1, C2, C3;

    int sizeB = B.size();
    int sizeA = A.size();

    std::vector<T> D0(sizeB);
    std::vector<T> Di(sizeB);

    // first element
    D0[0] = std::abs(A[0] - B[0]) + elastic * (std::abs(timeA[0] - 0));  // C3

    // first row
    for (int j = 1; j < sizeB; j++) {
        D0[j] = D0[j - 1] + std::abs(B[j - 1] - B[j]) + elastic * (timeB[j] - timeB[j - 1]) + penalty;  // C2
    }

    // second-->last row
    for (int i = 1; i < sizeA; i++) {
        // every first element in row
        Di[0] = D0[0] + std::abs(A[i - 1] - A[i]) + elastic * (timeA[i] - timeA[i - 1]) + penalty;  // C1

        // remaining elements in row
        for (int j = 1; j < sizeB; j++) {
            C1 = D0[j] + std::abs(A[i - 1] - A[i]) + elastic * (timeA[i] - timeA[i - 1]) + penalty;
            C2 = Di[j - 1] + std::abs(B[j - 1] - B[j]) + elastic * (timeB[j] - timeB[j - 1]) + penalty;
            C3 = D0[j - 1] + std::abs(A[i] - B[j]) + std::abs(A[i - 1] - B[j - 1])
                + elastic * (std::abs(timeA[i] - timeB[j]) + std::abs(timeA[i - 1] - timeB[j - 1]));
            Di[j] = (C1 < ((C2 < C3) ? C2 : C3)) ? C1 : ((C2 < C3) ? C2 : C3);  //Di[j] = std::min({C1,C2,C3});
        }
        std::swap(D0, Di);
    }

    T rvalue = D0[sizeB - 1];

    return rvalue;
}




// rainflow counting algorithm
template <typename Container>
std::vector<Container> rfc(Container const& data, Container const& time_in)
{
    using T = typename Container::value_type;

    auto findIndexOfTurningPoints = [](auto s) {
        std::vector<int> tp;
        tp.push_back(s[0]);  // first value is set as extreme

        auto diff_last = s[1] - s[0];
        for (int i = 1; i < s.size(); ++i) {
            auto diff = s[i] - s[i - 1];
            if (diff * diff_last < 0) {
                tp.push_back(i - 1);
            }
            diff_last = diff;
        }
        if (tp[tp.size() - 1] != s.size() - 1)  // last value is set as extreme
        {
            tp.push_back(s.size() - 1);
        }
        return tp;
    };

    auto ext_index = findIndexOfTurningPoints(data);

    Container ext;
    //ext.reserve(ext_index.size());

    for (int i = 0; i < ext_index.size(); ++i) {
        ext.push_back(data[ext_index[i]]);
    }

    std::vector<Container> cycles;

    int i = 0;
    int j = 1;
    while (ext.size() > (i + 1)) {
        T Y = std::abs(ext[i + 1] - ext[i]);
        T X = std::abs(ext[j + 1] - ext[j]);
        if (X >= Y) {
            if (i == 0)  // counts a half cycle and deletes the poit that is counted
            {
                Container cycle_info = { std::abs(ext[i] - ext[i + 1]) / 2, (ext[i] + ext[i + 1]) / 2, 0.5 };
                cycles.push_back(cycle_info);
                ext.erase(ext.begin());
            } else  //counts one cycle and deletes the poits that are counted
            {
                Container cycle_info = { std::abs(ext[i] - ext[i + 1]) / 2, (ext[i] + ext[i + 1]) / 2, 1.0 };
                cycles.push_back(cycle_info);
                ext.erase(ext.begin() + i, ext.begin() + i + 1 + 1);
            }
            i = 0;
            j = 1;
        } else {
            i += 1;
            j += 1;
        }
    }
    for (int i = 0; i < ext.size() - 1;
         ++i)  // counts the rest of the points that still not have been counted as a half cycle
    {

        Container cycle_info = { std::abs(ext[i] - ext[i + 1]) / 2, (ext[i] + ext[i + 1]) / 2, 0.5 };
        cycles.push_back(cycle_info);
    }
    return cycles;
}






// get subband stats
template <typename Container>
Container chebyshev(Container const& data, int polynom)
{
    using T = typename Container::value_type;

    Container r_data = {};

    static std::vector<std::function<T(T)>> chebyshevPolyFun {
        [](auto x) { return 1; },  // T0
        [](auto x) { return x; },  // T1
        [](auto x) { return 2 * std::pow(x, 2) - 1; },  // T2
        [](auto x) { return 4 * std::pow(x, 3) - 3 * x; },  // T3
        [](auto x) { return 8 * std::pow(x, 4) - 8 * std::pow(x, 2) + 1; },  // T4
        [](auto x) { return 16 * std::pow(x, 5) - 20 * std::pow(x, 3) + 5 * x; },  // T5
        [](auto x) { return 32 * std::pow(x, 6) - 48 * std::pow(x, 4) + 18 * std::pow(x, 2) - 1; },  // T6
        [](auto x) { return 64 * std::pow(x, 7) - 112 * std::pow(x, 5) + 56 * std::pow(x, 3) - 7 * x; }  // T7
    };
    for (int i = 0; i < data.size(); ++i) {
        r_data.push_back(chebyshevPolyFun[polynom](data[i]));
    }
    return r_data;
}




template <typename T>
T nextpow2(T value)
{
    int exp;
    if (std::frexp(value, &exp) == 0.5) {
        // Omit this case to round precise powers of two up to the *next* power
        return value;
    }
    return std::ldexp(T(1), exp);
}





template <typename Container>
Container rescale(Container const& A, typename Container::value_type a, typename Container::value_type b)
{
    using T = typename Container::value_type;

    Container R(A.size());

    T inputMin = *std::min_element(std::begin(A), std::end(A));
    T inputMax = *std::max_element(std::begin(A), std::end(A));
    bool constReg = (inputMin == inputMax);

    T sigma = std::max(std::min(T(0), inputMax), inputMin);
    inputMin = inputMin - sigma;
    inputMax = inputMax - sigma;

    T e1 = nextpow2(std::max(std::abs(inputMax), std::abs(inputMin)));
    T r1 = std::pow(2, (e1 - 1));
    T e2 = nextpow2(std::max(std::abs(a), std::abs(b)));
    T r2 = std::pow(2, (e2 - 1));
    T r3 = std::pow(2, (std::floor((e1 + e2) / 2) - 1));

    T z = ((inputMax / r1) * (a / r3) - (inputMin / r1) * (b / r3) + (a / r3) * (T(constReg) / r1))
        / ((inputMax / r1) - (inputMin / r1) + (T(constReg) / r1));
    T slope = ((b / r2) - (a / r2)) / ((inputMax / r3) - (inputMin / r3) + (T(constReg) / r3));

    for (int i = 0; i < A.size(); ++i) {
        R[i] = r2 * (slope / r3 * (A[i] - sigma) + (r3 / r2) * z);
    }
    return R;
}









}

#endif
