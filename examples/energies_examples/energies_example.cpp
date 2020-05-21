/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#if defined(__linux__)
	#include <dirent.h>
#endif

#include <vector>
#include <any>

#include <iostream>
#include <fstream>

#if defined(_WIN64)
	#include <filesystem>
#endif

#include <chrono>

#include "../../modules/utils/ThreadPool.hpp"
#include "../../modules/utils/Semaphore.h"

#include "assets/json.hpp"
#include "../../modules/mapping.hpp"
#include "../../modules/utils/poor_mans_quantum.hpp"

#include <algorithm>


using json = nlohmann::json;

///////////////////////////////////////////////////////

std::string RAW_DATA_DIRNAME = "assets/data";
int CLUSTERS_NUM = 7;

////////////////////////////////////////////////////////


namespace ooc_functions
{
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
std::vector<std::vector<T>>
transposeInPlace(std::vector<std::vector<T>> matrix)
{
  size_t rows = matrix.size();
  size_t cols = matrix[0].size();
  size_t n = std::min(rows, cols);

  for (size_t i = 0; i < n; i++)
  {
    for (size_t j = i; j < n; j++)
    {
      T temp = matrix[j][i];
      matrix[j][i] = matrix[i][j];
      matrix[i][j] = temp;
    }
  }

  if (cols > rows)
  {

    //std::vector<std::vector<T> rest(rows);
    std::vector<std::vector<T>> rest(rows, std::vector<T>());
    std::vector<T> rest2(cols - rows - 1);

    for (size_t i = 0; i < rows; ++i)
    {
      //rest[i] = matrix[i].splice(rows, cols)

      rest[i].insert(rest[i].begin(), std::make_move_iterator(matrix[i].begin() + rows), std::make_move_iterator(matrix[i].begin() + cols));
      matrix[i].erase(matrix[i].begin() + rows, matrix[i].begin() + cols);
    }

    for (size_t i = 0; i < cols - rows; ++i)
    {
      matrix.push_back(rest2);
    }

    for (size_t i = 0; i < rest[0].size(); ++i)
    {
      for (size_t j = 0; j < rest.size(); j++)
      {
        matrix[i + rows][j] = rest[j][i];
      }
    }
  }

  else if (cols < rows)
  {

    //std::vector<T> rests(rows-cols);
    std::vector<std::vector<T>> rest(rows - cols, std::vector<T>());

    for (size_t i = 0; i < cols; ++i)
    {
      // matrix[i].concat(rest)
      matrix[i].reserve(matrix[i].size()+rows-cols);
    }

    for (size_t i = 0; i < rows - cols; ++i)
    {
      //rest[i] = matrix[i + cols].splice(0, cols)

      rest[i].insert(rest[i].begin(), std::make_move_iterator(matrix[i + cols].begin() + 0), std::make_move_iterator(matrix[i + cols].begin() + cols));
      matrix[i + cols].erase(matrix[i + cols].begin() + 0, matrix[i + cols].begin() + cols);
    }

    for (size_t i = 0; i < rest[0].size(); ++i)
    {
      for (size_t j = 0; j < rest.size(); j++)
      {
        matrix[i][j + cols] = rest[j][i];
      }
    }

    for (size_t i = 0; i < rows - cols; ++i)
    {
      matrix.pop_back();
    }
  }
  return matrix;
}

// --------------------------------------------------------------
// sorting functions

template <typename T>
void quicksort(std::vector<T> &a, size_t lo, size_t hi);

template <typename T>
void insertionsort(std::vector<T> &a, size_t lo, size_t hi);

template <typename T>
void dualPivotSort(std::vector<T> &a, size_t lo = 0, size_t hi = 0)
{
  if (hi == 0)
  {
    hi = a.size();
  }
  size_t quicksort_sizeThreshold = 32;
  (hi - lo < quicksort_sizeThreshold ? insertionsort(a, lo, hi) : quicksort(a, lo, hi));
}

template <typename T>
void insertionsort(std::vector<T> &a, size_t lo, size_t hi)
{

  T t, x;
  size_t i, j;
  for (i = lo + 1; i < hi; ++i)
  {
    for (j = i, t = a[i], x = t; j > lo && a[j - 1] > x; --j)
    {

      a[j] = a[j - 1];
    }
    a[j] = t;
  }
}

template <typename T>
void quicksort(std::vector<T> &a, size_t lo, size_t hi)
{
  // Compute the two pivots by looking at 5 elements.
  size_t sixth = (hi - lo) / 6 | 0,
         i1 = lo + sixth,
         i5 = hi - 1 - sixth,
         i3 = (lo + hi - 1) >> 1, // The midpoint.
      i2 = i3 - sixth,
         i4 = i3 + sixth;

  T e1 = a[i1], x1 = e1,
    e2 = a[i2], x2 = e2,
    e3 = a[i3], x3 = e3,
    e4 = a[i4], x4 = e4,
    e5 = a[i5], x5 = e5;

  T t;

  if (x1 > x2)
    t = e1, e1 = e2, e2 = t, t = x1, x1 = x2, x2 = t;
  if (x4 > x5)
    t = e4, e4 = e5, e5 = t, t = x4, x4 = x5, x5 = t;
  if (x1 > x3)
    t = e1, e1 = e3, e3 = t, t = x1, x1 = x3, x3 = t;
  if (x2 > x3)
    t = e2, e2 = e3, e3 = t, t = x2, x2 = x3, x3 = t;
  if (x1 > x4)
    t = e1, e1 = e4, e4 = t, t = x1, x1 = x4, x4 = t;
  if (x3 > x4)
    t = e3, e3 = e4, e4 = t, t = x3, x3 = x4, x4 = t;
  if (x2 > x5)
    t = e2, e2 = e5, e5 = t, t = x2, x2 = x5, x5 = t;
  if (x2 > x3)
    t = e2, e2 = e3, e3 = t, t = x2, x2 = x3, x3 = t;
  if (x4 > x5)
    t = e4, e4 = e5, e5 = t, t = x4, x4 = x5, x5 = t;

  T pivot1 = e2, pivotValue1 = x2,
    pivot2 = e4, pivotValue2 = x4;

  a[i1] = e1;
  a[i2] = a[lo];
  a[i3] = e3;
  a[i4] = a[hi - 1];
  a[i5] = e5;

  size_t less = lo + 1, // First element in the middle partition.
      great = hi - 2;   // Last element in the middle partition.

  bool pivotsEqual = pivotValue1 <= pivotValue2 && pivotValue1 >= pivotValue2;
  if (pivotsEqual)
  {

    for (size_t k = less; k <= great; ++k)
    {
      T ek = a[k], xk = ek;
      if (xk < pivotValue1)
      {
        if (k != less)
        {
          a[k] = a[less];
          a[less] = ek;
        }
        ++less;
      }
      else if (xk > pivotValue1)
      {

        /* eslint no-constant-condition: 0 */
        while (true)
        {
          T greatValue = a[great];
          if (greatValue > pivotValue1)
          {
            great--;
            continue;
          }
          else if (greatValue < pivotValue1)
          {
            a[k] = a[less];
            a[less++] = a[great];
            a[great--] = ek;
            break;
          }
          else
          {
            a[k] = a[great];
            a[great--] = ek;
            break;
          }
        }
      }
    }
  }
  else
  {
    // (function () { // isolate scope
    {
      for (size_t k = less; k <= great; k++)
      {
        T ek = a[k], xk = ek;
        if (xk < pivotValue1)
        {
          if (k != less)
          {
            a[k] = a[less];
            a[less] = ek;
          }
          ++less;
        }
        else
        {
          if (xk > pivotValue2)
          {
            while (true)
            {
              T greatValue = a[great];
              if (greatValue > pivotValue2)
              {
                great--;
                if (great < k)
                  break;

                continue;
              }
              else
              {

                if (greatValue < pivotValue1)
                {
                  a[k] = a[less];
                  a[less++] = a[great];
                  a[great--] = ek;
                }
                else
                {
                  a[k] = a[great];
                  a[great--] = ek;
                }
                break;
              }
            }
          }
        }
      }
    }
    // })(); // isolate scope
  }

  a[lo] = a[less - 1];
  a[less - 1] = pivot1;
  a[hi - 1] = a[great + 1];
  a[great + 1] = pivot2;

  dualPivotSort(a, lo, less - 1);
  dualPivotSort(a, great + 2, hi);

  if (pivotsEqual)
  {
    return;
  }

  if (less < i1 && great > i5)
  {

    // (function () { // isolate scope
    {
      T lessValue, greatValue;
      while ((lessValue = a[less]) <= pivotValue1 && lessValue >= pivotValue1)
        ++less;
      while ((greatValue = a[great]) <= pivotValue2 && greatValue >= pivotValue2)
        --great;

      for (size_t k = less; k <= great; k++)
      {
        T ek = a[k], xk = ek;
        if (xk <= pivotValue1 && xk >= pivotValue1)
        {
          if (k != less)
          {
            a[k] = a[less];
            a[less] = ek;
          }
          less++;
        }
        else
        {
          if (xk <= pivotValue2 && xk >= pivotValue2)
          {
            /* eslint no-constant-condition: 0 */
            while (true)
            {
              greatValue = a[great];
              if (greatValue <= pivotValue2 && greatValue >= pivotValue2)
              {
                great--;
                if (great < k)
                  break;
                continue;
              }
              else
              {
                if (greatValue < pivotValue1)
                {
                  a[k] = a[less];
                  a[less++] = a[great];
                  a[great--] = ek;
                }
                else
                {
                  a[k] = a[great];
                  a[great--] = ek;
                }
                break;
              }
            }
          }
        }
      }
    }
    //})(); // isolate scope
  }

  dualPivotSort(a, less, great + 1);
}

// linspace (erzeugt einen linearen Datenvektor)
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

// akima interpolation
/*
Ref. : Hiroshi Akima, Journal of the ACM, Vol. 17, No. 4, October 1970,
      pages 589-602.
*/
template <typename T>
std::vector<T>
akimaInterp1(std::vector<T> const &x, std::vector<T> const &y, std::vector<T> const &xi, bool save_Mode = true)
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
    //ypi[i] = yp[k] + (3.0*u[k+2] - 2.0*yp[k] - yp[k+1])*2*a/b + (yp[k] + yp[k+1] - 2.0*u[k+2])*3*a*a/(b*b);

