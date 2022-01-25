#ifndef _GENETIC_IMPLEMENTATION_HPP
#define _GENETIC_IMPLEMENTATION_HPP

#include <iomanip>

template <typename P, typename T = double> class Genetic {
	static_assert(std::is_same<float, T>::value || std::is_same<double, T>::value,
				  "variable type can only be float or double, please amend.");

	// template <typename H,typename K>
	friend class genetic_details::Population<P, T>;
	template <typename H, typename K> friend class Chromosome;

	template <typename H, typename K = double> using Func = std::vector<K> (*)(const H &);

	template <typename H, typename K> using SelMethodFunc = void (*)(genetic_details::Population<H, K> &);
	template <typename H, typename K>
	using CrsMethodFunc = void (*)(const genetic_details::Population<H, K> &, genetic_details::CHR<H, K> &,
								   genetic_details::CHR<H, K> &);
	template <typename H, typename K> using MutMethodFunc = void (*)(genetic_details::CHR<H, K> &);
	template <typename H, typename K> using AdpMethodFunc = void (*)(genetic_details::Population<H, K> &);
	template <typename H> using HardConstraintFunc = bool(const H &);
	template <typename H, typename K = double> using SoftConstraintFunc = T(const H &);

  public:
	genetic_details::Population<P, T> pop;		// population of chromosomes
	std::vector<genetic_details::PAR<T>> param; // parameter(s)
	P lowerBound;								// parameter(s) lower bound
	P upperBound;								// parameter(s) upper bound
	std::vector<P> initialSet;					// initial set of parameter(s)
	std::vector<int> idx;						// indexes for chromosome breakdown
	// selection method initialized to roulette wheel selection
	SelMethodFunc<P, T> Selection = proportional_roulette_wheel_selection;
	// cross-over method initialized to 1-point cross-over
	CrsMethodFunc<P, T> CrossOver = one_point_random_crossover;
	// mutation method initialized to single-point mutation
	MutMethodFunc<P, T> Mutation = single_point_mutation;
	// adaptation to constraint(s) method
	AdpMethodFunc<P, T> Adaptation = nullptr;
	// constraint(s)
	std::vector<std::function<HardConstraintFunc<P>>> HardConstraints;
	std::vector<std::function<SoftConstraintFunc<P>>> SoftConstraints;
	// objective function pointer
	Func<P, T> Objective;
	T covrate = .50;   // cross-over rate
	T mutrate = .05;   // mutation rate
	T SP = 1.5;		   // selective pressure for linear_rank_based_with_selective_pressure_selection selection method
	T tolerance = 0.0; // terminal condition (inactive if equal to zero)

	int elitpop = 1;				   // elit population size
	int matsize;					   // mating pool size, set to popsize by default
	int tournament_selectionsize = 10; // tournament size
	int genstep = 10;				   // generation step for outputting results
	int precision = 5;				   // precision for outputting results

  public:
	// constructor
	Genetic(Func<P, T> objective, const std::vector<P> &args, int popsize = 100, int nbgen = 50, bool output = false);

	// set evolution methods
	void change_evolution_method(SelMethodFunc<P, T> sel, CrsMethodFunc<P, T> crs, MutMethodFunc<P, T> mut,
								 AdpMethodFunc<P, T> adp);
	void addHardConstraint(std::function<HardConstraintFunc<P>> cF) { HardConstraints.push_back(cF); };
	void addSoftConstraint(std::function<SoftConstraintFunc<P>> cF) { SoftConstraints.push_back(cF); };
	std::vector<T> (*Constraint)(const P &) = nullptr;
	// run genetic algorithm
	void run();
	void perform_evolutionary_steps(int numEvolStep);
	// return best chromosome
	const genetic_details::CHR<P, T> &result() const;
	std::vector<P> top(unsigned int n) const;
	std::vector<P> bottom(unsigned int n) const;
	void population();
	void resize_population(int np);
	void refresh_population(int refreshNum, bool random = true);
	void refresh_population(const std::vector<P> &, bool random = true);
	template <typename H> void addParam(const std::vector<H> &, const H &, const H &);

  public:
	int nbbit;	   // total number of bits per chromosome
	int nbgen;	   // number of generations
	int nogen = 0; // numero of generation
	int nbparam;   // number of parameters to be estimated
	int popsize;   // population size
	bool output;   // control if results must be outputted

	// end of recursion for initializing parameter(s) data

	// end of recursion for initializing parameter(s) data
	template <int I, typename... Tp>
	inline typename std::enable_if<I == sizeof...(Tp), void>::type init(const std::vector<std::tuple<Tp...>> &t);

	// recursion for initializing parameter(s) data
	template <int I, typename... Tp>
		inline typename std::enable_if < I<sizeof...(Tp), void>::type init(const std::vector<std::tuple<Tp...>> &t);

	// end of iterating parameter(s) data
	template <int I, typename... Tp>
	inline typename std::enable_if<I == sizeof...(Tp), void>::type printParam(std::tuple<Tp...> &paramset) const;

	// iterating parameter(s) data
	template <int I, typename... Tp>
		inline typename std::enable_if < I<sizeof...(Tp), void>::type printParam(std::tuple<Tp...> &paramset) const;

	// check inputs validity
	void check() const;
	// print results for each new generation
	void print(int n = 1) const;
};

