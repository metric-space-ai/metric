/*
		This Source Code Form is subject to the terms of the Mozilla Public
		License, v. 2.0. If a copy of the MPL was not distributed with this
		file, You can obtain one at http://mozilla.org/MPL/2.0/.

		Copyright (c) 2019 Panda Team
		Copyright (c) 2019 Oleg Popov
*/
/*
Laplacians is a package containing graph algorithms, with an emphasis on tasks related to 
spectral and algebraic graph theory. It contains (and will contain more) code for solving 
systems of linear equations in graph Laplacians, low stretch spanning trees, sparsifiation, 
clustering, local clustering, and optimization on graphs.

All graphs are represented by sparse adjacency matrices. This is both for speed, and because 
our main concerns are algebraic tasks. It does not handle dynamic graphs. It would be very slow 
to implement dynamic graphs this way.

https://github.com/danspielman/Laplacians.jl
*/

#ifndef _METRIC_UTILS_GRAPH_PARTITION_HPP
#define _METRIC_UTILS_GRAPH_PARTITION_HPP

#include <cmath>
#include "../../../3rdparty/blaze/Math.h"
#include <blaze/math/DynamicMatrix.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include "../solver/solver.hpp"

namespace metric {
/*
	Just implements Spielman-Srivastava
		as = sparsify(a; ep=0.5)

	Apply Spielman-Srivastava sparsification: sampling by effective resistances.
	`ep` should be less than 1.
*/


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
	left *= right;
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
void RemoveMatrixRows(blaze::DynamicMatrix<Tv> &source, blaze::DynamicVector<int> removeRows)
{
	blaze::DynamicMatrix<Tv> newMatrix(source.rows(), removeRows.size());

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
	return;
}

//template <typename Tv>
//double Union(blaze::DynamicVector<blaze::DynamicVector<Tv>> source)
//{
//	blaze::DynamicVector<Tv> result;
//
//	for (int i = 0; i < source.size(); i++)
//	{
//		for (int j = 0; j < source[i].size(); j++)
//		{
//			if (result.find(source[i][j]) == source[i].end())
//			{
//				
//			}
//		}
//	}
//
//	result /= source.size();
//	return result;
//}


/**
 * @brief Apply Spielman-Srivastava sparsification: sampling by effective resistances.
 * 
 * @param a 
 * @param ep 
 * @param matrixConcConst 
 * @param JLfac 
 * @return
 */
bool perform_graph_partition(blaze::DynamicMatrix<double> distanceMatrix, blaze::DynamicMatrix<int> &partition);
}

#include "partition.cpp"
#endif
