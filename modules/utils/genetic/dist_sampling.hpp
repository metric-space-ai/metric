#ifndef _DIST_SAMPLING_HPP
#define _DIST_SAMPLING_HPP

#include<vector>

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


#endif //header guard
