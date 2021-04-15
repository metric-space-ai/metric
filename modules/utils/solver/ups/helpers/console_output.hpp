#ifndef _UPS_CONSOLE_OUTPUT_HPP
#define _UPS_CONSOLE_OUTPUT_HPP


#include <iterator> // needed for std::ostram_iterator


// taken from https://stackoverflow.com/questions/10750057/how-do-i-print-out-the-contents-of-a-vector
template <typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
  if ( !v.empty() ) {
    out << '[';
    std::copy (v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
    out << "\b\b]";
  }
  return out;
}



#endif // _UPS_CONSOLE_OUTPUT_HPP
