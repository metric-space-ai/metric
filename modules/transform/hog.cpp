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
		blaze::DynamicMatrix<T, blaze::columnMajor> imageColumnMajor = image;
		blaze::DynamicMatrix<T, blaze::columnMajor> dxColumnMajor(image.rows(), image.columns());
		blaze::column(dxColumnMajor, 0) = blaze::column(image, 1);
		for (auto i = 1; i < image.columns() - 1; ++i) {
			blaze::column(dxColumnMajor, i) = blaze::column(image, i + 1) - blaze::column(image, i - 1);
		}
		blaze::column(dxColumnMajor, image.columns() - 1) = -blaze::column(image, image.columns() - 2);

		blaze::DynamicMatrix<T> dx = dxColumnMajor;


		/* Compute dy */
		blaze::DynamicMatrix<T> dy(image.rows(), image.columns());
		blaze::row(dy, 0) = blaze::row(image, 1);
		for (auto i = 1; i < image.rows() - 1; ++i) {
			blaze::row(dy, i) = blaze::row(image, i + 1) - blaze::row(image, i - 1);
		}
		blaze::row(dy, image.rows() - 1) = -blaze::row(image, image.rows() - 2);


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
		//std::cout << "---" << std::endl;
		//std::cout << blaze::min(angle) << std::endl;
		//std::cout << blaze::max(angle) << std::endl;
		angle = blaze::atan(angle);
		//std::cout << " atan" << std::endl;
		//std::cout << blaze::row(angle, 0) << std::endl;

		//std::cout << blaze::min(angle) << std::endl;
		//std::cout << blaze::max(angle) << std::endl;
		angle += M_PI / T(2);
		//std::cout << " norm" << std::endl;
		//std::cout << blaze::row(angle, 0) << std::endl;
		//std::cout << blaze::min(angle) << std::endl;
		//std::cout << blaze::max(angle) << std::endl;
		angle /= M_PI / T(orientations);
		//angle *= 0.9999;//T(1) - 20 * std::numeric_limits<T>::epsilon();

		//std::cout << blaze::min(angle) << std::endl;
		//std::cout << blaze::max(angle) << std::endl;
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

		/* Resulting features vector */
		Vector features(blockNumbers * blockHistogramSize);

		size_t blockCount = 0;
		for (auto blockStartRow = 0; blockStartRow < image.rows() - blockCellSize + 1; blockStartRow += blockCellSize) {
			for (auto blockStartColumn = 0; blockStartColumn < image.columns() - blockCellSize + 1; blockStartColumn += blockCellSize) {

				/* Compute block histogram */
				blaze::DynamicMatrix<T> blockHistogram = blaze::zero<T>(blockSize * blockSize, orientations);
				for (auto i = 0; i < blockCellSize; ++i) {
					for (auto j = 0; j < blockCellSize; ++j) {
						const size_t cellNumber = (i / cellSize) * blockSize + (j / cellSize);
						T angleBin = std::floor(angle(blockStartRow + i, blockStartColumn + j));
						if (angleBin == orientations) {
							--angleBin;
						}
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
				//std::cout << blockCount << std::endl;
			}
		}

		//std:: cout << "result" << std::endl;
		//std::cout << blaze::subvector(features, blockCount * blockHistogramSize + 1 * orientations, orientations) << std::endl;
		//std::cout << blaze::subvector(features, 0, features.size()) << std::endl;
		//std::cout << blaze::trans(features) << std::endl;
		return features;
	}

	template<typename T>
	blaze::DynamicMatrix<T> HOG<T>::groundDistance(blaze::DynamicMatrix<T> image)
	{
		T threshold = 0;
		size_t block_stride = 0;
		size_t blocks_per_image = floor((img_size./cell_size - blockSize)./(blockSize - block_stride) + 1);
		size_t n_hog_bins = prod([blocks_per_image, blockSize, orientations]);

		/* Spatial distance matrix */
		size_t spatial_dist_mat = spatial_dist(n_hog_bins,orientations, blockSize, blocks_per_image);
		if (threshold != 0) {
			spatial_dist_mat(spatial_dist_mat > threshold) = threshold;
		} else {
			spatial_dist_mat = repelem(spatial_dist_mat, orientations, orientations);
		}

		/* Orientations distance matrix */
		size_t orient_dist_cell = rotation_dist(orientations,signed);
		size_t orient_dist_mat = repmat(orient_dist_cell,n_hog_bins/orientations);

		/* Total ground distance matrix */
		size_t ground_dist = rotation_cost * orient_dist_mat + move_cost * spatial_dist_mat;

		return blaze::DynamicMatrix<T>();
	}


}