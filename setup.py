#!/usr/bin/env python
from setuptools import setup, Extension
import sys

long_description = ''

if 'upload' in sys.argv:
    with open('README.rst') as f:
        long_description = f.read()


classifiers = [
    'Development Status :: 3 - Alpha',
    'Intended Audience :: Developers',
    'License :: OSI Approved :: GNU Lesser General Public License v2 (LGPLv2)',
    'Natural Language :: English',
    'Programming Language :: C++',
    'Programming Language :: Python :: 2',
    'Programming Language :: Python :: 3',
    'Programming Language :: Python :: Implementation :: CPython',
]

setup(
    name='sortedmap',
    version='0.2.0',
    description='A sorted mapping object',
    author='Joe Jevnik',
    author_email='joejev@gmail.com',
    packages=[
        'sortedmap',
    ],
    long_description=long_description,
    license='LGPLv2',
    classifiers=classifiers,
    url='https://github.com/llllllllll/sortedmap',
    ext_modules=[
        Extension(
            'sortedmap._sortedmap',
            ['sortedmap/_sortedmap.cpp'],
            include_dirs=['sortedmap/include'],
            depends=['sortedmap/include/sortedmap.h'],
            extra_compile_args=[
                '-Wall',
                '-Wextra',
                '-Wno-missing-field-initializers',
                '-Wno-unused-parameter',
                '-std=gnu++14',
            ],
            language='c++',
        ),
    ],
    extras_require={
        'test': ['pytest==2.8.7', 'pytest-pep8>=1.0.6'],
    },
)
