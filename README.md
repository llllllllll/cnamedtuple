# namedtuple #


An implementation of namedtuple written in C for warp speed runtime.

Tested against Python2, maybe I will write for Python3.


## Numbers ##

### Setup ###


Use qualified imports so that we can tell which namedtuple we are using.

```python
In [1]: from namedtuple import namedtuple as cnamedtuple, _register_asdict

In [2]: from collections import namedtuple as stdnamedtuple, OrderedDict

In [3]: _register_asdict(OrderedDict)
```

NOTE: We need to use `_register_asdict` to pass in `OrderedDict`. The reason for
this is that I wanted to avoid a cycle between `collections` and `namedtuple` so
that you could import `namedtuple` from within `collections`.

This function sets what constructor will be used in the `_asdict` method. By
default, I needed to pick a static object, so I use `PyDict_Type = dict`.


### Type Creation ###


Time to create a new `namedtuple` class object.

```python
In [4]: %%timeit
   ...: stdnamedtuple('test', 'a b c d e f')
   ...:
1000 loops, best of 3: 505 µs per loop

In [5]: %%timeit
cnamedtuple('test', 'a b c d e f')
   ...:
10000 loops, best of 3: 298 µs per loop

```


### Instance Creation ###


Time to instantiate an instance of the new type.

```python
In [6]: s_nt = stdnamedtuple('s_nt', 'a b c d e f')

In [7]: %%timeit
   ...: s_nt(1, 2, 3, 4, 5, 6)
   ...:
1000000 loops, best of 3: 816 ns per loop

In [8]: c_nt = cnamedtuple('c_nt', 'a b c d e f')

In [9]: %%timeit
   ...: c_nt(1, 2, 3, 4, 5, 6)
   ...:
1000000 loops, best of 3: 294 ns per loop
```


### Item Access \(index\) ###


Time to access elements by index.
```python
In [10]: s_nt_inst = s_nt(1, 2, 3, 4, 5, 6)

In [11]: %%timeit
   ....: s_nt_inst[3]
   ....:
10000000 loops, best of 3: 39.8 ns per loop

In [12]: c_nt_inst = c_nt(1, 2, 3, 4, 5, 6)

In [13]: %%timeit
   ....:c_nt_inst[3]
   ....:
10000000 loops, best of 3: 39.5 ns per loop
```

NOTE: These both just inherit the `__getitem__` from `tuple`; however the C
inheritance caches the methods on the new type.


### Item Acess \(name\) ###


Time to access elements by name.
```python
In [14]: %%timeit
   ....: s_nt_inst.f
   ....:
10000000 loops, best of 3: 117 ns per loop

In [15]: %%timeit
   ....: c_nt_inst.f
   ....:
10000000 loops, best of 3: 51.7 ns per loop
```


### Item Replacement ###


Time to replace the fields. This creates a new instance of the `*_nt` type.

```python
In [16]: %%timeit
   ....: s_nt_inst._replace(a=2, b=3, c=4, d=5, e=6, f=7)
   ....:
100000 loops, best of 3: 2.79 µs per loop

In [17]: %%timeit
   ....: c_nt_inst._replace(a=2, b=3, c=4, d=5, e=6, f=7)
   ....:
1000000 loops, best of 3: 1.6 µs per loop
   ```


### Dict Conversion ###


Time to convert to an `OrderedDict`.

```python
In [18]: %%timeit
   ....: s_nt_inst._asdict()
   ....:
10000 loops, best of 3: 21.8 µs per loop

In [19]: %%timeit
   ....: c_nt_inst._asdict()
   ....:
10000 loops, best of 3: 21.5 µs per loop
```

NOTE: The bottleneck here is calling `OrderedDict`. Both the C and the Python
methods have a `PyObject*` pointing to `OrderedDict` directly accessible
(Through the `closed_as_dict` symbol in C or in the `co_closure` in Python) so
all this is really doing is calling it.


### Repr ###


Time to create a string representation of the instances.

```python

In [20]: %%timeit
repr(s_nt_inst)
   ....:
1000000 loops, best of 3: 780 ns per loop

In [21]: %%timeit
   ....: repr(c_nt_inst)
   ....:
1000000 loops, best of 3: 494 ns per loop
```

This _shouldn't_ be a bottleneck in your application; however, speed is speed.


## Pickle ##

Time to pickle and unpickle an instance.

```python
In [22]: s_nt_inst == loads(dumps(s_nt_inst))
Out[22]: True

In [23]: %%timeit
   ....: loads(dumps(s_nt_inst))
   ....:
10000 loops, best of 3: 24.9 µs per loop


In [24]: c_nt_inst == loads(dumps(c_nt_inst))
Out[24]: True

In [25]: %%timeit
   ....: loads(dumps(c_nt_inst))
   ....:
100000 loops, best of 3: 9.92 µs per loop
```


NOTE: The `*_nt_inst == loads(dumps(*_nt_inst))` is to show that it is being
pickled and unpickled correctly.


## Fields lookup ##


Time to get the fields as a tuple of strs.
```python
In [26]: s_nt._fields
Out[26]: ('a', 'b', 'c', 'd', 'e', 'f')

In [27]: %%timeit
   ....: s_nt._fields
10000000 loops, best of 3: 68.7 ns per loop

In [28]: c_nt._fields
Out[28]: ('a', 'b', 'c', 'd', 'e', 'f')

In [29]: %%timeit
   ....: c_nt._fields
   ....:
1000000 loops, best of 3: 178 ns per loop
   ```


For the other wins, the `_fields` is constructed from the `nt_fieldv` field of
the `nametuple` struct when requested. This keeps the memory profile lower by
not needing to hold onto this `PyTupleObject`. This is slower because the
`PyTupleObject` and `PyStringObject`s need to be allocated and constructed every
time. You manually cache it if you will be accessing it frequently.

```python
In [30]: c_nt._fields is c_nt._fields
Out[30]: False
```
