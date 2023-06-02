from setuptools import setup
from setuptools.dist import Distribution
import sys

NAME = 'pygmo_plugins_nonfree'
VERSION = '@pagmo_plugins_nonfree_VERSION@'
DESCRIPTION = 'Commercial solvers plugins for pygmo'
LONG_DESCRIPTION = 'A package affiliated to pygmo and providing additional solvers in the form of plugins (i.e. loading the third party libraries at run time)'
URL = 'https://github.com/esa/pagmo_plugins_nonfree'
AUTHOR = 'The pagmo development team'
AUTHOR_EMAIL = 'pagmo@googlegroups.com'
LICENSE = 'GPLv3+/LGPL3+'
CLASSIFIERS = [
    # How mature is this project? Common values are
    #   3 - Alpha
    #   4 - Beta
    #   5 - Production/Stable
    'Development Status :: 4 - Beta',

    'Operating System :: OS Independent',

    'Intended Audience :: Science/Research',
    'Topic :: Scientific/Engineering',
    'Topic :: Scientific/Engineering :: Mathematics',
    'Topic :: Scientific/Engineering :: Physics',

    'License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)',
    'License :: OSI Approved :: GNU Lesser General Public License v3 or later (LGPLv3+)',

    'Programming Language :: Python :: 2',
    'Programming Language :: Python :: 3'
]
KEYWORDS = 'science math physics optimization ai evolutionary-computing parallel-computing metaheuristics'
INSTALL_REQUIRES = ['numpy>=1.11', 'cloudpickle', 'pygmo==2.19.*']
PLATFORMS = ['Unix', 'Windows', 'OSX']


class BinaryDistribution(Distribution):

    def has_ext_modules(foo):
        return True


# Setup the list of external dlls.
import os
if os.name == 'nt':
    mingw_wheel_libs = 'mingw_wheel_libs.txt'
    l = open(mingw_wheel_libs, 'r').readlines()
    DLL_LIST = [os.path.basename(_[:-1]) for _ in l]

setup(name=NAME,
      version=VERSION,
      description=DESCRIPTION,
      long_description=LONG_DESCRIPTION,
      url=URL,
      author=AUTHOR,
      author_email=AUTHOR_EMAIL,
      license=LICENSE,
      classifiers=CLASSIFIERS,
      keywords=KEYWORDS,
      platforms=PLATFORMS,
      install_requires=INSTALL_REQUIRES,
      packages=['pygmo_plugins_nonfree'],
      # Include pre-compiled extension
      package_data={'pygmo_plugins_nonfree': ['core.pyd'] + \
                    DLL_LIST if os.name == 'nt' else ['core.so']},
      distclass=BinaryDistribution)
