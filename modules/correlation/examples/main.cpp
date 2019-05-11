#include <vector>
#include <deque>
#include <array>
#include <iostream>
#include <chrono>

#include "metric_correlation.hpp"

template <typename T>
void matrix_print(const std::vector<std::vector<T>> &mat)
{

    std::cout << "[";
    for (int i = 0; i < mat.size(); i++)
    {
        for (int j = 0; j < mat[i].size() - 1; j++)
        {
            std::cout << mat[i][j] << ", ";
        }
        std::cout << mat[i][mat[i].size() - 1] << " ]" << std::endl;
        ;
    }
}

template <typename T>
void vector_print(const std::vector<T> &vec)
{

    std::cout << "[";
    for (int i = 0; i < vec.size(); i++)
    {
        std::cout << vec[i] << ", ";
    }
    std::cout << vec[vec.size() - 1] << " ]" << std::endl;
}


struct simple_user_euclidian
{

    double operator()(const std::vector<double> &a, const std::vector<double> &b) const
    {
        double sum = 0;
        for (size_t i= 0; i<a.size(); ++i)
        {
            sum += (a[i]-b[i]) * (a[i]-b[i]);
        }
        return std::sqrt(sum);
    }
};

int main()
{

    // some data
    std::vector<std::vector<double>> A = {{-0.991021875880222},
                                          {-0.768275252129114},
                                          {-0.526359355330172},
                                          {-0.318170691552235},
                                          {-0.0468979315641954},
                                          {0.227289495956795},
                                          {0.317841938040349},
                                          {0.463820792484335},
                                          {0.660623198153296},
                                          {0.812210713528849}};

    std::vector<std::vector<double>> B = {{2.36088477872717},
                                          {1.23271042331569},
                                          {0.219758852423591},
                                          {0.0129766138306992},
                                          {0.00923506810444738},
                                          {1.49393468371558e-07},
                                          {0.00619896971968280},
                                          {0.212322021636953},
                                          {0.257245700714104},
                                          {1.59223791395715}};

    std::vector<std::vector<double>> A1 = {{-1.08661677587398},
                                           {-1.00699896410939},
                                           {-0.814135753976830},
                                           {-0.875364720432552},
                                           {-0.659607023272462},
                                           {-0.798949992922930},
                                           {-0.431585448024267},
                                           {-0.619123703544758},
                                           {-0.351492263653510},
                                           {-0.394814371972061},
                                           {-0.309693618374598},
                                           {-0.352009525808777},
                                           {-0.0803413535982411},
                                           {0.0103940699342647},
                                           {-0.130735385695596},
                                           {-0.138214899507693},
                                           {0.0279270082022143},
                                           {0.141670765995995},
                                           {0.112221224566625},
                                           {0.376767573021755},
                                           {0.186729429735154},
                                           {0.597349318463320},
                                           {0.451380104139401},
                                           {0.639237742050564},
                                           {0.797420868050314},
                                           {0.690091614630087},
                                           {0.921722674141222},
                                           {0.852593762434809},
                                           {0.954771723842945},
                                           {1.03297970279357}};

    std::deque<std::array<float, 1>> B1 = {{2.70625143351230},
                                           {1.41259513494005},
                                           {0.666086793692617},
                                           {0.647856446084279},
                                           {0.887764969338737},
                                           {0.286220905202707},
                                           {0.543682026943014},
                                           {0.0402339224257120},
                                           {0.105812168910424},
                                           {0.0230915137205610},
                                           {0.00298976085950325},
                                           {0.00366997150982423},
                                           {0.000384825484363474},
                                           {7.27293780465119e-05},
                                           {2.50809340229209e-07},
                                           {0.00306636655437742},
                                           {0.000456283181338950},
                                           {0.00801756105329616},
                                           {1.17238339150888e-09},
                                           {0.0803830108071682},
                                           {0.0774478107095828},
                                           {0.0474847202878941},
                                           {0.0818772460512609},
                                           {0.486406609209630},
                                           {0.197547677770060},
                                           {0.628321368933714},
                                           {1.02400551043736},
                                           {0.552591658802459},
                                           {1.52144482984914},
                                           {3.43908991254968}};
    
    // build functors (function objects) with user types and metrics
    typedef std::vector<double> Rec1;
    typedef std::array<float, 1> Rec2;
    typedef simple_user_euclidian Met1;
    typedef metric::correlation::mgc_details::generic_manhatten<Rec2> Met2;

    // set up the correlation function
    auto mgc_corr = metric::correlation::MGC<Rec1, Met1, Rec2, Met2>();

    // compute and benchmark
    auto t1 = std::chrono::steady_clock::now();
    auto result = mgc_corr(A1, B1); // A1 = std::vector<...>, A2 = std::deque<...>
    auto t2 = std::chrono::steady_clock::now();
    std::cout << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;

    return 0;
}