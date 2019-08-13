/*
		This Source Code Form is subject to the terms of the Mozilla Public
		License, v. 2.0. If a copy of the MPL was not distributed with this
		file, You can obtain one at http://mozilla.org/MPL/2.0/.

		Copyright (c) 2019 Panda Team
		Copyright (c) 2019 Oleg Popov
*/
/*
Laplacians is a package containing graph algorithms, with an emphasis on tasks related to 
spectral and algebraic graph theory. It contains (and will contain more) code for solving 
systems of linear equations in graph Laplacians, low stretch spanning trees, sparsifiation, 
clustering, local clustering, and optimization on graphs.

All graphs are represented by sparse adjacency matrices. This is both for speed, and because 
our main concerns are algebraic tasks. It does not handle dynamic graphs. It would be very slow 
to implement dynamic graphs this way.

https://github.com/danspielman/Laplacians.jl
*/

#include <iostream>
#include <assert.h>
#include <blaze/Math.h>
#include <functional>
#include <utility>
#include "../utils/solver/lapwrappers.h"
#include "../utils/solver/approxchol.h"
#include "../utils/solver/pcg_1.h"
#include "../utils/solver/ijvstruct.h"
#include "tests.h"

using blaze::DynamicVector;
using blaze::CompressedMatrix;
using namespace std;
using namespace laplacians;

void pcg_tests() {
	DynamicVector<double>pcg_a{ 1, 2, 3, 4 };
	DynamicVector<double>pcg_b{ 5, 4, 0, 1 };
	
	double beta = 2.3;
	bzbeta(beta, pcg_a, pcg_b);

//	cout << "Test of bzbeta function: "<< "\n" << pcg_a << "\n";

	assert(round(pcg_a[0] * 10) == 73 && round(pcg_a[1] * 10) == 86);

	//Out: (7.3, 8.6, 6.9, 10.2)

	DynamicVector<int>pcg_ai{ 1, 2, 3, 4 };
	DynamicVector<int>pcg_bi{ 5, 4, 0, 1 };

	int al = 4;
	axpy2(al, pcg_ai, pcg_bi);

	//cout << "Test of axpy2 function: " << "\n" << pcg_bi << "\n";
	assert(pcg_bi[0] == 9 && pcg_bi[3] == 17);

	//Out: 9, 12, 12, 17

}


