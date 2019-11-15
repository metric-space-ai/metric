#ifndef _GENETIC_POPULATION_HPP
#define _GENETIC_POPULATION_HPP

#include <set>

namespace genetic_details
{

template <typename P, typename T>
class Population
{
   static_assert(std::is_same<float, T>::value || std::is_same<double, T>::value, "variable type can only be float or double, please amend.");

 public:
   // nullary constructor
   Population() {}
   // constructor
   Population(const Genetic<P, T> &ga);
   // create a population of chromosomes
   void creation();
   // evolve population, get next generation
   void evolution();

   // access element in current population at position pos
   const CHR<P, T> &operator()(int pos) const;
   // access element in mating population at position pos
   const CHR<P, T> &operator[](int pos) const;
   // return iterator to current population beginning
   typename std::vector<CHR<P, T>>::iterator begin();
   // return const iterator to current population beginning
   typename std::vector<CHR<P, T>>::const_iterator cbegin() const;
   // return iterator to current population ending
   typename std::vector<CHR<P, T>>::iterator end();
   // return const iterator to current population ending
   typename std::vector<CHR<P, T>>::const_iterator cend() const;
   // select element at position pos in current population and copy it into mating population
   void select(int pos);
   // set all fitness to positive values
   void adjustFitness();
   // compute fitness sum of current population
   T getSumFitness() const;
   // get worst objective function total result from current population
   T getWorstTotal() const;
   // return population size
   int popsize() const;
   // return mating population size
   int matsize() const;
   // return tournament size
   int tournament_selectionsize() const;
   // return numero of generation
   int nogen() const;
   // return number of generations
   int nbgen() const;
   // return selection pressure
   T SP() const;
   void refresh(int n, bool random = true);
   void refresh(const std::vector<P> &, bool random = true);
   void addParam(int chrId, const std::string& ss);

 private:
   std::vector<CHR<P, T>> curpop; // current population
   std::vector<CHR<P, T>> matpop; // mating population
   std::vector<CHR<P, T>> newpop; // new population

   const Genetic<P, T> *ptr = nullptr; // pointer to genetic algorithm
   int nbrcrov;                        // number of cross-over
   int matidx;                         // mating population index

