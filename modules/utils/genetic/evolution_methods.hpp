#ifndef _GENETIC_EVOLUTION_METHODS_HPP
#define _GENETIC_EVOLUTION_METHODS_HPP

#include <random>

/* 
a library for selection, cross-over, mutation methods for the Poopulution class
*/

std::random_device rand_dev;
std::mt19937_64 rng(rand_dev());
std::uniform_real_distribution<> proba(0, 1);

template <typename T>
inline T uniform(T min, T max)
{
#ifndef NDEBUG
   if (min >= max)
   {
      throw std::invalid_argument("Error: in uniform(T, T), first argument must be < to second argument.");
   }
#endif

   return min + proba(rng) * (max - min);
}

/* 
   __|  __|  |     __|   __| __ __| _ _|   _ \   \ |     \  |  __| __ __| |  |   _ \  _ \    __| 
 \__ \  _|   |     _|   (       |     |   (   | .  |    |\/ |  _|     |   __ |  (   | |  | \__ \ 
 ____/ ___| ____| ___| \___|   _|   ___| \___/ _|\_|   _|  _| ___|   _|  _| _| \___/ ___/  ____/ 
                                                                                                 
*/

// proportional roulette wheel selection
// https://en.wikipedia.org/wiki/Fitness_proportionate_selection
template <typename P, typename T>
void proportional_roulette_wheel_selection(genetic_details::Population<P, T> &x)
{
   // adjusting all fitness to positive values
   x.adjustFitness();
   // computing fitness sum
   T fitsum = x.getSumFitness();

   // selecting mating population
   for (int i = 0, end = x.matsize(); i < end; ++i)
   {
      // generating a random fitness sum in [0,fitsum)
      T fsum = uniform<T>(0.0, fitsum);

      int j = 0;
      while (fsum >= 0.0)
      {
#ifndef NDEBUG
         if (j == x.popsize())
         {
            throw std::invalid_argument("Error: in proportional_roulette_wheel_selection(genetic_details::Population<P,T>&) index j cannot be equal to population size.");
         }
#endif
         fsum -= x(j)->fitness;
         j++;
      }
      // selecting element
      x.select(j - 1);
   }
}

// stochastic universal sampling selection
// https://en.wikipedia.org/wiki/Stochastic_universal_sampling
template <typename P, typename T>
void stochastic_universal_sampling_selection(genetic_details::Population<P, T> &x)
{
   // adjusting all fitness to positive values
   x.adjustFitness();
   // computing fitness sum
   T fitsum = x.getSumFitness();

   int matsize = x.matsize();
   // computing interval size
   T dist = fitsum / matsize;
   // initializing pointer
   T ptr = uniform<T>(0.0, dist);

   // selecting mating population
   for (int i = 0; i < matsize; ++i)
   {

      int j = 0;
      T fsum = 0;

      while (fsum <= ptr)
      {
#ifndef NDEBUG
         if (j == x.popsize())
         {
            throw std::invalid_argument("Error: in stochastic_universal_sampling_selection(genetic_details::Population<P,T>&) index j cannot be equal to population size.");
         }
#endif
         fsum += x(j)->fitness;
         j++;
      }
      // selecting element
      x.select(j - 1);

      // incrementing pointer
      ptr += dist;
   }
}

// classic linear rank-based selection
template <typename P, typename T>
void linear_rank_based_selection(genetic_details::Population<P, T> &x)
{
   int popsize = x.popsize();
   static std::vector<int> rank(popsize);
   static int ranksum;

   // this will only be run at the first generation
   if (x.nogen() == 1)
   {
      int n = popsize + 1;
      // generating ranks from highest to lowest
      std::generate_n(rank.begin(), popsize, [&n]() -> int { return --n; });
      // computing sum of ranks
      ranksum = .5 * popsize * (popsize + 1);
   }

   // selecting mating population
   for (int i = 0, end = x.matsize(); i < end; ++i)
   {
      // generating a random rank sum in [1,ranksum)
      int rsum = uniform<int>(1, ranksum);

      int j = 0;
      while (rsum > 0)
      {
#ifndef NDEBUG
         if (j == popsize)
         {
            throw std::invalid_argument("Error: in linear_rank_based_selection(genetic_details::Population<P,T>&) index j cannot be equal to population size.");
         }
#endif
         rsum -= rank[j];
         j++;
      }
      // selecting element
      x.select(j - 1);
   }
}

