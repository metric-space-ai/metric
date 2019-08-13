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

namespace laplacians {
	template <typename Tv>
	void matrixToFile(CompressedMatrix<Tv, blaze::columnMajor> A, string FileName) {
		ofstream f(FileName);

		for (size_t i = 0; i < A.rows(); i++)
		{
			for (size_t j = 0; j < A.columns(); j++) {
				f << A(i, j) << ", ";
			}
			f << endl;
		}
	}

	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor>grid2(size_t n, size_t m, Tv isotropy) {

		CompressedMatrix<Tv, blaze::columnMajor>res;

		IJV<Tv> ijv = grid2_ijv<Tv>(n, m);

		res = sparse(ijv);

		return move(res);
	}

	template <typename Tv>
	IJV<Tv> grid2_ijv(size_t n, size_t m, Tv isotropy) {

		IJV<Tv> isograph = isotropy * path_graph_ijv<Tv>(n);
		IJV<Tv> pgi = path_graph_ijv<Tv>(m);
		IJV<Tv> res = product_graph(isograph, pgi);

		return res;
	}

	template <typename Tv>
	IJV<Tv> product_graph(IJV<Tv> b, IJV<Tv> a) {

		size_t n = a.n * b.n;

		assert(a.i.size() == a.nnz);

		DynamicVector<Tv>bncollect(b.n), ancollect(a.n), annzOnes(a.nnz, 1),
			bnnzOnes(b.nnz, 1), anOnes(a.n, 1), bnOnes(b.n, 1);

		for (size_t i = 0; i < b.n; ++i)
			bncollect[i] = (Tv)i;


		for (size_t i = 0; i < a.n; ++i)
			ancollect[i] = (Tv)i;

		DynamicVector<Tv>ait(a.i.size());

		for (size_t i = 0; i < a.i.size(); ++i) {
			ait[i] = (Tv)a.i[i] + 1;
		}

		DynamicVector<Tv>ajt(a.j.size());

		for (size_t i = 0; i < a.j.size(); ++i) {
			ajt[i] = (Tv)a.j[i] + 1;
		}

		DynamicVector<Tv>bit(b.i.size());

		for (size_t i = 0; i < b.i.size(); ++i) {
			bit[i] = (Tv)(b.i[i]);
		}

		DynamicVector<Tv>bjt(b.j.size());

		for (size_t i = 0; i < b.j.size(); ++i) {
			bjt[i] = (Tv)(b.j[i]);
		}

		DynamicVector<Tv> a_edge_from = kron<Tv>(annzOnes, (Tv)a.n * bncollect);
		DynamicVector<Tv> ai = a_edge_from + kron(ait, bnOnes);
		DynamicVector<Tv> aj = a_edge_from + kron(ajt, bnOnes);
		DynamicVector<Tv> av = kron(a.v, bnOnes);

		DynamicVector<Tv> b_edge_from = kron(ancollect, bnnzOnes);
		DynamicVector<Tv> bi = b_edge_from + kron<Tv>(anOnes, bit * (Tv)a.n);
		DynamicVector<Tv> bj = b_edge_from + kron<Tv>(anOnes, bjt * (Tv)a.n);
		DynamicVector<Tv> bv = kron(anOnes, b.v);

		for (size_t i = 0; i < ai.size(); ++i) {
			ai[i] -= 1;
			aj[i] -= 1;
		}

		IJV<Tv>IJVA(n, av.size(), ai, aj, av);
		IJV<Tv>IJVB(n, bv.size(), bi, bj, bv);
		IJV<Tv>IJVRes = IJVA + IJVB;

		return IJVRes;
	}

