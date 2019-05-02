#ifndef _METRIC_DISTANCE_EDIT_HPP
#define _METRIC_DISTANCE_EDIT_HPP

namespace metric
{

    namespace distance
    {
/*** Edit distance (for strings) ***/
        template <typename V>
        struct Edit
        {
            using value_type = V;
            using distance_type = int;

            template<typename Container>
            distance_type operator()(const Container &str1, const Container &str2) const;

            
            distance_type operator()(const V* str1, const V* str2) const {
                return this->operator()(std::basic_string_view<V>(str1), std::basic_string_view<V>(str2));
            }

        };

    } // namespace distance

} // namespace metric

#include "Edit.cpp"

#endif // Header Guard