// constructor
template <typename P, typename T>
Genetic<P, T>::Genetic(Func<P, T> objective, const std::vector<P> &args, int popsize, int nbgen, bool output)
{
	this->Objective = objective;
	// getting total number of bits per chromosome
	this->nbgen = nbgen;
	// getting number of parameters in the pack
	this->popsize = popsize;
	this->matsize = popsize;
	this->output = output;
	this->nbparam = std::tuple_size<P>::value;
	this->nbbit = 0;
	initialSet = args;
	this->init<0>(args);
}

// end of recursion for initializing parameter(s) data
template <typename P, typename T>
template <int I, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
Genetic<P, T>::init(const std::vector<std::tuple<Tp...>> &paramset)
{
}

// recursion for initializing parameter(s) data
template <typename P, typename T>
	template <int I, typename... Tp>
	inline typename std::enable_if <
	I<sizeof...(Tp), void>::type Genetic<P, T>::init(const std::vector<std::tuple<Tp...>> &paramset)
{

	// getting Ith parameter in tuple
	auto par = std::get<I>(paramset[0]);
	// getting Ith parameter initial data
	// const std::vector<T>& data = par.getData();
	// copying parameter data
	param.emplace_back(new genetic_details::Parameter<T>());
	this->nbbit += param[I]->size();

	// if parameter has initial value

	// initialSet.push_back(par);
	//  setting indexes for chromosome breakdown
	if (I == 0) {
		idx.push_back(0);
	} else {
		idx.push_back(idx[I - 1] + param[I]->size());
	}
	auto minV = std::get<I>(paramset[0]);
	auto maxV = std::get<I>(paramset[0]);
	for (const auto &x : paramset) {
		minV = std::min(minV, std::get<I>(x));
		maxV = std::max(maxV, std::get<I>(x));
	}
	std::get<I>(lowerBound) = minV;
	std::get<I>(upperBound) = maxV;
	param[I]->boundaries(minV, maxV);
	// recursing
	init<I + 1, Tp...>(paramset);
}

// check inputs validity
template <typename P, typename T> void Genetic<P, T>::check() const
{
	if (SP < 1.0 || SP > 2.0) {
		throw std::invalid_argument("Error: in class genetic_details::Genetic<P,T>, selective pressure (SP) cannot be "
									"outside [1.0,2.0], please choose a real value within this interval.");
	}
	if (elitpop > popsize || elitpop < 0) {
		throw std::invalid_argument("Error: in class genetic_details::Genetic<P,T>, elit population (elitpop) cannot "
									"outside [0,popsize], please choose an integral value within this interval.");
	}
	if (covrate < 0.0 || covrate > 1.0) {
		throw std::invalid_argument("Error: in class genetic_details::Genetic<P,T>, cross-over rate (covrate) cannot "
									"outside [0.0,1.0], please choose a real value within this interval.");
	}
	if (genstep <= 0) {
		throw std::invalid_argument("Error: in class genetic_details::Genetic<P,T>, generation step (genstep) cannot "
									"be <= 0, please choose an integral value > 0.");
	}
}

// set evolution methods
template <typename P, typename T>
void Genetic<P, T>::change_evolution_method(SelMethodFunc<P, T> sel, CrsMethodFunc<P, T> crs, MutMethodFunc<P, T> mut,
											AdpMethodFunc<P, T> adp)
{
	// selection method initialized to roulette wheel selection
	SelMethodFunc<P, T> Selection = sel;
	// cross-over method initialized to 1-point cross-over
	CrsMethodFunc<P, T> CrossOver = crs;
	// mutation method initialized to single-point mutation
	MutMethodFunc<P, T> Mutation = mut;
	AdpMethodFunc<P, T> Adaptation = adp;
}

// run genetic algorithm
template <typename P, typename T> void Genetic<P, T>::run()
{
	// checking inputs validity
	this->check();

	// setting adaptation method to default if needed
	if (!SoftConstraints.empty() && Adaptation == nullptr) {
		Adaptation = DAC;
	}

	// initializing population
	pop = genetic_details::Population<P, T>(*this);

	if (output) {
		std::cout << "\n Running Genetic Algorithm...\n";
		std::cout << " ----------------------------\n";
	}

	// creating population
	pop.creation();
	// initializing best result and previous best result
	T bestResult = pop(0)->getTotal();
	T prevBestResult = bestResult;
	// outputting results
	if (output)
		print();

	// starting population evolution
	for (nogen = 1; nogen <= nbgen; ++nogen) {
		// evolving population
		pop.evolution();
		// getting best current result
		bestResult = pop(0)->getTotal();
		// outputting results
		if (output)
			print();
		// checking convergence
		if (tolerance != 0.0) {
			if (fabs(bestResult - prevBestResult) < fabs(tolerance)) {
				break;
			}
			prevBestResult = bestResult;
		}
	}
	if (output)
		std::cout << std::endl;
}

