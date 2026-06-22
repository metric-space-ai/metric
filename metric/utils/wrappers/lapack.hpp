#ifndef METRIC_UTILS_WRAPPERS_LAPACK_HPP
#define METRIC_UTILS_WRAPPERS_LAPACK_HPP

#include <metric/numeric/Math.h>

extern "C" {
extern void dsygv_(int *itype, char *jobz, char *uplo, int *n, double *A, int *LDA, double *B, int *LDB, double *W,
				   double *WORK, int *LWORK, int *INFO);
}

namespace mtrc {

inline void dsygv(int itype, char jobz, char uplo, int n, double *A, int lda, double *B, int ldb, double *W,
				  double *work, int lwork, int info)
{
	dsygv_(&itype, &jobz, &uplo, &n, A, &lda, B, &ldb, W, work, &lwork, &info);
}

template <typename MT, typename VT>
void sygv(mtrc::numeric::DynamicMatrix<MT, mtrc::numeric::rowMajor> A,
		  mtrc::numeric::DynamicMatrix<MT, mtrc::numeric::rowMajor> B,
		  mtrc::numeric::DynamicVector<VT, mtrc::numeric::columnVector> &w)
{
	w.resize(A.rows());
	int lwork = 3 * A.rows() - 1;
	std::vector<double> work(lwork);
	int info;

	dsygv(1, 'N', 'U', A.rows(), A.data(), A.spacing(), B.data(), B.spacing(), w.data(), work.data(), lwork, info);
}
} // namespace mtrc

#endif // METRIC_UTILS_WRAPPERS_LAPACK_HPP
