/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_DETAILS_K_STRUCTURED_EMD_CPP
#define _METRIC_DISTANCE_DETAILS_K_STRUCTURED_EMD_CPP
#include "EMD.hpp"

/*Fast and Robust Earth Mover's Distances
  Ofir Pele, Michael Werman
  ICCV 2009
  Original implementation by Ofir Pele 2009-2012, 
  Refactoring, API change and performance optimization by Michael Welsch, 2018
  All rights reserved.
*/

#include <set>
// #include <limits>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <vector>
#include <list>
#include <type_traits>
#include <utility>
#include <iostream>

#include <cassert> // uncommented by Max F in order to build under Clang & QMake
template<typename T>
inline void print_vector(const std::vector<T> & v) {
    std::cout << "[ ";
    for(auto r : v) {
        std::cout << r << " ";
    }
    std::cout << "]";
}
template<typename T>
inline void print_matrix(const std::vector<std::vector<T>> &m) {
    std::cout << "[ ";
    for(auto & r : m) {
        print_vector(r); std::cout << std::endl;
    }
    std::cout << "]";
}

namespace metric
{
    namespace distance
    {
    
        namespace EMD_details
        {
        
// // disabled by Max F because of no need after rolling back to original code
template <typename Container>
typename Container::value_type::value_type
max_in_distance_matrix(const Container &C)
{
   typedef typename Container::value_type::value_type T;
   T max = 0;
   for (size_t i = 0; i < C.size(); ++i)
   {
       {
           for (size_t j = 0; j < C[0].size(); ++j)
           {
               if (C[i][j] > max)
                   max = C[i][j];
           }
       }
   }

   return max;
}

            template <typename Container>
            struct Euclidian_thresholded_EMD_default
            {
                typedef typename Container::value_type T;
                static_assert(
                    std::is_floating_point<T>::value, "T must be a float type");

                T thres, factor;
            
                T operator()(const Container &a, const Container &b) const
                {
                    T sum = 0;
                    for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2)
                    {
                        sum += (*it1 - *it2) * (*it1 - *it2);
                    }
                    return std::min(thres, T(factor * sqrt(sum)));
                }
                Euclidian_thresholded_EMD_default(T thres = 1000, T factor = 3000) : thres(thres), factor(factor) {}
            };
        
            template <typename T, typename Metric = Euclidian_thresholded_EMD_default<std::vector<double>>>
            std::vector<std::vector<T>>
            ground_distance_matrix_of_2dgrid(size_t cols, size_t rows)
            {
                size_t n = rows * cols;
                Metric distance;
                std::vector<std::vector<T>> distM(n, std::vector<T>(n));
            
                size_t j = -1;
                for (size_t c1 = 0; c1 < cols; ++c1)
                {
                    for (size_t r1 = 0; r1 < rows; ++r1)
                    {
                        ++j;
                        size_t i = -1;
                        for (size_t c2 = 0; c2 < cols; ++c2)
                        {
                            for (size_t r2 = 0; r2 < rows; ++r2)
                            {
                                ++i;
                                std::vector<double> p = {static_cast<double>(r1), static_cast<double>(c1)};
                                std::vector<double> q = {static_cast<double>(r2), static_cast<double>(c2)};
                                distM[i][j] = distance(p, q);
                            }
                        }
                    }
                }
                return distM;
            }

            enum FLOW_TYPE_T
            {
                NO_FLOW = 0,
                WITHOUT_TRANSHIPMENT_FLOW,
                WITHOUT_EXTRA_MASS_FLOW
            };

/// returns the flow from/to transhipment vertex given flow F which was computed using
/// FLOW_TYPE_T of kind WITHOUT_TRANSHIPMENT_FLOW.
            template <typename T, typename C>
            void return_flow_from_to_transhipment_vertex(const std::vector<std::vector<T>> &F,
                                                         const C &P,
                                                         const C &Q,
                                                         std::vector<T> &flow_from_P_to_transhipment,
                                                         std::vector<T> &flow_from_transhipment_to_Q)
            {

                flow_from_P_to_transhipment.assign(std::begin(P), std::end(P));
                flow_from_transhipment_to_Q.assign(std::begin(Q), std::end(Q));
                for (size_t i = 0; i < P.size(); ++i)
                {
                    for (size_t j = 0; j < P.size(); ++j)
                    {
                        flow_from_P_to_transhipment[i] -= F[i][j];
                        flow_from_transhipment_to_Q[j] -= F[i][j];
                    }
                }

            } // return_flow_from_to_transhipment_vertex

/// Transforms the given flow F which was computed using FLOW_TYPE_T of kind WITHOUT_TRANSHIPMENT_FLOW,
/// to a flow which can be computed using WITHOUT_EXTRA_MASS_FLOW. If you want the flow to the extra mass,
/// you can use return_flow_from_to_transhipment_vertex on the returned F.
            template <typename T, typename C>
            void transform_flow_to_regular(std::vector<std::vector<T>> &F,
                                           const C &P,
                                           const C &Q)
            {
            
                const size_t N = P.size();
                std::vector<T> flow_from_P_to_transhipment(N);
                std::vector<T> flow_from_transhipment_to_Q(N);
                return_flow_from_to_transhipment_vertex(F, P, Q,
                                                        flow_from_P_to_transhipment,
                                                        flow_from_transhipment_to_Q);
            
                size_t i = 0;
                size_t j = 0;
                while (true)
                {
                
                    while (i < N && flow_from_P_to_transhipment[i] == 0)
                        ++i;
                    while (j < N && flow_from_transhipment_to_Q[j] == 0)
                        ++j;
                    if (i == N || j == N)
                        break;

                    if (flow_from_P_to_transhipment[i] < flow_from_transhipment_to_Q[j])
                    {
                        F[i][j] += flow_from_P_to_transhipment[i];
                        flow_from_transhipment_to_Q[j] -= flow_from_P_to_transhipment[i];
                        flow_from_P_to_transhipment[i] = 0;
                    }
                    else
                    {
                        F[i][j] += flow_from_transhipment_to_Q[j];
                        flow_from_P_to_transhipment[i] -= flow_from_transhipment_to_Q[j];
                        flow_from_transhipment_to_Q[j] = 0;
                    }
                }

            } // transform_flow_to_regular

// template <typename T, FLOW_TYPE_T FLOW_TYPE = NO_FLOW>
// struct EMD
// {
//     //typedef typename Container::value_type T;
//     T operator()(const std::vector<T> &P, const std::vector<T> &Q,
//                  const std::vector<std::vector<T>> &C,
//                  T extra_mass_penalty = -1,
//                  std::vector<std::vector<T>> *F = NULL);
// };

//------------------------------------------------------------------------------
            template <typename T>
            struct edge
            {
                edge(size_t to, T cost) : _to(to), _cost(cost) {}
                size_t _to;
                T _cost;
            };
        
