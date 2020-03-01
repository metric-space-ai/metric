#include "hog.hpp"


namespace metric {


	template<typename T>
	HOG<T>::HOG(const size_t orientations, const size_t cellSize, const size_t blockSize) :
																				orientations(orientations),
																				cellSize(cellSize),
																				blockSize(blockSize)
	{
	}


	template<typename T>
	typename HOG<T>::Vector HOG<T>::compute(const HOG::Matrix &image) const
	{
		/* Compute dx */
		blaze::DynamicMatrix<T> dx(image.rows(), image.columns());
		blaze::row(dx, 0) = blaze::row(image, 1);
		for (auto i = 0; i < image.rows() - 2; ++i) {
			blaze::row(dx, i + 1) = blaze::row(image, i + 2) - blaze::row(image, i);
		}
		blaze::row(dx, image.rows() - 1) = -blaze::row(image, image.rows() - 2);


		/* Compute dy */
		blaze::DynamicMatrix<T, blaze::columnMajor> imageColumnMajor = image;
		blaze::DynamicMatrix<T, blaze::columnMajor> dyColumnMajor(image.rows(), image.columns());
		blaze::column(dyColumnMajor, 0) = blaze::column(image, 1);
		for (auto i = 0; i < image.columns() - 2; ++i) {
			blaze::column(dyColumnMajor, i + 1) = blaze::column(image, i + 2) - blaze::column(image, i);
		}
		blaze::column(dyColumnMajor, image.columns() - 1) = -blaze::column(image, image.columns() - 2);

		blaze::DynamicMatrix<T> dy = dyColumnMajor;


		/* Compute magnitudes */
		Matrix dx2 = blaze::invsqrt(dx);
		Matrix dy2 = blaze::invsqrt(dy);

		Matrix magnitude = blaze::sqrt(dx2 + dy2);


		/* Compute angle */
		Matrix angle(image.rows(), image.columns());
		for (auto i = 0; i < image.rows(); ++i) {
			blaze::row(angle, i) = blaze::row(dx, i) / blaze::row(dy, i);
		}

		angle = blaze::atan(angle);


		const size_t blockCell = orientations * blockSize;

		blaze::DynamicVector<T> histograms = blaze::zero<T>((image.rows() / blockSize) * (image.columns() / blockSize) *
																std::pow(blockSize * orientations, 2));
		size_t blockCount = 0;
		const size_t blockCellSize = blockSize * cellSize;
		for (auto blockStartRow = 0; blockStartRow < image.rows() - blockCellSize; blockStartRow += blockCellSize) {
			for (auto blockStartColumn = 0; blockStartColumn < image.columns() - blockCellSize; blockStartColumn += blockCellSize) {

				/* Compute block histogram */
				blaze::DynamicMatrix<T> histogram(blockSize * blockSize, orientations);
				for (auto i = blockStartRow; i < blockCellSize; ++i) {
					for (auto j = blockStartColumn; j < blockCellSize; ++j) {
						size_t cellNumber = ((i - blockStartRow) / cellSize) *  ((j - blockStartColumn) / cellSize);
						size_t angleBin = std::floor(angle(i, j) / orientations);
						histogram(cellNumber, angleBin) += magnitude(i, j);
					}
				}

				T norm = blaze::sum(blaze::invsqrt(histogram));
				histogram /= norm;

				for (size_t k = 0; k < histogram.rows(); ++k) {
					blaze::subvector(histograms, blockCount * blockSize + k * orientations, orientations) =
							blaze::trans(blaze::row(histogram, k));
				}
			}
		}

		return histograms;
	}



}