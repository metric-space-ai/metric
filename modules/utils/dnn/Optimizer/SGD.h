#ifndef OPTIMIZER_SGD_H_
#define OPTIMIZER_SGD_H_

#include <Eigen/Core>
#include "../Optimizer.h"

namespace dnn
{


///
/// \ingroup Optimizers
///
/// The Stochastic Gradient Descent (SGD) algorithm
///
template <typename Scalar>
class SGD: public Optimizer<Scalar>
{
    private:
        typedef Eigen::Matrix<Scalar, Eigen::Dynamic, 1> Vector;
        typedef Vector::ConstAlignedMapType ConstAlignedMapVec;
        typedef Vector::AlignedMapType AlignedMapVec;

    public:
        Scalar m_lrate;
        Scalar m_decay;

        SGD() :
            m_lrate(Scalar(0.01)), m_decay(Scalar(0))
        {}

        void update(ConstAlignedMapVec& dvec, AlignedMapVec& vec)
        {
            vec.noalias() -= m_lrate * (dvec + m_decay * vec);
        }
};


} // namespace dnn


#endif /* OPTIMIZER_SGD_H_ */