            template <typename T>
            struct edgeCompareByCost
            {
                bool operator()(const edge<T> &a, const edge<T> &b)
                {
                    return a._cost < b._cost;
                }
            };
        
            template <typename T>
            struct edge0
            {
                edge0(size_t to, T cost, T flow) : _to(to), _cost(cost), _flow(flow) {}
                size_t _to;
                T _cost;
                T _flow;
            };
        
            template <typename T>
            struct edge1
            {
                edge1(size_t to, T reduced_cost) : _to(to), _reduced_cost(reduced_cost) {}
                size_t _to;
                T _reduced_cost;
            };
        
            template <typename T>
            struct edge2
            {
                edge2(size_t to, T reduced_cost, T residual_capacity) : _to(to), _reduced_cost(reduced_cost), _residual_capacity(residual_capacity) {}
                size_t _to;
                T _reduced_cost;
                T _residual_capacity;
            };
        
            template <typename T>
            struct edge3
            {
                edge3(size_t to = 0, T dist = 0) : _to(to), _dist(dist) {}
                size_t _to;
                T _dist;
            };
//------------------------------------------------------------------------------
        
//------------------------------------------------------------------------------
            template <typename T>
            class min_cost_flow
            {

                size_t _num_nodes;
                std::vector<size_t> _nodes_to_Q;