// linear rank-based selection with selective pressure
template <typename P, typename T>
void linear_rank_based_with_selective_pressure_selection(genetic_details::Population<P, T> &x)
{
   int popsize = x.popsize();
   static std::vector<T> rank(popsize);
   static T ranksum;

   // this will only be run at the first generation
   if (x.nogen() == 1)
   {
      // initializing ranksum
      ranksum = 0.0;
      // generating ranks from highest to lowest
      for (int i = 0; i < popsize; ++i)
      {
         rank[i] = 2 - x.SP() + 2 * (x.SP() - 1) * (popsize - i) / popsize;
         ranksum += rank[i];
      }
   }

   // selecting mating population
   for (int i = 0, end = x.matsize(); i < end; ++i)
   {
      // generating a random rank sum in [0,ranksum)
      T rsum = uniform<T>(0.0, ranksum);

      int j = 0;
      while (rsum >= 0.0)
      {
#ifndef NDEBUG
         if (j == popsize)
         {
            throw std::invalid_argument("Error: in linear_rank_based_with_selective_pressure_selection(genetic_details::Population<P,T>&) index j cannot be equal to population size.");
         }
#endif
         rsum -= rank[j];
         j++;
      }
      // selecting element
      x.select(j - 1);
   }
}

// tournament selection
// https://en.wikipedia.org/wiki/Tournament_selection
template <typename P, typename T>
void tournament_selection(genetic_details::Population<P, T> &x)
{
   int popsize = x.popsize();
   int tournament_selectionsize = x.tournament_selectionsize();

   // selecting mating population
   for (int i = 0, end = x.matsize(); i < end; ++i)
   {
      // selecting randomly a first element
      int bestIdx = uniform<int>(0, popsize);
      T bestFit = x(bestIdx)->fitness;

      // starting tournament
      for (int j = 1; j < tournament_selectionsize; ++j)
      {

         int idx = uniform<int>(0, popsize);
         T fit = x(idx)->fitness;

         if (fit > bestFit)
         {
            bestFit = fit;
            bestIdx = idx;
         }
      }
      // selecting element
      x.select(bestIdx);
   }
}

// transform ranking selection
template <typename P, typename T>
void transform_ranking_selection(genetic_details::Population<P, T> &x)
{
   static T c;
   // (re)initializing when running new GA
   if (x.nogen() == 1)
   {
      c = 0.2;
   }
   int popsize = x.popsize();
   // generating a random set of popsize values on [0,1)
   std::vector<T> r(popsize);
   std::for_each(r.begin(), r.end(), [](T &z) -> T { z = proba(rng); });
   // sorting them from highest to lowest
   std::sort(r.begin(), r.end(), [](T z1, T z2) -> bool { return z1 > z2; });
   // transforming population fitness
   auto it = x.begin();
   std::for_each(r.begin(), r.end(), [&it, popsize](T z) -> void {(*it)->fitness = ceil((popsize - popsize*exp(-c*z))/(1 - exp(-c))); it++; });

   // updating c for next generation
   c = c + 0.1; // arithmetic transition
   //c = c * 1.1; // geometric transition
   // computing fitness sum
   int fitsum = x.getSumFitness();

   // selecting mating population
   for (int i = 0, end = x.matsize(); i < end; ++i)
   {
      // generating a random fitness sum in [0,fitsum)
      T fsum = uniform<int>(0, fitsum);

      int j = 0;
      while (fsum >= 0)
      {
#ifndef NDEBUG
         if (j == popsize)
         {
            throw std::invalid_argument("Error: in transform_ranking_selection(genetic_details::Population<P,T>&) index j cannot be equal to population size.");
         }
#endif
         fsum -= x(j)->fitness;
         j++;
      }
      // selecting element
      x.select(j - 1);
   }
}

