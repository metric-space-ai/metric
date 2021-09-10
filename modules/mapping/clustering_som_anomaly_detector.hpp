
#include "modules/mapping/SOM.hpp"
#include "modules/mapping/kmedoids.hpp"

#include "modules/distance/k-random/RandomEMD.hpp"

//#include "modules/mapping/PCFA.hpp"

#include "3rdparty/nlohmann/json.hpp"

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




// discrete CDF for updated RandomEMD

template <typename T>
std::tuple<std::vector<T>, std::vector<T>> discrete_cdf(const std::vector<T> & sorted_sample, size_t steps = 0) {

//    T min_value =  std::numeric_limits<T>::max();
//    T max_value = -std::numeric_limits<T>::max();

//    for (const auto & val : sorted_sample) {
//        if (val < min_value)
//            min_value = val;
//        if (val > max_value)
//            max_value = val;
//    }

    if (steps == 0)
        steps = sorted_sample.size() + 1;

    std::vector<T> cdf_arg;
    std::vector<T> cdf_prob;
    cdf_arg.reserve(steps + 1);
    cdf_prob.reserve(steps + 1);
    T step = (sorted_sample[sorted_sample.size() - 1] - sorted_sample[0]) / (T)steps;
    T current_val = sorted_sample[0];
    T prev_val = current_val - 1;
    T count = 0;
    //T prob_step = 1/(T)steps;
    for (const auto & val : sorted_sample) {
        ++count;
        if (val <= prev_val) {
            //current_prob += prob_step;
            //++current_prob;
            cdf_prob[cdf_prob.size() - 1] = count;
            continue;
        }
        if (val >= current_val) {
            //current_prob += prob_step;
            //++current_prob;
            cdf_arg.push_back(val);
            cdf_prob.push_back(count);
            prev_val = val;
            current_val += step;
        }
    }
    for (auto & prob : cdf_prob)
        prob /= cdf_prob[cdf_prob.size() - 1];

    return std::make_tuple(cdf_arg, cdf_prob);
}



// updated RandomEMD (of degree 1)

template <typename T>
T discrete_randomEMD(
        const std::tuple<std::vector<T>, std::vector<T>> & cdf1,
        const std::tuple<std::vector<T>, std::vector<T>> & cdf2
        )
{

    std::vector<T> rv1 = std::get<0>(cdf1);
    std::vector<T> rv2 = std::get<0>(cdf2);
    std::vector<T> probs1 = std::get<1>(cdf1);
    std::vector<T> probs2 = std::get<1>(cdf2);

    std::vector<T> rv_concat = {-std::numeric_limits<T>::max()};
    rv_concat.insert(rv_concat.end(), rv1.begin(), rv1.end());
    rv_concat.insert(rv_concat.end(), rv2.begin(), rv2.end());
    sort(rv_concat.begin(), rv_concat.end());

    size_t rv1_idx = 0;
    size_t rv2_idx = 0;
    T prob1 = 0;
    T prob2 = 0;
    T area = 0;
    for (size_t idx = 0; idx < rv_concat.size() - 1; ++idx) {
        while ( (rv_concat[idx] >= rv1[rv1_idx]) && (rv1_idx < rv1.size()) ) {
            prob1 = probs1[rv1_idx];
            ++rv1_idx;
        }
        while ( (rv_concat[idx] >= rv2[rv2_idx]) && (rv2_idx < rv2.size()) ) {
            prob2 = probs2[rv2_idx];
            ++rv2_idx;
        }
        area += abs(prob2 - prob1) * (rv_concat[idx + 1] - rv_concat[idx]);
    }

    return area;
}



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
std::tuple<std::vector<std::vector<std::vector<T>>>, std::vector<T>, std::vector<T>>
set2multiconf(std::vector<T> set_0, std::vector<uint32_t> windowSizes, size_t samples, T confidencelevel)
{
  std::vector<std::vector<std::vector<T>>> multiquants;
  for (size_t i = 0; i < windowSizes.size(); ++i)
  {
    multiquants.push_back(set2conf(set_0, windowSizes[i], samples, confidencelevel));
  }

  auto dcdf = discrete_cdf(set_0, 500);
  std::cout << std::endl << "-------" << std::endl;  // TODO remove
  std::cout << set_0;
  std::cout << std::get<0>(dcdf) << std::endl;
  std::cout << std::get<1>(dcdf) << std::endl;

  return std::make_tuple(multiquants, std::get<0>(dcdf), std::get<1>(dcdf));
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




} // namespace clustering_som_anomaly_detector_details