            public:
                // e - supply(positive) and demand(negative).
                // c[i] - edges that goes from node i. first is the second nod
                // x - the flow is returned in it
                T operator()(std::vector<T> &e,
                             const std::vector<std::list<edge<T>>> &c,
                             std::vector<std::list<edge0<T>>> &x)
                {
                    assert(e.size() == c.size());
                    assert(x.size() == c.size());

                    _num_nodes = e.size();
                    _nodes_to_Q.resize(_num_nodes);

                    // init flow
                    {
                        for (size_t from = 0; from < _num_nodes; ++from)
                        {
                            {
                                for (typename std::list<edge<T>>::const_iterator it = c[from].begin(); it != c[from].end(); ++it)
                                {
                                    x[from].push_back(edge0<T>(it->_to, it->_cost, 0));
                                    x[it->_to].push_back(edge0<T>(from, -it->_cost, 0));
                                }
                            } // it
                        }
                    } // from

                    // reduced costs for forward edges (c[i,j]-pi[i]+pi[j])
                    // Note that for forward edges the residual capacity is infinity
                    std::vector<std::list<edge1<T>>> r_cost_forward(_num_nodes);
                    {
                        for (size_t from = 0; from < _num_nodes; ++from)
                        {
                            {
                                for (typename std::list<edge<T>>::const_iterator it = c[from].begin(); it != c[from].end(); ++it)
                                {
                                    r_cost_forward[from].push_back(edge1<T>(it->_to, it->_cost));
                                }
                            }
                        }
                    }

                    // reduced costs and capacity for backward edges (c[j,i]-pi[j]+pi[i])
                    // Since the flow at the beginning is 0, the residual capacity is also zero
                    std::vector<std::list<edge2<T>>> r_cost_cap_backward(_num_nodes);
                    {
                        for (size_t from = 0; from < _num_nodes; ++from)
                        {
                            {
                                for (typename std::list<edge<T>>::const_iterator it = c[from].begin(); it != c[from].end(); ++it)
                                {
                                    r_cost_cap_backward[it->_to].push_back(edge2<T>(from, -it->_cost, 0));
                                }
                            } // it
                        }
                    } // from

                    // Max supply TODO:demand?, given U?, optimization-> min out of demand,supply
                    T U = 0;
                    {
                        for (size_t i = 0; i < _num_nodes; ++i)
                        {
                            if (e[i] > U)
                                U = e[i];
                        }
                    }
                    T delta = static_cast<T>(pow(2.0l, ceil(log(static_cast<long double>(U)) / log(2.0))));

                    std::vector<T> d(_num_nodes);
                    std::vector<size_t> prev(_num_nodes);
                    delta = 1;

                    while (true)
                    { //until we break when S or T is empty

                        T maxSupply = 0;
                        size_t k = 0;
                        for (size_t i = 0; i < _num_nodes; ++i)
                        {
                            if (e[i] > 0)
                            {
                                if (maxSupply < e[i])
                                {
                                    maxSupply = e[i];
                                    k = i;
                                }
                            }
                        }
                        if (maxSupply == 0) {
                            break;
                        }
                        delta = maxSupply;

                        size_t l = 0;
                        compute_shortest_path(d, prev, k, r_cost_forward, r_cost_cap_backward, e, l);
                        //---------------------------------------------------------------
                        // find delta (minimum on the path from k to l)
                        //delta= e[k];
                        //if (-e[l]<delta) delta= e[k];
                        size_t to = l;
                        do
                        {
                            size_t from = prev[to];
                            assert(from != to);

                            // residual
                            typename std::list<edge2<T>>::iterator itccb = r_cost_cap_backward[from].begin();
                            while ((itccb != r_cost_cap_backward[from].end()) && (itccb->_to != to))
                            {
                                ++itccb;
                            }
                            if (itccb != r_cost_cap_backward[from].end())
                            {
                                if (itccb->_residual_capacity < delta)
                                    delta = itccb->_residual_capacity;
                            }

                            to = from;
                        } while (to != k);
                        //---------------------------------------------------------------

                        //---------------------------------------------------------------
                        // augment delta flow from k to l (backwards actually...)
                        to = l;
                        do
                        {
                            size_t from = prev[to];
                            assert(from != to);

                            // TODO - might do here O(n) can be done in O(1)
                            typename std::list<edge0<T>>::iterator itx = x[from].begin();
                            while (itx->_to != to)
                            {
                                ++itx;
                            }
                            itx->_flow += delta;

                            // update residual for backward edges
                            typename std::list<edge2<T>>::iterator itccb = r_cost_cap_backward[to].begin();
                            while ((itccb != r_cost_cap_backward[to].end()) && (itccb->_to != from))
                            {
                                ++itccb;
                            }
                            if (itccb != r_cost_cap_backward[to].end())
                            {
                                itccb->_residual_capacity += delta;
                            }
                            itccb = r_cost_cap_backward[from].begin();
                            while ((itccb != r_cost_cap_backward[from].end()) && (itccb->_to != to))
                            {
                                ++itccb;
                            }
                            if (itccb != r_cost_cap_backward[from].end())
                            {
                                itccb->_residual_capacity -= delta;
                            }

                            // update e
                            e[to] += delta;
                            e[from] -= delta;

                            to = from;
                        } while (to != k);
                        //---------------------------------------------------------------------------------

                    } // while true (until we break when S or T is empty)

                    // compute distance from x
                    T dist = 0;
                    {
                        for (size_t from = 0; from < _num_nodes; ++from)
                        {
                            {
                                for (typename std::list<edge0<T>>::const_iterator it = x[from].begin(); it != x[from].end(); ++it)
                                {
                                    //                        if (it->_flow!=0) cout << from << "->" << it->_to << ": " << it->_flow << "x" << it->_cost << endl;
                                    dist += (it->_cost * it->_flow);
                                }
                            } // it
                        }
                    } // from

                    return dist;
                } // operator()

