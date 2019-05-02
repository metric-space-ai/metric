#include "Edit.hpp"
#include <vector>
#include <vector>
#include <algorithm>

namespace metric
{
namespace distance
{

template <typename V>
template <typename Container>
auto Edit<V>::operator()(const Container &str1, const Container &str2) const -> distance_type
{
    size_t sizeA = str1.size();
    size_t sizeB = str2.size();

//TODO: check empty strings.

    std::vector<int> D0(sizeB+1);
    std::vector<int> Di(sizeB+1);

    int C1, C2, C3;


    // first element
    D0[0] = 0;

    // first row
    for (std::size_t j = 0; j < sizeB+1; j++)
    {
        // editDistance[0][j] = j;
        D0[j] = j;
    }

    // second-->last row
    for (std::size_t i = 1; i < sizeA+1; i++)
    {
        // every first element in row
        Di[0] = i;

        // remaining elements in row
        for (std::size_t j = 1; j < sizeB+1; j++)
        {
            if (str1[i - 1] == str2[j - 1])
            {
                Di[j] = D0[j - 1];
            }
            else
            {
                C1 = D0[j];
                C2 = Di[j - 1];
                C3 = D0[j - 1];
                Di[j] = (C1 < ((C2 < C3) ? C2 : C3)) ? C1 : ((C2 < C3) ? C2 : C3); //Di[j] = std::min({C1,C2,C3});
                Di[j] += 1;
            }
        }
        std::swap(D0, Di);
    }

    distance_type rvalue = D0[sizeB]; // +1 -1

    return rvalue;
}


} // namespace distance
} // namespace metric
