/*
		This Source Code Form is subject to the terms of the Mozilla Public
		License, v. 2.0. If a copy of the MPL was not distributed with this
		file, You can obtain one at http://mozilla.org/MPL/2.0/.

		Copyright (c) 2019 Panda Team
		Copyright (c) 2019 Oleg Popov
*/
#ifndef _METRIC_UTILS_GRAPH_PARTITION_CPP
#define _METRIC_UTILS_GRAPH_PARTITION_CPP

namespace metric {

bool perform_graph_partition(const blaze::DynamicMatrix<double> distanceMatrix,
    blaze::DynamicMatrix<int>& partition)
{

    srand(time(NULL));

    Random<double> rnd;
    Random<int> randomIntGenerator;

    int length = distanceMatrix.rows();

    //ra = RandomSample[Range[length], 100]
    //TO DO random choosen indices should not be limited to value 100
    blaze::DynamicVector<int> randomColumnsIndex;
    randomColumnsIndex.resize(100);
    for (int i = 0; i < 100; i++)
    {
        randomColumnsIndex[i] = i;
    }

    //dd = Table[du [[ra [[i]] ]], { i, Length[ra] }];
    blaze::DynamicMatrix<double> dd(randomColumnsIndex.size(), length);
    for (int i = 0; i < randomColumnsIndex.size(); i++)
    {
        for (int j = 0; j < length; j++)
        {
            dd(i, j) = distanceMatrix(randomColumnsIndex[i], j);
        }
    }
    
   /* std::cout << "dd Matrix: \n";
    PrintMatrix(randomColumns);*/

    /*blaze::DynamicMatrix<double> randomColumns(rows, randomColumnsIndex.size());
    for (int i = 0; i < randomColumnsIndex.size(); i++)
    {
        for (int j = 0; j < rows; j++)
        {
            randomColumns(j, i) = distanceMatrix(randomColumnsIndex[i], j);
        }
    }*/


    //a = 1. Transpose[dd]
    blaze::DynamicMatrix<double> a = CloneMatrix(dd).transpose();

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

          /*  std::cout << "r Matrix: \n";
            PrintMatrix(r);*/

            blaze::DynamicMatrix<double> dotAC = DotMatrix(a, c);
            blaze::DynamicMatrix<double> rtranspose = CloneMatrix(r).transpose();
            blaze::DynamicMatrix<double> p = DotMatrix(rtranspose, dotAC);

          /*  std::cout << "P Matrix: [" << p.rows() << "][" << p.columns() << "]\n";
            PrintMatrix(p);*/

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

            blaze::DynamicMatrix<double> ln = blaze::log(p + 0.0001);

            /*std::cout << "ln Matrix: [\n";
            PrintMatrix(ln);*/

            //blaze::DynamicMatrix<double> ln = p + blaze::pow(10, -5);
            //PrintMatrix(ln);
            /*for (int i = 0; i < ln.rows(); i++)
            {
                for (int j = 0; j < ln.columns(); j++)
                {
                    std::cout<<"old val: " << ln(i, j)<<"\n";
                    double localVal = ln(i, j);
                    double newVal = log(localVal);
                    ln(i, j) = newVal;
                    std::cout<<"new val: " << ln(i, j)<<"\n";
                }
            }*/

            //uRowSums = sum<blaze::rowwise>(p);
            blaze::DynamicVector<double> pi = sum<blaze::rowwise>(p);
              
          /*  std::cout << "pi vector: \n";
            PrintArray(pi);*/

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
  
                /*std::cout<<"LnMatrix: \n";
                PrintMatrix(ln);
                std::cout << "BMatrix: \n";
                PrintMatrix(b);*/
                //return true;

                blaze::DynamicMatrix<double> dotProduct = ln * b;

              /*  std::cout << "dotProduct: \n";
                PrintMatrix(dotProduct);*/

                rw.reset();
                rw.resize(dotProduct.rows(), dotProduct.columns());
                
                std::cout << "dot product Matrix: \n";
                PrintMatrix(dotProduct);

                std::cout << "pi arrray\n";
                PrintArray(pi);


                std::cout << "-----------------------------\n";
                std::cout << "-----------------------------\n";
                std::cout << "-----------------------------\n";
                std::cout << "-----------------------------\n";

                for (int i = 0; i < dotProduct.rows(); i++)
                {
                    for (int j = 0; j < dotProduct.columns(); j++)
                    {
                        rw(i, j) = -1 * (dotProduct(i, j) - pi[i]);
                    }
                }

               

                // ---------------------------------------------- ok until here -----------------------------------------------------

                rw = rw.transpose();

             /*   std::cout << "rw Matrix: \n";
                PrintMatrix(rw);*/

                ff.reset();
                ff.resize(rw.rows());
                for (int i = 0; i < rw.rows(); i++)
                {
                    double min = 0;
                    for(int j=0; j<rw.columns(); j++)
                    {          
                        //std::cout<<"check agains:"<< rw(i, j)<<"\n";
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

                std::cout << "ff array: \n";
                PrintArray(ff);

                /*div = Mean[Table[rw [[i, ff [[i]] ]], { i, n1 }]];
                ru = r * 0;
                For[i = 1, i <= n1, i++, ru [[i, ff [[i]] ]] = 1];*/
                blaze::DynamicVector<double> auxTable(n1);
                for (int i = 0; i < n1; i++)
                {
                    auxTable[i] = rw(i, ff[i]); 
                }
                
                double div = blaze::mean(auxTable);
                std::cout << "div: " << div << "\n";

                blaze::DynamicMatrix<int> ru(r.rows(), r.columns(), 0);
                for (int i = 0; i < n1; i++)
                {
                    ru(i, ff[i]) = 1;
                }

                //std::cout << "ru matrix: \n";
                //PrintMatrix(ru);

                //-------- move to function -------------
                /*blaze::DynamicVector<int> clustrow(ff);
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
                            clustrow.resize(clustrow.size() - 1, true); 
                        }
                    }
                }
                clustrow.shrinkToFit();*/
                //-------- move to function -------------


                blaze::DynamicVector<int> clustrow;
                for (int i = 0; i < ff.size(); i++)
                {
                    if (std::find(clustrow.begin(), clustrow.end(), ff[i]) == clustrow.end())
                    {
                        clustrow.extend(1);
                        clustrow[clustrow.size() - 1] = ff[i];
                    }   
                }

                //std::cout << "clustrow array: \n";
                //PrintArray(clustrow);

                blaze::DynamicMatrix<int> ruu = CloneMatrix(ru).transpose();
                blaze::DynamicMatrix<int> rne(clustrow.size(), ruu.columns());
                for (int i = 0; i < clustrow.size(); i++)
                {
                    for (int j = 0; j < ruu.columns(); j++)
                    {
                        rne(i, j) = ruu(clustrow[i], j);
                    }
                }


                std::cout << "rne matrix: \n";
                PrintMatrix(rne);

                blaze::DynamicMatrix<int> rnew = CloneMatrix(rne).transpose();

                r = rnew;

                u = CloneMatrix(rnew).transpose();

                blaze::DynamicVector<int> sdd;

                uRowSums = sum<blaze::rowwise>(u);
                
                std::cout << "u before extraction : \n";
                PrintMatrix(u);

                
                for (int i = 0; i < u.rows(); i++)
                {
                    if (uRowSums[i] == 0)
                    {
                        sdd.resize(sdd.size() + 1, true);
                        sdd[sdd.size() - 1] = i;
                    }
                }


                std::cout << "u before extraction : \n";
                PrintMatrix(u);

                std::cout << "sdd : \n";
                PrintArray(sdd);

                //If[Length[sdd] > 0, u = Delete[u, sdd]]
                if(sdd.size() > 0)
                {
                    //TO DO
                    //RemoveMatrixRows(u, sdd);
                    blaze::DynamicMatrix<int> backupU(u);
                    u.reset();
                    u.resize(backupU.rows(), backupU.columns());
                    int rowCount = 0;
                    for (int i = 0; i < backupU.rows(); i++)
                    {
                        bool skipRow = false;
                        for (int j = 0; j < sdd.size(); j++)
                        {
                            if (i == sdd[j])
                            {
                                skipRow = true;
                                break;
                            }
                        }

                        if (skipRow) continue;

                        for (int j = 0; j < backupU.columns(); j++)
                        {
                            u(rowCount, j) = backupU(i, j);
                        }
                        rowCount++;
                    }
                }

                std::cout << "u after extraction : \n";
                PrintMatrix(u);

                //clone(u).transpose;
                r = CloneMatrix(u).transpose();
                k1 = u.rows();
                
                blaze::DynamicMatrix<double> acProduct = a * c;
                
           /*     std::cout << "ac product : \n";
                PrintMatrix(acProduct);*/

                blaze::DynamicMatrix<double> pu = CloneMatrix(rnew).transpose() * acProduct;
              /*  
                std::cout << "pu matrix : \n";
                PrintMatrix(pu);*/


                rsize.clear();
                rsize.resize(k1);
                for (int i = 0; i < k1; i++)
                {
                    rsize[i] = uRowSums[i];
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
                
                std::cout << "p matrix : \n";
                PrintMatrix(p);

                uRowSums = sum<blaze::rowwise>(p);
                pi.reset();
                pi.resize(p.rows());

                for (int i = 0; i < uRowSums.size(); i++)
                {
                    pi[i] = uRowSums[i];
                }

                ln = lnu;
                t++;

                std::cout << "ln matrix : \n";
                PrintMatrix(ln);
            }


            std::cout << "ff: ------------------- \n";
            PrintArray(ff);

            std::cout <<"rw: ------------------- \n";
            PrintMatrix(rw);

            blaze::DynamicVector<double> meanAux(n1);
            for (int i = 0; i < n1; i++)
            {
                meanAux[i] = rw(i, ff[i]);
            }

            PrintArray(meanAux);

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
#endif
