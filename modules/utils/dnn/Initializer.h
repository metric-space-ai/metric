#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <blaze/Math.h>


namespace metric::dnn {
	template<typename Scalar>
	class Initializer
	{
	protected:
		using ColumnMatrix = blaze::DynamicMatrix<Scalar, blaze::columnMajor>;
		using RowVector = blaze::DynamicVector<Scalar, blaze::rowVector>;

	public:
		virtual void init(const size_t rows, const size_t columns, ColumnMatrix &matrix) = 0;

		virtual void init(const size_t size, RowVector &matrix) = 0;
	};
}
#endif // INITIALIZER_H
