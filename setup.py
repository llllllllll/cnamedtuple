from distutils.core import setup, Extension
import sys

long_description = None

if 'upload' in sys.argv:
    with open('README.rst') as readme:
        long_description = readme.read()

setup(
    name='cnamedtuple',
    version='0.1.4',
    description='collections.namedtuple implemented in c.',
    author='Joe Jevnik',
    author_email='joe@quantopian.com',
    packages=[
        'cnamedtuple',
    ],
    long_description=long_description,
    license='Apache 2.0',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: Apache Software License',
        'Natural Language :: English',
        'Programming Language :: Python :: 3 :: Only',
        'Programming Language :: Python :: Implementation :: CPython',
        'Operating System :: OS Independent',
        'Topic :: Utilities',
    ],
    url="https://github.com/llllllllll/cnamedtuple",
    ext_modules=[
        Extension('cnamedtuple._namedtuple', ['cnamedtuple/_namedtuple.c']),
    ],
)
