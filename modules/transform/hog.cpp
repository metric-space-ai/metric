#include "hog.hpp"


namespace metric {


	template<typename T>
	HOG<T>::HOG(const size_t orientations, const size_t cellSize, const size_t blockSize) :
																				orientations(orientations),
																				cellSize(cellSize),
																				blockSize(blockSize)
	{
		assert(orientations > 1);
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
	blaze::DynamicMatrix<T> HOG<T>::groundDistance(blaze::DynamicMatrix<T> image, T rotation_cost, T move_cost)
	{
		T threshold = 0;
		bool isSigned = false;

		size_t block_stride = 0;
		//floor((img_size./cell_size - blockSize)./(blockSize - block_stride) + 1);
		size_t blocks_per_image_rows = (image.rows() / T(cellSize) - blockSize) / (blockSize - block_stride) + 1;
		size_t blocks_per_image_columns = (image.columns() / T(cellSize) - blockSize) / (blockSize - block_stride) + 1;
		size_t n_hog_bins = blocks_per_image_rows * blocks_per_image_columns * blockSize * blockSize * orientations;


		/* Spatial distance matrix */
		Matrix spatial_dist_mat = spatial_dist(n_hog_bins, orientations, blockSize, blocks_per_image_rows, blocks_per_image_columns);
		//if (threshold != 0) {
		//	spatial_dist_mat(spatial_dist_mat > threshold) = threshold;
		//}

		spatial_dist_mat.resize(spatial_dist_mat.rows() * orientations, spatial_dist_mat.columns() * orientations);
		for (auto i = 0; i < spatial_dist_mat.rows(); ++i) {
			for (auto j = 0; j < spatial_dist_mat.columns(); ++j) {
				spatial_dist_mat(i, j) = spatial_dist_mat(i / orientations, j / orientations);
			}
		}

		/* Orientations distance matrix */
		Matrix orient_dist_cell = rotation_dist(orientations, isSigned);

		Matrix orient_dist_mat(orient_dist_cell.rows(), orient_dist_cell.columns());
		size_t scale = n_hog_bins / orientations;
		for (auto i = 0; i < orient_dist_mat.rows(); ++i) {
			for (auto j = 0; j < orient_dist_mat.columns(); ++j) {
				orient_dist_mat(i, j) = orient_dist_cell(i % orient_dist_cell.rows(), j % orient_dist_cell.columns());
			}
		}

		/* Total ground distance matrix */
		blaze::DynamicMatrix<T> ground_dist = rotation_cost * orient_dist_mat + move_cost * spatial_dist_mat;

		return ground_dist;
	}

	template<typename T>
	typename HOG<T>::Matrix
	HOG<T>::spatial_dist(size_t n_hog_bins, size_t orientations, size_t blockSize, size_t blocks_per_image_rows,
	                     size_t blocks_per_image_columns)
	{
		Vector cell_i_vect = blaze::zero<T>(n_hog_bins / orientations);
		Vector cell_j_vect = cell_i_vect;

		size_t idx = 0;
		for (size_t b_i = 1; b_i <= blocks_per_image_rows; ++b_i) {
			for (size_t b_j = 1; b_j <= blocks_per_image_columns; ++b_j) {
				for (size_t cb_j = 1; cb_j <= blockSize; ++cb_j) {
					for (size_t cb_i = 1; cb_i <= blockSize; ++cb_i) {
						size_t cell_j;
						if (b_j == 1) {
							cell_j = cb_j;
						} else {
							cell_j = cb_j + (b_j - 1);
						}

						size_t cell_i;
						if (b_i == 1) {
							cell_i = cb_i;
						} else {
							cell_i = cb_i + (b_i - 1);
						}

						cell_i_vect[idx] = cell_i;
						cell_j_vect[idx] = cell_j;
						idx = idx + 1;
					}
				}
			}
		}


		blaze::SymmetricMatrix<Matrix> spatial_dist_mat(cell_i_vect.size());
		for (size_t i = 0; i < spatial_dist_mat.rows(); ++i) {
			for (size_t j = i + 1; j < spatial_dist_mat.columns(); ++j) {
				spatial_dist_mat(i, j) = std::abs(cell_i_vect[i] - cell_i_vect[j]) + std::abs(cell_j_vect[i] - cell_j_vect[j]);
			}
		}

		return spatial_dist_mat;
	}

	template<typename T>
	typename HOG<T>::Matrix HOG<T>::rotation_dist(size_t orientations, bool isSigned)
	{
		size_t max_angle;
		if (isSigned == 1) {
			max_angle = 360;
		} else {
			max_angle = 180;
		}

		Vector orients_vect(orientations);
		for (size_t i = 0; i < orientations; ++i) {
			orients_vect[i] = max_angle * (T(1) - T(1) / orientations) / orientations * i;
		}


		blaze::SymmetricMatrix<Matrix> diff_mat(orientations);
		for (size_t i = 0; i < orients_vect.size(); ++i) {
			for (size_t j = i + 1; j < orients_vect.size(); ++j) {
				T normDeg = std::fmod(orients_vect[i] - orients_vect[j], max_angle);
				diff_mat(i, j) = std::min(max_angle - normDeg, normDeg);
			}
		}

		T angle_unit_cost = 20; // angle difference which is mapped to one cost unit
		diff_mat = diff_mat / angle_unit_cost;


		return diff_mat;
	}


}