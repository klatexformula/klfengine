.. klfengine documentation master file, created by
   sphinx-quickstart on Sun Jul 12 18:16:58 2020.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to klfengine's documentation!
=====================================

The `klfengine` library is a C++11/14/17 header-only library enabling the
compilation of LaTeX equations into various vector and pixel image formats.

While `klfengine` is header-only, it depends on a handful of other header-only
C++11 libraries, and it requires access to a working LaTeX distribution at run
time (e.g., `texlive <http://tug.org/texlive/>`_).  Most output formats produced
by `klfengine` also require `Ghostscript <https://www.ghostscript.com/>`_.

This library will provide the core functionality of future versions of
KLatexFormula.


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   setup
   using
   apidoc



Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
