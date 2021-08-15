
#include "modules/mapping/SOM.hpp"
#include "modules/mapping/kmeans.hpp"

#include "assets/json.hpp"

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>


//*

// ----- csv reader


template <typename T> T convert_to(const std::string & str)
{
    std::istringstream s(str);
    T num;
    s >> num;
    return num;
} // templated version of stof, stod, etc., thanks to https://gist.github.com/mark-d-holmberg/862733


template <class ValueType>
std::vector<std::vector<ValueType>> read_csv_num(std::string filename, std::string sep=",")
{
    typedef typename std::vector<ValueType> LINE;
    std::string line;
    int pos;
    std::vector<std::vector<ValueType>> array = {};
    std::ifstream in(filename);
    if(!in.is_open()) {
        std::cout << "Failed to open file" << std::endl;
        return array;
    }
    while( getline(in,line) ) {
        LINE ln;
        while( (pos = line.find(sep)) >= 0) {
            std::string field = line.substr(0, pos);
            line = line.substr(pos+1);
            ln.push_back(convert_to<ValueType>(field));
        }
        ln.push_back(convert_to<ValueType>(line));
        array.push_back(ln);
    }
    return array;
}


template <class ContainerType>
void vv_to_csv(ContainerType data, std::string filename, std::string sep=",")  // container
{
    std::ofstream outputFile;
    outputFile.open(filename);
        for (auto i = 0; i < data.size(); ++i) {
            for (auto j = 0; j < data[i].size(); j++) {
                outputFile << std::to_string(data[i][j]);
                if (j < data[i].size() - 1)
                    outputFile << sep;
            }
            outputFile << std::endl;
        }
        outputFile.close();
}


// ------ helper functions (copied from energies_example.cpp)

namespace ooc_functions
{


template <typename T>
std::vector<std::vector<T>>
transpose(std::vector<std::vector<T>> &a)
{

  size_t rows = a.size();
  size_t cols = a[0].size();

  std::vector<std::vector<T>> array(cols, std::vector<T>(rows));
  for (size_t i = 0; i < cols; ++i)
  {
    for (size_t j = 0; j < rows; ++j)
    {
      array[i][j] = a[j][i];
    }
  }
  return array;
}


////



// linspace (erzeugt einen linearen Datenvektor)
template <typename T>
std::vector<T>
linspace(T a, T b, int n)
{
  std::vector<T> array;
  if (n > 1)
  {
    T step = (b - a) / T(n - 1);
    int count = 0;
    while (count < n)
    {
      array.push_back(a + count * step);
      ++count;
    }
  }
  else
  {
    array.push_back(b);
  }
  return array;
}

template <typename T>
T Lerp(T v0, T v1, T t)
{
  return (1 - t) * v0 + t * v1;
}

template <typename T>
T quickQuantil(std::vector<T> data, T probs)
{

  if (!(data.size() > 0))
    return 0;

  if (1 == data.size())
    return data[0];

  T poi = Lerp(T(-0.5), data.size() - T(0.5), probs);

  int left = std::max(int(std::floor(poi)), int(0));
  int right = std::min(int(std::ceil(poi)), int(data.size() - 1));

  if (probs <= T(0.5))
    std::nth_element(data.begin(), data.begin() + left, data.end());
  else
    std::nth_element(data.begin(), data.begin() + right, data.end());

  T datLeft = data[left];
  T datRight = data[right];

  T quantile = Lerp(datLeft, datRight, poi - T(left));

  return quantile;
}

// akima interpolation
//
// Ref. : Hiroshi Akima, Journal of the ACM, Vol. 17, No. 4, October 1970,
//      pages 589-602.
//
template <typename T>
std::vector<T>
akimaInterp1(std::vector<T> const &x, std::vector<T> const &y, std::vector<T> const &xi, bool save_Mode = true)
{
  // check inputs

  //calculate u vector
  auto uVec = [](std::vector<T> const &x, std::vector<T> const &y) {
    size_t n = x.size();
    std::vector<T> u((n + 3));
    for (size_t i = 1; i < n; ++i)
    {
      u[i + 1] = (y[i] - y[i - 1]) / (x[i] - x[i - 1]); // Shift i to i+2
    }

    auto akima_end = [](const T &u1, const T &u2) {
      return 2.0 * u1 - u2;
    };

    u[1] = akima_end(u[2], u[3]);
    u[0] = akima_end(u[1], u[2]);
    u[n + 1] = akima_end(u[n], u[n - 1]);
    u[n + 2] = akima_end(u[n + 1], u[n]);

    return u;
  };
  std::vector<T> u = uVec(x, y);

  // calculate yp vector
  std::vector<T> yp(x.size());
  for (size_t i = 0; i < x.size(); ++i)
  {
    auto a = std::abs(u[i + 3] - u[i + 2]);
    auto b = std::abs(u[i + 1] - u[i]);
    if ((a + b) != 0)
    {
      yp[i] = (a * u[i + 1] + b * u[i + 2]) / (a + b);
    }
    else
    {
      yp[i] = (u[i + 2] + u[i + 1]) / 2.0;
    }
  }

  // calculte interpolated yi values
  auto kFind = [](const T &xii, const std::vector<T> &x, int start, int end) {

    int klo = start;
    int khi = end;
    // // Find subinterval by bisection
    while (khi - klo > 1)
    {
      int k = (khi + klo) / 2;
      x[k] > xii ? khi = k : klo = k;
    }
    return klo;
  };

  std::vector<T> yi(xi.size());
  for (size_t i = 0; i < xi.size(); ++i)
  {
    // Find the right place in the table by means of a bisection.
    int k = kFind(xi[i], x, int(0), x.size() - 1);

    // Evaluate Akima polynomial
    T b = x[k + 1] - x[k];
    T a = xi[i] - x[k];
    yi[i] = y[k] + yp[k] * a + (3.0 * u[k + 2] - 2.0 * yp[k] - yp[k + 1]) * a * a / b + (yp[k] + yp[k + 1] - 2.0 * u[k + 2]) * a * a * a / (b * b);

    // Differentiate to find the second-order interpolant
    //ypi[i] = yp[k] + (3.0*u[k+2] - 2.0*yp[k] - yp[k+1])*2*a/b + (yp[k] + yp[k+1] - 2.0*u[k+2])*3*a*a/(b*b);

    // Differentiate to find the first-order interpolant
    //yppi[i] = (3.0*u[k+2] - 2.0*yp[k] - yp[k+1])*2/b + (yp[k] + yp[k+1] - 2.0*u[k+2])*6*a/(b*b);
  }
  return yi;
}




}

