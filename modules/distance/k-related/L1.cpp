/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include "L1.hpp"
namespace metric {

// the UNFINISHED code for L1 metrics from the review papaer

template <typename V>
template <typename Container>
auto Sorensen<V>::operator()(const Container& a, const Container& b) const
    -> typename std::enable_if<!std::is_same<Container, V>::value, distance_type>::type
{
    double sum1 = 0;
    double sum2 = 0;
    Container z { 0 };
    auto it1 = a.begin();
    auto it2 = b.begin();

    while (true) {
        if (it1 == it2)  // both ends reached
            break;

        sum1 += (*it1 - *it2);
        sum2 += (*it1 + *it2);

        if (it1 + 1 != a.end() && it1 != z.begin())
            ++it1;
        else
            it1 = z.begin();  // end reached, using zero against values of other vector
        if (it2 + 1 != b.end() && it1 != z.begin())
            ++it2;
        else
            it2 = z.begin();
    }

    if (sum1 < 0)
        sum1 = -sum1;
    return sum1 / sum2;  // here the type is changed
}

template <typename V>
auto Sorensen<V>::operator()(const blaze::CompressedVector<V>& a, const blaze::CompressedVector<V>& b) const
    -> distance_type
{
    double sum1 = 0;
    double sum2 = 0;
    blaze::CompressedVector<V> z { 0 };
    auto it1 = a.begin();
    auto it2 = b.begin();

    while (true) {
        if (it1 == it2)  // both ends reached
            break;

        sum1 += (it1->value() - it2->value());
        sum2 += (it1->value() + it2->value());

        // TODO implement element skipping!!

        if (it1 + 1 != a.end() && it1 != z.begin())
            ++it1;
        else
            it1 = z.begin();  // end reached, using zero against values of other vector
        if (it2 + 1 != b.end() && it1 != z.begin())
            ++it2;
        else
            it2 = z.begin();
    }

    if (sum1 < 0)
        sum1 = -sum1;
    return sum1 / sum2;  // here the type is changed
}

}  // namespace metric
