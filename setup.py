from distutils.core import setup, Extension
import sys

LONG_DESCRIPTION = None
README_MARKDOWN = None

with open('README.md') as markdown_source:
    README_MARKDOWN = markdown_source.read()

if 'upload' in sys.argv:
    # Converts the README.md file to ReST, since PyPI uses ReST for formatting,
    # This allows to have one canonical README file, being the README.md
    # The conversion only needs to be done on upload.
    # Otherwise, the pandoc import and errors that are thrown when
    # pandoc are both overhead and a source of confusion for general
    # usage/installation.
    import pandoc
    pandoc.core.PANDOC_PATH = 'pandoc'
    doc = pandoc.Document()
    doc.markdown = README_MARKDOWN
    LONG_DESCRIPTION = doc.rst
else:
    # If pandoc isn't installed, e.g. when downloading from pip,
    # just use the regular README.
    LONG_DESCRIPTION = README_MARKDOWN

setup(
    name='cnamedtuple',
    version='0.1.0',
    description='collections.namedtuple implemented in c.',
    author='Joe Jevnik',
    author_email='joe@quantopian.com',
    packages=[
        'cnamedtuple',
    ],
    long_description=LONG_DESCRIPTION,
    license='Apache 2.0',
    classifiers=[
        'Development Status :: 4 - Beta',
        'License :: OSI Approved :: Apache Software License',
        'Natural Language :: English',
        'Programming Language :: Python :: 3 :: Only',
        'Programming Language :: Python :: Implementation :: CPython',
        'Operating System :: OS Independent',
        'Intended Audience :: Science/Research',
        'Topic :: Utilities',
    ],
    url="https://github.com/llllllllll/namedtuple",
    ext_modules=[
        Extension('cnamedtuple._namedtuple', ['cnamedtuple/_namedtuple.c']),
    ],
)