   // elitism => saving best chromosomes in new population
   void elitism();
   // create new population from recombination of the old one
   void recombination();
   // complete new population randomly
   void completion();
   // update population (adapting, sorting)
   void updating();
};

// constructor
template <typename P, typename T>
Population<P, T>::Population(const Genetic<P, T> &ga)
{
   ptr = &ga;
   nbrcrov = floor(ga.covrate * (ga.popsize - ga.elitpop));
   // adjusting nbrcrov (must be an even number)
   if (nbrcrov % 2 != 0)
      nbrcrov -= 1;
   // for convenience, we add elitpop to nbrcrov
   nbrcrov += ga.elitpop;
   // allocating memory
   curpop.resize(ga.popsize);
   matpop.resize(ga.matsize);
}

// create a population of chromosomes
template <typename P, typename T>
void Population<P, T>::refresh(int n, bool random)
{
   if (n >= popsize())
   {
      for (auto curch : curpop)
      {
         curch->reset();
         curch->create();
         //curch->evaluate();
      }
   }
   else if (random)
   {
      std::set<int> randset;
      while (randset.size() < n)
         randset.insert(rand() % (popsize() - 1) + 0);

      for (auto i : randset)
      {
         curpop[i]->reset();
         curpop[i]->create();
         //curpop[i]->evaluate();
      }
   }
   else
   {
      this->updating();
      for (auto i = popsize() - n - 1; i < popsize(); i++)
      {
         curpop[i]->reset();
         curpop[i]->create();
         //curpop[i]->evaluate();
      }
   }
}

// create a population of chromosomes
template <typename P, typename T>
void Population<P, T>::refresh(const std::vector<P> &newparamset, bool random)
{
   int n = newparamset.size();
   if (n >= popsize())
   {
      for (auto i = 0; i < popsize(); i++)
      {
         P np = newparamset[i];

         curpop[i]->reset();
         curpop[i]->initialize(np);
         //curpop[i]->evaluate();
      }
   }
   else if (random)
   {
      std::set<int> randset;
      while (randset.size() < n)
         randset.insert(rand() % (popsize() - 1) + 0);
      std::set<int>::iterator it = randset.begin();

      for (auto i = 0; i < n; i++)
      {
         int pos = (*it);
         P np = newparamset[i];

         curpop[pos]->reset();
         curpop[pos]->initialize(np);
         //curpop[pos]->evaluate();
         it++;
      }
   }
   else
   {
      this->updating();
      for (auto i = popsize() - n - 1; i < popsize(); i++)
      {
         curpop[i]->reset();
         P np = newparamset[i];
         curpop[i]->initialize(np);
         //curpop[i]->evaluate();
      }
   }
}

// create a population of chromosomes
template <typename P, typename T>
void Population<P, T>::creation()
{
   int start = 0;
   // initializing first chromosome
   if (!ptr->initialSet.empty())
   {
      curpop[0] = std::make_shared<Chromosome<P, T>>(*ptr);
      P initP = ptr->initialSet[start];
      curpop[0]->initialize(initP);
      curpop[0]->evaluate();
      start++;
   }

   // getting the rest

   for (int i = start; i < ptr->popsize; ++i)
   {
      curpop[i] = std::make_shared<Chromosome<P, T>>(*ptr);
      curpop[i]->create();
      curpop[i]->evaluate();
   }
   // updating population
   this->updating();
}

// population evolution (selection, recombination, completion, mutation), get next generation
template <typename P, typename T>
void Population<P, T>::evolution()
{
   // initializing mating population index
   matidx = 0;
   // selecting mating population
   ptr->Selection(*this);
   // applying elitism if required
   this->elitism();
   // crossing-over mating population
   this->recombination();
   // completing new population
   this->completion();
   // moving new population into current population for next generation
   curpop = std::move(newpop);

   // updating population
   this->updating();
}

// elitism => saving best chromosomes in new population, making a copy of each elit chromosome
template <typename P, typename T>
void Population<P, T>::elitism()
{
   // (re)allocating new population
   newpop.resize(ptr->popsize);

   if (ptr->elitpop > 0)
   {
      // copying elit chromosomes into new population
      std::transform(curpop.cbegin(), curpop.cend(), newpop.begin(), [](const CHR<P, T> &chr) -> CHR<P, T> { return std::make_shared<Chromosome<P, T>>(*chr); });
   }
}

// create new population from recombination of the old one
template <typename P, typename T>
void Population<P, T>::recombination()
{
   // creating a new population by cross-over
   for (int i = ptr->elitpop; i < nbrcrov; i = i + 2)
   {
      // initializing 2 new chromosome
      newpop[i] = std::make_shared<Chromosome<P, T>>(*ptr);
      newpop[i + 1] = std::make_shared<Chromosome<P, T>>(*ptr);
      // crossing-over mating population to create 2 new chromosomes
      ptr->CrossOver(*this, newpop[i], newpop[i + 1]);
      // mutating new chromosomes
      ptr->Mutation(newpop[i]);
      ptr->Mutation(newpop[i + 1]);
      // evaluating new chromosomes
      newpop[i]->evaluate();
      newpop[i + 1]->evaluate();
   }
}

// complete new population
template <typename P, typename T>
void Population<P, T>::completion()
{
   for (int i = nbrcrov; i < ptr->popsize; ++i)
   {
      // selecting chromosome randomly from mating population
      newpop[i] = std::make_shared<Chromosome<P, T>>(*matpop[uniform<int>(0, ptr->matsize)]);
      // mutating chromosome
      ptr->Mutation(newpop[i]);
      // evaluating chromosome
      newpop[i]->evaluate();
   }
}

// update population (adapting, sorting)
template <typename P, typename T>
void Population<P, T>::updating()
{
   // adapting population to constraints
   if (!ptr->SoftConstraints.empty())
   {
      ptr->Adaptation(*this);
   }
   for(auto x: curpop){ x->evaluate(); }
   // sorting chromosomes from best to worst fitness
   std::sort(curpop.begin(), curpop.end(), [](const CHR<P, T> &chr1, const CHR<P, T> &chr2) -> bool { return chr1->fitness > chr2->fitness; });
}

// access element in current population at position pos
template <typename P, typename T>
const CHR<P, T> &Population<P, T>::operator()(int pos) const
{
#ifndef NDEBUG
   if (pos > ptr->popsize - 1)
   {
      throw std::invalid_argument("Error: in genetic_details::Population<P,T>::operator()(int), exceeding current population memory.");
   }
#endif

   return curpop[pos];
}

// access element in mating population at position pos
template <typename P, typename T>
const CHR<P, T> &Population<P, T>::operator[](int pos) const
{
#ifndef NDEBUG
   if (pos > ptr->matsize - 1)
   {
      throw std::invalid_argument("Error: in genetic_details::Population<P,T>::operator[](int), exceeding mating population memory.");
   }
#endif

   return matpop[pos];
}

// return iterator to current population beginning
template <typename P, typename T>
inline typename std::vector<CHR<P, T>>::iterator Population<P, T>::begin()
{
   return curpop.begin();
}

// return const iterator to current population beginning
template <typename P, typename T>
inline typename std::vector<CHR<P, T>>::const_iterator Population<P, T>::cbegin() const
{
   return curpop.cbegin();
}

// return iterator to current population ending
template <typename P, typename T>
inline typename std::vector<CHR<P, T>>::iterator Population<P, T>::end()
{
   return curpop.end();
}

// return const iterator to current population ending
template <typename P, typename T>
inline typename std::vector<CHR<P, T>>::const_iterator Population<P, T>::cend() const
{
   return curpop.cend();
}

// select element at position pos in current population and copy it into mating population
template <typename P, typename T>
inline void Population<P, T>::select(int pos)
{
#ifndef NDEBUG
   if (pos > ptr->popsize - 1)
   {
      throw std::invalid_argument("Error: in genetic_details::Population<P,T>::select(int), exceeding current population memory.");
   }
   if (matidx == ptr->matsize)
   {
      throw std::invalid_argument("Error: in genetic_details::Population<P,T>::select(int), exceeding mating population memory.");
   }
#endif

   matpop[matidx] = curpop[pos];
   matidx++;
}

// set all fitness to positive values (used in proportional_roulette_wheel_selection and stochastic_universal_sampling_selection selection methods)
template <typename P,typename T>
void Population<P,T>::adjustFitness()
{
   //this->updating();
   // getting worst population fitness
   auto itWorstFitness = std::min_element(curpop.begin(), curpop.end(), [](const CHR<P,T>& chr1, const CHR<P,T>& chr2)->bool{return chr1->fitness < chr2->fitness;});
   T worstFitness = (*itWorstFitness)->fitness;
   
   if (worstFitness < 0) {
      // getting best fitness
     auto itBestFitness = std::max_element(curpop.begin(), curpop.end(), [](const CHR<P,T>& chr1, const CHR<P,T>& chr2)->bool{return chr1->fitness < chr2->fitness;});
     T bestFitness = (*itBestFitness)->fitness;
   
      // case where all fitness are equal and negative
      if (worstFitness == bestFitness) {
         std::for_each(curpop.begin(), curpop.end(), [](CHR<P,T>& chr)->void{chr->fitness *= -1;});
       } else {
         std::for_each(curpop.begin(), curpop.end(), [worstFitness](CHR<P,T>& chr)->void{chr->fitness -= worstFitness;});
      }
   }
}

// compute population fitness sum (used in transform_ranking_selection, proportional_roulette_wheel_selection and stochastic_universal_sampling_selection selection methods)
template <typename P, typename T>
inline T Population<P, T>::getSumFitness() const
{
   return std::accumulate(curpop.cbegin(), curpop.cend(), 0.0, [](T sum, const CHR<P, T> &chr) -> T { return sum + T(chr->fitness); });
}

// get worst objective function total result from current population (used in constraint(s) adaptation)
template <typename P, typename T>
inline T Population<P, T>::getWorstTotal() const
{
   auto it = std::min_element(curpop.begin(), curpop.end(), [](const CHR<P, T> &chr1, const CHR<P, T> &chr2) -> bool { return chr1->getTotal() < chr2->getTotal(); });

   return (*it)->getTotal();
}

// return population size
template <typename P, typename T>
inline int Population<P, T>::popsize() const
{
   return ptr->popsize;
}

// return mating population size
template <typename P, typename T>
inline int Population<P, T>::matsize() const
{
   return ptr->matsize;
}

// return tournament size
template <typename P, typename T>
inline int Population<P, T>::tournament_selectionsize() const
{
   return ptr->tournament_selectionsize;
}

// return numero of generation
template <typename P, typename T>
inline int Population<P, T>::nogen() const
{
   return ptr->nogen;
}

// return number of generations
template <typename P, typename T>
inline int Population<P, T>::nbgen() const
{
   return ptr->nbgen;
}

// return selection pressure
template <typename P, typename T>
inline T Population<P, T>::SP() const
{
   return ptr->SP;
}

// return selection pressure
template <typename P,typename T>
void Population<P,T>::addParam(int chrId, const std::string& ss)
{
  curpop[chrId]->append(ss);
}

} // namespace genetic_details

#endif // header guard
