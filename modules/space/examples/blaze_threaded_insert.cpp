# include <iostream>
# include <vector>
# include <random>
# include <thread>
# include <blaze/Math.h>
# include "assets/assets.cpp"
# include "../metric_space.hpp"

/*** define custom metric ***/
template <typename T>
struct recMetric_Blaze
{
    T operator()(const blaze::CompressedVector<T> &p, const blaze::CompressedVector<T> &q) const
    {
        return assets::sed(assets::zeroPad(p), assets::zeroPad(q), T(0), T(1));
    }
};

/*** simulation helper functions ***/
template <class recType, class Metric>
void insert_random(metric_space::Tree<recType, Metric> &cTree,int samples, int dimension)
{
    // random generator
    std::random_device rnd_device;
    std::mt19937 mersenne_engine(rnd_device());
    std::uniform_real_distribution<double> dist(-1, 1);
    auto gen = std::bind(dist, mersenne_engine);

    for (int i = 0; i < samples; ++i)
    {
        std::vector<double> vec = assets::linspace(gen(), gen(), dimension);  // gererator random lines.
        blaze::CompressedVector<double> comp_vec(dimension);          
        comp_vec = assets::smoothDenoise(vec, 0.1); // denoise and sparse the data
        cTree.insert(comp_vec);
    }
    return;
}

/*** fill a tree with 1 Mio records and search for nearest neighbour **/
int main(){

metric_space::Tree<blaze::CompressedVector<double>, recMetric_Blaze<double>> cTree;
    
int n_records = 25000;
int rec_dim = 100;
int threads = 4;

/*** parallel insert ***/
std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now(); // start inserting
std::vector<std::thread> inserter;
for (int i=0;i<threads;++i){
    inserter.push_back(std::thread(insert_random<blaze::CompressedVector<double>, recMetric_Blaze<double>>, std::ref(cTree), n_records, rec_dim));
}
for (int i=0;i<threads;++i){
    inserter[i].join();
}
std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now(); // end inserting
    
/*** search for a similar data record ***/
std::vector<double> vec1 = assets::linspace(0.3, -0.3, rec_dim);
auto comp_vec = assets::smoothDenoise(vec1, 0.1);
auto nn = cTree.nn(comp_vec); // nearest neigbour
(void)nn;
std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now(); // end searching
auto insert_time = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
auto nn_time = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
   
std::cout << "tree of " << (n_records*threads) << " curves build in " << insert_time / 1000.0 / 1000.0 << " seconds, " << "nn find in " << nn_time / 1000.0 / 1000.0 << " seconds, " << std::endl;
    
return 0;
}
