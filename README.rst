``sortedmap 0.2``
=================

A sorted mapping object.

``sortedmap`` is a python ``dict`` api interface to the C++ ``std::map`` type.
``sortedmap`` implements the full ``dict`` object interface with a few
differences:

1. Objects are stored in a red black tree. All keys must be comparable to
   eachother though they do not need to be hashable. This means all keys must
   implement at least ``__lt__`` and ``__eq__``.

2. ``O(log(n))`` lookup, insert, and deletes because of the red- black tree
   backing. This is worse than ``dict`` which offers ``O(1)`` lookup, insert,
   and delete. The ``C++`` implementation offers low constants

3. Iteration is in sorted order for ``.keys()`` , ``.values()`` and
   ``.items()``.

4. ``popitem`` accepts a ``first=True`` argument which says to pop from the
   front or the back. ``dict.popitem`` pops an abitrary item; however
   ``sortedmap`` pops the first or the last item.

5. Method names and semantics from Python 3 are picked over Python 2. This means
   that ``.keys()`` returns a ``keyview`` instead of a list and there is no
   ``.keyview()`` method. This shares more code across Python 2 and 3 and
   anything that is possible with the Python 2 interface is possible with the
   Python 3 interface.

6. Optional ``keyfunc`` for defining the sort order. By default this is the
   identity function. A different key function can be set at object construction
   time like::

     sortedmap[keyfunc](...)

   This can be retrieved later with the ``keyfunc`` attribute of ``sortedmap``
   objects.




Dependencies
------------

``sortedmap`` has no python package depencencies but requires
``CPython 2.7 or >=3.4``. ``sortedmap`` depends on CPython 2 or 3 and some means
of compiling ``C++14``.  We recommend using ``g++`` to compile ``sortedmap``.
Compilation and testing was done with ``gcc 5.3.0``


License
-------

``sortedmap`` is free software, licensed under the GNU Lesser General Public
License, version 2.1. For more information see the ``LICENSE`` file.


Source
------

Source code is hosted on github at
https://github.com/llllllllll/sortedmap.
