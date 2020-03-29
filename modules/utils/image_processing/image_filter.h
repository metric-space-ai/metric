//
// Created by Aleksey Timin on 3/27/20.
//

#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include <algorithm>
#include <cmath>
#include <blaze/Math.h>
#include <blaze/Blaze.h>

namespace image_processing {
	using RGB = blaze::StaticVector<uint8_t, 3>;
	using Image = blaze::DynamicMatrix<RGB>;
	using FilterKernel = blaze::DynamicMatrix<double>;
	using Shape = blaze::StaticVector<size_t, 2>;


	/**
	 *  The analog of Matlab's meshgrid
	 *
	 * @tparam T type of the elements
	 * @param x x-coordinates over grid
	 * @param y  y-coordinates over grid
	 * @return returns 2-D grid coordinates based on the coordinates contained in vectors x and y
	 */
	template<typename T>
	std::pair<blaze::DynamicMatrix<T>, blaze::DynamicMatrix<T>>
	meshgrid(const blaze::DynamicVector<T, blaze::rowVector> &x,
			 const blaze::DynamicVector<T, blaze::columnVector> &y) {
		blaze::DynamicMatrix<T, blaze::rowMajor> xMat(blaze::size(y), blaze::size(x));
		blaze::DynamicMatrix<T, blaze::columnMajor> yMat(blaze::size(y), blaze::size(x));

		for (int i = 0; i < xMat.rows(); ++i) {
			blaze::row(xMat, i) = x;
		}

		for (int i = 0; i < yMat.columns(); ++i) {
			blaze::column(yMat, i) = y;
		}

		return std::make_pair(xMat, yMat);
	}

	/**
	 * Creates a vector as range between two values
	 * @tparam T type of the lements
	 * @tparam P blaze::columnVector or blaze::rawVector
	 * @param start
	 * @param stop
	 * @param step
	 * @return
	 */
	template<typename T, bool P>
	blaze::DynamicVector<T, P> range(T start, T stop, T step = 1) {
		blaze::DynamicVector<T, P> vec(std::abs((stop - start) / step) + 1);
		for (auto &val : vec) {
			val = start;
			start += step;
		}

		return vec;
	}

	/**
	 * Finds elements in an intput matrix that corseponds true in the conditional matrix
	 * @tparam T the type of the elements
	 * @param input input matrix
	 * @param cond the conditional matrix
	 * @return returns a matrix of coordinates of founded elements
	 */
	template<typename T>
	blaze::DynamicVector<std::pair<size_t, size_t>, blaze::columnVector>
	mfind(const blaze::DynamicMatrix<T> &input, const blaze::DynamicMatrix<bool> &cond) {
		std::vector<std::pair<size_t, size_t>> indecies;

		for (auto i = 0; i < input.rows(); ++i) {
			for (auto j = 0; j < input.columns(); ++j) {
				if (cond(i, j)) {
					indecies.push_back(std::make_pair(i, j));
				}
			}
		}

		return blaze::DynamicVector<std::pair<size_t, size_t>, blaze::columnVector>(indecies.size(), indecies.data());
	}

	/**
	 * Rotetes matrix on 90 degree clockwise
	 * @tparam T type of the elements
	 * @param input matrix to rotate
	 * @return the rotated matrix
	 */
	template<typename T>
	blaze::DynamicMatrix<T> rot90(const blaze::DynamicMatrix<T> &input) {
		blaze::DynamicMatrix<T> out(input.columns(), input.rows());
		for (int i = 0; i < input.rows(); ++i) {
			auto r = blaze::trans(blaze::row(input, i));
			blaze::column(out, input.rows() - i - 1) = r;
		}

		return out;
	}

	/**
	 * Flips a matrix to down (analog og Matlab's flipud)
	 * @tparam T type of the elements
	 * @param input matrix to modify
	 * @return the flipped matrix
	 */
	template<typename T>
	blaze::DynamicMatrix<T> flipud(const blaze::DynamicMatrix<T> &input) {
		blaze::DynamicMatrix<T> out(input.rows(), input.columns());
		for (int i = 0; i < input.rows(); ++i) {
			blaze::row(out, input.rows() - i - 1) = blaze::row(input, i);
		}

		return out;
	}

	/**
	 * 	Average filter
	 */
	class AverageFilter {
	public:
		/**
		 * Creates an averaging filter
		  * @param shape the shape of the kernel
		  */
		AverageFilter(const Shape &shape) {
			FilterKernel f(shape[0], shape[1], 1.0);
			_kernel = f / blaze::prod(shape);
		}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
	};