/*
                                                                   |    |               |            
   _|   _| _ \ (_-< (_-< ____| _ \ \ \ /  -_)   _|     ` \    -_)   _|    \    _ \   _` | (_-<       
 \__| _| \___/ ___/ ___/     \___/  \_/ \___| _|     _|_|_| \___| \__| _| _| \___/ \__,_| ___/                                                                                                                                                                                                                                      
*/

// one-point random cross-over of 2 chromosomes
template <typename P, typename T>
void one_point_random_crossover(const genetic_details::Population<P, T> &x, genetic_details::CHR<P, T> &chr1, genetic_details::CHR<P, T> &chr2)
{
   // choosing randomly 2 chromosomes from mating population
   int idx1 = uniform<int>(0, x.matsize());
   int idx2 = uniform<int>(0, x.matsize());
   // choosing randomly a position for cross-over
   int pos = uniform<int>(0, chr1->size());
   // transmitting portion of bits to new chromosomes
   chr1->setPortion(*x[idx1], 0, pos);
   chr2->setPortion(*x[idx2], 0, pos);
   chr1->setPortion(*x[idx2], pos + 1);
   chr2->setPortion(*x[idx1], pos + 1);
}

// two-point random cross-over of 2 chromosomes
template <typename P, typename T, int... N>
void two_point_random_crossover(const genetic_details::Population<P, T> &x, genetic_details::CHR<P, T> &chr1, genetic_details::CHR<P, T> &chr2)
{
   // choosing randomly 2 chromosomes from mating population
   int idx1 = uniform<int>(0, x.matsize());
   int idx2 = uniform<int>(0, x.matsize());
   // choosing randomly 2 positions for cross-over
   int pos1 = uniform<int>(0, chr1->size());
   int pos2 = uniform<int>(0, chr1->size());
   // ordering these 2 random positions
   int m = std::min(pos1, pos2);
   int M = std::max(pos1, pos2);
   // transmitting portion of bits new chromosomes
   chr1->setPortion(*x[idx1], 0, m);
   chr2->setPortion(*x[idx2], 0, m);
   chr1->setPortion(*x[idx2], m + 1, M);
   chr2->setPortion(*x[idx1], m + 1, M);
   chr1->setPortion(*x[idx1], M + 1);
   chr2->setPortion(*x[idx2], M + 1);
}

// uniform random cross-over of 2 chromosomes
template <typename P, typename T>
void uniform_random_crossover(const genetic_details::Population<P, T> &x, genetic_details::CHR<P, T> &chr1, genetic_details::CHR<P, T> &chr2)
{
   // choosing randomly 2 chromosomes from mating population
   int idx1 = uniform<int>(0, x.matsize());
   int idx2 = uniform<int>(0, x.matsize());

   for (int j = 0; j < chr1->size(); ++j)
   {
      // choosing 1 of the 2 chromosomes randomly
      if (proba(rng) < 0.50)
      {
         // adding its jth bit to new chromosome
         chr1->addBit(x[idx1]->getBit(j));
         chr2->addBit(x[idx2]->getBit(j));
      }
      else
      {
         // adding its jth bit to new chromosomes
         chr1->addBit(x[idx2]->getBit(j));
         chr2->addBit(x[idx1]->getBit(j));
      }
   }
}

