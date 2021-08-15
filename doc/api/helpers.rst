Some helpers for `klfengine`
============================


Create temporary directories — File ``<klfengine/temporary_directory>``
-----------------------------------------------------------------------

.. doxygenclass:: klfengine::temporary_directory


Execute processes - File ``<klfengine/process>``
------------------------------------------------

Manipulate process environments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygentypedef:: klfengine::environment
.. doxygenfunction:: klfengine::current_environment
.. doxygenfunction:: klfengine::parse_environment

.. doxygenfunction:: klfengine::set_environment
.. doxygenstruct:: klfengine::provide_environment_variables
.. doxygenstruct:: klfengine::set_environment_variables
.. doxygenstruct:: klfengine::remove_environment_variables
.. doxygenstruct:: klfengine::prepend_path_environment_variables
.. doxygenstruct:: klfengine::append_path_environment_variables


Execute subprocesses
~~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: klfengine::process_exit_error

.. doxygenclass:: klfengine::process

