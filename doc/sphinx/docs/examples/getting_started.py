import pygmo as pg
import pygmo_plugins_nonfree as pg7

# 1 - Instantiate a pygmo problem constructing it from a UDP
# (user defined problem).
prob = pg.problem(pg.schwefel(30))

# 2 - Instantiate a pagmo_plugins_nonfree algorithm, in this case SNOPT. THis assumes a library snopt_c is found in
# the path "/usr/local/lib/". Otherwise you will get a runtime error.
algo = pg.algorithm(pg7.snopt7(false, "/usr/local/lib"))

# 3 - Instantiate an archipelago with 16 islands having each 20 individuals
archi = pg.archipelago(16, algo=algo, prob=prob, pop_size=20)

# 4 - Run the evolution in parallel on the 16 separate islands 10 times.
archi.evolve(10)

# 5 - Wait for the evolutions to be finished
archi.wait()

# 6 - Print the fitness of the best solution in each island
res = [isl.get_population().champion_f for isl in archi]
print(res)