// end of iterating parameter(s) data

template <typename P, typename T>
template <int I, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
Genetic<P, T>::printParam(std::tuple<Tp...> &paramset) const
{
}

// iterating parameter(s) data
template <typename P, typename T>
	template <int I, typename... Tp>
	inline typename std::enable_if <
	I<sizeof...(Tp), void>::type Genetic<P, T>::printParam(std::tuple<Tp...> &paramset) const
{
	std::cout << " X";
	std::cout << std::to_string(I + 1);
	std::cout << " = " << std::setw(9) << std::fixed << std::setprecision(precision) << std::get<I>(paramset) << " |";
	printParam<I + 1, Tp...>(paramset);
}

// return best chromosome
template <typename P, typename T> inline const genetic_details::CHR<P, T> &Genetic<P, T>::result() const
{
	return pop(0);
}

template <typename P, typename T> std::vector<P> Genetic<P, T>::top(unsigned int n) const
{
	std::vector<P> vP;
	for (auto i = 0; (i < n) && (i < pop.popsize()); i++) {
		vP.push_back(pop(i)->getParam());
	}
	return vP;
}

template <typename P, typename T> std::vector<P> Genetic<P, T>::bottom(unsigned int n) const
{
	std::vector<P> vP;
	for (auto i = pop.popsize() - 1; (i >= pop.popsize() - n) && (i > 0); i--) {
		vP.push_back(pop(i)->getParam());
	}
	return vP;
}

// print results for each new generation
template <typename P, typename T> void Genetic<P, T>::print(int n) const
{
	// getting best parameter(s) from best chromosome
	std::vector<P> bestParams = top(n);
	std::vector<T> bestResult = pop(0)->getResult();

	if (nogen % genstep == 0) {
		std::cout << " Generation = " << std::setw(std::to_string(nbgen).size()) << nogen << " |";
		for (auto pa : bestParams)
			printParam<0>(pa);
		for (unsigned i = 0; i < bestResult.size(); ++i) {
			std::cout << " F";
			if (bestResult.size() > 1) {
				std::cout << std::to_string(i + 1);
			}
			std::cout << "(x) = " << std::setw(12) << std::fixed << std::setprecision(precision) << bestResult[i];
			if (i < bestResult.size() - 1) {
				std::cout << " |";
			} else {
				std::cout << "\n";
			}
		}
	}
}

// print results for each new generation
template <typename P, typename T>
template <typename H>
void Genetic<P, T>::addParam(const std::vector<H> &staticParam, const H &minValue, const H &maxValue)
{
	if (staticParam.size() != popsize)
		return;
	param.emplace_back(new genetic_details::Parameter<T>());
	const int len = param.size();
	this->nbbit += param[len - 1]->size();
	param[len - 1]->boundaries(minValue, maxValue);
	for (auto i = 0; i < popsize; i++) {
		std::string ss = param[len - 1]->encode(staticParam[i]);
		pop.addParam(i, ss);
	}
}

// resize population
template <typename P, typename T> void Genetic<P, T>::resize_population(int npop)
{
	this->popsize = npop;
	this->matsize = npop;
	// initializing population
	pop = genetic_details::Population<P, T>(*this);

	// creating population
	pop.creation();
}

// resize population
template <typename P, typename T> void Genetic<P, T>::refresh_population(int refreshNum, bool random)
{
	pop.refresh(refreshNum, random);
}

// resize population
template <typename P, typename T> void Genetic<P, T>::refresh_population(const std::vector<P> &newParamset, bool random)
{
	pop.refresh(newParamset, random);
}

// run genetic algorithm
template <typename P, typename T> void Genetic<P, T>::perform_evolutionary_steps(int numEvolStep)
{

	nbgen = numEvolStep;

	// checking inputs validity
	this->check();

	// setting adaptation method to default if needed
	if (!SoftConstraints.empty() && Adaptation == nullptr) {
		Adaptation = DAC;
	}

	// initializing best result and previous best result
	T bestResult = pop(0)->getTotal();
	T prevBestResult = bestResult;
	// outputting results
	if (output)
		print();

	// starting population evolution
	for (nogen = 1; nogen <= nbgen; ++nogen) {
		// evolving population
		pop.evolution();
		// getting best current result
		bestResult = pop(0)->getTotal();
		// outputting results
		if (output)
			print();
		// checking convergence
		if (tolerance != 0.0) {
			if (fabs(bestResult - prevBestResult) < fabs(tolerance)) {
				break;
			}
			prevBestResult = bestResult;
		}
	}
	if (output)
		std::cout << std::endl;
}

#endif // header guard