/* 
   \  |  |  | __ __|  \ __ __| _ _|   _ \   \ |     \  |  __| __ __| |  |   _ \  _ \    __| 
  |\/ |  |  |    |   _ \   |     |   (   | .  |    |\/ |  _|     |   __ |  (   | |  | \__ \ 
 _|  _| \__/    _| _/  _\ _|   ___| \___/ _|\_|   _|  _| ___|   _|  _| _| \___/ ___/  ____/                                                                                          
*/

// boundary mutation: replacing a chromosome gene by its lower or upper bound
template <typename P, typename T>
void boundary_mutation(genetic_details::CHR<P, T> &chr)
{
   T mutrate = chr->mutrate();

   if (mutrate == 0.0)
      return;

   // getting chromosome lower bound(s)
   const std::vector<T> &lowerBound = chr->lowerBound();
   // getting chromosome upper bound(s)
   const std::vector<T> &upperBound = chr->upperBound();

   // looping on number of genes
   for (int i = 0; i < chr->nbgene(); ++i)
   {
      // generating a random probability
      if (proba(rng) <= mutrate)
      {
         // generating a random probability
         if (proba(rng) < .5)
         {
            // replacing ith gene by lower bound
            chr->initGene(i, lowerBound[i]);
         }
         else
         {
            // replacing ith gene by upper bound
            chr->initGene(i, upperBound[i]);
         }
      }
   }
}

// single point mutation: flipping a chromosome bit
template <typename P, typename T>
void single_point_mutation(genetic_details::CHR<P, T> &chr)
{
   T mutrate = chr->mutrate();

   if (mutrate == 0.0)
      return;

   // looping on chromosome bits
   for (int i = 0; i < chr->size(); ++i)
   {
      // generating a random probability
      if (proba(rng) <= mutrate)
      {
         // flipping ith bit
         chr->flipBit(i);
      }
   }
}

// uniform mutation: replacing a chromosome gene by a new one
template <typename P, typename T>
void uniform_mutation(genetic_details::CHR<P, T> &chr)
{
   T mutrate = chr->mutrate();

   if (mutrate == 0.0)
      return;

   // looping on number of genes
   for (int i = 0; i < chr->nbgene(); ++i)
   {
      // generating a random probability
      if (proba(rng) <= mutrate)
      {
         // replacing ith gene by a new one
         chr->setGene(i);
      }
   }
}

/* 
    \    _ \    \    _ \ __ __|  \ __ __| _ _|   _ \   \ |   __ __| _ \      __|   _ \   \ |   __| __ __| _ \    \   _ _|   \ | __ __| /  __| \ \      \  |  __| __ __| |  |   _ \  _ \    __| 
   _ \   |  |  _ \   __/    |   _ \   |     |   (   | .  |      |  (   |    (     (   | .  | \__ \    |     /   _ \    |   .  |    |  | \__ \    |    |\/ |  _|     |   __ |  (   | |  | \__ \ 
 _/  _\ ___/ _/  _\ _|     _| _/  _\ _|   ___| \___/ _|\_|     _| \___/    \___| \___/ _|\_| ____/   _|  _|_\ _/  _\ ___| _|\_|   _|  | ____/    |   _|  _| ___|   _|  _| _| \___/ ___/  ____/ 
                                                                                                                                     \_\       _/                                              
*/

// adapt population to genetic algorithm constraint(s)
template <typename P, typename T>
void DAC(genetic_details::Population<P, T> &x)
{
   // getting worst population objective function total result
   T worstTotal = x.getWorstTotal();
   for (auto it = x.begin(), end = x.end(); it != end; ++it) {
      // computing element constraint value(s) 
      const std::vector<T>& cst = (*it)->getConstraint();
      // adapting fitness if any constraint violated

      if (std::any_of(cst.cbegin(), cst.cend(), [](T x)->bool{return x >= 0.0;})) {
         auto val = std::accumulate(cst.cbegin(), cst.cend(), 0.0);
         (*it)->fitness = worstTotal - val;
      }

   }
}

#endif // headerguard
