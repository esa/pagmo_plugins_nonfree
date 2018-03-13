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
#include <pagmo/problems/inventory.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/types.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include <pagmo_plugins_nonfree/worhp.hpp>

#ifdef _MSC_VER
#define WORHP_LIB ".\\worhp.dll"
#elif defined __APPLE__
#define WORHP_LIB "./libworhp.dylib"
#elif defined __MINGW32__
#define WORHP_LIB ".\\libworhp.dll"
#else
#define WORHP_LIB "./libworhp.so"
#endif

using namespace pagmo;


BOOST_AUTO_TEST_CASE(construction)
{
    // We test construction of the worhp uda
    BOOST_CHECK_NO_THROW((worhp{false, WORHP_LIB}));
}

BOOST_AUTO_TEST_CASE(evolve)
{
    worhp uda{false, WORHP_LIB};
    uda.evolve(population{hock_schittkowsky_71{}, 1u});
}