////


template <typename T>
void vector_print(const std::vector<T> &vec)
{

    std::cout << "[ ";
    for (int i = 0; i < vec.size() - 1; i++)
    {
        std::cout << vec[i] << ", ";
    }
    std::cout << vec[vec.size() - 1] << " ]" << std::endl;
}



//template <typename T>
//void matrix_print(const std::vector<std::vector<T>> &mat)
//{

//    std::cout << "[";
//    std::cout << std::endl;
//    for (int i = 0; i < mat.size(); i++)
//    {
//        std::cout << "  [ ";
//        if (mat[i].size() > 0)
//        {
//            for (int j = 0; j < mat[i].size() - 1; j++)
//            {
//                std::cout << mat[i][j] << ", ";
//            }
//            std::cout << mat[i][mat[i].size() - 1];
//        }

//        std::cout << " ]" << std::endl;
//    }
//    std::cout << std::endl;
//    std::cout << "]" << std::endl;
//}




// ----------




// ------ confidence levels (copied from energies_example.cpp)


template <typename T>
std::vector<std::vector<T>>
set2conf(std::vector<T> set_0, size_t windowSize, size_t samples, T confidencelevel)
{

  std::random_device rd;  //seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<> dis(T(0), T(1));

  // propabiliy vector
  std::vector<T> prob_0 = ooc_functions::linspace(T(1) / T(set_0.size()), T(1) - T(1) / T(set_0.size()), set_0.size());
  std::sort(set_0.begin(), set_0.end());
  //ooc_functions::dualPivotSort(set_0);

  // compute probability matrix of set_0
  std::vector<std::vector<T>> m_0(samples, std::vector<T>(set_0.size()));
  //std::vector<std::vector<T>> m_prop_0(samples, std::vector<T>(set_0.size()));

  for (size_t i = 0; i < samples; ++i)
  {
    for (size_t j = 0; j < set_0.size(); ++j)
    {
      m_0[i][j] = T(dis(gen)); // fill with random numbers
    }

    std::sort(m_0[i].begin(), m_0[i].end()); // sort the row
    //ooc_functions::dualPivotSort(m_prop_0[i]);
    m_0[i] = ooc_functions::akimaInterp1(prob_0, set_0, m_0[i]); // interpolate the random numbers
  }

  // m_prop_0.clear();
  // m_prop_0.shrink_to_fit();

  // transpose
  auto m_0t = ooc_functions::transpose(m_0);

  m_0.clear();
  m_0.shrink_to_fit();

  // compute left and right confidence boundaries of set_0
  std::vector<T> set_0_left(set_0.size());
  std::vector<T> set_0_right(set_0.size());
  for (size_t i = 0; i < set_0.size(); ++i)
  {
    set_0_left[i] = ooc_functions::quickQuantil(m_0t[i], (T(1) - confidencelevel) / T(2));
    set_0_right[i] = ooc_functions::quickQuantil(m_0t[i], confidencelevel + (T(1) - confidencelevel) / T(2));
  }

    m_0t.clear();
  m_0t.shrink_to_fit();

  // compute probability matrix of left and right and medians of set_0
  std::vector<std::vector<T>> m_prop_1(samples, std::vector<T>(windowSize));


  for (size_t i = 0; i < samples; ++i)
  {
    for (size_t j = 0; j < windowSize; ++j)
    {
      m_prop_1[i][j] = T(dis(gen)); // fill with random numbers
    }

    std::sort(m_prop_1[i].begin(), m_prop_1[i].end()); // sort the row
  }

  std::vector<std::vector<T>> quants(3, std::vector<T>(windowSize));


  // left
  std::vector<std::vector<T>> m(samples, std::vector<T>(windowSize));
  for (size_t i = 0; i < samples; ++i)
  {
      m[i] = ooc_functions::akimaInterp1(prob_0, set_0_left, m_prop_1[i]); // interpolate the random numbers
  }

  // set_0_left.clear();
  // set_0_left.shrink_to_fit();

  auto mt = ooc_functions::transpose(m);

  for (size_t i = 0; i < windowSize; ++i)
  {
    quants[0][i] = ooc_functions::quickQuantil(mt[i], (T(1.0) - confidencelevel) / T(2.0));
  }

  //right
  for (size_t i = 0; i < samples; ++i)
  {
      m[i] = ooc_functions::akimaInterp1(prob_0, set_0_right, m_prop_1[i]);
  }

  // set_0_right.clear();
  // set_0_right.shrink_to_fit();

  mt = ooc_functions::transpose(m);

  for (size_t i = 0; i < windowSize; ++i)
  {
    quants[2][i] = ooc_functions::quickQuantil(mt[i], confidencelevel + (T(1.0) - confidencelevel) / T(2.0));
  }

  //median
  for (size_t i = 0; i < samples; ++i)
  {
      m[i] = ooc_functions::akimaInterp1(prob_0, set_0, m_prop_1[i]);
  }

  mt = ooc_functions::transpose(m);
  // m.clear();
  // m.shrink_to_fit();

  // m_prop_1.clear();
  // m_prop_1.shrink_to_fit();

  for (size_t i = 0; i < windowSize; ++i)
  {
    quants[1][i] = ooc_functions::quickQuantil(mt[i], T(0.5));
  }

  return quants;
}

