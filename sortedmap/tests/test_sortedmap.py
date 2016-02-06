from collections.abc import MutableMapping

from sortedmap import sortedmap


def test_from_kwargs():
    assert list(sortedmap(a=1, b=2, c=3)) == list('abc')


def test_from_dict():
    assert list(sortedmap({'a': 1, 'b': 2, 'c': 3})) == list('abc')


def test_from_mapping():
    class M:
        def __init__(self):
            self._map = {'a': 1, 'b': 2, 'c': 3}

        def __getitem__(self, key):
            return self._map[key]

        def keys(self):
            return self._map.keys()

    assert list(sortedmap(M())) == list('abc')


def test_from_seq2():
    assert list(sortedmap([('a', 1), ('b', 2), ('c', 3)])) == list('abc')


def test_from_sortedmap():
    m = sortedmap(a=1, b=2, c=3)
    n = sortedmap(m)
    assert n == m
    assert m is not n
    m['d'] = 4
    assert n != m


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