template <typename T>
class ClusteringSomAnomalyDetector {

public:

    ClusteringSomAnomalyDetector(const std::string & filename) {

        nlohmann::json model;
        {
            std::ifstream f(filename);
            f >> model;
        }

        if (model.size() < 1) {
            std::cout << "empty model" << std::endl;
            return;
        }

        metric::Euclidean<T> som_distance;

        //std::vector<std::vector<T>> means;

        for (auto cluster : model) {

            // read centroid
            //means.push_back(cluster["centroid"]);

            // read conf levels
            std::vector<std::vector<std::vector<std::vector<T>>>> cluster_conf_vec = {};
            std::vector<std::vector<std::vector<T>>> cluster_cdfs = {};

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
                cluster_conf_vec.push_back(subband_vec);

                std::vector<std::vector<T>> subband_cdfs = {};
                subband_cdfs.push_back(subband["cdf_rv_levels"]);
                subband_cdfs.push_back(subband["cdf_probs"]);
                cluster_cdfs.push_back(subband_cdfs);
            }

            conf_bounds.push_back(cluster_conf_vec);

            // read SOM units that belong to cluster
            std::vector<std::vector<T>> cluster_nodes = cluster["som_units"];
            nodes.push_back(cluster_nodes);

            cdfs.push_back(cluster_cdfs);

        }

