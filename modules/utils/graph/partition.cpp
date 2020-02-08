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
//#ifndef _METRIC_UTILS_GRAPH_PARTITION_CPP
//#define _METRIC_UTILS_GRAPH_PARTITION_CPP

namespace metric {

bool perform_graph_partition(const blaze::DynamicMatrix<double> distanceMatrix,
    blaze::DynamicMatrix<int>& partition)
{

    srand(time(NULL));

    Random<double> rnd;
    Random<int> randomIntGenerator;

    int rows = distanceMatrix.rows();
    int columns = distanceMatrix.columns();

    if (rows != columns)
        return false;

    //ra = RandomSample[Range[length], 100]
    //TO DO random choosen indices should not be limited to value 100
    blaze::DynamicVector<int> randomColumnsIndex;
    randomColumnsIndex.resize(100);
    for (int i = 0; i < 100; i++)
    {
        randomColumnsIndex[i] = 1;
    }

    //dd = Table[du [[ra [[i]] ]], { i, Length[ra] }];
   blaze::DynamicMatrix<double> randomColumns(randomColumnsIndex.size(), rows);
    for (int i = 0; i < randomColumnsIndex.size(); i++)
    {
        for (int j = 0; j < rows; j++)
        {
            randomColumns(i, j) = distanceMatrix(randomColumnsIndex[i], j);
        }
    }
    
    /*blaze::DynamicMatrix<double> randomColumns(rows, randomColumnsIndex.size());
    for (int i = 0; i < randomColumnsIndex.size(); i++)
    {
        for (int j = 0; j < rows; j++)
        {
            randomColumns(j, i) = distanceMatrix(randomColumnsIndex[i], j);
        }
    }*/


    //a = 1. Transpose[dd]
    blaze::DynamicMatrix<double> a = CloneMatrix(randomColumns).transpose();

    //n1 = Length[a];
    int n1 = a.rows();

    //n2 = Length[a[[1]]];
    int n2 = a.columns();

    //c = Table[Table[0, {i, n2}], {j, n2}];
    blaze::DynamicMatrix<double> c(n2, n2, 0);
    
    //For[i = 1, i <= n2, i++, c[[i, i]] = 1];
    for (int i = 0; i < n2; i++)
    {
        c(i, i) = 1;
    }

    blaze::DynamicMatrix<double> b = CloneMatrix(a).transpose();

    blaze::DynamicVector<blaze::DynamicMatrix<double>> re;
  
    for (int kk1 = 10; kk1 <= 10; kk1++)
    {
        int k1 = kk1;
        double div1 = blaze::pow(10, 10);
        int ko;
        blaze::DynamicMatrix<double> rmin;

        for (int toi = 1; toi <= 100; toi++)
        {
  
            blaze::DynamicMatrix<double> r(n1, kk1, 0);
            blaze::DynamicVector<double> rf(kk1, 1);
            blaze::DynamicMatrix<double> r1(r);
            blaze::DynamicVector<double> rf1(rf);

            int tt = 0;
            while (tt == 0)
            {     
                for (int i = 0; i < n1; i++)
                {
                    int l = RandomIntInRange(0, kk1);
                    r1(i, l) = 1;
                    rf1[l] = 0;
                    if (blaze::max(rf1) == 0)
                    {
                        tt = 1;
                        r = r1;
                    }
                }
            }

            blaze::DynamicMatrix<double> dotAC = DotMatrix(a, c);
            blaze::DynamicMatrix<double> p = DotMatrix(CloneMatrix(r).transpose(), dotAC);
            
            blaze::DynamicMatrix<double> u = CloneMatrix(r).transpose();

            k1 = u.rows();
            blaze::DynamicVector<double> rsize(k1);
            blaze::DynamicVector<double, blaze::columnVector> uRowSums;

            uRowSums = sum<blaze::rowwise>(u);

            for (int i = 0; i < k1; i++)
            {
                rsize[i] = uRowSums[i];
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
                int l = RandomIntInRange(0, kk1);
                r1(i, l) = 1;
                rf1[l] = 0; 
                r = r1;
            }
           
            p = DotMatrix(CloneMatrix(r).transpose(), DotMatrix(a, c));
            u = CloneMatrix(r).transpose();
            rsize.reset();
            uRowSums = sum<blaze::rowwise>(u);

            for (int i = 0; i < k1; i++)
            {
                rsize[i] = uRowSums[i];
            }

            for (int i = 0; i < k1; i++)
            {
                for (int j = 0; j < n2; j++)
                {
                    p(i, j) = p(i, j) / rsize[i];
                }
            }

            blaze::DynamicMatrix<double> ln = blaze::log(p + blaze::pow(10, -34));

            uRowSums = sum<blaze::rowwise>(p);
            blaze::DynamicVector<double> pi(p.rows());
            for (int i = 0; i < pi.size(); i++)
            {
                pi[i] = uRowSums[i];
            }

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

            while(t <= tmax)
            { 
  
                blaze::DynamicMatrix<double> dotProduct = ln * b;
                rw.reset();
                rw.resize(dotProduct.rows(), dotProduct.columns());
                
                for (int i = 0; i < dotProduct.rows(); i++)
                {
                    for (int j = 0; j < dotProduct.columns(); j++)
                    {
                        rw(i, j) = -1 * (dotProduct(i, j) - pi[j]);
                    }
                }
                rw = CloneMatrix(rw).transpose();
                ff.reset();
                ff.resize(rw.rows());
                for (int i = 0; i < rw.rows(); i++)
                {
                    for(int j=0; j<rw.columns(); j++)
                    {
                        if(j == 0)
                        {
                            ff[i] = 0;
                        }
                        else
                        {
                            if (rw(i, j) < ff[i])
                            {
                                ff[i] = j;
                            }
                        }
                    }
                }

                /*div = Mean[Table[rw [[i, ff [[i]] ]], { i, n1 }]];
                ru = r * 0;
                For[i = 1, i <= n1, i++, ru [[i, ff [[i]] ]] = 1];*/
                blaze::DynamicVector<double> auxTable(n1);
                for (int i = 0; i < n1; i++)
                {
                    auxTable[i] = rw(i, ff[i]); 
                }
                
                double div = blaze::mean(auxTable);

                blaze::DynamicMatrix<int> ru(r.rows(), r.columns(), 0);
                for (int i = 0; i < n1; i++)
                {
                    ru(i, ff[i]) = 1;
                }

                //-------- move to function -------------
                blaze::DynamicVector<int> clustrow(ff);
                int unionSize = clustrow.size();
                for (int i = 0; i < clustrow.size() - 1; i++)
                {
                    for (int j = i+1; j < clustrow.size(); j++)
                    {
                        if (clustrow[i] == clustrow[j])
                        {
                            unionSize -= 1;
                            for (int k = j; k < clustrow.size()-1; k++)
                            {
                                clustrow[k] = clustrow[k+1];
                            
                            }
                        }
                    }
                }
                clustrow.resize(unionSize, true);
                //-------- move to function -------------

                blaze::DynamicMatrix<int> ruu = CloneMatrix(ru).transpose();
                blaze::DynamicMatrix<int> rne(clustrow.size(), ruu.columns());
                for (int i = 0; i < clustrow.size(); i++)
                {
                    for (int j = 0; j < ruu.columns(); j++)
                    {
                        rne(i, j) = ruu(clustrow[i], j);
                    }
                }

                blaze::DynamicMatrix<int> rnew = CloneMatrix(ruu).transpose();

                r = rnew;

                u = CloneMatrix(rnew).transpose();

                blaze::DynamicVector<int> sdd;

                uRowSums = sum<blaze::rowwise>(u);
                for (int i = 0; i < u.rows(); i++)
                {
                    if (uRowSums[i] == 0)
                    {
                        sdd.resize(sdd.size() + 1, true);
                        sdd[sdd.size() - 1] = i;
                    }
                }

                //If[Length[sdd] > 0, u = Delete[u, sdd]]
                if(sdd.size() > 0)
                {
                    RemoveMatrixRows(u, sdd);
                }

                r = CloneMatrix(u).transpose();
                k1 = u.rows();
                
                blaze::DynamicMatrix<double> pu = CloneMatrix(rnew) * (a * c);
                
                rsize.clear();
                rsize.resize(k1);
                for (int i = 0; i < k1; i++)
                {
                    rsize[i] = uRowSums[i];
                }

   
                for (int i = 0; i < k1; i++)
                {
                    for (int j = 0; j <= n2; j++)
                    {
                        pu(i, j) = pu(i, j)/rsize[i];
                    }
                }

                blaze::DynamicMatrix<double> lnu = blaze::log(pu + blaze::pow(10, -34));
                p = pu;
                
                uRowSums = sum<blaze::rowwise>(p);
                pi.reset();
                pi.resize(p.rows());

                for (int i = 0; i < uRowSums.size(); i++)
                {
                    pi[i] = uRowSums[i];
                }

                ln = lnu;
                t++;
            }

            blaze::DynamicVector<double> meanAux(n1);
            for (int i = 0; i < n1; i++)
            {
                meanAux[n1] = rw(i, ff[i]);
            }
            double div = blaze::mean(meanAux);
            if(div < div1)
            {
                div1 = div;     
                rmin = r; 
                ko = k1;
            }
        }

        re.resize(re.size() + 1);
        re[re.size() - 1] = rmin;
    }
  
   return true;
}

}

//#endif