	template <typename Tv>
	IJV<Tv> path_graph_ijv(size_t n) {
		IJV<Tv> ijv;
		ijv.n = n;
		ijv.nnz = 2 * (n - 1);

		ijv.i.resize(ijv.nnz);
		ijv.j.resize(ijv.nnz);
		ijv.v.resize(ijv.nnz);

		size_t z = 0;

		for (size_t i = 0; i < ijv.n; i++) {
			long long k = i - 1;
			size_t l = i + 1;
			if (k >= 0) {
				ijv.i[z] = i;
				ijv.j[z] = k;
				ijv.v[z] = 1;
				++z;
			}

			if (l < n) {
				ijv.i[z] = i;
				ijv.j[z] = l;
				ijv.v[z] = 1;
				++z;
			}
		}
		return ijv;
	}

	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> forceLap(const CompressedMatrix<Tv, blaze::columnMajor>& a) {

		CompressedMatrix<Tv, blaze::columnMajor> af;

		if (blaze::min(a) < 0) {
			af = blaze::abs(a);
			af = ClearDiag(af);
		}
		else
			if (blaze::sum(blaze::abs(diag(a))) > 0) {
				af = ClearDiag(a);
			}
			else {
				af = a;
			}

		return Diagonal(sum(af)) - af;
	}

	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> lap(CompressedMatrix<Tv, blaze::columnMajor> A) {
		DynamicVector<Tv> ones(A.rows(), 1);
		DynamicVector<Tv> d = A * ones;
		CompressedMatrix<Tv, blaze::columnMajor>Dg = Diagonal(d);
		CompressedMatrix<Tv, blaze::columnMajor>Res = Dg - A;

		return Res;
	}

	template<typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> wtedEdgeVertexMat(const CompressedMatrix<Tv, blaze::columnMajor>& mat) {

		auto [ai, aj, av] = findnz(triu(mat));

		size_t m = ai.size();
		size_t n = mat.rows();

		DynamicVector<Tv> v = sqrt(av);

		CompressedMatrix<Tv, blaze::columnMajor> Sparse1, Sparse2, Res;

		vector<size_t> cl = collect(0, m);

		Sparse1 = sparse(cl, ai, v, m, n, true);

		Sparse2 = sparse(cl, aj, v, m, n);

		Res = Sparse1 - Sparse2;

		return Res;
	}

	template<typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> triu(const CompressedMatrix<Tv, blaze::columnMajor>& A, size_t k) {

		CompressedMatrix<Tv, blaze::columnMajor>Res(A.rows(), A.columns());

		Res.reserve(A.nonZeros() / 2);

		for (size_t i = k; i < A.columns(); ++i) {

			for (auto it = A.begin(i); it != A.end(i); it++) {
				size_t j = it->index();

				if (j < i)
					Res.append(j, i, it->value());
				else
					break;
			}
			Res.finalize(i);
		}

		return Res;
	}
	template<typename Tv>
	pair<Tv, size_t> findmax(const DynamicVector<Tv>& v) {

		size_t index = 0;
		Tv maxvalue = v[0];

		for (size_t i = 1; i < v.size(); ++i) {
			Tv curvalue = v[i];

			if (curvalue > maxvalue) {
				maxvalue = curvalue;
				index = i;
			}
		}

		return pair<Tv, size_t>(maxvalue, index);
	}

	template<typename Tv>
	pair<Tv, size_t> findmax(const CompressedMatrix<Tv, blaze::columnMajor>& A, int wise) {
		size_t index = 0;
		Tv maxvalue;

		if (wise == 1) {
			maxvalue = blaze::max(blaze::row(A, 0));

			for (size_t i = 1; i < A.rows(); i++) {

				Tv curvalue = blaze::max(blaze::row(A, i));

				if (curvalue > maxvalue) {
					maxvalue = curvalue;
					index = i;
				}
			}
		}
		else
			if (wise == 2) {
				maxvalue = blaze::max(blaze::column(A, 0));

				for (size_t i = 1; i < A.columns(); i++) {

					Tv curvalue = blaze::max(blaze::column(A, i));

					if (curvalue > maxvalue) {
						maxvalue = curvalue;
						index = i;
					}
					else throw("The value of wise parameter must be 1 or 2.");
				}
			}

		return pair<Tv, size_t>(maxvalue, index);
	}

