from collections import MutableMapping

from ._sortedmap import sortedmap


MutableMapping.register(sortedmap)
del MutableMapping


__version__ = '0.2.0'


__all__ = [
    'sortedmap',
]
