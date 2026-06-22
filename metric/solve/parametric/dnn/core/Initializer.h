#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <metric/numeric/Math.h>

namespace mtrc::solve::parametric::dnn {
template <typename Scalar> class Initializer {
  protected:
	using ColumnMatrix = mtrc::numeric::DynamicMatrix<Scalar, mtrc::numeric::columnMajor>;
	using RowVector = mtrc::numeric::DynamicVector<Scalar, mtrc::numeric::rowVector>;

  public:
	virtual void init(const size_t rows, const size_t columns, ColumnMatrix &matrix) = 0;

	virtual void init(const size_t size, RowVector &matrix) = 0;
};
} // namespace mtrc::solve::parametric::dnn
#endif // INITIALIZER_H
