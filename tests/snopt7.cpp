/* Copyright 2017 PaGMO development team

This file is part of the PaGMO library.

The PaGMO library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The PaGMO library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the PaGMO library.  If not,
see https://www.gnu.org/licenses/. */

#define BOOST_TEST_MODULE snopt7_test
#include <boost/test/included/unit_test.hpp>

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/ipopt.hpp>
#include <pagmo/algorithms/snopt7.hpp>
#include <pagmo/io.hpp>
#include <pagmo/problems/ackley.hpp>
#include <pagmo/problems/cec2006.hpp>

using namespace pagmo;

struct my_prob {

    vector_double fitness(const vector_double &x) const
    {
        double f0 = x[0] + x[1] * x[1] + x[2] * x[3] + 3. * x[4];
        double f1 = x[4] + x[1] * x[2] + 3. * x[3];
        double f2 = x[0] + x[1] - x[2];
        return {f0, f1, f2};
    }
    vector_double::size_type get_nec() const
    {
        return 1;
    }
    vector_double::size_type get_nic() const
    {
        return 1;
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{-1, -1, -1, -1, -1}, {1, 1, 1, 1, 1}};
    }
    sparsity_pattern gradient_sparsity() const
    {
        return {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {2, 0}, {2, 1}, {2, 2}};
    }
};

BOOST_AUTO_TEST_CASE(snopt7_test)
{
    algorithm uda{snopt7{false}};
    print(uda, "\n");
    population pop(cec2006{7}, 1u);
    pop.get_problem().set_c_tol({1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6});
    uda.set_verbosity(1u);
    pop = uda.evolve(pop);
    print(uda, "\n");
}