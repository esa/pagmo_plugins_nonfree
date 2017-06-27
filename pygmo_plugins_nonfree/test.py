from __future__ import absolute_import as _ai

import unittest as _ut


class snopt7_test_case(_ut.TestCase):
    """Test case for the snopt7 uda class.
    """

    def runTest(self):
        self.run_test_interface()

    def run_test_interface(self):
        import pygmo as pg
        from .core import snopt7
        uda = snopt7(screen_output=False,
                     absolute_lib_path="\\usr\\local\\lib\\")
        algo = pg.algorithm(uda)
        algo.extract(snopt7).set_integer_option("Major Iteration Limit", 1000)
        algo.extract(snopt7).set_numeric_option(
            "Major feasibility tolerance", 1E-10)


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
    # suite.addTest(snopt7_test_case())

    test_result = _ut.TextTestRunner(verbosity=2).run(suite)

    # Re-seed to random just in case anyone ever uses this function
    # in an interactive session or something.
    import random
    pg.set_global_rng_seed(random.randint(0, 2**30))

    if len(test_result.failures) > 0 or len(test_result.errors) > 0:
        retval = 1
    if retval != 0:
        raise RuntimeError('One or more tests failed.')
