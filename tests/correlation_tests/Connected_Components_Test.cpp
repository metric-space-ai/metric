/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#include <iostream>
#include <vector>
#include <string>
#include "../details/connected-components.hpp"



template <typename T>
class Matrix {
	std::vector<std::vector<T>> mx;

public:
	using ElementType = T;
	Matrix() {}

	Matrix(size_t size) : mx(size, std::vector<T>(size)){
	}
	
	Matrix(size_t size, size_t, bool): mx(size, std::vector<T>(size)) {}

	 typedef T value_type;

	size_t size() const {
		return mx.size();
	}
	void resize(size_t new_size, size_t) {
	    mx.resize(new_size);
	    for(auto & v : mx) {
		v.resize(new_size);
	    }
	}
	size_t rows() const {
	    return mx.size();
	}
	std::vector<T>& operator[](const size_t row) {
		return mx[row];
	}

	std::vector<T> operator[](const size_t row) const{
		return mx[row];
	}
	
	T & operator()(size_t row, size_t col)  {
	    return mx[row][col];
	}
	
	T operator()(size_t row, size_t col) const {
	    return mx[row][col];
	}
	


	void reset() {
		for (auto& e : mx)
			e.assign(size(), 0);
	}

	void resize(size_t size) {
		std::vector<T> v(size);
		mx.resize(size, v);
	}
};
template<typename Ty>
void MatrixTest(Matrix<Ty> M) {
	for (size_t i = 0; i < M.size(); ++i) {
		for (size_t j = 0; j < M.size(); ++j)
			if (M[i][j])
				std::cout << i << "," << j << " ";
			else
				std::cout << "    ";

		std::cout << std::endl;
	}

	std::cout << std::endl << "===========================================" << std::endl;
}




int main()
{
	std::vector<Matrix<unsigned>> res;

	Matrix<unsigned> M(9);

	std::cout << "Empty graph (with 0 vertices):" << std::endl;

	res = connected_components(M, 0);

	for (auto mx : res)
		MatrixTest(mx);

	M[0][1] = M[1][0] = M[1][2] = M[2][1] = M[2][4] = M[4][2] = M[4][3] = M[3][4] = M[2][5] = M[5][2] =
		M[3][6] = M[6][3] = M[7][3] = M[3][7] = M[7][8] = M[8][7] = 
		M[5][8] = M[8][5] = true;
	
	res= connected_components(M, 0);

	std::cout << "Full graph:" << std::endl;

	for (auto mx : res)
		MatrixTest(mx); 
			

	std::cout << std::endl << std::endl;
	
	//We will disconnect (2, 4) and (5, 8) pairs of vertices
	//the graph will fall to two components

	M[4][2] = M[2][4] = M[5][8] = M[8][5] = false;

	res = connected_components(M,0);

	std::cout << "Splitted graph:" << std::endl;

	for (auto mx : res)
		MatrixTest(mx);


	std::cout << std::endl << std::endl;

	// Disconnect (3, 7)

	M[3][7] = M[7][3] = false;

	std::cout << "3 component graph:" << std::endl;
	res = connected_components(M, 0);

	for (auto mx : res)
		MatrixTest(mx);


	std::cout << std::endl << std::endl;

	//// Disconnect (7, 8)
	std::cout << "4 component graph (with 2 null graphs (with 1 vertix)):" << std::endl;
	M[7][8] = M[8][7] = false;
	res = connected_components(M, 0);

	for (auto mx : res)
		MatrixTest(mx);


	std::cout << std::endl << std::endl;
}