	/**
	 * Disk average filer
	 */
	class DiskFilter {
	public:
		/**
		 * Creates circular averaging filter. Kernel size 2*rad+1
		 * @param rad the radius of the kernel
		 */
		explicit DiskFilter(double rad) : _rad(rad) {};

		FilterKernel operator()() const {

#if 0
			auto crad = std::ceil(_rad - 0.5);
			size_t matSize = crad * 2 + 1;

			auto xrange = range<FilterKernel::ElementType, blaze::rowVector>(-crad, crad);
			auto yrange = range<FilterKernel::ElementType, blaze::columnVector>(-crad, crad);
			auto[xMat, yMat] = meshgrid(xrange, yrange);


			FilterKernel xymaxMat = blaze::max(blaze::abs(xMat), blaze::abs(yMat));
			FilterKernel xyminMat = blaze::min(blaze::abs(xMat), blaze::abs(yMat));

#endif
			// TODO: Next steps are not clear. Implement!!!
			return FilterKernel();
		}


	private:
		FilterKernel::ElementType _rad;
	};

	/**
	 * Gaussian lowpass filter
	 */
	class GaussianFilter {
		friend class LogFilter;

	public:
		/**
		 * Creates a rotationally symmetric Gaussian lowpass filter
		 * @param shape the shape of the filter
		 * @param sigma standard deviation
		 */
		GaussianFilter(const Shape &shape, double sigma) {
			auto halfShape =
					(static_cast<blaze::StaticVector<FilterKernel::ElementType, 2>>(shape) - 1) / 2;

			auto xrange = range<FilterKernel::ElementType, blaze::rowVector>(-halfShape[1], halfShape[1]);
			auto yrange = range<FilterKernel::ElementType, blaze::columnVector>(-halfShape[0], halfShape[0]);
			auto[xMat, yMat] = meshgrid(xrange, yrange);

			auto arg = -(xMat % xMat + yMat % yMat) / (2 * sigma * sigma);
			_kernel = blaze::exp(arg);

			auto sumh = blaze::sum(_kernel);
			if (sumh != 0) {
				_kernel = _kernel / sumh;

			}

			_xMat = xMat;
			_yMat = yMat;
		}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
		FilterKernel _xMat;
		FilterKernel _yMat;
	};

	/**
	 * Laplacian filter
	 */
	class LaplacianFilter {
	public:
		/**
		 * Creates a 3-by-3 filter approximating the shape
		 * of the two-dimensional Laplacian operator.
		 * @param alpha must be in the range 0.0 to 1.0.
		 */
		explicit LaplacianFilter(double alpha) {
			alpha = std::max<double>(0, std::min<double>(alpha, 1));
			auto h1 = alpha / (alpha + 1);
			auto h2 = (1 - alpha) / (alpha + 1);

			_kernel = FilterKernel{
					{h1, h2,               h1},
					{h2, -4 / (alpha + 1), h2},
					{h1, h2,               h1}};
		}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;

	};

	/**
	 * Rotationally symmetric Laplacian of Gaussian filter
	 */
	class LogFilter {
	public:
		/**
		 * Creates a rotationally symmetric Laplacian of Gaussian filter
		 * @param shape the shape of the kernel
		 * @param sigma sigma standard deviation
		 */
		explicit LogFilter(const Shape &shape, double sigma) {
			auto std2 = sigma * sigma;
			GaussianFilter gausFilter(shape, sigma);

			auto h = gausFilter();
			_kernel = h % (gausFilter._xMat % gausFilter._xMat + gausFilter._yMat % gausFilter._yMat - 2 * std2)
					  / (std2 * std2);
			_kernel -= blaze::sum(_kernel) / blaze::prod(shape);
		}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
	};

