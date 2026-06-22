#include "Redif.hpp"
#include <iostream>

namespace mtrc {

template <typename Tv, class Metric>
Redif<Tv, Metric>::Redif(const std::vector<std::vector<Tv>> &trainData, size_t nNeighbors /* = 10 */,
						 size_t nIter /* = 15 */, Metric metric /*= Metric() */
)
{
	this->nNeighbors = nNeighbors;
	this->metric = metric;
	xTrain = mtrc::numeric::DynamicMatrix<Tv>(trainData.size(), trainData[0].size());
	for (size_t i = 0; i < xTrain.rows(); i++)
		for (size_t j = 0; j < xTrain.columns(); j++)
			xTrain(i, j) = trainData[i][j];

	xTrainEncoded = trainModel(nIter);
}

template <typename Tv, class Metric> mtrc::numeric::DynamicMatrix<Tv> Redif<Tv, Metric>::trainModel(size_t nIter)
{
	mtrc::numeric::DynamicMatrix<Tv> xTrainDe(this->xTrain);
	const size_t n = xTrainDe.rows();

	/// Iterative backward Diffusion process
	for (size_t i = 0; i < nIter; i++) {
		mtrc::numeric::DynamicMatrix<Tv> localDistMat = getLocalDistMatrix(xTrainDe);
		mtrc::numeric::DynamicMatrix<Tv> graphLap = calcWeightedGraphLaplacian(localDistMat);

		this->LArray.push_back(graphLap);
		mtrc::numeric::DynamicMatrix<Tv> D(n, n, 0);
		for (size_t i = 0; i < D.rows(); i++)
			D(i, i) = graphLap(i, i);
		mtrc::numeric::DynamicMatrix<Tv> invLeft(n, n, 0);
		invLeft = mtrc::numeric::inv(D + 0.25 * graphLap);
		xTrainDe = invLeft * (D * xTrainDe);
	}

	return xTrainDe;
}

template <typename Tv, class Metric>
mtrc::numeric::DynamicMatrix<Tv>
Redif<Tv, Metric>::getLocalDistMatrix(const mtrc::numeric::DynamicMatrix<Tv> &dataSample)
{
	size_t n = dataSample.rows();

	mtrc::numeric::DynamicMatrix<size_t> ixx(nNeighbors, n, 0);
	for (size_t i = 0; i < nNeighbors; i++)
		for (size_t j = 0; j < n; j++)
			ixx(i, j) = j;

	mtrc::numeric::DynamicMatrix<Tv> dist(n, n, 0);
	for (size_t i = 0; i < n; i++)
		for (size_t j = 0; j < n; j++) {
			if (i == j) {
				dist(i, j) = 0;
				continue;
			}
			mtrc::numeric::DynamicVector<Tv, mtrc::numeric::rowVector> rowi = row(dataSample, i);
			mtrc::numeric::DynamicVector<Tv, mtrc::numeric::rowVector> rowj = row(dataSample, j);

			std::vector<Tv> veci, vecj;
			for (size_t k = 0; k < rowi.size(); ++k) {
				veci.push_back(rowi[k]);
				vecj.push_back(rowj[k]);
			}

			dist(i, j) = metric(veci, vecj);
		}

	mtrc::numeric::DynamicMatrix<size_t> knnMat(nNeighbors, n, 0);
	mtrc::numeric::DynamicMatrix<Tv> knnDistMat(nNeighbors, n, 0);

	for (size_t i = 0; i < n; i++) {
		mtrc::numeric::DynamicVector<size_t> tempIdx(n);
		mtrc::numeric::DynamicVector<Tv> tempDist(n);
		for (size_t j = 0; j < n; j++) {
			tempIdx[j] = j;
			tempDist[j] = dist(j, i);
		}
		Quicksort(tempIdx, tempDist, 0, n - 1);
		for (size_t j = 0; j < nNeighbors; j++) {
			knnMat(j, i) = tempIdx[j + 1];
			knnDistMat(j, i) = tempDist[j + 1];
		}
	}

	mtrc::numeric::DynamicMatrix<Tv> localDist(n, n, 0);

	for (size_t i = 0; i < nNeighbors; i++)
		for (size_t j = 0; j < n; j++)
			localDist(ixx(i, j), knnMat(i, j)) = knnDistMat(i, j);

	mtrc::numeric::DynamicMatrix<Tv> retLocalDist(n, n, 0);
	for (size_t i = 0; i < n; i++)
		for (size_t j = 0; j < n; j++)
			retLocalDist(i, j) = (localDist(i, j) > localDist(j, i)) ? localDist(i, j) : localDist(j, i);

	return retLocalDist;
}

template <typename Tv, class Metric>
mtrc::numeric::DynamicMatrix<Tv>
Redif<Tv, Metric>::calcWeightedGraphLaplacian(const mtrc::numeric::DynamicMatrix<Tv> &localDist)
{
	size_t n = localDist.rows();

	/// Degree matrix
	mtrc::numeric::DynamicMatrix<Tv> D(n, n, 0);
	for (size_t i = 0; i < D.rows(); i++) {
		double sum = 0;
		for (size_t j = 0; j < n; j++)
			sum += localDist(i, j);
		if (sum == 0)
			sum = 1;
		D(i, i) = 1 / sum;
	}

	mtrc::numeric::DynamicMatrix<Tv> A(localDist.rows(), localDist.columns(), 0);
	A = D * localDist * D / n;

	mtrc::numeric::DynamicMatrix<Tv> D1(n, n, 0);
	for (size_t i = 0; i < D.rows(); i++) {
		double sum = 0;
		for (size_t j = 0; j < n; j++)
			sum += A(i, j);
		if (sum == 0)
			sum = 1 / n;
		D1(i, i) = sum;
	}

	mtrc::numeric::DynamicMatrix<Tv> graphLap(n, n, 0);
	graphLap = D1 - A;

	return graphLap;
}

template <typename Tv, class Metric>
std::vector<std::vector<Tv>> Redif<Tv, Metric>::encode(const std::vector<std::vector<Tv>> &x)
{
	mtrc::numeric::DynamicMatrix<Tv> x_as_matrix(x.size(), x[0].size(), 0);

	for (size_t i = 0; i < x_as_matrix.rows(); i++)
		for (size_t j = 0; j < x_as_matrix.columns(); j++)
			x_as_matrix(i, j) = x[i][j];

	auto encoded_data = encode(x_as_matrix);

	std::vector<std::vector<Tv>> encoded_data_as_vectors(encoded_data.rows(), std::vector<Tv>(encoded_data.columns()));
	for (size_t i = 0; i < encoded_data.rows(); i++)
		for (size_t j = 0; j < encoded_data.columns(); j++)
			encoded_data_as_vectors[i][j] = encoded_data(i, j);

	return encoded_data_as_vectors;
}

template <typename Tv, class Metric>
mtrc::numeric::DynamicMatrix<Tv> Redif<Tv, Metric>::encode(const mtrc::numeric::DynamicMatrix<Tv> &x)
{
	size_t nTrain = xTrain.rows();
	size_t nX = x.rows();
	size_t nIter = LArray.size();
	/// Find nearest Neighbors of each record of x in xTrain
	size_t l = 1; /// Number of nearest neighbors
	mtrc::numeric::DynamicVector<size_t> l_idx = mtrc::numeric::DynamicVector<Tv>(nX);
	size_t k_check = sqrt(nTrain); /// Parameter for checking model on the record

	/// Compute distances
	for (size_t i = 0; i < nX; i++) {
		mtrc::numeric::DynamicMatrix<Tv> distTotal(nTrain + 1, nTrain + 1, 0);
		for (size_t k = 0; k < nTrain + 1; k++)
			for (size_t j = 0; j < nTrain + 1; j++) {
				if (k == j)
					continue;

				mtrc::numeric::DynamicVector<Tv, mtrc::numeric::rowVector> rowk;
				mtrc::numeric::DynamicVector<Tv, mtrc::numeric::rowVector> rowj;

				if (k < nTrain)
					rowk = row(xTrain, k);
				else
					rowk = row(x, i);
				if (j < nTrain)
					rowj = row(xTrain, j);
				else
					rowj = row(x, i);

				std::vector<Tv> veck, vecj;
				for (size_t s = 0; s < rowk.size(); ++s) {
					veck.push_back(rowk[s]);
					vecj.push_back(rowj[s]);
				}

				distTotal(k, j) = metric(veck, vecj);
			}

		mtrc::numeric::DynamicVector<Tv, mtrc::numeric::columnVector> dist =
			column(distTotal, distTotal.columns() - 1);

		mtrc::numeric::DynamicVector<size_t> idxVec(dist.size());
		for (size_t k = 0; k < idxVec.size(); k++)
			idxVec[k] = k;
		Quicksort(idxVec, dist, 0, idxVec.size() - 1);

		double oneDist = 0;
		if (dist[0] == dist[1]) {
			size_t j = 0;
			j = (idxVec[0] > idxVec[1]) ? 0 : 1;
			if (j == 0) {
				l_idx[i] = idxVec[1];
				oneDist = dist[1];
			} else {
				l_idx[i] = idxVec[0];
				oneDist = dist[0];
			}
		} else {
			l_idx[i] = idxVec[1];
			oneDist = dist[1];
		}
		/// Check if Model is able to denoise Test Posize_t
		mtrc::numeric::DynamicVector<Tv, mtrc::numeric::columnVector> temp = column(distTotal, l_idx[i]);
		for (size_t k = 0; k < idxVec.size(); k++)
			idxVec[k] = k;
		Quicksort(idxVec, temp, 0, idxVec.size() - 1);
		if (oneDist > temp[k_check - 1])
			std::cout << "Error. Testposize_t of Index " << i << " cannot be denoised based on underlying model.\n";
	}

	mtrc::numeric::DynamicMatrix<Tv> xEncoded = xTrain;
	/// Iterative backward Diffusion process
	for (size_t i = 0; i < nX; i++)
		for (size_t j = 0; j < xEncoded.columns(); j++)
			xEncoded(l_idx[i], j) = x(i, j);

	for (size_t i = 0; i < nIter; i++) {
		mtrc::numeric::DynamicMatrix<Tv> D(nTrain, nTrain, 0);
		for (size_t k = 0; k < D.rows(); k++)
			D(k, k) = LArray[i](k, k);

		xEncoded = mtrc::numeric::inv(D + 0.25 * LArray[i]) * (D * xEncoded);
	}

	mtrc::numeric::DynamicMatrix<Tv> xEncodedRes(nX, xEncoded.columns());

	for (size_t i = 0; i < nX; i++) {
		for (size_t j = 0; j < xEncoded.columns(); j++) {
			xEncodedRes(i, j) = xEncoded(l_idx[i], j);
		}
	}

	return xEncodedRes;
}

template <typename Tv, class Metric>
std::vector<std::vector<Tv>> Redif<Tv, Metric>::decode(const std::vector<std::vector<Tv>> &xEncoded)
{
	mtrc::numeric::DynamicMatrix<Tv> x_as_matrix(xEncoded.size(), xEncoded[0].size(), 0);
	for (size_t i = 0; i < x_as_matrix.rows(); i++)
		for (size_t j = 0; j < x_as_matrix.columns(); j++)
			x_as_matrix(i, j) = xEncoded[i][j];

	auto decoded_data = decode(x_as_matrix);

	std::vector<std::vector<Tv>> decoded_data_as_vectors(decoded_data.rows(), std::vector<Tv>(decoded_data.columns()));
	for (size_t i = 0; i < decoded_data.rows(); i++)
		for (size_t j = 0; j < decoded_data.columns(); j++)
			decoded_data_as_vectors[i][j] = decoded_data(i, j);

	return decoded_data_as_vectors;
}

template <typename Tv, class Metric>
mtrc::numeric::DynamicMatrix<Tv> Redif<Tv, Metric>::decode(const mtrc::numeric::DynamicMatrix<Tv> &xEncoded)

{
	size_t nX = xEncoded.rows();
	size_t nTrain = xTrain.rows();
	mtrc::numeric::DynamicVector<size_t> l_idx = mtrc::numeric::DynamicVector<size_t>(nX);

	/// Compute distances
	for (size_t i = 0; i < nX; i++) {
		mtrc::numeric::DynamicVector<Tv, mtrc::numeric::rowVector> rowi;
		rowi = row(xEncoded, i);
		std::vector<Tv> veci;
		for (size_t s = 0; s < rowi.size(); ++s) {
			veci.push_back(rowi[s]);
		}

		Tv closest_dist = -1;
		for (size_t k = 0; k < nTrain; k++) {
			mtrc::numeric::DynamicVector<Tv, mtrc::numeric::rowVector> rowi, rowk;
			rowk = row(xTrainEncoded, k);
			std::vector<Tv> veck;
			for (size_t s = 0; s < rowk.size(); ++s) {
				veck.push_back(rowk[s]);
			}

			Tv dist = metric(veci, veck);
			if (closest_dist < 0 || closest_dist > dist) {
				l_idx[i] = k;
				closest_dist = dist;
			}
		}
	}

	size_t n = xTrainEncoded.rows();
	size_t nIter = LArray.size();
	for (size_t i = 0; i < l_idx.size(); i++)
		for (size_t j = 0; j < xTrainEncoded.columns(); j++)
			xTrainEncoded(l_idx[i], j) = xEncoded(i, j);

	for (size_t i = 0; i < nIter; i++) {
		mtrc::numeric::DynamicMatrix<Tv> temp = LArray[nIter - i - 1];
		mtrc::numeric::DynamicMatrix<Tv> D(n, n, 0);
		for (size_t k = 0; k < n; k++)
			D(k, k) = temp(k, k);
		xTrainEncoded = mtrc::numeric::inv(D) * (D + 0.25 * LArray[nIter - i - 1]) * xTrainEncoded;
	}
	mtrc::numeric::DynamicMatrix<Tv> xDecoded =
		mtrc::numeric::DynamicMatrix<Tv>(l_idx.size(), xTrainEncoded.columns(), 0);
	for (size_t i = 0; i < xDecoded.rows(); i++)
		for (size_t j = 0; j < xDecoded.columns(); j++)
			xDecoded(i, j) = xTrainEncoded(l_idx[i], j);

	return xDecoded;
}

template <typename Tv, class Metric>
template <bool flag>
size_t Redif<Tv, Metric>::partition(mtrc::numeric::DynamicVector<size_t> &idx,
									mtrc::numeric::DynamicVector<Tv, flag> &val, size_t start, size_t end)
{
	double pivot = val[end];

	size_t P_index = start;
	size_t i;
	size_t idxTemp;
	double valTemp;

	for (i = start; i < end; i++) {
		if (val[i] <= pivot) {
			valTemp = val[i];
			val[i] = val[P_index];
			val[P_index] = valTemp;

			idxTemp = idx[i];
			idx[i] = idx[P_index];
			idx[P_index] = idxTemp;

			P_index++;
		}
	}

	valTemp = val[end];
	val[end] = val[P_index];
	val[P_index] = valTemp;

	idxTemp = idx[end];
	idx[end] = idx[P_index];
	idx[P_index] = idxTemp;

	return P_index;
}

template <typename Tv, class Metric>
template <bool flag>
void Redif<Tv, Metric>::Quicksort(mtrc::numeric::DynamicVector<size_t> &idx,
								  mtrc::numeric::DynamicVector<Tv, flag> &data, size_t start, size_t end)
{
	if (start < end) {
		int P_index = partition(idx, data, start, end);
		if (P_index != 0)
			Quicksort(idx, data, start, P_index - 1);
		Quicksort(idx, data, P_index + 1, end);
	}
}

} // namespace mtrc
