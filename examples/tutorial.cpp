#include "../metric.hpp"

void distance_example()
{
    using namespace metric;

    // configure the metric
    Euclidean<double> euclidean_distance;
    // new: Euclidean<std::vector<double>> euclidean_distance;

    //apply the metric
    std::vector<double> a = { 1, 2, 5, 3, 1 };
    std::vector<double> b = { 2, 2, 1, 3, 2 };
    double result = euclidean_distance(a, b);
    std::cout << "result = " << result << std::endl;
}

void correlation_example()
{
    using namespace metric;
    // some data
    std::vector<std::vector<int>> A = {
        { 0, 1, 1, 1, 1, 1, 2, 3 },
        { 1, 1, 1, 1, 1, 2, 3, 4 },
        { 2, 2, 2, 1, 1, 2, 0, 0 },
        { 3, 3, 2, 2, 1, 1, 0, 0 },
        { 4, 3, 2, 1, 0, 0, 0, 0 },
        { 5, 3, 2, 1, 0, 0, 0, 0 },
        { 4, 6, 2, 2, 1, 1, 0, 0 },
    };

    // some other data
    std::deque<std::string> B = {
        "this",
        "test",
        "tests",
        "correlation",
        "of",
        "arbitrary",
        "data",
    };

    // configure the correlation
    auto mgc_corr = MGC<std::vector<int>, Euclidean<int>, std::string, Edit<std::string>>();
    // new auto mgc_corr = MGC<Euclidean<std::vector<int>>, Edit<std::string>>();

    // apply the correlation
    auto result = mgc_corr(A, B);

    std::cout << "Multiscale graph correlation: " << result << std::endl;
    // 0.0791671 (Time = 7.8e-05s)
    // Rows 2 and 3 are similar in both data sets, so that there is a minimal correlation.
}

void knn_example()
{
using namespace metric;
using rec_t = std::vector<double>;

    // some data
    std::vector<rec_t> A = {
        { 0, 1, 1, 1, 1, 1, 2, 3 },
        { 1, 1, 1, 1, 1, 2, 3, 4 },
        { 2, 2, 2, 1, 1, 2, 0, 0 },
        { 3, 3, 2, 2, 1, 1, 0, 0 },
        { 4, 3, 2, 1, 0, 0, 0, 0 },
        { 5, 3, 2, 1, 0, 0, 0, 0 },
        { 4, 6, 2, 2, 1, 1, 0, 0 },
    };

    // std::vector<double> v0 = {0, 1, 1, 1, 1, 1, 2, 3};
    // std::vector<double> v1 = {1, 1, 1, 1, 1, 2, 3, 4};
    // std::vector<double> v2 = {2, 2, 2, 1, 1, 2, 0, 0};
    // std::vector<double> v3 = {3, 3, 2, 2, 1, 1, 0, 0};
    rec_t b = { 2, 8, 2, 1, 0, 0, 0, 0 };

    // configure the tree
    Tree<rec_t, Manhatten<double>> searchTree(A);

    //apply a method of the seach tree
    auto nn = searchTree.nn(b);
    std::cout << "Tree: best match for b is A[" << nn->ID << "]" << std::endl;


    // configure the graph
    KNNGraph<rec_t, Manhatten<double>> searchGraph(A, 2, 4);

    auto nn_ID = searchGraph.nn(b);
    std::cout << "Graph: best match for b is A[" << nn_ID << "]" << std::endl;


    // configure the search tree
    Matrix<rec_t, Manhatten<double>> searchMatrix(A);
    std::cout << "Matrix: best match for b is A[" << searchMatrix.nn(b) << "]" << std::endl;


    std::cout << searchMatrix(0,3) << std::endl;
    std::cout << searchGraph(0,3) << std::endl;
    std::cout << searchTree(0,3) << std::endl;


}

int main()
{

    distance_example();
    correlation_example();
    knn_example();

    return 0;
}