            private:
                void compute_shortest_path(std::vector<T> &d,
                                           std::vector<size_t> &prev,

                                           size_t from,
                                           std::vector<std::list<edge1<T>>> &cost_forward,
                                           std::vector<std::list<edge2<T>>> &cost_backward,

                                           const std::vector<T> &e,
                                           size_t &l)
                {

                    //----------------------------------------------------------------
                    // Making heap (all inf except 0, so we are saving comparisons...)
                    //----------------------------------------------------------------
                    std::vector<edge3<T>> Q(_num_nodes);

                    Q[0]._to = from;
                    _nodes_to_Q[from] = 0;
                    Q[0]._dist = 0;

                    size_t j = 1;
                    // TODO: both of these into a function?
                    {
                        for (size_t i = 0; i < from; ++i)
                        {
                            Q[j]._to = i;
                            _nodes_to_Q[i] = j;
                            Q[j]._dist = std::numeric_limits<T>::max();
                            ++j;
                        }
                    }

                    {
                        for (size_t i = from + 1; i < _num_nodes; ++i)
                        {
                            Q[j]._to = i;
                            _nodes_to_Q[i] = j;
                            Q[j]._dist = std::numeric_limits<T>::max();
                            ++j;
                        }
                    }
                    //----------------------------------------------------------------

                    //----------------------------------------------------------------
                    // main loop
                    //----------------------------------------------------------------
                    std::vector<size_t> finalNodesFlg(_num_nodes, false);
                    do
                    {
                        size_t u = Q[0]._to;

                        d[u] = Q[0]._dist; // final distance
                        finalNodesFlg[u] = true;
                        if (e[u] < 0)
                        {
                            l = u;
                            break;
                        }

                        heap_remove_first(Q, _nodes_to_Q);

                        // neighbors of u
                        {
                            for (typename std::list<edge1<T>>::const_iterator it = cost_forward[u].begin(); it != cost_forward[u].end(); ++it)
                            {
                                assert(it->_reduced_cost >= 0);
                                T alt = d[u] + it->_reduced_cost;
                                size_t v = it->_to;
                                if ((_nodes_to_Q[v] < Q.size()) && (alt < Q[_nodes_to_Q[v]]._dist))
                                {
                                    //cout << "u to v==" << u << " to " << v << "   " << alt << endl;
                                    heap_decrease_key(Q, _nodes_to_Q, v, alt);
                                    prev[v] = u;
                                }
                            }
                        } //it
                        {
                            for (typename std::list<edge2<T>>::const_iterator it = cost_backward[u].begin(); it != cost_backward[u].end(); ++it)
                            {
                                if (it->_residual_capacity > 0)
                                {
                                    assert(it->_reduced_cost >= 0);
                                    T alt = d[u] + it->_reduced_cost;
                                    size_t v = it->_to;
                                    if ((_nodes_to_Q[v] < Q.size()) && (alt < Q[_nodes_to_Q[v]]._dist))
                                    {
                                        //cout << "u to v==" << u << " to " << v << "   " << alt << endl;
                                        heap_decrease_key(Q, _nodes_to_Q, v, alt);
                                        prev[v] = u;
                                    }
                                }
                            }
                        } //it

                    } while (!Q.empty());

                    //---------------------------------------------------------------------------------
                    // reduced costs for forward edges (c[i,j]-pi[i]+pi[j])
                    {
                        for (size_t from = 0; from < _num_nodes; ++from)
                        {
                            {
                                for (typename std::list<edge1<T>>::iterator it = cost_forward[from].begin();
                                     it != cost_forward[from].end(); ++it)
                                {
                                    if (finalNodesFlg[from])
                                    {
                                        it->_reduced_cost += d[from] - d[l];
                                    }
                                    if (finalNodesFlg[it->_to])
                                    {
                                        it->_reduced_cost -= d[it->_to] - d[l];
                                    }
                                }
                            }
                        }
                    }

                    // reduced costs and capacity for backward edges (c[j,i]-pi[j]+pi[i])
                    {
                        for (size_t from = 0; from < _num_nodes; ++from)
                        {
                            {
                                for (typename std::list<edge2<T>>::iterator it = cost_backward[from].begin();
                                     it != cost_backward[from].end(); ++it)
                                {
                                    if (finalNodesFlg[from])
                                    {
                                        it->_reduced_cost += d[from] - d[l];
                                    }
                                    if (finalNodesFlg[it->_to])
                                    {
                                        it->_reduced_cost -= d[it->_to] - d[l];
                                    }
                                }
                            } // it
                        }
                    }
                    //---------------------------------------------------------------------------------

                    //----------------------------------------------------------------

                } // compute_shortest_path

                void heap_decrease_key(std::vector<edge3<T>> &Q, std::vector<size_t> &nodes_to_Q,
                                       size_t v, T alt)
                {
                    size_t i = nodes_to_Q[v];
                    Q[i]._dist = alt;
                    while (i > 0 && Q[PARENT(i)]._dist > Q[i]._dist)
                    {
                        swap_heap(Q, nodes_to_Q, i, PARENT(i));
                        i = PARENT(i);
                    }
                } // heap_decrease_key

                void heap_remove_first(std::vector<edge3<T>> &Q, std::vector<size_t> &nodes_to_Q)
                {
                    swap_heap(Q, nodes_to_Q, 0, Q.size() - 1);
                    Q.pop_back();
                    heapify(Q, nodes_to_Q, 0);
                } // heap_remove_first

                void heapify(std::vector<edge3<T>> &Q, std::vector<size_t> &nodes_to_Q,
                             size_t i)
                {

                    do
                    {
                        // TODO: change to loop
                        size_t l = LEFT(i);
                        size_t r = RIGHT(i);
                        size_t smallest;
                        if ((l < Q.size()) && (Q[l]._dist < Q[i]._dist))
                        {
                            smallest = l;
                        }
                        else
                        {
                            smallest = i;
                        }
                        if ((r < Q.size()) && (Q[r]._dist < Q[smallest]._dist))
                        {
                            smallest = r;
                        }

                        if (smallest == i)
                            return;

                        swap_heap(Q, nodes_to_Q, i, smallest);
                        i = smallest;

                    } while (true);

                } // end heapify

                void swap_heap(std::vector<edge3<T>> &Q, std::vector<size_t> &nodes_to_Q, size_t i, size_t j)
                {
                    edge3<T> tmp = Q[i];
                    Q[i] = Q[j];
                    Q[j] = tmp;
                    nodes_to_Q[Q[j]._to] = j;
                    nodes_to_Q[Q[i]._to] = i;
                } // swap_heapify

                size_t LEFT(size_t i)
                {
                    return 2 * (i + 1) - 1;
                }

                size_t RIGHT(size_t i)
                {
                    return 2 * (i + 1); // 2*(i+1)+1-1
                }

                size_t PARENT(size_t i)
                {
                    return (i - 1) / 2;
                }

            }; // end min_cost_flow

