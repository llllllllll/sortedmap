#!/usr/bin/env python
from setuptools import setup, Extension
import sys

long_description = ''

if 'upload' in sys.argv:
    with open('README.rst') as f:
        long_description = f.read()

setup(
    name='sortedmap',
    version='0.1.0',
    description='A sorted map that does not use hashing.',
    author='Joe Jevnik',
    author_email='joejev@gmail.com',
    packages=[
        'sortedmap',
    ],
    long_description=long_description,
    license='GPL-2',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'License :: OSI Approved :: GNU General Public License v2 (GPLv2)',
        'Natural Language :: English',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: Implementation :: CPython',
        'Operating System :: POSIX',
    ],
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