	template<typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> sparse(const vector<size_t>& I, const vector<size_t>& J, const DynamicVector<Tv>& V,
		size_t m, size_t n, bool sort) {
		CompressedMatrix<Tv, blaze::columnMajor>res(m, n);

		size_t nnz = I.size();

		res.reserve(nnz);

		size_t curcol = 0;
		size_t col = SIZE_MAX;

		if (sort) {

			vector<size_t> idx = sortedIndices(I, J, V);

			for (size_t l = 0; l < nnz; ++l) {
				if (J[idx[l]] == col) {
					Tv vv = V[idx[l]];
					res.append(I[idx[l]], J[idx[l]], vv);
				}
				else {

					for (size_t k = curcol; k < J[idx[l]]; k++)
					{
						res.finalize(k);
					}
					curcol = col = J[idx[l]];
					Tv vv = V[idx[l]];
					res.append(I[idx[l]], J[idx[l]], vv);
				}
			}
		}
		else {

			for (size_t l = 0; l < nnz; ++l) {
				if (J[l] == col) {
					Tv vv = V[l];
					res.append(I[l], J[l], vv);
				}
				else {

					for (size_t k = curcol; k < J[l]; k++)
					{
						res.finalize(k);
					}
					curcol = col = J[l];
					Tv vv = V[l];
					res.append(I[l], J[l], vv);
				}
			}
		}

		res.finalize(res.columns() - 1);

		return move(res);
	}

	template<typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> kron(const CompressedMatrix<Tv,
		blaze::columnMajor>& A, const CompressedMatrix<Tv, blaze::columnMajor>& B) {
		CompressedMatrix<Tv, blaze::columnMajor> Res(A.rows() * B.rows(), A.columns() * B.columns());

		for (size_t i = 0; i < A.columns(); i++) {

			for (auto it = A.cbegin(i); it != A.cend(i); ++it) {
				size_t j = it->index();
				auto sbm = submatrix(Res, j * B.rows(), i * B.columns(), B.rows(), B.columns());

				sbm = A(j, i) * B;
			}
		}

		return Res;
	}

	template<typename Tv>
	vector<size_t> components(const SparseMatrixCSC<Tv>& mat) {
		size_t n = mat.n;

		vector<size_t> order(n, 0);
		vector<size_t> comp(n, 0);

		size_t color = 0;

		for (size_t x = 0; x != n; x++) {
			if (!comp[x]) { //not used
				comp[x] = ++color; //insert new color

				if (mat.colptr[x + 1] > mat.colptr[x]) {
					size_t ptr = 0, orderLen = 1;

					order[ptr] = x;

					while (ptr < orderLen) {
						size_t curNode = order[ptr]; // initial curNode=x


						for (size_t ind = mat.colptr[curNode]; ind != mat.colptr[curNode + 1]; ++ind) { // cycle by rows
							size_t nbr = mat.rowval[ind]; //nbr=row
							if (!comp[nbr]) { //not used
								comp[nbr] = color; // insert current component
								order[orderLen] = nbr;
								++orderLen;
							}
						}
						++ptr;
					}
				}
			}
		}
		return comp;
	}

	template<typename Tv>
	vector<size_t> components(const CompressedMatrix<Tv, blaze::columnMajor>& mat) {
		size_t n = mat.rows();

		vector<size_t> order(n, 0);
		vector<size_t> comp(n, 0);

		size_t color = 0;

		for (size_t x = 0UL; x < mat.rows(); ++x) {
			if (!comp[x]) { //not used
				comp[x] = ++color; //insert new color

				if (mat.begin(x) != mat.end(x)) {
					size_t ptr = 0, orderLen = 1;
					order[ptr] = x;

					while (ptr < orderLen) {
						size_t curNode = order[ptr]; // initial curNode=x


						for (auto it = mat.begin(curNode); it != mat.end(curNode); ++it) {
							size_t nbr = it->index();
							if (!comp[nbr]) { //not used
								comp[nbr] = color; // insert current component
								order[orderLen] = nbr;
								++orderLen;
							}
						}
						++ptr;
					}
				}
			}
		}
		return comp;
	}

	template<typename Tv>
	tuple<vector<size_t>, vector<size_t>, DynamicVector<Tv>> findnz(const CompressedMatrix<Tv, blaze::columnMajor>& mat) {

		size_t nnz = mat.nonZeros();

		vector<size_t> i(nnz), j(nnz);
		DynamicVector<Tv>v(nnz);

		size_t k = 0;

		//Fill i, row and v
		for (size_t l = 0UL; l < mat.rows(); ++l) {

			for (auto it = mat.cbegin(l); it != mat.cend(l); ++it) {

				i[k] = it->index();
				j[k] = l;
				v[k] = it->value();

				++k;
			}
		}

		return make_tuple(i, j, v);
	}

