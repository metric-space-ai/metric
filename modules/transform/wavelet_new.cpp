/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019  Michael Welsch
*/

#ifndef _METRIC_TRANSFORM_WAVELET_NEW_CPP
#define _METRIC_TRANSFORM_WAVELET_NEW_CPP
#include "wavelet_new.hpp"
#include <type_traits>
#include <memory>

/********************/
// vector operation overloads

template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b)
{
    assert(a.size() == b.size());
    std::vector<T> result(a.size());
    std::transform(a.begin(), a.end(), b.begin(), result.begin(), std::plus<T>());
    return result;
}

template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const T& b)
{
    std::vector<T> result(a.size());
    std::transform(a.begin(), a.end(), result.begin(), std::bind(std::plus<T>(), b));
    return result;
}
template <typename T>
std::vector<T> operator+(const T& b, const std::vector<T>& a)
{
    std::vector<T> result(a.size());
    std::transform(a.begin(), a.end(), result.begin(), std::bind(std::plus<T>(), b));
    return result;
}

// - -------------------------
template <typename T>
std::vector<T> operator-(const std::vector<T>& a, const std::vector<T>& b)
{
    assert(a.size() == b.size());
    std::vector<T> result(a.size());
    std::transform(a.begin(), a.end(), b.begin(), result.begin(), std::minus<T>());
    return result;
}
template <typename T>
std::vector<T> operator-(const std::vector<T>& a, const T& b)
{
    std::vector<T> result(a.size());
    std::transform(a.begin(), a.end(), result.begin(), std::bind(std::minus<T>(), b));
    return result;
}
template <typename T>
std::vector<T> operator-(const T& b, const std::vector<T>& a)
{
    std::vector<T> result(a.size());
    std::transform(a.begin(), a.end(), result.begin(), std::bind(std::minus<T>(), b));
    return result;
}

// * -------------------------
template <typename T>
std::vector<T> operator*(const std::vector<T>& a, const std::vector<T>& b)
{
    assert(a.size() == b.size());
    std::vector<T> result(a.size());
    std::transform(a.begin(), a.end(), b.begin(), result.begin(), std::multiplies<T>());
    return result;
}
template <typename T>
std::vector<T> operator*(const std::vector<T>& a, const T& b)
{
    std::vector<T> result(a.size());
    std::transform(a.begin(), a.end(), result.begin(), std::bind(std::multiplies<T>(), b));
    return result;
}
template <typename T>
std::vector<T> operator*(const T& b, const std::vector<T>& a)
{
    std::vector<T> result(a.size());
    std::transform(a.begin(), a.end(), result.begin(), std::bind(std::multiplies<T>(), b));
    return result;
}

// / -------------------------
template <typename T>
std::vector<T> operator/(const std::vector<T>& a, const std::vector<T>& b)
{
    assert(a.size() == b.size());
    std::vector<T> result(a.size());
    std::transform(a.begin(), a.end(), b.begin(), result.begin(), std::divides<T>());
    return result;
}
template <typename T>
std::vector<T> operator/(const std::vector<T>& a, const T& b)
{
    std::vector<T> result(a.size());
    std::transform(a.begin(), a.end(), result.begin(), std::bind(std::divides<T>(), b));
    return result;
}
template <typename T>
std::vector<T> operator/(const T& b, const std::vector<T>& a)
{
    std::vector<T> result(a.size());
    std::transform(a.begin(), a.end(), result.begin(), std::bind(std::divides<T>(), b));
    return result;
}

/**************************/

