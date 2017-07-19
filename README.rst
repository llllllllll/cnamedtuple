cnamedtuple 0.1.6
=================

An implementation of namedtuple written in c for warp speed.

Tested against Python 3.4 and 3.5, and 3.6.


Graphs
``````

These operations scale with the number of fields.

.. figure:: https://raw.githubusercontent.com/llllllllll/cnamedtuple/master/prof/type_creation_string.png
   :alt: Type creation from a string of field names.

   Type creation from a string of field names.
.. figure:: https://raw.githubusercontent.com/llllllllll/cnamedtuple/master/prof/type_creation_seq.png
   :alt: Type creation from a sequence of field names.

   Type creation from a sequence of field names.
.. figure:: https://raw.githubusercontent.com/llllllllll/cnamedtuple/master/prof/instance_creation.png
   :alt: Instance creation.

   Instance creation.

Benchmarks
----------

.. code::

   $ ./prof/bench
   Running with: Python 3.6.1 (default, Mar 27 2017, 00:27:06) [GCC 6.3.1 20170306]

   type creation from string with 1 field(s): namedtuple('NT', 'a')
     collections: 253 us +- 10 us
     cnamedtuple: 6.47 us +- 0.38 us
     ratio:       39.03

   type creation from string with 2 field(s): namedtuple('NT', 'a b')
     collections: 271 us +- 8 us
     cnamedtuple: 6.59 us +- 0.17 us
     ratio:       41.18

   type creation from string with 3 field(s): namedtuple('NT', 'a b c')
     collections: 290 us +- 6 us
     cnamedtuple: 6.77 us +- 0.17 us
     ratio:       42.80

   type creation from string with 4 field(s): namedtuple('NT', 'a b c d')
     collections: 308 us +- 12 us
     cnamedtuple: 7.07 us +- 0.21 us
     ratio:       43.62

   type creation from string with 5 field(s): namedtuple('NT', 'a b c d e')
     collections: 322 us +- 5 us
     cnamedtuple: 7.38 us +- 0.18 us
     ratio:       43.66

   type creation from string with 6 field(s): namedtuple('NT', 'a b c d e f')
     collections: 337 us +- 4 us
     cnamedtuple: 7.67 us +- 0.26 us
     ratio:       43.92

   type creation from string with 7 field(s): namedtuple('NT', 'a b c d e f g')
     collections: 353 us +- 5 us
     cnamedtuple: 8.11 us +- 0.18 us
     ratio:       43.46

   type creation from string with 8 field(s): namedtuple('NT', 'a b c d e f g h')
     collections: 369 us +- 7 us
     cnamedtuple: 8.34 us +- 0.20 us
     ratio:       44.18

   type creation from sequence with 1 field(s): namedtuple('NT', ['a'])
     collections: 252 us +- 4 us
     cnamedtuple: 6.21 us +- 0.10 us
     ratio:       40.56

   type creation from sequence with 2 field(s): namedtuple('NT', ['a', 'b'])
     collections: 351 us +- 44 us
     cnamedtuple: 6.57 us +- 0.18 us
     ratio:       53.42

   type creation from sequence with 3 field(s): namedtuple('NT', ['a', 'b', 'c'])
     collections: 289 us +- 6 us
     cnamedtuple: 6.77 us +- 0.23 us
     ratio:       42.66

   type creation from sequence with 4 field(s): namedtuple('NT', ['a', 'b', 'c', 'd'])
     collections: 310 us +- 12 us
     cnamedtuple: 6.92 us +- 0.21 us
     ratio:       44.84

   type creation from sequence with 5 field(s): namedtuple('NT', ['a', 'b', 'c', 'd', 'e'])
     collections: 322 us +- 3 us
     cnamedtuple: 7.31 us +- 0.16 us
     ratio:       44.02

   type creation from sequence with 6 field(s): namedtuple('NT', ['a', 'b', 'c', 'd', 'e', 'f'])
     collections: 336 us +- 5 us
     cnamedtuple: 7.52 us +- 0.18 us
     ratio:       44.72

   type creation from sequence with 7 field(s): namedtuple('NT', ['a', 'b', 'c', 'd', 'e', 'f', 'g'])
     collections: 352 us +- 6 us
     cnamedtuple: 8.03 us +- 0.19 us
     ratio:       43.83

   type creation from sequence with 8 field(s): namedtuple('NT', ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'])
     collections: 366 us +- 4 us
     cnamedtuple: 8.25 us +- 0.18 us
     ratio:       44.38

   type instance creation with positional arguments and 1 field(s): NT(0)
     collections: 333 ns +- 8 ns
     cnamedtuple: 221 ns +- 5 ns
     ratio:       1.51

   type instance creation with positional arguments and 2 field(s): NT(0, 1)
     collections: 344 ns +- 10 ns
     cnamedtuple: 228 ns +- 5 ns
     ratio:       1.51

   type instance creation with positional arguments and 3 field(s): NT(0, 1, 2)
     collections: 355 ns +- 8 ns
     cnamedtuple: 234 ns +- 7 ns
     ratio:       1.51

   type instance creation with positional arguments and 4 field(s): NT(0, 1, 2, 3)
     collections: 362 ns +- 6 ns
     cnamedtuple: 242 ns +- 5 ns
     ratio:       1.49

   type instance creation with positional arguments and 5 field(s): NT(0, 1, 2, 3, 4)
     collections: 378 ns +- 14 ns
     cnamedtuple: 249 ns +- 8 ns
     ratio:       1.52

   type instance creation with positional arguments and 6 field(s): NT(0, 1, 2, 3, 4, 5)
     collections: 382 ns +- 9 ns
     cnamedtuple: 254 ns +- 6 ns
     ratio:       1.51

   type instance creation with positional arguments and 7 field(s): NT(0, 1, 2, 3, 4, 5, 6)
     collections: 397 ns +- 10 ns
     cnamedtuple: 262 ns +- 11 ns
     ratio:       1.52

   type instance creation with positional arguments and 8 field(s): NT(0, 1, 2, 3, 4, 5, 6, 7)
     collections: 422 ns +- 11 ns
     cnamedtuple: 269 ns +- 5 ns
     ratio:       1.57

   type instance creation with keyword arguments and 1 field(s): NT(a=0)
     collections: 421 ns +- 7 ns
     cnamedtuple: 269 ns +- 5 ns
     ratio:       1.56

   type instance creation with keyword arguments and 2 field(s): NT(a=0, b=1)
     collections: 455 ns +- 7 ns
     cnamedtuple: 300 ns +- 6 ns
     ratio:       1.52

   type instance creation with keyword arguments and 3 field(s): NT(a=0, b=1, c=2)
     collections: 491 ns +- 15 ns
     cnamedtuple: 330 ns +- 12 ns
     ratio:       1.49

   type instance creation with keyword arguments and 4 field(s): NT(a=0, b=1, c=2, d=3)
     collections: 526 ns +- 11 ns
     cnamedtuple: 357 ns +- 6 ns
     ratio:       1.47

   type instance creation with keyword arguments and 5 field(s): NT(a=0, b=1, c=2, d=3, e=4)
     collections: 572 ns +- 9 ns
     cnamedtuple: 388 ns +- 11 ns
     ratio:       1.47

   type instance creation with keyword arguments and 6 field(s): NT(a=0, b=1, c=2, d=3, e=4, f=5)
     collections: 666 ns +- 20 ns
     cnamedtuple: 469 ns +- 15 ns
     ratio:       1.42

   type instance creation with keyword arguments and 7 field(s): NT(a=0, b=1, c=2, d=3, e=4, f=5, g=6)
     collections: 698 ns +- 17 ns
     cnamedtuple: 493 ns +- 11 ns
     ratio:       1.42

   type instance creation with keyword arguments and 8 field(s): NT(a=0, b=1, c=2, d=3, e=4, f=5, g=6, h=7)
     collections: 741 ns +- 15 ns
     cnamedtuple: 526 ns +- 17 ns
     ratio:       1.41

   field access: instance.b
     collections: 45.2 ns +- 1.2 ns
     cnamedtuple: 24.8 ns +- 0.8 ns
     ratio:       1.82


   median ratio: 1.82

Contributing
------------

The project is hosted on
`github <https://github.com/llllllllll/cnamedtuple>`__.

Before submitting a patch, please make sure your Python code is
`PEP8 <https://www.python.org/dev/peps/pep-0008/>`__ compliant and your
c code is `PEP7 <https://www.python.org/dev/peps/pep-0007/>`__
compliant.

Contact
-------

Please file all bug reports on
`github <https://github.com/llllllllll/cnamedtuple/issues>`__.

For questions or comments, feel free to email me at joe@quantopian.com
