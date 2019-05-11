#pragma once
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <vector>
#include <utility>
#include <iterator>
template<typename T,typename U>
inline
std::ostream & operator << (std::ostream & ostr, const ::std::pair<T,U> & p) {
  return ostr << "< " << p.first << ", " << p.second << " >,";
}
// template<typename T, typename U>
// bool operator != (const std::pair<T,U> & lhs, const std::pair<T,U> & rhs) {
//   return lhs.first != rhs.first || lhs.second != rhs.second;
  
// }
struct Record {
  std::string date;
  int quantity;
  int total;
  int tip;
  std::string type;
  std::vector<int> tags;
};

bool operator == (const Record & lhs, const Record & rhs) {
  return lhs.date == rhs.date
      && lhs.quantity == rhs.quantity
      && lhs.total == rhs.total
      && lhs.tip == rhs.tip
      && lhs.type == rhs.type
      && lhs.tags == rhs.tags;
}
std::ostream & operator <<(std::ostream & ostr, const Record & rec) {
  return ostr << "{" << rec.date << ',' << rec.quantity << ',' << rec.total << ',' << rec.tip << ',' << rec.type << "}";
}
std::ostream & operator << (std::ostream & ostr, const std::vector<Record> & vec) {
  ostr << "[ ";
  for(auto & r : vec) {
    ostr << r << ',' << std::endl;
  }
  ostr << " ]";
  return ostr;
  
}
struct CompoundReduce {
  int64_t count;
  int64_t total;
  bool operator == (const CompoundReduce & rhs) const {
    return count == rhs.count && total == rhs.total;
  }
  bool operator < (const CompoundReduce & rhs) const {
    if(count < rhs.count)
      return true;
    if(count == rhs.count)
      return total < rhs.total;
    return false;
  }
};

inline std::ostream &  operator << (std::ostream & os, const CompoundReduce & c) {
  return os << "{ count:" << c.count << ", total:" << c.total << " }";
}
template<typename T1, typename T2>
inline
bool check_collections_equal(T1 begin1, T1 end1, T2 begin2, T2 end2) {
  auto sz1 = std::distance(begin1,end1);
  auto sz2 = std::distance(begin2,end2);
  if(sz1 != sz2)
    return false;
  auto p1 = begin1;
  auto p2 = begin2;
  for(; p1 < end1 && p2 < end2; ++p1, ++ p2) {
    if(*p1 != *p2)
      return false;
  }
  return true;
}

template<typename T1, typename T2>
inline
bool check_collections_equal(T1 lhs, T2 rhs) {
  return check_collections_equal(std::begin(lhs), std::end(lhs),
                                 std::begin(rhs), std::end(rhs));
}


template <typename T>
inline
void print_vector(const std::vector<T> & v) {
  std::cout << '[';
  bool first = true;
  for (auto const& element : v) {
    std::cout << (!first ? "," : "") << element;
    first = false;
  }
  std::cout << ']' << std::endl;
}

namespace boost
{
  namespace test_tools
    {
    namespace tt_detail {

    template<typename T,typename U>
    inline
    std::ostream & operator << (std::ostream & ostr, const ::std::pair<T,U> & p) {
      return ostr << "< " << p.first << ", " << p.second << " >,";
    }
    template<>
    struct print_log_value<CompoundReduce >
    {
      void operator()(std::ostream& os, CompoundReduce const& pr)
      {
        os << "{ count:" << pr.count << ", total:" << pr.total << " }";
      }
    };

    template<typename T,typename U>
    struct print_log_value<std::pair<T, U> >
        {
            void operator()(std::ostream& os, std::pair<T, U> const& pr)
            {
                os << "<" << std::get<0>(pr) << "," << std::get<1>(pr) << ">";
            }
        };

    template<typename T>
    struct print_log_value<std::vector<T> >
    {
      void operator()(std::ostream& os, std::vector<T> const& pr)
      {
        os << "[ ";
        for(auto & i : pr) {
          os << i << ", ";
        }
        os << "]";
      }
    };


    }
    }
}