template <typename T>
std::vector<std::vector<std::vector<T>>>
set2multiconf(std::vector<T> set_0, std::vector<uint32_t> windowSizes, size_t samples, T confidencelevel)
{
  std::vector<std::vector<std::vector<T>>> multiquants;
  for (size_t i = 0; i < windowSizes.size(); ++i)
  {
    multiquants.push_back(set2conf(set_0, windowSizes[i], samples, confidencelevel));
  }

  return multiquants;
}


// */


/*

// Euclidean for compressed vectors


template <typename T>
T sparse_euclidean(const blaze::CompressedVector<T> & a, const blaze::CompressedVector<T> & b) {

//    typename blaze::CompressedVector<T>::Iterator it_a = a.begin();
//    typename blaze::CompressedVector<T>::Iterator it_b = b.begin();

    T dist = 0;

//    // working ver
//    typename blaze::CompressedVector<T> subtr = b - a;
//    for (typename blaze::CompressedVector<T>::Iterator it = subtr.begin(); it < subtr.end(); ++it) {
//        dist += it->value()*it->value();
//    }

    // 2nd ver, fixed
    typename blaze::CompressedVector<T>::ConstIterator it_long;
    typename blaze::CompressedVector<T>::ConstIterator it_short;
    typename blaze::CompressedVector<T>::ConstIterator end_long;
    typename blaze::CompressedVector<T>::ConstIterator end_short;
    blaze::CompressedVector<T> long_operand, short_operand;
    if (a.nonZeros() >= b.nonZeros()) {
        it_long = a.cbegin();
        end_long = a.cend();
        it_short = b.cbegin();
        end_short = b.cend();
        long_operand = std::move(a);
        short_operand = std::move(b);
    } else {
        it_long = b.cbegin();
        end_long = b.cend();
        it_short = a.cbegin();
        end_short = a.cend();
        long_operand = std::move(b);
        short_operand = std::move(a);
    }

    while (it_long < end_long) {
        dist += pow(it_long->value() - short_operand[it_long->index()], 2);
        //std::cout << "long dist upd,  " << dist <<  ", " << it_long->value() << ", " << it_long->index() << ", " << short_operand[it_long->index()] << std::endl;  // TODO remove
        ++it_long;
    }
    while (it_short < end_short) {
        //std::cout << "short dist chk, " << dist << ", " << it_short->value() << ", " << it_short->index() << ", " << long_operand[it_short->index()] << std::endl;  // TODO remove
        if (long_operand.find(it_short->index()) == long_operand.cend()) { // zero
        //if (long_operand[it_short->index()] == 0) {
            dist += pow(it_short->value(), 2);  // with zero
            //std::cout << "short dist upd, " << dist << ", " << it_short->value() << ", " << it_short->index() << ", " << long_operand[it_short->index()] << std::endl;  // TODO remove
        }
        ++it_short;
    }


    // old ver

//    while ( (it_a != a.end()) && (it_b != b.end()) ) {

//        if (it_a->index() < it_b->index()) {
//            ++it_a;
//        } else if (it_a->index() > it_b->index()) {
//            ++it_b;
//        } else {
//            dist += pow(it_a->value() - it_b->value(), 2);
//        }
//    }

    return sqrt(dist);
}


// */