            template <typename T>
            void fillFWithZeros(std::vector<std::vector<T>> &F)
            {
                for (size_t i = 0; i < F.size(); ++i)
                {
                    for (size_t j = 0; j < F[i].size(); ++j)
                    {
                        F[i][j] = 0;
                    }
                }
            }

//Forward declarations
            template <typename T, FLOW_TYPE_T FLOW_TYPE>
            struct emd_impl;

/*
  Main implementation
*/
            template <typename Container, FLOW_TYPE_T FLOW_TYPE>
            struct emd_impl_integral_types
            {
                typedef typename Container::value_type T;
                T operator()(
                    const Container &POrig, const Container &QOrig,
                    const Container &Pc, const Container &Qc, // P, Q, C replaced with Pc, Qc, Cc by Max F
                    const std::vector<std::vector<T>> &Cc,
                    //T maxC, // disabled by MaxF //now updated inside
                    T extra_mass_penalty,
                    std::vector<std::vector<T>> *F//,
                    // T abs_diff_sum_P_sum_Q // disabled by MaxF //now updated inside
                    )
                {

                    //-------------------------------------------------------
                    size_t N = Pc.size();
                    assert(Qc.size() == N);

                    // bool needToSwapFlow = false; // commented by Max F
                    // re-inserted by MAx F from the original code
                    // Ensuring that the supplier - P, have more mass.
                    std::vector<T> P;
                    std::vector<T> Q;
                    std::vector< std::vector<T> > C(Cc);
                    T abs_diff_sum_P_sum_Q;
                    T sum_P= 0;
                    T sum_Q= 0;
                    sum_P = std::accumulate(Pc.begin(), Pc.end(), T{0});
                    sum_Q = std::accumulate(Qc.begin(), Qc.end(), T{0});
                    // {for (std::size_t i=0; i<N; ++i) sum_P+= Pc[i];}
                    // {for (std::size_t i=0; i<N; ++i) sum_Q+= Qc[i];}
                    bool needToSwapFlow= false;
                    if (sum_Q>sum_P) {
                        needToSwapFlow= true;
                        P.assign(std::begin(Qc), std::end(Qc));
                        Q.assign(std::begin(Pc), std::end(Pc));
                        // transpose C
                        for (std::size_t i=0; i<N; ++i) {
                            for (std::size_t j=0; j<N; ++j) {
                                C[i][j]= Cc[j][i];
                            }
                        }
                        abs_diff_sum_P_sum_Q= sum_Q-sum_P;
                    } else {
                        P.assign(std::begin(Pc), std::end(Pc));
                        Q.assign(std::begin(Qc), std::end(Qc));
                        abs_diff_sum_P_sum_Q= sum_P-sum_Q;
                    }
                    //if (needToSwapFlow) cout << "needToSwapFlow" << endl;
                    // end of re-insertion

                    // creating the b vector that contains all vertexes
                    std::vector<T> b(2 * N + 2);
                    const size_t THRESHOLD_NODE = 2 * N;
                    const size_t ARTIFICIAL_NODE = 2 * N + 1; // need to be last !

                    for (size_t i = 0; i < N; ++i)
                    {
                        b[i] = P[i];
                    }

                    for (size_t i = N; i < 2 * N; ++i)
                    {
                        b[i] = (Q[i - N]);
                    }
                    // remark*) I put here a deficit of the extra mass, as mass that flows to the threshold node
                    // can be absorbed from all sources with cost zero (this is in reverse order from the paper,
                    // where incoming edges to the threshold node had the cost of the threshold and outgoing
                    // edges had the cost of zero)
                    // This also makes sum of b zero.
                    b[THRESHOLD_NODE] = -abs_diff_sum_P_sum_Q;
                    b[ARTIFICIAL_NODE] = 0;
                    //-------------------------------------------------------
                    // original code restored by Max F
                    T maxC = 0;
                    {for (std::size_t i=0; i<N; ++i) {
                            {for (std::size_t j=0; j<N; ++j) {
                                    assert(C[i][j]>=0);
                                    if ( C[i][j]>maxC ) maxC= C[i][j];
                                }}
                        }}
                    // end of restored code

                    if (extra_mass_penalty == -1)
                        extra_mass_penalty = maxC;
                    //-------------------------------------------------------

                    //=============================================================
                    std::set<size_t> sources_that_flow_not_only_to_thresh;
                    std::set<size_t> sinks_that_get_flow_not_only_from_thresh;
                    T pre_flow_cost = 0;
                    //=============================================================

                    //=============================================================
                    // regular edges between sinks and sources without threshold edges
                    std::vector<std::list<edge<T>>> c(b.size());
                    {
                        for (size_t i = 0; i < N; ++i)
                        {
                            if (b[i] == 0)
                                continue;
                            {
                                for (size_t j = 0; j < N; ++j)
                                {
                                    if (b[j + N] == 0)
                                        continue;
                                    if (C[i][j] == maxC)
                                        continue;
                                    c[i].push_back(edge<T>(j + N, C[i][j]));
                                }
                            } // j
                        }
                    } // i

                    // checking which are not isolated
                    {
                        for (size_t i = 0; i < N; ++i)
                        {
                            if (b[i] == 0)
                                continue;
                            {
                                for (size_t j = 0; j < N; ++j)
                                {
                                    if (b[j + N] == 0)
                                        continue;
                                    if (C[i][j] == maxC)
                                        continue;
                                    sources_that_flow_not_only_to_thresh.insert(i);
                                    sinks_that_get_flow_not_only_from_thresh.insert(j + N);
                                }
                            } // j
                        }
                    } // i

                    // converting all sinks to negative
                    {
                        for (size_t i = N; i < 2 * N; ++i)
                        {
                            b[i] = -b[i];
                        }
                    }
                    // add edges from/to threshold node,
                    // note that costs are reversed to the paper (see also remark* above)
                    // It is important that it will be this way because of remark* above.
                    {
                        for (size_t i = 0; i < N; ++i)
                        {
                            c[i].push_back(edge<T>(THRESHOLD_NODE, 0));
                        }
                    }
                    {
                        for (size_t j = 0; j < N; ++j)
                        {
                            c[THRESHOLD_NODE].push_back(edge<T>(j + N, maxC));
                        }
                    }

                    // artificial arcs - Note the restriction that only one edge i,j is artificial so I ignore it...
                    {
                        for (size_t i = 0; i < ARTIFICIAL_NODE; ++i)
                        {
                            c[i].push_back(edge<T>(ARTIFICIAL_NODE, maxC + 1));
                            c[ARTIFICIAL_NODE].push_back(edge<T>(i, maxC + 1));
                        }
                    }
                    //=============================================================

                    //====================================================
                    // remove nodes with supply demand of 0
                    // and vertexes that are connected only to the
                    // threshold vertex
                    //====================================================
                    size_t current_node_name = 0;
                    // Note here it should be vector<int> and not vector<size_t>
                    // as I'm using -1 as a special flag !!!
                    const int REMOVE_NODE_FLAG = -1;
                    std::vector<int> nodes_new_names(b.size(), REMOVE_NODE_FLAG);
                    std::vector<int> nodes_old_names;
                    nodes_old_names.reserve(b.size());
                    {
                        for (size_t i = 0; i < N * 2; ++i)
                        {
                            if (b[i] != 0)
                            {
                                if (sources_that_flow_not_only_to_thresh.find(i) != sources_that_flow_not_only_to_thresh.end() ||
                                    sinks_that_get_flow_not_only_from_thresh.find(i) != sinks_that_get_flow_not_only_from_thresh.end())
                                {
                                    nodes_new_names[i] = current_node_name;
                                    nodes_old_names.push_back(i);
                                    ++current_node_name;
                                }
                                else
                                {
                                    if (i >= N)
                                    { // sink
                                        pre_flow_cost -= (b[i] * maxC);
                                    }
                                    b[THRESHOLD_NODE] += b[i]; // add mass(i<N) or deficit (i>=N)
                                }
                            }
                        }
                    } //i
                    nodes_new_names[THRESHOLD_NODE] = current_node_name;
                    nodes_old_names.push_back(THRESHOLD_NODE);
                    ++current_node_name;
                    nodes_new_names[ARTIFICIAL_NODE] = current_node_name;
                    nodes_old_names.push_back(ARTIFICIAL_NODE);
                    ++current_node_name;

                    std::vector<T> bb(current_node_name);
                    size_t j = 0;
                    {
                        for (size_t i = 0; i < b.size(); ++i)
                        {
                            if (nodes_new_names[i] != REMOVE_NODE_FLAG)
                            {
                                bb[j] = b[i];
                                ++j;
                            }
                        }
                    }

                    std::vector<std::list<edge<T>>> cc(bb.size());
                    {
                        for (size_t i = 0; i < c.size(); ++i)
                        {
                            if (nodes_new_names[i] == REMOVE_NODE_FLAG)
                                continue;
                            {
                                for (typename std::list<edge<T>>::const_iterator it = c[i].begin(); it != c[i].end(); ++it)
                                {
                                    if (nodes_new_names[it->_to] != REMOVE_NODE_FLAG)
                                    {
                                        cc[nodes_new_names[i]].push_back(edge<T>(nodes_new_names[it->_to], it->_cost));
                                    }
                                }
                            }
                        }
                    }

                    min_cost_flow<T> mcf;

                    T my_dist;

                    std::vector<std::list<edge0<T>>> flows(bb.size());

                    T mcf_dist = mcf(bb, cc, flows);

                    if (FLOW_TYPE != NO_FLOW)
                    {
                        for (size_t new_name_from = 0; new_name_from < flows.size(); ++new_name_from)
                        {
                            for (typename std::list<edge0<T>>::const_iterator it = flows[new_name_from].begin(); it != flows[new_name_from].end(); ++it)
                            {
                                if (new_name_from == nodes_new_names[THRESHOLD_NODE] || it->_to == nodes_new_names[THRESHOLD_NODE])
                                    continue;
                                size_t i, j;
                                T flow = it->_flow;
                                bool reverseEdge = it->_to < new_name_from;
                                if (!reverseEdge)
                                {
                                    i = nodes_old_names[new_name_from];
                                    j = nodes_old_names[it->_to] - N;
                                }
                                else
                                {
                                    i = nodes_old_names[it->_to];
                                    j = nodes_old_names[new_name_from] - N;
                                }
                                if (flow != 0 && new_name_from != nodes_new_names[THRESHOLD_NODE] && it->_to != nodes_new_names[THRESHOLD_NODE])
                                {
                                    assert(i < N && j < N);
                                    if (needToSwapFlow)
                                        std::swap(i, j);
                                    if (!reverseEdge)
                                    {
                                        (*F)[i][j] += flow;
                                    }
                                    else
                                    {
                                        (*F)[i][j] -= flow;
                                    }
                                }
                            }
                        }
                    }

                    if (FLOW_TYPE == WITHOUT_EXTRA_MASS_FLOW)
                        transform_flow_to_regular(*F, POrig, QOrig);

                    my_dist =
                        pre_flow_cost +                              // pre-flowing on cases where it was possible
                        mcf_dist +                                   // solution of the transportation problem
                        (abs_diff_sum_P_sum_Q * extra_mass_penalty); // emd-hat extra mass penalty

                    return my_dist;
                    //-------------------------------------------------------

                } // emd_impl_integral_types (main implementation) operator()
            };
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/*** check types and convert float to integral type before continueing with emd_impl_integral_types()***/
            template <typename Container, FLOW_TYPE_T FLOW_TYPE>
            struct emd_impl
            {
                typedef typename Container::value_type T;
                typedef long long int CONVERT_TO_T;
                //typedef int T;

