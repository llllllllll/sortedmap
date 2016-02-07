``sortedmap 0.1``
=================

A sorted mapping object.

``sortedmap`` is a python dict api interface to the C++ std::map type.
``sortedmap`` implements the full ``dict`` object interface with a few
additions:

1. ``popitem`` accepts a ``first=True`` argument which says to pop from the
   front or the back. ``dict`` pops a random item; however ``sortedmap`` pops
   the first or the last item.
2. ``reverse_(keys|values|items)`` Implement reverse views that have reverse
   iteration semantics.  (**not implemented**)


Dependencies
------------

``sortedmap`` depends on CPython 2 or 3 and some means of compiling C++14
We recommend using ``g++`` to compile ``sortedmap``.
