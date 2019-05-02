#ifndef DIMENSION_HPP
#define DIMENSION_HPP

#include <vector>
#include <variant> // for DimensionSet
#include <type_traits>
#include <tuple>
#include <functional>

//#include "metric_distance.hpp"



namespace metric
{


    template <
        typename Record,
        typename MetricType,
        typename Accessor
        >
    class Dimension
    {
    public:
        using InputValueType = typename MetricType::value_type;
        using ReturnValueType = typename MetricType::distance_type;
        //using RetType = vector_view<InputValueType> ;
        MetricType DistanceFunctor; // = MetricType()
        

        // template<typename RetType, typename RecordType, typename M>
        // Dimension(std::vector<std::function<RetType(RecordType)>> accessors_,  const M & m_):DistanceFunctor(m_) // vector of accessors as input
        // {
        //     accessors = accessors_;
        // }

        template<typename A, typename M>
        Dimension(A accessor_, const M & m): DistanceFunctor(m),
                                             accessor(accessor_) {}

        ReturnValueType get_distance(Record r1, Record r2)
        {
            return DistanceFunctor(accessor(r1), accessor(r2));
        }

    private:
        //std::function<RetType(const Record&)> accessor;
        Accessor accessor;
    };

    template<typename T>
    struct function_traits;

    template<typename R, typename ...Args> 
    struct function_traits<std::function<R(Args...)>>
    {
        static const size_t nargs = sizeof...(Args);

        typedef R result_type;

        template <size_t i>
        struct arg
        {
            typedef typename std::tuple_element<i, std::tuple<Args...>>::type type1;
            typedef typename std::decay<type1>::type type;
        };
    };

    template<typename F, typename Ret, typename A, typename... Rest>
    A helper(Ret (F::*)(A, Rest...));
    template<typename F, typename Ret, typename A, typename... Rest>
    A helper(Ret (F::*)(A, Rest...) const);
    template<typename F>
    struct first_argument {
        typedef decltype( helper(&F::operator()) ) type1;
        using type = typename std::decay<type1>::type;
    };

    template<typename Accessor, typename Metric>
    inline auto make_dimension(Metric && m, Accessor a) -> Dimension<typename first_argument<Accessor>::type,Metric,Accessor> {
        return Dimension<typename first_argument<Accessor>::type,Metric,Accessor>(a,m);
    }

    // template<typename Accessor, typename Metric>
    // Dimension(Accessor a, const Metric & m) ->
    //     Dimension<typename function_traits<Accessor>::template arg<0>::type,Metric,Accessor>;
    // template<typename R, typename Rt, typename M>
    // Dimension(std::vector<std::function<R(Rt)>> accessors_,  const M & m_) -> Dimension<Rt,M>;
// // code for DimensionSet

//template <typename Record, typename Container, typename DistType>
//class DimensionSet
//{

//private:

//    typedef typename Container::value_type InputValueType; // elementary values inside field we create

//    typedef  std::variant<
//      metric::Dimension<Record, std::vector<InputValueType>, metric::distance::Euclidian>,
//      metric::Dimension<Record, std::vector<InputValueType>, metric::distance::Manhatten>,
//      metric::Dimension<Record, std::vector<InputValueType>, metric::distance::P_norm>,
//      metric::Dimension<Record, std::vector<InputValueType>, metric::distance::Euclidian_thresholded>,
//      metric::Dimension<Record, std::vector<InputValueType>, metric::distance::Cosine>,
//      //metric::Dimension<Record, std::vector<InputValueType>, metric::distance::SSIM>,
//      //metric::Dimension<Record, std::vector<InputValueType>, metric::distance::TWED>,
//      //metric::Dimension<Record, std::vector<InputValueType>, metric::distance::EMD>, // TODO determine why return type deduction fails
//      metric::Dimension<Record, std::vector<InputValueType>, metric::distance::Edit>
//    > VariantType;

//    std::vector<VariantType> variant_dims;

//public:

//    typedef Container FieldType;
//    typedef DistType DistanceType;

//    template <template <typename> class Metric>
//    void add(Dimension<Record, Container, Metric> dim) // add Dimension object to the set
//    {
//        variant_dims.push_back(dim);
//    }

//    size_t size()
//    {
//        return variant_dims.size();
//    }

//    Container get(Record r, size_t dim_idx) // get dim_idx field
//    {
//        Container field;
//        if (dim_idx >= variant_dims.size() || dim_idx < 0)
//            return field; // of rise exception?
//        auto d_vis = [&r](auto & d) { return d(r); };
//        field = std::visit(d_vis, variant_dims[dim_idx]); // call Dimension functor for current Record
//        return field;
//    }

//    DistanceType get_distance(Record r1, Record r2, size_t dim_idx)
//    {
//        Container field1 = get(r1, dim_idx);
//        Container field2 = get(r2, dim_idx);
//        auto d_dist_vis = [field1, field2](auto & d)
//        {
//            return (DistanceType)d.DistanceFunctor(field1, field2);
//        };
//        // TODO add static_assert
//        DistanceType distance = std::visit(d_dist_vis, variant_dims[dim_idx]);
//        return distance;
//    }
//};





} // namespace metric


#endif // DIMENSION_HPP