        assert(nodes.size() == conf_bounds.size());
        assert(cdfs.size() == conf_bounds.size());
        // model loaded
    }


    ClusteringSomAnomalyDetector(const std::vector<std::vector<T>> & ds) {  // train using features

        // setup ----  // TODO pass
        int som_w_grid_size = 30;
        int som_h_grid_size = 30;
        double som_start_learn_rate = 0.8;
        double som_final_learn_rate = 0.2;
        unsigned int som_iterations = 100; //10000;
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

//        conf_bounds = std::vector<std::vector<std::vector<std::vector<std::vector<T>>>>>(
//            counts.size(),
//            std::vector<std::vector<std::vector<std::vector<T>>>> (
//                num_subbands,
//                std::vector<std::vector<std::vector<T>>> (
//                    window_sizes.size(),
//                    std::vector<std::vector<T>> (
//                        3  // left, middle, right
//                    )
//                )
//            )
//        );

        conf_bounds.reserve(counts.size());
        nodes.reserve(counts.size());
        cdfs.reserve(counts.size());

        //nlohmann::json model;
        size_t cl_idx = 0;
        for (auto & cluster_data : clustered_energies) {

            // skip small clusters
            if (cluster_data[0].size() < 3) {  // set2multiconf needs at least 3 points
                ++cl_idx;
                std::cout << "small cluster" << std::endl;
                // TODO remove small cluster from model in memory
                continue;
            }

            std::vector<std::vector<std::vector<std::vector<T>>>> cluster_bounds (
                num_subbands,
                std::vector<std::vector<std::vector<T>>> (
                    window_sizes.size(),
                    std::vector<std::vector<T>> (
                        3  // left, middle, right
                    )
                )
            );
            std::vector<std::vector<std::vector<T>>> cluster_cdfs;
            cluster_cdfs.reserve(num_subbands);
            // add subband confidence intervals per window size
            //nlohmann::json all_subbands_json;
            size_t sb_idx = 0;
            for (auto & energy_subband_data : cluster_data) {

                std::sort(energy_subband_data.begin(), energy_subband_data.end());  // once instead repeated sorting inside function

                // returns quants for a single subbund
                auto conf = clustering_som_anomaly_detector_details::set2multiconf(energy_subband_data, window_sizes, samples, confidence_level);
                std::vector<std::vector<std::vector<T>>> multiquants = std::get<0>(conf);

                //nlohmann::json all_windows_json;
                size_t w_idx = 0;
                for (const auto & window : multiquants) {


                    // TODO compute CDFs per window size

//                    conf_bounds[cl_idx][sb_idx][w_idx][0] = window[0];
//                    conf_bounds[cl_idx][sb_idx][w_idx][1] = window[1];
//                    conf_bounds[cl_idx][sb_idx][w_idx][2] = window[2];
                    cluster_bounds[sb_idx][w_idx][0] = window[0];
                    cluster_bounds[sb_idx][w_idx][1] = window[1];
                    cluster_bounds[sb_idx][w_idx][2] = window[2];

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

                std::vector<std::vector<T>> cdf = {std::get<1>(conf), std::get<2>(conf)};
                cluster_cdfs.push_back(cdf);

//                nlohmann::json subband_json = {
//                    {"_subband", std::to_string(sb_idx)},
//                    {"conf_windows", all_windows_json}
//                };
//                all_subbands_json.push_back(subband_json);
                ++sb_idx;
            }
            conf_bounds.push_back(cluster_bounds);

            // add nodes that fall into cluster
            std::vector<std::vector<T>> cluster_nodes = {};
            for (size_t node_idx = 0; node_idx < assignments.size(); ++node_idx) {
                if (assignments[node_idx] == cl_idx) {
                    cluster_nodes.push_back(nodes_data[node_idx]);
                }
            }
            nodes.push_back(cluster_nodes);

            cdfs.push_back(cluster_cdfs);

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
        assert(nodes.size() == conf_bounds.size());

    }



    void save(const std::string & filename) {


        nlohmann::json model;
        size_t cl_idx = 0;
        for (const auto & cluster_data : conf_bounds) {

            // add subband confidence intervals per window size
            nlohmann::json all_subbands_json;
            size_t sb_idx = 0;
            for (const auto & energy_subband_data : cluster_data) {

                nlohmann::json all_windows_json;
                size_t w_idx = 0;
                for (const auto & window : energy_subband_data) {

                    nlohmann::json window_json = {
                        {"_window_length", std::to_string(window_sizes[w_idx])},
                        {"_window_length_index", std::to_string(w_idx)},
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
                    {"conf_windows", all_windows_json},
                    {"cdf_rv_levels", cdfs[cl_idx][sb_idx][0]},
                    {"cdf_probs",     cdfs[cl_idx][sb_idx][1]}
                };
                all_subbands_json.push_back(subband_json);
                ++sb_idx;
            }

//            // add nodes that fall into cluster
//            //std::vector<std::vector<T>> nodes = {};
//            for (size_t node_idx = 0; node_idx < nodes[cl_idx].size(); ++node_idx) {
//                if (nodes[cl_idx][node_idx] == cl_idx) {
//                    nodes.push_back(nodes_data[node_idx]);
//                }
//            }

            // write cluster json
            nlohmann::json cluster_json  = {
                {"_cluster", std::to_string(cl_idx)},
                {"subbands", all_subbands_json},
                //{"centroid", means[cl_idx]},
                {"som_units", nodes[cl_idx]}

            };
            model.push_back(cluster_json);
            ++cl_idx;
        }


    //    nlohmann::json model = {
    //        {"centroids", centroids},
    //        {"conf_windows", conf_wnds}
    //    };

        std::ofstream outputFile(filename);
        outputFile << std::setw(4) << model << std::endl;
        outputFile.close();

        std::cout << "model written" << std::endl;
    }



    std::vector<T> encode(const std::vector<std::vector<T>> & dataset) {

        auto entries = cluster_entries(dataset, 400, 400);  // debug code, TODO replace with sliding window

        // entropy
        T entropy = 0;
        for (const auto & el : entries) {
            if (el != 0)
                entropy -= el * std::log2(el);
        }


        T similarity = 0; // TODO check behaviour!
        T emd_distance = -1;

        if (entropy < 0.2) {  // locked in cluster  // TODO pass threshold!!

            // select best matching cluster
            T max_share = -1;
            size_t bestcl_idx = entries.size();
            for (size_t cl_idx = 0; cl_idx < entries.size(); ++cl_idx) {
                if (entries[cl_idx] > max_share) {
                    max_share = entries[cl_idx];
                    bestcl_idx = cl_idx;
                }
            }
            assert(bestcl_idx < entries.size());


            // distribution similarity & distance
            similarity = anomaly_score(dataset, 400, 400, bestcl_idx);

            // EMD
            //auto emd = metric::RandomEMD();

            //distr_emd_distance = emd()


            // TODO

        }


        return {similarity};  // TODO replace
    }


private:

    std::vector<T>  // numbers of points that fall into each cluster (by BMUs)
    cluster_entries(
        const std::vector<std::vector<T>> & dataset,
        const size_t pos_idx, // position just after window
        const size_t wnd_size
        // and maybe metric type as template parameter
        )
    {
        //assert(buf_size <= wnd_size);
        assert(wnd_size <= dataset.size());

        size_t num_subbands = conf_bounds[0].size();
        size_t num_windows = window_sizes.size();  // TODO save window sizes to JSON
        size_t num_clusters = nodes.size();

        assert(num_subbands == dataset[pos_idx].size());

        size_t buf_size = window_sizes[num_windows - 1];

        assert(wnd_size >= buf_size);

        metric::Euclidean<T> som_distance;

//        size_t pos_idx = start_idx;

//        std::vector<std::vector<T>> cl_probs = {};  // probability of distribution match for every position of sliding window against each cluster
//        std::vector<std::vector<T>> cl_n_by_BMUs = {}; // numbers of sliding window points that fall into each cluster (by BMUs)

        //std::vector<std::vector<std::vector<T>>> buffer;  // subbands, windows, data
        //buffer.reserve(num_subbands);

//        while ( ( dataset.begin() + start_idx + pos_idx < dataset.end() ) && pos_idx < wnd_size ) {  // slice loop
//        }

        // compute number of points that fall into each cluster by BMUs
        std::vector<T> cl_BMU_matches (num_clusters, 0);
        for (auto val_idx = pos_idx - buf_size; val_idx < pos_idx; ++val_idx) { // reading the sliging window
            T min_dist = std::numeric_limits<T>::max();
            size_t nearest_cluster = num_clusters; // should cause assertion failure if not updated
            for (size_t cl_idx = 0; cl_idx < num_clusters; ++cl_idx) {
                for (size_t node_idx = 0; node_idx < nodes[cl_idx].size(); ++node_idx) {
                    T dist = som_distance(nodes[cl_idx][node_idx], dataset[val_idx]);
                    if (dist < min_dist) {
                        min_dist = dist;
                        nearest_cluster = cl_idx;
                    }
                }
            }
            assert(nearest_cluster < num_clusters);
            ++cl_BMU_matches[nearest_cluster];
        }
        for (size_t cl_idx = 0; cl_idx < num_clusters; ++cl_idx) {
            cl_BMU_matches[cl_idx] /= (T)buf_size;  // normalize
        }

        return cl_BMU_matches;
    }



    T
    anomaly_score(
        const std::vector<std::vector<T>> & dataset,
        const size_t pos_idx, // position just after window
        const size_t wnd_size,
        const size_t cl_idx
        //...
        )
    {

        assert(wnd_size <= dataset.size());

        size_t num_subbands = conf_bounds[0].size();
        size_t num_windows = window_sizes.size();

        assert(num_subbands == dataset[pos_idx].size());

        // compute probabilities of distribution match
        //std::vector<T> avg_prob (model.size(), 0);
        T avg_prob = 0;
        for (size_t sb_idx = 0; sb_idx < num_subbands; ++sb_idx) {

            //std::vector<T> avg_subband_prob (model.size(), 0);
            T avg_subband_prob = 0;
            std::vector<std::vector<T>> subband = {};
            //for (auto wnd_size : window_sizes) {
            for (auto ws_idx = 0; ws_idx < window_sizes.size(); ++ws_idx) {
                auto wnd_size = window_sizes[ws_idx];
                std::vector<T> wnd;
                wnd.reserve(wnd_size);
                for (auto val_idx = pos_idx - wnd_size; val_idx < pos_idx; ++val_idx) { // reading window
                    wnd.push_back(dataset[val_idx][sb_idx]);
                }
                //std::vector<T> avg_wnd_prob (model.size(), 0);
                std::sort(wnd.begin(), wnd.end());
                // here we have window filled
                // loop clusters, compare each sorted element to respective bounds, save for current wndsize
//                for (size_t cl_idx = 0; cl_idx < model.size(); ++cl_idx) {
//                    T wnd_prob = 0;
//                    for (size_t el_idx = 0; el_idx < wnd.size(); el_idx++) {
//                        // compare element to bounds
//                        auto lb = conf_bounds[cl_idx][sb_idx][ws_idx][0][el_idx];
//                        auto rb = conf_bounds[cl_idx][sb_idx][ws_idx][2][el_idx];
//                        if ( (wnd[el_idx] >= lb) && (wnd[el_idx] <= rb) ) {
//                            wnd_prob++;
//                        }
//                    }
//                    wnd_prob /= (T)wnd.size();
//                    avg_subband_prob[cl_idx] += wnd_prob;
//                }
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
                avg_subband_prob += wnd_prob;  // 1 - wnd_prob  // if we need significance of difference, not similarity
                // here we have avg window probability evaluated for each cluster (stored in avg_wnd_prob)
                // avg_subband_prob[cl_idx] += wnd_prob[cl_idx];
                subband.push_back(wnd);
            }
            // subband filled
//            for (size_t cl_idx = 0; cl_idx < model.size(); ++cl_idx) {
//                avg_subband_prob[cl_idx] /= (T)num_windows;
//                avg_prob[cl_idx] += avg_subband_prob[cl_idx];  // adding each subband's summand to overall prob, per cluster
//            }
            avg_subband_prob /= (T)num_windows;
            avg_prob += avg_subband_prob;  // adding each subband's summand to overall prob, per cluster

            // here we have average subband probability for each cluster
            //buffer.push_back(subband);
        }
        // here buffer is filled for the current pos_idx
        // and overall cluster belonging probabilities are summed over al subbands
//        for (size_t cl_idx = 0; cl_idx < model.size(); ++cl_idx) {
//            avg_prob[cl_idx] /= (T)num_subbands;
//        }
        return avg_prob / (T)num_subbands;
        //std::cout << "buffer filled" << std::endl;
        // here we have probs for all clusters computed for the current input data window

//        cl_probs.push_back(avg_prob);

        // TODO
    }


    std::vector<uint32_t> window_sizes = {12, 24, 48, 96, 192, 384};

    std::vector<std::vector<std::vector<std::vector<std::vector<T>>>>> conf_bounds;
    // clusters, subbands, window sizes, bound types (left, med, right), quant values

    std::vector<std::vector<std::vector<T>>> nodes;  // clusters, nodes, node features

    std::vector<std::vector<std::vector<std::vector<T>>>> cdfs;
    // clusters, subbands, CDF vectors (values, probabilities), CDF vector values

}; // class clustering_som_anomaly_detector






}  // namespace metric
