#ifndef _GENETIC_PARAMETER_HPP
#define _GENETIC_PARAMETER_HPP

#include <random>
#include <sstream>

namespace genetic_details
{

// template metaprogramming for getting maximum unsigned integral value from N bits
template <unsigned int N>
struct MAXVALUE
{
   enum : uint64_t
   {
      value = 2 * MAXVALUE<N - 1>::value
   };
};

// template specialization for initial case N = 0
template <>
struct MAXVALUE<0>
{
   enum
   {
      value = 1
   };
};

 

// // Mersenne Twister 19937 pseudo-random number generator
std::random_device rand_dev;
std::mt19937_64 rng(rand_dev());

// static class for generating random unsigned integral numbers
template <int N>
class Randomize
{
   static_assert(N > 0 && N <= 64, "in class genetic_details::Randomize<N>, template parameter N cannot be ouside interval [1,64], please choose an integer within this interval.");

 public:
   // computation only done once for each different N
   static constexpr uint64_t MAXVAL = MAXVALUE<N>::value - 1;

   // generating random unsigned long long integer on [0,MAXVAL]
   static uint64_t generate()
   {
      // class constructor only called once for each different N
      static std::uniform_int_distribution<uint64_t> udistrib(0, MAXVAL);
      return udistrib(rng);
   }
};




   // convert unsigned long long integer to binary string
std::string GetBinary(uint64_t value)
{
   std::bitset<sizeof(uint64_t)*CHAR_BIT> bits(value);
   // NB: CHAR_BIT = number of bits in char usually 8 but not always on older machines
   return bits.to_string();
}

// convert binary string to unsigned long long integer
uint64_t GetValue(const std::string& s)
{
   uint64_t value, x = 0;
   for (std::string::const_iterator it = s.begin(), end = s.end(); it != end; ++it) {
      x = (x << 1) + (*it - '0');
   }
   memcpy(&value, &x, sizeof(uint64_t));

   return value;
}

template <typename T, int N>
class Parameter
{
   static_assert(std::is_same<float, T>::value || std::is_same<double, T>::value || std::is_same<int, T>::value, "variable type can only be float, double or int, please amend.");
   template <typename H, typename K>
   friend class Chromosome;

 private:
   T lb;
   T ub;

 public:
   // nullary constructor
   Parameter() {}
   ~Parameter() {}

   // return encoded parameter size in number of bits
   int size() const
   {
      return N;
   }

   void boundaries(const T &nlb, const T &nub)
   {
      lb = nlb;
      ub = nub;
   }

   const std::vector<T> &getData() const { return std::vector<T>({lb, ub}); }

 private:
   // encoding random unsigned integer
   std::string encode() const
   {
      std::string str = GetBinary(Randomize<N>::generate());
      return str.substr(str.size() - N, N);
   }
   // encoding known unsigned integer
   std::string encode(T z) const
   {
      if (z > ub || z < lb)
      {
         throw std::invalid_argument("Error: in class genetic_details::Parameter<P,T,N>, encode parameter out of bound. Please reset parameter set.");
      }
      uint64_t value = Randomize<N>::MAXVAL * (z - lb) / (ub - lb);
      std::string str = GetBinary(value);
      return str.substr(str.size() - N, N);
   }
   // decoding string to real value
   T decode(const std::string &str) const
   {
      return lb + (GetValue(str) / static_cast<double>(Randomize<N>::MAXVAL)) * (ub - lb);
   }
};

} // namespace genetic_details

#endif // header guard