void IJVtests() {

	CompressedMatrix<int, blaze::columnMajor> z{ {0,0,0,0}, {5,8,0,0}, {0,0,3,0}, {0,6,0,0} };

	//Convert a BLAZE sparse matrix to an IJV

	// A sample of  matrix was taken from 
	// https://en.wikipedia.org/wiki/Sparse_matrix
	CompressedMatrix<int, blaze::columnMajor> m{ {0,0,0,0}, {5,8,0,0}, {0,0,3,0}, {0,6,0,0} };
	
	SparseMatrixCSC<int> CSCMx(m);

	//cout << "Original blaze compressed matrix:\n" << m;
	
/*	cout << "\nCSC sparse matrix dump:\n";
	cout << "CSCMx.m=" << CSCMx.m << endl;
	cout << "CSCMx.n=" << CSCMx.n << endl;

	cout << "CSCMx.colptr= ";
	for (size_t k = 0; k <= CSCMx.n; ++k)
		cout << CSCMx.colptr[k] << " ";

	cout <<"\n"<< "CSCMx.rowval= ";
	for (size_t k = 0; k < CSCMx.nzval.size(); ++k)
		cout << CSCMx.rowval[k] << " ";

	cout << "\n" << "CSCMx.nzval= ";
	for (size_t k = 0; k < CSCMx.nzval.size(); ++k)
		cout << CSCMx.nzval[k] << " ";
*/		

	assert(CSCMx.m == 4 && CSCMx.n == 4 && CSCMx.colptr[0] == 0 && CSCMx.colptr[3] == 4 && 
		CSCMx.rowval[0] == 1 && CSCMx.rowval[3] == 2 && CSCMx.nzval[0] == 5 && CSCMx.nzval[2] == 6);

	/* for column major (default) matrix:
	CSCMx.m=4
	CSCMx.n=4
	CSCMx.colptr= 0 1 3 4 4
	CSCMx.rowval= 1 1 3 2
	CSCMx.nzval= 5 8 6 3
	*/

	//cout << endl << "\nTest toCompressedMatrix():\n";
	CompressedMatrix<int, blaze::columnMajor> mxtest;

	mxtest = CSCMx.toCompressedMatrix();
	//cout << "Compressed matrix\n"<<mxtest;

	assert(m == mxtest);
	
	//Convert a BLAZE sparse matrix to an IJV
	//cout << endl << "Convert from blaze compressed matrix to IJV structure.\n\n";

	IJV<int> ijv0(m);

	//dump_ijv(0, ijv0);

	assert(ijv0.n == 4 && ijv0.nnz == 4 && ijv0.i[0] == 1 && ijv0.i[2] == 3 && ijv0.j[0] == 0 && ijv0.j[3] == 2 &&
		ijv0.v[0] == 5 && ijv0.v[2] == 6);

	/*Out:
		ijv0.n=4
		ijv0.nnz=4
		ijv0.i= 1 1 3 2
		ijv0.j= 0 1 1 2
		ijv0.v= 5 8 6 3
	*/

	//cout << endl << "Convert from Julia SparseMatrixCSC to IJV structure.\n\n";
	IJV<int> ijv10(CSCMx);
	//dump_ijv(10, ijv10);

	assert(ijv0 == ijv10);

	/*Out:
		ijv10.n=4
		ijv10.nnz=4
		ijv10.i= 1 1 3 2
		ijv10.j= 0 1 1 2
		ijv10.v= 5 8 6 3
	*/

	//cout << "\n";

	//cout << "\nTest sparse function.\n\n";
	SparseMatrixCSC<int> CSCMx1;

	CSCMx1 = sparseCSC(ijv0);
	/*
	cout << "\nCSC sparse matrix dump:\n";
	cout << "CSCMx1.m=" << CSCMx1.m << endl;
	cout << "CSCMx1.n=" << CSCMx1.n << endl;

	cout << "CSCMx1.colptr= ";
	for (size_t k = 0; k <= CSCMx1.n; ++k)
		cout << CSCMx1.colptr[k] << " ";

	cout << "\n" << "CSCMx1.rowval= ";
	for (size_t k = 0; k < CSCMx1.nzval.size(); ++k)
		cout << CSCMx1.rowval[k] << " ";

	cout << "\n" << "CSCMx1.nzval= ";
	for (size_t k = 0; k < CSCMx1.nzval.size(); ++k)
		cout << CSCMx1.nzval[k] << " ";

		*/

	assert(CSCMx == CSCMx1);

	/* for column major (default) matrix:
	 CSCMx1.m=4
	 CSCMx1.n=4
	 CSCMx1.colptr.i= 0 1 3 4 4
	 CSCMx1.rowval= 1 1 3 2
	 CSCMx1.nzval.v= 5 8 6 3
	 */
	
	//cout << endl;

	//cout << "Test overloaded operators.\n";

	IJV<int> ijv1 = ijv0;

	//cout << "\noperator= :\n";

	//dump_ijv(1, ijv1);

	bool t = ijv1 == ijv0;
	//cout << "\n\noperator== :\n ijv1==ijv: " << (t ? "true" : "false");

	assert(ijv1 == ijv0);

	//Change ijv1
	ijv1.v[0] = 10;

	t = ijv1 == ijv0;
	//cout << "\n\noperator== :\n ijv1==ijv: " << (t ? "true" : "false");
	assert(!(ijv1 == ijv0));

	//cout << "\n\noperator* :\n ijv * 5";

	ijv1 = ijv0*5;

/*	cout << "\n" << "ijv1.v= ";
	for (size_t k = 0; k < ijv1.nnz; ++k)
		cout << ijv1.v[k] << " ";

	cout << "\n\noperator* :\n 5 * ijv";*/

	assert(ijv1.v[0] == 25 && ijv1.v[1] == 40);

	ijv1 = 5 * ijv0;

	/*
	cout << "\n" << "ijv1.v= ";
	for (size_t k = 0; k < ijv1.nnz; ++k)
		cout << ijv1.v[k] << " ";
	*/
	assert(ijv1.v[0] == 25 && ijv1.v[1] == 40);

//	cout << "\n\nTest a constructor:\n";

	IJV<int> ijv2(ijv0.n, ijv0.nnz, ijv0.i, ijv0.j, ijv0.v);

//	dump_ijv(2, ijv2);

	assert(ijv2 == ijv0);

//	cout << "\n\nTest IJV::ToCompressMatrix():\n";
	//dump_ijv(0, ijv0);

	CompressedMatrix<int, blaze::columnMajor> newm=ijv0.toCompressedMatrix();
	//cout << "toCompressedMatrix\n"<<newm;

	assert(newm == m);

	/*size_t h1 = hashijv(ijv0);
	size_t h2 = hashijv(ijv0, 5);
	cout << "\n\nTest hash(IJV) function:\n";
	cout <<"hash(ijv): " << h1 << endl;
	cout << "hash(ijv, 5): " << h2 << endl;

	assert(h1 != h2);*/

	//cout << "\n\nTest compress(IJV) function:\n";

	IJV<int>ijv3 = compress(ijv0);

	//dump_ijv(3, ijv3);

	assert(ijv0 == ijv3);

	//cout << endl << "\nTranspose ijv:\n ";
	IJV<int> ijv4 = transpose(ijv0);

	//dump_ijv(4, ijv4);
	assert(ijv4.i == ijv0.j && ijv4.j == ijv0.i && ijv4.v == ijv0.v);

	//cout << endl << ijv4.toCompressedMatrix()<<endl;

//	cout << endl<<"Test a SparseMatrics constructor:\n";
	SparseMatrixCSC<int> CSCMx2(CSCMx.m, CSCMx.n, CSCMx.colptr, CSCMx.rowval, CSCMx.nzval);

	assert(CSCMx2 == CSCMx);

	//cout << "\nTest IJV constructor with Dynamic Vectors:\n";

	DynamicVector<int> VV{ 5, 8, 6, 3 };
	
	vector<size_t> VI{ 1, 1, 3, 2 }, VJ{ 0, 1, 1, 2 };

	IJV<int> ijv5(4, 4, VI, VJ, VV);

	//dump_ijv(5, ijv5);

	assert(ijv5 == ijv0);

	//Testing IJV.sortbyCol

	IJV<double> ijv_sorted= grid2_ijv<double>(3);
	//ijv_sorted.dump_ijv(100);
	

	//Testing sortedIndices

	vector<size_t> indx = ijv_sorted.sortedIndices();

	/*for (size_t i: indx )
	{
		cout << i << " ";
	}
	cout << endl;*/

	assert(indx[0] == 3 && indx[2] == 0 && indx[16] == 23);

	ijv_sorted.sortByCol();
	//ijv_sorted.dump_ijv(200);

	assert(ijv_sorted.i[0] == 1 && ijv_sorted.i[1] == 3 && ijv_sorted.j[0] == 0 && ijv_sorted.j[5] == 2);

}

