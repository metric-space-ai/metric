/*
                This Source Code Form is subject to the terms of the Mozilla
   Public License, v. 2.0. If a copy of the MPL was not distributed with this
                file, You can obtain one at http://mozilla.org/MPL/2.0/.

                Copyright (c) 2019 Panda Team
                Copyright (c) 2019 Oleg Popov
*/
/*
Laplacians is a package containing graph algorithms, with an emphasis on tasks
related to spectral and algebraic graph theory. It contains (and will contain
more) code for solving systems of linear equations in graph Laplacians, low
stretch spanning trees, sparsifiation, clustering, local clustering, and
optimization on graphs.

All graphs are represented by sparse adjacency matrices. This is both for speed,
and because our main concerns are algebraic tasks. It does not handle dynamic
graphs. It would be very slow to implement dynamic graphs this way.

https://github.com/danspielman/Laplacians.jl
*/

#include <cstdlib>
#include <iostream>
void pcg_tests();
void IJVtests();
void CollectionTest();
void CollectionFunctionTest();
void sparsification_test();

int main()
{
    pcg_tests();

    IJVtests();

    CollectionTest();

    CollectionFunctionTest();

    // sparsification_test(); // error: ‘sparsify’ is not a member of ‘metric’; did you mean ‘sparse’?
}
