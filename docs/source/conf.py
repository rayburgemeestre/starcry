# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'starcry'
copyright = '2023, Ray Burgemeestre'
author = 'Ray Burgemeestre'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = []

templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'alabaster'
html_static_path = ['_static']

# breathe
import os
import sys
sys.path.append(os.getcwd() + "/breathe/")
#extensions = ['sphinx.ext.pngmath', 'sphinx.ext.todo', 'breathe' ]
extensions = ['sphinx_rtd_theme', 'breathe']
breathe_projects = {"starcry": os.getcwd() + "/../doxygen/xml/"}

breathe_default_project = "starcry"

html_theme = "sphinx_rtd_theme"

#import sphinx_bootstrap_theme
#html_theme_path = sphinx_bootstrap_theme.get_html_theme_path()
#html_theme = "bootstrap"
# html_theme_options = {
#     'collapse_navigation': False,
#     'navigation_depth': 4,
#     'sticky_navigation': False,
# }

