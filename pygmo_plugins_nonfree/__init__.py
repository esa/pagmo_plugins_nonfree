# for python 2.0 compatibility
from __future__ import absolute_import as _ai

# Version setup.
from ._version import __version__

# We import the sub-modules into the root namespace
from .core import *

# And we explicitly import the test submodule
from . import test
