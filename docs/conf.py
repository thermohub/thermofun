# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The site of the documentation, built with Sphinx from the Markdown pages of this folder:
#   pip install -r docs/requirements.txt
#   sphinx-build -W --keep-going -b html docs docs/_build/html
# The tutorials are notebooks (MyST Markdown) and run at each build, with the thermofun that Python imports.

import os
import pathlib
import re

project = "ThermoFun"
author = "ThermoFun contributors"
copyright = "2026, ThermoFun contributors"

extensions = [
    "myst_nb",
    "sphinx_copybutton",
]

# the pages are Markdown; the notebooks are the Markdown pages with a kernel in their front matter
source_suffix = {".md": "myst-nb"}
root_doc = "index"
exclude_patterns = ["_build", "**/.ipynb_checkpoints", "**/output"]

myst_enable_extensions = ["dollarmath", "amsmath", "colon_fence", "deflist"]
myst_heading_anchors = 3

# run each notebook at each build, and stop at the first error of a cell
nb_execution_mode = os.environ.get("THERMOFUN_DOCS_EXECUTE", "force")
nb_execution_raise_on_error = True
nb_execution_timeout = 300
nb_merge_streams = True

# a link to a file of the repository outside this folder goes to that file on GitHub
DOCS = pathlib.Path(__file__).resolve().parent
REPOSITORY = "https://github.com/thermohub/thermofun/blob/v2/"
LINK = re.compile(r"\]\((?!https?:|#|mailto:)([^)#\s]+)(#[^)\s]*)?\)")


def _outside_links_to_github(app, docname, source):
    folder = (DOCS / docname).parent

    def replace(match):
        target = (folder / match[1]).resolve()
        if target.is_relative_to(DOCS) or not target.exists():
            return match[0]
        return "](%s%s%s)" % (REPOSITORY, target.relative_to(DOCS.parent).as_posix(), match[2] or "")

    source[0] = LINK.sub(replace, source[0])


def setup(app):
    app.connect("source-read", _outside_links_to_github)

html_theme = "pydata_sphinx_theme"
html_title = "ThermoFun"
html_theme_options = {
    "github_url": "https://github.com/thermohub/thermofun",
    "use_edit_page_button": True,
    "navigation_with_keys": False,
    "show_toc_level": 2,
}
html_context = {
    "github_user": "thermohub",
    "github_repo": "thermofun",
    "github_version": "v2",
    "doc_path": "docs",
}
