#include <iostream>

#include <vector>
#include <deque>
#include <functional>

#include <variant>



#include "../details/classification/details/metric_dt_classifier.hpp"

#include "assets/helpers.cpp" // csv reader
#include "../../distance/metric_distance.cpp"


using namespace std;

int main()
{




    typedef std::variant<double, std::vector<double>, std::vector<std::vector<double>>, std::string> V; // field type
    typedef std::vector<V> Record;

    std::vector<std::vector<double>> img1 = { // needs to be larger than blur kernel size coded intarnally as 11
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    std::vector<std::vector<double>> img2 = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    std::vector<Record> selection = {
        {V((double)2),
         V(std::vector<double>({1, 2})),
         V(std::vector<double>({0, 1, 1, 1, 1, 1, 2, 3})),
         V(img1),
         V(""),
         V((double)1)
        },
        {V((double)2),
         V(std::vector<double>({1, 5})),
         V(std::vector<double>({1, 1, 1, 1, 1, 2, 3, 4})),
         V(img2),
         V("A"),
         V((double)1)
        },
        {V((double)1),
         V(std::vector<double>({4, 5})),
         V(std::vector<double>({2, 2, 2, 1, 1, 2, 0, 0})),
         V(img2),
         V("AA"),
         V((double)2)
        },
        {V((double)2),
         V(std::vector<double>({1, 2})),
         V(std::vector<double>({3, 3, 2, 2, 1, 1, 0, 0})),
         V(img1),
         V("AAA"),
         V((double)1)
        },
        {V((double)2),
         V(std::vector<double>({5})),
         V(std::vector<double>({4, 3, 2, 1, 0, 0, 0, 0})),
         V(img1),
         V("AAAA"),
         V((double)1)
        },
        {V((double)2),
         V(std::vector<double>({1, 4, 5})),
         V(std::vector<double>({4, 3, 2, 1, 0, 0, 0, 0})),
         V(img2),
         V("BAAA"),
         V((double)1)
        },
        {V((double)1),
         V(std::vector<double>({1, 2, 3, 4})),
         V(std::vector<double>({5, 3, 2, 1, 0, 0, 0, 0})),
         V(img2),
         V("BBAA"),
         V((double)3)
        },
        {V((double)1),
         V(std::vector<double>({1})),
         V(std::vector<double>({4, 6, 2, 2, 1, 1, 0, 0})),
         V(img1),
         V("BBA"),
         V((double)1)
        },
        {V((double)2),
         V(std::vector<double>({4, 5})),
         V(std::vector<double>({3, 7, 2, 1, 0, 0, 0, 0})),
         V(img2),
         V("BB"),
         V((double)1)
        },
        {V((double)2),
         V(std::vector<double>({1, 2, 4, 5})),
         V(std::vector<double>({2, 5, 1, 1, 0, 0, 1, 2})),
         V(img1),
         V("B"),
         V((double)1)
        }
    };

    typedef double InternalType;
        //typedef double DistanceType;

    //typedef std::vector<double> ContainerType;


    // vector of accessors for field 0
    // std::function<InternalType(Record)> field0accessor0 = [](Record r)
    // {
    //     return (InternalType)std::get<double>(r[0]);
    // };

//    std::vector<std::function<InternalType(Record)>> field0accessors = {field0accessor0};
    auto field0accessors = [] (const Record & r) {
                               return std::get<double>(r[0]);
                           };
    std::function<double(const Record &)> field01accessors = [] (const Record & r) {
                                                return std::get<double>(r[0]);
                                            };

    // vector of accessors for field 1
    // std::vector<std::function<InternalType(Record)> > field1accessors;
    // for (size_t i=0; i<5; i++)
    // {
    //     field1accessors.push_back(
    //         [=](Record r)
    //         {
    //             auto v = std::get<std::vector<double>>(r[1]);
    //             if (v.size() > i)
    //                 return (InternalType)v[i];
    //             else
    //                 return (InternalType)0; // 0 if no data
    //         }
    //     );
    // }
    auto field1accessors = [] (const Record & r) {
                               std::vector<double> v(std::get<std::vector<double>>(r[1]));
                               v.resize(4);
                               return v;
                           };


    // vector of accessors for field 2
    // std::vector<std::function<InternalType(Record)> > field2accessors;
    // for (size_t i=0; i<8; i++)
    // {
    //     field2accessors.push_back(
    //         [=](Record r)
    //         {
    //             auto v = std::get<std::vector<double>>(r[2]);
    //             return (InternalType)v[i];
    //         }
    //     );
    // }
    auto field2accessors = [] (const Record & r) {
                               std::vector<double> v(std::get<std::vector<double>>(r[2]));
                               v.resize(8);
                               return v;
                           };

    // vector of accessors for field 3
    // std::vector<std::function<std::vector<InternalType>(Record)> > field3accessors;
    // for (size_t i=0; i<23; i++) //
    // {
    //     field3accessors.push_back(
    //         [=](Record r)
    //         {
    //             auto v = std::get<std::vector<std::vector<double>>>(r[3]);
    //             std::vector<InternalType> result = {};
    //             for (size_t j=0; j<v[i].size(); j++)
    //             {
    //                 result.push_back((InternalType)v[i][j]);
    //             }
    //             return result;
    //         }
    //     );
    // }
    auto field3accessors = [] (const Record & r) {
                               return std::get<std::vector<std::vector<double>>>(r[3]);
                           };

    // vector of accessors for field 4
    // std::vector<std::function<char(Record)> > field4accessors;
    // // determine string size??
    // for (size_t i=0; i<8; i++) // TODO refactor in order to remove limit and get able to represent field of arbitrary length!
    // {
    //     field4accessors.push_back(
    //         [=](Record r)
    //         {
    //             auto v = std::get<std::string>(r[4]);
    //             if (v.size() < i)
    //                 return (char)v[i];
    //             else
    //                 return (char)0;
    //         }
    //     );
    // }

    auto field4accessors = [] (const Record & r) {
                               return std::get<std::string>(r[4]);
                           };
    // label accessor (for single record)
    std::function<int(Record)> response = [](const Record & a)
    {
        return (int)std::abs(std::get<double>(a[5]));
    };




    // build dimension and Dimension objects


    // metric::Dimension<Record, metric::distance::Euclidian<InternalType>, std::function<> dim0(field0accessors);
    // metric::Dimension<Record, metric::distance::Manhatten<std::vector<InternalType>>> dim1(field1accessors);
    // metric::Dimension<Record, metric::distance::P_norm<std::vector<InternalType>>> dim2(field2accessors);
    // metric::Dimension<Record, metric::distance::Euclidian_thresholded<std::vector<InternalType>>> dim3(field2accessors);
    // metric::Dimension<Record, metric::distance::Cosine<std::vector<InternalType>>> dim4(field2accessors);
    // metric::Dimension<Record, metric::distance::SSIM<std::vector<std::vector<InternalType>>>> dim5(field3accessors);
    // metric::Dimension<Record, metric::distance::TWED<std::vector<InternalType>>> dim6(field2accessors);
    // metric::Dimension<Record, metric::distance::Edit<std::string>> dim7(field4accessors);
    // metric::Dimension<Record, metric::distance::EMD<std::vector<InternalType>>> dim10(field2accessors);
    namespace md = metric::distance;

    using a0_type = decltype(field0accessors);
    using a1_type = decltype(field1accessors);
    using a2_type = decltype(field2accessors);
    using a3_type = decltype(field3accessors);
    using a4_type = decltype(field4accessors);
    auto dim0 = metric::make_dimension(md::Euclidian<InternalType>(), field0accessors);
    auto dim1 = metric::make_dimension(md::Manhatten<InternalType>(), field1accessors);
    auto dim2 = metric::make_dimension(md::P_norm<InternalType>(), field2accessors);
    auto dim3 = metric::make_dimension(md::Euclidian_thresholded<InternalType>(), field2accessors);
    auto dim4 = metric::make_dimension( md::Cosine<InternalType>(), field2accessors);
    auto dim5 = metric::make_dimension(md::SSIM<std::vector<InternalType>>(), field3accessors);
    auto dim6 = metric::make_dimension(md::TWED<InternalType>(), field2accessors);
    auto dim7 = metric::make_dimension(md::Edit<char>(), field4accessors);
    auto dim10 = metric::make_dimension(md::EMD<InternalType>(8,8), field2accessors);
// ---
//     metric::Dimension<Record, md::Euclidian<InternalType>, a0_type> dim0(field0accessors, md::Euclidian<InternalType>());

//     metric::Dimension<Record, md::Manhatten<InternalType>, a1_type> dim1(field1accessors, md::Manhatten<InternalType>());
//     metric::Dimension<Record, md::P_norm<InternalType>, a2_type> dim2(field2accessors, md::P_norm<InternalType>());
//     metric::Dimension<Record, md::Euclidian_thresholded<InternalType>, a2_type> dim3(field2accessors, md::Euclidian_thresholded<InternalType>());
//     metric::Dimension<Record, md::Cosine<InternalType>, a2_type> dim4(field2accessors, md::Cosine<InternalType>());
//     metric::Dimension<Record, md::SSIM<std::vector<InternalType>>,a3_type> dim5(field3accessors, md::SSIM<std::vector<InternalType>>());
//     metric::Dimension<Record, md::TWED<InternalType>, a2_type> dim6(field2accessors, md::TWED<InternalType>());
//     metric::Dimension<Record, md::Edit<char>, a4_type> dim7(field4accessors, md::Edit<char>());
//     metric::Dimension<Record, md::EMD<InternalType>, a2_type> dim10(field2accessors, md::EMD<InternalType>(8,8));

    // metric::Dimension<Record, metric::distance::P_norm<std::vector<InternalType>>> dim2(field2accessors);
    // metric::Dimension<Record, metric::distance::Euclidian_thresholded<std::vector<InternalType>>> dim3(field2accessors);
    // metric::Dimension<Record, metric::distance::Cosine<std::vector<InternalType>>> dim4(field2accessors);
    // metric::Dimension<Record, metric::distance::SSIM<std::vector<std::vector<InternalType>>>> dim5(field3accessors);
    // metric::Dimension<Record, metric::distance::TWED<std::vector<InternalType>>> dim6(field2accessors);
    // metric::Dimension<Record, metric::distance::Edit<std::string>> dim7(field4accessors);
    //metric::Dimension<Record, metric::distance::EMD<std::vector<InternalType>>> dim10(field2accessors);

    typedef  std::variant<
        metric::Dimension<Record, metric::distance::Euclidian<InternalType>, a0_type>,
        metric::Dimension<Record, metric::distance::Manhatten<InternalType>, a1_type>,
        metric::Dimension<Record, metric::distance::P_norm<InternalType>, a2_type>,
        metric::Dimension<Record, metric::distance::Euclidian_thresholded<InternalType>, a2_type>,
        metric::Dimension<Record, metric::distance::Cosine<InternalType>, a2_type>,
        metric::Dimension<Record, metric::distance::SSIM<std::vector<InternalType>> ,a3_type>,
        metric::Dimension<Record, metric::distance::TWED<InternalType>, a2_type>,
        metric::Dimension<Record, metric::distance::EMD<InternalType>, a2_type>, // matrix C is temporary created inside functor
        metric::Dimension<Record, metric::distance::Edit<std::string::value_type>, a4_type>
        > VariantType;

    // typedef  std::variant<
    //   metric::Dimension<Record, metric::distance::Euclidian<std::vector<InternalType>>>,
    //   metric::Dimension<Record, metric::distance::Manhatten<std::vector<InternalType>>>,
    //   metric::Dimension<Record, metric::distance::P_norm<std::vector<InternalType>>>,
    //   metric::Dimension<Record, metric::distance::Euclidian_thresholded<std::vector<InternalType>>>,
    //   metric::Dimension<Record, metric::distance::Cosine<std::vector<InternalType>>>,
    //   metric::Dimension<Record, metric::distance::SSIM<std::vector<std::vector<InternalType>>>>,
    //   metric::Dimension<Record, metric::distance::TWED<std::vector<InternalType>>>,
    //   metric::Dimension<Record, metric::distance::EMD<std::vector<InternalType>>>, // matrix C is temporary created inside functor
    //   metric::Dimension<Record, metric::distance::Edit<std::string>>
    // > VariantType;

    std::vector<VariantType> dims = {dim0, dim1, dim2, dim3, dim4, dim5, dim6, dim7, dim10};

//    auto t = dim0.get_distance(selection[0], selection[1]);

    auto dl2 = metric::classification::MetricDT<Record>();

    dl2.train(selection, dims, response);

    std::vector<int> pr2;

    dl2.predict(selection, dims, pr2);




    // test Edit separately

    metric::distance::Edit<char> edit_functor;
    auto edit_dist = edit_functor("AAAB", "AAC");

    std::cout << "\nEdit distance: " << edit_dist << "\n";



    // test SSIM separately

    metric::distance::SSIM<std::vector<double>> SSIM_functor;
    auto SSIM_dist = SSIM_functor(img1, img2);

    std::cout << "\nSSIM distance: " << SSIM_dist << "\n";




    // test EMD separately

 //    typedef int InputType;
     typedef float InputType;

    typedef std::variant<double, std::vector<int>, int> F; // field type
    typedef std::vector<F> R;
    std::vector<R> dataset = {
        {F((double)2), // not used
         F(std::vector<int>({1, 0, 0})),
         F((int)0) // label
        },
        {F((double)2),
         F(std::vector<int>({0, 2, 0})), // from example
         //F(std::vector<int>({0, 1, 0})),
         F((int)1)
        }
    };

    // vector of accessors for field 1
    // std::vector<std::function<InputType(R)>> a1;
    // for (size_t i=0; i<3; i++)
    // {
    //     a1.push_back(
    //         [=](R r)
    //         {
    //             auto v = std::get<std::vector<int>>(r[1]);
    //             return (InputType)v[i];
    //         }
    //     );
    // }
    auto a1 = [](const R & r) {
                  auto & v = std::get<std::vector<int>>(r[1]);
                  return std::vector<InputType>(v.begin(),v.end());
              };

    // label accessor (for single record)
    std::function<int(R)> resp = [](R a)
    {
        return (int)std::abs(std::get<int>(a[2]));
    };


    std::vector<std::vector<InputType>> C = {{0, 10, 20}, {10, 0, 10}, {20, 10, 0}}; // symmetric



    metric::Dimension<R, metric::distance::EMD<InputType>, decltype(a1)> dimEMD(a1, md::EMD<InputType>(3,3));
    auto emd_dist = dimEMD.get_distance(dataset[0], dataset[1]);

    metric::distance::EMD<InputType> EMD_functor(C);

    auto v1i = std::get<std::vector<int>>(dataset[0][1]);
    auto v2i = std::get<std::vector<int>>(dataset[1][1]);
    std::vector<InputType> v1t(v1i.begin(), v1i.end());
    std::vector<InputType> v2t(v2i.begin(), v2i.end());

    auto emd_dist_C = EMD_functor(v1t, v2t);

    std::cout << "\nEMD distance 1: " << emd_dist << " " << emd_dist_C << "\n";





//    //    test EMD separately



    //std::vector<InputType> v1 = {1, 0, 0};
    //std::vector<InputType> v2 = {0, 2, 0}; // from example

    std::vector<InputType> v1 = {0, 2, 0, 0, 0, 0};
    std::vector<InputType> v2 = {2, 0, 1, 0, 0, 0};
    std::vector<std::vector<InputType>> CC = {{0, 10, 20, 30, 40, 50},
                                             {10, 0, 10, 20, 30, 40},
                                             {20, 10, 0, 10, 20, 30},
                                             {30, 20, 10, 0, 10, 20},
                                             {40, 30, 20, 10, 0, 10},
                                             {50, 40, 30, 20, 10, 0}}; // symmetric
    auto emd_dist_2 = metric::distance::EMD<InputType>(CC)(v1, v2);

    auto emd_dist_3 = metric::distance::EMD<InputType>(6,6)(v1, v2);

    std::cout << "\nEMD distance 2 : " << emd_dist_2 << " | " << emd_dist_3 << "\n";








    std::cout << "\nmain() code executed\n";

    return 0;


}

