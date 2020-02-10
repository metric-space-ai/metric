/*
		This Source Code Form is subject to the terms of the Mozilla Public
		License, v. 2.0. If a copy of the MPL was not distributed with this
		file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/
#ifndef _METRIC_UTILS_GRAPH_PARTITION_CPP
#define _METRIC_UTILS_GRAPH_PARTITION_CPP

namespace metric {

int random_int_in_range(int left, int right) { return left + (std::rand() % (right - left + 1)); }

template <typename Tv>
blaze::DynamicMatrix<Tv> clone_matrix(blaze::DynamicMatrix<Tv> source)
{
    blaze::DynamicMatrix<Tv> new_matrix(source);
    return new_matrix;
}

template <typename Tv>
blaze::DynamicMatrix<Tv> matrix_dot_product(blaze::DynamicMatrix<Tv> left, blaze::DynamicMatrix<Tv> right)
{
    blaze::DynamicMatrix<Tv> result(left);
    result *= right;
    return result;
}

template <typename Tv>
void print_array(blaze::DynamicVector<Tv> source)
{
    std::cout << "array[" << source.size() << "] ";
    for (int i = 0; i < source.size(); i++)
    {
        std::cout << source[i] << ", ";
    }
    std::cout << "\n";
}

template <typename Tv>
void print_matrix(blaze::DynamicMatrix<Tv> source)
{
    std::cout << "matrix[" << source.rows() << ", " << source.columns() << "] ";
    for (int i = 0; i < source.rows(); i++)
    {
        std::cout << "\n(";
        for (int j = 0; j < source.columns(); j++)
        {
            std::cout << source(i, j) << ", ";
        }
        std::cout << "\n),";
    }
    std::cout << "\n";
}

bool perform_graph_partition(const blaze::DynamicMatrix<double> distance_matrix, blaze::DynamicMatrix<int>& partition, int global_optimum_attempts)
{
    srand(time(NULL));

    int length = distance_matrix.rows();

    blaze::DynamicVector<int> random_columns_index(length);
    for (int i = 0; i < length; i++)
    {
        random_columns_index[i] = i;
    }
    std::random_shuffle(random_columns_index.begin(), random_columns_index.end());
    random_columns_index.resize(100, true);

    blaze::DynamicMatrix<double> dd(random_columns_index.size(), length);
    for (int i = 0; i < random_columns_index.size(); i++)
    {
        for (int j = 0; j < length; j++)
        {
            dd(i, j) = distance_matrix(random_columns_index[i], j);
        }
    }
    
    blaze::DynamicMatrix<double> a = clone_matrix(dd).transpose();

    int n1 = a.rows();
    int n2 = a.columns();
    blaze::DynamicMatrix<double> c(n2, n2, 0);
    
    for (int i = 0; i < n2; i++)
    {
        c(i, i) = 1;
    }

    blaze::DynamicMatrix<double> b = clone_matrix(a).transpose();
    blaze::DynamicVector<blaze::DynamicMatrix<double>> re;
  
    for (int kk1 = 10; kk1 <= 10; kk1++)
    {
        int k1 = kk1;
        double div1 = 1000000;
        int ko;
        blaze::DynamicMatrix<double> rmin;

        for (int toi = 1; toi <= 5; toi++)
        {
  
            blaze::DynamicMatrix<double> r(n1, kk1, 0);
            blaze::DynamicVector<double> rf(kk1, 1);
            blaze::DynamicMatrix<double> r1(r);
            blaze::DynamicVector<double> rf1;

            int tt = 0;
            while (tt == 0)
            {        
                rf1 = rf;
                for (int i = 0; i < n1; i++)
                {
                    int l = random_int_in_range(0, kk1-1);
                    r1(i, l) = 1;
                    rf1[l] = 0;
                    if (blaze::max(rf1) == 0)
                    {
                        tt = 1;
                        r = r1;
                    }
                }
            }

            blaze::DynamicMatrix<double> p = matrix_dot_product(clone_matrix(r).transpose(), matrix_dot_product(a, c));
            blaze::DynamicMatrix<double> u = clone_matrix(r).transpose();

            k1 = u.rows();

            blaze::DynamicVector<double> rsize(k1);
            blaze::DynamicVector<double, blaze::columnVector> row_sum;

            row_sum = sum<blaze::rowwise>(u);

            for (int i = 0; i < k1; i++)
            {
                rsize[i] = row_sum[i];
            }

            for (int i = 0; i < k1; i++)
            {
                for (int j = 0; j < n2; j++)
                {
                    p(i, j) = p(i, j) / rsize[i];
                }
            }

            r1.reset();
            for (int i = 0; i < n1; i++)
            {
                int l = random_int_in_range(0, kk1-1);
                r1(i, l) = 1;
                rf1[l] = 0; 
                r = r1;
            }
             

            p = matrix_dot_product(clone_matrix(r).transpose(), matrix_dot_product(a, c));
            u = clone_matrix(r).transpose();
            rsize.reset();
            row_sum = sum<blaze::rowwise>(u);

            for (int i = 0; i < k1; i++)
            {
                rsize[i] = row_sum[i];
            }

            for (int i = 0; i < k1; i++)
            {
                for (int j = 0; j < n2; j++)
                {
                    p(i, j) = p(i, j) / rsize[i];
                }
            }

            blaze::DynamicMatrix<double> ln = blaze::log(p + 0.0001);
            blaze::DynamicVector<double> pi = sum<blaze::rowwise>(p);
       
            c.reset();

            for (int i = 0; i < n2; i++)
            {
                c(i,i) = 1;
            }

            double tmax = 10;
            double t = 1;

            blaze::DynamicVector<double> w(n2, 1);
            blaze::DynamicVector<int> ff;
            blaze::DynamicMatrix<double> rw;

            while(t <= tmax && t < 2)
            { 
                blaze::DynamicMatrix<double> dot_product = matrix_dot_product(ln, b);
                rw.reset();
                rw.resize(dot_product.rows(), dot_product.columns());
                
                for (int i = 0; i < dot_product.rows(); i++)
                {
                    for (int j = 0; j < dot_product.columns(); j++)
                    {
                        rw(i, j) = -1 * (dot_product(i, j) - pi[i]);
                    }
                }

                rw = rw.transpose();

                ff.reset();
                ff.resize(rw.rows());
                for (int i = 0; i < rw.rows(); i++)
                {
                    double min = 0;
                    for(int j=0; j<rw.columns(); j++)
                    {          
                        if(j == 0)
                        {
                            ff[i] = 0;
                            min = rw(i, j);
                        }
                        else
                        {
                            if (rw(i, j) < min)
                            {
                                min = rw(i, j);
                                ff[i] = j;
                            }
                        }
                    }
                }
   
                blaze::DynamicVector<double> aux_table(n1);
                for (int i = 0; i < n1; i++)
                {
                    aux_table[i] = rw(i, ff[i]); 
                }
                
                double div = blaze::mean(aux_table);

                blaze::DynamicMatrix<int> ru(r.rows(), r.columns(), 0);
                for (int i = 0; i < n1; i++)
                {
                    ru(i, ff[i]) = 1;
                }

                blaze::DynamicVector<int> clustrow;
                for (int i = 0; i < ff.size(); i++)
                {
                    if (std::find(clustrow.begin(), clustrow.end(), ff[i]) == clustrow.end())
                    {
                        clustrow.extend(1);
                        clustrow[clustrow.size() - 1] = ff[i];
                    }   
                }

                blaze::DynamicMatrix<int> ruu = clone_matrix(ru).transpose();
                blaze::DynamicMatrix<int> rne(clustrow.size(), ruu.columns());

                for (int i = 0; i < clustrow.size(); i++)
                {
                    for (int j = 0; j < ruu.columns(); j++)
                    {
                        rne(i, j) = ruu(clustrow[i], j);
                    }
                }

                blaze::DynamicMatrix<int> rnew = clone_matrix(rne).transpose();
                
                r = rnew;
                u = clone_matrix(rnew).transpose();

                blaze::DynamicVector<int> sdd;

                row_sum = sum<blaze::rowwise>(u);
                
                for (int i = 0; i < u.rows(); i++)
                {
                    if (row_sum[i] == 0)
                    {
                        sdd.resize(sdd.size() + 1, true);
                        sdd[sdd.size() - 1] = i;
                    }
                }


                if(sdd.size() > 0)
                {
                    blaze::DynamicMatrix<int> backup_u(u);
                    
                    u.reset();
                    u.resize(backup_u.rows(), backup_u.columns());
                    int row_count = 0;
                    for (int i = 0; i < backup_u.rows(); i++)
                    {
                        bool skip_row = false;
                        for (int j = 0; j < sdd.size(); j++)
                        {
                            if (i == sdd[j])
                            {
                                skip_row = true;
                                break;
                            }
                        }

                        if (skip_row) continue;

                        for (int j = 0; j < backup_u.columns(); j++)
                        {
                            u(row_count, j) = backup_u(i, j);
                        }
                        row_count++;
                    }
                }

                r = clone_matrix(u).transpose();
                k1 = u.rows();
                
                blaze::DynamicMatrix<double> acProduct = a * c;
                blaze::DynamicMatrix<double> pu = clone_matrix(rnew).transpose() * acProduct;

                rsize.clear();
                rsize.resize(k1);

                for (int i = 0; i < k1; i++)
                {
                    rsize[i] = row_sum[i];
                }
   
                for (int i = 0; i < k1; i++)
                {
                    for (int j = 0; j < n2; j++)
                    {
                       pu(i, j) = pu(i, j)/rsize[i];
                    }
                }

                blaze::DynamicMatrix<double> lnu = blaze::log(pu + 0.0001);
                p = pu;  

                row_sum = sum<blaze::rowwise>(p);
                pi.reset();
                pi.resize(p.rows());

                for (int i = 0; i < row_sum.size(); i++)
                {
                    pi[i] = row_sum[i];
                }

                ln = lnu;      
                t++;
            }

            blaze::DynamicVector<double> mean_aux(n1);
            for (int i = 0; i < n1; i++)
            {
                mean_aux[i] = rw(i, ff[i]);
            }

            double div = blaze::mean(mean_aux);

            if (div < div1)
            {
                div1 = div;
                rmin = r;
                ko = k1;
            }
        }

        re.resize(1);
        re[re.size() - 1] = rmin;
    }
 
    if (re.size() == 0)
    {
        return false;
    }
    else
    {
        partition = re[0];
        return true;
    }
}

}
#endif
