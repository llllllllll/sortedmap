from collections import MutableMapping

import pytest

from sortedmap import sortedmap


@pytest.fixture
def m():
    m = sortedmap()
    m['a'] = 1
    m['b'] = 2
    m['c'] = 3
    return m


@pytest.fixture
def keyfunc_m():
    return sortedmap[len](abc=1, bc=2, c=3)


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


def test_from_sortedmap_different_keyfunc():
    m = sortedmap[len](abc=1, bc=2, c=3)
    n = sortedmap(m)
    assert m != n
    assert list(m.items()) == [('c', 3), ('bc', 2), ('abc', 1)]


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


def test_get(m):
    ob = object()

    def assertion(key, value):
        assert m.get(key) == value
        assert m.get(key, ob) == value

    assertion('a', 1)
    assertion('b', 2)
    assertion('c', 3)
    assert m.get('d') is None
    assert m.get('d', ob) is ob


def test_pop(m):
    n = m.copy()
    ob = object()

    def assertion(key, value):
        assert m.pop(key) == value
        assert key not in m
        assert n.pop(key, ob) == value
        assert key not in n

    assertion('a', 1)
    assertion('b', 2)
    assertion('c', 3)

    with pytest.raises(KeyError) as e:
        m.pop('d')
    assert e.value.args[0] == 'd'

    assert n.pop('d', ob) is ob


def test_popitem(m):
    assert m.popitem() == ('a', 1)
    assert m.popitem(first=False) == ('c', 3)
    assert m.popitem(False) == ('b', 2)

    with pytest.raises(KeyError):
        m.popitem()


def test_setdefault():
    m = sortedmap()

    assert m.setdefault('a') is None
    assert m.setdefault('a', 1) is None

    assert m.setdefault('b', 1) == 1
    assert m.setdefault('b', 2) == 1


def test_fromkeys():
    keys = 'abc'
    assert sortedmap.fromkeys(keys) == sortedmap(a=None, b=None, c=None)
    ob = object()
    assert sortedmap.fromkeys(keys, ob) == sortedmap(a=ob, b=ob, c=ob)


def test_clear(m):
    n = sortedmap(m)
    m.clear()
    assert not len(m)
    assert m == sortedmap()

    # check that copy constructor doesn't share the maps
    assert len(n) == 3
    assert n == sortedmap(a=1, b=2, c=3)


def test_repr(m, keyfunc_m):
    assert repr(m) == "sortedmap.sortedmap([('a', 1), ('b', 2), ('c', 3)])"
    assert repr(keyfunc_m) == (
        "sortedmap.sortedmap[<built-in function len>]"
        "([('c', 3), ('bc', 2), ('abc', 1)])"
    )


def test_keyview_repr(m):
    assert repr(m.keys()) == "sortedmap.keyview(['a', 'b', 'c'])"


def test_valview_repr(m):
    assert repr(m.values()) == 'sortedmap.valview([1, 2, 3])'


def test_itemview_repr(m):
    assert (
        repr(m.items()) == "sortedmap.itemview([('a', 1), ('b', 2), ('c', 3)])"
    )


def test_keyfunc_partial_repr():
    assert (
        repr(sortedmap[len]) == 'sortedmap.sortedmap[<built-in function len>]'
    )


def test_keyfunc(keyfunc_m):
    assert keyfunc_m.keys() == ['c', 'bc', 'abc']
    assert dict(keyfunc_m) == {'abc': 1, 'bc': 2, 'c': 3}
