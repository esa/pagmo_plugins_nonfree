#define BOOST_TEST_MODULE worhp_test
#include <boost/lexical_cast.hpp>
#include <boost/test/included/unit_test.hpp>
#include <pagmo/algorithm.hpp>
#include <pagmo/io.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/ackley.hpp>
#include <pagmo/problems/cec2006.hpp>
#include <pagmo/problems/hock_schittkowsky_71.hpp>
#include <pagmo/problems/rastrigin.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/problems/luksan_vlcek1.hpp>
#include <pagmo/problems/inventory.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/types.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include <pagmo_plugins_nonfree/worhp.hpp>

#ifdef _MSC_VER
#define WORHP_LIB ".\\worhp_c.dll"
#elif defined __APPLE__
#define WORHP_LIB "./libworhp_c.dylib"
#elif defined __MINGW32__
#define WORHP_LIB ".\\libworhp_c.dll"
#else
#define WORHP_LIB "./libworhp_c.so"
#endif

using namespace pagmo;

/*-----------------------------------------------------------------------
 *
 * Minimise    f
 *
 * subject to      -0.5 <= x1 <=  INFTY
 *                   -2 <= x2 <=  INFTY
 *                    0 <= x3 <=  2
 *                   -2 <= x4 <=  2
 *                         g1 ==  1
 *               -INFTY <= g2 <= -1
 *                  2.5 <= g3 <=  5
 *
 * where         f (x1,x2,x3,x4) = x1^2 + 2 x2^2 - x3
 *               g1(x1,x2,x3,x4) = x1^2 + x3^2 + x1x3
 *               g2(x1,x2,x3,x4) = x3 - x4
 *               g3(x1,x2,x3,x4) = x2 + x4
 *
 * Optimal solution
 *                     x*  = (0, 0.5, 1, 2)
 *                   f(x*) = -0.5
 *                   g(x*) = (1, -1, 2.5)
 *
 *-----------------------------------------------------------------------*/
struct worhp_test_problem{
    vector_double fitness(const vector_double &x) const {
        vector_double retval(5, 0);
        retval[0] = x[0]*x[0] + 2 * x[1]*x[1] - x[2];
        retval[1] = x[0]*x[0] + x[2]*x[2] + x[0] * x[2]; 
        retval[2] = x[2] - x[3] + 1;
        retval[3] = x[1] + x[3] - 5;
        retval[4] = -(x[1] + x[3] - 2.5);
        return retval;
    }
    std::pair<vector_double, vector_double> get_bounds() const {
        return {{-0.5, -2, 0, -2},{5, 5, 2, 2}};
    }
    vector_double::size_type get_nec() const {
        return 1;
    }
    vector_double::size_type get_nic() const {
        return 3;
    }
    sparsity_pattern gradient_sparsity() const {
        return {{0,0},{0,1},{0,2},{1,0},{1,2},{2,2}, {2,3}, {3,1}, {3,3}, {4,1}, {4,3}};
    }
    vector_double gradient(const vector_double &x) const {
        return {
            2 * x[0], 4 * x[1], -1,
            2 * x[0] + x[2], 2 * x[2] + x[0],
            1, -1,
            1, 1,
            -1, -1
        };
    }
    std::vector<sparsity_pattern> hessians_sparsity() const {
        return {
            {{0,0}, {1,1}},
            {{0,0}, {2,0}, {2,2}},
            {},
            {},
            {},
        };
    }
    std::vector<vector_double> hessians(const vector_double &dv) const {
        return {
            {2, 4},
            {2, 1, 2},
            {},
            {},
            {}
        };
    }
};



BOOST_AUTO_TEST_CASE(construction)
{
    // We test construction of the worhp uda
    BOOST_CHECK_NO_THROW((worhp{false, WORHP_LIB}));
}

BOOST_AUTO_TEST_CASE(evolve)
{
    worhp uda{false, WORHP_LIB};
    uda.set_verbosity(1u);
    problem p{worhp_test_problem{}};
    p.set_c_tol(1e-6);
    uda.evolve(population{p, 1u});
    //uda.evolve(population{hock_schittkowsky_71{}, 1u});
    //uda.evolve(population{luksan_vlcek1{10u}, 1u});
    //uda.evolve(population{rastrigin{10u}, 1u});
    //uda.evolve(population{rosenbrock{5u}, 1u});
}
