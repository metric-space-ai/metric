/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/

#include <vector>
#include <blaze/Math.h>

namespace assets
{

/** smooth reduces of noise by threshold and give back a sparse vector. **/
template <typename T>
blaze::CompressedVector<T>
smoothDenoise(std::vector<T> const &data, T const &tresh)
{

    // initialize
    blaze::CompressedVector<T> svector(data.size());
    svector.reserve(data.size());
    bool lastEqualsZero;
    bool keepNext;
    lastEqualsZero = false;
    keepNext = false;

    //iterator through data
    for (std::size_t i = 0; i < data.size(); i++)
    {
        if (data[i] != T(0))
        {
            //bigger than theshold
            if (std::abs(data[i]) > tresh)
            {
                if (lastEqualsZero == true)
                {

                    svector.set(i - 1, data[i - 1]); // make retroactive changes to last value not to be zero
                    lastEqualsZero = false;
                }

                svector.append(i, data[i]);
                keepNext = true;
            }
            // smaller than theshold
            else
            {
                if (keepNext == true)
                {

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

/** add zero padding to sparsed vector (preprocessing for time elatic distance) **/
template <typename T>
blaze::CompressedVector<T>
zeroPad(blaze::CompressedVector<T> const &data)
{
    // adds zero pads to blaze::sparsevector (for preparing sed)
    blaze::CompressedVector<T> data_zeropadded(data.size());
    data_zeropadded.reserve(2 + data.nonZeros() * 2);
    T value;
    bool addZeroFront;
    bool addZeroLastBack;
    int index;
    int index_last = -1;

    if (data.nonZeros() == 0)
    {
        data_zeropadded.set(0, T(0));
        data_zeropadded.set(data.size() - 1, T(0));
    }
    else
    {

        for (blaze::CompressedVector<double>::ConstIterator it = data.cbegin(); it != data.cend(); ++it)
        {
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
        if (index_last < int(data.size()) - 1)
        {
            data_zeropadded.append(data.size() - 1, T(0));
        }
    }

    shrinkToFit(data_zeropadded);
    return data_zeropadded;
}

/*** distance measure by time elastic cost matrix. ***/
template <typename T>
T sed(blaze::CompressedVector<T> const &As, blaze::CompressedVector<T> const &Bs, T const &penalty = 0, T const &elastic = 1)
{
    std::vector<T> A;
    A.reserve(As.nonZeros());
    std::vector<T> timeA;
    timeA.reserve(As.nonZeros());

    std::vector<T> B;
    B.reserve(Bs.nonZeros());

    std::vector<T> timeB;
    timeB.reserve(Bs.nonZeros());

    for (blaze::CompressedVector<double>::ConstIterator it = As.cbegin(); it != As.cend(); ++it)
    {
        timeA.push_back(it->index()); // Read access to the index of the non-zero element.
        A.push_back(it->value());     // Read access to the value of the non-zero element.
    }

    for (blaze::CompressedVector<double>::ConstIterator it = Bs.cbegin(); it != Bs.cend(); ++it)
    {
        timeB.push_back(it->index()); // Read access to the index of the non-zero element.
        B.push_back(it->value());     // Read access to the value of the non-zero element.
    }

    T C1, C2, C3;

    int sizeB = B.size();
    int sizeA = A.size();

    std::vector<T> D0(sizeB);
    std::vector<T> Di(sizeB);

    // first element
    D0[0] = std::abs(A[0] - B[0]) + elastic * (std::abs(timeA[0] - 0)); // C3

    // first row
    for (int j = 1; j < sizeB; j++)
    {
        D0[j] = D0[j - 1] + std::abs(B[j - 1] - B[j]) + elastic * (timeB[j] - timeB[j - 1]) + penalty; // C2
    }

    // second-->last row
    for (int i = 1; i < sizeA; i++)
    {
        // every first element in row
        Di[0] = D0[0] + std::abs(A[i - 1] - A[i]) + elastic * (timeA[i] - timeA[i - 1]) + penalty; // C1

        // remaining elements in row
        for (int j = 1; j < sizeB; j++)
        {
            C1 = D0[j] + std::abs(A[i - 1] - A[i]) + elastic * (timeA[i] - timeA[i - 1]) + penalty;
            C2 = Di[j - 1] + std::abs(B[j - 1] - B[j]) + elastic * (timeB[j] - timeB[j - 1]) + penalty;
            C3 = D0[j - 1] + std::abs(A[i] - B[j]) + std::abs(A[i - 1] - B[j - 1]) + elastic * (std::abs(timeA[i] - timeB[j]) + std::abs(timeA[i - 1] - timeB[j - 1]));
            Di[j] = (C1 < ((C2 < C3) ? C2 : C3)) ? C1 : ((C2 < C3) ? C2 : C3); //Di[j] = std::min({C1,C2,C3});
        }
        std::swap(D0, Di);
    }

    T rvalue = D0[sizeB - 1];

    return rvalue;
}
 
/** linspace (generaters a "line curve") **/
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

} // end namespace
