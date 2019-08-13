/* PANDA presents
██████╗  ██████╗  ██████╗ ██████╗     ███╗   ███╗ █████╗ ███╗   ██╗███████╗     ██████╗ ██╗   ██╗ █████╗ ███╗   ██╗████████╗██╗   ██╗███╗   ███╗                                    
██╔══██╗██╔═══██╗██╔═══██╗██╔══██╗    ████╗ ████║██╔══██╗████╗  ██║██╔════╝    ██╔═══██╗██║   ██║██╔══██╗████╗  ██║╚══██╔══╝██║   ██║████╗ ████║                                    
██████╔╝██║   ██║██║   ██║██████╔╝    ██╔████╔██║███████║██╔██╗ ██║███████╗    ██║   ██║██║   ██║███████║██╔██╗ ██║   ██║   ██║   ██║██╔████╔██║                                    
██╔═══╝ ██║   ██║██║   ██║██╔══██╗    ██║╚██╔╝██║██╔══██║██║╚██╗██║╚════██║    ██║▄▄ ██║██║   ██║██╔══██║██║╚██╗██║   ██║   ██║   ██║██║╚██╔╝██║                                    
██║     ╚██████╔╝╚██████╔╝██║  ██║    ██║ ╚═╝ ██║██║  ██║██║ ╚████║███████║    ╚██████╔╝╚██████╔╝██║  ██║██║ ╚████║   ██║   ╚██████╔╝██║ ╚═╝ ██║                                    
╚═╝      ╚═════╝  ╚═════╝ ╚═╝  ╚═╝    ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚══════╝     ╚══▀▀═╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝    ╚═════╝ ╚═╝     ╚═╝                                    
                                                                                                                        Licensed under MPL 2.0.                                     
                                                                                                                        Michael Welsch (c) 2019.   

a library for computing with random distributed variables

a random distributed variable is just like a single value but with uncertainty around it.

you can do most operation with them like with real numbers 
and a few special ones, that do not make sense for real numbers.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Michael Welsch
*/

#ifndef _POOR_MANS_QUANTUM_HPP
#define _POOR_MANS_QUANTUM_HPP


#ifndef RV_SAMPLES
#define RV_SAMPLES 10000 // precision [1000 .. 1000000]
#endif

#ifndef RV_ERROR
#define RV_ERROR 0.05 // statistical error level alpha [0.001 .. 0.2]
#endif

// #define USE_VECTOR_SORT 1

#include "poor_mans_quantum/poor_mans_quantum.cpp"

#endif // headerguard
