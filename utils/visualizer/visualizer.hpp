#ifndef VISUALIZER
#define VISUALIZER

#include "blaze/Blaze.h"
#include <string.h>


namespace mat2bmp
{

template <class BlazeMatrix>
void blaze2bmp(BlazeMatrix m, std::string filename);

template <class BlazeMatrix>
void blaze2bmp_norm(BlazeMatrix m, std::string filename);

}


#include "visualizer.cpp"

#endif // VISUALIZER
