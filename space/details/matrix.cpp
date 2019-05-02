#include "matrix.hpp"
#include "3dparty/blaze/Math.h"

/*** standard euclidian (L2) Metric ***/
template <typename Container>
struct L2_Metric_STL;

template <typename Container>
struct L2_Metric_STL
{
    typedef typename Container::value_type result_type;
    static_assert(std::is_floating_point<result_type>::value,
                  "T must be a float type");

    result_type operator()(const Container &a, const Container &b) const
    {
        result_type sum = 0;
        for (auto it1 = a.begin(), it2 = b.begin();
             it1 != a.end() || it2 != b.end(); ++it1, ++it2)
        {
            sum += (*it1 - *it2) * (*it1 - *it2);
        }
        return std::sqrt(sum);
    }
};

template <typename recType, typename Metric = L2_Metric_STL<recType>, typename distType = float>
class Matrix
{
  private:
    /*** Properties ***/
    Metric metric_;
    blaze::SymmetricMatrix<blaze::DynamicMatrix<distType>> D_;
    std::vector<recType> data_;

  public:
    /*** Constructors ***/
    Matrix(Metric d = Metric());                                // empty Matrix
    Matrix(const recType &p, Metric d = Metric());              // Matrix with one data record
    Matrix(const std::vector<recType> &p, Metric d = Metric()); // with a vector of data records
    ~Matrix();                                                  // destuctor

    /*** Access Operations ***/
    bool append(const recType &p);                              // append data record into the Matix
    bool append_if(const recType &p, distType treshold);        // append data record into the Matrix only if distance bigger than a treshold
    bool append(const std::vector<recType> &p);                 // append a vector of records into the Matrix
    bool append_if(const std::vector<recType> &p,distType treshold); // append a vector of records into the Matrix only if distance bigger than a treshold
    bool erase(size_t id);                                      // erase data record from Matrix by ID
    bool set(size_t id, const recType &p);                      // change data record by ID
    recType operator[](size_t id);                              // access a data record by ID
    distType operator()(size_t i, size_t j);                    // access a distance by two IDs
};

/*** constructor: empty Matrix **/
template <typename recType, typename Metric, typename distType>
Matrix<recType, Metric,distType>::Matrix(Metric d)
    : metric_(d), data_()
{
}

/*** constructor: with a single data record **/
template <typename recType, typename Metric, typename distType>
Matrix<recType, Metric,distType>::Matrix(const recType &p, Metric d)
    : metric_(d), data_(p)
{
}

/*** constructor: with a vector data records **/
template <typename recType, typename Metric, typename distType>
Matrix<recType, Metric,distType>::Matrix(const std::vector<recType> &p, Metric d)
    : metric_(d), data_(p)
{
    for (size_t i = 0; i < D_.rows(); ++i)
    {
        D_(i, i) = 0;
        for (size_t j = i + 1; j < D_.rows(); ++j)
        {
            D_(i, j) = metric_(p[i], p[j]);
        }
    }
}


template <typename recType, typename Metric, typename distType>
distType
Matrix<recType, Metric,distType>::operator()(size_t i, size_t j){
return (D_(i,j));
}

template <typename recType, typename Metric, typename distType>
recType
Matrix<recType, Metric,distType>::operator[](size_t id){
return (data_(id));
}
