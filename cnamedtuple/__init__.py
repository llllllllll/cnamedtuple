from collections import OrderedDict

from cnamedtuple._namedtuple import namedtuple, _register_asdict

__all__ = [
    'namedtuple'
]

__version__ = '0.1.6'

# Register `OrderedDict` as the constructor to use when calling `_asdict`.
# This step exists because at one point there was work being done to move
# this project into Python 3.5, and this works to solve a circular dependency
# between 'cnamedtuple/_namedtuple.c' ('Modules/_collectionsmodule.c'
# in cpython) and 'Lib/collections.py'.
#
# However, after discussion with the CPython folks, it was determined that
# this project will not be moved in after all, and will remain as a
# a third-party project.
_register_asdict(OrderedDict)

# Clean up the namespace for this module, the only public api should be
# `namedtuple`.
del _register_asdict
del OrderedDict
