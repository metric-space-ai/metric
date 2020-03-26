/*
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/
#ifndef _METRIC_UTILS_GRAPH_PARTITION_CPP
#define _METRIC_UTILS_GRAPH_PARTITION_CPP

namespace metric {

    bool distance_matrix_is_valid(blaze::DynamicMatrix<double> matrix)
    {
        if (matrix.rows() != matrix.columns()) return false;

        for (int i = 0; i < matrix.rows(); i++)
        {
            for (int j = i; j < matrix.columns(); j++)
            {
                if (i == j && matrix(i, j) != 0)
                {
                    return false;
                }
                else if (matrix(i, j) != matrix(j, i))
                {
                    return false;
                }
            }
        }

        return true;
    };

    int random_int_in_range(int left, int right) { return left + (std::rand() % (right - left + 1)); }

    template <typename Tv>
    blaze::DynamicMatrix<Tv> clone_matrix(blaze::DynamicMatrix<Tv> source)
    {
        blaze::DynamicMatrix<Tv> new_matrix(source);
        return new_matrix;
    }

    template <typename Tv>
    blaze::DynamicMatrix<Tv> extract_random_matrix_rows(blaze::DynamicMatrix<Tv> source, int rows_count)
    {
        int length = source.rows();

        std::random_device rd;
        std::mt19937 g(rd());

        // ra is list random sample of processing_chunk_size columns
        blaze::DynamicVector<int> random_rows_index(length);
        for (int i = 0; i < length; i++) {
            random_rows_index[i] = i;
        }
        
        //std::random_shuffle(random_rows_index.begin(), random_rows_index.end());
        std::shuffle(random_rows_index.begin(), random_rows_index.end(), g);

        random_rows_index.resize(rows_count, true);

        blaze::DynamicMatrix<double> result(rows_count, length);
        for (int i = 0; i < random_rows_index.size(); i++) {
            for (int j = 0; j < length; j++) {
                result(i, j) = source(random_rows_index[i], j);
            }
        }

        return result;
    }

    blaze::DynamicMatrix<double> get_identity_matrix(int size)
    {
        blaze::DynamicMatrix<double> result(size, size, 0);

        for (int i = 0; i < size; i++) {
            result(i, i) = 1;
        }

        return result;
    }

    template <typename Tv>
    blaze::DynamicMatrix<Tv> remove_matrix_rows(
        blaze::DynamicMatrix<Tv> source, blaze::DynamicVector<int> remove_rows_indices)
    {
        blaze::DynamicMatrix<Tv> result;
        result.resize(source.rows(), source.columns());

        int row_count = 0;
        for (int i = 0; i < source.rows(); i++) {
            bool skip_row = false;
            for (int j = 0; j < remove_rows_indices.size(); j++) {
                if (i == remove_rows_indices[j]) {
                    skip_row = true;
                    break;
                }
            }

            if (skip_row)
                continue;

            for (int j = 0; j < source.columns(); j++) {
                result(row_count, j) = source(i, j);
            }
            row_count++;
        }

        return result;
    }

    void get_min_values_indices_on_columns(blaze::DynamicMatrix<double> input, blaze::DynamicVector<int>& output)
    {
        output.reset();
        output.resize(input.rows());
        for (int i = 0; i < input.rows(); i++) {
            double min = 0;
            for (int j = 0; j < input.columns(); j++) {
                if (j == 0) {
                    output[i] = 0;
                    min = input(i, j);
                }
                else {
                    if (input(i, j) < min) {
                        min = input(i, j);
                        output[i] = j;
                    }
                }
            }
        }
    }

    int perform_graph_partition(blaze::DynamicMatrix<double> distance_matrix, blaze::DynamicMatrix<int>& partition_matrix,
        int global_optimum_attempts, int processing_chunk_size, __int64 random_seed)
    {
        if (random_seed < 0) {
            random_seed = time(NULL);
        }
        srand(random_seed);

        int length = distance_matrix.rows();

        if (processing_chunk_size > length)
        {
            return 1;
        }

        if (!distance_matrix_is_valid(distance_matrix))
        {
            return 3;
        }

        blaze::DynamicMatrix<double> dd = extract_random_matrix_rows(distance_matrix, processing_chunk_size);
        blaze::DynamicMatrix<double> a = clone_matrix(dd).transpose();

        int n1 = a.rows();
        int n2 = a.columns();

        //prepare unit matrix
        blaze::DynamicMatrix<double> c = get_identity_matrix(n2);
        blaze::DynamicMatrix<double> b = clone_matrix(a).transpose();

        for (int kk1 = 10; kk1 <= 10; kk1++) {
            //kk1 is number of row clusters
            int k1 = kk1;
            double div1 = blaze::inf;
            int ko = 0;

            blaze::DynamicMatrix<double> rmin;

            //global_optimum_attempts is number of attempts to find global optima, SOMETHING LIKE 100 - 1000
            for (int toi = 0; toi <= global_optimum_attempts; toi++) {
                /*
                r = row cluster matrix, n1 times k1 matrix, initially all zero
                elements, when clustering is defined 1 indicates the cluster number
                */
                blaze::DynamicMatrix<double> r(n1, kk1, 0);
                blaze::DynamicVector<double> rf(kk1, 1);
                blaze::DynamicMatrix<double> r1(r);
                blaze::DynamicVector<double> rf1;

                bool keep_going = true;

                while (keep_going) {
                    /*
                    choose a random row clusterin to start, rf is a list of clusters \
                    memership, value 0 means at least one member, 1 means no members,
                    max cf = 0 means all clusters have members, run untill all rf \
                    elements are zeroes
                    */
                    rf1 = rf;
                    for (int i = 0; i < n1; i++)
                    {
                        int l = random_int_in_range(0, kk1 - 1);
                        r1(i, l) = 1;
                        rf1[l] = 0;
                        if (blaze::max(rf1) == 0)
                        {
                            keep_going = false;
                            r = r1;
                        }
                    }
                }

                blaze::DynamicMatrix<double> p = clone_matrix(r).transpose() * (a * c);
                blaze::DynamicMatrix<double> u = clone_matrix(r).transpose();

                k1 = u.rows();

                blaze::DynamicVector<double> rsize(k1);
                blaze::DynamicVector<double, blaze::columnVector> row_sum;

                row_sum = sum<blaze::rowwise>(u);

                for (int i = 0; i < k1; i++) {
                    rsize[i] = row_sum[i];
                }

                for (int i = 0; i < k1; i++) {
                    for (int j = 0; j < n2; j++) {
                        p(i, j) = p(i, j) / rsize[i];
                    }
                }

                r1.reset();
                for (int i = 0; i < n1; i++) {
                    int l = random_int_in_range(0, kk1 - 1);
                    r1(i, l) = 1;
                    rf1[l] = 0;
                    r = r1;
                }

                p = clone_matrix(r).transpose() * (a * c);
                u = clone_matrix(r).transpose();
                rsize.reset();
                row_sum = sum<blaze::rowwise>(u);

                for (int i = 0; i < k1; i++) {
                    rsize[i] = row_sum[i];
                }

                for (int i = 0; i < k1; i++) {
                    for (int j = 0; j < n2; j++) {
                        p(i, j) = p(i, j) / rsize[i];
                    }
                }

                blaze::DynamicMatrix<double> ln = blaze::log(p + 0.0001);
                blaze::DynamicVector<double> pi = sum<blaze::rowwise>(p);

                c.reset();

                for (int i = 0; i < n2; i++) {
                    c(i, i) = 1;
                }

                double tmax = 10;
                double t = 1;

                blaze::DynamicVector<double> w(n2, 1);
                blaze::DynamicVector<int> ff;
                blaze::DynamicMatrix<double> rw;

                while (t <= tmax) {
                    blaze::DynamicMatrix<double> dot_product = ln * b;
                    rw.reset();
                    rw.resize(dot_product.rows(), dot_product.columns());

                    for (int i = 0; i < dot_product.rows(); i++) {
                        for (int j = 0; j < dot_product.columns(); j++) {
                            rw(i, j) = -1 * (dot_product(i, j) - pi[i]);
                        }
                    }

                    rw = rw.transpose();

                    get_min_values_indices_on_columns(rw, ff);

                    blaze::DynamicVector<double> aux_table(n1);
                    for (int i = 0; i < n1; i++) {
                        aux_table[i] = rw(i, ff[i]);
                    }

                    double div = blaze::mean(aux_table);

                    blaze::DynamicMatrix<int> ru(r.rows(), r.columns(), 0);
                    for (int i = 0; i < n1; i++) {
                        ru(i, ff[i]) = 1;
                    }

                    blaze::DynamicVector<int> clustrow;
                    for (int i = 0; i < ff.size(); i++) {
                        if (std::find(clustrow.begin(), clustrow.end(), ff[i]) == clustrow.end()) {
                            clustrow.extend(1);
                            clustrow[clustrow.size() - 1] = ff[i];
                        }
                    }

                    blaze::DynamicMatrix<int> ruu = clone_matrix(ru).transpose();
                    blaze::DynamicMatrix<int> rne(clustrow.size(), ruu.columns());

                    for (int i = 0; i < clustrow.size(); i++) {
                        for (int j = 0; j < ruu.columns(); j++) {
                            rne(i, j) = ruu(clustrow[i], j);
                        }
                    }

                    blaze::DynamicMatrix<int> rnew = clone_matrix(rne).transpose();

                    r = rnew;
                    u = clone_matrix(rnew).transpose();

                    blaze::DynamicVector<int> sdd;

                    row_sum = sum<blaze::rowwise>(u);

                    for (int i = 0; i < u.rows(); i++) {
                        if (row_sum[i] == 0) {
                            sdd.resize(sdd.size() + 1, true);
                            sdd[sdd.size() - 1] = i;
                        }
                    }

                    if (sdd.size() > 0) {
                        u = remove_matrix_rows(u, sdd);
                    }

                    r = clone_matrix(u).transpose();
                    k1 = u.rows();

                    blaze::DynamicMatrix<double> acProduct = a * c;
                    blaze::DynamicMatrix<double> pu = clone_matrix(rnew).transpose() * acProduct;

                    rsize.clear();
                    rsize.resize(k1);

                    for (int i = 0; i < k1; i++) {
                        rsize[i] = row_sum[i];
                    }

                    for (int i = 0; i < k1; i++) {
                        for (int j = 0; j < n2; j++) {
                            pu(i, j) = pu(i, j) / rsize[i];
                        }
                    }

                    blaze::DynamicMatrix<double> lnu = blaze::log(pu + 0.0001);
                    p = pu;

                    row_sum = sum<blaze::rowwise>(p);
                    pi.reset();
                    pi.resize(p.rows());

                    for (int i = 0; i < row_sum.size(); i++) {
                        pi[i] = row_sum[i];
                    }

                    ln = lnu;
                    t++;
                }

                blaze::DynamicVector<double> mean_aux(n1);
                for (int i = 0; i < n1; i++) {
                    mean_aux[i] = rw(i, ff[i]);
                }

                double div = blaze::mean(mean_aux);

                if (div < div1) {
                    div1 = div;
                    rmin = r;
                    ko = k1;
                }
            }

            partition_matrix = rmin;
        }

        return 0;
    }

}
#endif
