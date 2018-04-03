#include <pagmo/algorithm.hpp>
#include <pagmo/archipelago.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo_plugins_nonfree/pagmo_plugins_nonfree.hpp>

using namespace pagmo;

int main()
{
    // 1 - Instantiate a pagmo problem constructing it from the Rosenbrock
    // UDP (user defined problem).
    problem prob{rosenbrock(30)};

    // 2 - Instantiate a pagmo_plugins_nonfree algorithm, in this case SNOPT. This assumes a library libsnopt7_c.so is
    // found in the path "/usr/local/lib/". Otherwise you will get a runtime error.
    algorithm algo(snopt7(false, "/usr/local/lib/libsnopt7_c.so"));

    // 3 - Instantiate an archipelago with 16 islands having each 1 individual (the initial guess)
    archipelago archi{16, algo, prob, 1};

    // 4 - Run the snopt7 calls in parallel on the 16 separate islands.
    archi.evolve(1);

    // 5 - Wait for the snopt7 calls to be finished
    archi.wait();

    // 6 - Print the fitness of the best solution in each island
    for (const auto &isl : archi) {
        print(isl.get_population().champion_f(), "\n");
    }
}
