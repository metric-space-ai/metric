/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

	Created by Aleksey Timin on 3/29/20.
*/

#include "image_filter.hpp"

constexpr double PI = 3.14159265358979323846;

namespace mtrc {
using namespace mtrc::image_processing_details;

template <typename T, size_t N> Image<T, N> iminit(size_t rows, size_t columns, T initValue)
{
	return Image<T, N>(mtrc::numeric::DynamicMatrix<T>(rows, columns, initValue));
}

template <typename T>
std::pair<mtrc::numeric::DynamicMatrix<T>, Shape> PadModel<T>::pad(const Shape &shape,
																	 const mtrc::numeric::DynamicMatrix<T> &src) const
{
	using namespace mtrc::numeric;
	size_t padRow = shape[0];
	size_t padCol = shape[1];

	// Init padded matrix
	DynamicMatrix<T> dst;
	switch (_padDirection) {
	case PadDirection::PRE:
	case PadDirection::POST:
		dst = mtrc::numeric::DynamicMatrix<T>(src.rows() + padRow, src.columns() + padCol, _initValue);
		break;
	case PadDirection::BOTH:
		dst = mtrc::numeric::DynamicMatrix<T>(src.rows() + padRow * 2, src.columns() + padCol * 2, _initValue);
		break;
	}

	// Fill the padded matrix
	if (_padDirection == PadDirection::POST) {
		padRow = 0;
		padCol = 0;
	}

	mtrc::numeric::submatrix(dst, padRow, padCol, src.rows(), src.columns()) = src;
	// Padding
	for (size_t i = 0; i < dst.rows(); ++i) {
		for (size_t j = 0; j < dst.columns(); ++j) {
			int si = i - padRow;
			int sj = j - padCol;

			bool inside = si >= 0 && si < src.rows() && sj >= 0 && sj < src.columns();
			if (inside) {
				j += src.columns(); // work only in pad area
			} else {
				switch (_padType) {
				case PadType::CONSTANT:
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

					int xN = std::ceil((float)distX / src.rows());
					int yN = std::ceil((float)distY / src.columns());
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

//	template <typename ImgT, typename Filter, PadDirection PadDir, PadType PadType>
//	ImgT imfilter<ImgT, Filter, PadDir, PadType>::operator()(const ImgT& input) {
//		return ::mtrc::image_processing_details::filter(input, _filter, _padModel);
//	}
template <typename ChannelType, size_t N, typename Filter, PadDirection PadDir, PadType PadType>
Channel<ChannelType> imfilter<ChannelType, N, Filter, PadDir, PadType>::operator()(const Channel<ChannelType> &input)
{
	return ::mtrc::image_processing_details::filter(input, _filter, _padModel);
}

template <typename ChannelType, size_t N, typename Filter, PadDirection PadDir, PadType PadType>
Image<ChannelType, N> imfilter<ChannelType, N, Filter, PadDir, PadType>::operator()(const Image<ChannelType, N> &input)
{
	return ::mtrc::image_processing_details::filter(input, _filter, _padModel);
}

inline FilterType::AVERAGE::AVERAGE(size_t rows, size_t columns)
{
	FilterKernel f(rows, columns, 1.0);
	_kernel = f / mtrc::numeric::prod(Shape{rows, columns});
}

inline FilterType::GAUSSIAN::GAUSSIAN(size_t rows, size_t columns, double sigma)
{
	Shape shape{rows, columns};
	mtrc::numeric::StaticVector<FilterKernel::ElementType, 2> halfShape =
		(static_cast<mtrc::numeric::StaticVector<FilterKernel::ElementType, 2>>(shape) - 1) / 2;

	auto xrange = range<FilterKernel::ElementType, mtrc::numeric::rowVector>(-halfShape[1], halfShape[1]);
	auto yrange = range<FilterKernel::ElementType, mtrc::numeric::columnVector>(-halfShape[0], halfShape[0]);
	auto [xMat, yMat] = meshgrid(xrange, yrange);

	auto arg = -(xMat % xMat + yMat % yMat) / (2 * sigma * sigma);
	_kernel = mtrc::numeric::exp(arg);
	FilterKernel::ElementType max = mtrc::numeric::max(_kernel);
	for (int i = 0; i < _kernel.rows(); ++i) {
		for (int j = 0; j < _kernel.columns(); ++j) {
			_kernel(i, j) =
				_kernel(i, j) < max * std::numeric_limits<FilterKernel::ElementType>::epsilon() ? 0 : _kernel(i, j);
		}
	}

	auto sumh = mtrc::numeric::sum(_kernel);
	if (sumh != 0) {
		_kernel = _kernel / sumh;
	}

	_xMat = xMat;
	_yMat = yMat;
}

inline FilterType::LAPLACIAN::LAPLACIAN(double alpha)
{
	alpha = std::max<double>(0, std::min<double>(alpha, 1));
	auto h1 = alpha / (alpha + 1);
	auto h2 = (1 - alpha) / (alpha + 1);

	_kernel = FilterKernel{{h1, h2, h1}, {h2, -4 / (alpha + 1), h2}, {h1, h2, h1}};
}

inline FilterType::LOG::LOG(size_t rows, size_t columns, double sigma)
{
	Shape shape{rows, columns};
	auto std2 = sigma * sigma;
	GAUSSIAN gausFilter(rows, columns, sigma);

	auto h = gausFilter();
	_kernel =
		h % (gausFilter._xMat % gausFilter._xMat + gausFilter._yMat % gausFilter._yMat - 2 * std2) / (std2 * std2);
	_kernel -= mtrc::numeric::sum(_kernel) / mtrc::numeric::prod(shape);
}

inline FilterType::MOTION::MOTION(double len, int theta)
{
	len = std::max<double>(1, len);
	auto half = (len - 1) / 2;
	auto phi = static_cast<double>(theta % 180) / 180 * PI;

	double cosphi = std::cos(phi);
	double sinphi = std::sin(phi);
	int xsign = cosphi > 0 ? 1 : -1;
	double linewdt = 1;

	auto eps = std::numeric_limits<double>::epsilon();
	auto sx = std::trunc(half * cosphi + linewdt * xsign - len * eps);
	auto sy = std::trunc(half * sinphi + linewdt - len * eps);

	auto xrange = range<FilterKernel::ElementType, mtrc::numeric::rowVector>(0, sx, xsign);
	auto yrange = range<FilterKernel::ElementType, mtrc::numeric::columnVector>(0, sy);
	auto [xMat, yMat] = meshgrid(xrange, yrange);

	FilterKernel dist2line = (yMat * cosphi - xMat * sinphi);
	auto rad = mtrc::numeric::sqrt(xMat % xMat + yMat % yMat);

	// find points beyond the line's end-point but within the line width
	mtrc::numeric::DynamicMatrix<bool> cond =
		mtrc::numeric::map(rad, [half](const auto &x) { return x >= half; }) &&
		mtrc::numeric::map(abs(dist2line), [linewdt](const auto &x) { return x <= linewdt; });

	auto lastpix = mfind(static_cast<FilterKernel>(dist2line), cond);

	for (auto [i, j] : lastpix) {
		auto v = dist2line(i, j);
		auto pix = half - abs((xMat(i, j) + v * sinphi) / cosphi);
		dist2line(i, j) = std::sqrt(v * v + pix * pix);
	}

	dist2line = linewdt + eps - abs(dist2line);
	// zero out anything beyond line width
	dist2line = mtrc::numeric::map(dist2line, [](const FilterKernel::ElementType &v) { return v < 0 ? 0 : v; });

	auto h = rot90(rot90<FilterKernel::ElementType>(dist2line));

	_kernel = FilterKernel(h.rows() * 2 - 1, h.columns() * 2 - 1);
	mtrc::numeric::submatrix(_kernel, 0, 0, h.rows(), h.columns()) = h;
	mtrc::numeric::submatrix(_kernel, h.rows() - 1, h.columns() - 1, dist2line.rows(), dist2line.columns()) =
		dist2line;

	_kernel /= mtrc::numeric::sum(_kernel) + eps * len * len;
	if (cosphi > 0) {
		_kernel = flipud(_kernel);
	}
}

inline FilterType::UNSHARP::UNSHARP(double alpha)
{
	_kernel = FilterKernel{{0, 0, 0}, {0, 1, 0}, {0, 0, 0}} - LAPLACIAN(alpha)();
}

namespace image_processing_details {

template <typename T>
std::pair<mtrc::numeric::DynamicMatrix<T>, mtrc::numeric::DynamicMatrix<T>>
meshgrid(const mtrc::numeric::DynamicVector<T, mtrc::numeric::rowVector> &x,
		 const mtrc::numeric::DynamicVector<T, mtrc::numeric::columnVector> &y)
{
	mtrc::numeric::DynamicMatrix<T, mtrc::numeric::rowMajor> xMat(mtrc::numeric::size(y),
																	  mtrc::numeric::size(x));
	mtrc::numeric::DynamicMatrix<T, mtrc::numeric::columnMajor> yMat(mtrc::numeric::size(y),
																		 mtrc::numeric::size(x));

	for (int i = 0; i < xMat.rows(); ++i) {
		mtrc::numeric::row(xMat, i) = x;
	}

	for (int i = 0; i < yMat.columns(); ++i) {
		mtrc::numeric::column(yMat, i) = y;
	}

	return std::make_pair(xMat, yMat);
}

template <typename T, bool P> mtrc::numeric::DynamicVector<T, P> range(T start, T stop, T step)
{
	mtrc::numeric::DynamicVector<T, P> vec(std::abs((stop - start) / step) + 1);
	for (auto &val : vec) {
		val = start;
		start += step;
	}

	return vec;
}

template <typename T>
mtrc::numeric::DynamicVector<std::pair<size_t, size_t>, mtrc::numeric::columnVector>
mfind(const mtrc::numeric::DynamicMatrix<T> &input, const mtrc::numeric::DynamicMatrix<bool> &cond)
{
	std::vector<std::pair<size_t, size_t>> indecies;

	for (auto i = 0; i < input.rows(); ++i) {
		for (auto j = 0; j < input.columns(); ++j) {
			if (cond(i, j)) {
				indecies.push_back(std::make_pair(i, j));
			}
		}
	}

	return mtrc::numeric::DynamicVector<std::pair<size_t, size_t>, mtrc::numeric::columnVector>(indecies.size(),
																									indecies.data());
}

template <typename T> mtrc::numeric::DynamicMatrix<T> flipud(const mtrc::numeric::DynamicMatrix<T> &input)
{
	mtrc::numeric::DynamicMatrix<T> out(input.rows(), input.columns());
	for (int i = 0; i < input.rows(); ++i) {
		mtrc::numeric::row(out, input.rows() - i - 1) = mtrc::numeric::row(input, i);
	}

	return out;
}

static mtrc::numeric::DynamicMatrix<double> imgcov2(const mtrc::numeric::DynamicMatrix<double> &input,
													  const FilterKernel &kernel)
{
	size_t funcRows = kernel.rows();
	size_t funcCols = kernel.columns();

	mtrc::numeric::DynamicMatrix<double> resultMat(input.rows() - std::ceil((double)funcRows / 2),
													 input.columns() - std::ceil((double)funcCols / 2));
	for (auto i = 0; i < input.rows() - funcRows; ++i) {
		for (auto j = 0; j < input.columns() - funcCols; ++j) {
			auto bwProd = mtrc::numeric::submatrix(input, i, j, funcRows, funcCols) % kernel;
			auto filteredVal = mtrc::numeric::sum(bwProd);
			double val = mtrc::numeric::round(filteredVal);
			resultMat(i, j) = val > 0 ? val : 0;
		}
	}

	return resultMat;
}

template <typename Filter, typename ChannelType>
Channel<ChannelType> filter(const Channel<ChannelType> &channel, const Filter &impl,
							const PadModel<ChannelType> &padmodel, bool full)
{
	auto kernel = impl();
	Channel<ChannelType> result;
	Shape padShape{kernel.rows() - 1, kernel.columns() - 1};
	auto [paddedCh, imgCord] = padmodel.pad(padShape, channel);
	auto filteredChannel = imgcov2(paddedCh, kernel);
	if (full) {
		result = filteredChannel;
	} else {
		result = mtrc::numeric::submatrix(filteredChannel, std::max<size_t>(0, imgCord[0] - 1),
											std::max<size_t>(0, imgCord[1] - 1), channel.rows(), channel.columns());
	}

	return result;
}

template <typename Filter, typename ChannelType, size_t ChannelNumber>
Image<ChannelType, ChannelNumber> filter(const Image<ChannelType, ChannelNumber> &img, const Filter &impl,
										 const PadModel<ChannelType> &padmodel, bool full)
{
	auto kernel = impl();
	Shape padShape{kernel.rows() - 1, kernel.columns() - 1};
	Image<ChannelType, ChannelNumber> result;
	for (size_t ch = 0; ch < img.size(); ++ch) {
		auto [paddedCh, imgCord] = padmodel.pad(padShape, img[ch]);
		auto filteredChannel = imgcov2(paddedCh, kernel);
		if (full) {
			result[ch] = filteredChannel;
		} else {
			result[ch] =
				mtrc::numeric::submatrix(filteredChannel, std::max<size_t>(0, imgCord[0] - 1),
										   std::max<size_t>(0, imgCord[1] - 1), img[ch].rows(), img[ch].columns());
		}
	}

	return result;
}
} // namespace image_processing_details
} // namespace mtrc
