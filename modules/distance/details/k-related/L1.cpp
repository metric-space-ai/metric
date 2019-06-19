#include "L1.hpp"

//#include <cmath>

namespace metric
{
namespace distance
{

// the UNFINISHED code for L1 metrics from the review papaer


/*
template <typename V>
template<typename Container>
auto Sorensen_t<V>::operator()(const Container &a, const Container &b) const -> distance_type
//typename Sorensen_t<V>::distance_type Sorensen<V>::operator()(const Container &a, const Container &b) const
{
    static_assert(std::is_floating_point<value_type>::value, "T must be a float type");

    distance_type sum1 = 0;
    distance_type sum2 = 0;
    for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2) // TODO add support of vectors of different length!!
    {
        sum1 += (*it1 - *it2);
        sum2 += (*it1 + *it2);
    }

    //  return std::fabs(sum1)/sum2;
    if (sum1<0)
        sum1 = -sum1;
    return sum1/sum2;  // here the type is changed
}




template<typename Container>
typename Container::value_type Sorensen_s::operator()(const Container &a, const Container &b) const
{
    static_assert(std::is_floating_point<typename Container::value_type>::value, "Data must be of a float type");

    typename Container::value_type sum1 = 0;
    typename Container::value_type sum2 = 0;
    for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2) // TODO add support of vectors of different length!!
    {
        sum1 += (*it1 - *it2);
        sum2 += (*it1 + *it2);
    }

    //  return std::fabs(sum1)/sum2;
    if (sum1<0)
        sum1 = -sum1;
    return sum1/sum2;
}



template <typename V>
template<typename Container>
V Sorensen<V>::operator()(const Container &a, const Container &b) const
{
    V sum1 = 0;
    V sum2 = 0;
    Container z {0};
    auto it1 = a.begin();
    auto it2 = b.begin();
    static_assert(std::is_floating_point<typename std::remove_reference<decltype(*it1)>::type>::value, "T must be a float type");
    while (true)
    {
        if (it1 == it2) // both ends reached
            break;

        sum1 += (*it1 - *it2);
        sum2 += (*it1 + *it2);
        //sum1 += (it1->value() - it2->value());
        //sum2 += (it1->value() + it2->value());

        if (it1+1 != a.end() && it1 != z.begin())
            ++it1;
        else
            it1 = z.begin(); // end reached, using zero against values of other vector
        if (it2+1 != b.end() && it1 != z.begin())
            ++it2;
        else
            it2 = z.begin();
    }

    //  return std::fabs(sum1)/sum2;
    if (sum1<0)
        sum1 = -sum1;
    return sum1/sum2;  // here the type is changed
}


template <typename V>
template<typename ValueType>
V Sorensen<V>::operator()(const blaze::CompressedVector<ValueType> &a, const blaze::CompressedVector<ValueType> &b) const
{
    V sum1 = 0;
    V sum2 = 0;
    blaze::CompressedVector<ValueType> z {0};
    auto it1 = a.begin();
    auto it2 = b.begin();
    static_assert(std::is_floating_point<typename std::remove_reference<decltype(it1->value())>::type>::value, "T must be a float type");

    while (true)
    {
        if (it1 == it2) // both ends reached
            break;

        //sum1 += (*it1 - *it2);
        //sum2 += (*it1 + *it2);
        sum1 += (it1->value() - it2->value());
        sum2 += (it1->value() + it2->value());

        if (it1+1 != a.end() && it1 != z.begin())
            ++it1;
        else
            it1 = z.begin(); // end reached, using zero against values of other vector
        if (it2+1 != b.end() && it1 != z.begin())
            ++it2;
        else
            it2 = z.begin();
    }

    //  return std::fabs(sum1)/sum2;
    if (sum1<0)
        sum1 = -sum1;
    return sum1/sum2;  // here the type is changed
}

*/


//template <template <typename> class Container, typename Value>
//double sorensen(Container<Value> &a, Container<Value> &b)
//{
//    return 0;
//}

//template <template <typename, typename> class Container, typename Value, typename Allocator>
//double sorensen(Container<Value, Allocator> &a, Container<Value, Allocator> &b)
//{
//    return -1;
//}

template <typename T>
double sorensen(T &a, T &b)
{
    double sum1 = 0;
    double sum2 = 0;
    T z {0};
    auto it1 = a.begin();
    auto it2 = b.begin();
//    static_assert(std::is_floating_point<typename std::remove_reference<decltype(*it1)>::type>::value, "T must be a float type");
    while (true)
    {
        if (it1 == it2) // both ends reached
            break;

        sum1 += (*it1 - *it2);
        sum2 += (*it1 + *it2);
        //sum1 += (it1->value() - it2->value());
        //sum2 += (it1->value() + it2->value());

        if (it1+1 != a.end() && it1 != z.begin())
            ++it1;
        else
            it1 = z.begin(); // end reached, using zero against values of other vector
        if (it2+1 != b.end() && it1 != z.begin())
            ++it2;
        else
            it2 = z.begin();
    }

    //  return std::fabs(sum1)/sum2;
    if (sum1<0)
        sum1 = -sum1;
    return sum1/sum2;  // here the type is changed
}

template <typename Value>
double sorensen(blaze::CompressedVector<Value> &a, blaze::CompressedVector<Value> &b)
{
    double sum1 = 0;
    double sum2 = 0;
    blaze::CompressedVector<Value> z {0};
    auto it1 = a.begin();
    auto it2 = b.begin();
//    static_assert(std::is_floating_point<typename std::remove_reference<decltype(it1->value())>::type>::value, "T must be a float type");

    while (true)
    {
        if (it1 == it2) // both ends reached
            break;

        //sum1 += (*it1 - *it2);
        //sum2 += (*it1 + *it2);
        sum1 += (it1->value() - it2->value());
        sum2 += (it1->value() + it2->value());

        // TODO implement element skipping!!

        if (it1+1 != a.end() && it1 != z.begin())
            ++it1;
        else
            it1 = z.begin(); // end reached, using zero against values of other vector
        if (it2+1 != b.end() && it1 != z.begin())
            ++it2;
        else
            it2 = z.begin();
    }

    //  return std::fabs(sum1)/sum2;
    if (sum1<0)
        sum1 = -sum1;
    return sum1/sum2;  // here the type is changed
}



// container wrapper functions


//template <typename T>
////template <typename T::value_type)>
//T::value_type get_data(T t)
//{
//    return *t;
//}

// TODO if needed



} // namespace distance
} // namespace metric
