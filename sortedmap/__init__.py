from collections import MutableMapping

from ._sortedmap import sortedmap


MutableMapping.register(sortedmap)
del MutableMapping


__all__ = [
    'sortedmap',
]
