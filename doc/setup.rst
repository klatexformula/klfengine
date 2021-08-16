Setting up `klfengine` for your project
=======================================

The official way to use `klfengine` into your project is using `CMake
<https://cmake.org/>`_.  Here are a few steps to get started.

Setting up `klfengine`
----------------------

1. Download klfengine.  You can clone for instance the git repo::

     ~/MyDownlaods> git clone https://github.com/klatexformula/klfengine.git
     ~/MyDownloads> cd klfengine/

2. Run cmake a first time::

     ~/MyDownlaods/klfengine> mkdir build
     ~/MyDownlaods/klfengine> cd build
     ~/MyDownlaods/klfengine/build> cmake ..

3. Read CMake's output.  At this point you might have to worry about making all
   of klfengine's (C++ header-only) dependencies available.  Check the output of
   cmake and follow the given instructions.

   If you have missing dependencies, You can run a simple custom script that
   will automatically download and "install" them locally within the klfengine
   directory.  In the sources root directory, you can run::

     ~/MyDownloads/klfengine> python3 fetch_and_setup_dependencies.py

   (Use the ``--help`` switch to see how to download and set up individual
   dependencies.)

   Then re-run cmake.  You can set CMake variables using the syntax
   "-DVARIABLENAME=VALUE".  For instance, to install klfengine at a custom
   location, you can run::

     ~/MyDownloads/klfengine/build> cmake .. -DCMAKE_INSTALL_PREFIX=/custom/path

4. Run ``make install``::

     ~/MyDownloads/klfengine/build> make install


**At this point, you have installed klfengine (perhaps to a custom location if
you set CMAKE_INSTALL_PREFIX) and it can be used in your projects.**

.. note:: If you prefer not to install the library on your system, even in a
          user-local folder like ``~/.local/``, you can still download and
          "install" `klfengine` in a folder within your project.  Control the
          installation location with ``CMAKE_INSTALL_PREFIX``; when you set up
          your project, indicate to cmake where to look for those installed
          files by setting ``CMAKE_PREFIX_PATH``.


Setting up your project
-----------------------

For use in your project, simply use CMake's ``find_package(klfengine)``::

  # your project's CMakeLists.txt:
  project(...)

  find_package(klfengine REQUIRED)

  ...

  target_link_libraries(mytarget
    PRIVATE klfengine::klfengine
  )

  ...

When you run CMake, `klfengine`'s configuration scripts will tell you how
`klfengine` is configured and explain which CMake variables to set to change its
configuration.


Options for using `klfengine`
-----------------------------

The klfengine library can be configured to be used in different ways.  Each
option can be specified as a CMake variable ``KLFENGINE_XXXXXX`` at the time you
set up your project.

For instance, you can set CMake variables on the command line when running cmake
for your project, as follows::

  ~/path/to/myproject/build> cmake .. -DKLFENGINE_USE_MPARK_VARIANT=true

You can also set this variable in your CMake project file.  For instance::

  # your project's CMakeLists.txt:
  project(...)

  set(KLFENGINE_SEPARATE_IMPLEMENTATION true)
  find_package(klfengine REQUIRED)

  ...


The possible options you can set are the following.

* **Variant type:** `klfengine` uses the std::variant type that was introduced
  into C++17.  Alternatively, `klfengine` can also use instead the third-party
  library `mpark/variant <https://github.com/mpark/variant>`_ that also provides
  the same functionality but is compatible with C++11 and C++14.

  If you set the CMake variable ``KLFENGINE_USE_MPARK_VARIANT=true``, then
  `klfengine` will be set up to use the `mpark/variant` library instead of
  C++17's `std::variant`.  You can specify the location prefix where the
  mpark/variant library was installed and is to be searched for via the CMake
  variable ``CMAKE_PREFIX_PATH``.


* **Filesystem library:** `klfengine` uses the std::filesystem library that was
  introduced into C++17.  Alternatively, `klfengine` can also use instead the
  third-party library `gulrak/filesystem
  <https://github.com/gulrak/filesystem>`_ that also provides the same
  functionality but is compatible with C++11 and C++14.

  If you set the CMake variable ``KLFENGINE_USE_GULRAK_FILESYSTEM=true``, then
  `klfengine` will be set up to use the `gulrak/filesystem` library instead of
  C++17's `std::filesystem`.  You can specify the location prefix where the
  gulrak/filesystem library was installed and is to be searched for via the
  CMake variable ``CMAKE_PREFIX_PATH``.


* **Separate implementation:** `klfengine` is a header-only library, meaning
  that the implementation is contained in the headers that are included in your
  project.  This structure enables you to use `klfengine` without having to
  worry about compiling a separate library and linking to that library.

  Yet if you use `klfengine` in multiple source files, the implementation code
  is included in each of your source files and is compiled multiple times,
  slowing down the compilation of your project.  The `klfengine` headers offer a
  way to prevent this redundancy: If you set the CMake variable
  ``KLFENGINE_SEPARATE_IMPLEMENTATION=true``, then the headers won't include the
  implementation source code.  Instead, you are responsible for issuing the
  instruction ``#include <klfengine/implementation>`` in a single one of your
  source files.  In this case, the klfengine implementation sources are compiled
  only once, in a single compilation unit.


* **Linking and/or loading Ghostscript's C API Library:** The `Ghostscript
  interpreter <https://www.ghostscript.com/>`_ plays a crucial role in
  klfengine's compilation process.  The `klfengine` library can run
  Ghostscript's ``gs`` program to perform any required processing.  Instead of
  launching an external process, you can also instruct `klfengine` to use
  Ghostscript's C API library (`libgs`).  There are two options: `klfengine` can
  be directly linked against `libgs` (set cmake variable
  ``KLFENGINE_USE_LINKED_GHOSTSCRIPT=true``) or it can load `libgs` at run-time
  (set cmake variable ``KLFENGINE_USE_LOAD_GHOSTSCRIPT=true``).  In both cases,
  CMake needs to be able to find Ghostscript's C API headers, and in case you
  link directly against the library, it needs to find the library itself.

  .. warning:: loading `libgs` at runtime isn't yet implemented.
