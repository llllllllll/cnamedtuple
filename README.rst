cnamedtuple 0.1.6
=================

An implementation of namedtuple written in c for warp speed.

Tested against Python 3.4 and 3.5, and 3.6 as of ``3084914245d2``

Warp Speed
----------

Time to go fast.

Setup
~~~~~

Use qualified imports so that we can tell which namedtuple we are using.

.. code:: python

    >>> from namedtuple import namedtuple as cnamedtuple
    >>> from collections import namedtuple as stdnamedtuple
    >>> from pickle import dumps, loads

Little numbers
~~~~~~~~~~~~~~

Cute graphs
```````````

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


Less cute numbers
`````````````````

These operations do not scale with number of fields.

``std_inst`` is an instance of a ``collections.namedtuple`` created type
with six named fields: a, b, c, d, e, and f.

``c_inst`` is an instance of a ``cnamedtuple.namedtuple`` created type
with six named fields: a, b, c, d, e, and f.

Field Access
''''''''''''

.. code:: python

   In [1]: %timeit std_inst.c
   10000000 loops, best of 3: 71.9 ns per loop

   In [2]: %timeit c_inst.c
   10000000 loops, best of 3: 38.4 ns per loop


Pickle Roundtrip
''''''''''''''''

   In [3]: %timeit loads(dumps(std_inst))
   100000 loops, best of 3: 6.28 µs per loop

   In [4]: %timeit loads(dumps(c_inst))
   100000 loops, best of 3: 4.72 µs per loop


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