void CollectionTest() {
	CompressedMatrix<int, blaze::columnMajor> m, m1;

	//Test path_graph_ijv
	IJV<int> ijv = path_graph_ijv<int>(5);

	//dump_ijv(0, ijv);
	m1 = ijv.toCompressedMatrix();
	//cout << endl << endl << m1 << endl;
	assert(m1(0, 0) == 0 && m1(0, 1) == 1 && m1(1, 0) == 1);

	/* Out:
	0  1  0  0  0
	1  0  1  0  0
	0  1  0  1  0
	0  0  1  0  1
	0  0  0  1  0
	
	*/


	//Test testZeroDiag

	assert(testZeroDiag(m1));

	//Add non zero to a diagonal cell
	m1(2, 2) = 1;

	assert(!testZeroDiag(m1));

	// Test connections()
	CompressedMatrix<int, blaze::columnMajor> m10(10,10);
	m10(1, 0) = 1;	m10(2, 0) = 1;	m10(0, 1) = 1;	m10(5, 1) = 1;	m10(8, 1) = 1;
	m10(0, 2) = 1;	m10(5, 3) = 1;	m10(7, 4) = 1;	m10(1, 5) = 1;	m10(3, 5) = 1;
	m10(8, 5) = 1;	m10(4, 7) = 1;	m10(8, 7) = 1;	m10(1, 8) = 1;	m10(5, 8) = 1;
	m10(7, 8) = 1;

	//cout << endl << endl;
	//cout << "\n\nCall function componets():\n";

	SparseMatrixCSC<int> sprs(m10);
	vector<size_t> comp = components(sprs);
	
	/*for (int i = 0; i < comp.size(); i++)
		cout << comp[i] << " ";*/

	//cout << endl;

	vector<size_t>comp1 = components(m10);

	/*for (int i = 0; i < comp1.size(); i++)
		cout << comp1[i] << " ";*/

	//cout << endl;
	assert(comp == comp1);
	assert(comp[0] == 1 && comp[6] == 2 && comp[9] == 3);

	//Out = 1, 1, 1, 1, 1, 1, 2, 1, 1, 3
	
	//cout << endl << endl;
	
	//Test Kronecker product function kron(A, B)

	CompressedMatrix<int, blaze::columnMajor> A(2,2), B(2,2), C;

	//Create two occasional matrices
	//See an example at https://en.wikipedia.org/wiki/Kronecker_product

	A(0, 0) = 1; A(0, 1) = 2; A(1, 0) = 3; A(1, 1) = 4;
	B(0, 1) = 5; B(1, 0) = 6; B(1, 1) = 7;

	//cout << A << endl;
	//cout << B << endl;

	C = kron(A, B);
	//cout << "\nC = kron(A, B)=\n" << C;
	assert(C(0, 0) == 0 && C(1, 1) == 7 && C(3, 3) == 28);

/*Out:
	kron(A, B)=

	0   5   0  10
	6   7  12  14
	0  15   0  20
	18  21  24  28
	
	*/

	assert(C(0, 1) == 5 && C(1, 1) == 7 && C(3, 2) == 24);

	//Test flipIndex

	//cout << endl << endl << "flipIndex(C)=\n";

	vector<size_t> v = flipIndex(C);

	/*for (int i = 0; i < v.size(); i++) 
		cout << v[i] << " ";*/
	
	assert(v[0] == 2 && v[1] == 8 && v[3] == 3);
	
	//Out="flipIndex(C)=2, 8, 0, 3, 6, 9, 4, 10, 1, 5, 7, 11"

	//Test function diag()

	//cout << endl << endl << "diag(C)=\n";

	DynamicVector<int> v1 = diag(C, 1);

	/*for (int i = 0; i < v1.size(); i++) {
		cout << v1[i] << " ";
	}*/

	assert(v1[0] == 5 && v1[1] == 12 && v1[2] == 20);
	//Out: diag(C) = 5 12 20

	//Test function Diagonal()
	
	DynamicVector<int> dv=v1;
	
	CompressedMatrix<int, blaze::columnMajor>Dg = Diagonal(dv);

	//cout << endl << endl << "Diagonal(v)=\n" << Dg;
	assert(Dg(0, 0) == 5 && Dg(1, 1) == 12 & Dg(2, 2) == 20);

	/* Out:
	5  0  0
	0 12  0
	0  0 20
	*/

	//Test sum()

	/*
	C=	0  5   0  10
	    6  7  12 14
		0  15  0 20
		18 21 24 28
		
		*/

	// rowwise sum
	DynamicVector<int> vec1 = sum(C);

	//cout << "\nsum(C) = \n" << vec1;
	//Out: (15, 39, 35, 91)
	assert(vec1[0] == 15 && vec1[1] == 39 && vec1[2] == 35 && vec1[3] == 91);

	//columnwise sum
	vec1 = sum(C, 2);
	//cout << "\nsum(C) = \n" << vec1;
	//Out: (24, 48, 36, 72)
	assert(vec1[0] == 24 && vec1[1] == 48 && vec1[2] == 36 && vec1[3] == 72);

	//Test diagmat()
	CompressedMatrix<int, blaze::columnMajor> DiagMx = diagmat(C);
	//cout << "\ndiagmat(C) = \n" << DiagMx;

	assert(DiagMx(0, 0) == 15 && DiagMx(1, 1) == 39 && DiagMx(2, 2) == 35 && DiagMx(3, 3) == 91);
	
	/*
	Out:
	
	15 0 0 0
	0 39 0 0
	0 0 35 0
	0 0 0 91 
  */

	//Test findmax function

	pair<int, size_t> mxpair = findmax(C);

	//cout << "findmax=" << "(" << mxpair.first << ", " << mxpair.second << ")" << endl;

	bool bl = mxpair == pair<int, size_t>(28, 3);
	assert(bl);

	DynamicVector<int> vmax{ 1, 2, 5, 4, 0 };
	mxpair = findmax(vmax);

	//cout << "findmax=" << "(" << mxpair.first << ", " << mxpair.second << ")" << endl;

	bl = mxpair == pair<int, size_t>(5, 2);
	assert(bl);

	CompressedMatrix<int, blaze::columnMajor> mx1{ {1,2}, {3,4} };
	CompressedMatrix<int, blaze::columnMajor>powmx = pow(mx1, 2);

	//cout << "\npow(M) = \n" << powmx;

	/* Out:
		7  10
		15  22
	*/

	assert(powmx(0, 0) == 7 && powmx(0, 1) == 10 && powmx(1, 0) == 15 && powmx(1, 1) == 22);

	//Test power():
	CompressedMatrix<int, blaze::columnMajor>powmx1 = power(mx1, 2);

	//cout << "\npower(C, 2) = \n" << powmx1;
	
	/* Out:
		0  10
		15  0
	*/

	assert(powmx1(0, 0) == 0 && powmx1(0, 1) == 10 && powmx1(1, 0) == 15 && powmx1(1, 1) == 0);

	//Test kron()  for vectors

	DynamicVector<int>Av{ 1,2,3 }, Bv{ 4, 5, 6 }, Cv;

	Cv = kron(Av, Bv);
	//cout << "\nCv = kron(Av, Bv)=\n" << Cv;

	//Out = (4, 5, 6, 8, 10, 12, 12, 15, 18)
	assert(Cv[0] == 4 && Cv[3] == 8 && Cv[8] == 18);

	//cout << "\nTest overloaded IJV operators.\n";

	IJV<int> ijv1(mx1), ijv2(powmx), ijv3;

	ijv3 = ijv1 + ijv2;

	//cout << "\noperator+ :\n";

	//dump_ijv(1, ijv1);
	//cout << endl;
	//dump_ijv(2, ijv2);
	//cout << endl;
	//dump_ijv(3, ijv3);
	//cout << endl;
	assert(ijv3.i[0] == 0 && ijv3.i[4] == 0 && ijv3.j[0] == 0 && ijv3.j[4] == 0 && ijv3.v[0] == 1 && ijv3.v[4] == 7);
}

