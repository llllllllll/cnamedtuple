cnamedtuple 0.1.2
=================

An implementation of namedtuple written in c for warp speed.

Tested against Python3.4.2 and Python3.5 0a0.

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

Standard Library
~~~~~~~~~~~~~~~~

::

    Running: "stdnamedtuple('std_nt_type', 'a b c d e f')"
    1000 loops, best of 3: 769 usec per loop

    Running: "stdnamedtuple('std_nt_type', ('a', 'b', 'c', 'd', 'e', 'f'))"
    1000 loops, best of 3: 771 usec per loop

    Running: "std_nt_type(1, 2, 3, 4, 5, 6)"
    1000000 loops, best of 3: 1.04 usec per loop

    Running: "std_nt_inst.c"
    10000000 loops, best of 3: 0.12 usec per loop

    Running: "std_nt_inst[3]"
    10000000 loops, best of 3: 0.0532 usec per loop

    Running: "std_nt_inst._replace(a=2, b=3, c=4, d=5, e=6, f=7)"
    100000 loops, best of 3: 3.19 usec per loop

    Running: "std_nt_inst._asdict()"
    10000 loops, best of 3: 26.2 usec per loop

    Running: "repr(std_nt_inst)"
    1000000 loops, best of 3: 1.59 usec per loop

    Running: "dumps(std_nt_inst)"
    100000 loops, best of 3: 7.74 usec per loop

    Running: "loads(std_nt_inst_dumped)"
    100000 loops, best of 3: 4.46 usec per loop

This Implementation
~~~~~~~~~~~~~~~~~~~

::

    Running: "cnamedtuple('c_nt_type', 'a b c d e f')"
    100000 loops, best of 3: 18.8 usec per loop

    Running: "cnamedtuple('c_nt_type', ('a', 'b', 'c', 'd', 'e', 'f'))"
    100000 loops, best of 3: 18.5 usec per loop

    Running: "c_nt_type(1, 2, 3, 4, 5, 6)"
    1000000 loops, best of 3: 0.709 usec per loop

    Running: "c_nt_inst.c"
    10000000 loops, best of 3: 0.0657 usec per loop

    Running: "c_nt_inst[3]"
    10000000 loops, best of 3: 0.0523 usec per loop

    Running: "c_nt_inst._replace(a=2, b=3, c=4, d=5, e=6, f=7)"
    1000000 loops, best of 3: 1.85 usec per loop

    Running: "c_nt_inst._asdict()"
    10000 loops, best of 3: 25.9 usec per loop

    Running: "repr(c_nt_inst)"
    1000000 loops, best of 3: 1.32 usec per loop

    Running: "dumps(c_nt_inst)"
    100000 loops, best of 3: 5.72 usec per loop

    Running: "loads(c_nt_inst_dumped)"
    100000 loops, best of 3: 3.82 usec per loop

Contributing
------------

The project is hosted on
`github <https://github.com/llllllllll/cnamedtuple>`__.

Before submitting a patch, please make sure your Python code is
`PEP8 <https://www.python.org/dev/peps/pep-0008/>`__ compliant and your c
code is `PEP7 <https://www.python.org/dev/peps/pep-0007/>`__ compliant.

Contact
-------

Please file all bug reports on
`github <https://github.com/llllllllll/cnamedtuple/issues>`__.

For questions or comments, feel free to email me at joe@quantopian.com
