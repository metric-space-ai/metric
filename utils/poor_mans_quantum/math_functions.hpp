#ifndef _PMQ_MATH_FUNCTIONS_HPP
#define _PMQ_MATH_FUNCTIONS_HPP
/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Michael Welsch
*/

#include<vector>


namespace metric {


template <typename T>
std::vector<T>
linspace(T a, T b, int n);

// akima interpolation
/*
Ref. : Hiroshi Akima, Journal of the ACM, Vol. 17, No. 4, October 1970,
pages 589-602.
*/

template <typename T>
std::vector<T>
akimaInterp1(std::vector<T> const &x, std::vector<T> const &y, std::vector<T> const &xi, bool save_Mode = true);




template <typename T>
std::vector<T>
linspace(T a, T b, int n)
{
	std::vector<T> array;
	if (n > 1)
	{
		T step = (b - a) / T(n - 1);
		int count = 0;
		while (count < n)
		{
		array.push_back(a + count * step);
		++count;
		}
	}
	else
	{
		array.push_back(b);
	}
	return array;
}


template <typename T>
std::vector<T>
akimaInterp1(std::vector<T> const &x, std::vector<T> const &y, std::vector<T> const &xi, bool save_Mode )
{
// check inputs

	//calculate u vector
	auto uVec = [](std::vector<T> const &x, std::vector<T> const &y) {
		size_t n = x.size();
		std::vector<T> u((n + 3));
		for (size_t i = 1; i < n; ++i)
		{
		u[i + 1] = (y[i] - y[i - 1]) / (x[i] - x[i - 1]); // Shift i to i+2
		}

		auto akima_end = [](const T &u1, const T &u2) {
		return 2.0 * u1 - u2;
		};

		u[1] = akima_end(u[2], u[3]);
		u[0] = akima_end(u[1], u[2]);
		u[n + 1] = akima_end(u[n], u[n - 1]);
		u[n + 2] = akima_end(u[n + 1], u[n]);

		return u;
	};
	std::vector<T> u = uVec(x, y);

	// calculate yp vector
	std::vector<T> yp(x.size());
	for (size_t i = 0; i < x.size(); ++i)
	{
		auto a = std::abs(u[i + 3] - u[i + 2]);
		auto b = std::abs(u[i + 1] - u[i]);
		if ((a + b) != 0)
		{
		yp[i] = (a * u[i + 1] + b * u[i + 2]) / (a + b);
		}
		else
		{
		yp[i] = (u[i + 2] + u[i + 1]) / 2.0;
		}
	}

	// calculte interpolated yi values
	auto kFind = [](const T &xii, const std::vector<T> &x, int start, int end) {

		int klo = start;
		int khi = end;
		// // Find subinterval by bisection
		while (khi - klo > 1)
		{
		int k = (khi + klo) / 2;
		x[k] > xii ? khi = k : klo = k;
		}
		return klo;
	};

	std::vector<T> yi(xi.size());
	for (size_t i = 0; i < xi.size(); ++i)
	{
		// Find the right place in the table by means of a bisection.
		int k = kFind(xi[i], x, int(0), x.size() - 1);

		// Evaluate Akima polynomial
		T b = x[k + 1] - x[k];
		T a = xi[i] - x[k];
		yi[i] = y[k] + yp[k] * a + (3.0 * u[k + 2] - 2.0 * yp[k] - yp[k + 1]) * a * a / b + (yp[k] + yp[k + 1] - 2.0 * u[k + 2]) * a * a * a / (b * b);

		// Differentiate to find the second-order interpolant
		//ypi[i] = yp[k] + (3.0u[k+2] - 2.0yp[k] - yp[k+1])2a/b + (yp[k] + yp[k+1] - 2.0u[k+2])3aa/(b*b);

		// Differentiate to find the first-order interpolant
		//yppi[i] = (3.0u[k+2] - 2.0yp[k] - yp[k+1])2/b + (yp[k] + yp[k+1] - 2.0u[k+2])6a/(b*b);
	}
	return yi;
}

template <typename T>
std::vector<T>
resize(std::vector<T> y, size_t n)
{
    std::sort(y.begin(), y.end());
    auto x0 = linspace(T(0.5) / T(n), T(1) - T(0.5) / T(n), n);
    auto x = linspace(T(0.5) / T(y.size()), T(1) - T(0.5) / T(y.size()), y.size());
    return akimaInterp1(x, y, x0);
}

template <typename T>
std::vector<std::vector<T>>
transpose(std::vector<std::vector<T>> &a)
{

    size_t rows = a.size();
    size_t cols = a[0].size();

    std::vector<std::vector<T>> array(cols, std::vector<T>(rows));
    for (size_t i = 0; i < cols; ++i)
    {
        for (size_t j = 0; j < rows; ++j)
        {
            array[i][j] = a[j][i];
        }
    }
    return array;
}

template <typename T>
T Lerp(T v0, T v1, T t)
{
    return (1 - t) * v0 + t * v1;
}

template <typename T>
T quickQuantil(std::vector<T> data, T probs)
{

    if (!(data.size() > 0))
        return 0;

    if (1 == data.size())
        return data[0];

    T poi = Lerp(T(-0.5), data.size() - T(0.5), probs);

    int left = std::max(int(std::floor(poi)), int(0));
    int right = std::min(int(std::ceil(poi)), int(data.size() - 1));

    if (probs <= T(0.5))
        std::nth_element(data.begin(), data.begin() + left, data.end());
    else
        std::nth_element(data.begin(), data.begin() + right, data.end());

    T datLeft = data[left];
    T datRight = data[right];

    T quantile = Lerp(datLeft, datRight, poi - T(left));

    return quantile;
}

} // end namespace
#endif //header guard