#include "hog.hpp"


namespace metric {


	template<typename T>
	HOG<T>::HOG(const size_t orientations, const size_t cellSize, const size_t blockSize) :
																				orientations(orientations),
																				cellSize(cellSize),
																				blockSize(blockSize)
	{
		assert(orientations > 2);
		assert(cellSize > 0);
		assert(blockSize > 0);
		//assert(cellSize % 2 == 0);
	}


	template<typename T>
	typename HOG<T>::Vector HOG<T>::encode(const HOG::Matrix &image) const
	{
		assert(image.rows() > 0);
		assert(image.columns() > 0);
		assert(image.rows() % blockSize * cellSize == 0);
		assert(image.columns() % blockSize * cellSize == 0);

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


		/* Compute magnitude */
		Matrix dx2 = blaze::pow(dx, 2);
		Matrix dy2 = blaze::pow(dy, 2);

		Matrix magnitude = blaze::sqrt(dx2 + dy2);


		/* Compute angle */
		Matrix angle(image.rows(), image.columns());
		dx += 2 * std::numeric_limits<T>::epsilon();
		for (auto i = 0; i < image.rows(); ++i) {
			blaze::row(angle, i) = blaze::row(dy, i) / blaze::row(dx, i);
		}

		//std::cout << " dev" << std::endl;
		//std::cout << blaze::row(angle, 0) << std::endl;
		angle = blaze::atan(angle);
		//std::cout << " atan" << std::endl;
		//std::cout << blaze::row(angle, 0) << std::endl;

		angle += M_PI / T(2);
		//std::cout << " norm" << std::endl;
		//std::cout << blaze::row(angle, 0) << std::endl;
		angle /= M_PI / T(orientations);
		//std::cout << " norm" << std::endl;
		//std::cout << blaze::row(angle, 0) << std::endl;
		//std::cout << angle << std::endl;



		/* Define HOG features */

		/* Length of histogram of block */
		const size_t blockHistogramSize = blockSize * blockSize * orientations;

		/* Size of block in pixels */
		const size_t blockCellSize = blockSize * cellSize;

		/* Number of blocks in image */
		const size_t blockNumbers = (image.rows() / blockCellSize) * (image.columns() / blockCellSize);

		/* Resulting fetures vector */
		Vector features(blockNumbers * blockHistogramSize);

		size_t blockCount = 0;
		for (auto blockStartRow = 0; blockStartRow < image.rows() - blockCellSize + 1; blockStartRow += blockCellSize) {
			for (auto blockStartColumn = 0; blockStartColumn < image.columns() - blockCellSize + 1; blockStartColumn += blockCellSize) {

				/* Compute block histogram */
				blaze::DynamicMatrix<T> blockHistogram = blaze::zero<T>(blockSize * blockSize, orientations);
				for (auto i = 0; i < blockCellSize; ++i) {
					for (auto j = 0; j < blockCellSize; ++j) {
						const size_t cellNumber = (i / cellSize) * blockSize + (j / cellSize);
						const T angleBin = std::floor(angle(blockStartRow + i, blockStartColumn + j));
						//std::cout << i << " " << j << " " << cellNumber << " " << angleBin << std::endl;
						blockHistogram(cellNumber, angleBin) += magnitude(blockStartRow + i, blockStartColumn + j);

					}
				}

				/* Normalize block */
				T norm = std::sqrt(blaze::sum(blaze::pow(blockHistogram, 2)) + 2 * std::numeric_limits<T>::epsilon());

				//std::cout << norm << std::endl;
				blockHistogram /= norm;
				//std::cout << blockHistogram << std::endl;

				for (size_t k = 0; k < blockHistogram.rows(); ++k) {
					//std::cout << blockCount * blockHistogramSize + k * orientations << " " << orientations << std::endl;
					blaze::subvector(features, blockCount * blockHistogramSize + k * orientations, orientations) =
							blaze::trans(blaze::row(blockHistogram, k));

				//std::cout <<  blaze::row(blockHistogram, k) << std::endl;


					//std::cout << blaze::subvector(features, blockCount * blockHistogramSize + k * orientations, orientations) << std::endl;

				}
				//std::cout << blaze::trans(features) << std::endl;
				++blockCount;
				std::cout << blockCount << std::endl;
			}
		}

		//std:: cout << "result" << std::endl;
		//std::cout << blaze::subvector(features, blockCount * blockHistogramSize + 1 * orientations, orientations) << std::endl;
		//std::cout << blaze::subvector(features, 0, features.size()) << std::endl;
		//std::cout << blaze::trans(features) << std::endl;
		return features;
	}



}