    // Differentiate to find the first-order interpolant
    //yppi[i] = (3.0*u[k+2] - 2.0*yp[k] - yp[k+1])*2/b + (yp[k] + yp[k+1] - 2.0*u[k+2])*6*a/(b*b);
  }
  return yi;
}
}

// end helper functions


//////////////////////////////////////////////////////////////////////


template <typename T>
void matrix_print(const std::vector<std::vector<T>> &mat)
{

    std::cout << "[";
    std::cout << std::endl;
	for (int i = 0; i < mat.size(); i++)
	{
		std::cout << "  [ ";
		if (mat[i].size() > 0)
		{
			for (int j = 0; j < mat[i].size() - 1; j++)
			{
				std::cout << mat[i][j] << ", ";
			}
			std::cout << mat[i][mat[i].size() - 1];
		}
			
		std::cout << " ]" << std::endl;
    }
    std::cout << std::endl;
    std::cout << "]" << std::endl;
}

template <typename T>
void vector_print(const std::vector<T> &vec)
{

    std::cout << "[ ";
    for (int i = 0; i < vec.size() - 1; i++)
    {
        std::cout << vec[i] << ", ";
    }
    std::cout << vec[vec.size() - 1] << " ]" << std::endl;
}

template <typename T>
void vector_print(const std::vector<T> &vec,const size_t width, const size_t height)
{
	if ((width * height) != vec.size()) {
		std::cout << "width * height != vector.size()" << std::endl;
		return;
	}
	
	int max_digits = 1;
	for (auto index = 0; index < vec.size(); ++index) {
		int pos = 10;
		int digits_num = 1;
		while (vec[index] / pos >= 1)
		{
			digits_num++;
			pos *= 10;
		}
		if (digits_num > max_digits)
		{
			max_digits = digits_num;
		}
	}

	for (auto index = 0; index < vec.size(); ++index) {
		
		int pos = 10;
		int digits_num = 1;
		while (vec[index] / pos >= 1)
		{
			digits_num++;
			pos *= 10;
		}
		for (auto i = 0; i < max_digits - digits_num; ++i) {
			std::cout << " ";
		}
		std::cout << vec[index] << " ";

		if ((index + 1) % width == 0) {
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
}

void printDataInfo(const json& data)
{
	for (const auto& [key, value]: data.items()) {
		std::cout << key << " " << value.size() << std::endl;
	}
}

template <typename T>
std::vector<size_t> sort_indexes(const std::vector<T> &v) {

  // initialize original index locations
  std::vector<size_t> idx(v.size());
  std::iota(idx.begin(), idx.end(), 0);

  // sort indexes based on comparing values in v
  std::sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

  return idx;
}

bool get_hex_bounds(std::vector<int> assignments, int checking_cluster_index, int near_cluster_index)
{		
	if (near_cluster_index >= 0 && near_cluster_index < assignments.size())
	{
		if (assignments[checking_cluster_index] != assignments[near_cluster_index])
		{
			return true;
		}
	}

	return false;
}


std::vector<std::vector<std::string>> readCsvData(std::string filename, char delimeter)
{
	std::fstream fin;

	fin.open(filename, std::ios::in);
	
	std::vector<std::string> row;
	std::string line, word, w;

	std::vector<std::vector<std::string>> rows;

	// omit headers 
	getline(fin, line);

	int i = 0;
	while (getline(fin, line))
	{
		i++;
		//std::cout << "row " << i << std::endl;
		std::stringstream s(line);

		row.clear();
		while (getline(s, word, delimeter))
		{
			//std::cout << " -> " << word << std::endl;
			
			row.push_back(word);
		}

		rows.push_back(row);
	}

	return rows;
}

std::vector<std::vector<double>> readEnergies(std::string dirname)
{
	#if defined(__linux__)
		std::vector<std::string> files;
		DIR *dp;
		struct dirent *dirp;
		if((dp  = opendir(dirname.c_str())) == NULL) {
			std::cout << "Error(" << errno << ") opening " << dirname << std::endl;
			return std::vector<std::vector<double>>();
		}

		while ((dirp = readdir(dp)) != NULL) {
			std::string fn = std::string(dirp->d_name);
			if (fn.size() > 4 && fn.substr(fn.size() - 4) == ".log")
			{
				files.push_back(dirname + "/" + fn);
			}
		}
		closedir(dp);
	#endif
	
	std::vector<double> row;
	std::vector<double> speeds;
	std::string line, word, w;

	std::vector<std::vector<double>> rows;
	
	#if defined(_WIN64)
		for (const auto & entry : std::filesystem::directory_iterator(dirname))
	#endif
	#if defined(__linux__)
		for (auto filename : files)
	#endif
    {
		#if defined(_WIN64)
			auto filename = entry.path();
		#endif
		std::cout << "reading data from " << filename << "... " << std::endl;

		std::fstream fin;

		fin.open(filename, std::ios::in);

		char delimeter = 9;

		int i = 0;
		while (getline(fin, line))
		{
			std::stringstream s(line);

			row.clear();
			// omit first digit
			getline(s, word, delimeter);

			while (getline(s, word, delimeter))
			{
				// std::cout << " -> " << word << std::endl;

				row.push_back(std::stold(word));
			}
			// erase last element
			double speed = row[row.size() - 1];
			speeds.push_back(speed);
			
			row.pop_back();

			if (speed >= 1)
			{
				for (auto k = 0; k < row.size(); k++)
				{
					row[k] = sqrt(row[k]);
				}
			}
			else
			{
				for (auto k = 0; k < row.size(); k++)
				{
					row[k] = 0;
				}
			}

			rows.push_back(row);
		}

		rows.pop_back();
	}

	return rows;
}

void saveToCsv(std::string filename, const std::vector<std::vector<std::string>> &mat, const std::vector<std::string> &features)
{
	std::ofstream outputFile;

	// create and open the .csv file
	outputFile.open(filename);

	// write the file headers
	for (auto i = 0; i < features.size(); ++i)
	{
		outputFile << features[i];
		outputFile << ",";
	}
	outputFile << std::endl;

	// last item in the mat is date
	for (auto i = 0; i < mat.size(); ++i)
	{
		//outputFile << dates[i] << ";";
		for (auto j = 0; j < mat[i].size(); j++)
		{
			outputFile << mat[i][j] << ",";
		}
		outputFile << std::endl;
	}

	// close the output file
	outputFile.close();
}


std::mutex mu;

template <typename T, typename Metric, typename Graph, typename Distribution>
double runConfiguration(int i, std::vector<std::vector<T>> data, Metric distance, Graph graph, Distribution distribution, 
	unsigned int iterations, double start_learn_rate, double final_learn_rate, double neighborhood_start_size, double neigbour_range_decay, long long random_seed)
{
	
	mu.lock();
	std::cout << "configuration #" << i << " started" << std::endl;
	std::cout << "  Distribution: " << typeid(distribution).name() << std::endl;
	std::cout << "  iterations: " << iterations << "  start_learn_rate: " << start_learn_rate << "  final_learn_rate: " 
		<< final_learn_rate << "  neighborhood_start_size: " << neighborhood_start_size << "  neigbour_range_decay: " << neigbour_range_decay << std::endl;
	std::cout << std::endl;
	mu.unlock();

	auto t1 = std::chrono::steady_clock::now();

	metric::SOM<std::vector<T>, Graph, Metric, Distribution> som_model(graph, distance, start_learn_rate, final_learn_rate, iterations, distribution, neighborhood_start_size, neigbour_range_decay, random_seed);
	
	som_model.train(data);
	
	// we will calculate std deviation
	auto std_deviation = som_model.std_deviation(data);
		
	auto t2 = std::chrono::steady_clock::now();
	mu.lock();
	std::cout << "configuration #" << i << " finished, score: " << std_deviation << std::endl;
	std::cout << "  Distribution: " << typeid(distribution).name() << std::endl;
	std::cout << "  iterations: " << iterations << "  start_learn_rate: " << start_learn_rate << "  final_learn_rate: " 
		<< final_learn_rate << "  neighborhood_start_size: " << neighborhood_start_size << "  neigbour_range_decay: " << neigbour_range_decay << std::endl;
	std::cout << "deviation: " << std_deviation << 
		" (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl << std::endl;
	mu.unlock();

	return std_deviation;
}

template <typename T>
std::vector<std::vector<T>>
set2conf(std::vector<T> set_0, size_t windowSize, size_t samples, T confidencelevel)
{

  std::random_device rd;  //seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<> dis(T(0), T(1));

  // propabiliy vector
  std::vector<T> prob_0 = ooc_functions::linspace(T(1) / T(set_0.size()), T(1) - T(1) / T(set_0.size()), set_0.size());
  std::sort(set_0.begin(), set_0.end());
  //ooc_functions::dualPivotSort(set_0);

  // compute probability matrix of set_0
  std::vector<std::vector<T>> m_0(samples, std::vector<T>(set_0.size()));
  //std::vector<std::vector<T>> m_prop_0(samples, std::vector<T>(set_0.size()));

  for (size_t i = 0; i < samples; ++i)
  {
    for (size_t j = 0; j < set_0.size(); ++j)
    {
      m_0[i][j] = T(dis(gen)); // fill with random numbers
    }

    std::sort(m_0[i].begin(), m_0[i].end()); // sort the row
    //ooc_functions::dualPivotSort(m_prop_0[i]);
    m_0[i] = ooc_functions::akimaInterp1(prob_0, set_0, m_0[i]); // interpolate the random numbers
  }

  // m_prop_0.clear();
  // m_prop_0.shrink_to_fit();

  // transpose
  auto m_0t = ooc_functions::transpose(m_0);

  m_0.clear();
  m_0.shrink_to_fit();

  // compute left and right confidence boundaries of set_0
  std::vector<T> set_0_left(set_0.size());
  std::vector<T> set_0_right(set_0.size());
  for (size_t i = 0; i < set_0.size(); ++i)
  {
    set_0_left[i] = ooc_functions::quickQuantil(m_0t[i], (T(1) - confidencelevel) / T(2));
    set_0_right[i] = ooc_functions::quickQuantil(m_0t[i], confidencelevel + (T(1) - confidencelevel) / T(2));
  }

    m_0t.clear();
  m_0t.shrink_to_fit();

  // compute probability matrix of left and right and medians of set_0
  std::vector<std::vector<T>> m_prop_1(samples, std::vector<T>(windowSize));


  for (size_t i = 0; i < samples; ++i)
  {
    for (size_t j = 0; j < windowSize; ++j)
    {
      m_prop_1[i][j] = T(dis(gen)); // fill with random numbers
    }

    std::sort(m_prop_1[i].begin(), m_prop_1[i].end()); // sort the row
  }

  std::vector<std::vector<T>> quants(3, std::vector<T>(windowSize));


  // left
  std::vector<std::vector<T>> m(samples, std::vector<T>(windowSize));
  for (size_t i = 0; i < samples; ++i)
  {
      m[i] = ooc_functions::akimaInterp1(prob_0, set_0_left, m_prop_1[i]); // interpolate the random numbers
  }

  // set_0_left.clear();
  // set_0_left.shrink_to_fit();

  auto mt = ooc_functions::transpose(m);

  for (size_t i = 0; i < windowSize; ++i)
  {
    quants[0][i] = ooc_functions::quickQuantil(mt[i], (T(1.0) - confidencelevel) / T(2.0));
  }

  //right
  for (size_t i = 0; i < samples; ++i)
  {
      m[i] = ooc_functions::akimaInterp1(prob_0, set_0_right, m_prop_1[i]);
  }

  // set_0_right.clear();
  // set_0_right.shrink_to_fit();

  mt = ooc_functions::transpose(m);

  for (size_t i = 0; i < windowSize; ++i)
  {
    quants[2][i] = ooc_functions::quickQuantil(mt[i], confidencelevel + (T(1.0) - confidencelevel) / T(2.0));
  }

  //median
  for (size_t i = 0; i < samples; ++i)
  {
      m[i] = ooc_functions::akimaInterp1(prob_0, set_0, m_prop_1[i]);
  }

  mt = ooc_functions::transpose(m);
  // m.clear();
  // m.shrink_to_fit();

  // m_prop_1.clear();
  // m_prop_1.shrink_to_fit();

  for (size_t i = 0; i < windowSize; ++i)
  {
    quants[1][i] = ooc_functions::quickQuantil(mt[i], T(0.5));
  }

  return quants;
}

template <typename T>
std::vector<std::vector<std::vector<T>>>
set2multiconf(std::vector<T> set_0, std::vector<uint32_t> windowSizes, size_t samples, T confidencelevel)
{
  std::vector<std::vector<std::vector<T>>> multiquants;
  for (size_t i = 0; i < windowSizes.size(); ++i)
  {
    multiquants.push_back(set2conf(set_0, windowSizes[i], samples, confidencelevel));
  }

  return multiquants;
}


template <typename T, typename Metric, typename Graph>
double iterateThroughDistributions(int distribution_type, int i, std::vector<std::vector<T>> speeds, Metric distance, Graph graph,
	unsigned int iterations, double s_learn_rate, double f_learn_rate, double initial_neighbour_size, double neigbour_range_decay, long long random_seed)
{
	double score;

	if (distribution_type == 0)
	{
		// uniform_real_distribution
		std::uniform_real_distribution<double> distribution(-1, 1);

		score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (distribution_type == 1)
	{
		// normal_distribution
		std::normal_distribution<double> distribution(-1, 1);

		score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (distribution_type == 2)
	{
		// exponential_distribution
		std::exponential_distribution<double> distribution(1);

		score = runConfiguration(i, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}

	return score;
}


template <typename T, typename Graph>
double iterateThroughDistances(int metric_type, int distribution_type, int i, std::vector<std::vector<T>> speeds, Graph graph,
	unsigned int iterations, double s_learn_rate, double f_learn_rate, double initial_neighbour_size, double neigbour_range_decay, long long random_seed)
{
	double score;

	if (metric_type == 0)
	{
		// Euclidean
		metric::Euclidean<double> distance;
		score = iterateThroughDistributions(distribution_type, i, speeds, distance, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (metric_type == 1)
	{
		// Manhatten
		metric::Manhatten<double> distance;
		score = iterateThroughDistributions(distribution_type, i, speeds, distance, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (metric_type == 2)
	{
		// P_norm
		metric::P_norm<double> distance;
		score = iterateThroughDistributions(distribution_type, i, speeds, distance, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (metric_type == 3)
	{
		// Euclidean_thresholded
		metric::Euclidean_thresholded<double> distance;
		score = iterateThroughDistributions(distribution_type, i, speeds, distance, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (metric_type == 4)
	{
		// Cosine
		metric::Cosine<double> distance;
		score = iterateThroughDistributions(distribution_type, i, speeds, distance, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (metric_type == 5)
	{
		// Chebyshev
		metric::Chebyshev<double> distance;
		score = iterateThroughDistributions(distribution_type, i, speeds, distance, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}

	return score;
}


template <typename T>
double iterateThroughGraphs(int w_grid_size, int h_grid_size, int graph_type, int metric_type, int distribution_type, int i, std::vector<std::vector<T>> speeds,
	unsigned int iterations, double s_learn_rate, double f_learn_rate, double initial_neighbour_size, double neigbour_range_decay, long long random_seed)
{
	double score;
	
	if (graph_type == 0)
	{
		// Grid4
		metric::Grid4 graph(w_grid_size, h_grid_size);
		score = iterateThroughDistances(metric_type, distribution_type, i, speeds, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (graph_type == 1)
	{
		// Grid6
		metric::Grid6 graph(w_grid_size, h_grid_size);
		score = iterateThroughDistances(metric_type, distribution_type, i, speeds, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (graph_type == 2)
	{
		// Grid8
		metric::Grid8 graph(w_grid_size, h_grid_size);
		score = iterateThroughDistances(metric_type, distribution_type, i, speeds, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (graph_type == 3)
	{
		// Paley
		metric::Paley graph(w_grid_size * h_grid_size);
		score = iterateThroughDistances(metric_type, distribution_type, i, speeds, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (graph_type == 4)
	{
		// LPS
		metric::LPS graph(w_grid_size * h_grid_size);
		score = iterateThroughDistances(metric_type, distribution_type, i, speeds, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (graph_type == 5)
	{
		// Margulis
		metric::Margulis graph(w_grid_size * h_grid_size);
		score = iterateThroughDistances(metric_type, distribution_type, i, speeds, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}

	return score;
}


template <typename T, typename Metric, typename Graph, typename Distribution>

std::tuple<std::vector<int>, std::vector<int>, std::vector<std::vector<std::vector<std::vector<double>>>>> 
get_weights_from_som(int w_grid_size, int h_grid_size, std::vector<std::vector<T>> speeds, Metric distance, Graph graph, Distribution distribution,
	unsigned int iterations, double s_learn_rate, double f_learn_rate, double initial_neighbour_size, double neigbour_range_decay, long long random_seed)
{
	std::cout << "  SOM Graph: " << typeid(graph).name() << std::endl;
	std::cout << "  SOM Distance: " << typeid(distance).name() << std::endl;
	std::cout << "  SOM Distribution: " << typeid(distribution).name() << std::endl;
	std::cout << std::endl;

	metric::SOM<std::vector<T>, Graph, Metric, Distribution> som(		
		graph, 
		distance, 
		s_learn_rate, 
		f_learn_rate, 
		iterations, 
		distribution, 
		initial_neighbour_size, 
		neigbour_range_decay, 
		random_seed
	);	
	
	som.train(speeds);

	auto nodes_data = som.get_weights();
	
	json nodes_data_json(nodes_data);
		
	std::ofstream som_output(RAW_DATA_DIRNAME + "/result/som_" + std::to_string(w_grid_size) + "x" + std::to_string(h_grid_size) + ".json");
	som_output << std::setw(4) << nodes_data_json << std::endl;
	som_output.close();	
	
	// clustering on the reduced data
	
    //metric::Matrix<std::vector<double>, metric::Cosine<double>> distance_matrix(nodes_data);	
    //auto [assignments, exemplars, counts] = metric::affprop(distance_matrix, (float)0.25);
    auto [assignments, exemplars, counts] = metric::kmeans(nodes_data, CLUSTERS_NUM, 1000);


	std::cout << "assignments:" << std::endl;
	vector_print(assignments, w_grid_size, h_grid_size);
	std::cout << std::endl;

	std::cout << "counts:" << std::endl;
	vector_print(counts);
	std::cout << std::endl;


	// split and reshape raw data by clusters [sensor -> cluster -> energy -> values]
	
	int num_sensors = 8;
	int num_levels = 7;
	std::vector<std::vector<std::vector<std::vector<double>>>> clustered_energies(num_sensors, std::vector<std::vector<std::vector<double>>>(counts.size(), std::vector<std::vector<double>>(num_levels)));

	std::vector<int> total(assignments.size());
	for (auto record : speeds)
	{
		// find cluster id for a record
		auto bmu = som.BMU(record);
		auto cluster_index = assignments[bmu];
		total[bmu]++;
		for (int i = 0; i < num_sensors; i++)
		{			
			for (int j = 0; j < num_levels; j++)
			{
				clustered_energies[i][cluster_index][j].push_back(record[i*num_levels + j]);
			}
		}
	}
	
	std::cout << "cluster sizes:" << std::endl;
	for (int k = 0; k < clustered_energies[0].size(); k++)
	{
		std::cout << "cluster_index: " << k << ", size: "<< clustered_energies[0][k][0].size() << std::endl;
	}
	std::cout << std::endl;
	
	std::cout << "som nodes sizes:" << std::endl;
	vector_print(total, w_grid_size, h_grid_size);
	std::cout << std::endl;
	
    return { assignments, counts, clustered_energies };
}


template <typename T, typename Metric, typename Graph>
std::tuple<std::vector<int>, std::vector<int>, std::vector<std::vector<std::vector<std::vector<double>>>>> 
iterateThroughDistributionsBest(int w_grid_size, int h_grid_size, int distribution_type, std::vector<std::vector<T>> speeds, Metric distance, Graph graph,
	unsigned int iterations, double s_learn_rate, double f_learn_rate, double initial_neighbour_size, double neigbour_range_decay, long long random_seed)
{

	if (distribution_type == 0)
	{
		// uniform_real_distribution
		std::uniform_real_distribution<double> distribution(-1, 1);

		return get_weights_from_som(w_grid_size, h_grid_size, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (distribution_type == 1)
	{
		// normal_distribution
		std::normal_distribution<double> distribution(-1, 1);
		
		return get_weights_from_som(w_grid_size, h_grid_size, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (distribution_type == 2)
	{
		// exponential_distribution
		std::exponential_distribution<double> distribution(1);
		
		return get_weights_from_som(w_grid_size, h_grid_size, speeds, distance, graph, distribution, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	
    return std::tuple<std::vector<int>, std::vector<int>, std::vector<std::vector<std::vector<std::vector<double>>>>>();
}


template <typename T, typename Graph>
std::tuple<std::vector<int>, std::vector<int>, std::vector<std::vector<std::vector<std::vector<double>>>>> 
iterateThroughDistancesBest(int w_grid_size, int h_grid_size, int metric_type, int distribution_type, std::vector<std::vector<T>> speeds, Graph graph,
	unsigned int iterations, double s_learn_rate, double f_learn_rate, double initial_neighbour_size, double neigbour_range_decay, long long random_seed)
{

	if (metric_type == 0)
	{
		// Euclidean
		metric::Euclidean<double> distance;
		return iterateThroughDistributionsBest(w_grid_size, h_grid_size, distribution_type, speeds, distance, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (metric_type == 1)
	{
		// Manhatten
		metric::Manhatten<double> distance;
		return iterateThroughDistributionsBest(w_grid_size, h_grid_size, distribution_type, speeds, distance, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (metric_type == 2)
	{
		// P_norm
		metric::P_norm<double> distance;
		return iterateThroughDistributionsBest(w_grid_size, h_grid_size, distribution_type, speeds, distance, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (metric_type == 3)
	{
		// Euclidean_thresholded
		metric::Euclidean_thresholded<double> distance;
		return iterateThroughDistributionsBest(w_grid_size, h_grid_size, distribution_type, speeds, distance, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (metric_type == 4)
	{
		// Cosine
		metric::Cosine<double> distance;
		return iterateThroughDistributionsBest(w_grid_size, h_grid_size, distribution_type, speeds, distance, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (metric_type == 5)
	{
		// Chebyshev
		metric::Chebyshev<double> distance;
		return iterateThroughDistributionsBest(w_grid_size, h_grid_size, distribution_type, speeds, distance, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}

	
    return std::tuple<std::vector<int>, std::vector<int>, std::vector<std::vector<std::vector<std::vector<double>>>>>();
}


template <typename T>
std::tuple<std::vector<int>, std::vector<int>, std::vector<std::vector<std::vector<std::vector<double>>>>> 
iterateThroughGraphsBest(int w_grid_size, int h_grid_size, int graph_type, int metric_type, int distribution_type, std::vector<std::vector<T>> speeds,
	unsigned int iterations, double s_learn_rate, double f_learn_rate, double initial_neighbour_size, double neigbour_range_decay, long long random_seed)
{
	
	if (graph_type == 0)
	{
		// Grid4
		metric::Grid4 graph(w_grid_size, h_grid_size);
		return iterateThroughDistancesBest(w_grid_size, h_grid_size, metric_type, distribution_type, speeds, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (graph_type == 1)
	{
		// Grid6
		metric::Grid6 graph(w_grid_size, h_grid_size);
		return iterateThroughDistancesBest(w_grid_size, h_grid_size, metric_type, distribution_type, speeds, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (graph_type == 2)
	{
		// Grid8
		metric::Grid8 graph(w_grid_size, h_grid_size);
		return iterateThroughDistancesBest(w_grid_size, h_grid_size, metric_type, distribution_type, speeds, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (graph_type == 3)
	{
		// Paley
		metric::Paley graph(w_grid_size * h_grid_size);
		return iterateThroughDistancesBest(w_grid_size, h_grid_size, metric_type, distribution_type, speeds, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (graph_type == 4)
	{
		// LPS
		metric::LPS graph(w_grid_size * h_grid_size);
		return iterateThroughDistancesBest(w_grid_size, h_grid_size, metric_type, distribution_type, speeds, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	else if (graph_type == 5)
	{
		// Margulis
		metric::Margulis graph(w_grid_size * h_grid_size);
		return iterateThroughDistancesBest(w_grid_size, h_grid_size, metric_type, distribution_type, speeds, graph, iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);
	}
	
    return std::tuple<std::vector<int>, std::vector<int>, std::vector<std::vector<std::vector<std::vector<double>>>>>();
}

std::tuple<std::vector<std::vector<int>>, std::vector<std::vector<int>>> 
getPositionsAndBorders(std::vector<int> assignments, int clusters_num, int row_len)
{
	std::vector<std::vector<int>> positions(clusters_num);
	//std::vector<std::vector<std::vector<int>>> borders(counts.size());
	std::vector<std::vector<int>> borders(clusters_num);
	for (int i = 0; i < assignments.size(); i++)
	{
		positions[assignments[i]].push_back(i);

		//

		int row = i / row_len;
		int near_cluster_index;
		int near_cluster_index_row;

		// prev row

		if (row % 2 == 0)
		{
			near_cluster_index = i - row_len - 1;
			near_cluster_index_row = near_cluster_index / row_len;
			if(row - 1 == near_cluster_index_row)
				if (get_hex_bounds(assignments, i, near_cluster_index))
				{
					//borders[assignments[i]].push_back(std::vector<int> {near_cluster_index, i});
					borders[assignments[i]].push_back(near_cluster_index);
					borders[assignments[i]].push_back(i);
				}

			near_cluster_index = i - row_len;
			near_cluster_index_row = near_cluster_index / row_len;
			if(row - 1 == near_cluster_index_row)
				if (get_hex_bounds(assignments, i, near_cluster_index))
				{
					//borders[assignments[i]].push_back(std::vector<int> {near_cluster_index, i});
					borders[assignments[i]].push_back(near_cluster_index);
					borders[assignments[i]].push_back(i);
				}
		}
		else
		{
			near_cluster_index = i - row_len;
			near_cluster_index_row = near_cluster_index / row_len;
			if(row - 1 == near_cluster_index_row)
				if (get_hex_bounds(assignments, i, near_cluster_index))
				{
					//borders[assignments[i]].push_back(std::vector<int> {near_cluster_index, i});
					borders[assignments[i]].push_back(near_cluster_index);
					borders[assignments[i]].push_back(i);
				}

			near_cluster_index = i - row_len + 1;
			near_cluster_index_row = near_cluster_index / row_len;
			if(row - 1 == near_cluster_index_row)
				if (get_hex_bounds(assignments, i, near_cluster_index))
				{
					//borders[assignments[i]].push_back(std::vector<int> {near_cluster_index, i});
					borders[assignments[i]].push_back(near_cluster_index);
					borders[assignments[i]].push_back(i);
				}
		} 

		// current row

		near_cluster_index = i - 1;
		near_cluster_index_row = near_cluster_index / row_len;
		if(row == near_cluster_index_row)
			if (get_hex_bounds(assignments, i, near_cluster_index))
			{
				//borders[assignments[i]].push_back(std::vector<int> {near_cluster_index, i});
				borders[assignments[i]].push_back(near_cluster_index);
				borders[assignments[i]].push_back(i);
			}

		near_cluster_index = i + 1;
		near_cluster_index_row = near_cluster_index / row_len;
		if(row == near_cluster_index_row)
			if (get_hex_bounds(assignments, i, near_cluster_index))
			{
				//borders[assignments[i]].push_back(std::vector<int> {i, near_cluster_index});
				borders[assignments[i]].push_back(i);
				borders[assignments[i]].push_back(near_cluster_index);
			}

		// next row 
		
		if (row % 2 == 0)
		{
			near_cluster_index = i + row_len - 1;
			near_cluster_index_row = near_cluster_index / row_len;
			if(row + 1 == near_cluster_index_row)
				if (get_hex_bounds(assignments, i, near_cluster_index))
				{
					//borders[assignments[i]].push_back(std::vector<int> {i, near_cluster_index});
					borders[assignments[i]].push_back(i);
					borders[assignments[i]].push_back(near_cluster_index);
				}

			near_cluster_index = i + row_len;
			near_cluster_index_row = near_cluster_index / row_len;
			if(row + 1 == near_cluster_index_row)
				if (get_hex_bounds(assignments, i, near_cluster_index))
				{
					//borders[assignments[i]].push_back(std::vector<int> {i, near_cluster_index});
					borders[assignments[i]].push_back(i);
					borders[assignments[i]].push_back(near_cluster_index);
				}
		}
		else
		{
			near_cluster_index = i + row_len;
			near_cluster_index_row = near_cluster_index / row_len;
			if(row + 1 == near_cluster_index_row)
				if (get_hex_bounds(assignments, i, near_cluster_index))
				{
					//borders[assignments[i]].push_back(std::vector<int> {i, near_cluster_index});
					borders[assignments[i]].push_back(i);
					borders[assignments[i]].push_back(near_cluster_index);
				}

			near_cluster_index = i + row_len + 1;
			near_cluster_index_row = near_cluster_index / row_len;
			if(row + 1 == near_cluster_index_row)
				if (get_hex_bounds(assignments, i, near_cluster_index))
				{
					//borders[assignments[i]].push_back(std::vector<int> {i, near_cluster_index});
					borders[assignments[i]].push_back(i);
					borders[assignments[i]].push_back(near_cluster_index);
				}
		} 
	}

	return {positions, borders};
}

int main(int argc, char *argv[])
{
	std::cout << "SOM example have started" << std::endl;
	std::cout << '\n';

	auto t1 = std::chrono::steady_clock::now();
	
	bool hyperparams_tune = false;
	bool default_hyperparams = false;

	if (argc > 1)
	{
		if (argv[1] == std::string("hyperparams_tune"))
		{
			hyperparams_tune = true;
		}
		else if (argv[1] == std::string("default"))
		{
			default_hyperparams = true;
		}
		else
		{
			RAW_DATA_DIRNAME = argv[1];
		}
	}

	/* Load data */
	auto speeds = readEnergies(RAW_DATA_DIRNAME);
	std::cout << "" << std::endl;
	std::cout << "Num records: " << speeds.size() << std::endl;
	std::cout << "Num values in the record: " << speeds[0].size() << std::endl;


	unsigned concurentThreadsSupported = std::thread::hardware_concurrency();
	std::cout << "Num cores: " << concurentThreadsSupported << std::endl;
	ThreadPool pool(concurentThreadsSupported);
	Semaphore sem;
	
	std::vector<int> graph_types = {1};
	std::vector<int> metric_types = {0};
	std::vector<int> distribution_types = {0, 1, 2};

	
	std::vector<std::vector<size_t>> grid_sizes = { {15, 15}, {20, 20}, {30, 30} };
	std::vector<double> s_learn_rates = {0.2, 0.4, 0.6, 0.8, 1.0};
	std::vector<double> f_learn_rates = {0.0, 0.1, 0.2, 0.3, 0.5, 0.7};
	std::vector<double> initial_neighbour_sizes = {1, 3, 5, 10, 20};
	std::vector<double> neigbour_range_decays = {1.5, 2.0, 3.0, 4.0};
	std::vector<long long> random_seeds = {0};
	std::vector<unsigned int> iterations_all = {1, 5, 10, 20};
				

	//
	std::vector<std::string> graph_type_names = {"Grid4", "Grid6", "Grid8", "Paley", "LPS", "Margulis"};
	std::vector<std::string> metric_type_names = {"Euclidean", "Manhatten", "P_norm", "Euclidean_thresholded", "Cosine", "Chebyshev"};
	std::vector<std::string> distribution_type_names = {"uniform_real_distribution", "normal_distribution", "exponential_distribution"};

	////
	//
	int best_graph;
	int best_metric;
	int best_distribution;
				
	size_t best_w_grid_size;
	size_t best_h_grid_size;

	double best_s_learn_rate;
	double best_f_learn_rate;
				
	double best_initial_neighbour_size;

	double best_neigbour_range_decay;

	long long best_random_seed;
	
	unsigned int best_iterations;

	//

	////
	if (hyperparams_tune)
	{
		std::vector<std::string> metaparams_grid = {"w_grid_size", "h_grid_size", "s_learn_rate", "f_learn_rate", "initial_neighbour_size", "neigbour_range_decay",
			"random_seed", "iterations", "distribution_type", "metric_type", "graph_type", "score"};
		std::vector<std::vector<std::string>> results_grid;

		//
		const int count = graph_types.size() * metric_types.size() * distribution_types.size() * 
			grid_sizes.size() * s_learn_rates.size() * f_learn_rates.size() * initial_neighbour_sizes.size() * neigbour_range_decays.size() * random_seeds.size() * iterations_all.size();

		std::vector<double> results(count, INFINITY);
		std::cout << "Num configurations: " << count << std::endl;

		int i = 0;
		for (auto grid_size : grid_sizes)
		{
			for (auto s_learn_rate : s_learn_rates)
			{
				for (auto f_learn_rate : f_learn_rates)
				{
					for (auto initial_neighbour_size : initial_neighbour_sizes)
					{
						for (auto neigbour_range_decay : neigbour_range_decays)
						{
							for (auto random_seed : random_seeds)
							{
								for (auto iterations : iterations_all)
								{
									for (auto distribution_type : distribution_types)
									{
										for (auto metric_type : metric_types)
										{
											for (auto graph_type : graph_types)
											{

												pool.execute([i, &sem, &results, &speeds, graph_type, metric_type, distribution_type, 
													iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, grid_size, random_seed, &results_grid, &metaparams_grid,
													&graph_type_names, &metric_type_names, &distribution_type_names]() {

													double score = INF;

													try {

														score = iterateThroughGraphs(grid_size[0], grid_size[1], graph_type, metric_type, distribution_type, i, speeds, 
															iterations, s_learn_rate, f_learn_rate, initial_neighbour_size, neigbour_range_decay, random_seed);

													}
													catch (const std::runtime_error& e) {
														std::cout << "configuration #" << i << ": runtime error: " << e.what() << std::endl;
													}
													catch (const std::exception& e) {
														std::cout << "configuration #" << i << ": exception: " << e.what() << std::endl;
													}
													catch (...) {
														std::cout << "configuration #" << i << ": unknown error" << std::endl;
													}
												
													mu.lock();
												
													std::vector<std::string> current_result = {std::to_string(grid_size[0]), std::to_string(grid_size[1]), std::to_string(s_learn_rate), 
																								std::to_string(f_learn_rate), 
																								std::to_string(initial_neighbour_size), std::to_string(neigbour_range_decay),
																								std::to_string(random_seed), std::to_string(iterations), 
																								distribution_type_names[distribution_type], metric_type_names[metric_type], graph_type_names[graph_type], 
																								std::to_string(score)};

													results_grid.push_back(current_result);

													if (i % 10000 == 0) {
														saveToCsv("assets/metaparams_checkpoint_" + std::to_string(i) + ".csv", results_grid, metaparams_grid);
													}

													mu.unlock();

													results.at(i) = score;

													sem.notify();
												});

												i++;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	
		for (auto grid_size : grid_sizes)
		{
			for (auto s_learn_rate : s_learn_rates)
			{
				for (auto f_learn_rate : f_learn_rates)
				{
					for (auto initial_neighbour_size : initial_neighbour_sizes)
					{
						for (auto neigbour_range_decay : neigbour_range_decays)
						{
							for (auto random_seed : random_seeds)
							{
								for (auto iterations : iterations_all)
								{
									for (auto distribution_type : distribution_types)
									{
										for (auto metric_type : metric_types)
										{
											for (auto graph_type : graph_types)
											{
												sem.wait();
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		pool.close();
	
		saveToCsv("assets/metaparams_checkpoint_final.csv", results_grid, metaparams_grid);
	
		double minimal_score = INFINITY;

		i = 0;
		for (auto grid_size : grid_sizes)
		{
			for (auto s_learn_rate : s_learn_rates)
			{
				for (auto f_learn_rate : f_learn_rates)
				{
					for (auto initial_neighbour_size : initial_neighbour_sizes)
					{
						for (auto neigbour_range_decay : neigbour_range_decays)
						{
							for (auto random_seed : random_seeds)
							{
								for (auto iterations : iterations_all)
								{
									for (auto distribution_type : distribution_types)
									{
										for (auto metric_type : metric_types)
										{
											for (auto graph_type : graph_types)
											{
												if (results[i] < minimal_score)
												{
													minimal_score = results[i];
													best_w_grid_size = grid_size[0];
													best_h_grid_size = grid_size[1];
													
													best_graph = graph_type;
													best_metric = metric_type;
													best_distribution = distribution_type;
												
													best_s_learn_rate = s_learn_rate;
													best_f_learn_rate = f_learn_rate;
													best_initial_neighbour_size = initial_neighbour_size;
													best_neigbour_range_decay = neigbour_range_decay;
													best_random_seed = random_seed;
													best_iterations = iterations;
												}
												i++;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

	
		std::cout << std::endl;
		std::cout << std::endl;
		std::cout << "The best configuration: " << std::endl;
		std::cout << "  Score: " << minimal_score << std::endl;
		std::cout << "  Graph: " << graph_type_names[best_graph] << std::endl;
		std::cout << "  Distance: " << metric_type_names[best_metric] << std::endl;
		std::cout << "  Distribution: " << distribution_type_names[best_distribution] << std::endl;
		std::cout << "  Grid size: " << best_w_grid_size << "x" << best_h_grid_size << std::endl;
		std::cout << "  Iterations: " << best_iterations << std::endl;
		std::cout << "  Start learn rate: " << best_s_learn_rate << std::endl;
		std::cout << "  Final learn rate: " << best_f_learn_rate << std::endl;
		std::cout << "  Initial neighbour size: " << best_initial_neighbour_size << std::endl;
		std::cout << "  Neigbour range decay: " << best_neigbour_range_decay << std::endl;
		std::cout << "  Random seeds: " << best_random_seed << std::endl;
	}
	else
	{
		// load metaparms tune results and shood the best (with the lowets score)

		auto metaparams_grid = readCsvData("assets/metaparams_checkpoint_final.csv", ',');
		
		std::vector<double> scores;

		for (auto row : metaparams_grid)
		{
			scores.push_back(std::stod(row[11]));
		}
		
		std::cout << std::endl;
		std::cout << "Num scores: " << scores.size() << std::endl;

		int minElementIndex = std::min_element(scores.begin(), scores.end()) - scores.begin();
		//
		//std::cout << "The best metaparams index: " << minElementIndex << std::endl;
		
		//auto it = std::find (graph_type_names.begin(), graph_type_names.end(), metaparams_grid[minElementIndex][10]); 
		//best_graph = std::distance(graph_type_names.begin(), it);
		best_graph = 1;
		
		//it = std::find (metric_type_names.begin(), metric_type_names.end(), metaparams_grid[minElementIndex][9]); 
		//best_metric = std::distance(metric_type_names.begin(), it);
		best_metric = 0;
		
		auto it = std::find (distribution_type_names.begin(), distribution_type_names.end(), metaparams_grid[minElementIndex][8]); 
		best_distribution = std::distance(distribution_type_names.begin(), it);
						
		best_w_grid_size = std::stod(metaparams_grid[minElementIndex][0]);
		best_h_grid_size = std::stod(metaparams_grid[minElementIndex][1]);
		best_s_learn_rate = std::stod(metaparams_grid[minElementIndex][2]);
		best_f_learn_rate = std::stod(metaparams_grid[minElementIndex][3]);
		best_initial_neighbour_size = std::stod(metaparams_grid[minElementIndex][4]);
		best_neigbour_range_decay = std::stod(metaparams_grid[minElementIndex][5]);
		best_random_seed = std::stod(metaparams_grid[minElementIndex][6]);
		best_iterations = std::stod(metaparams_grid[minElementIndex][7]);

		// defaults
		if (default_hyperparams)
		{
			best_distribution = 0;
			best_w_grid_size = 30;
			best_h_grid_size = 20;
			best_s_learn_rate = 0.8;
			best_f_learn_rate = 0.0;
			best_initial_neighbour_size = std::sqrt(double(best_w_grid_size * best_h_grid_size)); // use default
			best_neigbour_range_decay = 2.0; // use default
			best_random_seed = 0;
			best_iterations = 20;
		}
	
		std::cout << std::endl;
		std::cout << "The best configuration: " << std::endl;
		std::cout << "  Score: " << scores[minElementIndex] << std::endl;
		std::cout << "  Graph: " << graph_type_names[best_graph] << std::endl;
		std::cout << "  Distance: " << metric_type_names[best_metric] << std::endl;
		std::cout << "  Distribution: " << distribution_type_names[best_distribution] << std::endl;
		std::cout << "  Grid size: " << best_w_grid_size << "x" << best_h_grid_size << std::endl;
		std::cout << "  Iterations: " << best_iterations << std::endl;
		std::cout << "  Start learn rate: " << best_s_learn_rate << std::endl;
		std::cout << "  Final learn rate: " << best_f_learn_rate << std::endl;
		std::cout << "  Initial neighbour size: " << best_initial_neighbour_size << std::endl;
		std::cout << "  Neigbour range decay: " << best_neigbour_range_decay << std::endl;
		std::cout << "  Random seeds: " << best_random_seed << std::endl;
		std::cout << std::endl;
	}

	// if overrided from arguments
	
	if (argc > 3)
	{
		best_w_grid_size = std::stod(argv[2]);
		best_h_grid_size = std::stod(argv[3]);
	}
	
	if (argc > 4)
	{
		CLUSTERS_NUM = std::stod(argv[4]);
	}

	// create, train SOM over the raw data and reduce the data	
	// then make clustering on the reduced data

	auto [assignments, counts, clustered_energies] = iterateThroughGraphsBest(best_w_grid_size, best_h_grid_size, best_graph, best_metric, best_distribution, speeds, 
		best_iterations, best_s_learn_rate, best_f_learn_rate, best_initial_neighbour_size, best_neigbour_range_decay, best_random_seed);

	// calculate borders and positions of each cluster

	auto [positions, borders] = getPositionsAndBorders(assignments, counts.size(), best_w_grid_size);
	
	std::cout << "positions:" << std::endl;
	matrix_print(positions);
	std::cout << std::endl;

	std::cout << "borders:" << std::endl;
	matrix_print(borders);
	std::cout << std::endl;

	// sort clusters by mean energies
		
	std::vector<double> energy_means(clustered_energies[0].size());
	std::vector<size_t> energy_means_sorted_indexes;
	std::vector<std::vector<double>> energy_for_means(clustered_energies[0].size());
	for (int i = 0; i < clustered_energies.size(); i++)
	{
		for (int j = 0; j < clustered_energies[i].size(); j++)
		{
			for (int k = 0; k < clustered_energies[i][j].size(); k++)
			{
				for (int p = 0; p < clustered_energies[i][j][k].size(); p++)
				{
					energy_for_means[j].push_back(clustered_energies[i][j][k][p]);
				}
			}
		}
	}
	for (int i = 0; i < energy_for_means.size(); i++)
	{
		energy_means[i] = std::accumulate( energy_for_means[i].begin(), energy_for_means[i].end(), 0.0) / energy_for_means[i].size();
	}

	energy_means_sorted_indexes = sort_indexes(energy_means);
	
	std::cout << "energy means:" << std::endl;
	vector_print(energy_means);
	std::cout << std::endl;
	
	std::cout << "energy means sorted indexs:" << std::endl;
	vector_print(energy_means_sorted_indexes);
	std::cout << std::endl;

	// calculate confs based on clustered energies and fill the result json
	
	std::vector<std::string> conf_names = {"conf_l", "conf_m", "conf_r"};
	std::vector<std::string> sensor_names = {"vorne_li-1", "vorne_li-2", "vorne_li-3", "hinten_re-1", "vorne_re-1", "vorne_re-2", "vorne_re-3", "hinten_re-2"};
	std::vector<uint32_t> windowSizes = {12, 24, 48, 96, 192, 384};
	uint32_t samples = 1000;
	double confidencelevel = 0.99;
	
	std::cout << "--->" << std::endl;
	json reference_data;
	int sensor_index = 0;
	for (auto sensor_data : clustered_energies)
	{		
		std::cout << "  ---> sensor " << sensor_index << std::endl;
		std::vector<json> clusters_json;

		// get cluster index from sorted by energy means
		for(auto ei : energy_means_sorted_indexes)
		{			
			auto cluster_data = sensor_data[ei];
			std::cout << "    ---> cluster " << ei << std::endl;
			// if there are data in the cluster (size of subbands is equal, so we look at the first subband size)
			if (cluster_data[0].size() > 1)
			{
				std::vector<json> energy_subbands_json;

				for (int ci = 0; ci < cluster_data.size(); ci++)
				{
					auto energy_subband_data = cluster_data[ci];

					// returns quants for a single subbund
					std::vector<std::vector<std::vector<double>>> multiquants = set2multiconf(energy_subband_data, windowSizes, samples, confidencelevel);

					json energy_subband_json;
					for (auto window : multiquants)
					{
						json window_json = {
							{"conf_l", window[0]},
							{"conf_m", window[1]},
							{"conf_r", window[2]}
						};
						energy_subband_json.push_back(window_json);
					}
					energy_subbands_json.push_back(energy_subband_json);
					//std::cout << "      ---:" << std::endl;
				}
				json cluster_json = {
					{"name", "Level" + std::to_string(ei)},
					{"border", borders[ei]},
					{"position", positions[ei]},
					{"quant", energy_subbands_json}
				};
				clusters_json.push_back(cluster_json);
				//std::cout << "    ---:" << std::endl;
			}
		}
		
		json sensor_json = {
			{"id", sensor_names[sensor_index]}, 		
			{"data", clusters_json}
		};
		reference_data.push_back(sensor_json);
		sensor_index++;
		std::cout << "  ---:" << std::endl;
	}
	std::ofstream outputFile(RAW_DATA_DIRNAME + "/result/reference_data_" + std::to_string(best_w_grid_size) + "x" + std::to_string(best_h_grid_size) + ".json");
	outputFile << std::setw(4) << reference_data << std::endl;
	outputFile.close();	
	std::cout << "---:" << std::endl;

	auto t2 = std::chrono::steady_clock::now();
	std::cout << "(Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << " s)" << std::endl;
	std::cout << "" << std::endl;


    return 0;
}
