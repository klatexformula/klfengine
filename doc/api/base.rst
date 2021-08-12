Basic `klfengine` definitions
=============================

``<klfengine/basedefs>``
------------------------

.. doxygentypedef:: klfengine::binary_data

.. doxygenclass:: klfengine::exception


``<klfengine/value>``
------------------------

Generic value type
~~~~~~~~~~~~~~~~~~

Provides type-agnostic value storage and JSON data trees.  This type is used,
for instance, to store any additional custom parameters for the
:cpp:struct:`klfengine::input` class.


.. doxygentypedef:: klfengine::value


Related helper functions
~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: klfengine::dict_get(const value::dict &, const std::string &)


``<klfengine/version>``
-----------------------

.  ............


``<klfengine/process>``
-----------------------

.  ..................


``<klfengine/temporary_directory>``
-----------------------------------

.   ....................


