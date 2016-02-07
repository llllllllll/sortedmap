from collections import MutableMapping

import pytest

from sortedmap import sortedmap


def test_from_kwargs():
    assert (
        list(sortedmap(a=1, b=2, c=3).items()) ==
        list(zip('abc', (1, 2, 3)))
    )


def test_from_dict():
    assert (
        list(sortedmap({'a': 1, 'b': 2, 'c': 3}).items()) ==
        list(zip('abc', (1, 2, 3)))
    )


def test_from_mapping():
    class M:
        def __init__(self):
            self._map = {'a': 1, 'b': 2, 'c': 3}

        def __getitem__(self, key):
            return self._map[key]

        def keys(self):
            return self._map.keys()

    assert (
        list(sortedmap(M()).items()) ==
        list(zip('abc', (1, 2, 3)))
    )


def test_from_seq2():
    assert (
        list(sortedmap([('a', 1), ('b', 2), ('c', 3)]).items()) ==
        list(zip('abc', (1, 2, 3)))
    )


def test_from_sortedmap():
    m = sortedmap(a=1, b=2, c=3)
    n = sortedmap(m)
    assert n == m
    assert m is not n
    m['d'] = 4
    assert n != m


@pytest.fixture
def m():
    m = sortedmap()
    m['a'] = 1
    m['b'] = 2
    m['c'] = 3
    return m


def test_update_kwargs(m):
    m.update(a=4, b=5, d=6)
    assert m == sortedmap(
        a=4,
        b=5,
        c=3,
        d=6,
    )


def test_update_dict(m):
    m.update({'a': 4, 'b': 5, 'd': 6})
    assert m == sortedmap(
        a=4,
        b=5,
        c=3,
        d=6,
    )


def test_update_mapping(m):
    class M:
        def __init__(self):
            self._map = {'a': 4, 'b': 5, 'd': 6}

        def __getitem__(self, key):
            return self._map[key]

        def keys(self):
            return self._map.keys()

    m.update(M())
    assert m == sortedmap(
        a=4,
        b=5,
        c=3,
        d=6,
    )


def test_is_mapping():
    assert issubclass(sortedmap, MutableMapping)
    assert isinstance(sortedmap(), MutableMapping)


def test_basic():
    m = sortedmap()

    m['a'] = 1
    assert 'a' in m
    assert m['a'] == 1
    assert list(m) == ['a']
    assert m.values() == [1]

    m['c'] = 3
    assert 'c' in m
    assert m['c'] == 3
    assert list(m) == ['a', 'c']
    assert m.values() == [1, 3]

    m['b'] = 2
    assert 'b' in m
    assert m['b'] == 2
    assert list(m) == ['a', 'b', 'c']
    assert m.values() == [1, 2, 3]


def test_keys():
    m = sortedmap(a=1, b=2, c=3)
    assert m.keys() == list('abc')
    assert m.keys() == set('abc')
    assert m.keys() == tuple('abc')
    assert list(m.keys()) == list('abc')  # check iter order

    # check that updates are seen
    view = m.keys()
    m['d'] = 4
    assert view == list('abcd')
    assert view == set('abcd')
    assert view == tuple('abcd')
    assert list(m.keys()) == list('abcd')  # check iter order


def test_values():
    m = sortedmap(a=1, b=2, c=3)
    assert m.values() == [1, 2, 3]
    assert m.values() == {1, 2, 3}
    assert m.values() == (1, 2, 3)
    assert list(m.values()) == [1, 2, 3]  # check iter order

    # check that updates are seen
    view = m.values()
    m['d'] = 4
    assert view == [1, 2, 3, 4]
    assert view == {1, 2, 3, 4}
    assert view == (1, 2, 3, 4)
    assert list(m.values()) == [1, 2, 3, 4]  # check iter order


def test_items():
    m = sortedmap(a=1, b=2, c=3)
    assert m.items() == [('a', 1), ('b', 2), ('c', 3)]
    assert m.items() == {('a', 1), ('b', 2), ('c', 3)}
    assert m.items() == (('a', 1), ('b', 2), ('c', 3))
    # check iter order
    assert list(m.items()) == [('a', 1), ('b', 2), ('c', 3)]

    # check that updates are seen
    view = m.items()
    m['d'] = 4
    assert view == [('a', 1), ('b', 2), ('c', 3), ('d', 4)]
    assert view == {('a', 1), ('b', 2), ('c', 3), ('d', 4)}
    assert view == (('a', 1), ('b', 2), ('c', 3), ('d', 4))
    # check iter order
    assert list(m.items()) == [('a', 1), ('b', 2), ('c', 3), ('d', 4)]


@pytest.mark.parametrize('itertype', ('keys', 'values', 'items'))
def test_invalidate_iter(itertype):
    m = sortedmap()
    it = iter(getattr(m, itertype)())
    m[1] = 1  # update the size
    with pytest.raises(RuntimeError):
        next(it)  # advance the iterator

    it = iter(getattr(m, itertype)())
    m[1] = 2  # change a value without changing the size
    next(it)  # works without raising
