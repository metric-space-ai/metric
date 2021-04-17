#ifndef _UPS_CONSOLE_OUTPUT_HPP
#define _UPS_CONSOLE_OUTPUT_HPP


#include <iterator> // needed for std::ostram_iterator
#include <iostream>
#include <sstream>


// uses code from https://stackoverflow.com/questions/10750057/how-do-i-print-out-the-contents-of-a-vector
template <typename T>
std::ostream & operator << (std::ostream & out, const std::vector<T> & v) {
    std::ostream delimeter (nullptr);
    std::stringbuf str;
    delimeter.rdbuf(&str);
    delimeter << " " << std::endl;
    if ( !v.empty() ) {
      out << '[' << std::endl;
      //std::copy (v.begin(), v.end(), std::ostream_iterator<T>(out, ", "));
      std::copy (v.begin(), v.end(), std::ostream_iterator<T>(out, str.str().c_str()));
      out << "]" << std::endl;
    }
    return out;
}


template <typename T>
std::ostream & operator << (std::ostream & out, const std::vector<std::vector<T>> & v) {
    std::ostream delimeter (nullptr);
    std::stringbuf str;
    delimeter.rdbuf(&str);
    delimeter << " " << std::endl;
    if ( !v.empty() ) {
      out << '[' << std::endl;
      for (size_t el = 0; el < v.size(); ++el) {
          out << v[el];
      }
      out << "]" << std::endl;

    }
    return out;
}


#endif // _UPS_CONSOLE_OUTPUT_HPP
