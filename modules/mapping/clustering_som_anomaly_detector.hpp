
#include "modules/mapping/SOM.hpp"
#include "modules/mapping/kmedoids.hpp"

//#include "modules/mapping/PCFA.hpp"

//#include "assets/json.hpp"

#include <vector>
#include <iostream>  // TODO remove debug output
#include <fstream>
#include <sstream>

//#include <string>




namespace metric {

namespace clustering_som_anomaly_detector_details {




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





// positiona and boundaries of clusters

bool get_hex_bounds(std::vector<int> assignments, int checking_cluster_index, int near_cluster_index)
{
    if (near_cluster_index >= 0 && near_cluster_index < assignments.size())
    {
        if (assignments[checking_cluster_index] != assignments[near_cluster_index])
        {
            return true;
        }
    }

    return false;
}


std::tuple<std::vector<std::vector<int>>, std::vector<std::vector<int>>>
getPositionsAndBorders(std::vector<int> assignments, int clusters_num, int row_len)
{
    std::vector<std::vector<int>> positions(clusters_num);
    //std::vector<std::vector<std::vector<int>>> borders(counts.size());
    std::vector<std::vector<int>> borders(clusters_num);
    for (int i = 0; i < assignments.size(); i++)
    {
        positions[assignments[i]].push_back(i);

        //

        int row = i / row_len;
        int near_cluster_index;
        int near_cluster_index_row;

        // prev row

        if (row % 2 == 0)
        {
            near_cluster_index = i - row_len - 1;
            near_cluster_index_row = near_cluster_index / row_len;
            if(row - 1 == near_cluster_index_row)
                if (get_hex_bounds(assignments, i, near_cluster_index))
                {
                    //borders[assignments[i]].push_back(std::vector<int> {near_cluster_index, i});
                    borders[assignments[i]].push_back(near_cluster_index);
                    borders[assignments[i]].push_back(i);
                }

            near_cluster_index = i - row_len;
            near_cluster_index_row = near_cluster_index / row_len;
            if(row - 1 == near_cluster_index_row)
                if (get_hex_bounds(assignments, i, near_cluster_index))
                {
                    //borders[assignments[i]].push_back(std::vector<int> {near_cluster_index, i});
                    borders[assignments[i]].push_back(near_cluster_index);
                    borders[assignments[i]].push_back(i);
                }
        }
        else
        {
            near_cluster_index = i - row_len;
            near_cluster_index_row = near_cluster_index / row_len;
            if(row - 1 == near_cluster_index_row)
                if (get_hex_bounds(assignments, i, near_cluster_index))
                {
                    //borders[assignments[i]].push_back(std::vector<int> {near_cluster_index, i});
                    borders[assignments[i]].push_back(near_cluster_index);
                    borders[assignments[i]].push_back(i);
                }

            near_cluster_index = i - row_len + 1;
            near_cluster_index_row = near_cluster_index / row_len;
            if(row - 1 == near_cluster_index_row)
                if (get_hex_bounds(assignments, i, near_cluster_index))
                {
                    //borders[assignments[i]].push_back(std::vector<int> {near_cluster_index, i});
                    borders[assignments[i]].push_back(near_cluster_index);
                    borders[assignments[i]].push_back(i);
                }
        }

        // current row

        near_cluster_index = i - 1;
        near_cluster_index_row = near_cluster_index / row_len;
        if(row == near_cluster_index_row)
            if (get_hex_bounds(assignments, i, near_cluster_index))
            {
                //borders[assignments[i]].push_back(std::vector<int> {near_cluster_index, i});
                borders[assignments[i]].push_back(near_cluster_index);
                borders[assignments[i]].push_back(i);
            }

        near_cluster_index = i + 1;
        near_cluster_index_row = near_cluster_index / row_len;
        if(row == near_cluster_index_row)
            if (get_hex_bounds(assignments, i, near_cluster_index))
            {
                //borders[assignments[i]].push_back(std::vector<int> {i, near_cluster_index});
                borders[assignments[i]].push_back(i);
                borders[assignments[i]].push_back(near_cluster_index);
            }

        // next row

        if (row % 2 == 0)
        {
            near_cluster_index = i + row_len - 1;
            near_cluster_index_row = near_cluster_index / row_len;
            if(row + 1 == near_cluster_index_row)
                if (get_hex_bounds(assignments, i, near_cluster_index))
                {
                    //borders[assignments[i]].push_back(std::vector<int> {i, near_cluster_index});
                    borders[assignments[i]].push_back(i);
                    borders[assignments[i]].push_back(near_cluster_index);
                }

            near_cluster_index = i + row_len;
            near_cluster_index_row = near_cluster_index / row_len;
            if(row + 1 == near_cluster_index_row)
                if (get_hex_bounds(assignments, i, near_cluster_index))
                {
                    //borders[assignments[i]].push_back(std::vector<int> {i, near_cluster_index});
                    borders[assignments[i]].push_back(i);
                    borders[assignments[i]].push_back(near_cluster_index);
                }
        }
        else
        {
            near_cluster_index = i + row_len;
            near_cluster_index_row = near_cluster_index / row_len;
            if(row + 1 == near_cluster_index_row)
                if (get_hex_bounds(assignments, i, near_cluster_index))
                {
                    //borders[assignments[i]].push_back(std::vector<int> {i, near_cluster_index});
                    borders[assignments[i]].push_back(i);
                    borders[assignments[i]].push_back(near_cluster_index);
                }

            near_cluster_index = i + row_len + 1;
            near_cluster_index_row = near_cluster_index / row_len;
            if(row + 1 == near_cluster_index_row)
                if (get_hex_bounds(assignments, i, near_cluster_index))
                {
                    //borders[assignments[i]].push_back(std::vector<int> {i, near_cluster_index});
                    borders[assignments[i]].push_back(i);
                    borders[assignments[i]].push_back(near_cluster_index);
                }
        }
    }

    return {positions, borders};
}



} // namespace




template <typename T>
class ClusteringSomAnomalyDetector {

public:

    ClusteringSomAnomalyDetector(const std::string & filename) {
        // TODO
    }

    ClusteringSomAnomalyDetector(const std::vector<std::vector<T>> & ds) {  // train using features

        std::vector<uint32_t> window_sizes = {12, 24, 48, 96, 192, 384};

        // setup ----  // TODO pass
        int som_w_grid_size = 30;
        int som_h_grid_size = 30;
        double som_start_learn_rate = 0.8;
        double som_final_learn_rate = 0.2;
        unsigned int som_iterations = 10000;
        double som_initial_neighbour_size = 5;
        double som_neigbour_range_decay = 2;
        long long som_random_seed = 0;

        size_t num_clusters = 4; //10;

        //std::vector<uint32_t> window_sizes = {12, 24, 48, 96, 192, 384};
        uint32_t samples = 1000;
        double confidence_level = 0.99;


        // PCA  // TODO add flag
//        auto pcfa = metric::PCFA<std::vector<T>, void>(ds_raw, 30);
//        auto ds = pcfa.encode(ds_raw);

        // save PCFA model
//        auto eigenmodes = pcfa.eigenmodes();
//        nlohmann::json pcfa_model = eigenmodes;
//        std::ofstream pcfa_model_f("pcfa_model.json");
//        pcfa_model_f << std::setw(4) << pcfa_model << std::endl;
//        pcfa_model_f.close();

//        std::cout << "PCFA completed and written" << std::endl;

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
        std::cout << "SOM trained" << std::endl;

        //auto [assignments, means, counts] = metric::kmeans(nodes_data, num_clusters, 1000);
        metric::Matrix<std::vector<T>, metric::Euclidean<T>> matrix (nodes_data);
        auto [assignments, means, counts] = metric::kmedoids(matrix, num_clusters);
        std::cout << "clustering completed" << std::endl;

//        nlohmann::json centroids;
//        for (auto mean : means) {
//            centroids.push_back(mean);
//        };

        // debug output
        clustering_som_anomaly_detector_details::vv_to_csv(nodes_data, "anomaly_detector_data_1/script/som_nodes.csv");
        clustering_som_anomaly_detector_details::vv_to_csv(std::vector<std::vector<int>> {assignments}, "anomaly_detector_data_1/script/som_nodes_clusters.csv");


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

        conf_bounds = std::vector<std::vector<std::vector<std::vector<std::vector<T>>>>>(
            counts.size(),
            std::vector<std::vector<std::vector<std::vector<T>>>> (
                num_subbands,
                std::vector<std::vector<std::vector<T>>> (
                    window_sizes.size(),
                    std::vector<std::vector<T>> (
                        3  // left, middle, right
                    )
                )
            )
        );

        nodes = std::vector<std::vector<std::vector<T>>>(counts.size());

        //nlohmann::json model;
        size_t cl_idx = 0;
        for (auto cluster_data : clustered_energies) {

            // skip small clusters
            if (cluster_data[0].size() < 3) {  // set2multiconf needs at least 3 points
                ++cl_idx;
                std::cout << "small cluster" << std::endl;
                continue;
            }

            // add subband confidence intervals per window size
            //nlohmann::json all_subbands_json;
            size_t sb_idx = 0;
            for (auto energy_subband_data : cluster_data) {

                // returns quants for a single subbund
                std::vector<std::vector<std::vector<T>>> multiquants = clustering_som_anomaly_detector_details::set2multiconf(energy_subband_data, window_sizes, samples, confidence_level);

                //nlohmann::json all_windows_json;
                size_t w_idx = 0;
                for (auto window : multiquants) {

                    conf_bounds[cl_idx][sb_idx][w_idx][0] = window[0];
                    conf_bounds[cl_idx][sb_idx][w_idx][1] = window[1];
                    conf_bounds[cl_idx][sb_idx][w_idx][2] = window[2];

//                    nlohmann::json window_json = {
//                        {"_window_length_index", std::to_string(window_sizes[w_idx])},
//                        {"_window_length", std::to_string(w_idx)},
//                        {"conf_l", window[0]},
//                        {"conf_m", window[1]},
//                        {"conf_r", window[2]}
//                    };
//                    all_windows_json.push_back(window_json);

//                    std::cout << "conf_l: " << std::endl;
//                    vector_print(window[0]);
//                    std::cout << "conf_m: " << std::endl;
//                    vector_print(window[1]);
//                    std::cout << "conf_r: " << std::endl;
//                    vector_print(window[2]);

                    ++w_idx;
                }
//                nlohmann::json subband_json = {
//                    {"_subband", std::to_string(sb_idx)},
//                    {"conf_windows", all_windows_json}
//                };
//                all_subbands_json.push_back(subband_json);
                ++sb_idx;
            }

            // add nodes that fall into cluster
            std::vector<std::vector<T>> cluster_nodes = {};
            for (size_t node_idx = 0; node_idx < assignments.size(); ++node_idx) {
                if (assignments[node_idx] == cl_idx) {
                    cluster_nodes.push_back(nodes_data[node_idx]);
                }
            }
            nodes.push_back(cluster_nodes);

//            // write cluster json
//            nlohmann::json cluster_json  = {
//                {"_cluster", std::to_string(cl_idx)},
//                {"subbands", all_subbands_json},
//                //{"centroid", means[cl_idx]},
//                {"som_units", nodes}

//            };
//            model.push_back(cluster_json);
            ++cl_idx;
        }


    //    nlohmann::json model = {
    //        {"centroids", centroids},
    //        {"conf_windows", conf_wnds}
    //    };

//        std::ofstream outputFile("model.json");
//        outputFile << std::setw(4) << model << std::endl;
//        outputFile.close();

//        std::cout << "model written" << std::endl;

    }

    void save(const std::string & filename) {
        // TODO
    }

    std::vector<T> encode(const std::vector<std::vector<T>> & dataset) {
        // TODO
    }


private:

    std::vector<T>  // numbers of sliding window points that fall into each cluster (by BMUs)
    cluster_entries(
        const std::vector<std::vector<std::vector<T>>> & nodes,
        const std::vector<std::vector<T>> & dataset,
        const size_t pos_idx, // where to start from
        const size_t wnd_size
        // and maybe metric type as template parameter
        )
    {
        // TODO
    }

    T // single anomaly estimation value
    estimate_anomaly(
        const std::vector<std::vector<T>> & data,  // general window (slice of timeseries of features), passed the entropy check
        const std::vector<std::vector<std::vector<std::vector<T>>>> cluster_conf_bounds // cluster from model, confidence bounds for all window sizes
        //...
        )
    {
        // TODO
    }


    std::vector<std::vector<std::vector<std::vector<std::vector<T>>>>> conf_bounds;
    // clusters, subbands, window sizes, bound types (left, med, right), quant values

    std::vector<std::vector<std::vector<T>>> nodes;  // clusters, nodes, node features


}; // class clustering_som_anomaly_detector






}  // namespace metric
