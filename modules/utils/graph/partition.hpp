/*
		This Source Code Form is subject to the terms of the Mozilla Public
		License, v. 2.0. If a copy of the MPL was not distributed with this
		file, You can obtain one at http://mozilla.org/MPL/2.0/.

		Copyright (c) 2019 Panda Team
		Copyright (c) 2019 Oleg Popov
*/

#ifndef _METRIC_UTILS_GRAPH_PARTITION_HPP
#define _METRIC_UTILS_GRAPH_PARTITION_HPP

#include <cmath>
#include "../../../3rdparty/blaze/Math.h"
#include <blaze/math/DynamicMatrix.h>
#include <blaze/math/Rows.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include "../solver/solver.hpp"

namespace metric {

//TO DO don't forgot to move this
int RandomIntInRange(int left, int right) { return left + (std::rand() % (right - left + 1)); }

template <typename Tv>
blaze::DynamicMatrix<Tv> CloneMatrix(blaze::DynamicMatrix<Tv> source)
{
	blaze::DynamicMatrix<Tv> newMatrix(source);
	return newMatrix;
}

template <typename Tv>
blaze::DynamicMatrix<Tv> DotMatrix(blaze::DynamicMatrix<Tv> left, blaze::DynamicMatrix<Tv> right)
{
	blaze::DynamicMatrix<Tv> result(left);
	result *= right;
	return result;
}

template <typename Tv>
double VectorMean(blaze::DynamicVector<Tv> source)
{
	double result = 0;
	for (int i = 0; i < source.size(); i++)
	{
		result += source[i];
	}

	result /= source.size();
	return result;
}


template <typename Tv>
void PrintArray(blaze::DynamicVector<Tv> source)
{
	std::cout<<"array[" << source.size() << "] ";
	for (int i = 0; i < source.size(); i++)
	{
		std::cout<<source[i]<<", ";
	}
	std::cout<<"\n";
}

template <typename Tv>
void PrintMatrix(blaze::DynamicMatrix<Tv> source)
{
	std::cout << "matrix[" << source.rows() << ", " << source.columns() << "] ";
	for (int i = 0; i < source.rows(); i++)
	{
		std::cout << "\n";
		for (int j = 0; j < source.columns(); j++)
		{
			std::cout << source(i, j) << ", ";
		}
	}
	std::cout << "\n";
}

template <typename Tv>
blaze::DynamicMatrix<Tv> RemoveMatrixRows(blaze::DynamicMatrix<Tv> source, blaze::DynamicVector<int> removeRows)
{
	blaze::DynamicMatrix<Tv> newMatrix(source.rows() - removeRows.size(), removeRows.size());

	int rowCount = 0;
	for (int i = 0; i < source.rows(); i++)
	{
		bool skipRow = false;
		for (int j = 0; j < removeRows.size(); j++)
		{
			if (i == removeRows[j])
			{
				skipRow = true;
				break;
			}
		}

		if(skipRow) continue;

		for (int j = 0; j < source.columns(); j++)
		{
			newMatrix(rowCount, j) = source(i, j);
		}
		rowCount++;
	}

	return newMatrix;
}

bool perform_graph_partition(blaze::DynamicMatrix<double> distanceMatrix, blaze::DynamicMatrix<int> &partition);

}

#include "partition.cpp"
#endif