// ------ main


using T = double;


int main() {


    //std::vector<std::vector<std::vector<T>>> clustered_energies;

    std::vector<uint32_t> window_sizes = {12, 24, 48, 96, 192, 384};

    /*
    // saving the model
    {

        // setup ----
        int som_w_grid_size = 30;
        int som_h_grid_size = 30;
        double som_start_learn_rate = 0.8;
        double som_final_learn_rate = 0.2;
        unsigned int som_iterations = 100; //10000;
        double som_initial_neighbour_size = 5;
        double som_neigbour_range_decay = 2;
        long long som_random_seed = 0;

        size_t num_clusters = 5;

        //std::vector<uint32_t> window_sizes = {12, 24, 48, 96, 192, 384};
        uint32_t samples = 1000;
        double confidence_level = 0.99;

        auto ds = read_csv_num<T>("anomaly_detector_data_1/script/energies01_short.csv");
        // -----------


        size_t num_subbands = ds[0].size();

        std::uniform_real_distribution<T> som_distribution (-1, 1);
        metric::Euclidean<T> som_distance;
        metric::Grid4 som_graph (som_w_grid_size, som_h_grid_size);

        metric::SOM<std::vector<T>, metric::Grid4> som (
                    som_graph,
                    som_distance,
                    som_start_learn_rate,
                    som_final_learn_rate,
                    som_iterations,
                    som_distribution,
                    som_initial_neighbour_size,
                    som_neigbour_range_decay,
                    som_random_seed
                    );

        som.train(ds);


        auto nodes_data = som.get_weights();

        auto [assignments, means, counts] = metric::kmeans(nodes_data, num_clusters, 1000);


    //    nlohmann::json centroids;
    //    for (auto mean : means) {
    //        centroids.push_back(mean);
    //    };


        std::vector<std::vector<std::vector<T>>> clustered_energies (
        //clustered_energies = std::vector<std::vector<std::vector<T>>>(
            counts.size(),
            std::vector<std::vector<T>>(num_subbands)
        );

        for (auto record : ds)
        {
            // find cluster id for a record
            auto bmu = som.BMU(record);
            auto cluster_index = assignments[bmu];
            for (int sb = 0; sb < num_subbands; sb++) {
                clustered_energies[cluster_index][sb].push_back(record[sb]);
            }


        }

    //    std::vector<std::vector<std::vector<std::vector<std::vector<T>>>>> conf_bounds (
    //        counts.size(),
    //        std::vector<std::vector<std::vector<std::vector<T>>>> (
    //            num_subbands,
    //            std::vector<std::vector<std::vector<T>>> (
    //                window_sizes.size(),
    //                std::vector<std::vector<T>> (
    //                    3  // left, middle, right
    //                )
    //            )
    //        )
    //    );


        nlohmann::json model;
        size_t cl_idx = 0;
        for (auto cluster_data : clustered_energies) {

            if (cluster_data[0].size() < 3) {  // set2multiconf needs at least 3 points
                ++cl_idx;
                std::cout << "small cluster" << std::endl;
                continue;
            }

            nlohmann::json all_subbands_json;
            size_t sb_idx = 0;
            for (auto energy_subband_data : cluster_data) {

//                if (energy_subband_data.size() < 5) {
                    //++sb_idx;
                    //continue;
//                    std::cout << "small cluster" << std::endl;
//                    if (energy_subband_data.size() == 2) {   // TODO remove
//                        energy_subband_data.push_back(100.5);
//                    }
//                }

                // returns quants for a single subbund
                std::vector<std::vector<std::vector<T>>> multiquants = set2multiconf(energy_subband_data, window_sizes, samples, confidence_level);

                nlohmann::json all_windows_json;
                size_t w_idx = 0;
                for (auto window : multiquants) {

    //                conf_bounds[cl_idx][sb_idx][w_idx][0] = window[0];
    //                conf_bounds[cl_idx][sb_idx][w_idx][1] = window[1];
    //                conf_bounds[cl_idx][sb_idx][w_idx][2] = window[2];

                    nlohmann::json window_json = {
                        {"_window_length_index", std::to_string(window_sizes[w_idx])},
                        {"_window_length", std::to_string(w_idx)},
                        {"conf_l", window[0]},
                        {"conf_m", window[1]},
                        {"conf_r", window[2]}
                    };
                    all_windows_json.push_back(window_json);

    //                std::cout << "conf_l: " << std::endl;
    //                vector_print(window[0]);
    //                std::cout << "conf_m: " << std::endl;
    //                vector_print(window[1]);
    //                std::cout << "conf_r: " << std::endl;
    //                vector_print(window[2]);

                    ++w_idx;
                }
                nlohmann::json subband_json = {
                    {"_subband", std::to_string(sb_idx)},
                    {"conf_windows", all_windows_json}
                };
                all_subbands_json.push_back(subband_json);
                ++sb_idx;
            }
            nlohmann::json cluster_json  = {
                {"_cluster", std::to_string(cl_idx)},
                {"subbands", all_subbands_json},
                {"centroid", means[cl_idx]},

            };
            model.push_back(cluster_json);
            ++cl_idx;
        }


    //    nlohmann::json model = {
    //        {"centroids", centroids},
    //        {"conf_windows", conf_wnds}
    //    };

        std::ofstream outputFile("model.json");
        outputFile << std::setw(4) << model << std::endl;
        outputFile.close();


        std::cout << "training done" << std::endl;




    }
    //*/



    //*

    // applying the model
    {

        std::ifstream f("model.json");
        nlohmann::json model;
        f >> model;

        if (model.size() < 1) {
            std::cout << "empty model" << std::endl;
            return 0;
        }

        std::vector<std::vector<T>> means;

//        std::vector<std::vector<std::vector<std::vector<std::vector<T>>>>> conf_bounds (
//            model.size(),
//            std::vector<std::vector<std::vector<std::vector<T>>>> (
//                model[0]["subbands"].size(),
//                std::vector<std::vector<std::vector<T>>> (
//                    model[0]["subbands"][0]["conf_windows"].size(),
//                    std::vector<std::vector<T>> (
//                        3  // left, middle, right
//                    )
//                )
//            )
//        );
        std::vector<std::vector<std::vector<std::vector<std::vector<T>>>>> conf_bounds = {};

        for (auto cluster : model) {
            means.push_back(cluster["centroid"]);
            std::vector<std::vector<std::vector<std::vector<T>>>> cluster_vec = {};
            for (auto subband : cluster["subbands"]) {
                std::vector<std::vector<std::vector<T>>> subband_vec = {};
                for (auto window : subband["conf_windows"]) {
                    std::vector<std::vector<T>> window_vec = {};
                    window_vec.push_back(window["conf_l"]);
                    window_vec.push_back(window["conf_m"]);
                    window_vec.push_back(window["conf_r"]);
                    //auto w = window["conf_l"];  // TODO remove
                    //if (window["conf_m"].m_type != nlohmann::detail::value_t::null) {
                    //    window_vec.push_back(window["conf_l"]);
                    //    window_vec.push_back(window["conf_m"]);
                    //    window_vec.push_back(window["conf_r"]);
                    //} else {
                    //    std::cout << "empty cluster found" << std::endl;
                    //}
                    subband_vec.push_back(window_vec);
                }
                cluster_vec.push_back(subband_vec);
            }
            conf_bounds.push_back(cluster_vec);
        }
        // model loaded



        size_t num_subbands = conf_bounds[0].size();
        size_t num_windows = window_sizes.size();  // TODO save window sizes to JSON

        auto ds = read_csv_num<T>("anomaly_detector_data_1/script/energies01_short.csv");

        assert(num_subbands == ds[0].size());
        assert(num_subbands == model[0]["subbands"].size());  // TODO remove

        size_t buf_size = window_sizes[num_windows - 1];

        assert(buf_size <= ds.size());

        std::vector<std::vector<T>> cl_probs = {};

        std::vector<std::vector<std::vector<T>>> buffer;  // subbands, windows, data
        buffer.reserve(num_subbands);
        size_t pos_idx = buf_size;
        while (ds.begin() + pos_idx < ds.end()) {  // sliding along timeseries

//            //std::vector<std::vector<T>> buffer (ds.begin(), ds.begin() + buf_size);
//            std::vector<std::vector<std::vector<T>>> buffer (num_subbands, std::vector<std::vector<T>>(num_windows, std::vector<T>()));

//            //std::vector<T> w = {};
//            for (size_t w_sz_idx = 0; w_sz_idx < window_sizes; ++w_sz_idx) { // we assume window sizes are sorted so first we take the smallest
//                //w.puch_back
//                for (size_t sb_idx = 0; sb_idx < num_subbands; ++sb_idx) {
//                    //for (size_t rec_idx = 0; )
//                    //buffer[sb][w_sz].push_back
//                }
//            }

            std::vector<T> avg_prob (model.size(), 0);
            for (size_t sb_idx = 0; sb_idx < num_subbands; ++sb_idx) {

                std::vector<T> avg_subband_prob (model.size(), 0);
                std::vector<std::vector<T>> subband = {};
                //for (auto wnd_size : window_sizes) {
                for (auto ws_idx = 0; ws_idx < window_sizes.size(); ++ws_idx) {
                    auto wnd_size = window_sizes[ws_idx];
                    std::vector<T> wnd;
                    wnd.reserve(wnd_size);
                    for (auto val_idx = pos_idx - wnd_size; val_idx < pos_idx; ++val_idx) { // reading window
                        wnd.push_back(ds[val_idx][sb_idx]);
                    }
                    //std::vector<T> avg_wnd_prob (model.size(), 0);
                    std::sort(wnd.begin(), wnd.end());
                    // here we have window filled
                    // loop clusters, compare each sorted element to respective bounds, save for current wndsize
                    for (size_t cl_idx = 0; cl_idx < model.size(); ++cl_idx) {
                        T wnd_prob = 0;
                        for (size_t el_idx = 0; el_idx < wnd.size(); el_idx++) {
                            // compare element to bounds
                            auto lb = conf_bounds[cl_idx][sb_idx][ws_idx][0][el_idx];
                            auto rb = conf_bounds[cl_idx][sb_idx][ws_idx][2][el_idx];
                            if ( (wnd[el_idx] >= lb) && (wnd[el_idx] <= rb) ) {
                                wnd_prob++;
                            }
                        }
                        wnd_prob /= (T)wnd.size();
                        avg_subband_prob[cl_idx] += wnd_prob;
                    }
                    // here we have avg window probability evaluated for each cluster (stored in avg_wnd_prob)
                    // avg_subband_prob[cl_idx] += wnd_prob[cl_idx];
                    subband.push_back(wnd);
                }
                // subband filled
                for (size_t cl_idx = 0; cl_idx < model.size(); ++cl_idx) {
                    avg_subband_prob[cl_idx] /= (T)num_windows;
                    avg_prob[cl_idx] += avg_subband_prob[cl_idx];  // adding each subband's summand to overall prob, per cluster
                }
                // here we have average subband probability for each cluster
                buffer.push_back(subband);
            }
            // here buffer is filled for the current pos_idx
            // and overall cluster belonging probabilities are summed over al subbands
            for (size_t cl_idx = 0; cl_idx < model.size(); ++cl_idx) {
                avg_prob[cl_idx] /= (T)num_subbands;
            }
            //std::cout << "buffer filled" << std::endl;
            // here we have probs for all clusters computed for the current input data window

            cl_probs.push_back(avg_prob);

            // TODO compute final anomaly score


            buffer = {};   // TODO optimize with sliding (via push-pop) ring buffer, maybe based on deque
            ++pos_idx;
        }


        vv_to_csv(cl_probs, "anomaly_detector_data_1/script/cl_probs.csv");



        std::cout << "done" << std::endl;

    }


    // */



    /*

    // measurement

    std::vector<double> sparsities = {-0.01, 0.02, 0.04, 0.06, 0.08, 0.1, 0.2, 0.4, 0.6, 0.8, 1.1};
    //std::vector<double> overreserves = { 0.1, 0.3,  0.3,  0.3,  0.3, 0.3, 0.4, 0.5, 0.4, 0.2, 0};  // l = 30
    std::vector<double> overreserves = { 0.1, 0.1,  0.1,  0.1,  0.1, 0.1, 0.3, 0.2, 0.1, 0.1, 0}; // l = 500

    for (size_t i = 0; i<sparsities.size(); ++i) {
    ////


    size_t n = 50000;
    size_t l = 500;
    double val_range = 1000;
    //double sparsity = 0.08;
    //double overreserve = 0.1;
    double sparsity = sparsities[i];
    double overreserve = overreserves[i];


    std::vector<blaze::DynamicVector<double>> A;
    std::vector<blaze::DynamicVector<double>> B;
    std::vector<blaze::CompressedVector<double>> Ac;
    std::vector<blaze::CompressedVector<double>> Bc;
    std::vector<blaze::CompressedVector<double>> Ac2;
    std::vector<blaze::CompressedVector<double>> Bc2;

    double t_sum_1 = 0;
    double t_sum_2 = 0;
    double t_sum_1_c = 0;
    double t_sum_2_c = 0;
    double t_sum_custom = 0;

    int capacity_1 = 0;
    int capacity_2 = 0;

    auto eng = std::mt19937();
    auto randDouble = std::uniform_real_distribution<> {0, 1};

    auto t1 = std::chrono::steady_clock::now();
    auto t2 = std::chrono::steady_clock::now();

    double dist;

    double dist1 = 0;
    double dist2 = 0;
    double dist3 = 0;
    double dist4 = 0;
    double dist5 = 0;


    for (size_t n_idx = 0; n_idx < n; ++n_idx) {

        blaze::DynamicVector<double> a (l, 0);
        blaze::DynamicVector<double> b (l, 0);
        blaze::CompressedVector<double> ac (l);
        blaze::CompressedVector<double> bc (l);
//        blaze::CompressedVector<double> ac;
//        blaze::CompressedVector<double> bc;
        ac.reserve(l*(sparsity + overreserve));  // risky
        bc.reserve(l*(sparsity + overreserve));
//        ac.reserve(l);
//        bc.reserve(l);

        for (size_t l_idx = 0; l_idx < l; ++l_idx) {
            //double x1 = (double)l_idx - l/2;
            //double x2 = (double)l_idx + l/2;

            if (randDouble(eng) < sparsity) {
                //a[l_idx] = x1 + randDouble(eng)*l;
                //b[l_idx] = x2 + randDouble(eng)*l;

                a[l_idx] = randDouble(eng)*val_range - val_range/2;
                //b[l_idx] = randDouble(eng)*val_range - val_range/2;

//                ac[l_idx] = a[l_idx];
//                bc[l_idx] = b[l_idx];

//                a.append(l_idx, randDouble(eng)*val_range - val_range/2);
//                b.append(l_idx, randDouble(eng)*val_range - val_range/2);

                ac.append(l_idx, a[l_idx]);
                //bc.append(l_idx, b[l_idx]);
            }

            if (randDouble(eng) < sparsity) {
                b[l_idx] = randDouble(eng)*val_range - val_range/2;
                bc.append(l_idx, b[l_idx]);
//                bc[l_idx] = b[l_idx];
            }

//            if (n_idx > n - 500)
//                std::cout << a[l_idx] << ", " << b[l_idx] << std::endl;
        }

        capacity_1 += ac.capacity();
        capacity_2 += bc.capacity();

//        if (n_idx > n - 500)
//            std::cout << std::endl;

        A.push_back(a);
        B.push_back(b);
        Ac.push_back(ac);
        Bc.push_back(bc);


        t1 = std::chrono::steady_clock::now();
        dist = blaze::norm(b - a);
        t2 = std::chrono::steady_clock::now();
        t_sum_1 += double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        dist1 += dist;

        t1 = std::chrono::steady_clock::now();
        dist = blaze::l2Norm(b - a);
        t2 = std::chrono::steady_clock::now();
        t_sum_2 += double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        dist2 += dist;

        t1 = std::chrono::steady_clock::now();
        dist = blaze::norm(bc - ac);
        t2 = std::chrono::steady_clock::now();
        t_sum_1_c += double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        dist3 += dist;

        t1 = std::chrono::steady_clock::now();
        dist = blaze::l2Norm(bc - ac);
        t2 = std::chrono::steady_clock::now();
        t_sum_2_c += double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        dist4 += dist;

        t1 = std::chrono::steady_clock::now();
        dist = sparse_euclidean(bc, ac);
        t2 = std::chrono::steady_clock::now();
        t_sum_custom += double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;
        dist5 += dist;

        if ( abs(dist1 - dist2) + abs(dist1 - dist3) + abs(dist1 - dist4) + abs(dist1 - dist5) > 1e-07) {
            std::cout << std::endl << dist1 << ", " << dist2 << ", " << dist3 << ", " << dist4 << ", " << dist5 << std::endl;
            return 0;
        }
        //std::cout << abs(dist1 - dist2) + abs(dist1 - dist3) + abs(dist1 - dist4) << std::endl;


    }

    std::cout << std::endl;
    std::cout << "vectors: " << n << ", vec length: " << l << ", nonzero elements share: " << sparsity << std::endl;

    std::cout << std::endl;
    std::cout << "sum of times of all calls:" << std::endl;


    std::cout << "     norm, Dynamic: " << t_sum_1 << std::endl;
    std::cout << "   l2norm, Dynamic: " << t_sum_2 << std::endl;
    std::cout << "  norm, Compressed: " << t_sum_1_c << std::endl;
    std::cout << "l2norm, Compressed: " << t_sum_2_c << std::endl;
    std::cout << "custom, Compressed: " << t_sum_custom << std::endl;

    std::cout << "    norm, speed-up: " << t_sum_1 / t_sum_1_c << std::endl;
    std::cout << "  l2norm, speed-up: " << t_sum_2 / t_sum_2_c << std::endl;
    std::cout << "  custom, speed-up: " << t_sum_2 / t_sum_custom << std::endl;

    std::cout << "sums of distances: " << dist1 << ", " << dist2 << ", " << dist3 << ", " << dist4 << ", " << dist5 << std::endl;




    // loop

    dist1 = 0;
    dist2 = 0;
    dist3 = 0;
    dist4 = 0;
    dist5 = 0;

    t1 = std::chrono::steady_clock::now();
    for (size_t idx = 0; idx < n; ++idx) {
        dist = blaze::norm(B[idx] - A[idx]);
        dist1 += dist;
    }
    t2 = std::chrono::steady_clock::now();
    t_sum_1 = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;

    t1 = std::chrono::steady_clock::now();
    for (size_t idx = 0; idx < n; ++idx) {
        dist = blaze::l2Norm(B[idx] - A[idx]);
        dist2 += dist;
    }
    t2 = std::chrono::steady_clock::now();
    t_sum_2 = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;

    t1 = std::chrono::steady_clock::now();
    for (size_t idx = 0; idx < n; ++idx) {
        dist = blaze::norm(Bc[idx] - Ac[idx]);
        dist3 += dist;
    }
    t2 = std::chrono::steady_clock::now();
    t_sum_1_c = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;

    t1 = std::chrono::steady_clock::now();
    for (size_t idx = 0; idx < n; ++idx) {
        dist = blaze::l2Norm(Bc[idx] - Ac[idx]);
        dist4 += dist;
    }
    t2 = std::chrono::steady_clock::now();
    t_sum_2_c = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;

    t1 = std::chrono::steady_clock::now();
    for (size_t idx = 0; idx < n; ++idx) {
        dist = sparse_euclidean(Bc[idx], Ac[idx]);
        dist5 += dist;
    }
    t2 = std::chrono::steady_clock::now();
    t_sum_custom = double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000;


    std::cout << std::endl;
    std::cout << "time of execution in for loop:" << std::endl;

    std::cout << "     norm, Dynamic: " << t_sum_1 << std::endl;
    std::cout << "   l2norm, Dynamic: " << t_sum_2 << std::endl;
    std::cout << "  norm, Compressed: " << t_sum_1_c << std::endl;
    std::cout << "l2norm, Compressed: " << t_sum_2_c << std::endl;
    std::cout << "custom, Compressed: " << t_sum_custom << std::endl;

    std::cout << "    norm, speed-up: " << t_sum_1 / t_sum_1_c << std::endl;
    std::cout << "  l2norm, speed-up: " << t_sum_2 / t_sum_2_c << std::endl;
    std::cout << "  custom, speed-up: " << t_sum_2 / t_sum_custom << std::endl;

    std::cout << "sums of distances: " << dist1 << ", " << dist2 << ", " << dist3 << ", " << dist4 << ", " << dist5 << std::endl;

    std::cout << "avg capacities: " << capacity_1/(double)n << ", " << capacity_2/(double)n << std::endl;


    ////
    }  // for i

    // */


    /*
    // sparse_euclidean test

    blaze::CompressedVector<double> ac (10);
    blaze::CompressedVector<double> bc (10);
    ac[2] = 1;
    ac[8] = 1;
    bc[5] = 1;
    bc[8] = 1;
    bc[10] = 1;

    double d = sparse_euclidean(ac, bc);

    std::cout << d << ", " << blaze::l2Norm(ac - bc) << std::endl;



    // */





    return 0;
}