                T operator()(
                    const Container &POrig,
                    const Container &QOrig,
                    const Container &P,
                    const Container &Q,
                    const std::vector<std::vector<T>> &C,
                    // T maxC, // disabled by Max F
                    T extra_mass_penalty,
                    std::vector<std::vector<T>> *F//,
                    // T abs_diff_sum_P_sum_Q // disabled by Max F
                    )
                {
                    /*** integral types ***/
                    if (std::is_integral<T>::value)
                    {
                        //return emd_impl_integral_types<Container, FLOW_TYPE>()(POrig, QOrig, P, Q, C, maxC, extra_mass_penalty, F, abs_diff_sum_P_sum_Q);
                        return emd_impl_integral_types<Container, FLOW_TYPE>()(POrig, QOrig, P, Q, C, extra_mass_penalty, F); // replaced by Max F
                    }
                    /*** floating types ***/
                    else if (std::is_floating_point<T>::value)
                    {

                        // TODO: static assert
                        assert(sizeof(CONVERT_TO_T) >= 8);

                        // This condition should hold:
                        // ( 2^(sizeof(CONVERT_TO_T*8)) >= ( MULT_FACTOR^2 )
                        // Note that it can be problematic to check it because
                        // of overflow problems. I simply checked it with Linux calc
                        // which has arbitrary precision.
                        const double MULT_FACTOR = 1000000;

                        // Constructing the input
                        const size_t N = P.size();
                        std::vector<CONVERT_TO_T> iPOrig(N);
                        std::vector<CONVERT_TO_T> iQOrig(N);
                        std::vector<CONVERT_TO_T> iP(N);
                        std::vector<CONVERT_TO_T> iQ(N);
                        std::vector<std::vector<CONVERT_TO_T>> iC(N, std::vector<CONVERT_TO_T>(N));
                        std::vector<std::vector<CONVERT_TO_T>> iF(N, std::vector<CONVERT_TO_T>(N));

                        // Converting to CONVERT_TO_T
                        double sumP = 0.0;
                        double sumQ = 0.0;
                        double imaxC = C[0][0];
                        for (size_t i = 0; i < N; ++i)
                        {
                            sumP += POrig[i];
                            sumQ += QOrig[i];
                            for (size_t j = 0; j < N; ++j)
                            {
                                if (C[i][j] > imaxC)
                                    imaxC = C[i][j];
                            }
                        }
                        double minSum = std::min(sumP, sumQ);
                        double maxSum = std::max(sumP, sumQ);
                        double PQnormFactor = MULT_FACTOR / maxSum;
                        double CnormFactor = MULT_FACTOR / imaxC;
                        for (size_t i = 0; i < N; ++i)
                        {
                            iPOrig[i] = static_cast<CONVERT_TO_T>(floor(POrig[i] * PQnormFactor + 0.5));
                            iQOrig[i] = static_cast<CONVERT_TO_T>(floor(QOrig[i] * PQnormFactor + 0.5));
                            iP[i] = static_cast<CONVERT_TO_T>(floor(P[i] * PQnormFactor + 0.5));
                            iQ[i] = static_cast<CONVERT_TO_T>(floor(Q[i] * PQnormFactor + 0.5));
                            for (size_t j = 0; j < N; ++j)
                            {
                                iC[i][j] = static_cast<CONVERT_TO_T>(floor(C[i][j] * CnormFactor + 0.5));
                                if (FLOW_TYPE != NO_FLOW)
                                {
                                    iF[i][j] = static_cast<CONVERT_TO_T>(floor(((*F)[i][j]) * PQnormFactor + 0.5));
                                }
                            }
                        }

                        // computing distance without extra mass penalty
//            double dist = emd_impl<std::vector<CONVERT_TO_T>, FLOW_TYPE>()(iPOrig, iQOrig, iP, iQ, iC, imaxC, 0, &iF, abs_diff_sum_P_sum_Q);
                        double dist = emd_impl<std::vector<CONVERT_TO_T>, FLOW_TYPE>()(iPOrig, iQOrig, iP, iQ, iC, 0, &iF); // replaced by Max F
                        // unnormalize
                        dist = dist / PQnormFactor;
                        dist = dist / CnormFactor;

                        // adding extra mass penalty
                        if (extra_mass_penalty == -1)
                            extra_mass_penalty = imaxC;
                        dist += (maxSum - minSum) * extra_mass_penalty;

                        // converting flow to double
                        if (FLOW_TYPE != NO_FLOW)
                        {
                            for (size_t i = 0; i < N; ++i)
                            {
                                for (size_t j = 0; j < N; ++j)
                                {
                                    (*F)[i][j] = (iF[i][j] / PQnormFactor);
                                }
                            }
                        }

                        return dist;
                    }
                }

            }; // emd_impl

        } // namespace EMD_details

/*
  ____|   \  |  __ \  
  __|    |\/ |  |   | 
  |      |   |  |   | 
  _____| _|  _| ____/  */

/*
  input:
  Pc:  vector like container 
  Qc:  vector like container 
  C: ground distance matrix
  for images: serialize the T typed matricies in a vector and compute the ground distance matrix of the serialized grid with auto C = metric::ground_distance_matrix_of_2dgrid<T>(cols, rows);

*/


