# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

import re
import textwrap # dedent()

# -- Project information -----------------------------------------------------

project = 'klfengine'
copyright = '2021, Philippe Faist'
author = 'Philippe Faist'

# The full version, including alpha/beta/rc tags
#
# Parse version information from include/klfengine/h/version_number_raw.h
relase = None
with open('../include/klfengine/h/version_number_raw.h') as f:
    v_d = {}
    for m in re.finditer(
            r'^\#define\s+KLFENGINE_VERSION_(?P<vpartname>[A-Za-z0-9_]+)\s+(?P<val>.*?)\s*$',
            f.read(),
            flags=re.MULTILINE
    ):
        v_d[m.group('vpartname')] = eval(m.group('val'))
    release = "{}.{}.{}{}".format(v_d.get('MAJOR'), v_d.get('MINOR'), v_d.get('RELEASE'),
                                  v_d.get('SUFFIX'))
print("Parsed klfengine version from header file = {!r}".format(release))
#release = '0.1'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'breathe',
#    'exhale',
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# -- Options for C++/Doxygen/Breathe -----------------------------------------

# Setup the breathe extension
breathe_projects = {
    "klfengine": "./doxyoutput/xml"
}
breathe_default_project = "klfengine"

#breathe_default_members = ('members', 'protected-members', 'private-members', 'undoc-members')
breathe_default_members = ('members', 'protected-members', 'undoc-members')

# # Setup the exhale extension
# exhale_args = {
#     # These arguments are required
#     "containmentFolder":     "./api",
#     "rootFileName":          "library_root.rst",
#     "rootFileTitle":         "Library API",
#     "doxygenStripFromPath":  "..",
#     # Suggested optional arguments
#     "createTreeView":        True,
#     # TIP: if using the sphinx-bootstrap-theme, you need
#     # "treeViewIsBootstrap": True,
#     "exhaleExecutesDoxygen": True,
#     "exhaleDoxygenStdin":    textwrap.dedent("""
#
#         # where our documented header files are
#         INPUT = ../include/klfengine/h
#
#         # some predefined macros
#         PREDEFINED += _KLFENGINE_PROCESSED_BY_DOXYGEN
#
# """)
# }

# Tell sphinx what the primary language being documented is.
primary_domain = 'cpp'

# Tell sphinx what the pygments highlight language should be.
highlight_language = 'cpp'


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'alabaster'

html_theme_options = {
    'logo': 'klflogo.svg',
    'logo_name': True,
    'github_user': 'klatexformula',
    'github_repo': 'klfengine',

    # fonts

    'font_family': "'Merriweather light', serif",
    'font_size': '15px',
    'head_font_family': "'Merriweather', serif",

    'code_font_size': '14px',
    'code_font_family': "'Consolas', monospace",

    'caption_font_size': '16px', # e.g. "Contents:"
    'caption_font_family': "'Merriweather', serif", 

}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = [ '_static' ]