	/**
	 * Filter to approximate, once convolved with an image,
	 * the linear motion of a camera
	 */
	class MotionFilter {
	public:
		/**
		 * Creates a filter
		 * @param len linear motion of a camera in pixels
		 * @param theta motion angle in degrees
		 */
		MotionFilter(double len, int theta) {
			len = std::max<double>(1, len);
			auto half = (len - 1) / 2;
			auto phi = static_cast<double>(theta % 180) / 180 * M_PI;


			double cosphi = std::cos(phi);
			double sinphi = std::sin(phi);
			int xsign = cosphi > 0 ? 1 : -1;
			double linewdt = 1;

			auto eps = std::numeric_limits<double>::epsilon();
			auto sx = std::trunc(half * cosphi + linewdt * xsign - len * eps);
			auto sy = std::trunc(half * sinphi + linewdt - len * eps);

			auto xrange =
					range<FilterKernel::ElementType, blaze::rowVector>(0, sx, xsign);
			auto yrange = range<FilterKernel::ElementType, blaze::columnVector>(0, sy);
			auto[xMat, yMat] = meshgrid(xrange, yrange);

			FilterKernel dist2line = (yMat * cosphi - xMat * sinphi);
			auto rad = blaze::sqrt(xMat % xMat + yMat % yMat);

			// find points beyond the line's end-point but within the line width
			blaze::DynamicMatrix<bool> cond = blaze::map(rad, [half](const auto &x) { return x >= half; })
											  && blaze::map(abs(dist2line),
															[linewdt](const auto &x) { return x <= linewdt; });

			auto lastpix = mfind(static_cast<FilterKernel>(dist2line), cond);

			for (auto[i, j] : lastpix) {
				auto v = dist2line(i, j);
				auto pix = half - abs((xMat(i, j) + v * sinphi) / cosphi);
				dist2line(i, j) = std::sqrt(v * v + pix * pix);
			}

			dist2line = linewdt + eps - abs(dist2line);
			// zero out anything beyond line width
			dist2line = blaze::map(dist2line, [](const FilterKernel::ElementType &v) {
				return v < 0 ? 0 : v;
			});

			auto h = rot90(rot90<FilterKernel::ElementType>(dist2line));

			_kernel = FilterKernel(h.rows() * 2 - 1, h.columns() * 2 - 1);
			blaze::submatrix(_kernel, 0, 0, h.rows(), h.columns()) = h;
			blaze::submatrix(_kernel,
							 h.rows() - 1, h.columns() - 1, dist2line.rows(), dist2line.columns()) = dist2line;

			_kernel /= blaze::sum(_kernel) + eps * len * len;
			if (cosphi > 0) {
				_kernel = flipud(_kernel);
			}

		}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
	};

	/**
	 * Prewitt filter
	 */
	class PrewittFilter {
	public:

		/**
		 * Creates 3-by-3 filter that emphasize
		 * horizontal edges by approximating a vertical gradient.
		 */
		PrewittFilter() :
				_kernel{
						{1,  1,  1},
						{0,  0,  0},
						{-1, -1, -1}
				} {}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
	};

	/**
	 * Unsharp filter
	 */
	class UnsharpFilter {
	public:
		/**
		 * Creates 3-by-3 unsharp contra enhancement filter.
		 * @param alpha must be in the range 0.0 to 1.0
		 */
		UnsharpFilter(double alpha) {
			_kernel = FilterKernel{
					{0, 0, 0},
					{0, 1, 0},
					{0, 0, 0}
			} - LaplacianFilter(alpha)();
		}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
	};

	/**
 	* Sobel filter
 	*/
	class SobelFilter {
	public:
		/**
		 * Creates 3-by-3 filter that emphasizes
		 * horizontal edges utilizing the smoothing effect by approximating a
		 * vertical gradient
		 */
		SobelFilter() :
				_kernel{
						{1,  2,  1},
						{0,  0,  0},
						{-1, -2, -1}
				} {}

		FilterKernel operator()() const {
			return _kernel;
		}

	private:
		FilterKernel _kernel;
	};


	enum class PadDirection {
		POST,
		PRE,
		BOTH
	};

	enum class PadType {
		CONST,
		REPLICATE,
		SYMMETRIC,
		CIRCULAR,
	};

	/**
	 * Implements different ways to pad images before to apply a filter on them
	 * see Matlab's padarray for more details
	 * @tparam T
	 */
	template<typename T>
	class PadModel {
	public:
		/**
		 * Creates the model
		 * @param padDirection
		 * @param padType
		 * @param initValue
		 */
		PadModel(PadDirection padDirection, PadType padType, T initValue = {})
				: _padDirection(padDirection), _padType(padType), _initValue(initValue) {};