        // template <typename V>
        // auto
        // EMD<V>::operator()(const Container &Pc,
        //                            const Container &Qc,
        //                            //const std::vector<std::vector<typename Container::value_type>> &C,
        //                            //typename Container::value_type maxC, // disabled my Max F
        //                            typename Container::value_type extra_mass_penalty,
        //                            std::vector<std::vector<typename Container::value_type>> *F) const
        // {
        //     //std::vector<std::vector<typename Container::value_type>> C = EMD_details::ground_distance_matrix_of_2dgrid<typename Container::value_type>(Pc.size(), Qc.size()); // TODO replace with proper call
        //     // add default matrix
        //     std::vector<std::vector<typename Container::value_type>> C(Pc.size(), std::vector<typename Container::value_type>(Qc.size(), 0));
        //     int t = std::min(Pc.size(), Qc.size()) / 2; // by default, ground distance saturates at the half of maximum distance possible
        //     for (size_t i=0; i<Pc.size(); i++)
        //         for (size_t j=0; j<Qc.size(); j++)
        //             C[i][j] = std::min(t, std::abs((int)(i - j))); // non-square matrix is supported here, BUT IS NOT SUPPORTED IN THE EMD IMPL
        //     return (*this)(Pc, Qc, C, extra_mass_penalty, F);
        // }
        template<typename V>
        inline auto
        EMD<V>::default_ground_matrix(std::size_t rows, std::size_t cols) const  -> std::vector<std::vector<value_type>> {
            std::vector<std::vector<value_type>> matrix(rows, std::vector<value_type>(cols, 0));
            if(rows == 1 && cols == 1) {
                matrix[0][0] = 1;
                return matrix;
            }
            int t = std::min(rows, cols) / 2; // by default, ground distance saturates at the half of maximum distance possible

            for (size_t i = 0; i < rows; i++) {
                for (size_t j = 0; j < cols; j++) {
                    matrix[i][j] = std::min(t, std::abs((int)(i - j))); // non-square matrix is supported here, BUT IS NOT SUPPORTED IN THE EMD IMPL
                }
            }
            return matrix;
        }
        template <typename V>
        template <typename Container>
        auto
        EMD<V>::operator()(const Container &Pc,
                                   const Container &Qc) const -> distance_type
                                   // const std::vector<std::vector<typename Container::value_type>> &C,
                                   // //typename Container::value_type maxC, // disabled my Max F
                                   // typename Container::value_type extra_mass_penalty,
                                   // std::vector<std::vector<typename Container::value_type>> *F) const
        {
            if(!is_C_initialized) {
                C = default_ground_matrix(Pc.size(), Pc.size());
                is_C_initialized = true;
            }
            //typedef typename Container::value_type T;
            using T = value_type;
            const EMD_details::FLOW_TYPE_T FLOW_TYPE = EMD_details::NO_FLOW;
//    // if maxC is not given seperatly // disabled by Max F when rolled back to original version
//    if (maxC == std::numeric_limits<T>::min())
//    {
//        maxC = EMD_details::max_in_distance_matrix(C);
//    }
            if (FLOW_TYPE != EMD_details::NO_FLOW)
            {
                EMD_details::fillFWithZeros(*F);
            }

            assert((F != NULL) || (FLOW_TYPE == EMD_details::NO_FLOW));

            std::vector<T> P(std::begin(Pc), std::end(Pc));
            std::vector<T> Q(std::begin(Qc), std::end(Qc));

            // Assuming metric property we can pre-flow 0-cost edges
            {
                for (size_t i = 0; i < P.size(); ++i)
                {
                    if (P[i] < Q[i])
                    {
                        if (FLOW_TYPE != EMD_details::NO_FLOW)
                        {
                            ((*F)[i][i]) = P[i];
                        }
                        Q[i] -= P[i];
                        P[i] = 0;
                    }
                    else
                    {
                        if (FLOW_TYPE != EMD_details::NO_FLOW)
                        {
                            ((*F)[i][i]) = Q[i];
                        }
                        P[i] -= Q[i];
                        Q[i] = 0;
                    }
                }
            }

//    // need to swap? // disabled by Max F
//    T sum_P = 0;
//    T sum_Q = 0;
//    {
//        for (size_t i = 0; i < Pc.size(); ++i)
//            sum_P += Pc[i];
//    }
//    {
//        for (size_t i = 0; i < Qc.size(); ++i)
//            sum_Q += Qc[i];
//    }

//    // need to swap?
//    if (sum_Q > sum_P)
//    {
//        std::swap(P, Q);
//    }

//    T abs_diff_sum_P_sum_Q = std::abs(sum_P - sum_Q);
//    T abs_diff_sum_P_sum_Q = 0; // temporary added by MAx F // TODO remove

//    return EMD_details::emd_impl<std::vector<T>, FLOW_TYPE>()(Pc, Qc, P, Q, C, maxC, extra_mass_penalty, F, abs_diff_sum_P_sum_Q);
            return EMD_details::emd_impl<Container, FLOW_TYPE>()(Pc, Qc, P, Q, C, extra_mass_penalty, F); // turned to original state by Max F

        }; // EMD

    } // namespace distance
} // namespace metric

#endif
