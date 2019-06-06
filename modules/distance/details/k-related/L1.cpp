#include "L1.hpp"

namespace metric
{
namespace distance
{


template <typename V>
template<typename Container>
auto Sorensen<V>::operator()(const Container &a, const Container &b) const -> distance_type
{
  static_assert(
      std::is_floating_point<value_type>::value, "T must be a float type");
  distance_type sum1 = 0;
  distance_type sum2 = 0;
  for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2)
  {
    sum1 += (*it1 - *it2);
    sum2 += (*it1 + *it2);
  }
  return sum1/sum2;
}




} // namespace distance
} // namespace metric