		/**
		 * Pads the matrix
		 * @param shape shape of the padding
		 * @param src matrix to pad
		 * @return the padded matrix
		 */
		std::pair<blaze::DynamicMatrix<T>, Shape> pad(const Shape &shape, const blaze::DynamicMatrix<T> &src) const {
			using namespace blaze;
			size_t padRow = shape[0];
			size_t padCol = shape[1];

			// Init padded matrix
			DynamicMatrix<T> dst;
			switch (_padDirection) {
				case PadDirection::PRE:
				case PadDirection::POST:
					dst = blaze::DynamicMatrix<T>(src.rows() + padRow, src.columns() + padCol, _initValue);
					break;
				case PadDirection::BOTH:
					dst = blaze::DynamicMatrix<T>(src.rows() + padRow * 2, src.columns() + padCol * 2, _initValue);
					break;
			}

			// Fill the padded matrix
			if (_padDirection == PadDirection::POST) {
				padRow = 0;
				padCol = 0;
			}

			blaze::submatrix(dst, padRow, padCol, src.rows(), src.columns()) = src;
			// Padding
			for (size_t i = 0; i<dst.rows(); ++i) {
				for (size_t j = 0; j < dst.columns(); ++j) {
					int si = i - padRow;
					int sj = j - padCol;

					bool inside = si >= 0 && si < src.rows()
								  && sj >= 0 && sj < src.columns();
					if (inside) {
						j += src.columns();    //work only in pad area
					} else {
						switch (_padType) {
							case PadType::CONST:
								break;

							case PadType::REPLICATE:
								si = std::max<int>(0, si);
								si = std::min<int>(src.rows() - 1, si);

								sj = std::max<int>(0, sj);
								sj = std::min<int>(src.columns() - 1, sj);
								dst(i, j) = src(si, sj);
								break;

							case PadType::CIRCULAR:
								si = (i + padRow + 1) % src.rows();
								sj = (j + padCol + 1) % src.columns();
								dst(i, j) = src(si, sj);
								break;

							case PadType::SYMMETRIC: {
								int distX = padRow - i;
								int distY = padCol - j;

								int xN = std::ceil((float) distX / src.rows());
								int yN = std::ceil((float) distY / src.columns());
								int cordMx = padRow - xN * src.rows();
								int cordMy = padCol - yN * src.columns();
								int xi = (i - cordMx) % src.rows();
								int xj = (j - cordMy) % src.columns();

								si = xN % 2 == 0 ? xi : src.rows() - xi - 1;
								sj = yN % 2 == 0 ? xj : src.columns() - xj - 1;
								dst(i, j) = src(si, sj);
								break;
							}
						}
					}
				}
			}

			return std::make_pair(dst, Shape{padRow, padCol});
		}

	private:
		PadDirection _padDirection;
		PadType _padType;
		T _initValue;
	};

	/**
	 * Returns the two-dimensional convolution of a matrix and kernel
	 * @param input input matrix
	 * @param kernel the kernel to convole 
	 * @return
	 */
	Image imgcov2(const Image &input, const FilterKernel &kernel) {
		size_t funcRows = kernel.rows();
		size_t funcCols = kernel.columns();
		using DoubleImage = blaze::DynamicMatrix<blaze::StaticVector<double, 3>>;

		Image resultMat(input.rows() - std::ceil((double) funcRows / 2),
						input.columns() - std::ceil((double) funcCols / 2));
		for (auto i = 0; i < input.rows() - funcRows; ++i) {
			for (auto j = 0; j < input.columns() - funcCols; ++j) {
				DoubleImage window =
						blaze::submatrix(input, i, j, funcRows, funcCols);


				DoubleImage bwProd = window % kernel;
				auto filteredVal = blaze::sum(bwProd);
				for (auto &v : filteredVal) {
					v = v < 0 ? 0 : v;
				}
				resultMat(i, j) = blaze::round(filteredVal);
			}
		}

		return resultMat;
	}

	/**
	 * Image filter 
	 * @tparam Filter type of the filter 
	 * @tparam ChannelType type of the chanel (RGB or mono)
	 * @param img image to filter
	 * @param impl implementation of the filter
	 * @param padmodel padding
	 * @param full if true it returns the matrix with padding, else returns only the image
	 * @return the filtered image
	 */
	template<typename Filter, typename ChannelType>
	blaze::DynamicMatrix<ChannelType>
	imfilter(const blaze::DynamicMatrix<ChannelType> &img, const Filter &impl,
			 const PadModel<ChannelType> &padmodel, bool full = true) {
		auto kernel = impl();
		Shape padShape{kernel.rows() - 1, kernel.columns() - 1};
		auto[paddedImage, imgCord] = padmodel.pad(padShape, img);
		auto ret = imgcov2(paddedImage, kernel);
		if (full) {
			return ret;
		} else {
			return blaze::submatrix(ret,
									std::max<size_t>(0, imgCord[0] - 1),
									std::max<size_t>(0, imgCord[1] - 1),
									img.rows(),
									img.columns());
		}

	}

}
#endif //IMAGE_FILTER