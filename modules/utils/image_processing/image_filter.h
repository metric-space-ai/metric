//
// Created by Aleksey Timin on 3/27/20.
//

#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include <algorithm>
#include <cmath>
#include <blaze/Math.h>
#include <blaze/Blaze.h>

/**
 * Module of image filters based on 2 convolution.
 *
 * Usage:
 *
 * auto input = iminit<uint8_t, 3>(1920, 1080, 50);
 *
 * MotionFilter filter(5, 45)
 * PadModel<uint8_t> padmodel(PadDirection::BOTH, PadType::CONST);
 *
 * Image<uint8_t, 3> ouput = imfilter(input, filter, padmodel);
 *
 * Implemented filters:
 *   AverageFilter
 *   GaussianFilter
 *   LaplacianFilter
 *   LogFilter
 *   MotionFilter
 *   PrewittFilter
 *   UnsharpFilter
 *   SobelFilter
 */
namespace metric {
	template<typename T>
	using Channel = blaze::DynamicMatrix<T>;

	template<typename T, size_t N>
	using Image = blaze::StaticVector<Channel<T>, N>;

	using FilterKernel = blaze::DynamicMatrix<double>;
	using Shape = blaze::StaticVector<size_t, 2>;

	/**
	 * Creates an Image
	 * @tparam T type of the element in a channel
	 * @tparam N number of channels
	 * @param rows number of rows (height)
	 * @param columns  number of columns (width)
	 * @param initValue default value for all elements in a channel
	 * @return
	 */
	template<typename T, size_t N>
	Image<T,N> iminit(size_t rows, size_t columns, T initValue={});

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
	 * Implements different ways to pad a channel before to apply a filter on them
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
		std::pair<blaze::DynamicMatrix<T>, Shape> pad(const Shape &shape, const blaze::DynamicMatrix<T> &src) const;

	private:
		PadDirection _padDirection;
		PadType _padType;
		T _initValue;
	};

	/**
	 * Filter an image
	 * @tparam Filter type of the filter
	 * @tparam ChannelType type of the chanel (RGB or gray)
	 * @param img image to filter
	 * @param impl implementation of the filter
	 * @param padmodel padding
	 * @param full if true it returns the matrix with padding, else returns only the image
	 * @return the filtered image
	 */
	template<typename Filter, typename ChannelType, size_t ChannelNumber>
	Image<ChannelType, ChannelNumber>
	imfilter(const Image<ChannelType, ChannelNumber> &img, const Filter &impl,
			 const PadModel <ChannelType> &padmodel, bool full = true);
	/**
	 * 	Average filter
	 */
	class AverageFilter {
	public:
		/**
		 * Creates an averaging filter
		  * @param shape the shape of the kernel
		  */
		AverageFilter(const Shape &shape);

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
		GaussianFilter(const Shape &shape, double sigma);

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
		explicit LaplacianFilter(double alpha);

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
		LogFilter(const Shape &shape, double sigma);

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
		MotionFilter(double len, int theta);

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
		UnsharpFilter(double alpha);

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

	namespace image_processing_details {
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
				 const blaze::DynamicVector<T, blaze::columnVector> &y);

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
		blaze::DynamicVector<T, P> range(T start, T stop, T step = 1);

		/**
		 * Finds elements in an intput matrix that corseponds true in the conditional matrix
		 * @tparam T the type of the elements
		 * @param input input matrix
		 * @param cond the conditional matrix
		 * @return returns a matrix of coordinates of founded elements
		 */
		template<typename T>
		blaze::DynamicVector<std::pair<size_t, size_t>, blaze::columnVector>
		mfind(const blaze::DynamicMatrix<T> &input, const blaze::DynamicMatrix<bool> &cond);

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
		blaze::DynamicMatrix<T> flipud(const blaze::DynamicMatrix<T> &input);


		/**
		 * Returns the two-dimensional convolution of a matrix and kernel
		 * @param kernel the kernel to convolute
		 * @return
		 */
		blaze::DynamicMatrix<double> imgcov2(const blaze::DynamicMatrix<double> &input, const FilterKernel &kernel);
	}
}

#include "image_filter.cpp"
#endif //IMAGE_FILTER