	inline const vector<vector<size_t>> vecToComps(vector<size_t>& compvec) {

		size_t nc = *max_element(compvec.begin(), compvec.end());

		vector<vector<size_t>> comps(nc);
		vector<size_t> sizes(nc, 0);

		for (size_t i : compvec)
			sizes[i - 1]++;

		for (size_t i = 0; i < nc; i++)
			comps[i].resize(sizes[i]);

		vector<size_t>ptrs(nc, 0);

		for (size_t i = 0; i < compvec.size(); i++)
		{
			size_t c = compvec[i] - 1;

			comps[c][ptrs[c]++] = i;
		}

		return comps;
	}

	template <typename Tv>
	DynamicVector<Tv>indexbool(const DynamicVector<Tv>& vect, const vector<bool>& idx) {

		assert(vect.size() == idx.size());

		vector<Tv> v;

		for (size_t i = 0; i < idx.size(); i++)
			if (idx[i])
				v.push_back(vect[i]);

		DynamicVector<Tv> res(v.size());

		for (size_t i = 0; i < v.size(); i++)
		{
			res[i] = v[i];
		}

		return res;
	}

	template <typename Tv>
	void index(CompressedMatrix<Tv, blaze::columnMajor>& mout, const vector<size_t>& idx, size_t idx2, const DynamicVector<Tv>& vin) {

		assert(idx.size() == vin.size());

		mout.reserve(idx2, idx.size());

		for (size_t i = 0; i < idx.size(); i++) {
			mout.append(idx[i], idx2, vin[i]);
		}
		mout.finalize(idx2);
	}

	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> index(const CompressedMatrix<Tv, blaze::columnMajor>& A,
		const vector<size_t>& idx1, const vector<size_t>& idx2) {

		CompressedMatrix<Tv, blaze::columnMajor>Res(idx1.size(), idx2.size(), 0);

		for (size_t j = 0; j < idx2.size(); j++) {
			Res.reserve(j, idx1.size());

			auto curIt = A.cbegin(idx2[j]);
			for (size_t i = 0; i < idx1.size(); ++i) {

				for (auto it = curIt; it != A.cend(idx2[j]); it++) {

					size_t idx = it->index();
					size_t idx_1 = idx1[i];
					if (idx == idx_1) {
						Tv v = it->value();
						Res.append(i, j, v);
						curIt = it;
						break;
					}
				}
			}
			Res.finalize(j);
		}

		return Res;
	}

	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> extendMatrix(const CompressedMatrix<Tv, blaze::columnMajor>& a,
		DynamicVector<Tv> d)
	{
		assert(a.rows() == d.size());

		if (blaze::sum(blaze::abs(d)) == 0)
			return a;

		DynamicVector<Tv> dpos(d.size() + 1, 0);


		for (size_t i = 0; i < d.size(); i++)
			dpos[i] = d[i] * (d[i] > 0);

		size_t n = d.size();

		CompressedMatrix<Tv, blaze::columnMajor> Res(a.rows() + 1, a.columns() + 1);

		submatrix(Res, 0, 0, a.rows(), a.columns()) = a;
		row(Res, a.rows()) = trans(dpos);
		column(Res, a.columns()) = dpos;

		return Res;
	}

	template <typename Tv>
	CompressedMatrix<Tv, blaze::columnMajor> Diagonal(const DynamicVector<Tv>& V) {

		size_t vsize = V.size();
		CompressedMatrix<Tv, blaze::columnMajor>Res(vsize, vsize);

		Res.reserve(vsize);

		for (size_t i = 0; i < vsize; i++) {
			Res.append(i, i, V[i]);
			Res.finalize(i);
		}

		return Res;
	}

	template<typename Tv>
	vector<size_t> flipIndex(const CompressedMatrix<Tv, blaze::columnMajor>& A) {

		CompressedMatrix<size_t, blaze::columnMajor> B(A.rows(), A.columns());

		size_t k = 1;
		B.reserve(A.nonZeros());

		for (size_t i = 0; i < A.columns(); i++) {

			for (auto it = A.cbegin(i); it != A.cend(i); ++it) {
				size_t j = it->index();
				B.append(j, i, k);
				++k;
			}
			B.finalize(i);
		}

		B = trans(B);

		vector<size_t> resv;
		resv.reserve(B.nonZeros());

		for (size_t i = 0; i < B.columns(); i++) {

			for (auto it = B.cbegin(i); it != B.cend(i); ++it) {
				size_t v = it->value();

				resv.push_back(v - 1);
			}
		}

		return resv;
	}
}
