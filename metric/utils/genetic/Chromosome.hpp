#ifndef _GENETIC_CHROMOSOME_HPP
#define _GENETIC_CHROMOSOME_HPP

namespace genetic_details {

template <typename P, typename T> class Chromosome {
	static_assert(std::is_same<float, T>::value || std::is_same<double, T>::value,
				  "variable type can only be float or double, please amend.");

  public:
	// constructor
	Chromosome(const Genetic<P, T> &ga);
	// copy constructor
	Chromosome(const Chromosome<P, T> &rhs);
	// create new chromosome
	void create();
	// initialize chromosome
	void initialize(P &initP);
	// evaluate chromosome
	void evaluate();
	// reset chromosome
	void reset();
	// set or replace kth gene by a new one
	void setGene(int k);
	// initialize or replace kth gene by a know value
	void initGene(int k, T value);
	// add bit to chromosome
	void addBit(char bit);
	// initialize or replace an existing chromosome bit
	void setBit(char bit, int pos);
	// flip an existing chromosome bit
	void flipBit(int pos);
	// get chromosome bit
	char getBit(int pos) const;
	// initialize or replace a portion of bits with a portion of another chromosome
	void setPortion(const Chromosome<P, T> &x, int start, int end);
	// initialize or replace a portion of bits with a portion of another chromosome
	void setPortion(const Chromosome<P, T> &x, int start);
	// get parameter value(s) from chromosome
	const P &getParam() const;
	// get objective function result
	const std::vector<T> &getResult() const;
	// get the total sum of all objective function(s) result
	T getTotal() const;
	// get constraint value(s)
	const bool getHardConstraint() const;
	const std::vector<T> getSoftConstraint() const;
	const std::vector<T> getConstraint() const;
	// return chromosome size in number of bits
	int size() const;
	// return number of chromosome bits to mutate
	T mutrate() const;
	// return number of genes in chromosome
	int nbgene() const;
	// return numero of generation this chromosome belongs to
	int nogen() const;
	// return lower bound(s)
	const P &lowerBound() const;
	// return upper bound(s)
	const P &upperBound() const;

  private:
	P param;							// estimated parameter(s)
	std::vector<T> result;				// chromosome objective function(s) result
	std::string chr;					// string of bits representing chromosome
	const Genetic<P, T> *ptr = nullptr; // pointer to genetic algorithm
										// end of recursion for initializing parameter(s) data
	template <int I, typename... Tp>
	inline typename std::enable_if<I == sizeof...(Tp), void>::type decodeChr(std::tuple<Tp...> &tp);

	// recursion for initializing parameter(s) data
	template <int I, typename... Tp>
		inline typename std::enable_if < I<sizeof...(Tp), void>::type decodeChr(std::tuple<Tp...> &tp);

	template <int I, typename... Tp>
	inline typename std::enable_if<I == sizeof...(Tp), void>::type encodeChr(std::tuple<Tp...> &tp);

	// recursion for initializing parameter(s) data
	template <int I, typename... Tp>
		inline typename std::enable_if < I<sizeof...(Tp), void>::type encodeChr(std::tuple<Tp...> &tp);

  public:
	T fitness; // chromosome fitness, objective function(s) result that can be modified (adapted to constraint(s), set
			   // to positive values, etc...)
  private:
	T total;	 // total sum of objective function(s) result
	int chrsize; // chromosome size (in number of bits)
	int numgen;	 // numero of generation
};

// constructor
template <typename P, typename T> genetic_details::Chromosome<P, T>::Chromosome(const Genetic<P, T> &ga)
{
	// param.resize(ga.nbparam);
	ptr = &ga;
	chrsize = ga.nbbit;
	numgen = ga.nogen;
}

// copy constructor
template <typename P, typename T> genetic_details::Chromosome<P, T>::Chromosome(const Chromosome<P, T> &rhs)
{
	param = rhs.param;
	result = rhs.result;
	chr = rhs.chr;
	ptr = rhs.ptr;
	// re-initializing fitness to its original value
	fitness = rhs.total;
	total = rhs.total;
	chrsize = rhs.chrsize;
	numgen = rhs.numgen;
}

// create new chromosome
template <typename P, typename T> inline void Chromosome<P, T>::create()
{
	chr.clear();

	for (const auto &x : ptr->param) {
		// encoding parameter random value
		std::string str = x->encode();
		chr.append(str);
	}
}

// end of recursion for initializing parameter(s) data
template <typename P, typename T>
template <int I, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type Chromosome<P, T>::encodeChr(std::tuple<Tp...> &tp)
{
}

// recursion for initializing parameter(s) data
template <typename P, typename T>
	template <int I, typename... Tp>
	inline typename std::enable_if < I<sizeof...(Tp), void>::type Chromosome<P, T>::encodeChr(std::tuple<Tp...> &tp)
{
	// copying parameter data
	std::string str = ptr->param[I]->encode(std::get<I>(tp));
	// recursing
	chr.append(str);
	encodeChr<I + 1>(tp);
}

// initialize chromosome (from known parameter values)
template <typename P, typename T> inline void Chromosome<P, T>::initialize(P &initP)
{
	chr.clear();
	encodeChr<0>(initP);
}

// end of recursion for initializing parameter(s) data
template <typename P, typename T>
template <int I, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type Chromosome<P, T>::decodeChr(std::tuple<Tp...> &tp)
{
}

// recursion for initializing parameter(s) data
template <typename P, typename T>
	template <int I, typename... Tp>
	inline typename std::enable_if < I<sizeof...(Tp), void>::type Chromosome<P, T>::decodeChr(std::tuple<Tp...> &tp)
{

	// copying parameter data
	std::get<I>(tp) = ptr->param[I]->decode(chr.substr(ptr->idx[I], ptr->param[I]->size()));
	// recursing

	decodeChr<I + 1>(tp);
}

// evaluate chromosome fitness
template <typename P, typename T> inline void Chromosome<P, T>::evaluate()
{
	decodeChr<0>(param);

	// computing objective result(s)
	result = ptr->Objective(param);
	// computing sum of all results (in case there is not only one objective functions)
	total = std::accumulate(result.begin(), result.end(), 0.0);
	// initializing fitness to this total
	fitness = total;
}

// reset chromosome
template <typename P, typename T> inline void Chromosome<P, T>::reset()
{
	chr.clear();
	result.clear();
	total = 0.0;
	fitness = 0.0;
}

// set or replace kth gene by a new one
template <typename P, typename T> inline void Chromosome<P, T>::setGene(int k)
{
#ifndef NDEBUG
	if (k < 0 || k >= ptr->nbparam) {
		throw std::invalid_argument("Error: in genetic_details::Chromosome< P,T>::setGene(int), argument cannot be "
									"outside interval [0,nbparam-1], please amend.");
	}
#endif

	// generating a new gene
	std::string s = ptr->param[k]->encode();
	// adding or replacing gene in chromosome
	chr.replace(ptr->idx[k], s.size(), s, 0, s.size());
}

// initialize or replace kth gene by a know value
template <typename P, typename T> inline void Chromosome<P, T>::initGene(int k, T x)
{
#ifndef NDEBUG
	if (k < 0 || k >= ptr->nbparam) {
		throw std::invalid_argument("Error: in genetic_details::Chromosome< P,T>::initGene(int), first argument cannot "
									"be outside interval [0,nbparam-1], please amend.");
	}
#endif

	// encoding gene
	std::string s = ptr->param[k]->encode(x);
	// adding or replacing gene in chromosome
	chr.replace(ptr->idx[k], s.size(), s, 0, s.size());
}

// add chromosome bit to chromosome (when constructing a new one)
template <typename P, typename T> inline void Chromosome<P, T>::addBit(char bit)
{
	chr.push_back(bit);

#ifndef NDEBUG
	if (chr.size() > chrsize) {
		throw std::out_of_range(
			"Error: in genetic_details::Chromosome< P,T>::setBit(char), exceeding chromosome size.");
	}
#endif
}

// initialize or replace an existing chromosome bit
template <typename P, typename T> inline void Chromosome<P, T>::setBit(char bit, int pos)
{
#ifndef NDEBUG
	if (pos >= chrsize) {
		throw std::out_of_range("Error: in genetic_details::Chromosome< P,T>::replaceBit(char, int), second argument "
								"cannot be equal or greater than chromosome size.");
	}
#endif

	std::stringstream ss;
	std::string str;
	ss << bit;
	ss >> str;
	chr.replace(pos, 1, str);
}

// flip an existing chromosome bit
template <typename P, typename T> inline void Chromosome<P, T>::flipBit(int pos)
{
#ifndef NDEBUG
	if (pos >= chrsize) {
		throw std::out_of_range("Error: in genetic_details::Chromosome< P,T>::flipBit(int), argument cannot be equal "
								"or greater than chromosome size.");
	}
#endif

	if (chr[pos] == '0') {
		chr.replace(pos, 1, "1");
	} else {
		chr.replace(pos, 1, "0");
	}
}

// get a chromosome bit
template <typename P, typename T> inline char Chromosome<P, T>::getBit(int pos) const
{
#ifndef NDEBUG
	if (pos >= chrsize) {
		throw std::out_of_range("Error: in genetic_details::Chromosome< P,T>::getBit(int), argument cannot be equal or "
								"greater than chromosome size.");
	}
#endif

	return chr[pos];
}

// initialize or replace a portion of bits with a portion of another chromosome (from position start to position end
// included)
template <typename P, typename T>
inline void Chromosome<P, T>::setPortion(const Chromosome<P, T> &x, int start, int end)
{
#ifndef NDEBUG
	if (start > chrsize) {
		throw std::out_of_range("Error: in genetic_details::Chromosome< P,T>::setPortion(const Chromosome< P,T>&, int, "
								"int), second argument cannot be greater than chromosome size.");
	}
#endif
	chr.replace(start, end - start + 1, x.chr, start, end - start + 1);
}

// initialize or replace a portion of bits with a portion of another chromosome (from position start to the end of he
// chromosome)
template <typename P, typename T> inline void Chromosome<P, T>::setPortion(const Chromosome<P, T> &x, int start)
{
#ifndef NDEBUG
	if (start > chrsize) {
		throw std::out_of_range("Error: in genetic_details::Chromosome< P,T>::setPortion(const Chromosome< P,T>&, "
								"int), second argument cannot be greater than chromosome size.");
	}
#endif

	chr.replace(start, chrsize, x.chr, start, x.chrsize);
}

// get parameter value(s) from chromosome
template <typename P, typename T> inline const P &Chromosome<P, T>::getParam() const { return param; }

// get objective function result
template <typename P, typename T> inline const std::vector<T> &Chromosome<P, T>::getResult() const { return result; }

// get the total sum of all objective function(s) result
template <typename P, typename T> inline T Chromosome<P, T>::getTotal() const { return total; }

// get constraint value(s) for this chromosome
template <typename P, typename T> const std::vector<T> Chromosome<P, T>::getConstraint() const
{
	return ptr->Constraint(param);
}

// return chromosome size in number of bits
template <typename P, typename T> inline int Chromosome<P, T>::size() const { return chrsize; }

// return mutation rate
template <typename P, typename T> inline T Chromosome<P, T>::mutrate() const { return ptr->mutrate; }

// return number of genes in chromosome
template <typename P, typename T> inline int Chromosome<P, T>::nbgene() const { return ptr->nbparam; }

// return numero of generation this chromosome belongs to
template <typename P, typename T> inline int Chromosome<P, T>::nogen() const { return numgen; }

// return lower bound(s)
template <typename P, typename T> inline const P &Chromosome<P, T>::lowerBound() const { return ptr->lowerBound; }

// return upper bound(s)
template <typename P, typename T> inline const P &Chromosome<P, T>::upperBound() const { return ptr->upperBound; }

} // namespace genetic_details

#endif // header guard
