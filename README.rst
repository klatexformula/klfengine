klfengine
==========

C++ header-only library to convert latex equations into various formats.

Requires some other C++ header-only libraries when compiling your project.  At
run-time, `klfengine` requires a latex distribution as well as `Ghostscript`.



Quick how to
------------

- Get all dependency headers, install them to a custom location, and install
  `klfengine` there, too:

  .. code-block:: bash

     > python3 ./fetch_and_setup_dependencies.py --prefix=/custom/location/prefix
     > mkdir build && cd build
     build> cmake .. -DCMAKE_PREFIX_PATH=/custom/location/prefix \
                     -DCMAKE_INSTALL_PREFIX=/custom/location/prefix
     build> make install

- Generate the documentation:

  .. code-block:: bash

     > cd doc/
     doc> poetry run doxygen
     doc> poetry run make html
 
     # View index file doc/_build/html/index.html




License
-------

See LICENSE.txt (MIT License).
