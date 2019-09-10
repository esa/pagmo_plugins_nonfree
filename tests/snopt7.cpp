#define BOOST_TEST_MODULE snopt7_test
#define BOOST_TEST_DYN_LINK
#include <boost/lexical_cast.hpp>
#include <boost/test/included/unit_test.hpp>
#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/null_algorithm.hpp>
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

#include <pagmo_plugins_nonfree/snopt7.hpp>

#ifdef _MSC_VER
#define SNOPT7C_LIB ".\\snopt7_c.dll"
#elif defined __APPLE__
#define SNOPT7C_LIB "./libsnopt7_c.dylib"
#elif defined __MINGW32__
#define SNOPT7C_LIB ".\\libsnopt7_c.dll"
#else
#define SNOPT7C_LIB "./libsnopt7_c.so"
#endif

using namespace pagmo;

// a throwing problem. It throws every 50 evals
struct throwing_udp {
    static unsigned counter;
    vector_double fitness(const vector_double &x) const
    {
        double f0 = x[0] + x[1] * x[1] + x[2] * x[3] + 3. * x[4];
        double f1 = x[4] + x[1] * x[2] + 3. * x[3];
        double f2 = x[0] + x[1] - x[2];
        counter++;
        if (counter % 50u == 0u) {
            throw std::invalid_argument("error");
        }
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
unsigned throwing_udp::counter = 0u;

BOOST_AUTO_TEST_CASE(construction)
{
    // We test construction of the snopt7 uda
    BOOST_CHECK_NO_THROW((snopt7{false, SNOPT7C_LIB}));
    BOOST_CHECK_NO_THROW((snopt7{true, "./"}));
    BOOST_CHECK_NO_THROW((snopt7{true, "I CAN PUT WHATEVER IN HERE"}));
    // We test construction of a snopt7 pagmo algorithm
    BOOST_CHECK_NO_THROW((algorithm{snopt7{false, SNOPT7C_LIB}}));
    BOOST_CHECK_NO_THROW((algorithm{snopt7{true, SNOPT7C_LIB}}));
    BOOST_CHECK_NO_THROW((algorithm{snopt7{true, "I CAN PUT WHATEVER IN HERE"}}));
}

BOOST_AUTO_TEST_CASE(option_setting_mechanism)
{
    // We test the mechanism to set the snopt7 options
    // 1 - set integer options
    snopt7 uda{snopt7{true, SNOPT7C_LIB}};
    uda.set_integer_option("my_int_option1", 1);
    uda.set_integer_options({{"my_int_option2", 2}, {"my_int_option3", 3}});
    BOOST_CHECK_EQUAL(uda.get_integer_options()["my_int_option1"], 1);
    BOOST_CHECK_EQUAL(uda.get_integer_options()["my_int_option2"], 2);
    BOOST_CHECK_EQUAL(uda.get_integer_options()["my_int_option3"], 3);
    // 2 - set numeric options
    uda.set_numeric_option("my_num_option1", 1.1);
    uda.set_numeric_options({{"my_num_option2", 2.2}, {"my_num_option3", 3.3}});
    BOOST_CHECK_EQUAL(uda.get_numeric_options()["my_num_option1"], 1.1);
    BOOST_CHECK_EQUAL(uda.get_numeric_options()["my_num_option2"], 2.2);
    BOOST_CHECK_EQUAL(uda.get_numeric_options()["my_num_option3"], 3.3);
    // 3 - reset options
    BOOST_CHECK_EQUAL(uda.get_numeric_options().size(), 3u);
    BOOST_CHECK_EQUAL(uda.get_integer_options().size(), 3u);
    uda.reset_numeric_options();
    uda.reset_integer_options();
    BOOST_CHECK_EQUAL(uda.get_numeric_options().size(), 0u);
    BOOST_CHECK_EQUAL(uda.get_integer_options().size(), 0u);
    // 4 - Cannot set verbosity if SNOPT output is choosen
    BOOST_CHECK_THROW(uda.set_verbosity(1), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(evolve)
{
    {
        // We start testing the throws if the problem in the population is not suitable for snopt7
        snopt7 uda{true, SNOPT7C_LIB};

        BOOST_CHECK_THROW(uda.evolve(population{zdt{1}, 20u}), std::invalid_argument);
        BOOST_CHECK_THROW(uda.evolve(population{inventory{}, 20u}), std::invalid_argument);

        // We test the throw if the library is not well formed
        BOOST_CHECK_THROW(snopt7(true, "IDONOTEXIST").evolve(population{ackley{10}, 1u}), std::invalid_argument);
        BOOST_CHECK_THROW(snopt7(false, "IDONOTEXIST").evolve(population{ackley{10}, 1u}), std::invalid_argument);

        // We test the throw if the user has tried to set the derivative option
        uda.set_integer_option("Derivative option", 2);
        BOOST_CHECK_THROW(uda.evolve(population{ackley{10}, 1u}), std::invalid_argument);
        uda.reset_integer_options();
        // We test the throw if the user has tried to set an invalid (for the snopt7 API) option
        uda.set_integer_option("invalid_integer_option", 32);
        BOOST_CHECK_THROW(uda.evolve(population{ackley{10}, 1u}), std::invalid_argument);
        uda.reset_integer_options();
        uda.set_numeric_option("invalid_numeric_option", 32.32);
        BOOST_CHECK_THROW(uda.evolve(population{ackley{10}, 1u}), std::invalid_argument);
        uda.reset_numeric_options();

        // We change screen output to trigger that branch in the coverage
        snopt7 uda2{false, SNOPT7C_LIB};

        // We call the evolve having set the constraint tolerance as to test the setting of "Major feasibility
        // tolerance"
        problem prob{cec2006{1}};
        prob.set_c_tol({1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6, 1e-6});
        BOOST_CHECK_NO_THROW((uda2.evolve(population{prob, 1u})));
        // We now call the evolve. Not much to test in terms of outputs, so we just check that it does not throw.
        BOOST_CHECK_NO_THROW((uda2.evolve(population{hock_schittkowsky_71{}, 1u})));
        BOOST_CHECK_NO_THROW((uda2.evolve(population{cec2006{1}, 1u})));
        BOOST_CHECK_NO_THROW((uda2.evolve(population{ackley{10}, 1u})));
        BOOST_CHECK_NO_THROW((uda2.evolve(population{ackley{10}, 0u})));
        BOOST_CHECK_NO_THROW(uda.evolve(population{ackley{10}, 0u}));
    }

    // We now test the mechanism to rethrow exceptions thrown by the usrfun
    snopt7 uda{false, SNOPT7C_LIB};
    uda.set_verbosity(1u);
    population pop{throwing_udp{}, 1u};
    BOOST_CHECK_THROW(uda.evolve(pop), std::invalid_argument);
}
BOOST_AUTO_TEST_CASE(streams_and_log)
{
    snopt7 uda{false, SNOPT7C_LIB};
    population pop{cec2006{1}, 1u};
    uda.set_verbosity(1u);
    pop = uda.evolve(pop);
    BOOST_CHECK_EQUAL(uda.get_log().size(), 100);
    uda.set_verbosity(23u);
    BOOST_CHECK(uda.get_verbosity() == 23u);
    BOOST_CHECK(uda.get_name().find("SNOPT7") != std::string::npos);
    BOOST_CHECK(uda.get_extra_info().find("Name of the snopt7_c library") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(serialization_test)
{
    // Make one evolution
    problem prob{cec2006{7u}};
    population pop{prob, 10u, 23u};
    algorithm algo{snopt7{false, SNOPT7C_LIB}};
    algo.set_verbosity(1u);
    algo.extract<snopt7>()->set_integer_option("some_int", 4);
    algo.extract<snopt7>()->set_numeric_option("some_float", 2.2);
    pop = algo.evolve(pop);

    // Store the string representation of p.
    std::stringstream ss;
    auto before_text = boost::lexical_cast<std::string>(algo);
    auto before_log = algo.extract<snopt7>()->get_log();
    // Now serialize, deserialize and compare the result.
    {
        boost::archive::binary_oarchive oarchive(ss);
        oarchive << algo;
    }
    // Change the content of algo before deserializing.
    algo = algorithm{null_algorithm{}};
    {
        boost::archive::binary_iarchive iarchive(ss);
        iarchive >> algo;
    }
    auto after_text = boost::lexical_cast<std::string>(algo);
    BOOST_CHECK_EQUAL(before_text, after_text);
}
