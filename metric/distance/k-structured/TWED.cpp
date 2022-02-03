/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_K_STRUCTURED_TWED_CPP
#define _METRIC_DISTANCE_K_STRUCTURED_TWED_CPP
#include "TWED.hpp"
#include <algorithm>
#include <vector>

namespace metric {

/*** distance measure with time elastic cost matrix. ***/
template <typename V>
template <typename Container>
auto TWED<V>::operator()(const Container &As, const Container &Bs) const -> distance_type
{
	std::vector<value_type> A;
	A.reserve(As.size());
	std::vector<value_type> timeA;
	timeA.reserve(As.size());

	std::vector<value_type> B;
	B.reserve(Bs.size());

	std::vector<value_type> timeB;
	timeB.reserve(Bs.size());

	for (auto it = As.cbegin(); it != As.cend(); ++it) {
		if constexpr (std::is_same<blaze::CompressedVector<V>, Container>::value) {
			timeA.push_back(it->index()); // Read access to the index of the non-zero element.
			A.push_back(it->value());	  // Read access to the value of the non-zero element.
		} else {
			timeA.push_back(std::distance(As.begin(), it)); // Read access to the index of the non-zero element.
			A.push_back(*it);								// Read access to the value of the non-zero element.
		}
	}

	for (auto it = Bs.cbegin(); it != Bs.cend(); ++it) {
		if constexpr (std::is_same<blaze::CompressedVector<V>, Container>::value) {
			timeB.push_back(it->index()); // Read access to the index of the non-zero element.
			B.push_back(it->value());	  // Read access to the value of the non-zero element.
		} else {
			timeB.push_back(std::distance(Bs.begin(), it)); // Read access to the index of the non-zero element.
			B.push_back(*it);								// Read access to the value of the non-zero element.
		}
	}

	value_type C1, C2, C3;

	int sizeB = B.size();
	int sizeA = A.size();

	std::vector<value_type> D0(sizeB);
	std::vector<value_type> Di(sizeB);

	// first element
	D0[0] = std::abs(A[0] - B[0]) + elastic * (std::abs(timeA[0] - 0)); // C3

	// first row
	for (int j = 1; j < sizeB; j++) {
		D0[j] = D0[j - 1] + std::abs(B[j - 1] - B[j]) + elastic * (timeB[j] - timeB[j - 1]) + penalty; // C2
	}

	// second-->last row
	for (int i = 1; i < sizeA; i++) {
		// every first element in row
		Di[0] = D0[0] + std::abs(A[i - 1] - A[i]) + elastic * (timeA[i] - timeA[i - 1]) + penalty; // C1

		// remaining elements in row
		for (int j = 1; j < sizeB; j++) {
			C1 = D0[j] + std::abs(A[i - 1] - A[i]) + elastic * (timeA[i] - timeA[i - 1]) + penalty;
			C2 = Di[j - 1] + std::abs(B[j - 1] - B[j]) + elastic * (timeB[j] - timeB[j - 1]) + penalty;
			C3 = D0[j - 1] + std::abs(A[i] - B[j]) + std::abs(A[i - 1] - B[j - 1]) +
				 elastic * (std::abs(timeA[i] - timeB[j]) + std::abs(timeA[i - 1] - timeB[j - 1]));
			Di[j] = (C1 < ((C2 < C3) ? C2 : C3)) ? C1 : ((C2 < C3) ? C2 : C3); // Di[j] = std::min({C1,C2,C3});
			// std::cout << Di[j] << " [" << C1 << " " << C2 << " " << C3 << "] |  "; // code for debug, added by Max F
		}
		// std::cout << "\n"; // code for debug, added by Max F
		std::swap(D0, Di);
	}

	distance_type rvalue = D0[sizeB - 1];

	return rvalue;
}

namespace TWED_details {
/** add zero padding to sparsed vector (preprocessing for time elatic distance) **/
template <typename T> blaze::CompressedVector<T> addZeroPadding(blaze::CompressedVector<T> const &data)
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
			index = it->index(); // Read access to the index of the non-zero element.
			value = it->value(); // Read access to the value of the non-zero element.

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

		if (index_last < int(data.size()) - 2) // vorletzter nicht vorhanden
		{
			data_zeropadded.append(index_last + 1, T(0));
		}
		if (index_last < int(data.size()) - 1) {
			data_zeropadded.append(data.size() - 1, T(0));
		}
	}

	shrinkToFit(data_zeropadded);
	return data_zeropadded;
}

} // namespace TWED_details

} // namespace metric
#endif // header guard
