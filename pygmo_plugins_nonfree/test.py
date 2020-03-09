from __future__ import absolute_import as _ai

import unittest as _ut


class snopt7_test_case(_ut.TestCase):
    """Test case for the snopt7 uda class.
    """

    def runTest(self):
        self.run_test_interface()

    def run_test_interface(self):
        import pygmo as pg
        from .core import snopt7, _test_intermodule
        uda = snopt7(screen_output=False,
                     library="/usr/local/lib/libsnopt7.so")
        algo = pg.algorithm(uda)
        algo.extract(snopt7).set_integer_option("Major Iteration Limit", 1000)
        algo.extract(snopt7).set_numeric_option(
            "Major feasibility tolerance", 1E-10)
        algo.set_verbosity(1)
        name = algo.get_name()
        extra_info = algo.get_extra_info()
        pop = pg.population(pg.ackley(10),1)
        
        # We test the intermodule operability
        pop2 = _test_intermodule(pop)
        self.assertEqual(pop.get_f()[0], pop2.get_f()[0])


class worhp_test_case(_ut.TestCase):
    """Test case for the worhp uda class.
    """

    def runTest(self):
        self.run_test_interface()

    def run_test_interface(self):
        import pygmo as pg
        from .core import worhp, _test_intermodule
        uda = worhp(screen_output=False,
                     library="/usr/local/lib/libworhp.so")
        algo = pg.algorithm(uda)
        algo.extract(worhp).set_integer_option("MaxIter", 1000)
        algo.extract(worhp).set_numeric_option("TolFeas", 1E-10)
        algo.extract(worhp).set_bool_option("CheckStructureDF", True)
        algo.set_verbosity(1)
        name = algo.get_name()
        extra_info = algo.get_extra_info()
        pop = pg.population(pg.ackley(10),1)

        # We test the intermodule operability
        pop2 = _test_intermodule(pop)
        self.assertEqual(pop.get_f()[0], pop2.get_f()[0])


def run_test_suite(level=0):
    """Run the full test suite.
    This function will raise an exception if at least one test fails.
    Args:
        level(``int``): the test level (higher values run longer tests)
    """
    import pygmo as pg

    # Make test runs deterministic.
    # NOTE: we'll need to place the async/migration tests at the end, so that at
    # least the first N tests are really deterministic.
    pg.set_global_rng_seed(42)

    retval = 0
    suite = _ut.TestLoader().loadTestsFromTestCase(snopt7_test_case)
    suite.addTest(worhp_test_case())

    test_result = _ut.TextTestRunner(verbosity=2).run(suite)

    # Re-seed to random just in case anyone ever uses this function
    # in an interactive session or something.
    import random
    pg.set_global_rng_seed(random.randint(0, 2**30))

    if len(test_result.failures) > 0 or len(test_result.errors) > 0:
        retval = 1
    if retval != 0:
        raise RuntimeError('One or more tests failed.')
