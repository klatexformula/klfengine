How to use `klfengine`
======================

Components of the library
-------------------------

. ... library structure: utilities, input/settings, engines. .......

Engines
-------

. ..... call engine's run() method, obtain "run instance" ...........

. ...... call compile() on the run instance and then get_data(...) .......


The `latextoimage` engine
-------------------------

This engine corresponds closely to KLatexFormula 4's klfbackend workflow.

. .... run (pdf)latex [then run dvips necessary], then run Ghostscript to produce
required output(s). ... ......

. .... description of workflow.


The `klfimplpkg` engine
-----------------------

This engine creates a LaTeX document that directly compiles to the correct
output with appropriate page size, background color, etc.

Ghostscript is used to produce data in the required output format(s). ... ......

. .... description of workflow.


. .... .description of ``klfimpl.sty``, pointer to source file etc.
