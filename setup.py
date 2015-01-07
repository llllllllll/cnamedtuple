from distutils.core import setup, Extension


setup(ext_modules=[Extension(
    "_namedtuple",
    ["namedtuple/_namedtuple.c"],
)])