void CollectionFunctionTest() {

	//Test product_graph() function

	CompressedMatrix<int, blaze::columnMajor>
		GrA{ {0, 1, 0, 1, 0, 0, 0, 0, 0}, {1, 0, 1, 0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0, 0},
			{1, 0, 0, 0, 0, 0, 1, 1, 0}, {0, 1, 0, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 1, 0, 0, 0, 1},
			{0, 0, 0, 1, 0, 0, 0, 1, 0}, {0, 0, 0, 1, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 0, 1, 0, 0, 0} };

	//cout << endl << GrA;

	/*SparseMatrixCSC<double> CSCMx(GrA);

	cout << "Original blaze compressed matrix:\n" << GrA;

	cout << "\nCSC sparse matrix dump:\n";
	cout << "CSCMx.m=" << CSCMx.m << endl;
	cout << "CSCMx.n=" << CSCMx.n << endl;

	cout << "CSCMx.colptr= ";
	for (size_t k = 0; k <= CSCMx.n; ++k)
		cout << CSCMx.colptr[k] << " ";

	cout << "\n" << "CSCMx.rowval= ";
	for (size_t k = 0; k < CSCMx.nzval.size(); ++k)
		cout << CSCMx.rowval[k] << " ";

	cout << "\n" << "CSCMx.nzval= ";
	for (size_t k = 0; k < CSCMx.nzval.size(); ++k)
		cout << CSCMx.nzval[k] << " ";
	*/

	CompressedMatrix<int, blaze::columnMajor>
		GrB{ {0, 0, 0, 0, 1, 0, 0, 0, 0}, {0, 0, 1, 1, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 1, 0, 0, 0},
			 {0, 1, 0, 0, 0, 0, 0, 1, 0}, {1, 0, 0, 0, 0, 1, 1, 0, 0}, {0, 0, 1, 0, 1, 0, 0, 0, 1},
			 { 0, 0, 0, 0, 1, 0, 0, 0, 0}, {0, 0, 0, 1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 1, 0, 0, 0} };

	//cout << endl << GrB;

	IJV<int>IJVA(GrA), IJVB(GrB);

	IJV<int> ijv0 = product_graph(IJVA, IJVB);
	/*cout << "\n\nproduct_graph(B, A)=\n";
	dump_ijv(0, ijv0);*/
	assert(ijv0.i[2] == 22 && ijv0.i[ijv0.i.size() - 3] == 35 && ijv0.j[2] == 18 && ijv0.j[ijv0.j.size() - 3] == 71);

	CompressedMatrix<int, blaze::columnMajor>GridMx;

	//Test grid2()
	GridMx = grid2<int>(5);

	//cout << GridMx;

	assert(GridMx(0, 1) == 1 && GridMx(0, 5) == 1 && GridMx(1, 0) == 1);

	//Test lap function
	CompressedMatrix<int, blaze::columnMajor> LapMx = lap(GrA);
	//cout << LapMx << endl;

	assert(LapMx(0, 0) == 2 && LapMx(1, 1) == 3 && LapMx(3, 0) == -1 && LapMx(0, 1) == -1);

	//Test forceLap function
	CompressedMatrix<int, blaze::columnMajor>forceLapMx = forceLap(GrA);
	//cout << forceLapMx << endl;
	assert(forceLapMx(0, 0) == 2 && forceLapMx(1, 1) == 3 && forceLapMx(3, 0) == -1 && forceLapMx(0, 1) == -1);

	//Test vecToComps() function

	vector<size_t> V{ 1, 2, 1, 2, 3, 3, 3 };

	vector<vector<size_t>>comp = vecToComps(V);

	//cout << comp;

	assert(comp[0][1] == 2 && comp[1][1] == 3 && comp[2][1] == 5);

	CompressedMatrix<int, blaze::columnMajor> Ma{ {1, 2, 3}, {4, 5, 6}, {7, 8, 9} };
	CompressedMatrix<int, blaze::columnMajor> Midx = index<int>(Ma, { 0, 1 }, { 1, 2 });

	//cout << "\nMidx=\n" << Midx << endl;

	assert(Midx(0, 0) == 2 && Midx(0, 1) == 3 && Midx(1, 0) == 5 && Midx(1, 1) == 6);

	vector<size_t> Idx0{ 0 };

	Midx = index<int>(Ma, { 0, 1 }, Idx0);
	  
	//cout << Midx << endl;

	assert(Midx(0, 0) == 1 && Midx(1, 0) == 4);

	DynamicVector<int> Vidx;

	Vidx = index<int>(Ma, { 0, 1 });
	//cout << Midx << endl;

	assert(Vidx[0] == 1 && Vidx[1] == 4);

	DynamicVector<int> vout(10, 0), vin{ 3, 5, 9 };

	index(vout, { 1, 2, 6 }, vin);

	//cout << "vout[idx]=vin\n" << vout << endl;

	assert(vout[1] == 3 && vout[2] == 5 && vout[6] == 9);

	
	//cout << vout << endl;

	vout = index<int>(vin, { 1, 2 });

	//cout << "vout=vin[idx]\n" << vout << endl;

	assert(vout[0] == 5 && vout[1] == 9);

	CompressedMatrix<int, blaze::columnMajor> Midx08{ {0,1,2}, {3,4,5}, {6,7,8} };
	DynamicVector<int> v11;
	
	v11 = index<int>(Midx08, { 0,1,2 }, 1);
	//cout << endl << v11 << endl;
	assert(v11[0] == 1 && v11[1] == 4 && v11[2] == 7);

	v11 = index<int>(Midx08, 1, { 0, 1, 2 });
	//cout << endl << v11 << endl;
	assert(v11[0] == 3 && v11[1] == 4 && v11[2] == 5);

	CompressedMatrix<int, blaze::columnMajor> Midx00(3, 3);

	index(Midx00, { 0,1,2 }, 1, vin);

	//cout << endl << Midx00 << endl;

	assert(Midx00(0, 1) == 3 && Midx00(1, 1) == 5 && Midx00(2, 1) == 9);

	DynamicVector<int> v10{ 1,2,3,4,5 };

	DynamicVector<int> vbool;
	vbool = indexbool(v10, { 0, 0, 1, 0, 1 });
	//cout << endl << vbool << endl;
	
	vector<size_t> vsz = indexbool({ 1, 2, 3, 4, 5 }, { 0, 0, 1, 0, 1 });

	/*for (size_t i = 0; i < vsz.size(); i++)
		cout << vsz[i]<<" ";

	cout << endl << endl;*/

	//Test sparse function

	//A = sparse(I, J, V, 10, 10)

	vector<size_t> I1{ 1, 2, 0, 5, 8, 0, 5, 7, 1, 3, 8, 4, 8, 1, 5, 7 };
	vector<size_t> J1{ 0, 0, 1, 1, 1, 2, 3, 4, 5, 5, 5, 7, 7, 8, 8, 8 };
	DynamicVector<int> V1{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	CompressedMatrix<int, blaze::columnMajor> sparseRes = sparse(I1, J1, V1, 10, 10);

	//cout << sparseRes << endl;

	assert(sparseRes(1, 0) == 1 && sparseRes(1, 8) == 1 && sparseRes(7, 8)==1);
	
	// Cholesky decomposition
	// Test cholesky function

	//Test example from https://en.wikipedia.org/wiki/Cholesky_decomposition

	CompressedMatrix<double, blaze::columnMajor> ChA{ {4.0, 12.0, -16.0}, {12.0, 37.0,	 -43.0}, {-16.0, -43.0, 98.0 } };
	CompressedMatrix<double, blaze::columnMajor> L;

	Factorization<double> f;

	try {
		f = cholesky(ChA);
	}
	catch (std::runtime_error ex) {
		cout << ex.what();
	}

	//cout << f.Lower << endl;

	CompressedMatrix<double, blaze::columnMajor> ChA1 = f.Lower * blaze::ctrans(f.Lower);

	assert(ChA1 == ChA);

	//Test chol_subst function

	CompressedMatrix<double, blaze::columnMajor>X{ {1.0,0.0,0.0}, {2.0,0.0,0.0}, {3.,0.,0.} };

	DynamicVector<double> x;

	try {
		x = chol_subst(f.Lower, X);
	}
	catch (std::runtime_error ex) {
		cout << ex.what();
	}
	DynamicVector<double> B = ChA * x;

	//cout <<"X=\n" << x << endl;
	//cout << "B=\n" << B << endl;

	assert(abs(B[0] - X(0, 0)) < 1e-6 && abs(B[1] - X(1, 0)) < 1e-6 && abs(B[2] - X(2, 0)) < 1e-6);

	//Test calculation error

	//Create random matrices A and B

	CompressedMatrix<double, blaze::columnMajor> a{
	{-0.356543, -0.136045, -1.93844, 1.18337, -0.207743},
	{-0.67799, 1.95279, -0.193003, -1.84183, -0.662046},
	{2.61283, 1.51118, 0.672955, -0.840613, 2.01147},
	{0.859572, -0.943768, 0.375822, -1.57407, -0.858285},
	{-0.0863611, -1.47299, 1.02716, 1.904, -0.42796}
	};

	CompressedMatrix<double, blaze::columnMajor> b
	{ {1.064160977905516,0,0,0,0},
	{-0.3334067812850509,0,0,0,0},
	{0.7919292830316926,0,0,0,0},
	{0.01651278833545206,0,0,0,0},
	{-0.6051230029995152,0,0,0,0} };

	a = a * blaze::trans(a);

	vector<size_t> pcgits;
	SolverBMat<double>SolveA = wrapInterfaceMat<double>(cholesky<double>, a, pcgits);

	DynamicVector<double> b1(b.rows());

	for (int i = 0; i < b1.size(); i++)
		b1[i] = b(i, 0);

	DynamicVector<double> vvv = SolveA(b);

	double l2 = norm(a*SolveA(b) - b1);

	//cout << "norm(ax-b)=" << l2 << endl;

	assert(abs(l2) < 2e-16);

	//Test chol_sddm 
	SolverAMat<double> ch_sddm = chol_sddm_mat<double>();
	SubSolverMat<double> SolveA1 = ch_sddm(a);

	l2 = norm(a*SolveA1(b) - b1);

	//cout << "norm2(ax-b)=" << l2 << endl;

	assert(abs(l2) < 2e-16);

	//Test isConnected function

	CompressedMatrix<int, blaze::columnMajor> m10(10, 10);
	m10(1, 0) = 1;	m10(2, 0) = 1;	m10(0, 1) = 1;	m10(5, 1) = 1;	m10(8, 1) = 1;
	m10(0, 2) = 1;	m10(5, 3) = 1;	m10(7, 4) = 1;	m10(1, 5) = 1;	m10(3, 5) = 1;
	m10(8, 5) = 1;	m10(4, 7) = 1;	m10(8, 7) = 1;	m10(1, 8) = 1;	m10(5, 8) = 1;
	m10(7, 8) = 1;

	assert(!isConnected(m10));

	CompressedMatrix<int, blaze::columnMajor>
		MeanA{ {0, 1, 0, 1, 0, 0, 0, 0, 0}, {1, 0, 1, 0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0, 0},
			{1, 0, 0, 0, 0, 0, 1, 1, 0}, {0, 1, 0, 0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 1, 0, 0, 0, 1},
			{0, 0, 0, 1, 0, 0, 0, 1, 0}, {0, 0, 0, 1, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 0, 1, 0, 0, 0} };

	//cout << "mean = " << lapld.mean(MeanA) << endl;

	SolverB<double>SolveAv = wrapInterface<double>(cholesky<double>, a, pcgits);

	b1 = { 1.064160977905516, -0.3334067812850509, 0.7919292830316926, 0.01651278833545206, -0.6051230029995152 };

	double l2v = norm(a*SolveAv(b1) - b1);

	//cout << "norm(ax-b)=" << l2v << endl;

	assert(abs(l2v) < 2e-16);

	//Test chol_sddm for vector

	SolverA<double> ch_sddmv = chol_sddm<double>();
	SubSolver<double> SolveA1v = ch_sddmv(a);
	DynamicVector<double> x11 = SolveA1v(b1, pcgits);

	l2 = norm(a*x11 - b1);

	//cout << "norm2(ax-b)=" << l2 << endl;

	assert(abs(l2) < 2e-16);

	/* 
	Create four component graph of 10 vertices to test
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0
	0, 0, 0, 0, 1, 0, 0, 1, 0, 0
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0
	0, 0, 0, 0, 0, 1, 0, 0, 0, 0
	1, 0, 1, 0, 0, 0, 1, 0, 0, 0
	0, 0, 0, 1, 0, 0, 0, 0, 0, 0

	*/
	CompressedMatrix<double, blaze::columnMajor> GraphA10({
		{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, 
		{ 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 1, 0, 0, 1, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
		{ 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
		{ 1, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
		{ 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 }
		});

	DynamicVector<double> b10{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0 };

	//Test lapWrapComponents

	SolverB<double> Solver10 = lapWrapComponents(ch_sddmv, GraphA10, pcgits);

	DynamicVector<double> x10 = Solver10(b10);

	//cout << "x= \n" << trans(x10) << endl;
	assert(abs(x10[0] - (-3)) < 1e-6 && abs(x10[1]) < 1e-6 && abs(x10[4] - (-1.444444)) < 1e-6);

	//Test lapWrapSDDM
	
	SolverA<double> lap_sddmv = lapWrapSDDM(chol_sddm<double>());
	
	SubSolver<double> SolveA11 = lap_sddmv(GraphA10);

	x10 = SolveA11(b10);
	//cout << "\nx= \n" << trans(x10) << endl;
	assert(abs(x10[0] - (-3)) < 1e-6 && abs(x10[1]) < 1e-6 && abs(x10[4] - (-1.444444)) < 1e-6);

	// Test adj function

	// One component graph

	CompressedMatrix<double, blaze::columnMajor> Graph1comp{ 
		{0, 0, 0, 1, 1}, 
		{0, 0, 1, 0, 0},
		{0, 1, 0, 1, 1},
		{1, 0, 1, 0, 0},
		{1, 0, 1, 0, 0}
	};

	CompressedMatrix<double, blaze::columnMajor>lapGraph = lap(Graph1comp);

	auto [am, dd] = adj(lapGraph);
		
	//cout << "\nadj=\n" << am << endl;

	assert(abs(mean(Graph1comp) - mean(am)) < 1e-6);
	assert(abs(mean(dd)) < 1e-6);

	// Test extendMatrix function

	DynamicVector<double> d{ 10, 20, -30, 40, 50 };
	CompressedMatrix<double, blaze::columnMajor> ExtM = extendMatrix(lapGraph, d);

	//cout << ExtM << endl;

	assert(abs(round(ExtM(0, 0))) == 2 && abs(round(ExtM(2, 2))) == 3 && abs(round(ExtM(5, 0))) == 10 && abs(round(ExtM(2, 5))) == 0);

	//Test sddmWrapLap

	/*SolverA<double> lap_sddmv1 = lapld.sddmWrapLap(lapld.chol_sddm());

	SolveA11 = lap_sddmv1(GraphA10);

	x10 = SolveA11(b10);
	cout << endl << "\nx= \n" << trans(x10) << endl;
	
	*/

	//Test findnz function

	auto [iv, jv, vv] = findnz(Graph1comp);


	/*cout << "iv= ";
	for (size_t i = 0; i < iv.size(); i++)
		cout << iv[i];
		
	cout << endl;

	cout << "jv= ";
	for (size_t i = 0; i < iv.size(); i++)
		cout << jv[i];
	
	cout << endl;
	
		
	cout << "vv= " << vv << endl;
	*/

	assert(iv[0] == 3 && iv[2] == 2 && jv[0] == 0 && jv[2] == 1 && round(vv[0]==1) && round(vv[2])==1);

	// Test triu function;
	CompressedMatrix<double, blaze::columnMajor> TriUpper = triu(Graph1comp);

	//cout << "Tri Upper= \n" << TriUpper;

	assert(TriUpper(0, 4) == 1 && TriUpper(0, 4) == 1 && TriUpper(2, 3) == 1);
	assert(isUpper(TriUpper));

	// Test wtedEdgeVertexMat function

	CompressedMatrix<double, blaze::columnMajor>U;

	U = wtedEdgeVertexMat(Graph1comp);

	//cout << "U=\n" << U << endl;

	assert(round(U(0, 1)) == 1 && round(U(0, 2))== -1 && round(U(1, 3)) == -1);

	//Test LLmatp constructor

	/*CompressedMatrix<double, blaze::columnMajor>LMat{ {1, 2, 3}, {4, 5, 6 }, {7, 8, 9} };

	LLmatp<double> llmatp(LMat);

	//print_ll_col(llmatp, 0);

	LLp<double>* ll = llmatp.cols[0];

	assert(round(ll->val) == 7 && round(ll->next->val) == 4 && round(ll->next->next->val) == 1);
	*/

	// Test random function
	Random<double> rnd;

//	for (size_t i = 0; i < 10; i++)
//		cout << rnd.rand0_1()<<" ";
	
//	cout << endl;

//for (size_t i = 0; i < 10; i++)
//		cout << rnd.randn() << " ";

	//cout << endl;

	vector<size_t> col = collect(0, 10);
	//for (size_t i = 0; i < col.size(); i++)
	//cout << col[i] << " ";

	//cout << endl;

	CompressedMatrix<double, blaze::columnMajor> grd2 = grid2<double>(2);
	CompressedMatrix<double, blaze::columnMajor> p = power(grd2, 15);
	//cout << p;

}

	