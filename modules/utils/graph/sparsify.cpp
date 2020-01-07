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
#ifndef _METRIC_UTILS_GRAPH_SPARSIFY_CPP
#define _METRIC_UTILS_GRAPH_SPARSIFY_CPP

namespace metric {
template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> sparsify(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a, float ep, float matrixConcConst, float JLfac)
{

    Random<double> rnd;
    std::vector<size_t> pcgIts;

    SolverB<Tv> f = approxchol_lap(a, pcgIts, 1e-2F);

    size_t n = a.rows();
    size_t k = (size_t)round(JLfac * std::log(n));  //number of dims for JL

    blaze::CompressedMatrix<Tv, blaze::columnMajor> U = wtedEdgeVertexMat(a);

    size_t m = U.rows();

    blaze::DynamicMatrix<double, blaze::columnMajor> R(m, k);

    for (size_t i = 0; i < m; i++) {

        for (size_t j = 0; j < k; j++)
            R(i, j) = rnd.randn();
    }
    blaze::CompressedMatrix<Tv, blaze::columnMajor> UR = adjoint(U) * R;

    blaze::CompressedMatrix<Tv, blaze::columnMajor> V(n, k, 0);

    for (size_t i = 0; i < k; i++) {
        blaze::DynamicVector<Tv> x, b;

        std::vector<size_t> idx = collect(0, UR.rows());
        b = index(UR, idx, i);

        x = f(b);

        idx = collect(0, V.rows());
        index(V, idx, i, x);
    }

    auto [ai, aj, av] = findnz(triu(a));

    blaze::DynamicVector<Tv> prs(av.size());

    for (size_t h = 0; h < av.size(); h++) {
        size_t i = ai[h];
        size_t j = aj[h];

        blaze::DynamicVector<Tv> vi, vj, vr;

        std::vector<size_t> idx = collect(0, V.columns());
        vi = index(V, i, idx);
        vj = index(V, j, idx);
        Tv nr = std::pow(norm(vi - vj), 2) / k;
        Tv tmp = av[h] * nr * matrixConcConst * std::log(n) / std::pow(ep, 2);
        prs[h] = (1 < tmp) ? 1 : tmp;
    }

    std::vector<bool> ind(prs.size());
    blaze::DynamicVector<double> rndvec = rnd.randv(prs.size());

    for (size_t i = 0; i < prs.size(); i++)
        ind[i] = rndvec[i] < prs[i];

    std::vector<size_t> ai_ind = indexbool(ai, ind);
    std::vector<size_t> aj_ind = indexbool(aj, ind);
    blaze::DynamicVector<Tv> av_ind = indexbool(av, ind);
    blaze::DynamicVector<Tv> prs_ind = indexbool(prs, ind);

    blaze::DynamicVector<Tv> divs = av_ind / prs_ind;

    blaze::CompressedMatrix<Tv, blaze::columnMajor> as = sparse(ai_ind, aj_ind, divs, n, n);

    as = as + adjoint(as);

    return as;
}

namespace kruskal_sparsify_detail {
    
template <typename Tv>
class KruskalEdge {
private:
    size_t node_from;
    size_t node_to;
    Tv weight;
    bool enabled;
public:
    KruskalEdge(size_t node_from, size_t node_to, Tv weight) :
        node_from(node_from), node_to(node_to), weight(weight),
        enabled(false) {
    }

    KruskalEdge(KruskalEdge&& e) noexcept : node_from(e.node_from),
                                            node_to(e.node_to),
                                            weight(std::move(e.weight)),
                                            enabled(e.enabled) {}
    KruskalEdge& operator=(KruskalEdge&& e) {
        if (this != &e) {
            node_from = e.node_from;
            node_to = e.node_to;
            weight = std::move(e.weight);
            enabled = e.enabled;
        }
        return *this;
    }

    inline const Tv& getWeight() const {
        return weight;
    }
    inline size_t getNodeFrom() const {
        return node_from;
    }
    inline size_t getNodeTo() const {
        return node_to;
    }
    inline void enable() {
        enabled = true;
    }
    inline bool isEnabled() const {
        return enabled;
    }
};
    
class KruskalNode {
private:
    KruskalNode* parent;
    size_t size;
    KruskalNode* find() {
        if (!parent)
            return this;
        KruskalNode* p = parent->find();
        parent = p;
        return p;
    }
    void union_with(KruskalNode* node) {
        KruskalNode* root0 = find();
        KruskalNode* root1 = node->find();
        root0->size += root1->size;
        root1->parent = root0;
    }
public:
    KruskalNode() : parent(0), size(1) {}
    inline bool isConnected(KruskalNode& node) {
        return find() == node.find();
    }
    
    inline void connect(KruskalNode& node) {
        if (node.size > size)
            node.union_with(this);
        else
            union_with(&node);
    }
    
    std::string toString() {
        std::stringstream ss;
        ss << this << ":" << parent << ":" << size;
        return ss.str();
    }
};
    
}

template <typename Tv>
blaze::CompressedMatrix<Tv, blaze::columnMajor> kruskal_sparsify(
    const blaze::CompressedMatrix<Tv, blaze::columnMajor>& a, bool minimum) {
    if (a.columns() != a.rows())
        throw std::invalid_argument("expected square matrix");

    using namespace kruskal_sparsify_detail;

    const size_t node_count = a.columns();    
    const size_t edge_count = a.nonZeros();

    // initializing edge list
    std::vector<KruskalEdge<Tv> > edges;
    edges.reserve(edge_count);
    for (size_t node_from = 0; node_from < node_count; node_from++) {
        // not checking elements above diagonal (graph is undirected)
        for (auto i = a.begin(node_from); i != a.end(node_from); ++i) {
            size_t node_to = i->index();
            
            if (node_to >= node_from)
                break;
            
            Tv weight = i->value();
            edges.push_back(KruskalEdge<Tv>(node_from, node_to, weight));
        }
    }

    // sorting edge list
    if (minimum) {
        sort(edges.begin(), edges.end(), [](const KruskalEdge<Tv>& a,
                                            const KruskalEdge<Tv>& b) {
                 return a.getWeight() < b.getWeight();
             });
    } else {
        sort(edges.begin(), edges.end(), [](const KruskalEdge<Tv>& a,
                                            const KruskalEdge<Tv>& b) {
                 return a.getWeight() > b.getWeight();
             });
    }

    // initializing node list (needed to have disjoint-set data structure)
    std::vector<KruskalNode> nodes;
    nodes.resize(node_count);
    
    // traversing edge list, addition happens only if no loops are
    // created in the process
    size_t new_edge_count = 0;
    for (auto& i : edges) {
        KruskalNode& node_from = nodes[i.getNodeFrom()];
        KruskalNode& node_to = nodes[i.getNodeTo()];

        if (!node_from.isConnected(node_to)) {
            node_from.connect(node_to);
            i.enable();
            new_edge_count++;
        }
    }

    // putting together result
    blaze::CompressedMatrix<Tv, blaze::columnMajor> res(node_count,
                                                        node_count);
    res.reserve(new_edge_count * 2);

    for (auto& i : edges) {
        if (i.isEnabled()) {
            res(i.getNodeTo(), i.getNodeFrom()) = i.getWeight();
            res(i.getNodeFrom(), i.getNodeTo()) = i.getWeight();
        }
    }
    return res;
}
    
}

#endif
