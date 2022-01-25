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
#ifndef _METRIC_UTILS_SOLVER_APPROXCHOL_CPP
#define _METRIC_UTILS_SOLVER_APPROXCHOL_CPP
#include "../approxchol.hpp"
#include <blaze/Math.h>
#include <vector>

namespace metric {
template <typename Tv> LLmatp<Tv>::LLmatp(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &A)
{
	SparseMatrixCSC<Tv> a(A);

	n = A.rows();
	size_t m = A.nonZeros();

	degs.resize(n);
	std::vector<size_t> flips = flipIndex(A);

	cols.resize(n, NULL);
	llelems.resize(m, NULL);

	for (size_t i = 0; i < n; i++) {
		degs[i] = a.colptr[i + 1] - a.colptr[i];

		size_t ind = a.colptr[i];
		size_t j = a.rowval[ind];
		Tv v = a.nzval[ind];
		LLp<Tv> *llpend = new LLp<Tv>(j, v);
		LLp<Tv> *next = llelems[ind] = llpend;

		for (size_t ind = a.colptr[i] + 1; ind < a.colptr[i + 1]; ind++) {
			size_t j = a.rowval[ind];
			Tv v = a.nzval[ind];
			next = llelems[ind] = new LLp<Tv>(j, v, next);
		}

		cols[i] = next;
	}

	for (size_t i = 0; i < n; i++) {

		for (size_t ind = a.colptr[i]; ind < a.colptr[i + 1]; ind++)
			llelems[ind]->reverse = llelems[flips[ind]];
	}
}

template <typename Tv> void print_ll_col(const LLmatp<Tv> &llmat, const size_t i)
{

	LLp<Tv> *ll = llmat.cols[i];

	std::cout << "col " << i + 1 << " row " << ll->row + 1 << " : " << ll->val << std::endl;
	std::cout << "col " << i + 1 << " reverse->row=" << ll->reverse->row + 1 << std::endl;
	std::cout << "col " << i + 1 << " next->row=" << ll->next->row + 1 << std::endl;

	while (ll->next != ll) {
		ll = ll->next;

		std::cout << "col " << i + 1 << " row " << ll->row + 1 << " : " << ll->val << std::endl;
		std::cout << "col " << i + 1 << " reverse->row=" << ll->reverse->row + 1 << std::endl;
		std::cout << "col " << i + 1 << " next->row=" << ll->next->row + 1 << std::endl;
	}
}

template <typename Tv> void LDLinv<Tv>::debug() const
{
	std::cout << "\ncol=";
	for (size_t i = 0; i < col.size(); i++)
		std::cout << col[i] + 1 << " ";
	std::cout << std::endl;

	std::cout << "\ncolptr=";
	for (size_t i = 0; i < colptr.size(); i++)
		std::cout << colptr[i] + 1 << " ";
	std::cout << std::endl;

	std::cout << "\nrowval=";
	for (size_t i = 0; i < rowval.size(); i++)
		std::cout << rowval[i] + 1 << " ";
	std::cout << std::endl;

	std::cout << "\nfval=";
	for (size_t i = 0; i < fval.size(); i++)
		std::cout << fval[i] << " ";
	std::cout << std::endl;

	std::cout.precision(8);
	std::cout << "\nd=";
	for (size_t i = 0; i < d.size(); i++)
		std::cout << d[i] << " ";
	std::cout << std::endl;
}

inline ApproxCholPQ::ApproxCholPQ(const std::vector<size_t> &a)
{

	n = a.size();
	elems.resize(n);
	lists = std::vector<size_t>(2 * n + 1, SIZE_MAX);
	minlist = 0;

	for (size_t i = 0; i < n; i++) {
		size_t key = a[i] - 1;
		size_t head = lists[key];

		if (head != SIZE_MAX) {
			elems[i] = ApproxCholPQElem(SIZE_MAX, head, key);
			elems[head] = ApproxCholPQElem(i, elems[head].next, elems[head].key);
		} else
			elems[i] = ApproxCholPQElem(SIZE_MAX, SIZE_MAX, key);

		lists[key] = i;
	}

	// Assigned with a default constructor
	nitems = n;
}
/* ApproxCholPQ
	It only implements pop, increment key, and decrement key.
	All nodes with degrees 1 through n appear in their own doubly - linked lists.
	Nodes of higher degrees are bundled together.B
	*/

inline void ApproxCholPQ::move(size_t i, size_t newkey, size_t oldlist, size_t newlist)
{

	size_t prev = elems[i].prev;
	size_t next = elems[i].next;

	// remove i from its old list

	if (next != SIZE_MAX)
		elems[next] = ApproxCholPQElem(prev, elems[next].next, elems[next].key);

	if (prev != SIZE_MAX)
		elems[prev] = ApproxCholPQElem(elems[prev].prev, next, elems[prev].key);
	else
		lists[oldlist] = next;

	// insert i into its new list
	size_t head = lists[newlist];

	if (head != SIZE_MAX)
		elems[head] = ApproxCholPQElem(i, elems[head].next, elems[head].key);

	lists[newlist] = i;
	elems[i] = ApproxCholPQElem(SIZE_MAX, head, newkey);
}

// Increment the key of element i
// This could crash if i exceeds the maxkey

inline void ApproxCholPQ::inc(size_t i)
{

	size_t oldlist = keyMap(elems[i].key, n);
	size_t newlist = keyMap(elems[i].key + 1, n);

	if (newlist != oldlist)
		move(i, elems[i].key + 1, oldlist, newlist);
	else
		elems[i] = ApproxCholPQElem(elems[i].prev, elems[i].next, elems[i].key + 1);
}

inline size_t ApproxCholPQ::pop()
{
	assert(nitems != 0);

	while (lists[minlist] == SIZE_MAX)
		minlist++;

	size_t i = lists[minlist];
	size_t next = elems[i].next;

	lists[minlist] = next;

	if (next != SIZE_MAX)
		elems[next] = ApproxCholPQElem(SIZE_MAX, elems[next].next, elems[next].key);

	nitems--;

	return i + 1;
}

// Decrement the key of element i
// This could crash if i exceeds the maxkey

inline void ApproxCholPQ::dec(size_t i)
{

	size_t oldlist = keyMap(elems[i].key, n);
	size_t newlist = keyMap(elems[i].key - 1, n);

	if (newlist != oldlist) {
		move(i, elems[i].key - 1, oldlist, newlist);

		if (newlist < minlist)
			minlist = newlist;
	} else
		elems[i] = ApproxCholPQElem(elems[i].prev, elems[i].next, elems[i].key - 1);
}

inline void ApproxCholPQ::debug()
{
	std::cout << std::endl << "elems" << std::endl;

	for (size_t i = 0; i < elems.size(); i++) {
		std::cout << "prev= " << elems[i].prev + 1 << " next= " << elems[i].next + 1 << " key= " << elems[i].key + 1
				  << std::endl;
	}

	std::cout << std::endl << "lists" << std::endl;
	for (size_t i = 0; i < lists.size(); i++) {
		std::cout << lists[i] + 1 << " ";
	}

	std::cout << std::endl << std::endl;

	std::cout << "minlist= " << minlist + 1 << " nitems= " << nitems << " n= " << n;
	std::cout << std::endl;
}

template <typename Tv> size_t get_ll_col(const LLmatp<Tv> &llmat, size_t i, std::vector<LLp<Tv> *> &colspace)
{

	LLp<Tv> *ll = llmat.cols[i - 1];
	size_t len = 0;

	while (ll->next != ll) {

		if (ll->val > 0) {
			++len;

			if (len > colspace.size())
				colspace.push_back(ll);
			else
				colspace[len - 1] = ll;
		}

		ll = ll->next;
	}

	if (ll->val > 0) {
		++len;

		if (len > colspace.size())
			colspace.push_back(ll);
		else
			colspace[len - 1] = ll;
	}

	return len;
}

template <typename Tv> size_t compressCol(std::vector<LLp<Tv> *> &colspace, size_t len, ApproxCholPQ &pq)
{

	sort(colspace.begin(), colspace.begin() + len, [](auto x1, auto x2) { return x1->row < x2->row; });

	size_t ptr = 0;
	size_t currow = SIZE_MAX;

	std::vector<LLp<Tv> *> &c = colspace;

	for (size_t i = 0; i < len; i++) {
		if (c[i]->row != currow) {

			currow = c[i]->row;
			c[ptr] = c[i];
			ptr++;
		} else {
			c[ptr - 1]->val += c[i]->val;
			c[i]->reverse->val = 0;

			pq.dec(currow);
		}
	}

	sort(colspace.begin(), colspace.begin() + ptr, [](auto x1, auto x2) { return x1->val < x2->val; });

	return ptr;
}

template <typename Tv> size_t compressCol(std::vector<LLcol<Tv>> &colspace, size_t len)
{
	sort(colspace.begin(), colspace.begin() + 1, [](auto x1, auto x2) { return x1.row < x2.row; });

	std::vector<LLcol<Tv>> &c = colspace;
	size_t ptr = 0;
	size_t currow = c[0].row;
	Tv curval = c[0].val;
	size_t curptr = c[0].ptr;

	for (size_t i = 1; i < len; i++) {
		if (c[i].row != currow) {

			c[ptr] = LLcol<Tv>(currow, curptr, curval); // next is abuse here: really keep where it came from.
			++ptr;

			currow = c[i].row;
			curval = c[i].val;
			curptr = c[i].ptr;
		} else
			curval = curval + c[i].val;
	}

	// emit the last row

	c[ptr] = LLcol<Tv>(currow, curptr, curval);
	ptr++;

	sort(colspace.begin(), colspace.begin() + ptr, [](auto x1, auto x2) { return x1.val < x2.val; });

	return ptr;
}

template <typename Tv> void debugLLp(const std::vector<LLp<Tv> *> &colspace, size_t len)
{

	for (size_t i = 0; i < len; i++) {
		std::cout << "row= " << colspace[i]->row + 1 << " ";
	}
	std::cout << std::endl;

	for (size_t i = 0; i < len; i++) {
		std::cout << "val= " << colspace[i]->val << " ";
	}
	std::cout << std::endl;

	for (size_t i = 0; i < len; i++) {
		std::cout << "reverse->row= " << colspace[i]->reverse->row + 1 << " ";
	}
	std::cout << std::endl;

	for (size_t i = 0; i < len; i++) {
		std::cout << "next->row= " << colspace[i]->next->row + 1 << " ";
	}
	std::cout << std::endl;
}

template <typename Tv> LDLinv<Tv> approxChol(LLmatp<Tv> &a)
{

	size_t n = a.n;
	LDLinv ldli(a);
	size_t ldli_row_ptr = 0;

	std::vector<Tv> d(n, 0);
	ApproxCholPQ pq(a.degs);

	size_t it = 0;

	std::vector<LLp<Tv> *> colspace(n);
	std::vector<Tv> cumspace(n);

	std::vector<Tv> vals(n);
	size_t len = 0;

	while (it + 1 < n) {

		size_t i = pq.pop();

		ldli.col[it] = i - 1;
		ldli.colptr[it] = ldli_row_ptr;

		it++;

		len = get_ll_col(a, i, colspace); // changed

		len = compressCol(colspace, len, pq);

		Tv csum = 0;

		for (size_t ii = 0; ii < len; ii++) {
			vals[ii] = colspace[ii]->val;
			csum += colspace[ii]->val;
			cumspace[ii] = csum;
		}

		Tv wdeg = csum;
		Tv colScale = 1;

		Random<double> rnd;

		for (size_t joffset = 0; joffset + 1 < len; joffset++) {
			LLp<Tv> *ll = colspace[joffset];

			Tv w = vals[joffset] * colScale;
			size_t j = ll->row;

			LLp<Tv> *revj = ll->reverse;

			Tv f = w / wdeg;

			vals[joffset] = 0;

			Tv rand0_1 = rnd.rand0_1();
			Tv r = rand0_1 * (csum - cumspace[joffset]) + cumspace[joffset];
			// Tv r = 0.2 * (csum - cumspace[joffset]) + cumspace[joffset];

			auto firstit = find_if(cumspace.begin(), cumspace.begin() + len, [=](auto x) { return x > r; });

			size_t koff;

			if (firstit != cumspace.end())
				koff = firstit - cumspace.begin();
			else
				koff = 0;

			size_t k = colspace[koff]->row;

			pq.inc(k);

			Tv newEdgeVal = f * (1 - f) * wdeg;

			// fix row k in col j
			revj->row = k; // dense time hog: presumably because of cache
			revj->val = newEdgeVal;
			revj->reverse = ll;

			// fix row j in col k

			LLp<Tv> *khead = a.cols[k];
			a.cols[k] = ll;
			ll->next = khead;
			ll->reverse = revj;
			ll->val = newEdgeVal;
			ll->row = j;

			colScale = colScale * (1 - f);
			wdeg = wdeg * std::pow(1 - f, 2);

			ldli.rowval.push_back(j);
			ldli.fval.push_back(f);
			ldli_row_ptr++;
		}

		LLp<Tv> *ll = colspace[len - 1];
		Tv w = vals[len - 1] * colScale;
		size_t j = ll->row;
		LLp<Tv> *revj = ll->reverse;

		if (it + 1 < n) {
			pq.dec(j);
		}

		revj->val = 0;

		ldli.rowval.push_back(j);
		ldli.fval.push_back(1);
		ldli_row_ptr++;

		d[i - 1] = w;
	}

	ldli.colptr[it] = ldli_row_ptr;
	ldli.d = d;

	return ldli;
}

template <typename Tv> void LDLinv<Tv>::forward(blaze::DynamicVector<Tv> &y) const
{

	for (size_t ii = 0; ii < col.size(); ii++) {
		size_t i = col[ii];

		size_t j0 = colptr[ii];
		size_t j1 = colptr[ii + 1] - 1;

		Tv yi = y[i];

		for (size_t jj = j0; jj < j1; jj++) {
			size_t j = rowval[jj];
			y[j] += fval[jj] * yi;
			yi *= (1 - fval[jj]);
		}

		size_t j = rowval[j1];
		y[j] += yi;
		y[i] = yi;
	}
}
template <typename Tv> void LDLinv<Tv>::backward(blaze::DynamicVector<Tv> &y) const
{

	size_t sz = col.size();

	for (size_t ii = 0; ii < sz; ++ii) {

		size_t iib = sz - ii - 1;
		size_t i = col[iib];

		size_t j0 = colptr[iib];
		size_t j1 = colptr[iib + 1] - 1;

		size_t j = rowval[j1];
		Tv yi = y[i] + y[j];

		for (size_t jj = 0; jj < j1 - j0; jj++) {
			size_t jjb = j1 - jj - 1;

			size_t j = rowval[jjb];
			yi = (1 - fval[jjb]) * yi + fval[jjb] * y[j];
		}
		y[i] = yi;
	}
}
// The routines that do the solve.

template <typename Tv> blaze::DynamicVector<Tv> LDLinv<Tv>::LDLsolver(const blaze::DynamicVector<Tv> &b) const
{

	blaze::DynamicVector<Tv> y = b;

	forward(y);

	for (size_t i = 0; i < d.size(); i++)

		if (d[i] != 0)
			y[i] /= d[i];

	backward(y);

	Tv mu = mean(y);

	for (size_t i = 0; i < y.size(); i++)
		y[i] = y[i] - mu;

	return y;
}

template <typename Tv>
SubSolver<Tv> approxchol_lapGreedy(const blaze::CompressedMatrix<Tv, blaze::columnMajor> &a,
								   std::vector<size_t> &pcgIts, float tol, double maxits, double maxtime, bool verbose,
								   ApproxCholParams params)
{

	blaze::CompressedMatrix<Tv, blaze::columnMajor> la = lap(a);
	LLmatp<Tv> llmat(a);
	LDLinv<Tv> ldli = approxChol(llmat);

	SolverB<Tv> F = [=](const blaze::DynamicVector<Tv> &b) { return ldli.LDLsolver(b); };

	return SubSolver<Tv>([=](const blaze::DynamicVector<Tv> &b, std::vector<size_t> &pcgIts) {
		Tv mn = mean(b);
		blaze::DynamicVector<Tv> b1(b.size());

		for (size_t i = 0; i < b.size(); i++)
			b1[i] = b[i] - mn;

		blaze::DynamicVector<Tv> res = pcg(la, b1, F, pcgIts, tol, maxits, maxtime, verbose, params.stag_test);
		return res;
	});
}

} // namespace metric

#endif