namespace wavelet {

template <typename T, bool SO>
void shrinkToFit(blaze::CompressedVector<T, SO>& mat)
{
    blaze::CompressedVector<T>(~mat).swap(~mat);
}

// valid convolution
template <typename T>
std::vector<T> conv_valid(std::vector<T> const& f, std::vector<T> const& g)
{
    int const nf = f.size();
    int const ng = g.size();
    std::vector<T> const& min_v = (nf < ng) ? f : g;
    std::vector<T> const& max_v = (nf < ng) ? g : f;
    int const n = std::max(nf, ng) - std::min(nf, ng) + 1;
    std::vector<T> out(n, T());
    for (auto i(0); i < n; ++i) {
        for (int j(min_v.size() - 1), k(i); j >= 0; --j) {
            out[i] += min_v[j] * max_v[k];
            ++k;
        }
    }
    return out;
}

// valid convolution, overload added by Max F
template <typename Container>
Container conv_valid(Container const& f, Container const& g)
{
    int const nf = f.size();
    int const ng = g.size();
    Container const& min_v = (nf < ng) ? f : g;
    Container const& max_v = (nf < ng) ? g : f;
    int const n = std::max(nf, ng) - std::min(nf, ng) + 1;
    Container out(n, typename Container::value_type());
    for (auto i(0); i < n; ++i) {
        for (int j(min_v.size() - 1), k(i); j >= 0; --j) {
            out[i] += min_v[j] * max_v[k];
            ++k;
        }
    }
    return out;
}

// full convolution
template <typename T>
std::vector<T> conv(std::vector<T> const& f, std::vector<T> const& g)
{
    int const nf = f.size();
    int const ng = g.size();
    int const n = nf + ng - 1;
    std::vector<T> out(n, T());
    for (auto i(0); i < n; ++i) {
        int const jmn = (i >= ng - 1) ? i - (ng - 1) : 0;
        int const jmx = (i < nf - 1) ? i : nf - 1;
        for (auto j(jmn); j <= jmx; ++j) {
            out[i] += (f[j] * g[i - j]);
        }
    }
    return out;
}

// full convolution, overload added by Max F
template <typename Container>
Container conv(Container const& f, Container const& g)
{
    int const nf = f.size();
    int const ng = g.size();
    int const n = nf + ng - 1;
    Container out(n, typename Container::value_type());
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
template <typename T>
std::vector<T> linspace(T a, T b, int n)
{
    std::vector<T> array;
    if (n > 1) {
        T step = (b - a) / T(n - 1);
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

template <typename T>
T Lerp(T v0, T v1, T t)
{
    return (1 - t) * v0 + t * v1;
}

template <typename T>
std::vector<T> quantile(std::vector<T> data, const std::vector<T>& probs)
{

    if (data.empty()) {
        return std::vector<T>();
    }

    if (1 == data.size()) {
        return std::vector<T>(1, data[0]);
    }

    std::sort(data.begin(), data.end());
    std::vector<T> quantiles;

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
template <typename T>
std::vector<T> lininterp(const std::vector<T>& x, const std::vector<T>& y, std::vector<T> xi, bool extrapolate = true)
{
    int n = x.size();
    std::vector<T> yi(xi.size());
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
template <typename T>
std::vector<T> akima(std::vector<T> const& x, std::vector<T> const& y, std::vector<T> const& xi, bool save_Mode = true)
{
    // check inputs
    if (save_Mode) {
        auto diff = [](std::vector<T> const& x) {
            std::vector<T> v(x.size() - 1);
            for (int i = 1; i < x.size(); ++i) {
                v[i - 1] = x[i] - x[i - 1];
            }
            return v;
        };
        std::vector<T> d = diff(x);

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
    auto uVec = [](std::vector<T> const& x, std::vector<T> const& y) {
        int n = x.size();
        std::vector<T> u((n + 3));
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
    std::vector<T> u = uVec(x, y);

    // calculate yp vector
    std::vector<T> yp(x.size());
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
    auto kFind = [](const T& xii, const std::vector<T>& x, int start, int end) {
        int klo = start;
        int khi = end;
        // // Find subinterval by bisection
        while (khi - klo > 1) {
            int k = (khi + klo) / 2;
            x[k] > xii ? khi = k : klo = k;
        }
        return klo;
    };

    std::vector<T> yi(xi.size());
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
template <typename T>
std::vector<T> pchip(std::vector<T> const& x, std::vector<T> const& y, std::vector<T> const& xi, bool save_Mode = false)
{
    // check inputs
    if (save_Mode) {
        auto diff = [](std::vector<T> const& x) {
            std::vector<T> v(x.size() - 1);
            for (int i = 1; i < x.size(); ++i) {
                v[i - 1] = x[i] - x[i - 1];
            }
            return v;
        };
        std::vector<T> d = diff(x);

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

    auto diff = [](std::vector<T> const& x) {
        std::vector<T> v(x.size() - 1);
        for (int i = 1; i < x.size(); ++i) {
            v[i - 1] = x[i] - x[i - 1];
        }
        return v;
    };

    // First derivatives
    std::vector<T> h = diff(x);

    std::vector<T> delta(xi.size() - 1);
    for (int i = 1; i < xi.size(); ++i) {
        delta[i - 1] = (y[i] - y[i - 1]) / (x[i] - x[i - 1]);  // Shift i to i+2
    }

    auto pchip_slopes = [](std::vector<T> const& h, std::vector<T> const& delta) {
        // Slopes at interior points
        int n = h.size() + 1;
        std::vector<T> d(n, 0);

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
    std::vector<T> d = pchip_slopes(h, delta);

    // Piecewise polynomial coefficients
    std::vector<T> a(h.size() - 1);
    std::vector<T> b(h.size() - 1);
    for (int i = 0; i < h.size() - 1; ++i) {
        a[i] = (3.0 * delta[i] - 2.0 * d[i] - d[i + 1]) / h[i];
        b[i] = (d[i] - 2.0 * delta[i] + d[i + 1]) / (h[i] * h[i]);
    }

    // calculte interpolated yi values
    auto kFind = [](const T& xii, const std::vector<T>& x, int start, int end) {
        int klo = start;
        int khi = end;
        // // Find subinterval by bisection
        while (khi - klo > 1) {
            int k = (khi + klo) / 2;
            x[k] > xii ? khi = k : klo = k;
        }
        return klo;
    };

    std::vector<T> yi(xi.size());
    for (int i = 0; i < xi.size(); ++i) {
        int k = kFind(xi[i], x, int(1), x.size() - 2);
        T s = xi[i] - x[k];
        yi[i] = y[k] + s * (d[k] + s * (a[k] + s * b[k]));
    }

    return yi;
}

// upsconv
template <typename T>
std::vector<T> upsconv(std::vector<T> const& x, std::vector<T> const& f, int len)
{

    //initialize std::vector dyay
    T tmp[x.size() * 2];
    std::vector<T> dyay(tmp, tmp + x.size() * 2);

    for (int i = 0, j = 0; i < x.size(); ++i, j = j + 2) {
        dyay[j] = x[i];
        dyay[j + 1] = 0.0;
    }
    dyay.pop_back();

    std::vector<T> out = conv(dyay, f);

    int d = (out.size() - len) / 2;
    int first = 1 + (out.size() - len) / 2;  //floor inclucded
    int last = first + len;

    out.erase(out.begin() + last - 1, out.end());
    out.erase(out.begin(), out.begin() + first - 1);
    return out;
}

// upsconv, overload added by Max F
template <typename Container>
Container upsconv(Container const& x, Container const& f, int len)
{

    //initialize std::vector dyay
    typename Container::value_type tmp[x.size() * 2];
    Container dyay(tmp, tmp + x.size() * 2);

    for (int i = 0, j = 0; i < x.size(); ++i, j = j + 2) {
        dyay[j] = x[i];
        dyay[j + 1] = 0.0;
    }
    dyay.pop_back();

    Container out = conv(dyay, f);

    int d = (out.size() - len) / 2;
    int first = 1 + (out.size() - len) / 2;  //floor inclucded
    int last = first + len;

    out.erase(out.begin() + last - 1, out.end());
    out.erase(out.begin(), out.begin() + first - 1);
    return out;
}


// dbwavf
template <typename T>
std::vector<T> dbwavf(int const wnum, T returnTypeExample)
{
    static const std::vector<std::function<std::vector<T>()>> F { []() {
                                                                     std::vector<T> F0 = {};
                                                                     return F0;
                                                                 },
        []() {
            std::vector<T> F1 = { 0.50000000000000, 0.50000000000000 };
            return F1;
        },
        []() {
            std::vector<T> F2 = { 0.34150635094622, 0.59150635094587, 0.15849364905378, -0.09150635094587 };
            return F2;
        },
        []() {
            std::vector<T> F3 = { 0.23523360389270, 0.57055845791731, 0.32518250026371, -0.09546720778426,
                -0.06041610415535, 0.02490874986589 };
            return F3;
        },
        []() {
            std::vector<T> F4 = { 0.16290171402562, 0.50547285754565, 0.44610006912319, -0.01978751311791,
                -0.13225358368437, 0.02180815023739, 0.02325180053556, -0.00749349466513 };
            return F4;
        },
        []() {
            std::vector<T> F5
                = { 0.11320949129173, 0.42697177135271, 0.51216347213016, 0.09788348067375, -0.17132835769133,
                      -0.02280056594205, 0.05485132932108, -0.00441340005433, -0.00889593505093, 0.00235871396920 };
            return F5;
        },
        []() {
            std::vector<T> F6 = { 0.07887121600143, 0.34975190703757, 0.53113187994121, 0.22291566146505,
                -0.02233187416548, 0.00039162557603, 0.00337803118151, -0.00076176690258, -0.15999329944587,
                -0.09175903203003, 0.06894404648720, 0.01946160485396 };
            return F6;
        },
        []() {
            std::vector<T> F7 = { 0.05504971537285, 0.28039564181304, 0.51557424581833, 0.33218624110566,
                -0.10175691123173, -0.15841750564054, 0.05042323250485, 0.05700172257986, -0.02689122629486,
                -0.01171997078235, 0.00887489618962, 0.00030375749776, -0.00127395235906, 0.00025011342658 };
            return F7;
        },
        []() {
            std::vector<T> F8 = { 0.03847781105406, 0.22123362357624, 0.47774307521438, 0.41390826621166,
                -0.01119286766665, -0.20082931639111, 0.00033409704628, 0.09103817842345, -0.01228195052300,
                -0.03117510332533, 0.00988607964808, 0.00618442240954, -0.00344385962813, -0.00027700227421,
                0.00047761485533, -0.00008306863060 };
            return F8;
        },
        []() {
            std::vector<T> F9 = { 0.02692517479416, 0.17241715192471, 0.42767453217028, 0.46477285717278,
                0.09418477475112, -0.20737588089628, -0.06847677451090, 0.10503417113714, 0.02172633772990,
                -0.04782363205882, 0.00017744640673, 0.01581208292614, -0.00333981011324, -0.00302748028715,
                0.00130648364018, 0.00016290733601, -0.00017816487955, 0.00002782275679 };
            return F9;
        },
        []() {
            std::vector<T> F10
                = { 0.01885857879640, 0.13306109139687, 0.37278753574266, 0.48681405536610, 0.19881887088440,
                      -0.17666810089647, -0.13855493935993, 0.09006372426666, 0.06580149355070, -0.05048328559801,
                      -0.02082962404385, 0.02348490704841, 0.00255021848393, -0.00758950116768, 0.00098666268244,
                      0.00140884329496, -0.00048497391996, -0.00008235450295, 0.00006617718320, -0.00000937920789 };
            return F10;
        } };

    return F[wnum]();
}

template <typename Container>
Container dbwavf(int const wnum, typename Container::value_type returnTypeExample) // overload added by Max F
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
template <typename T>
std::tuple<std::vector<T>, std::vector<T>, std::vector<T>, std::vector<T>> orthfilt(std::vector<T> const& W_in)
{

    auto qmf = [](std::vector<T> const& x) {
        std::vector<T> y(x.rbegin(), x.rend());
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
    auto sqrt = [](std::vector<T> const& x) {
        std::vector<T> out;
        out.reserve(x.size());
        for (int i = 0; i < x.size(); ++i) {
            out.push_back(std::sqrt(2) * (x[i]));
        }

        return out;
    };

    T W_in_sum = std::accumulate(W_in.begin(), W_in.end(), 0);

    std::vector<T> Lo_R = sqrt(W_in);
    std::vector<T> Hi_R = qmf(Lo_R);
    std::vector<T> Hi_D(Hi_R.rbegin(), Hi_R.rend());
    std::vector<T> Lo_D(Lo_R.rbegin(), Lo_R.rend());

    return { Lo_D, Hi_D, Lo_R, Hi_R };
}

// orthfilt, overload added by Max F
template <typename Container>
std::tuple<Container, Container, Container, Container> orthfilt(Container const& W_in)
{

    auto qmf = [](Container const& x) {
        Container y(x.rbegin(), x.rend());
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
        Container out;
        //out.reserve(x.size());
        for (int i = 0; i < x.size(); ++i) {
            out.push_back(std::sqrt(2) * (x[i]));
        }

        return out;
    };

    typename Container::value_type W_in_sum = std::accumulate(W_in.begin(), W_in.end(), 0);

    Container Lo_R = sqrt(W_in);
    Container Hi_R = qmf(Lo_R);
    Container Hi_D(Hi_R.rbegin(), Hi_R.rend());
    Container Lo_D(Lo_R.rbegin(), Lo_R.rend());

    return { Lo_D, Hi_D, Lo_R, Hi_R };
}


// dwt
template <typename T>
std::tuple<std::vector<T>, std::vector<T>> dwt(std::vector<T> const& x, int waveletType)
{

    std::vector<T> F = dbwavf(waveletType, T(1.0));

    auto [Lo_D, Hi_D, Lo_R, Hi_R] = orthfilt(F);

    int lf = Lo_D.size();
    int lx = x.size();

    int first = 2;
    int lenEXT = lf - 1;
    int last = lx + lf - 1;

    std::vector<T> x_ext;
    x_ext.reserve(lx + 2 * lenEXT);  // preallocate memory
    x_ext.insert(x_ext.end(), x.rbegin() + (lx - lenEXT), x.rend());
    x_ext.insert(x_ext.end(), x.begin(), x.end());
    x_ext.insert(x_ext.end(), x.rbegin(), x.rend() - (lx - lenEXT));

    std::vector<T> z1 = conv_valid(x_ext, Lo_D);
    std::vector<T> z2 = conv_valid(x_ext, Hi_D);
    std::vector<T> a;
    std::vector<T> d;
    a.reserve(last);
    d.reserve(last);

    for (int i = first - 1; i < last; i = i + 2) {
        a.push_back(z1[i]);
        d.push_back(z2[i]);
    }

    return { a, d };
}

// dwt, overload added by Max F
template <typename Container>
std::tuple<Container, Container> dwt(Container const& x, int waveletType)
{

    Container F = dbwavf<Container>(waveletType, typename Container::value_type(1.0));

    auto [Lo_D, Hi_D, Lo_R, Hi_R] = orthfilt(F);

    int lf = Lo_D.size();
    int lx = x.size();

    int first = 2;
    int lenEXT = lf - 1;
    int last = lx + lf - 1;

    Container x_ext;
    //x_ext.reserve(lx + 2 * lenEXT);  // preallocate memory
    x_ext.insert(x_ext.end(), x.rbegin() + (lx - lenEXT), x.rend());
    x_ext.insert(x_ext.end(), x.begin(), x.end());
    x_ext.insert(x_ext.end(), x.rbegin(), x.rend() - (lx - lenEXT));

    Container z1 = conv_valid(x_ext, Lo_D);
    Container z2 = conv_valid(x_ext, Hi_D);
    Container a;
    Container d;
    //a.reserve(last);
    //d.reserve(last);

    for (int i = first - 1; i < last; i = i + 2) {
        a.push_back(z1[i]);
        d.push_back(z2[i]);
    }

    return { a, d };
}


template <typename T>
std::vector<T> idwt(std::vector<T> a, std::vector<T> d, int waveletType, int lx)
{

    std::vector<T> F = dbwavf(waveletType, T(1.0));
    auto [Lo_D, Hi_D, Lo_R, Hi_R] = orthfilt(F);

    std::vector<T> out1 = upsconv(a, Lo_R, lx);
    std::vector<T> out2 = upsconv(d, Hi_R, lx);
    return out1 + out2;
}

template <typename Container>
Container idwt(Container a, Container d, int waveletType, int lx) // overload added by Max F, called in DSPCC
{

    Container F = dbwavf<Container>(waveletType, typename Container::value_type(1.0));
    auto [Lo_D, Hi_D, Lo_R, Hi_R] = orthfilt(F);

    Container out1 = upsconv(a, Lo_R, lx);
    Container out2 = upsconv(d, Hi_R, lx);
    Container result(out1.size());
    //std::transform(out1.begin(), out1.end(), result.begin(), std::bind(std::plus<typename Container::value_type>(), out2));
    for (size_t i = 0; i<out1.size(); ++i)
        result[i] = out1[i] + out2[i];
    return result; //out1 + out2;
}


int wmaxlev(int sizeX, int waveletType)
{
    std::vector<double> F = dbwavf(waveletType, double(1.0));
    auto [Lo_D, Hi_D, Lo_R, Hi_R] = orthfilt(F);

    int lev = (int)(std::log2((double)sizeX / ((double)Lo_D.size() - 1.0)));
    if (lev >= 1) {
        return lev;
    } else {
        return 0;
    }
}

template <typename T>
std::deque<std::vector<T>> wavedec(std::vector<T> const& x, int order, int waveletType)
{

    std::deque<std::vector<T>> subBands;
    std::vector<T> d;
    std::vector<T> x_tmp = x;

    std::vector<T> zeros(x.size(), 0);

    subBands.push_front(zeros);

    for (int k = 0; k < order; ++k) {
        auto [x_tmp2, d] = dwt(x_tmp, waveletType);
        x_tmp = x_tmp2;
        subBands.push_front(d);
    }
    subBands.push_front(x_tmp);
    return subBands;
}

template <typename T>
std::vector<T> waverec(std::deque<std::vector<T>> const& subBands, int waveletType)
{
    std::deque<std::vector<T>> a;
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

template <typename T>
blaze::CompressedVector<T> smoothDenoise(std::vector<T> const& data, T const& tresh)
{
    //smooth reduces of noise by threshold and gives back a sparse vector.
    // initialize

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
template <typename T>
std::vector<std::vector<T>> rfc(std::vector<T> const& data, std::vector<T> const& time_in)
{
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

    std::vector<T> ext;
    ext.reserve(ext_index.size());

    for (int i = 0; i < ext_index.size(); ++i) {
        ext.push_back(data[ext_index[i]]);
    }

    std::vector<std::vector<T>> cycles;

    int i = 0;
    int j = 1;
    while (ext.size() > (i + 1)) {
        T Y = std::abs(ext[i + 1] - ext[i]);
        T X = std::abs(ext[j + 1] - ext[j]);
        if (X >= Y) {
            if (i == 0)  // counts a half cycle and deletes the poit that is counted
            {
                std::vector<T> cycle_info = { std::abs(ext[i] - ext[i + 1]) / 2, (ext[i] + ext[i + 1]) / 2, 0.5 };
                cycles.push_back(cycle_info);
                ext.erase(ext.begin());
            } else  //counts one cycle and deletes the poits that are counted
            {
                std::vector<T> cycle_info = { std::abs(ext[i] - ext[i + 1]) / 2, (ext[i] + ext[i + 1]) / 2, 1.0 };
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

        std::vector<T> cycle_info = { std::abs(ext[i] - ext[i + 1]) / 2, (ext[i] + ext[i + 1]) / 2, 0.5 };
        cycles.push_back(cycle_info);
    }
    return cycles;
}

// get subband stats
template <typename T>
std::vector<T> chebyshev(std::vector<T> const& data, int polynom)
{
    std::vector<T> r_data = {};

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

template <typename T>
std::vector<T> rescale(std::vector<T> const& A, T a, T b)
{
    std::vector<T> R(A.size());

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


// 2d functions


template <typename Container>
std::tuple<std::vector<Container>, std::vector<Container>, std::vector<Container>, std::vector<Container>>
dwt2(std::vector<Container> const & x, int waveletType)
{
    std::vector<Container> ll, lh, hl, hh, l, h;

    for (size_t row_idx = 0; row_idx<x.size(); ++row_idx) { // top-level split, by rows
        auto row_split = dwt(x[row_idx], waveletType);
        l.push_back(std::get<0>(row_split));
        h.push_back(std::get<1>(row_split));
    }
    assert(l.size() == h.size()); // TODO remove after testing
    assert(l[0].size() == h[0].size()); // TODO remove after testing

    //bool vector_empty = true;
    for (size_t col_idx = 0; col_idx<l[0].size(); col_idx++) { // 2 lower level splits, by colmns
        Container l_col, h_col;
        for (size_t row_idx = 0; row_idx<l.size(); ++row_idx) { // we assume sizes of l and r are equal
            l_col.push_back(l[row_idx][col_idx]);
            h_col.push_back(h[row_idx][col_idx]);
        }
        {
            auto col_split_l = dwt(l_col, waveletType);
            assert(std::get<0>(col_split_l).size()==std::get<1>(col_split_l).size()); // TODO remove after testing
            //if (vector_empty) {
            if (col_idx < 1) { // first iteration only
                // init
                for (size_t row_idx=0; row_idx<std::get<0>(col_split_l).size(); ++row_idx) {
                    ll.push_back(Container(l[0].size(), 0)); // adding zero vectors
                    lh.push_back(Container(l[0].size(), 0));
                    hl.push_back(Container(h[0].size(), 0));
                    hh.push_back(Container(h[0].size(), 0));
                }
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
    std::vector<Container> out;
    //
    assert(ll.size()==lh.size()); // TODO remove after testing and add exception
    assert(ll.size()==hl.size());
    assert(ll.size()==hh.size());
    assert(ll[0].size()==lh[0].size());
    assert(ll[0].size()==hl[0].size());
    assert(ll[0].size()==hh[0].size());

    std::vector<Container> l_colmajor, h_colmajor;
    for (size_t col_idx = 0; col_idx<ll[0].size(); col_idx++) {
        Container col_ll, col_lh, col_hl, col_hh, col_split_l, col_split_h;
        for (size_t row_idx = 0; row_idx<ll.size(); ++row_idx) {
            col_ll.push_back(ll[row_idx][col_idx]);
            col_lh.push_back(lh[row_idx][col_idx]);
            col_hl.push_back(hl[row_idx][col_idx]);
            col_hh.push_back(hh[row_idx][col_idx]);
        }
        col_split_l = wavelet::idwt(col_ll, col_lh, waveletType, hx);
        l_colmajor.push_back(col_split_l);
        col_split_h = wavelet::idwt(col_hl, col_hh, waveletType, hx);
        h_colmajor.push_back(col_split_h);
    }

    assert(l_colmajor.size()==h_colmajor.size()); // TODO remove after testing
    assert(l_colmajor[0].size()==h_colmajor[0].size());

    // transpose and apply second idwt
    for (size_t row_idx = 0; row_idx<l_colmajor[0].size(); ++row_idx) {
        Container row_split_l, row_split_h;
        for (size_t col_idx = 0; col_idx<l_colmajor.size(); col_idx++) {
            row_split_l.push_back(l_colmajor[col_idx][row_idx]);
            row_split_h.push_back(h_colmajor[col_idx][row_idx]);
        }
        //Container row = idwt(row_split_l, row_split_h, waveletType, wx);
        out.push_back(idwt(row_split_l, row_split_h, waveletType, wx));
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



}  // end namespace
#endif
