/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#include "visualizer.hpp"
#include <cstring>

namespace mat2bmp {

template <class BlazeMatrix> void blaze2bmp(BlazeMatrix m, std::string filename)
{

	int w = m.columns();
	int h = m.rows();

	int x, y, r, g, b;

	FILE *f;
	unsigned char *img = NULL;
	int filesize = 54 + 3 * w * h;

	img = (unsigned char *)malloc(3 * w * h);
	std::memset(img, 0, 3 * w * h);

	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			x = i;
			y = j;
			r = 0;
			g = 0;
			b = 0;
			int p = m(j, i) * 255;
			if (p > 0) // green for positive, red for negative
			{
				g = p;
				b = p;
			} else
				r = -p;
			if (r > 255)
				r = 255;
			if (g > 255)
				g = 255;
			if (b > 255)
				b = 255;
			img[(x + y * w) * 3 + 2] = (unsigned char)(r);
			img[(x + y * w) * 3 + 1] = (unsigned char)(g);
			img[(x + y * w) * 3 + 0] = (unsigned char)(b);
		}
	}

	unsigned char bmpfileheader[14] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
	unsigned char bmpinfoheader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0};
	unsigned char bmppad[3] = {0, 0, 0};

	bmpfileheader[2] = (unsigned char)(filesize);
	bmpfileheader[3] = (unsigned char)(filesize >> 8);
	bmpfileheader[4] = (unsigned char)(filesize >> 16);
	bmpfileheader[5] = (unsigned char)(filesize >> 24);

	bmpinfoheader[4] = (unsigned char)(w);
	bmpinfoheader[5] = (unsigned char)(w >> 8);
	bmpinfoheader[6] = (unsigned char)(w >> 16);
	bmpinfoheader[7] = (unsigned char)(w >> 24);
	bmpinfoheader[8] = (unsigned char)(h);
	bmpinfoheader[9] = (unsigned char)(h >> 8);
	bmpinfoheader[10] = (unsigned char)(h >> 16);
	bmpinfoheader[11] = (unsigned char)(h >> 24);

	f = fopen(filename.c_str(), "wb");
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);
	for (int i = 0; i < h; i++) {
		fwrite(img + (w * (h - i - 1) * 3), 3, w, f);
		fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
	}

	free(img);
	fclose(f);
}

template <class BlazeMatrix> void blaze2bmp_norm(BlazeMatrix m, std::string filename)
{
	auto maxval = max(m);
	auto minval = min(m);
	auto M = m / (maxval > -minval ? maxval : -minval);
	mat2bmp::blaze2bmp(M, filename);
}

template <template <typename, typename> class OuterContainer, template <typename, typename> class InnerContainer,
		  typename ValueType, typename OuterAllocator, typename InnerAllocator>
void blaze2bmp_norm(OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> m_stl,
					std::string filename, float magnitude = 1)
{
	blaze::DynamicMatrix<ValueType> m(m_stl.size(), m_stl[0].size(),
									  0); // we assume length of all rows is equal and there is at least 1 row
	for (size_t i = 0; i < m_stl.size(); ++i)
		for (size_t j = 0; j < m_stl[0].size(); ++j)
			m(i, j) = m_stl[i][j];
	auto maxval = max(m);
	auto minval = min(m);
	auto M = m / (maxval > -minval ? maxval : -minval) * magnitude;
	mat2bmp::blaze2bmp(M, filename);
}

template <template <typename, typename> class OuterContainer, template <typename, typename> class InnerContainer,
		  typename ValueType, typename OuterAllocator, typename InnerAllocator>
void blaze2bmp(OuterContainer<InnerContainer<ValueType, InnerAllocator>, OuterAllocator> m_stl, std::string filename,
			   float magnitude = 1)
{
	blaze::DynamicMatrix<ValueType> m(m_stl.size(), m_stl[0].size(),
									  0); // we assume length of all rows is equal and there is at least 1 row
	// blaze::DynamicMatrix<double> m(m_stl.size(), m_stl[0].size(), 0); // we assume length of all rows is equal and
	// there is at least 1 row
	for (size_t i = 0; i < m_stl.size(); ++i)
		for (size_t j = 0; j < m_stl[0].size(); ++j)
			m(i, j) = m_stl[i][j];
	// m(i, j) = (double)m_stl[i][j];
	auto M = m * magnitude;
	mat2bmp::blaze2bmp(M, filename);
}

} // namespace mat2bmp
