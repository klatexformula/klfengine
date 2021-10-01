Basic `klfengine` definitions
=============================

File ``<klfengine/basedefs>``
-----------------------------

.. doxygentypedef:: klfengine::binary_data

.. doxygenclass:: klfengine::exception

.. doxygenclass:: klfengine::invalid_json_value

.. doxygenclass:: klfengine::invalid_parameter

.. doxygenclass:: klfengine::invalid_value


File ``<klfengine/value>``
--------------------------

Generic value type
~~~~~~~~~~~~~~~~~~

Provides type-agnostic value storage and JSON data trees.  This type is used,
for instance, to store any additional custom parameters for the
:cpp:struct:`klfengine::input` class.


.. doxygentypedef:: klfengine::value


Related helper functions
~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: klfengine::dict_get(const value::dict &, const std::string &)
.. doxygenfunction:: klfengine::dict_get(const value::dict &, const std::string &, X)
.. doxygenfunction:: klfengine::dict_take(value::dict &, const std::string &)
.. doxygenfunction:: klfengine::dict_take(value::dict &, const std::string &, X)
.. doxygenfunction:: klfengine::dict_do_if(const value::dict &, const std::string &, std::function<void(const X&)>)

.. doxygenclass:: klfengine::parameter_taker


File ``<klfengine/version>``
----------------------------

.. doxygendefine:: KLFENGINE_VERSION_MAJOR
.. doxygendefine:: KLFENGINE_VERSION_MINOR
.. doxygendefine:: KLFENGINE_VERSION_RELEASE
.. doxygendefine:: KLFENGINE_VERSION_SUFFIX

.. doxygenstruct:: klfengine::version_info

.. doxygenfunction:: klfengine::version()
.. doxygenfunction:: klfengine::implementation_version()


